import { defineStore } from 'pinia'
import { useJunctionStore } from '@/stores/junction'
import { getJunction } from '@/utils/mapCache'
import { registryStore } from '@/utils/tools'

interface IState {
  roadEndEdges: Array<string>
  junctionId: string
  timestamp: number
}

const storeName = 'junctionInteraction'
function createInitValue (): IState {
  return {
    roadEndEdges: [],
    junctionId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useJunctionInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    currentJunction: (state) => {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.junctionId) return null
      const junction = getJunction(state.junctionId)
      if (!junction) return null

      return junction
    },
    judgeExist: (state) => {
      return (endEdge: string) => {
        return state.roadEndEdges.includes(endEdge)
      }
    },
    // 当前是否允许执行删除操作
    enableRemove: (state) => {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      if (state.junctionId) return true
      return false
    },
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const { roadEndEdges, junctionId } = lastState
      this.roadEndEdges = roadEndEdges
      this.junctionId = junctionId
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 删除当前选中的路口数据
    removeCurrentSelectedJunction (junctionId: string = '') {
      const removeJunctionId = junctionId || this.junctionId
      if (!removeJunctionId) return null

      const junctionStore = useJunctionStore()
      const removeJunctionRes = junctionStore.removeJunction(removeJunctionId)
      return removeJunctionRes
    },
    selectRoadEndEdge (endEdge: string | Array<string>) {
      if (Array.isArray(endEdge)) {
        this.roadEndEdges.push(...endEdge)
      } else {
        this.roadEndEdges.push(endEdge)
      }
    },
    unselectRoadEndEdge (endEdge: string) {
      const index = this.roadEndEdges.findIndex(
        _endEdge => _endEdge === endEdge,
      )
      if (index > -1) {
        const _linkRoad = this.roadEndEdges.splice(index, 1)
        return _linkRoad
      }
      return ''
    },
    clearRoadEndEdge () {
      const lastRoadEndEdges = [...this.roadEndEdges]
      this.roadEndEdges.length = 0
      return lastRoadEndEdges
    },
    // 路口相关
    selectJunction (junctionId: string) {
      if (this.junctionId === junctionId) return
      this.junctionId = junctionId

      const junctionStore = useJunctionStore()
      const junction = junctionStore.getJunctionById(junctionId)
      if (!junction) return
      // 选中路口后，自动将选中的道路端面进行同步
      this.roadEndEdges = [...junction.linkRoads]
    },
    unselectJunction () {
      // 取消选中路口，会默认取消所有选中的道路首尾端面
      this.clearRoadEndEdge()
      const lastJunctionId = this.junctionId
      this.junctionId = ''
      return lastJunctionId
    },
  },
})
