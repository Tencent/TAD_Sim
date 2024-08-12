import { cloneDeep } from 'lodash'
import type { Vector3 } from 'three'
import {
  createTransitionBoundaryInSection,
  updateLaneBoundaryInSection,
} from './boundary'
import {
  judgeTargetLaneBlockIndex,
  removeLaneInSection,
  updateLaneCenterPoints,
} from './lane'
import { fixedPrecision } from '@/utils/common3d'
import { Constant } from '@/utils/business'

interface IDivideSectionWithTween {
  leftIndex: string
  rightIndex: string
  road: biz.IRoad
  section: biz.ISection
  laneId: string
  tweenSamplePoints: Array<Vector3>
}

/**
 * 默认创建单 section
 * @returns
 */
export function createSection (params?: {
  id?: string
  pStart?: number
  pEnd?: number
  roadLength: number
}): biz.ISection {
  const { id = '0', pStart = 0, pEnd = 1, roadLength = 0 } = params || {}
  const length = fixedPrecision(roadLength * (pEnd - pStart))
  return {
    id,
    length,
    lanes: [],
    boundarys: [],
    pStart,
    pEnd,
  }
}

/**
 * 仅仅将一个 section 拆分成两个常规的 section
 * @param params
 */
export function divideSectionOnly (params: {
  road: biz.IRoad
  section: biz.ISection
  laneId: string
  newPercent: number
}) {
  const {
    road,
    section: targetSection,
    laneId: targetLaneId,
    newPercent,
  } = params
  // 先判断车道之间是否在数量上有截断
  const { head: headBlockIndex, tail: tailBlockIndex } =
    judgeTargetLaneBlockIndex(road, targetSection.id, targetLaneId)
  const { keyPath, length: roadLength } = road
  const { pStart, pEnd } = targetSection

  for (let i = 0; i < road.sections.length; i++) {
    // 对于截断区域之外的 section 不处理
    if (headBlockIndex !== -1 && i <= headBlockIndex) continue
    if (tailBlockIndex !== -1 && i >= tailBlockIndex) continue
    const section = road.sections[i]
    const { id: _sectionId } = section
    // 单纯截断，对 pStart, pEnd, sectionId 做调整，然后 section 基于新的属性重新获取车道边界线和车道中心线的采样
    if (targetSection.id === _sectionId) {
      // 要一分为二的 section
      // 当前section，改成前一部分的 section
      section.pEnd = newPercent
      section.length = fixedPrecision((newPercent - pStart) * roadLength)
      // 先拷贝一份留给新的 section
      const boundarysBackup = cloneDeep(section.boundarys)
      const lanesBackup = cloneDeep(section.lanes)
      // 更新前一部分 section 的边界线采样点
      updateLaneBoundaryInSection({
        keyPath,
        section,
      })

      // 新增后一部分的 section
      const newSection: biz.ISection = {
        id: String(Number(_sectionId) + 1),
        length: fixedPrecision((pEnd - newPercent) * roadLength),
        lanes: lanesBackup,
        boundarys: boundarysBackup,
        pStart: newPercent,
        pEnd,
      }
      // 将新增的 section 插入数组，并且递增 i 跳过这个新增的 section
      road.sections.splice(i + 1, 0, newSection)
      i++
      // 更新后一部分 section 的边界线采样点
      updateLaneBoundaryInSection({
        keyPath,
        section: newSection,
        updateBoundaryId: true,
      })
    } else if (Number(_sectionId) > Number(section)) {
      // 截断 section 之后的，需要 id 自增，其他不做调整
      section.id = String(Number(_sectionId) + 1)
    }
  }
}

/**
 * 从 n-1 车道拓宽成 n 车道，并划分 section
 * @param params
 */
