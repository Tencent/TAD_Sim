import { defineStore } from 'pinia'
import { registryStore } from '@/utils/tools'
import { getRoad } from '@/utils/mapCache'

interface IState {
  roadId: string
  timestamp: number
}

const storeName = 'elevationInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

// 地图元素交互状态【该模式下为高程的交互状态】
export const useElevationInteractionStore = defineStore(storeName, {
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
    // 道路相关
    selectRoad (roadId: string) {
      if (this.roadId === roadId) return
      this.roadId = roadId
    },
    // 清空道路元素的选中状态
    unselectRoad () {
      if (this.roadId) {
        const tempId = this.roadId
        this.roadId = ''
        return tempId
      }

      this.roadId = ''
      return ''
    },
  },
})
