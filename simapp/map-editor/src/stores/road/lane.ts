// 车道相关的方法
import { pick } from 'lodash'
import { getBoundaryById, updateTransitionBoundaryInLane } from './boundary'
import {
  DefaultLaneBoundaryMark,
  LaneFriction,
  LaneTypeEnum,
  getLeftBoundarySamplePointByRight,
  getMatchingSegment,
  getParallelCurveSamplePoints,
  getRightBoundarySamplePointByLeft,
} from '@/utils/business'
import {
  getCenterLineByTwoLines,
  sortByAbsId,
  sortById,
} from '@/utils/common3d'
import { genLaneBoundaryId } from '@/utils/guid'

interface IUpdateLaneInSection {
  laneId: string
  section: biz.ISection
  keyPath: biz.ICurve3
  isOpenFile?: boolean // 判断当前是否是打开了地图
  elevationPath?: biz.ICurve3
}

interface IUpdateLaneWidthInSection extends IUpdateLaneInSection {
  width: number
}

/**
 * 获取 section 中最左侧和最右侧车道
 * @param lanes
 * @returns
 */
export function getLaneOnBothSide (lanes: Array<biz.ILane>) {
  let leftLane, rightLane
  if (lanes.length < 1) {
    return {
      left: leftLane,
      right: rightLane,
    }
  }

  // 如果只有一条车道
  if (lanes.length === 1) {
    leftLane = lanes[0]
    rightLane = lanes[0]
    return {
      left: leftLane,
      right: rightLane,
    }
  }

  // 存在多条车道才需要遍历
  // 考虑正反向车道的情况
  const forwardLanes = lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)

  if (forwardLanes.length < 1) {
    // 只存在反向车道
    leftLane = reverseLanes[reverseLanes.length - 1]
    rightLane = reverseLanes[0]
  } else if (reverseLanes.length < 1) {
    // 只存在正向车道
    leftLane = forwardLanes[0]
    rightLane = forwardLanes[forwardLanes.length - 1]
  } else {
    leftLane = reverseLanes[reverseLanes.length - 1]
    rightLane = forwardLanes[forwardLanes.length - 1]
  }
  return {
    left: leftLane,
    right: rightLane,
  }
}

/**
 * 获取道路一侧的左右车道（正向或反向）
 */
export function getLaneByDirection (
  lanes: Array<biz.ILane>,
  direction: biz.ILaneDirection,
) {
  const forwardLanes = lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)
  if (direction === 'forward') {
    if (forwardLanes.length < 1) {
      return {
        left: null,
        right: null,
      }
    } else {
      return {
        left: forwardLanes[0],
        right: forwardLanes[forwardLanes.length - 1],
      }
    }
  } else {
    if (reverseLanes.length < 1) {
      return {
        left: null,
        right: null,
      }
    } else {
      return {
        left: reverseLanes[reverseLanes.length - 1],
        right: reverseLanes[0],
      }
    }
  }
}

/**
 * 获取 section 中最内侧和最外侧的车道边界线
 * @param section
 */
export function getLaneBoundaryOnBothSides (
  section: biz.ISection,
  direction?: biz.ILaneDirection, // 区分正向车道和反向车道
) {
  const { lanes, boundarys } = section
  // 先获取最内侧和最外侧的车道
  let leftLane, rightLane
  if (direction === 'forward' || direction === 'reverse') {
    const { left: _leftLane, right: _rightLane } = getLaneByDirection(
      lanes,
      direction,
    )
    leftLane = _leftLane
    rightLane = _rightLane
  } else {
    const { left: _leftLane, right: _rightLane } = getLaneOnBothSide(lanes)
    leftLane = _leftLane
    rightLane = _rightLane
  }
  if (!leftLane || !rightLane) {
    return {
      left: null,
      right: null,
    }
  }

  const { lbid } = leftLane
  const leftBoundary = getBoundaryById(boundarys, lbid)
  const { rbid } = rightLane
  const rightBoundary = getBoundaryById(boundarys, rbid)

  return {
    left: leftBoundary,
    right: rightBoundary,
  }
}

/**
 * 通过 id 找对应的 lane
 * @param lanes
 * @param laneId
 */
export function getLaneById (lanes: Array<biz.ILane>, laneId: string) {
  const lane = lanes.find(lane => lane.id === laneId)
  return lane
}

/**
 * 基于已有的边界线更新车道中心线的采样点属性
 * @param params
 */
