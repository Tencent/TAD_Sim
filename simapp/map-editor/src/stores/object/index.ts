import { defineStore } from 'pinia'
import type { Intersection } from 'three'
import { createPole, movePole, movePoleByST, rotatePole } from './pole'
import {
  createRoadSign,
  moveCustomRoadSign,
  moveRoadSign,
  moveRoadSignByST,
  rotateRoadSign,
  updateRoadSignByJunction,
  updateRoadSignByLane,
  updateRoadSignSize,
} from './roadSign'
import {
  createObjectOnPole,
  moveObjectOnPole,
  moveObjectOnPoleByHeightAndDist,
  rotateObjectOnPole,
  toggleDirection,
  updateSensorDeviceParams,
} from './objectOnPole'
import {
  updateTrafficLightControlJunction,
  updateTrafficLightControlRoad,
  updateTrafficLightType,
} from './trafficLight'
import {
  createOther,
  moveOther,
  moveOtherByST,
  rotateOther,
  updateBridgeSpan,
  updateOtherSize,
} from './other'
import { getAllSensorExceptRsu, getObjectOnPole } from './common'
import {
  type ParkingSpacePropertyType,
  copyParkingSpace,
  createParkingSpace,
  moveParkingSpace,
  moveParkingSpaceByST,
  rotateParkingSpace,
  updateParkingSpaceProperty,
} from './parkingSpace'
import type { ICustomModelConfig } from './modelCache'
import {
  createCustomModel,
  moveCustomModel,
  moveCustomModelByST,
  rotateCustomModel,
  updateCustomModelSize,
} from './customModel'
import { useModelCacheStore } from './modelCache'
import { getObject, removeObject, setObject } from '@/utils/mapCache'
import type { IPoleConfig } from '@/config/pole'
import type { IRoadSignConfig } from '@/config/roadSign'
import type { ISignalBoardConfig } from '@/config/signalBoard'
import type { ITrafficLightConfig } from '@/config/trafficLight'
import type { IOtherConfig } from '@/config/other'
import type { IParkingSpaceConfig } from '@/config/parkingSpace'
import { transformArrayObjectToArray } from '@/utils/common3d'
import type { ISensorConfig } from '@/config/sensor'

interface IState {
  ids: Array<string>
  elements: Array<biz.IObject>
}

const storeName = 'object'
function createInitValue (): IState {
  return {
    ids: [],
    elements: [],
  }
}

