import { defineStore } from 'pinia'
import type {
  CarModel,
  DynamicData,
  EgoModel,
  ObstacleModel,
  PedestrianModel,
  Planner,
} from 'models-manager/src/catalogs/class.ts'
import { cloneDeep } from 'lodash-es'
import { getAllCatalogs } from '@/api/planners'
import { getModelDisplayName, setModelDisplayName } from '@/common/utils.ts'
import vuexStore from '@/store'

/**
 * 模型管理状态接口
 */
interface State {
  plannerList: EgoModel[]
  vehicleList: CarModel[]
  pedestrianList: PedestrianModel[]
  obstacleList: ObstacleModel[]
}

const vehicleInfoReg = /(\w+)(\((\d+)\))?/

const useModelsManager = defineStore('models-manager', {
  state (): State {
    return {
      plannerList: [],
      vehicleList: [],
      pedestrianList: [],
      obstacleList: [],
    }
  },
  actions: {
    /**
     * 获取所有模型
     */
    async getAllCatalogs () {
      // 先搁在这，后面有用
      const catalogs = await getAllCatalogs()
      const {
        driverCatalog,
        vehicleCatalog,
        pedestrianCatalog,
        miscobjectCatalog,
      } = catalogs
      this.plannerList = driverCatalog
      this.vehicleList = vehicleCatalog
      this.pedestrianList = pedestrianCatalog
      this.obstacleList = miscobjectCatalog
      return {
        plannerList: driverCatalog,
        vehicleList: vehicleCatalog,
        pedestrianList: pedestrianCatalog,
        obstacleList: miscobjectCatalog,
      }
    },
    /**
     * 复制主车
     * @param variable
     */
    async copyPlanner (variable: string) {
      const {
        dynamics,
        plannerList,
        currentDynamicData,
      } = vuexStore.state.planners

      const target = (plannerList as Array<Planner>).find(p => p.variable === variable)
      if (target) {
        const newPlanner = cloneDeep(target)
        setModelDisplayName(newPlanner, `${getModelDisplayName(newPlanner)}副本`)
        // 处理一下动力学，如果有动力学，则复制动力学，id 自增
        if (target.catalogParams[0].properties.dynamic) {
          const newDynamicLen = dynamics.length + 1
          let newDynamicId = '1'
          for (let i = 1; i <= newDynamicLen; i++) {
            newDynamicId = `${i}`
            const index = (dynamics as Array<DynamicData>).findIndex(dynamic => dynamic.id === newDynamicId)
            if (index === -1) {
              break
            }
          }
          newPlanner.catalogParams[0].properties.dynamic = +newDynamicId
          dynamics.push({
            id: newDynamicId,
            dynamicData: cloneDeep(currentDynamicData),
          })
        }

        const plannerLength = plannerList.length + 2
        // 处理传感器组，如果有传感器组，则复制传感器组，id 自增
        let newSensorGroupName = 1
        for (let i = 1; i < plannerLength; i++) {
          newSensorGroupName = i
          const index = (plannerList as Array<Planner>)
            .findIndex(planner => planner.sensor?.groupName === newSensorGroupName)
          if (index === -1) {
            break
          }
        }
        newPlanner.sensor.groupName = newSensorGroupName

        let vehicleIndex = 0
        let vehicleType
        const vehicleInfo = newPlanner.variable.match(vehicleInfoReg)
        if (vehicleInfo && vehicleInfo.length) {
          vehicleType = vehicleInfo[1]
        } else {
          throw new Error('variable 解析失败，请联系系统管理员。')
        }
        for (const v of plannerList) {
          const vInfo = v.variable.match(vehicleInfoReg)
          if (vInfo?.length) {
            if (vehicleType === vInfo[1]) {
              vehicleIndex = Math.max(vehicleIndex, +vInfo[3] || 0)
            }
          } else {
            throw new Error('variable 解析失败，请联系系统管理员。')
          }
        }
        vehicleIndex += 1
        // 括号编号自增
        newPlanner.variable = `${vehicleType}(${vehicleIndex})`
        newPlanner.catalogParams[0].properties.preset = false
        plannerList.push(newPlanner)
      }
    },
  },
})

export default useModelsManager
