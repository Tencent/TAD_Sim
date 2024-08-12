// load 阶段解析过程
import { get, pick } from 'lodash'
import { CatmullRomCurve3, MathUtils, Vector2, Vector3 } from 'three'
import pool from '../worker'
import { parseObject } from './object'
import { useEditRoadStore } from '@/plugins/editRoad/store'
import { useJunctionStore } from '@/stores/junction'
import { useObjectStore } from '@/stores/object'
import { useRoadStore } from '@/stores/road'
import { getBoundaryById } from '@/stores/road/boundary'
import {
  Constant,
  LaneFriction,
  LaneTypeEnum,
  MapVersion,
  judgeTrendByBoundarySamplePoints,
  parseLaneBoundaryMark,
} from '@/utils/business'
import {
  CatmullromTension,
  CurveType,
  PI,
  createArcPoints,
  fixedPrecision,
  getCurveEndPointVerticalVec3,
  getValidDegreeInRange,
  getValidRadInRange,
  halfPI,
  sortById,
  transformCoordinateToThree,
  transformVec3ByObject,
} from '@/utils/common3d'
import { cacheId, genRoadId, genUuid } from '@/utils/guid'
import { getRoad, setJunction, setRoad } from '@/utils/mapCache'
import { uniqVec3Array } from '@/utils/tools'
import { globalConfig } from '@/utils/preset'

// 本地临时缓存
let tempRoadIds: Array<string> | null = null
let tempControlPoints: Array<biz.IControlPoint> | null = null
let tempJunctionIds: Array<string> | null = null

export async function parseMapElements (originData: biz.ICommonMapElements) {
  console.log('=load file=', originData)
  // 优先解析元素 id
  const { junctionIds } = parseElementsId(originData)

  const { roads, lanelinks = [], objects = [], header } = originData

  // 是否是旧版地图编辑器自制的地图
  let isOldMap = false
  if (header && header.version && header.version === MapVersion.v1) {
    isOldMap = true
  }

  // 先解析道路元素
  console.time('parse road')
  await parseRoadAndJunction({
    roads,
    junctionIds,
    lanelinks,
  })
  console.timeEnd('parse road')
  // 再解析物体元素
  const objectIds = await parseObject({
    objects,
    laneLinks: lanelinks,
    isOldMap,
    controlPoints: tempControlPoints || [],
  })

  // 在完成所有数据解析以后，触发 store 的方法，调用渲染层
  const syncAllStore = []
  if (tempRoadIds) {
    const roadStore = useRoadStore()
    // 将 road 数据同步到 store 中
    syncAllStore.push(
      roadStore.applyMapFileState({
        ids: tempRoadIds,
        elements: [],
      }),
    )
  }

  if (tempJunctionIds) {
    // 将解析的交叉路口和车道连接线的数据，同步到前端 store 中
    const junctionStore = useJunctionStore()
    syncAllStore.push(
      junctionStore.applyMapFileState({
        ids: tempJunctionIds,
        elements: [],
      }),
    )
  }

  if (tempControlPoints) {
    if (useEditRoadStore) {
      // 将控制点的状态同步到 store 中
      const editRoadStore = useEditRoadStore()
      syncAllStore.push(
        editRoadStore.applyMapFileState({
          controlPoints: tempControlPoints,
        }),
      )
    }
  }

  if (objectIds) {
    // 将物体的状态同步到 store 中
    const objectStore = useObjectStore()
    syncAllStore.push(
      objectStore.applyMapFileState({
        ids: objectIds,
        elements: [],
      }),
    )
  }

  await Promise.all(syncAllStore)

  tempRoadIds = null
  tempControlPoints = null
  tempJunctionIds = null
}

