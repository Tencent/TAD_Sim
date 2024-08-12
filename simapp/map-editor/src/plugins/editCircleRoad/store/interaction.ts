import { defineStore } from 'pinia'
import { getRoad } from '@/utils/mapCache'
import { registryStore } from '@/utils/tools'

interface IState {
  roadId: string
  timestamp: number
}

const storeName = 'circleRoadInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useCircleRoadInteraction = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    currentRoad: (state) => {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.roadId) return null
      const road = getRoad(state.roadId)
      if (!road) return null

      return road
    },
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const { roadId } = lastState
      this.roadId = roadId
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 选中道路
    selectRoad (roadId: string) {
      if (this.roadId === roadId) return
      this.roadId = roadId
    },
    // 清空道路元素的选中状态
    unselectRoad () {
      if (this.roadId) {
        const tempRoadId = this.roadId
        this.roadId = ''
        return tempRoadId
      }

      this.roadId = ''
      return ''
    },
  },
})
