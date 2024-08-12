import { defineStore } from 'pinia'
import { getObject } from '@/utils/mapCache'
import { registryStore } from '@/utils/tools'
import { applyDefaultControlJunctionAndRoad } from '@/stores/object/common'

interface IState {
  trafficLightId: string // 当前选中的信号灯的 id
  canSelectRoad: boolean // 当前是否能选中道路
  canSelectJunction: boolean // 当前是否能选中路口
  timestamp: number
}

const storeName = 'signalControlInteraction'
function createInitValue (): IState {
  return {
    trafficLightId: '',
    canSelectRoad: false,
    canSelectJunction: false,
    timestamp: 0,
  }
}

// 本地缓存的可交互元素的标识，方便通过变量 this[`unselect${flag}`] 调用自身 action
const cacheElementFlags = ['TrafficLight']

registryStore(storeName, createInitValue)

export const useSignalControlInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    currentTrafficLight (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.trafficLightId) return null
      const trafficLight = getObject(state.trafficLightId) as biz.ITrafficLight
      if (!trafficLight) return null
      return trafficLight
    },
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const { trafficLightId } = lastState
      this.canSelectJunction = false
      this.canSelectRoad = false
      this.trafficLightId = trafficLightId
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 是否支持点选道路和路口功能的可用性
    setSelectRoadEnabled (enabled: boolean) {
      this.canSelectRoad = enabled
      if (enabled) {
        // 道路和路口的选择交互不同时可用
        this.canSelectJunction = false
      }
    },
    setSelectJunctionEnabled (enabled: boolean) {
      this.canSelectJunction = enabled
      if (enabled) {
        // 道路和路口的选择交互不同时可用
        this.canSelectRoad = false
      }
    },
    // 交通信号灯
    selectTrafficLight (trafficLightId: string, force: boolean = false) {
      // 如果是强制选中，则必须执行后续选中信号灯的渲染层逻辑
      if (!force && this.trafficLightId === trafficLightId) return ''
      const tempId = this.trafficLightId
      this.trafficLightId = trafficLightId

      // 如果信号灯没有信控配置，自动配置控制的路口和道路
      applyDefaultControlJunctionAndRoad(trafficLightId)

      return tempId
    },
    unselectTrafficLight () {
      this.setSelectRoadEnabled(false)
      this.setSelectJunctionEnabled(false)

      if (this.trafficLightId) {
        const tempId = this.trafficLightId
        this.trafficLightId = ''
        return tempId
      }

      this.trafficLightId = ''
      return ''
    },
  },
})