function parseElementsId (data: biz.ICommonMapElements) {
  console.time('parseId')
  const { roads = [], lanelinks = [], objects = [] } = data

  // 缓存道路元素的 id
  for (const road of roads) {
    cacheId('road', road.id)
    for (const section of road.sections) {
      for (const boundary of section.boundarys) {
        // 优先对 boundaryId 做缓存
        cacheId('laneBoundary', boundary.id)
      }
    }
  }

  // 通过 laneLink 解析 junctionId
  // 缓存车道连接线的 id
  const junctionIds: Array<string> = []
  for (const laneLink of lanelinks) {
    const { junctionid, id: laneLinkId, roadid = '' } = laneLink
    if (!junctionIds.includes(junctionid)) {
      junctionIds.push(junctionid)
    }
    cacheId('laneLink', laneLinkId)

    // 为了适配 xodr 文件增量保存 lanelink，在 lanelink 属性中挂载了唯一的 roadid
    if (roadid && roadid !== '-1') {
      // 如果 roadid 有效，也需要做唯一性的缓存
      cacheId('road', roadid)
    }
  }
  // 缓存路口元素的 id
  for (const junctionId of junctionIds) {
    cacheId('junction', junctionId)
  }
  console.timeEnd('parseId')

  // 缓存物体元素的 id
  for (const object of objects) {
    cacheId('object', object.id)
  }

  return {
    junctionIds,
  }
}

