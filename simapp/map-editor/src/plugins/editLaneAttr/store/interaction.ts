import { defineStore } from 'pinia'
import { registryStore } from '@/utils/tools'
import { useRoadStore } from '@/stores/road'
import { getRoad } from '@/utils/mapCache'

interface IState {
  roadId: string
  sectionId: string
  laneId: string
  timestamp: number
}

interface IQueryLane {
  roadId: string
  sectionId: string
  laneId: string
}

const storeName = 'laneAttrInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    sectionId: '',
    laneId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useLaneAttrInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    currentRoad (state) {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      if (!state.roadId) return null
      const road = getRoad(state.roadId)
      if (!road) return null
      return road
    },
    currentSection (state) {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      if (!this.currentRoad) return
      if (!state.sectionId) return
      for (const section of this.currentRoad.sections) {
        if (section.id === state.sectionId) {
          return section
        }
      }
      return null
    },
    currentLane (state) {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      if (!this.currentRoad || !this.currentSection) return
      if (!state.laneId) return
      for (const lane of this.currentSection.lanes) {
        if (lane.id === state.laneId) {
          return lane
        }
      }
      return null
    },
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
    updateTimestamp () {
      this.timestamp = Date.now()
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
    // section 级别
    selectSection (params: { roadId: string, sectionId: string }) {
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
      // 取消 laneId 的选中
      this.laneId = ''
      return {
        roadId: lastRoadId,
        sectionId: lastSectionId,
      }
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
    unselectAll () {
      this.unselectSection()
      this.unselectLane()
    },
  },
})
