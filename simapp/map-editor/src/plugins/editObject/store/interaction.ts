import { defineStore } from 'pinia'
import { useObjectStore } from '@/stores/object'
import { getObject } from '@/utils/mapCache'
import { registryStore } from '@/utils/tools'

interface IState {
  poleId: string
  roadSignId: string
  trafficLightId: string
  signalBoardId: string
  sensorId: string
  otherId: string
  customModelId: string
  timestamp: number
  canSelectJunction: boolean // 当前是否能选中 rsu 关联的路口
  canSelectRsu: boolean // 当前是否能选中传感器关联的 rsu
}

const storeName = 'objectInteraction'
function createInitValue (): IState {
  return {
    poleId: '',
    roadSignId: '',
    trafficLightId: '',
    signalBoardId: '',
    sensorId: '',
    otherId: '',
    customModelId: '',
    timestamp: 0,
    canSelectJunction: false,
    canSelectRsu: false,
  }
}

// 本地缓存的可交互元素的标识，方便通过变量 this[`unselect${flag}`] 调用自身 action
const cacheElementFlags = [
  'Pole',
  'RoadSign',
  'TrafficLight',
  'SignalBoard',
  'Sensor',
  'Other',
  'CustomModel',
]

registryStore(storeName, createInitValue)

export const useObjectInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    currentPole (state) {
      // 只是为了在 getter 中拿到最新的物体状态，无实际意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.poleId) return null
      const object = getObject(state.poleId) as biz.IPole
      if (!object) return null
      return object
    },
    currentRoadSign (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.roadSignId) return null
      const object = getObject(state.roadSignId) as biz.IRoadSign
      if (!object) return null
      if (object.mainType !== 'roadSign') return null
      return object
    },
    currentParkingSpace (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.roadSignId) return null
      const object = getObject(state.roadSignId) as biz.IParkingSpace
      if (!object) return null
      if (object.mainType !== 'parkingSpace') return null
      return object
    },
    currentSignalBoard (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.signalBoardId) return null
      const object = getObject(state.signalBoardId) as biz.ISignalBoard
      if (!object) return null
      return object
    },
    currentTrafficLight (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.trafficLightId) return null
      const object = getObject(state.trafficLightId) as biz.ITrafficLight
      if (!object) return null
      return object
    },
    currentSensor (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.sensorId) return null
      const object = getObject(state.sensorId) as biz.ISensor
      if (!object) return null
      return object
    },
    currentOther (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.otherId) return null
      const object = getObject(state.otherId) as biz.IOther
      if (!object) return null
      return object
    },
    currentCustomModel (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.customModelId) return null
      const object = getObject(state.customModelId) as biz.ICustomModel
      if (!object) return null
      return object
    },
    currentObject (state) {
      // 无意义
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      const objectId =
        state.poleId ||
        state.roadSignId ||
        state.signalBoardId ||
        state.trafficLightId ||
        state.sensorId ||
        state.otherId ||
        state.customModelId
      if (!objectId) return null
      const object = getObject(objectId)
      if (!object) return null
      return object
    },
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const {
        poleId,
        roadSignId,
        trafficLightId,
        signalBoardId,
        sensorId,
        otherId,
        customModelId,
      } = lastState

      this.poleId = poleId
      this.roadSignId = roadSignId
      this.trafficLightId = trafficLightId
      this.signalBoardId = signalBoardId
      this.sensorId = sensorId
      this.otherId = otherId
      this.customModelId = customModelId

      // 在应用状态的时候，取消关联元素选取的可用性
      this.setSelectJunctionEnabled(false)
      this.setSelectRsuEnabled(false)
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 是否支持点选关联路口的可用性
    setSelectJunctionEnabled (enabled: boolean) {
      this.canSelectJunction = enabled
      if (enabled) {
        // 选中路口和选中 rsu 不能同时可用
        this.canSelectRsu = false
      }
    },
    setSelectRsuEnabled (enabled: boolean) {
      this.canSelectRsu = enabled
      if (enabled) {
        this.canSelectJunction = false
      }
    },
    // 删除当前选中的元素（针对不同元素的类型，调用不同的方法）
    removeCurrentSelected () {
      const objectStore = useObjectStore()
      // 先获取当前选中的元素 id
      const {
        poleId,
        roadSignId,
        signalBoardId,
        trafficLightId,
        sensorId,
        otherId,
        customModelId,
      } = this

      if (poleId) {
        objectStore.removePole(poleId, true)
      } else if (roadSignId) {
        objectStore.removeRoadSign(roadSignId, true)
      } else if (signalBoardId) {
        objectStore.removeSignalBoard(signalBoardId, true)
      } else if (trafficLightId) {
        objectStore.removeTrafficLight(trafficLightId, true)
      } else if (sensorId) {
        objectStore.removeSensor(sensorId, true)
      } else if (otherId) {
        objectStore.removeOther(otherId, true)
      } else if (customModelId) {
        objectStore.removeCustomModel(customModelId, true)
      }

      // 取消选中
      this.unselectExceptSomeone()
    },
    // 杆
    selectPole (poleId: string) {
      this.unselectExceptSomeone('Pole')

      if (this.poleId === poleId) return ''

      const tempId = this.poleId
      this.poleId = poleId

      return tempId
    },
    unselectPole () {
      if (this.poleId) {
        const tempId = this.poleId
        this.poleId = ''
        return tempId
      }

      this.poleId = ''
      return ''
    },
    // 路面标识
    selectRoadSign (roadSignId: string) {
      this.unselectExceptSomeone('RoadSign')

      // 上一次选中的路面标线
      const tempId = this.roadSignId
      this.roadSignId = roadSignId

      return tempId
    },
    unselectRoadSign () {
      if (this.roadSignId) {
        const tempId = this.roadSignId
        this.roadSignId = ''
        return tempId
      }

      this.roadSignId = ''
      return ''
    },
    // 交通信号灯
    selectTrafficLight (trafficLightId: string) {
      this.unselectExceptSomeone('TrafficLight')

      if (this.trafficLightId === trafficLightId) return ''

      // 上一次选中的信号灯
      const tempId = this.trafficLightId
      this.trafficLightId = trafficLightId

      return tempId
    },
    unselectTrafficLight () {
      if (this.trafficLightId) {
        const tempId = this.trafficLightId
        this.trafficLightId = ''
        return tempId
      }

      this.trafficLightId = ''
      return ''
    },
    // 信号标志牌
    selectSignalBoard (signalBoardId: string) {
      this.unselectExceptSomeone('SignalBoard')

      if (this.signalBoardId === signalBoardId) return ''

      // 上一次选中的标志牌
      const tempId = this.signalBoardId
      this.signalBoardId = signalBoardId

      return tempId
    },
    unselectSignalBoard () {
      if (this.signalBoardId) {
        const tempId = this.signalBoardId
        this.signalBoardId = ''
        return tempId
      }

      this.signalBoardId = ''
      return ''
    },
    // 路侧传感器
    selectSensor (sensorId: string) {
      this.unselectExceptSomeone('Sensor')

      if (this.sensorId === sensorId) return ''

      // 上一次选中的传感器
      const tempId = this.sensorId
      this.sensorId = sensorId

      return tempId
    },
    unselectSensor () {
      if (this.sensorId) {
        const tempId = this.sensorId
        this.sensorId = ''
        return tempId
      }

      this.sensorId = ''
      return ''
    },
    // 其他类型
    selectOther (otherId: string) {
      this.unselectExceptSomeone('Other')

      if (this.otherId === otherId) return ''

      const tempId = this.otherId
      this.otherId = otherId
      return tempId
    },
    unselectOther () {
      if (this.otherId) {
        const tempId = this.otherId
        this.otherId = ''
        return tempId
      }

      this.otherId = ''
      return ''
    },
    // 自定义模型
    selectCustomModel (modelId: string) {
      this.unselectExceptSomeone('CustomModel')

      if (this.customModelId === modelId) return ''
      const tempId = this.customModelId
      this.customModelId = modelId
      return tempId
    },
    unselectCustomModel () {
      if (this.customModelId) {
        const tempId = this.customModelId
        this.customModelId = ''
        return tempId
      }

      this.customModelId = ''
      return ''
    },
    /**
     * 取消其他元素的选中状态，可以提供某一个元素除外
     * @param name
     */
    unselectExceptSomeone (name?: string) {
      // 取消任何元素的状态都将取消三维场景中路口和 rsu 可选项
      this.setSelectJunctionEnabled(false)
      this.setSelectRsuEnabled(false)

      cacheElementFlags.forEach((flag) => {
        if (name && name === flag) return
        const actionName = `unselect${flag}`
        // @ts-expect-error
        this[actionName]()
      })
    },
  },
})
