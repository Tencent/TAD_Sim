import type { Vector3 } from 'three'
import { updateLaneCenterPoints } from './lane'
import {
  Constant,
  DefaultLaneBoundaryMark,
  getLeftBoundarySamplePointByRight,
  getMatchingSegment,
  getParallelCurveSamplePoints,
  getRightBoundarySamplePointByLeft,
  getSegmentByLength,
  getTransitionBoundarySamplePoints,
} from '@/utils/business'
import {
  getSamplePointByOffset,
  sortByAbsId,
} from '@/utils/common3d'
import { genLaneBoundaryId } from '@/utils/guid'

interface ILaneBoundaryParams {
  keyPath: biz.ICurve3
  elevationPath?: biz.ICurve3
  pStart: number
  pEnd: number
  laneWidth: number
  laneNumber: number
  roadType: string
  isTwoWay: boolean
}

/**
 * 通过 id 从 boundary 的数组中获取对应的边界线
 * @param boundary
 * @param id
 * @returns
 */
export function getBoundaryById (
  boundary: Array<biz.ILaneBoundary>,
  id: string,
) {
  if (!boundary || boundary.length < 1) return null
  const _boundary = boundary.find(b => b.id === id)
  return _boundary
}

/**
 * 将某一个 section 中的边界线 id 重新赋值，同时调整车道绑定的左右边界线 id
 * @param section
 */
export function updateBoundaryIdInSection (section: biz.ISection) {
  const { lanes, boundarys } = section
  // 更新当前 section 中所有边界线 id，车道关联的边界线也需要同步修改
  const boundaryIdMap = new Map()
  for (const boundary of boundarys) {
    const { id: oldId } = boundary
    const newId = genLaneBoundaryId()
    // 当前 section 边界线新旧 id 映射
    boundaryIdMap.set(oldId, newId)
    boundary.id = newId
  }
  for (const lane of lanes) {
    const { rbid, lbid } = lane
    if (boundaryIdMap.has(rbid)) {
      lane.rbid = boundaryIdMap.get(rbid)
    }
    if (boundaryIdMap.has(lbid)) {
      lane.lbid = boundaryIdMap.get(lbid)
    }
  }
  // 完成更新后手动清空
  boundaryIdMap.clear()
}

/**
 * 通过道路中心线创建车道边界线
 * @param params
 */
export function createLaneBoundary (
  params: ILaneBoundaryParams,
): Array<biz.ILaneBoundary> {
  const {
    keyPath,
    elevationPath,
    laneNumber,
    laneWidth,
    pStart,
    pEnd,
    roadType = 'default',
    isTwoWay,
  } = params
  const roadLength = keyPath.getLength()
  const boundary: Array<biz.ILaneBoundary> = []
  if (roadLength < 0) return boundary
  let offset = 0
  // 由于默认是单 section，基于车道长度计算采样点的间隔
  const segment = getSegmentByLength(roadLength)
  // 创建边界的次数，如果是双向车道，则需要再创建一次反向车道的边界
  // side=0 表示正向车道；side=1 表示反向车道
  const count = isTwoWay ? 2 : 1
  for (let side = 0; side < count; side++) {
    // 从正向车道切换到反向车道，需要重置车道宽度偏移量
    offset = 0
    for (let i = 0; i < laneNumber; i++) {
      offset += laneWidth
      const { refPoints, offsetPoints } = getParallelCurveSamplePoints({
        keyPath,
        elevationPath,
        offset,
        pStart,
        pEnd,
        segment,
        side,
      })
      // 对于正向车道，创建中间共用的边界线
      if (side === 0 && i === 0) {
        // 如果索引是 0 则需要将参考线的车道也保存
        let mark = DefaultLaneBoundaryMark.DoubleSolidYellow // 默认最内侧的边界线为双黄线
        if (roadType === 'ramp') {
          // 如果是匝道，最内侧的车道边界线样式为白色单实线
          mark = DefaultLaneBoundaryMark.SingleSolidWhite
        }
        boundary.push({
          id: genLaneBoundaryId(),
          mark,
          samplePoints: refPoints,
          firstLineAttr: null,
          secondLineAttr: null,
          isForward: true,
        })
      }

      // 如果是最外侧的边界线，则默认为白色单实线，否则为白色单虚线
      const boundaryMark =
        i === laneNumber - 1 ?
          DefaultLaneBoundaryMark.SingleSolidWhite :
          DefaultLaneBoundaryMark.SingleDashWhite
      boundary.push({
        id: genLaneBoundaryId(),
        mark: boundaryMark,
        samplePoints: offsetPoints,
        firstLineAttr: null,
        secondLineAttr: null,
        isForward: side === 0,
      })
    }
  }

  return boundary
}