// 将每一条道路的解析抽离，同时解析多个道路
async function parseOneRoad (params: {
  road: biz.ICommonRoad
  roadIds: Array<string>
  controlPoints: Array<biz.IControlPoint>
  boundaryMap: Map<string, biz.ILaneBoundary>
}) {
  const { road, roadIds, controlPoints, boundaryMap } = params
  return new Promise((resolve) => {
    const {
      id: roadId,
      samplePoints: originRoadSamplePoints,
      sections,
      length: roadLength,
      type: roadType,
      controlPoints: originControlPoints = [],
      controlType = 'catmullrom',
      elevation: originElevation,
    } = road

    let editRoadStore
    if (useEditRoadStore) {
      editRoadStore = useEditRoadStore()
    }

    // 道路参考线必经的采样点
    let _samplePoints: Array<Vector3> = []

    let isCircleRoad = false
    let circleOption: biz.ICircleOption | null = null

    // 如果有道路参考线控制点的数据，则直接使用控制点数据来创建道路参考线即可
    if (controlType && originControlPoints.length > 0) {
      // 高效率、粗略地取整，不要求精确度
      originControlPoints.forEach((p: common.vec3) => {
        p.x = Number(p.x.toFixed(4))
        p.y = Number(p.y.toFixed(4))
        p.z = Number(p.z.toFixed(4))
      })
      // 去重和转换成符合编辑器三维场景规范的坐标
      const uniqControlPoints = uniqVec3Array(
        transformCoordinateToThree(originControlPoints),
      )
      // 转换成 Vector3 类型
      _samplePoints = transformVec3ByObject(uniqControlPoints)

      // 如果控制点的类型是 arc 圆弧类型，需要基于控制点重新采样
      if (controlType === 'arc') {
        // 圆弧只提供2个控制点
        const startPoint = _samplePoints[0]
        const startHdg = originControlPoints[0].hdg
        const endPoint = _samplePoints[1]
        const endHdg = originControlPoints[1].hdg
        // 基于2个环形道路的控制点，计算环形道路对应的属性
        circleOption = parseArcControlPoint(
          {
            x: startPoint.x,
            y: startPoint.y,
            z: startPoint.z,
            hdg: startHdg,
          },
          {
            x: endPoint.x,
            y: endPoint.y,
            z: endPoint.z,
            hdg: endHdg,
          },
        )

        if (
          circleOption &&
          circleOption.points &&
          circleOption.points.length > 2
        ) {
          // 认为是之前创建的环形道路，即圆弧道路
          isCircleRoad = true
          // 使用计算的道路参考线采样点
          _samplePoints = transformVec3ByObject(circleOption.points)
        }
      } else if (controlType === 'catmullrom') {
        // 如果是 catmullrom 线形的控制点，在保存透传时多传了首尾2个虚拟控制点，在重新加载时需要将这2个虚拟控制点删除
        _samplePoints.shift()
        _samplePoints.pop()
      }
    } else {
      // 如果没有控制点，则直接使用采样点作为 catmullrom 线形的控制点（主要针对第三方地图）
      // 先对地图中加载的采样点数据做精度调整，避免后端给到的数据精度不准确带来的去重误差
      // 高效率、粗略地取整，不要求精确度
      originRoadSamplePoints.forEach((p: common.vec3) => {
        p.x = Number(p.x.toFixed(4))
        p.y = Number(p.y.toFixed(4))
        p.z = Number(p.z.toFixed(4))
      })

      // 将地图文件中存储的道路中心线的采样点还原成三维样条曲线的关键控制点时，需要对原采样点做去重处理，保证还原效果稳定性和准确性
      const uniqRoadSamplePoints = uniqVec3Array(
        // 道路中心线采样点坐标转换
        transformCoordinateToThree(originRoadSamplePoints),
      )
      // 将采样点转换成 Vector3 类型
      _samplePoints = transformVec3ByObject(uniqRoadSamplePoints)
    }

    // 车道中心线关键道路结构
    const keyPath = new CatmullRomCurve3(_samplePoints)
    keyPath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      keyPath.tension = CatmullromTension
    }
    // 通过采样点还原的道路中心线长度
    const calcRoadLength = fixedPrecision(keyPath.getLength())

    let elevationPath = null
    if (originElevation && originElevation.length > 1) {
      const _controlPoints = []
      for (let i = 0; i < originElevation.length; i++) {
        const p = originElevation[i]
        if (i === 0) {
          // 第一个控制点
          _controlPoints.push({
            x: 0,
            y: p.h,
            z: 0,
          })
        } else if (i === originElevation.length - 1) {
          // 最后一个控制点
          _controlPoints.push({
            x: calcRoadLength,
            y: p.h,
            z: 0,
          })
        } else {
          _controlPoints.push({
            x: p.s,
            y: p.h,
            z: 0,
          })
        }
      }
      // 高程的曲线
      elevationPath = new CatmullRomCurve3(
        transformVec3ByObject(_controlPoints),
      )
      elevationPath.curveType = CurveType
      if (CurveType === 'catmullrom') {
        elevationPath.tension = CatmullromTension
      }
    } else {
      // 如果道路默认没有高程数据，则默认添加一条
      const pHead = { x: 0, y: 0, z: 0 }
      const pTail = { x: calcRoadLength, y: 0, z: 0 }
      elevationPath = new CatmullRomCurve3(
        transformVec3ByObject([pHead, pTail]),
      )
      elevationPath.curveType = CurveType
      if (CurveType === 'catmullrom') {
        elevationPath.tension = CatmullromTension
      }
    }

    // TODO 如果跟文件中存储的长度有偏差可以通过提示给出，尽可能将误差控制到毫米级别
    // 由于误差比较小并且是毫米级别，是否可以直接将地图文件中的road/section length直接应用到前端侧？？
    if (calcRoadLength !== roadLength) {
      // ...
    }

    // TODO 将采样点通过算法采样成控制点同步到 editRoad 插件中的控制点状态
    // TODO 先全量作为控制点
    if (editRoadStore) {
      const parentId = genUuid()
      const _points = _samplePoints.map(p => ({
        x: p.x,
        y: p.y,
        z: p.z,
        parentId,
        id: genUuid(),
      }))
      const _controlPoint = {
        id: parentId,
        // 加载的地图，每条 road 都单独有一组控制点
        roadId: [roadId],
        points: _points,
      }
      controlPoints.push(_controlPoint)
    }

    const _sections: Array<biz.ISection> = []
    // 将通用 section 的属性，调整成符合前端渲染的状态
    for (const section of sections) {
      const { id: sectionId, startPercent, endPercent } = section
      // 基于重新计算的道路中心线长度计算当前 section 的长度
      const newSectionLength = fixedPrecision(
        calcRoadLength * (endPercent - startPercent),
      )
      const _section: biz.ISection = {
        id: sectionId,
        length: newSectionLength,
        pStart: startPercent,
        pEnd: endPercent,
        lanes: [],
        boundarys: [],
      }

      const _lanes: Array<biz.ILane> = []
      const _boundarys: Array<biz.ILaneBoundary> = []

      // 先解析车道边界线，作为解析车道的前置依赖
      for (const boundary of section.boundarys) {
        const { samplePoints: boundarySamplePoints = [] } = boundary
        if (boundarySamplePoints.length < 1) continue
        // 将车道边界线采样点做坐标转换
        const _boundarySamplePoints =
          transformCoordinateToThree(boundarySamplePoints)

        // 在初始化边界线数据时，默认几何体属性为空
        const _boundary: biz.ILaneBoundary = {
          id: boundary.id,
          mark: boundary.mark || 0, // 如果接口数据没有返回 mark 字段，则默认为 mark = 0，无边界
          samplePoints: _boundarySamplePoints,
          firstLineAttr: null,
          secondLineAttr: null,
        }
        _boundarys.push(_boundary)

        // 将 boundary 对象缓存到映射表中，便于查找
        boundaryMap.set(boundary.id, _boundary)
      }

      // 后解析车道
      for (const lane of section.lanes) {
        const { samplePoints: laneSamplePoints = [], lbid, rbid } = lane
        if (laneSamplePoints.length < 1) continue
        // 将车道中心线采样点做坐标转换
        const _laneSamplePoints = transformCoordinateToThree(laneSamplePoints)
        // 采样点精度调整

        // 由于之前已经保存的地图文件不一定存在 friction 和 sOffset 属性，手动兼容
        const type: number = lane.type
        // 通过枚举获取对应的车道类型字符串
        const typeStr = LaneTypeEnum[type] as biz.ILaneType
        // 如果从文件中有透传，则适用透传的数值；如果没有则适用默认值
        const friction = lane?.friction || LaneFriction[typeStr] || 0
        let sOffset = lane?.sOffset || 0
        // 如果地图文件中透传的材质纵向距离超过 section 长度，则进行限制
        if (sOffset > newSectionLength) {
          sOffset = newSectionLength
        }

        const _lane: biz.ILane = {
          samplePoints: _laneSamplePoints,
          ...pick(lane, ['id', 'type', 'lbid', 'rbid', 'speedlimit']),
          enabled: true, // 从文件中加载的 lane 都是有效的
          isTransition: false, // 先默认为 false，后续再做解析验证
          normalWidth: 0, // 先默认为 0，后续再做解析更新
          geoAttr: null,
          friction,
          sOffset,
        }

        // 判断当前车道的边界线是否有平滑过渡的趋势，如果有则需要单独处理
        // 以及如果是平滑过渡车道，需要判断趋势 isExtends
        const leftBoundary = getBoundaryById(_boundarys, lbid)
        const rightBoundary = getBoundaryById(_boundarys, rbid)
        if (leftBoundary && rightBoundary) {
          // 通过当前车道左右边界线的采样点，初步判断是否存在过渡趋势
          const {
            isTransition,
            isExtends = undefined,
            normalWidth,
          } = judgeTrendByBoundarySamplePoints({
            leftPoints: leftBoundary.samplePoints,
            rightPoints: rightBoundary.samplePoints,
          })
          // 更新车道的属性
          _lane.normalWidth = normalWidth
          _lane.isTransition = isTransition
          if (isTransition) {
            _lane.isExtends = isExtends
          }
        }

        // TODO 如果 lane.normalWidth 异常，可以先缓存用于报错提示

        _lanes.push(_lane)
      }

      // 更新 section 的车道和边界线属性
      _section.lanes = _lanes
      _section.boundarys = _boundarys
      _sections.push(_section)
    }

    // 垂直于道路中心线首尾顶点的法向量
    const [verticalAtHead, verticalAtTail] =
      getCurveEndPointVerticalVec3(keyPath)

    const _road: biz.IRoad = {
      id: roadId,
      sections: _sections,
      length: calcRoadLength,
      keyPath,
      linkJunction: [], // 当前道路连接那一条道路需要在解析 laneLink 后才能确认
      type: roadType,
      verticalAtHead,
      verticalAtTail,
    }
    if (elevationPath) {
      _road.elevationPath = elevationPath
    }
    // 如果解析控制点为 arc 圆弧道路，则给道路赋值对应的属性
    if (isCircleRoad && circleOption) {
      _road.isCircleRoad = isCircleRoad
      _road.circleOption = circleOption
    }

    // 解析 openCrg 相关的配置
    if (road.opencrgs && road.opencrgs.length > 0) {
      const crgs: Array<biz.IOpenCrg> = road.opencrgs.map((option) => {
        const {
          file,
          orientation,
          mode,
          purpose,
          sOffset,
          tOffset,
          zOffset,
          zScale,
          hOffset,
        } = option
        // 将弧度制的偏航角，转换成角度制
        const _hOffset = String(
          fixedPrecision(
            // 将弧度制转换成角度制时，前端显示保留1位小数点
            getValidDegreeInRange(MathUtils.radToDeg(Number(hOffset))),
            1,
          ),
        )
        // 将crg文件路径解析成对应的文件名称
        const { isCloud } = globalConfig
        // 路径分隔符
        let sep = '\\'
        if (isCloud) {
          sep = '/'
        } else {
          // 单机版通过 electron 透传的分隔符来使用
          sep = get(window, 'electron.path.sep') || '\\'
        }
        const fileSegment = file.split(sep)
        const showName = fileSegment[fileSegment.length - 1]
        return {
          file,
          showName,
          orientation,
          mode,
          purpose,
          sOffset: String(fixedPrecision(Number(sOffset), 1)),
          tOffset: String(fixedPrecision(Number(tOffset), 1)),
          zOffset: String(fixedPrecision(Number(zOffset), 1)),
          zScale: String(fixedPrecision(Number(zScale), 1)),
          hOffset: _hOffset,
        }
      })
      _road.crgConfig = crgs
    }

    roadIds.push(_road.id)
    setRoad(_road.id, _road)

    resolve(_road.id)
  })
}