export function updateLaneCenterPoints (params: {
  lanes: Array<biz.ILane>
  boundarys: Array<biz.ILaneBoundary>
}) {
  const { lanes, boundarys } = params
  for (const lane of lanes) {
    const { lbid, rbid, isTransition, id: laneId } = lane
    // 【注意】224版本暂时不处理平滑过渡车道，均通过左右边界采样点来计算车道中心采样点
    const leftBoundary = getBoundaryById(boundarys, lbid)
    const rightBoundary = getBoundaryById(boundarys, rbid)
    if (!leftBoundary || !rightBoundary) continue
    const lanePoints = getCenterLineByTwoLines(
      leftBoundary.samplePoints,
      rightBoundary.samplePoints,
      true,
      false,
    )
    lane.samplePoints = lanePoints
    // 【注意】以下逻辑很重要，暂时不要删
    // if (!isTransition) {
    //   // 常规车道，即使是平滑过渡车道外侧的车道，只要有一个固定的车道宽度，就可以视为常规车道
    //   // 通过左右两侧边界线的采样点中点来计算
    //   const leftBoundary = getBoundaryById(boundarys, lbid)
    //   const rightBoundary = getBoundaryById(boundarys, rbid)
    //   if (!leftBoundary || !rightBoundary) continue
    //   const lanePoints = getCenterLineByTwoLines(
    //     leftBoundary.samplePoints,
    //     rightBoundary.samplePoints,
    //     true
    //   )
    //   lane.samplePoints = lanePoints
    // } else {
    //   // 平滑过渡车道中心线的处理
    //   // 由于平滑过渡车道当前不能是最内侧车道，所以可以通过 id+1 得到平滑过渡车道左侧相邻的车道
    //   const leftLaneId = String(Number(laneId) + 1)
    //   const leftLane = getLaneById(lanes, leftLaneId)
    //   if (!leftLane) continue
    //   // 获取左侧车道的左边界线
    //   const { lbid: leftLaneLbid } = leftLane
    //   const leftLaneLeftBoundary = getBoundaryById(boundarys, leftLaneLbid)
    //   // 获取当前车道的左右边界线
    //   const leftBoundary = getBoundaryById(boundarys, lbid)
    //   const rightBoundary = getBoundaryById(boundarys, rbid)
    //   if (!leftLaneLeftBoundary || !leftBoundary || !rightBoundary) continue

    //   const length = rightBoundary.samplePoints.length
    //   const targetLaneSamplePoints = []
    //   for (let i = 0; i < length; i++) {
    //     // 当前采样点索引所占的比例
    //     const rightPoint = rightBoundary.samplePoints[i]
    //     const leftPoint = leftBoundary.samplePoints[i]
    //     // 左侧车道的左侧边界线某个采样点
    //     const leftLaneLeftPoint = leftLaneLeftBoundary.samplePoints[i]
    //     const ratio = i / (length - 1)
    //     // 目标车道左侧边界线采样点，跟左侧车道左侧边界线采样点的线性插值结果
    //     const linearPoint = getLinearValueByTwoPoints({
    //       point1: leftPoint,
    //       point2: leftLaneLeftPoint,
    //       ratio,
    //     })
    //     // 目标车道右侧边界线采样点，跟线性插值的点的中点
    //     const lanePoint = getCenterByTwoVec3({
    //       v1: linearPoint,
    //       v2: rightPoint,
    //       fixed: true,
    //     })
    //     targetLaneSamplePoints.push(lanePoint)
    //   }

    //   lane.samplePoints = targetLaneSamplePoints
    // }
  }
}

/**
 * 判断指定的 section 区间中是否存在指定车道外侧的平滑过渡车道
 * @param params
 */
export function judgeOtherTransitionLaneInSection (params: {
  sections: Array<biz.ISection>
  startSectionId: string
  endSectionId: string
  laneId: string
}) {
  const { sections, startSectionId, endSectionId, laneId } = params
  for (let i = 0; i < sections.length; i++) {
    const section = sections[i]
    if (
      Number(section.id) < Number(startSectionId) ||
      Number(section.id) > Number(endSectionId)
    ) {
      // 跳过区间外的 section
      continue
    }

    for (const lane of section.lanes) {
      // 找到目标车道外侧的车道
      if (Math.abs(Number(lane.id)) > Math.abs(Number(laneId))) {
        if (lane.isTransition) return true
      }
    }
  }

  return false
}

/**
 * 获取跟平滑过渡车道存在连通关系车道的 section 区间
 * @param params
 */
export function getTransitionLaneLinkSection (params: {
  sections: Array<biz.ISection>
  sectionId: string
  laneId: string
}) {
  const { sections, sectionId, laneId } = params
  // section 是从 0 开始递增的，跟索引类似，所以可以将目标 sectionId 作为一个索引临界值
  let startSectionId = '0'
  let endSectionId = String(sections.length - 1)

  // 从目标 section 向前遍历，判断是否存在 isExtends 为 true 的平滑过渡车道
  for (let i = Number(sectionId); i >= 0; i--) {
    const section = sections[i]
    for (const lane of section.lanes) {
      if (lane.id === laneId) {
        if (lane.isTransition) {
          if (lane.isExtends) {
            // 找到第一个符合要求的拓宽趋势平滑过渡车道
            startSectionId = sectionId
          } else {
            // 在找到符合要求的拓宽趋势车道之前，遇到了变窄车道，会连带删除过渡车道外侧的车道，不符合规则
            // 由于 sectionId 正常情况不能为负数，手动赋值为 -1 标明出现异常情况，直接返回
            // 不能是目标 section 的目标车道
            if (section.id !== sectionId) {
              return {
                start: '-1',
                end: '-1',
              }
            }
          }
        }
        // 遍历完 id 相同的车道就可以退出车道的迭代
        break
      }
    }
    // 如果起始的 sectionId 跟默认第一个 sectionId 不同，则可以提前退出 section 的迭代
    if (startSectionId !== '0') break
  }

  // 从目标 section 向后遍历，判断是否存在 isExtends 为 false 的平滑过渡车道
  for (let i = Number(sectionId); i < sections.length; i++) {
    const section = sections[i]
    for (const lane of section.lanes) {
      if (lane.id === laneId) {
        if (lane.isTransition) {
          if (!lane.isExtends) {
            // 找到第一个符合要求的变窄趋势平滑过渡车道
            endSectionId = section.id
          } else {
            // 在找到符合要求的变窄趋势车道之前，遇到了拓宽车道，会连带删除过渡车道外侧的车道，不符合规则
            // 不能是目标 section 的目标车道
            if (section.id !== sectionId) {
              return {
                start: '-1',
                end: '-1',
              }
            }
          }
        }
        // 遍历完 id 相同的车道就可以退出车道的迭代
        break
      }
    }
    // 如果截止的 sectionId 跟默认最后一个 sectionId 不同，则可以提前退出 section 的迭代
    if (endSectionId !== String(section.length - 1)) break
  }
  return {
    start: startSectionId,
    end: endSectionId,
  }
}

