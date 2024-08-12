import { defineStore } from 'pinia'
import { registryStore } from '@/utils/tools'
import { useRoadStore } from '@/stores/road'

interface IState {
  roadId: string
  sectionId: string
  laneId: string
  timestamp: number
}

interface IQuerySection {
  roadId: string
  sectionId: string
}
interface IQueryLane extends IQuerySection {
  laneId: string
}

const storeName = 'laneNumberInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    sectionId: '',
    laneId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useLaneNumberInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    // 当前是否允许执行删除操作
    enableRemove (state) {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      if (state.roadId && state.sectionId && state.laneId) return true
      return false
    },
  },
  actions: {
    // 应用操作记录中得状态
    applyState (lastState: IState, diffState: IState) {
      const { roadId, sectionId, laneId } = lastState
      this.roadId = roadId
      this.sectionId = sectionId
      this.laneId = laneId
    },
    /**
     * 删除当前选中的车道
     * @param params
     * @returns
     */
    async removeSelectedLane () {
      if (!this.roadId || !this.sectionId || !this.laneId) return
      const roadStore = useRoadStore()
      await roadStore.removeLane({
        roadId: this.roadId,
        sectionId: this.sectionId,
        laneId: this.laneId,
      })
      // 在删除车道后，取消交互选中的状态
      this.unselectLane()
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 车道级别
    selectLane (params: IQueryLane) {
      const { roadId, sectionId, laneId } = params
      if (
        this.roadId === roadId &&
        this.sectionId === sectionId &&
        this.laneId === laneId
      ) {
        return false
      }

      const lastRoadId = this.roadId
      const lastSectionId = this.sectionId
      const lastLaneId = this.laneId
      this.roadId = roadId
      this.sectionId = sectionId
      this.laneId = laneId
      return {
        roadId: lastRoadId,
        sectionId: lastSectionId,
        laneId: lastLaneId,
      }
    },
    unselectLane () {
      const lastRoadId = this.roadId
      const lastSectionId = this.sectionId
      const lastLaneId = this.laneId
      this.roadId = ''
      this.sectionId = ''
      this.laneId = ''
      return {
        roadId: lastRoadId,
        sectionId: lastSectionId,
        laneId: lastLaneId,
      }
    },
    // section 级别
    selectSection (params: IQuerySection) {
      const { roadId, sectionId } = params
      if (this.roadId === roadId && this.sectionId === sectionId) return null
      const lastRoadId = this.roadId
      const lastSectionId = this.sectionId
      this.roadId = roadId
      this.sectionId = sectionId
      return {
        roadId: lastRoadId,
        sectionId: lastSectionId,
      }
    },
    unselectSection () {
      const lastRoadId = this.roadId
      const lastSectionId = this.sectionId
      this.roadId = ''
      this.sectionId = ''
      return {
        roadId: lastRoadId,
        sectionId: lastSectionId,
      }
    },
    unselectAll () {
      if (this.laneId) {
        this.unselectLane()
      } else {
        this.unselectSection()
      }
    },
  },
})