/**
 * 解析 road 层级的元素
 * @param roads
 * @returns
 */
async function parseRoadAndJunction (params: {
  roads: Array<biz.ICommonRoad>
  junctionIds: Array<string>
  lanelinks: Array<biz.ICommonLaneLink>
}) {
  const { roads, junctionIds, lanelinks } = params
  if (!roads || roads.length < 1) return

  const _roadIds: Array<string> = []
  const _controlPoints: Array<biz.IControlPoint> = []
  // 通过映射表先缓存所有的边界线对象，方便在解析边界线样式元素几何体属性后，找到对应的 boundary
  const boundaryMap: Map<string, biz.ILaneBoundary> = new Map()
  const laneMap: Map<string, biz.ILane> = new Map()

  console.time('parseAllRoad')
  await Promise.all(
    roads.map(road =>
      parseOneRoad({
        road,
        roadIds: _roadIds,
        controlPoints: _controlPoints,
        boundaryMap,
      }),
    ),
  )
  console.timeEnd('parseAllRoad')

  const computedLaneGeoAttr = async () => {
    const computedLaneGeoAttrPromise: Array<Promise<any>> = []
    for (const roadId of _roadIds) {
      const road = getRoad(roadId)
      if (!road) continue
      for (const section of road.sections) {
        const { lanes, boundarys } = section
        // 车道排序
        lanes.sort(sortById)
        for (const lane of lanes) {
          const { lbid, rbid, id: laneId } = lane
          // 通过车道边界线 id 找出对应车道边界线的数据
          const leftBoundary = boundarys.find(b => b.id === lbid)
          const rightBoundary = boundarys.find(b => b.id === rbid)
          if (!leftBoundary || !rightBoundary) continue
          if (
            leftBoundary.samplePoints.length < 2 ||
            rightBoundary.samplePoints.length < 2
          ) {
            continue
          }

          const laneFlag = `${roadId}_${section.id}_${laneId}`
          laneMap.set(laneFlag, lane)

          // 将采样点整理成扁平结构 [p1x, p1y, p1z, p2x, p2y, p2z, ...]
          const points1: Array<number> = []
          const points2: Array<number> = []
          for (const p of leftBoundary.samplePoints) {
            points1.push(p.x, p.y, p.z)
          }
          for (const p of rightBoundary.samplePoints) {
            points2.push(p.x, p.y, p.z)
          }

          const arrayBuffer1 = new Float32Array(points1)
          const arrayBuffer2 = new Float32Array(points2)
          const payload = {
            points1: arrayBuffer1.buffer,
            points2: arrayBuffer2.buffer,
            roadId,
            sectionId: section.id,
            laneId,
          }
          computedLaneGeoAttrPromise.push(
            pool.exec('getLaneGeoAttr', [payload], {
              transfer: [arrayBuffer1.buffer, arrayBuffer2.buffer],
            }),
          )
        }
      }
    }

    console.time('calcLaneGeoAttr')
    // 通过 webWorker 异步计算采样点的属性
    const computedLaneAttr = await Promise.all(computedLaneGeoAttrPromise)
    // 将计算得到的车道几何体属性，同步到对应的 lane 对象属性中
    for (const laneAttr of computedLaneAttr) {
      const { roadId, sectionId, laneId, vertices, indices, uvs } = laneAttr
      const laneFlag = `${roadId}_${sectionId}_${laneId}`

      if (!laneMap.has(laneFlag)) continue
      const lane = laneMap.get(laneFlag)
      if (!lane) continue
      lane.geoAttr = {
        vertices,
        indices,
        uvs,
      }
    }
    console.timeEnd('calcLaneGeoAttr')
  }

  const computedAllBoundaryAttr = async () => {
    // 遍历所有的road->section->boundary，通过 worker 线程来计算首次加载绘制的边界线几何体属性
    const computeAllBoundaryAttrPromise: Array<Promise<any>> = []
    for (const roadId of _roadIds) {
      const road = getRoad(roadId)
      if (!road) continue
      for (const section of road.sections) {
        for (const boundary of section.boundarys) {
          const { mark, samplePoints } = boundary
          if (samplePoints.length < 1) continue

          // 将采样点整理成扁平结构 [p1x, p1y, p1z, p2x, p2y, p2z, ...]
          const points: Array<number> = []
          for (const p of samplePoints) {
            points.push(p.x, p.y, p.z)
          }
          // 根据 mark 解析线形
          const { lineStyle } = parseLaneBoundaryMark(mark)
          // 无边界则直接返回
          if (lineStyle.length < 1) continue

          const isSingle = lineStyle.length === 1
          const width = Constant.markWidth
          // 针对单线和双线的边界线样式做处理
          lineStyle.forEach((style, index: number) => {
            const arrayBuffer = new Float32Array(points)
            const isFirstLine = index === 0
            const offset = isFirstLine ?
              Constant.markOffset :
                -Constant.markOffset
            if (style === 'solid') {
              // 将实线的顶点属性计算逻辑推入 worker 线程
              const payload = {
                width,
                isSingle,
                offset,
                points: arrayBuffer.buffer,
                isFirst: isFirstLine,
                roadId,
                sectionId: section.id,
                boundaryId: boundary.id,
              }
              computeAllBoundaryAttrPromise.push(
                pool.exec('getSolidBoundaryGeoAttr', [payload], {
                  transfer: [arrayBuffer.buffer],
                }),
              )
            } else if (style === 'dash') {
              // 将虚线的顶点属性计算逻辑推入 worker 线程
              const payload = {
                width,
                isSingle,
                offset,
                points: arrayBuffer.buffer,
                unitLength: Constant.markUnitLength,
                gapLength: Constant.markGapLength,
                isFirst: isFirstLine,
                roadId,
                sectionId: section.id,
                boundaryId: boundary.id,
              }
              computeAllBoundaryAttrPromise.push(
                pool.exec('getDashedBoundaryGeoAttr', [payload], {
                  transfer: [arrayBuffer.buffer],
                }),
              )
            }
          })
        }
      }
    }

    console.time('calcBoundaryGeoAttr')
    // 通过 webWorker 异步计算采样点的属性
    const computedBoundaryAttr = await Promise.all(
      computeAllBoundaryAttrPromise,
    )
    // 将计算得到的边界线采样点属性，同步到对应的 boundary 对象属性中
    for (const boundaryAttr of computedBoundaryAttr) {
      const { boundaryId, vertices, indices, isFirst } = boundaryAttr
      if (!boundaryMap.has(boundaryId)) continue
      const boundary = boundaryMap.get(boundaryId)
      if (!boundary) continue
      if (isFirst) {
        boundary.firstLineAttr = {
          vertices,
          indices,
        }
      } else {
        boundary.secondLineAttr = {
          vertices,
          indices,
        }
      }
    }
    console.timeEnd('calcBoundaryGeoAttr')
  }

  await Promise.all([
    // 解析路口跟计算车道边界线几何体属性同步执行，应该不冲突
    parseJunctionAndLaneLink({
      junctionIds,
      lanelinks,
    }),
    computedLaneGeoAttr(),
    computedAllBoundaryAttr(),
  ])

  // 清空映射表
  boundaryMap.clear()
  laneMap.clear()

  // 先临时缓存，暂不触发 store 的方法
  tempRoadIds = _roadIds
  tempControlPoints = _controlPoints
}

