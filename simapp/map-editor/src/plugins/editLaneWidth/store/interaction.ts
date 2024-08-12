import { defineStore } from 'pinia'
import { useRoadStore } from '@/stores/road'
import { registryStore } from '@/utils/tools'

interface IState {
  roadId: string
  sectionId: string
  laneId: string
  // 每次更新车道宽度，都更新时间戳，保证撤销和重做能够 diff 到差异执行 applyState 方法
  timestamp: number
}

interface IQuerySection {
  roadId: string
  sectionId: string
}
interface IQueryLane extends IQuerySection {
  laneId: string
}

const storeName = 'laneWidthInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    sectionId: '',
    laneId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useLaneWidthInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    selectedLaneInfo: (state) => {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.roadId || !state.sectionId || !state.laneId) return null
      const roadStore = useRoadStore()
      const queryLaneRes = roadStore.getLaneById(
        state.roadId,
        state.sectionId,
        state.laneId,
      )
      if (!queryLaneRes) return null

      return queryLaneRes
    },
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const { roadId, sectionId, laneId } = lastState
      this.roadId = roadId
      this.sectionId = sectionId
      this.laneId = laneId
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