/**
 * 基于道路中心线更新车道边界线的采样点
 * @param params
 */
export function updateLaneBoundaryInRoad (params: {
  keyPath: biz.ICurve3
  road: biz.IRoad
}) {
  const { keyPath, road } = params
  const { sections, elevationPath } = road

  // 遍历道路中已存在的 section
  for (const section of sections) {
    updateLaneBoundaryInSection({
      keyPath,
      elevationPath,
      section,
    })
  }
}

/**
 * 基于 section 内自身的属性和原有车道、边界结构，更新当前 section 内的采样点数据
 * @param params
 */
export function updateLaneBoundaryInSection (params: {
  section: biz.ISection
  keyPath: biz.ICurve3
  elevationPath?: biz.ICurve3
  updateBoundaryId?: boolean // 是否要强制更新当前 section 内的所有边界线 id
}) {
  const { section, keyPath, elevationPath, updateBoundaryId = false } = params
  // 由于每一条 lane 都是由左右两条边界线形成的，lanes 数量始终比 boundary 少 1
  // 通过遍历 lanes 顺便调整 boundary 的参数
  const { boundarys, lanes, pStart, pEnd } = section
  const segment = getMatchingSegment(section)
  let offset = 0
  // section 最内侧车道的左边界（与道路中心线重合）的边界 id
  let keyPathBoundaryId = ''
  // 平滑过渡车道 id
  let forwardTransitionLaneId = ''
  let reverseTransitionLaneId = ''
  // 将车道划分成正向车道和反向车道单独处理
  const forwardLanes = lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)
  // 正向车道
  for (const lane of forwardLanes) {
    const { lbid, rbid, normalWidth, isTransition, id: laneId } = lane
    if (!isTransition) {
      // 常规宽度的车道，是能够通过一个偏移常量量来计算偏移的采样点
      offset += normalWidth

      if (!forwardTransitionLaneId) {
        // 没有经过平滑车道的常规车道
        const { refPoints, offsetPoints } = getParallelCurveSamplePoints({
          keyPath,
          elevationPath,
          offset,
          pStart,
          pEnd,
          segment,
        })
        // 更新当前车道对应的车道边界线的采样点
        if (laneId === '-1') {
          // 记录最内侧车道左边界的 id
          keyPathBoundaryId = lbid
          // 靠近道路中心线最内侧的道路，始终 id 为 -1
          const leftBoundary = getBoundaryById(boundarys, lbid)
          if (leftBoundary) {
            leftBoundary.samplePoints = refPoints
          }
        }
        const rightBoundary = getBoundaryById(boundarys, rbid)
        if (rightBoundary) {
          rightBoundary.samplePoints = offsetPoints
        }
      } else {
        // 如果已经遍历过平滑过渡的车道，则后续外层的车道，需要基于当前车道的左边界采样点来计算
        // 获取当前车道的左边界采样点
        const leftBoundary = getBoundaryById(boundarys, lane.lbid)
        const rightBoundary = getBoundaryById(boundarys, lane.rbid)
        const keyPathBoundary = getBoundaryById(boundarys, keyPathBoundaryId)
        if (leftBoundary && rightBoundary && keyPathBoundary) {
          // 基于左侧边界线计算出来的偏移右侧边界线采样点
          const newRightBoundarySamplePoints =
            getRightBoundarySamplePointByLeft({
              keyPath,
              keyPathSamplePoints: keyPathBoundary.samplePoints,
              leftBoundarySamplePoints: leftBoundary.samplePoints,
              offset: lane.normalWidth,
              pStart,
              pEnd,
              segment,
            })
          rightBoundary.samplePoints = newRightBoundarySamplePoints
        }
      }
    } else {
      // 记录平滑过渡车道的 id
      forwardTransitionLaneId = lane.id
      // 更新当前车道的平滑过渡右边界采样点
      const { rbid, lbid } = lane
      const rightBoundary = getBoundaryById(boundarys, rbid)
      const leftBoundary = getBoundaryById(boundarys, lbid)
      if (rightBoundary && leftBoundary) {
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          offset,
          // 获取车道左边界的采样间隔数（采样点数 - 1）
          segment: leftBoundary.samplePoints.length - 1,
        })
        rightBoundary.samplePoints = transitionBoundarySamplePoints
      }
    }
  }

  // 在遍历反向车道时，重置车道宽度偏移量
  offset = 0
  // 反向车道
  for (const lane of reverseLanes) {
    const { lbid, rbid, normalWidth, isTransition, id: laneId } = lane
    if (!isTransition) {
      // 常规宽度的车道，是能够通过一个偏移常量量来计算偏移的采样点
      offset += normalWidth

      if (!reverseTransitionLaneId) {
        // 没有经过平滑车道的常规车道
        const { refPoints, offsetPoints } = getParallelCurveSamplePoints({
          keyPath,
          elevationPath,
          offset,
          pStart,
          pEnd,
          segment,
          side: 1, // 反向车道
        })

        // 如果在正向车道遍历过程中，没有更新道路参考线重合的边界线，则在遍历反向车道过程中进行更新
        if (laneId === '1' && !keyPathBoundaryId) {
          keyPathBoundaryId = rbid
          const rightBoundary = getBoundaryById(boundarys, rbid)
          if (rightBoundary) {
            rightBoundary.samplePoints = refPoints
          }
        }

        const leftBoundary = getBoundaryById(boundarys, lbid)
        if (leftBoundary) {
          leftBoundary.samplePoints = offsetPoints
        }
      } else {
        // 如果已经遍历过平滑过渡的车道，则后续外层的车道，需要基于当前车道的左边界采样点来计算
        // 获取当前车道的左边界采样点
        const leftBoundary = getBoundaryById(boundarys, lane.lbid)
        const rightBoundary = getBoundaryById(boundarys, lane.rbid)
        const keyPathBoundary = getBoundaryById(boundarys, keyPathBoundaryId)
        if (leftBoundary && rightBoundary && keyPathBoundary) {
          // 基于左侧边界线计算出来的偏移右侧边界线采样点
          const newRightBoundarySamplePoints =
            getLeftBoundarySamplePointByRight({
              keyPath,
              keyPathSamplePoints: keyPathBoundary.samplePoints,
              rightBoundarySamplePoints: rightBoundary.samplePoints,
              offset: lane.normalWidth,
              pStart,
              pEnd,
              segment,
            })
          leftBoundary.samplePoints = newRightBoundarySamplePoints
        }
      }
    } else {
      // 记录平滑过渡车道的 id
      reverseTransitionLaneId = lane.id
      // 更新当前车道的平滑过渡右边界采样点
      const { rbid, lbid } = lane
      const rightBoundary = getBoundaryById(boundarys, rbid)
      const leftBoundary = getBoundaryById(boundarys, lbid)
      if (rightBoundary && leftBoundary) {
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          // 反向车道传入相反的偏移量
          offset: -offset,
          // 获取车道左边界的采样间隔数（采样点数 - 1）
          segment: rightBoundary.samplePoints.length - 1,
        })
        leftBoundary.samplePoints = transitionBoundarySamplePoints
      }
    }
  }

  if (updateBoundaryId) {
    updateBoundaryIdInSection(section)
  }

  // 更新完车道边界线后，更新车道中心线采样点
  updateLaneCenterPoints({
    lanes,
    boundarys,
  })
}