export function divideSectionWithWidenTween (params: IDivideSectionWithTween) {
  const {
    road,
    section: targetSection,
    laneId: targetLaneId,
    leftIndex,
    rightIndex,
    tweenSamplePoints,
  } = params
  // 先判断车道之间是否在数量上有截断
  const { head: headBlockIndex, tail: tailBlockIndex } =
    judgeTargetLaneBlockIndex(road, targetSection.id, targetLaneId)
  const { keyPath, length: roadLength } = road
  const { pStart, pEnd } = targetSection
  const [_leftIndex, totalIndex] = leftIndex
    .split('/')
    .map(num => Number(num))
  const [_rightIndex] = rightIndex.split('/').map(num => Number(num))
  const totalSegment = totalIndex - 1
  const delta = (pEnd - pStart) / totalSegment
  // 左边界线的截断点在前
  const leftPercent = fixedPrecision(
    pStart + delta * _leftIndex,
    Constant.offsetPointsSamplePrecision,
  )
  // 右边界线的截断点在后
  const rightPercent = fixedPrecision(
    pStart + delta * _rightIndex,
    Constant.offsetPointsSamplePrecision,
  )
  for (let i = 0; i < road.sections.length; i++) {
    // 对于截断区域之外的 section 不处理
    if (headBlockIndex !== -1 && i <= headBlockIndex) continue
    if (tailBlockIndex !== -1 && i >= tailBlockIndex) continue

    const section = road.sections[i]
    const { id: sectionId } = section

    // 无论是什么情况，目标section之前的section，都需要删除目标 laneId 的车道（如果存在的话）
    if (Number(sectionId) < Number(targetSection.id)) {
      // 目标 section 之前的 section，删除对应的 laneId 车道
      removeLaneInSection({
        laneId: targetLaneId,
        section,
        keyPath,
      })
      // 删除 section 中的 lane，需要手动更新车道中心线采样点
      updateLaneCenterPoints({
        lanes: section.lanes,
        boundarys: section.boundarys,
      })
    }

    if (leftPercent === pStart && rightPercent === pEnd) {
      // 平滑过渡效果横跨整个section，不需要拆分当前 section
      if (sectionId === targetSection.id) {
        // 目标 section 实现平滑过渡的效果
        createTransitionBoundaryInSection({
          section,
          laneId: targetLaneId,
          tweenSamplePoints,
          keyPath,
          isExtends: true,
        })
        break
      }
    } else if (leftPercent === pStart && rightPercent !== pEnd) {
      // 在 section 起点的地方开始拓展平滑过渡边界线
      if (sectionId === targetSection.id) {
        section.pEnd = rightPercent
        section.length = fixedPrecision((rightPercent - pStart) * roadLength)
        const boundarysBackup = cloneDeep(section.boundarys)
        const lanesBackup = cloneDeep(section.lanes)
        // 目标 section 前一部分实现平滑过渡的效果
        createTransitionBoundaryInSection({
          section,
          laneId: targetLaneId,
          tweenSamplePoints,
          keyPath,
          isExtends: true,
        })

        // 新增后一部分
        const newSection: biz.ISection = {
          id: String(Number(sectionId) + 1),
          length: fixedPrecision((pEnd - rightPercent) * roadLength),
          lanes: lanesBackup,
          boundarys: boundarysBackup,
          pStart: rightPercent,
          pEnd,
        }
        // 将新增的 section 插入数组，并且递增 i 跳过这个新增的 section
        road.sections.splice(i + 1, 0, newSection)
        i++
        // 更新后一部分 section 的边界线采样点
        updateLaneBoundaryInSection({
          keyPath,
          section: newSection,
          updateBoundaryId: true, // 重置车道边界线的 id
        })
      } else if (Number(sectionId) > Number(targetSection.id)) {
        // 后续 section 需要 id 递增
        section.id = String(Number(sectionId) + 1)
      }
    } else if (leftPercent !== pStart && rightPercent === pEnd) {
      // 在 section 截止的地方，拓展平滑过渡边界线
      if (sectionId === targetSection.id) {
        section.pEnd = leftPercent
        section.length = fixedPrecision((leftPercent - pStart) * roadLength)
        const boundarysBackup = cloneDeep(section.boundarys)
        const lanesBackup = cloneDeep(section.lanes)
        // 更新前一部分常规的车道，并删除目标 id 的车道
        removeLaneInSection({
          section,
          keyPath,
          laneId: targetLaneId,
        })
        updateLaneBoundaryInSection({
          section,
          keyPath,
        })
        // 后一部分为新增的平滑过渡部分
        const newSection: biz.ISection = {
          id: String(Number(sectionId) + 1),
          length: fixedPrecision((pEnd - leftPercent) * roadLength),
          lanes: lanesBackup,
          boundarys: boundarysBackup,
          pStart: leftPercent,
          pEnd,
        }
        road.sections.splice(i + 1, 0, newSection)
        i++
        createTransitionBoundaryInSection({
          section: newSection,
          laneId: targetLaneId,
          keyPath,
          tweenSamplePoints,
          isExtends: true,
          updateBoundaryId: true,
        })
      } else if (Number(sectionId) > Number(targetSection.id)) {
        section.id = String(Number(sectionId) + 1)
      }
    } else {
      // 更常见的情况
      if (sectionId === targetSection.id) {
        // 左侧截断点前的 section，需要删除目标车道同时调整对应边界线的采样点
        section.pEnd = leftPercent
        section.length = fixedPrecision((leftPercent - pStart) * roadLength)

        const boundarysBackupForMiddle = cloneDeep(section.boundarys)
        const lanesBackupForMiddle = cloneDeep(section.lanes)
        const boundarysBackupForBack = cloneDeep(section.boundarys)
        const lanesBackupForBack = cloneDeep(section.lanes)

        removeLaneInSection({
          section,
          laneId: targetLaneId,
          keyPath,
        })
        updateLaneBoundaryInSection({
          section,
          keyPath,
        })
        // 左右截断点之间的 section，需要转换成平滑过渡的边界线
        const middleSection: biz.ISection = {
          id: String(Number(sectionId) + 1),
          length: fixedPrecision((rightPercent - leftPercent) * roadLength),
          lanes: lanesBackupForMiddle,
          boundarys: boundarysBackupForMiddle,
          pStart: leftPercent,
          pEnd: rightPercent,
        }
        road.sections.splice(i + 1, 0, middleSection)
        i++
        createTransitionBoundaryInSection({
          section: middleSection,
          laneId: targetLaneId,
          tweenSamplePoints,
          keyPath,
          updateBoundaryId: true,
          isExtends: true,
        })
        // 右侧截断点之后的 section，需要调整边界线的采样点
        const backSection: biz.ISection = {
          id: String(Number(sectionId) + 2),
          length: fixedPrecision((pEnd - rightPercent) * roadLength),
          lanes: lanesBackupForBack,
          boundarys: boundarysBackupForBack,
          pStart: rightPercent,
          pEnd,
        }
        road.sections.splice(i + 2, 0, backSection)
        i++
        updateLaneBoundaryInSection({
          section: backSection,
          keyPath,
          updateBoundaryId: true,
        })
      } else if (Number(sectionId) > Number(targetSection.id)) {
        // 目标 section 后续的 section，id 需要递增 +2
        section.id = String(Number(section) + 2)
      }
    }
  }
}
/**
 * 从 n 车道变窄成 n-1 车道，并划分 section
 * @param params
 */
