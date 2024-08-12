// section 编辑模式辅助元素自身状态
import { defineStore } from 'pinia'
import type { Vector3 } from 'three'
import { useSectionInteractionStore } from './interaction'
import { useRoadStore } from '@/stores/road'
import {
  Constant,
  getMatchingSegment,
  getParallelCurveSamplePoints,
  getTransitionBoundarySamplePoints,
} from '@/utils/business'
import { findClosestPoint, fixedPrecision } from '@/utils/common3d'
import { registryStore } from '@/utils/tools'

interface IState {
  roadId: string
  sectionId: string
  laneId: string
  // 当前选中车道左右两侧的边界线采样点
  leftPoints: Array<Vector3>
  rightPoints: Array<Vector3>
  // 平滑过渡曲线的补间控制点，实际上属于 vec3WithId
  // tweenControlPoints: Array<common.vec3WithId>
  // 平滑过渡曲线两个端点在左右车道边界线上预设截断点的索引，使用 index/total的形式
  leftIndex: string
  rightIndex: string
  // 平滑过渡曲线采样点
  tweenCurveSamplePoints: Array<Vector3>
  // 已完成的道路划分辅助元素状态缓存
  completed: Array<any>
}

const storeName = 'editSection'
function createInitValue (): IState {
  return {
    roadId: '',
    sectionId: '',
    laneId: '',
    leftPoints: [],
    rightPoints: [],
    leftIndex: '',
    rightIndex: '',
    // tweenControlPoints: [],
    tweenCurveSamplePoints: [],
    completed: [],
  }
}

registryStore(storeName, createInitValue)

// 车道边界线上的预设辅助点的采样间隔，跟section长度计算出来的间隔比例
const presetPointSegmentRatio = 4