/**
 * 将重新计算 junction 结构属性的逻辑抽离成异步 promise
 * @param params
 */
async function updateJunctionStructure (params: {
  junction: biz.IJunction
  junctionIds: Array<string>
}) {
  const { junction, junctionIds } = params
  return new Promise((resolve) => {
    const junctionStore = useJunctionStore()

    // 更新 junction
    junctionStore
      .updateJunction({
        junction,
        // 基于解析的地图文件更新 junction 结构，需要提供 isInit 参数
        isInit: true,
      })
      .then((data) => {
        junctionIds.push(junction.id)

        // 在计算完路口所有车道的连接关系后，将连接关系同步到 road 中，更新 road.linkJunction 属性
        for (const linkRoad of junction.linkRoads) {
          // 获取当前路口连接的道路 id
          const [roadId] = linkRoad.split('_')
          const road = getRoad(roadId)
          if (road) {
            road.linkJunction.push(junction.id)
          }
        }
        // TODO 缓存路口的对象属性
        setJunction(junction.id, junction)

        resolve(junction.id)
      })
  })
}

/**
 * 处理 junction 层级的元素
 * @param junctions
 */
async function parseJunctionAndLaneLink (params: {
  lanelinks: Array<biz.ICommonLaneLink>
  junctionIds: Array<string>
}) {
  const { lanelinks, junctionIds } = params
  if (lanelinks.length < 1 || junctionIds.length < 1) return

  // 先用一个映射表保存已经预处理的交叉路口数据
  const _junctionMap = new Map()
  console.time('=parse junction=')
  // junction 目前没有太多的属性，暂时可以忽略
  for (const junctionId of junctionIds) {
    const _junction: biz.IJunction = {
      id: junctionId,
      // 以上属性需要通过 laneLinks 的连通关系计算而来
      laneLinks: [],
      linkRoads: [],
      refRoads: [],
      geoAttr: null,
    }
    _junctionMap.set(junctionId, _junction)
  }

  // 解析车道连接线的数据
  for (const laneLink of lanelinks) {
    const {
      junctionid,
      samplePoints = [],
      frid,
      ftype,
      trid,
      ttype,
      controlPoints = [],
      controlType = 'catmullrom',
    } = laneLink
    const junction = _junctionMap.get(junctionid)
    if (!junction) continue

    // 在加载地图文件时，使用文件中获取到的laneLink采样点
    // 将车道连接线采样点坐标转换
    const _samplePoints = transformVec3ByObject(
      transformCoordinateToThree(samplePoints),
    )
    const _laneLink: biz.ILaneLink = {
      samplePoints: _samplePoints,
      enabled: true, // 从文件加载的车道连接线均为有效可用的
      roadid: laneLink.roadid || genRoadId(), // 如果有 lanelink 绑定的 roadid，则使用，否则新建
      controlPoints, // 直接使用透传的控制点数据
      ...pick(laneLink, [
        'id',
        'fid',
        'frid',
        'fsid',
        'ftype',
        'tid',
        'trid',
        'tsid',
        'ttype',
        'length',
      ]),
    }
    // 如果有控制点数据，则更新对应的控制点线形
    if (_laneLink.controlPoints.length > 0 && controlType) {
      _laneLink.controlType = controlType
    }

    // 解析所有车道连接关系，计算对应的道路连接关系
    const isFromDirection = Number(laneLink.fid) < 0 ? 'forward' : 'reverse'
    const isToDirection = Number(laneLink.tid) < 0 ? 'forward' : 'reverse'
    const fromRoadPercent = ftype === 'start' ? 0 : 1
    const toRoadPercent = ttype === 'start' ? 0 : 1
    const _linkFromRoad = `${frid}_${fromRoadPercent}_${isFromDirection}`
    const _linkToRoad = `${trid}_${toRoadPercent}_${isToDirection}`
    // 判断当前 junction 中的 linkRoads 数组中是否有对应的连通道路，如果没有就保存
    if (!junction.linkRoads.includes(_linkFromRoad)) {
      junction.linkRoads.push(_linkFromRoad)
    }
    if (!junction.linkRoads.includes(_linkToRoad)) {
      junction.linkRoads.push(_linkToRoad)
    }

    // 将当前解析的车道连接线添加到 junction 的数据结构中
    junction.laneLinks.push(_laneLink)
  }
  console.timeEnd('=parse junction=')

  console.time('calcJunctionAttr')
  const _junctionIds: Array<string> = []
  const updateAllJunctions = []
  // 再次遍历所有的交叉路口数据结构，更新当前交叉路口的闭合区域边界线采样点
  for (const _junction of _junctionMap.values()) {
    updateAllJunctions.push(
      updateJunctionStructure({
        junction: _junction,
        junctionIds: _junctionIds,
      }),
    )
  }
  await Promise.all(updateAllJunctions)
  console.timeEnd('calcJunctionAttr')

  // 手动清除缓存
  _junctionMap.clear()

  tempJunctionIds = _junctionIds
}