export function divideSectionWithNarrowTween (params: IDivideSectionWithTween) {
  const {
    road,
    section: targetSection,
    laneId: targetLaneId,
    leftIndex,
    rightIndex,
    tweenSamplePoints,
  } = params
  // 先判断车道之间是否在数量上有截断
  const { head: headBlockIndex, tail: tailBlockIndex } =
    judgeTargetLaneBlockIndex(road, targetSection.id, targetLaneId)
  const { keyPath, length: roadLength } = road
  const { pStart, pEnd } = targetSection
  const [_leftIndex, totalIndex] = leftIndex
    .split('/')
    .map(num => Number(num))
  const [_rightIndex] = rightIndex.split('/').map(num => Number(num))
  const totalSegment = totalIndex - 1
  const delta = (pEnd - pStart) / totalSegment
  // 右侧边界截断点在前
  const rightPercent = fixedPrecision(
    pStart + delta * _rightIndex,
    Constant.offsetPointsSamplePrecision,
  )
  // 左侧边界截断点在前
  const leftPercent = fixedPrecision(
    pStart + delta * _leftIndex,
    Constant.offsetPointsSamplePrecision,
  )
  for (let i = 0; i < road.sections.length; i++) {
    // 对于截断区域之外的 section 不处理
    if (headBlockIndex !== -1 && i <= headBlockIndex) continue
    if (tailBlockIndex !== -1 && i >= tailBlockIndex) continue

    const section = road.sections[i]
    const { id: sectionId } = section

    if (rightPercent === pStart && leftPercent === pEnd) {
      // 平滑过渡效果横跨整个section，不需要拆分当前 section
      if (sectionId === targetSection.id) {
        // 目标 section 实现平滑过渡的效果
        createTransitionBoundaryInSection({
          section,
          laneId: targetLaneId,
          tweenSamplePoints,
          keyPath,
          isExtends: false,
        })
        break
      }
    } else if (rightPercent === pStart && leftPercent !== pEnd) {
      // 在 section 起点的地方开始有变窄趋势的平滑过渡边界线
      if (sectionId === targetSection.id) {
        section.pEnd = leftPercent
        section.length = fixedPrecision((leftPercent - pStart) * roadLength)
        const boundarysBackup = cloneDeep(section.boundarys)
        const lanesBackup = cloneDeep(section.lanes)
        // 目标 section 前一部分实现平滑过渡的效果
        createTransitionBoundaryInSection({
          section,
          laneId: targetLaneId,
          tweenSamplePoints,
          keyPath,
          isExtends: false,
        })

        // 新增后一部分
        const newSection: biz.ISection = {
          id: String(Number(sectionId) + 1),
          length: fixedPrecision((pEnd - leftPercent) * roadLength),
          lanes: lanesBackup,
          boundarys: boundarysBackup,
          pStart: leftPercent,
          pEnd,
        }
        // 将新增的 section 插入数组，并且递增 i 跳过这个新增的 section
        road.sections.splice(i + 1, 0, newSection)
        i++
        // 由于是车道变窄的趋势，后续的车道需要删除目标 id 的车道
        removeLaneInSection({
          section: newSection,
          laneId: targetLaneId,
          keyPath,
        })
        // 更新后一部分 section 的边界线采样点
        updateLaneBoundaryInSection({
          keyPath,
          section: newSection,
          updateBoundaryId: true, // 重置车道边界线的 id
        })
      } else if (Number(sectionId) > Number(targetSection.id)) {
        // 后续 section 需要 id 递增
        section.id = String(Number(sectionId) + 1)
        // 并且删除目标 id 的车道
        removeLaneInSection({
          section,
          laneId: targetLaneId,
          keyPath,
        })
        // 删除 section 中的 lane，需要手动更新车道中心线的采样点
        updateLaneCenterPoints({
          lanes: section.lanes,
          boundarys: section.boundarys,
        })
      }
    } else if (rightPercent !== pStart && leftPercent === pEnd) {
      // 在 section 截止的地方，车道存在变窄的趋势，对应边界线为平滑过渡效果
      if (sectionId === targetSection.id) {
        section.pEnd = rightPercent
        section.length = fixedPrecision((rightPercent - pStart) * roadLength)
        const boundarysBackup = cloneDeep(section.boundarys)
        const lanesBackup = cloneDeep(section.lanes)
        // 更新前一部分常规的车道
        updateLaneBoundaryInSection({
          section,
          keyPath,
        })
        // 后一部分为新增的平滑过渡部分
        const newSection: biz.ISection = {
          id: String(Number(sectionId) + 1),
          length: fixedPrecision((pEnd - rightPercent) * roadLength),
          lanes: lanesBackup,
          boundarys: boundarysBackup,
          pStart: rightPercent,
          pEnd,
        }
        road.sections.splice(i + 1, 0, newSection)
        i++
        createTransitionBoundaryInSection({
          section: newSection,
          laneId: targetLaneId,
          keyPath,
          tweenSamplePoints,
          isExtends: false,
          updateBoundaryId: true,
        })
      } else if (Number(sectionId) > Number(targetSection.id)) {
        section.id = String(Number(sectionId) + 1)
        // 后续的section 删除目标 id 的车道
        removeLaneInSection({
          section,
          laneId: targetLaneId,
          keyPath,
        })
        // 删除 section 中的 lane，需要手动更新车道中心线的采样点
        updateLaneCenterPoints({
          lanes: section.lanes,
          boundarys: section.boundarys,
        })
      }
    } else {
      // 更常见的情况
      if (sectionId === targetSection.id) {
        // 前一部分为右边界截断点划分的常规section
        section.pEnd = rightPercent
        section.length = fixedPrecision((rightPercent - pStart) * roadLength)
        const boundarysBackupForMiddle = cloneDeep(section.boundarys)
        const lanesBackupForMiddle = cloneDeep(section.lanes)
        const boundarysBackupForBack = cloneDeep(section.boundarys)
        const lanesBackupForBack = cloneDeep(section.lanes)
        updateLaneBoundaryInSection({
          section,
          keyPath,
        })
        // 中间部分为右边界截断点和左边界截断点一起划分的平滑过渡区域，呈变窄趋势
        const middleSection: biz.ISection = {
          id: String(Number(sectionId) + 1),
          length: fixedPrecision((leftPercent - rightPercent) * roadLength),
          lanes: lanesBackupForMiddle,
          boundarys: boundarysBackupForMiddle,
          pStart: rightPercent,
          pEnd: leftPercent,
        }
        road.sections.splice(i + 1, 0, middleSection)
        i++
        createTransitionBoundaryInSection({
          section: middleSection,
          laneId: targetLaneId,
          keyPath,
          tweenSamplePoints,
          isExtends: false,
          updateBoundaryId: true,
        })
        // 后一部分为左边界截断点划分的常规section，需要删除目标 id 的车道
        const backSection: biz.ISection = {
          id: String(Number(sectionId) + 2),
          length: fixedPrecision((pEnd - leftPercent) * roadLength),
          lanes: lanesBackupForBack,
          boundarys: boundarysBackupForBack,
          pStart: leftPercent,
          pEnd,
        }
        road.sections.splice(i + 2, 0, backSection)
        i++
        removeLaneInSection({
          section: backSection,
          laneId: targetLaneId,
          keyPath,
        })
        updateLaneBoundaryInSection({
          section: backSection,
          keyPath,
          updateBoundaryId: true,
        })
      } else if (Number(sectionId) > Number(targetSection.id)) {
        // 后续的 section id 需要递增 +2
        section.id = String(Number(sectionId) + 2)
        // 后续的section 删除目标 id 的车道
        removeLaneInSection({
          section,
          laneId: targetLaneId,
          keyPath,
        })
        // 删除 section 中的 lane，需要手动更新车道中心线的采样点
        updateLaneCenterPoints({
          lanes: section.lanes,
          boundarys: section.boundarys,
        })
      }
    }
  }
}