/**
 * 创建平滑过渡的车道边界线
 * @param params
 */
export function createTransitionBoundaryInSection (params: {
  section: biz.ISection
  laneId: string
  tweenSamplePoints: Array<Vector3>
  keyPath: biz.ICurve3
  elevationPath?: biz.ICurve3
  updateBoundaryId?: boolean // 是否要强制更新当前 section 内的所有边界线 id
  isExtends: boolean // 是否是拓展车道的趋势
}) {
  const {
    section,
    laneId: targetLaneId,
    tweenSamplePoints,
    keyPath,
    elevationPath,
    updateBoundaryId = false,
    isExtends,
  } = params
  const { pStart, pEnd, boundarys, lanes } = section
  // 按照补间采样点的数量进行分段采样
  const segment = Constant.tweenSampleSegment
  let offset = 0
  // 道路中心线，即为最内侧车道的左边界线的 id
  let keyPathBoundaryId = ''
  for (let i = 0; i < lanes.length; i++) {
    const lane = lanes[i]
    const { id: laneId, normalWidth, lbid, rbid } = lane
    if (Math.abs(Number(laneId)) < Math.abs(Number(targetLaneId))) {
      // 如果有内侧的车道
      offset += normalWidth
      const { refPoints, offsetPoints } = getParallelCurveSamplePoints({
        keyPath,
        elevationPath,
        offset,
        pStart,
        pEnd,
        segment,
      })
      // 更新当前车道对应的车道边界线的采样点
      if (laneId === '-1') {
        // 靠近道路中心线最内侧的道路，始终 id 为 -1
        const leftBoundary = getBoundaryById(boundarys, lbid)
        if (leftBoundary) {
          leftBoundary.samplePoints = refPoints
          // 如果是最内侧车道，则记录左侧边界线 id
          keyPathBoundaryId = lbid
        }
      }
      const rightBoundary = getBoundaryById(boundarys, rbid)
      if (rightBoundary) {
        rightBoundary.samplePoints = offsetPoints
      }
    } else if (laneId === targetLaneId) {
      if (laneId === '-1') {
        // 如果目标车道是最内侧车道，还需要更新左边界的采样点
        const { refPoints } = getParallelCurveSamplePoints({
          keyPath,
          elevationPath,
          offset,
          pStart,
          pEnd,
          segment,
        })
        const leftBoundary = getBoundaryById(boundarys, lbid)
        if (leftBoundary) {
          leftBoundary.samplePoints = refPoints
          // 如果是最内侧车道，则记录左侧边界线 id
          keyPathBoundaryId = lbid
        }
      }

      // 更新目标车道右侧边界线采样点为平滑过渡的补间采样点
      const rightBoundary = getBoundaryById(boundarys, rbid)
      if (rightBoundary) {
        rightBoundary.samplePoints = tweenSamplePoints
      }
      lane.isTransition = true
      lane.isExtends = isExtends
    } else if (Math.abs(Number(laneId)) > Math.abs(Number(targetLaneId))) {
      // 要基于上一个车道右边界（即当前车道的左边界）采样点来重新当前车道右侧边界线的采样点
      const leftBoundary = getBoundaryById(boundarys, lbid)
      const rightBoundary = getBoundaryById(boundarys, rbid)
      const keyPathBoundary = getBoundaryById(boundarys, keyPathBoundaryId)
      if (leftBoundary && rightBoundary && keyPathBoundary) {
        // 通过左侧边界线的采样点，来计算车道右侧边界线的采样点
        const newSamplePoints = getRightBoundarySamplePointByLeft({
          keyPath,
          keyPathSamplePoints: keyPathBoundary.samplePoints,
          leftBoundarySamplePoints: leftBoundary.samplePoints,
          offset: normalWidth,
          pStart,
          pEnd,
          segment,
        })
        rightBoundary.samplePoints = newSamplePoints
      }
    }
  }

  if (updateBoundaryId) {
    updateBoundaryIdInSection(section)
  }

  // 更新完车道边界线后，更新车道中心线采样点
  updateLaneCenterPoints({
    lanes,
    boundarys,
  })
}