/**
 * 遍历所有 section 中存在平滑过渡车道的最内侧 laneId
 * @param params
 */
export function getInnerTransitionLaneId (sections: Array<biz.ISection>) {
  let transitionLaneId = ''
  for (const section of sections) {
    for (const lane of section.lanes) {
      if (lane.isTransition) {
        // 如果平滑过渡车道 id 不存在，则直接记录
        if (!transitionLaneId) {
          transitionLaneId = lane.id
          // 跳出当前 section 中剩余 lane 的遍历
          break
        } else {
          // 如果已经存在，则判断是否有更小的 laneId
          if (Math.abs(Number(lane.id)) < Math.abs(Number(transitionLaneId))) {
            transitionLaneId = lane.id
            break
          }
        }
      }
    }
  }
  return transitionLaneId
}

/**
 * 判断目标的车道前后是否存在截断
 * @param road
 * @param sectionId
 * @param laneId
 */
export function judgeTargetLaneBlockIndex (
  road: biz.IRoad,
  sectionId: string,
  laneId: string,
) {
  // 按照 sectionId 从小到大排序 section
  road.sections.sort(sortById)
  let headBlockIndex = -1
  let tailBlockIndex = -1
  let meetTargetLane = false
  // 先遍历一遍 section 中的车道分布情况，检查是否有截断
  for (let i = 0; i < road.sections.length; i++) {
    const section = road.sections[i]
    const targetLane = section.lanes.find(lane => lane.id === laneId)
    if (section.id !== sectionId) {
      // 前后的 section
      if (!meetTargetLane) {
        // 如果没有遇到目标的车道，判断是否有之前的截断
        if (!targetLane) {
          // 找到离目标车道最近的一个前方截断 section
          headBlockIndex = i
        }
      } else {
        // 如果已经遇到了目标的车道，判断是否有之后的截断
        if (!targetLane && tailBlockIndex === -1) {
          // 只获取目标车道的第一个后方截断 section
          tailBlockIndex = i
        }
      }
    } else {
      // 匹配的 section
      if (targetLane) {
        meetTargetLane = true
      }
    }
  }
  return {
    head: headBlockIndex,
    tail: tailBlockIndex,
  }
}

/**
 * 基于车道边界线，创建车道自身的数据
 * @param boundary
 * @returns
 */
export function createLane (params: {
  boundary: Array<biz.ILaneBoundary>
  width: number
  roadType: string
  // 是否是双向道路
  isTwoWay: boolean
}) {
  const { boundary, width, roadType = 'default', isTwoWay = false } = params
  const lanes: Array<biz.ILane> = []
  let forwardId = 0
  let reverseId = 0
  // 位于道路参考线上的边界线，创建时默认为第一条
  const refBoundary = boundary[0]
  for (let i = 0; i < boundary.length - 1; i++) {
    let laneId,
      leftBoundaryId,
      rightBoundaryId,
      leftBoundaryPoints,
      rightBoundaryPoints
    const { isForward: isLeftForward } = boundary[i]
    const { isForward: isRightForward } = boundary[i + 1]
    // 不会出现left=false，right=true的情况
    if (isLeftForward === true && isRightForward === true) {
      // 正向车道的边界
      laneId = String(--forwardId)
      leftBoundaryId = boundary[i].id
      leftBoundaryPoints = boundary[i].samplePoints
      rightBoundaryId = boundary[i + 1].id
      rightBoundaryPoints = boundary[i + 1].samplePoints
    } else if (isLeftForward === false && isRightForward === false) {
      // 反向车道的边界
      laneId = String(++reverseId)
      leftBoundaryId = boundary[i + 1].id
      leftBoundaryPoints = boundary[i + 1].samplePoints
      rightBoundaryId = boundary[i].id
      rightBoundaryPoints = boundary[i].samplePoints
    } else if (isLeftForward === true && isRightForward === false) {
      // 此时将 i+1 边界跟参考线边界用来计算第一条反向车道
      laneId = String(++reverseId)
      leftBoundaryId = boundary[i + 1].id
      leftBoundaryPoints = boundary[i + 1].samplePoints
      rightBoundaryId = refBoundary.id
      rightBoundaryPoints = refBoundary.samplePoints
    } else {
      continue
    }

    const lanePoints = getCenterLineByTwoLines(
      leftBoundaryPoints,
      rightBoundaryPoints,
      true,
      false,
    )
    // 车道类型的 string 字符
    let type: biz.ILaneType = 'Driving'
    if (roadType === 'ramp') {
      // 如果是绘制匝道，则车道属性也需要调整成匝道
      type = 'ConnectingRamp'
    }
    const laneType = LaneTypeEnum[type]

    // 每一种车道类型，有默认的摩擦力系数
    const friction = LaneFriction[type] || 0

    lanes.push({
      id: laneId,
      // 默认新建车道类型为行驶车道
      type: laneType,
      lbid: leftBoundaryId,
      rbid: rightBoundaryId,
      // 跟后端沟通后，暂定先给车道默认限速 30
      speedlimit: 30,
      // speedlimit: 0,
      samplePoints: lanePoints,
      enabled: true,
      normalWidth: width,
      isTransition: false,
      geoAttr: null,
      // 车道材质属性
      friction,
      sOffset: 0, // 默认从 0 开始
    })
  }

  return lanes
}

/**
 * 更新某一个 section 中的车道宽度
 * @param params
 */