export const useEditSectionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {},
  actions: {
    // 应用历史状态
    applyState (lastState: IState, diffState: IState) {},
    /**
     * 选中一条车道，计算对应的辅助元素属性
     * @param roadId
     * @param sectionId
     * @param laneId
     * @returns
     */
    selectLane (roadId: string, sectionId: string, laneId: string) {
      const interactionStore = useSectionInteractionStore()
      // 调用交互元素状态的选中车道方法
      const res = interactionStore.selectLane(roadId, sectionId, laneId)
      // 如果没有属性的更新，直接返回
      if (!res) return

      // 更新当前编辑模式保存的元素 id
      this.roadId = roadId
      this.sectionId = sectionId
      this.laneId = laneId

      // TODO 通过道路中心线做采样间隔来计算偏移的车道边界线上的平均采样点
      // 此时通过 store.getters 是能够拿到更新后的地图元素属性的
      if (!roadId || !sectionId || !laneId) return
      const roadStore = useRoadStore()
      const queryLaneRes = roadStore.getLaneById(roadId, sectionId, laneId)
      if (!queryLaneRes) return
      const {
        road: selectedRoad,
        section: selectedSection,
        lane: selectedLane,
      } = queryLaneRes

      // TODO 需要区分平滑过渡车道的选中效果！！！

      const { keyPath } = selectedRoad
      const { pStart, pEnd } = selectedSection
      // 获取当前 section 预设截断点的数量
      const basicSegment = getMatchingSegment(selectedSection)
      const detailedSegment = basicSegment * presetPointSegmentRatio

      let offset = 0
      let lPoints: Array<Vector3> = []
      let rPoints: Array<Vector3> = []
      // 从内向外遍历选中 section 中的所有车道
      for (const lane of selectedSection.lanes) {
        const { normalWidth, id: laneId } = lane
        offset += normalWidth
        // 特殊情况，如果选中的最内侧的车道
        if (laneId === selectedLane.id && laneId === '-1') {
          const { refPoints, offsetPoints } = getParallelCurveSamplePoints({
            keyPath,
            offset,
            pStart,
            pEnd,
            segment: detailedSegment,
          })
          lPoints = refPoints
          rPoints = offsetPoints
          // 最内侧车道左右车道边界的采样点，即为目标采样点，遍历完就推出循环
          break
        }

        // 如果是选中车道左侧的车道，则获取这个左侧车道的右侧边界采样点
        if (Number(laneId) === Number(selectedLane.id) + 1) {
          const { offsetPoints } = getParallelCurveSamplePoints({
            keyPath,
            offset,
            pStart,
            pEnd,
            segment: detailedSegment,
          })
          lPoints = offsetPoints
        } else if (laneId === selectedLane.id) {
          // 选中车道的右侧边界线采样点
          const { offsetPoints } = getParallelCurveSamplePoints({
            keyPath,
            offset,
            pStart,
            pEnd,
            segment: detailedSegment,
          })
          rPoints = offsetPoints
        }
      }

      this.leftPoints = lPoints
      this.rightPoints = rPoints
    },
    /**
     * 取消车道选中
     */
    unselectLane () {
      const interactionStore = useSectionInteractionStore()
      interactionStore.unselectLane()

      this.roadId = ''
      this.sectionId = ''
      this.laneId = ''
      this.leftPoints = []
      this.rightPoints = []
      this.leftIndex = ''
      this.rightIndex = ''
      // this.tweenControlPoints = []
      this.tweenCurveSamplePoints = []
    },
    // TODO 新增和更新 section 应该都是用这个方法？？
    // 涉及选择车道左右边界线上的预设截断点
    addSection (params: { leftPoint: Vector3, rightPoint: Vector3 }) {
      // 如果都没有选中车道，直接返回
      if (!this.roadId || !this.sectionId || !this.laneId) return

      const { leftPoint, rightPoint } = params
      if (!leftPoint || !rightPoint) return

      // 获取两个截断点在数组中的索引
      // 采样点计算结果误差，在误差范围内的都可视为找到了对应的点
      const { point: leftClosestPoint, index: leftIndex } = findClosestPoint({
        targetPoint: leftPoint,
        points: this.leftPoints,
        newId: true,
      })
      const { point: rightClosestPoint, index: rightIndex } = findClosestPoint({
        targetPoint: rightPoint,
        points: this.rightPoints,
        newId: true,
      })

      if (!leftClosestPoint || !rightClosestPoint) return
      const presetPointsNumber = this.leftPoints.length
      this.leftIndex = `${leftIndex}/${presetPointsNumber}`
      this.rightIndex = `${rightIndex}/${presetPointsNumber}`

      // 调用 roadStore 截断 section 的方法，同时调整平滑过渡车道边界线和车道中心线的采样点
      const roadStore = useRoadStore()
      const queryRes = roadStore.getLaneById(
        this.roadId,
        this.sectionId,
        this.laneId,
      )
      if (!queryRes) return
      const { road, section: targetSection, lane: targetLane } = queryRes
      const { keyPath } = road
      const { pStart, pEnd } = targetSection
      const { normalWidth } = targetLane
      if (leftIndex === rightIndex) {
        if (
          (leftIndex === 0 && rightIndex === 0) ||
          (leftIndex === rightIndex && leftIndex === presetPointsNumber)
        ) {
          // 左右边界索引为 section 的起始点或者截止点，视为没有做任何处理
        } else {
          // 如果索引相同且不是首尾的情况，视为单纯截断 section
          roadStore.addSectionOnly({
            roadId: this.roadId,
            sectionId: this.sectionId,
            laneId: this.laneId,
            tweenSamplePoints: [],
            leftIndex: this.leftIndex,
            rightIndex: this.rightIndex,
          })
        }
      } else {
        // 是否是拓展车道的趋势
        const isExtends = leftIndex < rightIndex

        // 计算截断点在整个道路中心线的占比
        const totalSegment = presetPointsNumber - 1
        const delta = (pEnd - pStart) / totalSegment
        const leftPercent = fixedPrecision(
          pStart + delta * leftIndex,
          Constant.offsetPointsSamplePrecision,
        )
        const rightPercent = fixedPrecision(
          pStart + delta * rightIndex,
          Constant.offsetPointsSamplePrecision,
        )
        let _pStart, _pEnd
        if (isExtends) {
          if (leftIndex === 0 && rightIndex === presetPointsNumber) {
            _pStart = pStart
            _pEnd = pEnd
          } else if (leftIndex === 0 && rightIndex !== presetPointsNumber) {
            _pStart = pStart
            _pEnd = rightPercent
          } else if (leftIndex !== 0 && rightIndex === presetPointsNumber) {
            _pStart = leftPercent
            _pEnd = pEnd
          } else {
            _pStart = leftPercent
            _pEnd = rightPercent
          }
        } else {
          if (rightIndex === 0 && leftIndex === presetPointsNumber) {
            _pStart = pStart
            _pEnd = pEnd
          } else if (rightIndex === 0 && leftIndex !== presetPointsNumber) {
            _pStart = pStart
            _pEnd = leftPercent
          } else if (rightIndex !== 0 && leftIndex === presetPointsNumber) {
            _pStart = rightPercent
            _pEnd = pEnd
          } else {
            _pStart = rightPercent
            _pEnd = leftPercent
          }
        }

        // TODO 控制点逻辑优化，基于车道中心线的采样点做不等差的偏移量采样点计算
        // 遍历目标 section 获取最小和最大的偏移量
        let minOffset = 0
        let maxOffset = 0
        for (const lane of targetSection.lanes) {
          if (Math.abs(Number(lane.id)) < Math.abs(Number(targetLane.id))) {
            // 目标车道内侧的车道，叠加偏移量
            minOffset += lane.normalWidth
            maxOffset += lane.normalWidth
          } else if (lane.id === targetLane.id) {
            maxOffset += lane.normalWidth
            // 遍历到目标车道即可
            break
          }
        }
        // 获取平滑过车边界线的采样点
        const { transitionPoints, controlPoints } =
          getTransitionBoundarySamplePoints({
            keyPath,
            maxOffset,
            minOffset,
            pStart: _pStart,
            pEnd: _pEnd,
            leftPoint,
            rightPoint,
            isExtends,
          })
        this.tweenCurveSamplePoints = transitionPoints

        // 将 section 一分为三，同时对车道边界做平滑过渡处理
        roadStore.addSectionWithTween({
          roadId: this.roadId,
          sectionId: this.sectionId,
          laneId: this.laneId,
          tweenSamplePoints: transitionPoints,
          rightIndex: this.rightIndex,
          leftIndex: this.leftIndex,
        })
      }

      // 添加完 section 后，取消选中状态
      this.unselectLane()
    },
    // TODO 【低优】更新平滑过渡补间控制点的位置
    updateTweenControlPoint () {},
  },
})
