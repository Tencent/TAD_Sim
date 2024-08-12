// store 中部分属性的默认模板
import { CatmullRomCurve3 } from 'three'
import { createLaneBoundary } from './boundary'
import { createLane, getLaneBoundaryOnBothSides } from './lane'
import { createSection } from './section'
import {
  CatmullromTension,
  CurveType,
  fixedPrecision,
  getCurveEndPointVerticalVec3,
  sortById,
  transformVec3,
  transformVec3ByObject,
} from '@/utils/common3d'
import {
  Constant,
  getDashedBoundaryGeoAttrPromise,
  getLaneGeoAttrPromise,
  getSolidBoundaryGeoAttrPromise,
  parseLaneBoundaryMark,
} from '@/utils/business'

/**
 * 获取道路首/尾某一端的最内侧和最外侧的车道边界线端点
 * @param params
 */
export function getRoadEndPoints (params: {
  road: biz.IRoad
  isTail: boolean
  direction?: biz.ILaneDirection
}) {
  const { road, isTail, direction = '' } = params
  const { sections } = road
  // 判断是第一个还是最后一个 section
  const section = isTail ? sections[sections.length - 1] : sections[0]
  // 获取当前 section 的最内侧和最外侧的端点
  let leftBoundary, rightBoundary
  if (direction) {
    const { left: _leftBoundary, right: _rightBoundary } =
      getLaneBoundaryOnBothSides(section, direction)
    leftBoundary = _leftBoundary
    rightBoundary = _rightBoundary
  } else {
    const { left: _leftBoundary, right: _rightBoundary } =
      getLaneBoundaryOnBothSides(section)
    leftBoundary = _leftBoundary
    rightBoundary = _rightBoundary
  }

  if (!leftBoundary || !rightBoundary) {
    return {
      left: null,
      right: null,
    }
  }

  // 根据首尾的情况，获取具体的左右车道边界线的采样点
  // 由于第三方地图中，左右车道边界线的采样点数量不一定一致，需要单独使用自身的长度
  const leftLength = leftBoundary.samplePoints.length
  const rightLength = rightBoundary.samplePoints.length
  const leftPoint = isTail ?
    leftBoundary.samplePoints[leftLength - 1] :
    leftBoundary.samplePoints[0]
  const rightPoint = isTail ?
    rightBoundary.samplePoints[rightLength - 1] :
    rightBoundary.samplePoints[0]
  // 将点转换成三维场景中的顶点
  const _leftPoint = transformVec3(leftPoint)
  const _rightPoint = transformVec3(rightPoint)

  return {
    left: _leftPoint,
    right: _rightPoint,
  }
}

/**
 * 创建一条默认的道路
 * @param params
 * @returns
 */
export function createRoadData (params: {
  roadId: string
  keyPath: biz.ICurve3
  laneWidth: number
  laneNumber: number
  roadType?: string
}): biz.IRoad {
  const {
    roadId,
    keyPath,
    laneNumber,
    laneWidth,
    roadType = 'default',
  } = params
  const roadLength = fixedPrecision(keyPath.getLength())
  // 创建 section
  const sections = [
    createSection({
      roadLength,
    }),
  ]

  // 是否是双向道路
  let isTwoWay = true
  let _laneNumber = laneNumber
  // 如果是创建匝道，则为单向单车道
  if (roadType === 'ramp') {
    _laneNumber = 1
    isTwoWay = false
  }

  // 高程默认控制点
  const pHead = { x: 0, y: 0, z: 0 }
  const pTail = { x: roadLength, y: 0, z: 0 }
  const elevationPoints = [pHead, pTail]

  // 获取高程曲线的路径
  const elevationPath = new CatmullRomCurve3(
    transformVec3ByObject(elevationPoints),
  )
  elevationPath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    elevationPath.tension = CatmullromTension
  }

  for (const section of sections) {
    // 创建当前道路的车道边界线属性
    const boundary = createLaneBoundary({
      keyPath,
      elevationPath,
      laneWidth,
      laneNumber: _laneNumber,
      pStart: section.pStart,
      pEnd: section.pEnd,
      roadType,
      isTwoWay,
    })
    // 创建当前道路的车道属性
    const lanes = createLane({
      width: laneWidth,
      boundary,
      roadType,
      isTwoWay,
    })

    section.boundarys = boundary
    section.lanes = lanes
  }

  // 计算车道中心线曲线的首尾垂直向量
  const [verticalAtHead, verticalAtTail] = getCurveEndPointVerticalVec3(keyPath)

  const road: biz.IRoad = {
    id: roadId,
    sections,
    elevationPath,
    length: roadLength,
    keyPath,
    linkJunction: [],
    verticalAtHead,
    verticalAtTail,
    // TODO 先使用默认的一个值，后续有映射关系再对应补充
    type: 0,
  }

  // 如果是创建的隧道，需要更新道路的 roadType
  if (roadType === 'tunnel') {
    // 通过变量的引用，来调整为隧道类型挂载属性
    const _road = road as biz.ITunnel
    _road.roadType = 'tunnel'
    _road.tunnelS = 0
    _road.tunnelLength = roadLength
    return _road
  }

  return road
}