/**
 * 在已经划分了平滑过渡 section 的前提下，更新平滑过渡车道边界线
 * @param params
 */
export function updateTransitionBoundaryInLane (params: {
  keyPath: biz.ICurve3
  elevationPath?: biz.ICurve3
  lane: biz.ILane
  pStart: number
  pEnd: number
  offset: number // 距离车道左侧边界线的 offset
  segment: number
}) {
  const { lane, pStart, pEnd, offset, keyPath, elevationPath, segment } = params
  // 更新当前车道的平滑过渡右边界采样点
  const { normalWidth } = lane
  const isExtends = lane.isExtends as boolean
  const minOffset = offset
  const maxOffset = minOffset + normalWidth

  // 【TODO】平滑过渡车道边界线的算法逻辑优化，如果存在从地图文件中加载的初始边界，可以保持使用这个边界的间距

  // 右侧边界线的首尾的端点，根据平滑过渡的趋势来分别获取位于左、右边界线上的关键点
  let leftPoint
  let rightPoint
  if (isExtends) {
    leftPoint = getSamplePointByOffset({
      keyPath,
      elevationPath,
      percent: pStart,
      offset: minOffset,
    }).offsetPoint
    rightPoint = getSamplePointByOffset({
      keyPath,
      elevationPath,
      percent: pEnd,
      offset: maxOffset,
    }).offsetPoint
  } else {
    leftPoint = getSamplePointByOffset({
      keyPath,
      elevationPath,
      percent: pEnd,
      offset: maxOffset,
    }).offsetPoint
    rightPoint = getSamplePointByOffset({
      keyPath,
      elevationPath,
      percent: pStart,
      offset: minOffset,
    }).offsetPoint
  }

  const { transitionPoints } = getTransitionBoundarySamplePoints({
    keyPath,
    maxOffset,
    minOffset,
    pStart,
    pEnd,
    leftPoint,
    rightPoint,
    segment,
    isExtends,
  })

  return transitionPoints
}