export function updateLaneWidthInSection (params: IUpdateLaneWidthInSection) {
  const {
    section,
    keyPath,
    elevationPath,
    laneId: targetLaneId,
    width,
    isOpenFile = false,
  } = params
  const { boundarys, lanes, pStart, pEnd } = section
  let segment = getMatchingSegment(section)
  if (isOpenFile) {
    // 如果是加载地图做二次编辑的情况下，采样间隔需要基于原有的边界线采样点数量来定
    const targetLane = getLaneById(lanes, targetLaneId)
    if (targetLane) {
      const boundary = getBoundaryById(boundarys, targetLane.rbid)
      if (boundary) {
        // 采样间隔 = 采样点数 - 1
        segment = boundary.samplePoints.length - 1
      }
    }
  }
  let offset = 0
  // 平滑过渡车道 id
  let transitionLaneId = ''
  // section 最内侧车道的左边界（与道路中心线重合）的边界 id
  let keyPathBoundaryId = ''

  // 判断当前更新的车道 id 是正向车道还是反向车道
  const isForward = Number(targetLaneId) < 0
  const forwardLanes = lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)

  if (isForward) {
    for (const lane of forwardLanes) {
      const { normalWidth, isTransition } = lane
      // 如果是目标车道内侧的车道，则不需要调整，只需要获取正确的偏移量
      if (Math.abs(Number(lane.id)) < Math.abs(Number(targetLaneId))) {
        offset += normalWidth
        // 最内侧车道的左边界 id 需要记录
        if (lane.id === '-1') {
          keyPathBoundaryId = lane.lbid
        }
        // 如果有平滑过渡车道需要记录 id
        if (isTransition) {
          transitionLaneId = lane.id
        }
        continue
      }
      // 此处只剩下目标车道和外侧的车道
      if (!isTransition) {
        // 常规车道的调整

        // 如果是目标车道
        if (lane.id === targetLaneId) {
          // 最内侧车道的左边界 id 需要记录
          if (lane.id === '-1') {
            keyPathBoundaryId = lane.lbid
          }

          // 调整目标车道的宽度
          offset += width
          lane.normalWidth = width
        } else {
          offset += normalWidth
        }

        if (!transitionLaneId) {
          // 没有经过平滑车道的常规车道
          const { offsetPoints } = getParallelCurveSamplePoints({
            keyPath,
            elevationPath,
            offset,
            pStart,
            pEnd,
            segment,
          })
          const rightBoundary = getBoundaryById(boundarys, lane.rbid)
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
        // 平滑过渡车道的调整
        // 记录平滑过渡车道的 id
        transitionLaneId = lane.id
        // 更新当前车道的平滑过渡右边界采样点
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          offset,
          segment,
        })
        // 更新平滑过渡车道的右侧边界采样点
        const rightBoundary = getBoundaryById(boundarys, lane.rbid)
        if (rightBoundary) {
          rightBoundary.samplePoints = transitionBoundarySamplePoints
        }
      }
    }
  } else {
    for (const lane of reverseLanes) {
      const { normalWidth, isTransition } = lane
      // 如果是目标车道内侧的车道，则不需要调整，只需要获取正确的偏移量
      if (Math.abs(Number(lane.id)) < Math.abs(Number(targetLaneId))) {
        offset += normalWidth
        // 反向车道的最内侧车道的右边界 id 需要记录
        if (lane.id === '1') {
          keyPathBoundaryId = lane.rbid
        }
        // 如果有平滑过渡车道需要记录 id
        if (isTransition) {
          transitionLaneId = lane.id
        }
        continue
      }
      // 此处只剩下目标车道和外侧的车道
      if (!isTransition) {
        // 常规车道的调整

        // 如果是目标车道
        if (lane.id === targetLaneId) {
          // 反向车道的最内侧车道的右边界 id 需要记录
          if (lane.id === '1') {
            keyPathBoundaryId = lane.rbid
          }

          // 调整目标车道的宽度
          offset += width
          lane.normalWidth = width
        } else {
          offset += normalWidth
        }

        if (!transitionLaneId) {
          // 没有经过平滑车道的常规车道
          const { offsetPoints } = getParallelCurveSamplePoints({
            keyPath,
            elevationPath,
            offset,
            pStart,
            pEnd,
            segment,
            side: 1, // 反向车道
          })
          const leftBoundary = getBoundaryById(boundarys, lane.lbid)
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
            const newLeftBoundarySamplePoints =
              getLeftBoundarySamplePointByRight({
                keyPath,
                keyPathSamplePoints: keyPathBoundary.samplePoints,
                rightBoundarySamplePoints: rightBoundary.samplePoints,
                offset: lane.normalWidth,
                pStart,
                pEnd,
                segment,
              })
            leftBoundary.samplePoints = newLeftBoundarySamplePoints
          }
        }
      } else {
        // 平滑过渡车道的调整
        // 记录平滑过渡车道的 id
        transitionLaneId = lane.id
        // 更新当前车道的平滑过渡右边界采样点
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          offset: -offset,
          segment,
        })
        // 更新平滑过渡车道的左侧边界采样点
        const leftBoundary = getBoundaryById(boundarys, lane.lbid)
        if (leftBoundary) {
          leftBoundary.samplePoints = transitionBoundarySamplePoints
        }
      }
    }
  }
}

/**
 * 新增某一个 section 中的车道
 * @param params
 */