/**
 * 更新某一条道路所有的车道和车道线几何体的属性
 * @param road
 */
export async function updateRoadAllGeoAttr (
  road: biz.IRoad,
  options?: {
    ignoreLane?: boolean
    ignoreBoundary?: boolean
  },
) {
  const { id: roadId, sections } = road
  const { ignoreLane = false, ignoreBoundary = false } = options || {}
  // 基于当前创建的 lane 更新对应的 laneGeoAttr
  const laneMap: Map<string, biz.ILane> = new Map()
  const boundaryMap: Map<string, biz.ILaneBoundary> = new Map()

  // 【TODO】重要：如果道路的结构比较复杂，考虑借助 worker 线程辅助计算！！
  // 先通过主线程的方法来计算某一条道路的车道和车道边界线的几何体属性
  // 区别于第一次打开地图文件，通过 webworker 线程来计算的数据结构
  const computeLaneGeoAttr: Array<Promise<any>> = []
  const computeBoundaryGeoAttr: Array<Promise<any>> = []
  for (const section of sections) {
    const { lanes, boundarys } = section
    lanes.sort(sortById)
    // 判断是否忽略车道结构的更新
    if (!ignoreLane) {
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

        computeLaneGeoAttr.push(
          getLaneGeoAttrPromise({
            roadId,
            sectionId: section.id,
            laneId,
            points1: leftBoundary.samplePoints,
            points2: rightBoundary.samplePoints,
          }),
        )
      }
    }

    // 判断是否忽略边界线结构的更新
    if (!ignoreBoundary) {
      for (const boundary of boundarys) {
        const { id: boundaryId, mark, samplePoints } = boundary
        if (samplePoints.length < 1) continue

        // 根据 mark 解析线形
        const { lineStyle } = parseLaneBoundaryMark(mark)
        // 无边界则直接返回
        if (lineStyle.length < 1) continue

        const isSingle = lineStyle.length === 1
        const width = Constant.markWidth

        boundaryMap.set(boundaryId, boundary)
        // 针对单线和双线的边界线样式做处理
        lineStyle.forEach((style, index: number) => {
          const isFirstLine = index === 0
          const offset = isFirstLine ?
            Constant.markOffset :
              -Constant.markOffset

          if (style === 'solid') {
            const payload = {
              width,
              isSingle,
              offset,
              points: samplePoints,
              isFirst: isFirstLine,
              roadId,
              sectionId: section.id,
              boundaryId,
            }
            computeBoundaryGeoAttr.push(getSolidBoundaryGeoAttrPromise(payload))
          } else if (style === 'dash') {
            const payload = {
              width,
              isSingle,
              offset,
              points: samplePoints,
              unitLength: Constant.markUnitLength,
              gapLength: Constant.markGapLength,
              isFirst: isFirstLine,
              roadId,
              sectionId: section.id,
              boundaryId,
            }
            computeBoundaryGeoAttr.push(
              getDashedBoundaryGeoAttrPromise(payload),
            )
          }
        })
      }
    }
  }

  const computedLaneGeoAttr = await Promise.all(computeLaneGeoAttr)
  // 将计算得到的车道几何体属性，同步到对应的 lane 对象属性中
  for (const laneGeoAttr of computedLaneGeoAttr) {
    const { roadId, sectionId, laneId, vertices, indices, uvs } = laneGeoAttr
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

  const computedBoundaryAttr = await Promise.all(computeBoundaryGeoAttr)
  // 已经处理过的边界线 id
  const tempBoundaryId: Array<string> = []
  // 将计算得到的边界线采样点属性，同步到对应的 boundary 对象属性中
  for (const boundaryAttr of computedBoundaryAttr) {
    const { boundaryId, vertices, indices, isFirst } = boundaryAttr
    if (!boundaryMap.has(boundaryId)) continue
    const boundary = boundaryMap.get(boundaryId)
    if (!boundary) continue
    if (!tempBoundaryId.includes(boundaryId)) {
      // 如果是第一次遍历到的边界线 id，则需要清空原有的 boundary 属性
      boundary.firstLineAttr = null
      boundary.secondLineAttr = null
      tempBoundaryId.push(boundaryId)
    }
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
  // 手动清空数据
  tempBoundaryId.length = 0
}
