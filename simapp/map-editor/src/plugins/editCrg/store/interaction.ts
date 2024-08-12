import { defineStore } from 'pinia'
import { getRoad } from '@/utils/mapCache'
import { registryStore } from '@/utils/tools'

interface IState {
  roadId: string
  // 每次更新车道宽度，都更新时间戳，保证撤销和重做能够 diff 到差异执行 applyState 方法
  timestamp: number
}

const storeName = 'crgInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useCrgInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    selectedRoad (state) {
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
    selectRoad (roadId: string) {
      if (this.roadId === roadId) return
      this.roadId = roadId
    },
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