export function addLaneInSection (params: IUpdateLaneInSection) {
  const {
    laneId: targetLaneId,
    section,
    keyPath,
    elevationPath,
    isOpenFile = false,
  } = params
  const { lanes, boundarys, pEnd, pStart } = section
  let segment = getMatchingSegment(section)
  if (isOpenFile) {
    // 如果是加载地图做二次编辑的情况下，采样间隔需要基于原有的边界线采样点数量来定
    const targetLane = getLaneById(lanes, targetLaneId)
    if (targetLane) {
      const boundary = getBoundaryById(boundarys, targetLane.rbid)
      if (boundary) {
        // 采样间隔 = 采样点数 - 1
        segment = boundary.samplePoints.length - 1
      }
    }
  }
  // 新增的车道边界线 id
  let newLaneBoundaryId
  let offset = 0
  // 平滑过渡车道 id
  let transitionLaneId = ''
  // section 最内侧车道的左边界（与道路中心线重合）的边界 id
  let keyPathBoundaryId = ''

  // 判断当前更新的车道 id 是正向车道还是反向车道
  const isForward = Number(targetLaneId) < 0
  const forwardLanes = lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)

  if (isForward) {
    // 正向车道
    for (let j = 0; j < forwardLanes.length; j++) {
      const lane = forwardLanes[j]
      const { isTransition } = lane
      // 如果是目标车道内侧的车道，则不需要调整，只需要获取正确的偏移量
      if (Math.abs(Number(lane.id)) < Math.abs(Number(targetLaneId))) {
        offset += lane.normalWidth
        // 最内侧车道的左边界 id 需要记录
        if (lane.id === '-1') {
          keyPathBoundaryId = lane.lbid
        }
        // 如果有平滑过渡车道需要记录 id
        if (isTransition) {
          transitionLaneId = lane.id
        }
        // 目标车道之前的车道边界线采样点都不需要更新
        continue
      }
      // 此处只剩下目标车道和外侧的车道（包含新增的那条车道）
      if (!isTransition) {
        offset += lane.normalWidth
        if (lane.id === targetLaneId) {
          // 如果是最内侧车道，则记录其左边界 id
          if (lane.id === '-1') {
            keyPathBoundaryId = lane.lbid
          }

          // 如果是目标车道，在当前车道右侧新增一条车道
          newLaneBoundaryId = genLaneBoundaryId()
          const newLane: biz.ILane = {
            id: String(Number(targetLaneId) - 1),
            lbid: lane.rbid, // 新增车道左边界，为目标车道的右边界
            rbid: newLaneBoundaryId,
            samplePoints: [],
            geoAttr: null,
            friction:
              LaneFriction[LaneTypeEnum[lane.type] as biz.ILaneType] || 0,
            sOffset: 0,
            ...pick(lane, [
              'isTransition',
              'normalWidth',
              'speedlimit',
              'type',
              'enabled',
            ]),
          }
          forwardLanes.splice(j + 1, 0, newLane)

          // 新增车道边界线的默认样式，默认跟目标车道右边界的样式一致
          const targetLaneRightBoundary = getBoundaryById(boundarys, lane.rbid)
          const newBoundaryMark = targetLaneRightBoundary ?
            targetLaneRightBoundary.mark :
            DefaultLaneBoundaryMark.SingleDashWhite
          // 新增一条车道线
          boundarys.push({
            id: newLaneBoundaryId,
            mark: newBoundaryMark,
            samplePoints: [],
            firstLineAttr: null,
            secondLineAttr: null,
          })
          continue
        } else {
          // 由于新增的车道跟目标车道原先右侧的车道 id 相同，通过边界来判断是哪一条
          if (lane.id === String(Number(targetLaneId) - 1)) {
            if (lane.rbid !== newLaneBoundaryId) {
              // 原先右侧车道的左边界要调整成新的边界，id 递减
              lane.lbid = newLaneBoundaryId as string
              lane.id = String(Number(lane.id) - 1)
            }
          } else {
            // 其余的车道需要对 id 进行递减
            lane.id = String(Number(lane.id) - 1)
          }
          if (!transitionLaneId) {
            // 没有经过平滑车道的常规车道
            const { offsetPoints } = getParallelCurveSamplePoints({
              keyPath,
              elevationPath,
              offset,
              pStart,
              pEnd,
              segment,
            })
            // 更新车道右侧边界线的采样点
            const rightBoundary = getBoundaryById(boundarys, lane.rbid)
            if (rightBoundary) {
              rightBoundary.samplePoints = offsetPoints
            }
          } else {
            // 如果已经遍历过平滑过渡的车道，则后续外层的车道，需要基于当前车道的左边界采样点来计算
            // 获取当前车道的左边界采样点
            const leftBoundary = getBoundaryById(boundarys, lane.lbid)
            const rightBoundary = getBoundaryById(boundarys, lane.rbid)
            const keyPathBoundary = getBoundaryById(
              boundarys,
              keyPathBoundaryId,
            )
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
        }
      } else {
        // 平滑过渡车道的调整
        // 如果平滑过渡车道靠近目标车道，由于新增的车道跟目标车道原先右侧的车道 id 相同，通过边界来判断是哪一条
        if (lane.id === String(Number(targetLaneId) - 1)) {
          if (lane.rbid !== newLaneBoundaryId) {
            // 原先右侧车道的左边界要调整成新的边界
            lane.lbid = newLaneBoundaryId as string
          }
        }
        // id 递减
        lane.id = String(Number(lane.id) - 1)

        // 记录平滑过渡车道的 id，需要在更新 id 后
        transitionLaneId = lane.id

        // 更新当前车道的平滑过渡右边界采样点
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          offset,
          segment,
        })
        // 更新平滑过渡车道的右侧边界采样点
        const rightBoundary = getBoundaryById(boundarys, lane.rbid)
        if (rightBoundary) {
          rightBoundary.samplePoints = transitionBoundarySamplePoints
        }
      }
    }
  } else {
    // 反向车道
    for (let j = 0; j < reverseLanes.length; j++) {
      const lane = reverseLanes[j]
      const { isTransition } = lane
      // 如果是目标车道内侧的车道，则不需要调整，只需要获取正确的偏移量
      if (Math.abs(Number(lane.id)) < Math.abs(Number(targetLaneId))) {
        offset += lane.normalWidth
        // 最内侧车道的右边界 id 需要记录
        if (lane.id === '1') {
          keyPathBoundaryId = lane.rbid
        }
        // 如果有平滑过渡车道需要记录 id
        if (isTransition) {
          transitionLaneId = lane.id
        }
        // 目标车道之前的车道边界线采样点都不需要更新
        continue
      }
      // 此处只剩下目标车道和外侧的车道（包含新增的那条车道）
      if (!isTransition) {
        offset += lane.normalWidth
        if (lane.id === targetLaneId) {
          // 如果是最内侧车道，则记录其右边界 id
          if (lane.id === '1') {
            keyPathBoundaryId = lane.rbid
          }

          // 如果是目标车道，在当前车道左侧新增一条车道
          newLaneBoundaryId = genLaneBoundaryId()
          const newLane: biz.ILane = {
            // 反向车道的 id 为正数且递增
            id: String(Number(targetLaneId) + 1),
            lbid: newLaneBoundaryId,
            rbid: lane.lbid, // 新增车道右边界，为目标车道的左边界
            samplePoints: [],
            geoAttr: null,
            friction:
              LaneFriction[LaneTypeEnum[lane.type] as biz.ILaneType] || 0,
            sOffset: 0,
            ...pick(lane, [
              'isTransition',
              'normalWidth',
              'speedlimit',
              'type',
              'enabled',
            ]),
          }
          reverseLanes.splice(j + 1, 0, newLane)

          // 新增车道边界线的默认样式，默认跟目标车道左边界的样式一致
          const targetLaneLeftBoundary = getBoundaryById(boundarys, lane.lbid)
          const newBoundaryMark = targetLaneLeftBoundary ?
            targetLaneLeftBoundary.mark :
            DefaultLaneBoundaryMark.SingleDashWhite
          // 新增一条车道线
          boundarys.push({
            id: newLaneBoundaryId,
            mark: newBoundaryMark,
            samplePoints: [],
            firstLineAttr: null,
            secondLineAttr: null,
          })
          continue
        } else {
          // 由于新增的车道跟目标车道原先右侧的车道 id 相同，通过边界来判断是哪一条
          if (lane.id === String(Number(targetLaneId) + 1)) {
            if (lane.lbid !== newLaneBoundaryId) {
              // 原先左侧车道的右边界要调整成新的边界，id 递增
              lane.rbid = newLaneBoundaryId as string
              lane.id = String(Number(lane.id) + 1)
            }
          } else {
            // 其余的车道需要对 id 进行递增
            lane.id = String(Number(lane.id) + 1)
          }
          if (!transitionLaneId) {
            // 没有经过平滑车道的常规车道
            const { offsetPoints } = getParallelCurveSamplePoints({
              keyPath,
              elevationPath,
              offset,
              pStart,
              pEnd,
              segment,
              side: 1, // 反向车道
            })
            // 更新车道左侧边界线的采样点
            const leftBoundary = getBoundaryById(boundarys, lane.lbid)
            if (leftBoundary) {
              leftBoundary.samplePoints = offsetPoints
            }
          } else {
            // 如果已经遍历过平滑过渡的车道，则后续外层的车道，需要基于当前车道的右边界采样点来计算
            // 获取当前车道的右边界采样点
            const leftBoundary = getBoundaryById(boundarys, lane.lbid)
            const rightBoundary = getBoundaryById(boundarys, lane.rbid)
            const keyPathBoundary = getBoundaryById(
              boundarys,
              keyPathBoundaryId,
            )
            if (leftBoundary && rightBoundary && keyPathBoundary) {
              // 基于左侧边界线计算出来的偏移右侧边界线采样点
              const newLeftBoundarySamplePoints =
                getLeftBoundarySamplePointByRight({
                  keyPath,
                  keyPathSamplePoints: keyPathBoundary.samplePoints,
                  rightBoundarySamplePoints: rightBoundary.samplePoints,
                  offset: lane.normalWidth,
                  pStart,
                  pEnd,
                  segment,
                })
              leftBoundary.samplePoints = newLeftBoundarySamplePoints
            }
          }
        }
      } else {
        // 平滑过渡车道的调整
        // 如果平滑过渡车道靠近目标车道，由于新增的车道跟目标车道原先左侧的车道 id 相同，通过边界来判断是哪一条
        if (lane.id === String(Number(targetLaneId) + 1)) {
          if (lane.lbid !== newLaneBoundaryId) {
            // 原先左侧车道的右边界要调整成新的边界
            lane.rbid = newLaneBoundaryId as string
          }
        }
        // id 递增
        lane.id = String(Number(lane.id) + 1)

        // 记录平滑过渡车道的 id，需要在更新 id 后
        transitionLaneId = lane.id

        // 更新当前车道的平滑过渡左边界采样点
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          offset: -offset,
          segment,
        })
        // 更新平滑过渡车道的左侧边界采样点
        const leftBoundary = getBoundaryById(boundarys, lane.lbid)
        if (leftBoundary) {
          leftBoundary.samplePoints = transitionBoundarySamplePoints
        }
      }
    }
  }

  // 重新组装 section 的车道
  section.lanes = [...reverseLanes, ...forwardLanes]
}

