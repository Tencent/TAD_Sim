import workerpool from 'workerpool'
import { difference } from 'lodash'
import {
  Constant,
  geJunctionGeoByTwoRoad,
  getBezierEdgeSamplePoints,
  getCornerVerticesAttr,
  getGeometryAttrBySamplePoints,
  getJunctionGeoByMultipleRoad,
  getLocationInJunction,
  handleDashedBoundaryData,
  handleSolidBoundaryData,
  sortJunctionRefBoundary,
} from '@/utils/business'
import {
  addCatmullromVirtualPoint,
  fixedPrecision,
  getBezier3WithDirection,
  getWorldAngle,
  transformOneCoordinateToCommon,
  transformVec3ByFlatArray,
} from '@/utils/common3d'
import { getLaneLinkUniqueFlag } from '@/stores/junction/link'

// worker 线程对外暴露的方法
const exposeFunctions = {
  init: () => {},
  // 获取一条【实线】车道边界线的几何体属性
  getSolidBoundaryGeoAttr: (payload: {
    width: number
    offset: number
    isSingle: boolean
    points: ArrayBuffer
    isFirst: boolean
    roadId: string
    sectionId: string
    boundaryId: string
  }) => {
    const { roadId, sectionId, boundaryId, isFirst } = payload
    const { vertices, indices } = handleSolidBoundaryData(payload)
    const res = {
      vertices,
      indices,
      roadId,
      sectionId,
      boundaryId,
      isFirst,
    }
    // 将返回的数组通过 Transferable 回传引用，提升性能
    return new workerpool.Transfer(res, [vertices.buffer, indices.buffer])
  },
  // 获取一条【虚线】车道边界线的几何体属性
  getDashedBoundaryGeoAttr: (payload: {
    width: number
    offset: number
    isSingle: boolean
    points: Array<common.vec3> | ArrayBuffer
    unitLength: number
    gapLength: number
    isFirst: boolean
    roadId: string
    sectionId: string
    boundaryId: string
  }) => {
    const { roadId, sectionId, boundaryId, isFirst } = payload
    const { vertices, indices } = handleDashedBoundaryData(payload)
    const res = {
      vertices,
      indices,
      roadId,
      sectionId,
      boundaryId,
      isFirst,
    }
    return new workerpool.Transfer(res, [vertices.buffer, indices.buffer])
  },
  // 获取路口 laneLinks 属性
  getJunctionLaneLinks: (payload: {
    originLaneLinks: Array<biz.ILaneLink> // 原有的 laneLinks 关系
    laneInfos: Array<biz.ILaneLinkInfo>
  }) => {
    const { originLaneLinks, laneInfos } = payload
    // 通过映射表来存储所有车道端点处的数据，便于查找
    const laneInfoMap: Map<string, biz.ILaneLinkInfo> = new Map()
    // 驶入路口的车道
    const fromLanes = []
    // 驶出路口的车道
    const toLanes = []
    // 重新计算后的 laneLinks 集合
    const newLaneLinks: Array<biz.ILaneLink> = []
    // 原有的 laneLink 连接的道路
    const originLinkRoads: Array<string> = []
    // 重新计算后的 laneLink 连接的道路
    const newLinkRoads: Array<string> = []
    for (const _laneInfo of laneInfos) {
      // 区分正反向车道
      const { isTail, direction } = _laneInfo
      if (isTail && direction === 'forward') {
        fromLanes.push(_laneInfo)
      } else if (isTail && direction === 'reverse') {
        toLanes.push(_laneInfo)
      } else if (!isTail && direction === 'forward') {
        toLanes.push(_laneInfo)
      } else if (!isTail && direction === 'reverse') {
        fromLanes.push(_laneInfo)
      }
      laneInfoMap.set(_laneInfo.flag, _laneInfo)
    }

    // 将驶入和驶出路口的车道，按照 laneId 匹配的关系做默认的连接【该情况会默认来连接掉头车道】
    for (const fromLane of fromLanes) {
      const [fromRoadId, fromSectionId, fromLaneId, fromType, fromDirection] =
        fromLane.flag.split('_')

      const fromRoadFlag = `${fromRoadId}_${fromType}_${fromDirection}`
      if (!newLinkRoads.includes(fromRoadFlag)) {
        // 记录新的连接线连接的道路
        newLinkRoads.push(fromRoadFlag)
      }
      for (const toLane of toLanes) {
        const [toRoadId, toSectionId, toLaneId, toType, toDirection] =
          toLane.flag.split('_')
        const toRoadFlag = `${toRoadId}_${toType}_${toDirection}`
        if (!newLinkRoads.includes(toRoadFlag)) {
          // 记录新的连接线连接的道路
          newLinkRoads.push(toRoadFlag)
        }

        // 生成新的 laneLinks，计算全量的情况
        newLaneLinks.push({
          fid: fromLaneId,
          frid: fromRoadId,
          fsid: fromSectionId,
          ftype: fromType,
          tid: toLaneId,
          trid: toRoadId,
          tsid: toSectionId,
          ttype: toType,
          // id 暂时置空，后续需要跟原 laneLink 进行对比再判断是否赋新 id
          id: '',
          length: 0,
          samplePoints: [],
          roadid: '', // 默认绑定的 roadid 为空
          enabled: false,
          controlPoints: [], // 默认贝塞尔曲线的控制点为空
        })
      }
    }

    // 根据实际可行的 laneLink 连接关系，更新对应的路径相关数据
    for (const laneLink of newLaneLinks) {
      const { fid, frid, fsid, ftype, tid, trid, tsid, ttype } = laneLink
      const fDirection = Number(fid) < 0 ? 'forward' : 'reverse'
      const tDirection = Number(tid) < 0 ? 'forward' : 'reverse'
      const fromFlag = getLaneLinkUniqueFlag(frid, fsid, fid, ftype, fDirection)
      const toFlag = getLaneLinkUniqueFlag(trid, tsid, tid, ttype, tDirection)
      // 由于映射关系在上一轮迭代中添加的数据，所以当前迭代肯定存在
      const fromLaneInfo = laneInfoMap.get(fromFlag)
      const toLaneInfo = laneInfoMap.get(toFlag)
      if (!fromLaneInfo || !toLaneInfo) continue
      const { laneEndPoint: fromPoint, tangent: fromDirection } = fromLaneInfo
      const { laneEndPoint: toPoint, tangent: toDirection } = toLaneInfo

      // 通过两条车道的车道中心线的采样点，以及车道方向，计算对应的贝塞尔曲线
      const {
        curve: laneLinkCurve,
        cp1,
        cp2,
      } = getBezier3WithDirection({
        point1: fromPoint,
        direction1: fromDirection,
        direction2: toDirection,
        point2: toPoint,
      })

      // 车道连接线的长度
      const length = fixedPrecision(laneLinkCurve.getLength())
      // 车道连接线的采样点
      const samplePoints = laneLinkCurve.getSpacedPoints(
        Constant.laneLinkSegment,
      )
      // 更新当前 laneLink 的数据
      laneLink.length = length
      laneLink.samplePoints = samplePoints

      // 将 laneLink 贝塞尔曲线的控制点缓存起来
      // 同时设置曲线类型和控制点
      // laneLink.controlType = 'bezier'
      // const controlPoints: Array<biz.ICommonControlPoint> = []
      // // 曲线上第一个点
      // controlPoints.push(getPointInfoOnCurve(laneLinkCurve, 0))

      // // 贝塞尔曲线的2个控制点
      // const _cp1 = transformOneCoordinateToCommon(cp1)
      // const _cp2 = transformOneCoordinateToCommon(cp2)
      // controlPoints.push(
      //   {
      //     ..._cp1,
      //     hdg: 'null',
      //   },
      //   {
      //     ..._cp2,
      //     hdg: 'null',
      //   }
      // )

      // // 曲线上的最后一个点
      // controlPoints.push(getPointInfoOnCurve(laneLinkCurve, 1))
      // laneLink.controlPoints = controlPoints

      // 由于后端咱们还没弄好 bezier 曲线的控制点解析，暂时用 catmullrom 类型控制点
      laneLink.controlType = 'catmullrom'
      // 获取 catmullrom 类型曲线的采样比例
      const percent = [0, 0.15, 0.5, 0.85, 1]
      const controlPoints: Array<biz.ICommonControlPoint> = []
      percent.forEach((_percent) => {
        const point = laneLinkCurve.getPointAt(_percent)
        const tangent = laneLinkCurve.getTangentAt(_percent)
        // 获取切线方向的世界坐标航向角
        const hdg = getWorldAngle(tangent)
        // 将控制点转换成平面坐标下的坐标
        const _point = transformOneCoordinateToCommon(point)
        controlPoints.push({
          ..._point,
          hdg: String(hdg),
        })
      })

      // 补充 catmullrom 线形道路的首尾2个虚拟控制点
      addCatmullromVirtualPoint(controlPoints)

      laneLink.controlPoints = controlPoints
    }

    // 缓存原来有效的 laneLink
    interface ILaneLinkFrag {
      id: string
      roadid: string
    }
    const laneLinkFlagMap: Map<string, ILaneLinkFrag> = new Map()
    // 将原 laneLinks 中的车道连接映射关系做缓存
    for (const originLaneLink of originLaneLinks) {
      const {
        id: originId,
        fid,
        frid,
        fsid,
        ftype,
        tid,
        trid,
        tsid,
        ttype,
        roadid = '',
      } = originLaneLink
      // 记录原有的 laneLink 连接的道路 id
      // notes: 考虑到环形道路是同一条道路的首端和尾端可能都会连接入路口，需要加上道路对应的首尾标识
      const fDirection = Number(fid) < 0 ? 'forward' : 'reverse'
      const tDirection = Number(tid) < 0 ? 'forward' : 'reverse'

      const fromRoadFlag = `${frid}_${ftype}_${fDirection}`
      const toRoadFlag = `${trid}_${ttype}_${tDirection}`
      if (!originLinkRoads.includes(fromRoadFlag)) {
        originLinkRoads.push(fromRoadFlag)
      }
      if (!originLinkRoads.includes(toRoadFlag)) {
        originLinkRoads.push(toRoadFlag)
      }

      // 将驶入和驶出车道的信息作为唯一标识
      const fromFlag = getLaneLinkUniqueFlag(frid, fsid, fid, ftype, fDirection)
      const toFlag = getLaneLinkUniqueFlag(trid, tsid, tid, ttype, tDirection)
      const flag = `${fromFlag}_${toFlag}`
      if (!laneLinkFlagMap.has(flag)) {
        // 将原有的 laneLink 的 id 缓存
        laneLinkFlagMap.set(flag, {
          id: originId,
          roadid,
        })
      }
    }

    // 获取 newLinkRoads 中有，但 originLinkRoads 中没有的 roadId
    // 即新增连接的道路
    const addedRoadFlag = difference(newLinkRoads, originLinkRoads)

    // 将新计算的 newLaneLinks 跟旧的 originLaneLinks 做 diff
    for (const newLaneLink of newLaneLinks) {
      const { fid, frid, fsid, ftype, tid, trid, tsid, ttype } = newLaneLink
      // 将驶入和驶出车道的信息作为唯一标识
      const fDirection = Number(fid) < 0 ? 'forward' : 'reverse'
      const tDirection = Number(tid) < 0 ? 'forward' : 'reverse'

      const fromFlag = getLaneLinkUniqueFlag(frid, fsid, fid, ftype, fDirection)
      const toFlag = getLaneLinkUniqueFlag(trid, tsid, tid, ttype, tDirection)
      const flag = `${fromFlag}_${toFlag}`

      const sameLaneLink = laneLinkFlagMap.get(flag)
      if (sameLaneLink) {
        const { id: originId, roadid } = sameLaneLink
        newLaneLink.id = originId
        newLaneLink.enabled = true
        // 如果是新的lanelink 跟旧的 lanelink 是同一条，则使用同样的 roadid
        newLaneLink.roadid = roadid
      } else {
        // 如果没有找到匹配的车道连接关系，说明原有的 laneLink 中不存在，属于新增的
        // 只有新增的连接道路，才会使用新计算出来的 laneLink
        const fromRoadFlag = `${frid}_${ftype}_${fDirection}`
        const toRoadFlag = `${trid}_${ttype}_${tDirection}`
        if (
          addedRoadFlag.includes(fromRoadFlag) ||
          addedRoadFlag.includes(toRoadFlag)
        ) {
          // 新增的 laneLinks，默认只添加 laneId 一致的【其他的 laneLink 通过连接线编辑模式去更新】
          // 由于支持了正反向车道，根据 laneId 的绝对值来判定默认的链接
          if (Math.abs(Number(fid)) === Math.abs(Number(tid))) {
            // 跟后端沟通后，正反车道的双向道路，避免出现默认的掉头车道（几乎不用）
            if (
              frid === trid &&
              fsid === tsid &&
              ftype === ttype &&
              fDirection !== tDirection
            ) {
              newLaneLink.enabled = false
            } else {
              newLaneLink.enabled = true
            }
            // id 在 worker 外部获取，否则不能保证 genlaneLinkId 生成的 ID 正确
            // 使用新建的 id
            // newLaneLink.id = genLaneLinkId()
          }
        }
      }
    }

    // 筛选有效的 laneLink
    const sortedLaneLink = newLaneLinks.filter(link => link.enabled)

    // 手动清空 Map 缓存
    laneLinkFlagMap.clear()

    return sortedLaneLink
  },
  // 获取路口的几何体属性
  getJunctionGeoAttr: (payload: {
    refBoundary: Array<biz.IRefRoad>
    enableClosePoint: boolean
    isMultipleRoad: boolean
    junctionId?: string
  }) => {
    const {
      refBoundary,
      enableClosePoint,
      isMultipleRoad,
      junctionId = '',
    } = payload
    const sortedRefBoundary = sortJunctionRefBoundary(refBoundary)
    // 获取当前交叉路口的闭合区域边界线的采样点
    const samplePoints = getBezierEdgeSamplePoints(
      sortedRefBoundary,
      enableClosePoint,
    )
    if (isMultipleRoad) {
      const { vertices, indices } = getJunctionGeoByMultipleRoad(samplePoints)
      return new workerpool.Transfer(
        {
          vertices,
          indices,
          junctionId,
        },
        [vertices.buffer, indices.buffer],
      )
    } else {
      const { vertices, indices } = geJunctionGeoByTwoRoad(samplePoints)
      return new workerpool.Transfer(
        {
          vertices,
          indices,
          junctionId,
        },
        [vertices.buffer, indices.buffer],
      )
    }
  },
  // 获取车道的几何体属性
  getLaneGeoAttr: (payload: {
    roadId: string
    sectionId: string
    laneId: string
    points1: ArrayBuffer
    points2: ArrayBuffer
  }) => {
    const { points1, points2, roadId, sectionId, laneId } = payload
    const pointsArr1 = Array.prototype.slice.call(new Float32Array(points1))
    const _points1 = transformVec3ByFlatArray(pointsArr1)
    const pointsArr2 = Array.prototype.slice.call(new Float32Array(points2))
    const _points2 = transformVec3ByFlatArray(pointsArr2)
    const { vertices, indices, uvs } = getGeometryAttrBySamplePoints(
      _points1,
      _points2,
    )
    const res = {
      vertices,
      indices,
      uvs,
      roadId,
      sectionId,
      laneId,
    }
    return new workerpool.Transfer(res, [
      vertices.buffer,
      indices.buffer,
      uvs.buffer,
    ])
  },
  // 求路口中的绝对坐标，在路口中所有 laneLink 参考线坐标系下的最优 st 坐标
  getLocationInJunction: (payload: {
    laneLinks: Array<biz.ILaneLink>
    point: common.vec3
    originLaneLinkId?: string // 是否有指定的 laneLinkId
  }) => {
    const res = getLocationInJunction(payload)
    return res
  },
  // 获取停车位四个角坐标的几何体顶点属性
  getCornerVerticesAttr: (payload: {
    curvePathPoints: Array<common.vec3>
    elevationPathPoints?: Array<common.vec3>
    refLineLocation: biz.IRefLineLocation
    count: number // 重复次数
    width: number // 停车位宽度
    length: number // 停车位长度
    margin: number // 相邻停车位几何中心点间距
    lineWidth: number // 线宽
    yaw: number // 停车位跟参考线的偏航角（角度制）
    innerAngle: number // 停车位内部的夹角（角度制），暂时理解成左下的夹角，在 [45, 135] 度范围中
    isDashed?: boolean // 虚线的限时停车位
  }) => {
    const res = getCornerVerticesAttr(payload)
    return res
  },
}

// 创建一个线程池，并且注册对外暴露的方法
workerpool.worker(exposeFunctions)