export const useObjectStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {},
  actions: {
    // 应用地图文件中的物体状态
    /**
     * 将地图文件中解析的物体状态同步到 store 中
     * @param newState
     */
    applyMapFileState (newState: IState) {
      return new Promise((resolve) => {
        const { ids } = newState
        this.ids = ids
        resolve(true)
      })
    },
    // 应用操作记录中的状态
    applyState (params: {
      last: IState
      diff: IState
      lastDiffIds: Array<string>
    }) {
      const { last: lastState, lastDiffIds } = params
      const { ids: lastIds, elements } = lastState
      // 无效的元素 id 集合
      const invalidIds = []
      const modelCacheStore = useModelCacheStore()
      elements &&
      elements.forEach((object) => {
        if (
        // 如果是停车位，需要手动给停车位的 geoAttr 赋值
          object.mainType === 'parkingSpace' ||
          // 如果是通过 geoAttr 创建几何体的特殊路面标线（导向车道线、纵向减速标线）
          object.name === 'Road_Guide_Lane_Line' ||
          object.name === 'Variable_Direction_Lane_Line' ||
          object.name === 'Longitudinal_Deceleration_Marking' ||
          object.name === 'Lateral_Deceleration_Marking' ||
          object.name ===
          'White_Semicircle_Line_Vehicle_Distance_Confirmation' ||
          object.name === 'Intersection_Guide_Line'
        ) {
          ;(object as biz.IParkingSpace).geoAttrs.forEach((geoAttr) => {
            geoAttr.vertices = transformArrayObjectToArray(geoAttr.vertices)
          })
        }

        const { id: objectId, name, mainType } = object
        if (mainType === 'customModel') {
          const customModelConfig =
              modelCacheStore.getCustomModelConfigByName(name)
          if (!customModelConfig) {
            // 如果自定义模型配置无法找到，则说明对应的自定义模型数据需要删除
            invalidIds.push(objectId)
            return
          }
        }

        // 有效的物体元素继续记录在缓存中
        setObject(object.id, object)
      })
      for (const diffId of lastDiffIds) {
        if (!lastState.ids.includes(diffId)) {
          removeObject(diffId)
        }
      }
      // 由于自定义模型配置删除不保存操作记录且不可逆，所有历史操作保留的对应自定义模型的数据需要删除
      this.ids = lastIds.filter(id => !invalidIds.includes(id))
    },
    // 删除简单的物体（指不是容器的物体）
    removeSimpleObject (objectId: string) {
      const objectData = getObject(objectId)
      if (!objectData) return

      // 从记录的 id 集合中删除
      const index = this.ids.findIndex(id => id === objectId)
      if (index > -1) {
        this.ids.splice(index, 1)
      }
      // 从源数据中删除
      removeObject(objectId)

      return objectData
    },
    // 在 object/index 中保留方法名称，主要是为了在 dispatchers 中针对每一个方法调用渲染层
    // ---------- 杆 start ----------
    addPole (params: { option: IPoleConfig, intersection: Intersection }) {
      const pole = createPole(params)
      if (!pole) return
      this.ids.push(pole.id)
      return pole.id
    },
    movePole (params: {
      objectId: string
      point: common.vec3
      roadId?: string
      refLineLocation?: biz.IRefLineLocation
    }) {
      const pole = movePole(params)
      if (!pole) return
      return pole.id
    },
    movePoleByST (params: {
      objectId: string
      s: number
      t: number
      saveRecord?: boolean
    }) {
      const pole = movePoleByST(params)
      if (!pole) return
      return pole.id
    },
    rotatePole (params: { objectId: string, yaw?: number, angle?: number }) {
      const pole = rotatePole(params)
      if (!pole) return
      return pole.id
    },
    removePole (objectId: string, saveRecord: boolean) {
      const poleData = getObject(objectId) as biz.IPole
      if (!poleData) return []

      // 获取杆上物体的集合
      const objectsOnPole = getObjectOnPole({
        poleId: objectId,
        ids: this.ids,
      })

      // 将删除元素的源数据先缓存，供操作记录保存源数据用于状态还原
      const removedObjectOnPole: Array<biz.ISignalBoard | biz.ITrafficLight> =
        []
      // 遍历杆上的物体，先挨个删除
      for (const _object of objectsOnPole) {
        const { mainType } = _object
        if (mainType === 'signalBoard') {
          // 删除杆时，杆上物体联动删除，不重复触发保存操作记录的逻辑
          const removeRes = this.removeSignalBoard(_object.id, false)
          // 如果成功删除杆上的物体，则将杆上物体的源数据缓存
          if (removeRes) {
            removedObjectOnPole.push(removeRes)
          }
        } else if (mainType === 'trafficLight') {
          const removeRes = this.removeTrafficLight(_object.id, false)
          if (removeRes) {
            removedObjectOnPole.push(removeRes)
          }
        } else if (mainType === 'sensor') {
          const removeRes = this.removeSensor(_object.id, false)
          if (removeRes) {
            removedObjectOnPole.push(removeRes)
          }
        }
      }

      // 从记录的 id 集合中删除
      const index = this.ids.findIndex(id => id === objectId)
      if (index > -1) {
        this.ids.splice(index, 1)
      }
      // 从源数据中删除
      removeObject(poleData.id)

      if (!poleData) return []
      // 删除物体，返回的是删除的源数据整体
      return [poleData, ...removedObjectOnPole]
    },
    // ---------- 杆 end ----------
    // ---------- 路面标识 start ----------
    addRoadSign (params: {
      option: IRoadSignConfig
      intersection: Intersection
    }) {
      const roadSign = createRoadSign(params)
      if (!roadSign) return
      this.ids.push(roadSign.id)
      return roadSign.id
    },
    moveRoadSign (params: {
      objectId: string
      point: common.vec3
      roadId?: string
      refLineLocation?: biz.IRefLineLocation
    }) {
      const roadSign = moveRoadSign(params)
      if (!roadSign) return
      return roadSign.id
    },
    moveCustomRoadSign (params: { objectId: string, point: common.vec3 }) {
      const roadSign = moveCustomRoadSign(params)
      if (!roadSign) return
      return roadSign.id
    },
    moveRoadSignByST (params: {
      objectId: string
      s: number
      t: number
      saveRecord?: boolean
    }) {
      const roadSign = moveRoadSignByST(params)
      if (!roadSign) return
      return roadSign.id
    },
    updateRoadSignSize (params: {
      objectId: string
      length: number
      width: number
    }) {
      const roadSign = updateRoadSignSize(params)
      if (!roadSign) return
      return roadSign.id
    },
    // 由车道结构的变化，触发的路面标识位置的更新
    updateRoadSignByLane (params: { objectId: string, type?: string }) {
      const roadSign = updateRoadSignByLane(params)
      if (!roadSign) return
      return roadSign
    },
    // 由于路口结构变化，触发路口中的特殊路面标线的更新
    updateRoadSignByJunction (objectId: string) {
      const roadSign = updateRoadSignByJunction(objectId)
      if (!roadSign) return
      return roadSign
    },
    rotateRoadSign (params: { objectId: string, yaw?: number, angle?: number }) {
      const roadSign = rotateRoadSign(params)
      if (!roadSign) return
      return roadSign.id
    },
    removeRoadSign (objectId: string, saveRecord: boolean) {
      return this.removeSimpleObject(objectId)
    },
    // ---------- 路面标识 end ----------
    // ---------- 标志牌 start ----------
    addSignalBoard (params: {
      option: ISignalBoardConfig
      intersection: Intersection
    }) {
      const signalBoard = createObjectOnPole({ ...params, type: 'signalBoard' })
      if (!signalBoard) return
      this.ids.push(signalBoard.id)
      return signalBoard.id
    },
    moveSignalBoard (params: { objectId: string, point: common.vec3 }) {
      const signalBoard = moveObjectOnPole(params)
      if (!signalBoard) return
      return signalBoard.id
    },
    moveSignalBoardByHeightAndDist (params: {
      objectId: string
      height?: number
      distance?: number
    }) {
      const signalBoard = moveObjectOnPoleByHeightAndDist(params)
      if (!signalBoard) return
      return signalBoard.id
    },
    rotateSignalBoard (params: { objectId: string, angle: number }) {
      const signalBoard = rotateObjectOnPole(params)
      if (!signalBoard) return
      return signalBoard.id
    },
    // 快速切换标志牌的朝向
    toggleSignalBoardDirection (params: {
      objectId: string
      isForward: boolean
    }) {
      const signalBoard = toggleDirection(params)
      if (!signalBoard) return
      return signalBoard.id
    },
    removeSignalBoard (objectId: string, saveRecord: boolean) {
      return this.removeSimpleObject(objectId)
    },
    // ---------- 标志牌 end ----------
    // ---------- 信号灯 start ----------
    addTrafficLight (params: {
      option: ITrafficLightConfig
      intersection: Intersection
    }) {
      const trafficLight = createObjectOnPole({
        ...params,
        type: 'trafficLight',
      })
      if (!trafficLight) return
      this.ids.push(trafficLight.id)
      return trafficLight.id
    },
    moveTrafficLight (params: { objectId: string, point: common.vec3 }) {
      const trafficLight = moveObjectOnPole(params)
      if (!trafficLight) return
      return trafficLight.id
    },
    moveTrafficLightByHeightAndDist (params: {
      objectId: string
      height?: number
      distance?: number
    }) {
      const trafficLight = moveObjectOnPoleByHeightAndDist(params)
      if (!trafficLight) return
      return trafficLight.id
    },
    rotateTrafficLight (params: { objectId: string, angle: number }) {
      const trafficLight = rotateObjectOnPole(params)
      if (!trafficLight) return
      return trafficLight.id
    },
    // 更新信号灯的类型
    updateTrafficLightType (params: { objectId: string, typeName: string }) {
      const trafficLight = updateTrafficLightType(params)
      if (!trafficLight) return
      return trafficLight.id
    },
    updateTrafficLightControlJunction (params: {
      objectId: string
      junctionId: string
    }) {
      const trafficLight = updateTrafficLightControlJunction(params)
      if (!trafficLight) return
      return trafficLight.id
    },
    updateTrafficLightControlRoad (params: {
      objectId: string
      roadId: string
    }) {
      const trafficLight = updateTrafficLightControlRoad(params)
      if (!trafficLight) return
      return trafficLight.id
    },
    // 快速切换信号灯的朝向
    toggleTrafficLightDirection (params: {
      objectId: string
      isForward: boolean
    }) {
      const trafficLight = toggleDirection(params)
      if (!trafficLight) return
      return trafficLight.id
    },
    removeTrafficLight (objectId: string, saveRecord: boolean) {
      return this.removeSimpleObject(objectId)
    },
    // ---------- 信号灯 end ----------
    // ---------- 传感器 start ----------
    addSensor (params: { option: ISensorConfig, intersection: Intersection }) {
      const sensor = createObjectOnPole({ ...params, type: 'sensor' })
      if (!sensor) return
      this.ids.push(sensor.id)
      return sensor.id
    },
    moveSensor (params: { objectId: string, point: common.vec3 }) {
      const sensor = moveObjectOnPole(params)
      if (!sensor) return
      return sensor.id
    },
    moveSensorByHeightAndDist (params: {
      objectId: string
      height?: number
      distance?: number
    }) {
      const sensor = moveObjectOnPoleByHeightAndDist(params)
      if (!sensor) return
      return sensor.id
    },
    rotateSensor (params: { objectId: string, angle: number }) {
      const sensor = rotateObjectOnPole(params)
      if (!sensor) return
      return sensor.id
    },
    // 快速切换传感器的朝向
    toggleSensorDirection (params: { objectId: string, isForward: boolean }) {
      const sensor = toggleDirection(params)
      if (!sensor) return
      return sensor.id
    },
    removeSensor (objectId: string, saveRecord: boolean) {
      // 判断当前传感器的类型，是否是 rsu。如果是 rsu，需要判断是否有跟当前删除的 rsu 绑定的摄像头、激光雷达、毫米波雷达
      const objectData = getObject(objectId) as biz.ISensor
      if (objectData) {
        if (objectData.name === 'RSU') {
          // 先获取除了 rsu 以外的所有传感器的列表
          const sensorList = getAllSensorExceptRsu()
          for (const sensor of sensorList) {
            // 判断传感器关联的通信单元，是否是当前删除的 rsu，如果是则解除绑定关系
            if (sensor.deviceParams.BelongRSU === objectId) {
              sensor.deviceParams.BelongRSU = ''
            }
          }
        }
      }
      return this.removeSimpleObject(objectId)
    },
    // 更新传感器的设备参数
    updateSensorDeviceParams (params: {
      objectId: string
      deviceParams: any
      // 当前是否是更新 rsu 关联的路口 id。如果是，需要在 dispatcher 中做对应的渲染联动
      isUpdateRsuJunctionIds?: boolean
    }) {
      const sensor = updateSensorDeviceParams(params)
      if (!sensor) return
      return sensor.id
    },
    // ---------- 传感器 end ----------
    // ---------- 其他类型 start ----------
    addOther (params: { option: IOtherConfig, intersection: Intersection }) {
      const other = createOther(params)
      if (!other) return
      this.ids.push(other.id)
      return other.id
    },
    moveOther (params: {
      objectId: string
      point: common.vec3
      roadId?: string
      refLineLocation?: biz.IRefLineLocation
    }) {
      const other = moveOther(params)
      if (!other) return
      return other.id
    },
    moveOtherByST (params: {
      objectId: string
      s: number
      t: number
      saveRecord?: boolean
    }) {
      const other = moveOtherByST(params)
      if (!other) return
      return other.id
    },
    rotateOther (params: { objectId: string, yaw?: number, angle?: number }) {
      const other = rotateOther(params)
      if (!other) return
      return other.id
    },
    updateOtherSize (params: {
      objectId: string
      length: number
      width: number
      height?: number
    }) {
      const other = updateOtherSize(params)
      if (!other) return
      return other.id
    },
    updateBridgeSpan (params: { objectId: string, span: number }) {
      const bridge = updateBridgeSpan(params)
      if (!bridge) return
      return bridge.id
    },
    removeOther (objectId: string, saveRecord: boolean) {
      return this.removeSimpleObject(objectId)
    },
    // ---------- 其他类型 end ----------
    // ---------- 单独处理的停车位 start ----------
    addParkingSpace (params: {
      option: IParkingSpaceConfig
      intersection: Intersection
    }) {
      const parkingSpace = createParkingSpace(params)
      if (!parkingSpace) return
      this.ids.push(parkingSpace.id)
      return parkingSpace.id
    },
    moveParkingSpace (params: {
      objectId: string
      point: common.vec3
      roadId?: string
      refLineLocation?: biz.IRefLineLocation
    }) {
      const parkingSpace = moveParkingSpace(params)
      if (!parkingSpace) return
      return parkingSpace.id
    },
    moveParkingSpaceByST (params: {
      objectId: string
      s: number
      t: number
      saveRecord?: boolean
    }) {
      const parkingSpace = moveParkingSpaceByST(params)
      if (!parkingSpace) return
      return parkingSpace.id
    },
    rotateParkingSpace (params: { objectId: string, yaw: number }) {
      const parkingSpace = rotateParkingSpace(params)
      if (!parkingSpace) return
      return parkingSpace.id
    },
    updateParkingSpaceProperty (params: {
      objectId: string
      type: ParkingSpacePropertyType
      value: string | number
    }) {
      const parkingSpace = updateParkingSpaceProperty(params)
      if (!parkingSpace) return
      return parkingSpace.id
    },
    removeParkingSpace (objectId: string, saveRecord: boolean) {
      return this.removeSimpleObject(objectId)
    },
    // 复制停车位
    copyParkingSpace (objectId: string) {
      const newParkingSpace = copyParkingSpace(objectId)
      if (!newParkingSpace) return
      this.ids.push(newParkingSpace.id)
      return newParkingSpace.id
    },
    // ---------- 单独处理的停车位 end ----------
    // ---------- 自定义导入三维模型 start ----------
    addCustomModel (params: {
      option: ICustomModelConfig
      intersection: Intersection
    }) {
      const model = createCustomModel(params)
      if (!model) return
      this.ids.push(model.id)
      return model.id
    },
    moveCustomModel (params: { objectId: string, point: common.vec3 }) {
      const model = moveCustomModel(params)
      if (!model) return
      return model.id
    },
    moveCustomModelByST (params: {
      objectId: string
      s: number
      t: number
      saveRecord?: boolean
    }) {
      const model = moveCustomModelByST(params)
      if (!model) return
      return model.id
    },
    rotateCustomModel (params: {
      objectId: string
      yaw?: number
      angle?: number
    }) {
      const model = rotateCustomModel(params)
      if (!model) return
      return model.id
    },
    updateCustomModelSize (params: {
      objectId: string
      length: number
      width: number
      height: number
    }) {
      const model = updateCustomModelSize(params)
      if (!model) return
      return model.id
    },
    removeCustomModel (objectId: string, saveRecord: boolean) {
      return this.removeSimpleObject(objectId)
    },
    // ---------- 自定义导入三维模型 end ----------
  },
})