/**
 * 删除 section 中某一条车道
 * @param params
 */
export function removeLaneInSection (params: IUpdateLaneInSection) {
  // 由于前置的判断已经规避了多种异常情况，并且暂不支持删除平滑过渡车道外侧的车道
  // 可视为存在 4 种情况：
  // 1、删除常规车道
  // 2、删除平滑过渡车道内侧的常规车道（平滑过渡车道边界需要联动调整）
  // 3、删除外侧没有常规车道的平滑过渡车道
  // 4、删除外层有常规车道的平滑过渡车道
  const { laneId: targetLaneId, section, keyPath, elevationPath } = params
  const { lanes, boundarys, pEnd, pStart } = section
  const segment = getMatchingSegment(section)
  // 待删除车道左边界线的 id（正向车道）
  let removeLaneLbid
  // 待删除车道右边界线的 id（反向车道）
  let removeLaneRbid
  let offset = 0
  // 记录平滑过渡车道的 id，如果删除的是平滑过渡的车道，则该值需要清空
  let transitionLaneId = ''
  // section 最内侧车道的左边界（与道路中心线重合）的边界 id
  let keyPathBoundaryId = ''

  // 判断当前更新的车道 id 是正向车道还是反向车道
  const isForward = Number(targetLaneId) < 0
  const forwardLanes = lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)

  if (isForward) {
    // 正向车道
    for (let j = 0; j < forwardLanes.length; j++) {
      const lane = forwardLanes[j]
      // 如果是目标车道内侧的车道，则不需要调整，只需要获取正确的偏移量
      if (Math.abs(Number(lane.id)) < Math.abs(Number(targetLaneId))) {
        offset += lane.normalWidth
        if (lane.id === '-1') {
          keyPathBoundaryId = lane.lbid
        }
        continue
      }
      // 此处只剩下目标车道和外侧的车道
      const { isTransition } = lane

      // 不管是常规车道和平滑过渡车道，都是同样的删除逻辑
      if (!removeLaneLbid && lane.id === targetLaneId) {
        if (lane.id === '-1') {
          keyPathBoundaryId = lane.lbid
        }

        // 如果是目标车道，需要提前备份数据
        removeLaneLbid = lane.lbid

        // 将该车道右边界删除
        const rightBoundaryIndex = boundarys.findIndex(
          b => b.id === lane.rbid,
        )

        // 如果删除的是平滑过渡车道，那么后续的常规车道就不会受到平滑过渡车道的影响，反而需要将记录的 id 重置
        if (lane.isTransition) {
          transitionLaneId = ''
        }

        if (rightBoundaryIndex > -1) {
          boundarys.splice(rightBoundaryIndex, 1)
        }

        // 将车道删除，由于删除了一个元素，手动控制索引递减
        forwardLanes.splice(j, 1)
        j--
        continue
      }

      // 此处只剩下目标车道的外侧车道
      if (!isTransition) {
        // 常规车道，也分为 2 种：平滑过渡车道内侧、平滑过渡车道外侧，通过是否有 transitionLaneId 来判断

        // 已删除的车道不参与偏移量的计算，剩余的车道参与
        offset += lane.normalWidth

        // 已删除车道右侧的车道
        if (lane.id === String(Number(targetLaneId) - 1)) {
          // 将已删除车道的左边界，赋给原先右侧车道的左边界
          lane.lbid = removeLaneLbid as string
        }
        // 由于少了一条车道，剩余车道的 id 需要 +1
        lane.id = String(Number(lane.id) + 1)

        if (!transitionLaneId) {
          // 平滑过渡车道内侧的常规车道
          const { offsetPoints } = getParallelCurveSamplePoints({
            keyPath,
            elevationPath,
            offset,
            pStart,
            pEnd,
            segment,
          })
          // 更新车道右侧边界线的采样点
          const rightBoundary = getBoundaryById(boundarys, lane.rbid)
          if (rightBoundary) {
            rightBoundary.samplePoints = offsetPoints
          }
        } else {
          // 平滑过渡车道外侧的常规车道，由于平滑过渡车道还存在，需要用左边界的采样点来计算右侧边界的采样点
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
        // 如果删除的是平滑车道内侧的常规车道，那么此处的平滑过渡车道需要调整采样点

        // 记录平滑过渡车道的 id
        transitionLaneId = lane.id

        // 如果平滑过渡车道，是已删除车道的右侧车道
        if (lane.id === String(Number(targetLaneId) - 1)) {
          // 将已删除车道的左边界，赋值给原先右侧车道的左边界
          lane.lbid = removeLaneLbid as string
        }
        // 由于少了一条车道，剩余车道的 id 需要 +1
        lane.id = String(Number(lane.id) + 1)

        // 获取平滑过渡车道新的右侧边界线采样点
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          offset,
          segment,
        })
        // 更新右侧边界线的采样点
        const { rbid } = lane
        const rightBoundary = getBoundaryById(boundarys, rbid)
        if (rightBoundary) {
          rightBoundary.samplePoints = transitionBoundarySamplePoints
        }
      }
    }
  } else {
    // 反向车道
    for (let j = 0; j < reverseLanes.length; j++) {
      const lane = reverseLanes[j]
      // 如果是目标车道内侧的车道，则不需要调整，只需要获取正确的偏移量
      if (Math.abs(Number(lane.id)) < Math.abs(Number(targetLaneId))) {
        offset += lane.normalWidth
        if (lane.id === '1') {
          keyPathBoundaryId = lane.rbid
        }
        continue
      }
      // 此处只剩下目标车道和外侧的车道
      const { isTransition } = lane

      // 不管是常规车道和平滑过渡车道，都是同样的删除逻辑
      if (!removeLaneRbid && lane.id === targetLaneId) {
        if (lane.id === '1') {
          keyPathBoundaryId = lane.rbid
        }

        // 如果是目标车道，需要提前备份数据
        removeLaneRbid = lane.rbid

        // 将该车道左边界删除
        const leftBoundaryIndex = boundarys.findIndex(b => b.id === lane.lbid)

        // 如果删除的是平滑过渡车道，那么后续的常规车道就不会受到平滑过渡车道的影响，反而需要将记录的 id 重置
        if (lane.isTransition) {
          transitionLaneId = ''
        }

        if (leftBoundaryIndex > -1) {
          boundarys.splice(leftBoundaryIndex, 1)
        }

        // 将车道删除，由于删除了一个元素，手动控制索引递减
        reverseLanes.splice(j, 1)
        j--
        continue
      }

      // 此处只剩下目标车道的外侧车道
      if (!isTransition) {
        // 常规车道，也分为 2 种：平滑过渡车道内侧、平滑过渡车道外侧，通过是否有 transitionLaneId 来判断

        // 已删除的车道不参与偏移量的计算，剩余的车道参与
        offset += lane.normalWidth

        // 已删除车道左侧的车道
        if (lane.id === String(Number(targetLaneId) + 1)) {
          // 将已删除车道的右边界，赋给原先左侧车道的左边界
          lane.rbid = removeLaneRbid as string
        }
        // 由于少了一条车道，剩余车道的 id 需要 -1
        lane.id = String(Number(lane.id) - 1)

        if (!transitionLaneId) {
          // 平滑过渡车道内侧的常规车道
          const { offsetPoints } = getParallelCurveSamplePoints({
            keyPath,
            elevationPath,
            offset,
            pStart,
            pEnd,
            segment,
            side: 1, // 反向车道
          })
          // 更新车道左侧边界线的采样点
          const leftBoundary = getBoundaryById(boundarys, lane.lbid)
          if (leftBoundary) {
            leftBoundary.samplePoints = offsetPoints
          }
        } else {
          // 平滑过渡车道外侧的常规车道，由于平滑过渡车道还存在，需要用右边界的采样点来计算左侧边界的采样点
          // 获取当前车道的右边界采样点
          const leftBoundary = getBoundaryById(boundarys, lane.lbid)
          const rightBoundary = getBoundaryById(boundarys, lane.rbid)
          const keyPathBoundary = getBoundaryById(boundarys, keyPathBoundaryId)
          if (leftBoundary && rightBoundary && keyPathBoundary) {
            // 基于右侧边界线计算出来的偏移左侧边界线采样点
            const newLeftBoundarySamplePoints =
              getLeftBoundarySamplePointByRight({
                keyPath,
                keyPathSamplePoints: keyPathBoundary.samplePoints,
                rightBoundarySamplePoints: rightBoundary.samplePoints,
                offset: lane.normalWidth,
                pStart,
                pEnd,
                segment,
              })
            leftBoundary.samplePoints = newLeftBoundarySamplePoints
          }
        }
      } else {
        // 如果删除的是平滑车道内侧的常规车道，那么此处的平滑过渡车道需要调整采样点

        // 记录平滑过渡车道的 id
        transitionLaneId = lane.id

        // 如果平滑过渡车道，是已删除车道的左侧车道
        if (lane.id === String(Number(targetLaneId) + 1)) {
          // 将已删除车道的右边界，赋值给原先左侧车道的右边界
          lane.rbid = removeLaneRbid as string
        }
        // 由于少了一条车道，剩余车道的 id 需要 -1
        lane.id = String(Number(lane.id) - 1)

        // 获取平滑过渡车道新的右侧边界线采样点
        const transitionBoundarySamplePoints = updateTransitionBoundaryInLane({
          keyPath,
          elevationPath,
          lane,
          pStart,
          pEnd,
          offset: -offset,
          segment,
        })
        // 更新左侧边界线的采样点
        const leftBoundary = getBoundaryById(boundarys, lane.lbid)
        if (leftBoundary) {
          leftBoundary.samplePoints = transitionBoundarySamplePoints
        }
      }
    }
  }

  // 由于正向车道 id 始终是按照 -1, -2, ... 递减，通过 id 获取最外侧车道
  const forwardOuterLane = getLaneById(
    forwardLanes,
    String(0 - forwardLanes.length),
  )
  if (forwardOuterLane) {
    // 在完成车道的删除操作后，检查最外侧车道的右边界线样式，保证其为默认的白色单实线
    const { rbid } = forwardOuterLane
    const rightBoundary = getBoundaryById(boundarys, rbid)
    if (rightBoundary) {
      rightBoundary.mark = DefaultLaneBoundaryMark.SingleSolidWhite
    }
  }

  // 同上，反向车道 id 始终是按照 1, 2, ... 递增，通过 id 获取最外侧车道
  const reverseOuterLane = getLaneById(
    reverseLanes,
    String(reverseLanes.length),
  )
  if (reverseOuterLane) {
    // 在完成车道的删除操作后，检查最外侧车道的左边界线样式，保证其为默认的白色单实线
    const { lbid } = reverseOuterLane
    const leftBoundary = getBoundaryById(boundarys, lbid)
    if (leftBoundary) {
      leftBoundary.mark = DefaultLaneBoundaryMark.SingleSolidWhite
    }
  }

  // 重新组装 section 的车道
  section.lanes = [...reverseLanes, ...forwardLanes]
}