// 基于圆弧道路的2个控制点，计算圆弧道路的基本属性和参考线采样点
function parseArcControlPoint (
  p1: biz.ICommonControlPoint,
  p2: biz.ICommonControlPoint,
) {
  // 控制点切线的角度 = 控制点跟圆心连线的向量的角度 + 90°
  // 圆弧起始和截止的夹角（控制点跟圆心连线的向量的角度）
  const hdg1 = getValidRadInRange(Number(p1.hdg))
  const hdg2 = getValidRadInRange(Number(p2.hdg))
  const startPoint = new Vector2(p1.z, p1.x)
  const endPoint = new Vector2(p2.z, p2.x)
  const startDirection = new Vector2(Math.cos(hdg1), Math.sin(hdg1))
  // 从截止点到起始点的向量
  const endToStart = startPoint.clone().sub(endPoint).normalize()
  // 计算从截止点到起始点的向量，朝向是在起始方向的那一侧
  const crossRes = startDirection.cross(endToStart)
  let isClockwise = false
  if (crossRes > 0) {
    isClockwise = true
  }

  let startAngle = 0
  let endAngle = 0
  if (isClockwise) {
    startAngle = (hdg1 + halfPI) % (PI * 2)
    endAngle = (hdg2 + halfPI) % (PI * 2)
    // 如果是顺时针创建圆弧，则起始的角度要比截至角度大
    if (startAngle < endAngle) {
      startAngle += PI * 2
    }
  } else {
    startAngle = (hdg1 - halfPI) % (PI * 2)
    endAngle = (hdg2 - halfPI) % (PI * 2)
    // 逆时针创建圆弧，起始角度比截至角度小
    if (endAngle < startAngle) {
      endAngle += PI * 2
    }
  }
  // 圆弧区域的角度
  const deltaAngle = Math.abs(endAngle - startAngle)
  const dist = endPoint.distanceTo(startPoint)

  // 如果距离过于近的异常情况处理
  if (fixedPrecision(dist) === 0) return null

  // 圆弧的半径
  const radius = dist / 2 / Math.sin(deltaAngle / 2)

  // 基于起始点的 hdg 航向角，计算指向圆弧圆心的方向向量（顺逆时针不同，指向圆心的方向也不同）
  const toCenterDirection = isClockwise ?
    new Vector3(-Math.cos(hdg1), (p1.y + p2.y) / 2, Math.sin(hdg1)) :
    new Vector3(Math.cos(hdg1), (p1.y + p2.y) / 2, -Math.sin(hdg1))
  // 基于起始点计算圆心的坐标
  const center = new Vector3(p1.x, p1.y, p1.z).addScaledVector(
    toCenterDirection,
    radius,
  )

  // 2 度对应的弧度，即每隔2度取一个采样点
  const unit = PI / 90
  // 基于圆弧相关的属性，计算道路参考线上的采样点坐标
  const points = createArcPoints({
    center,
    radius,
    startAngle,
    endAngle,
    unit,
    isClockwise,
  })

  // 如果道路参考线采样点的数量过少
  if (points.length < 2) return null

  return {
    radius,
    startAngle,
    endAngle,
    center: {
      x: center.x,
      y: center.y,
      z: center.z,
    },
    points,
    isClockwise,
  }
}
