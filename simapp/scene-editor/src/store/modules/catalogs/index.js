import { saveModel as saveModelService } from '@/api/models.ts'
import { editor } from '@/api/interface'

// const models = [
//   {
//     type: 'planner', list: [
//       'suv', 'truck',
//       'MPV_001', 'AIV_FullLoad_001', 'AIV_Empty_001',
//       'AIV_FullLoad_002', 'AIV_Empty_002', 'MIFA_01',
//       'MiningDumpTruck_001',
//     ],
//   },
//   {
//     type: 'car',
//     list: [
//       'sedan', 'suv', 'truck', 'bus', 'sedan_001', 'sedan_002', 'sedan_003', 'sedan_004', 'suv_001', 'suv_002',
//       'suv_003', 'suv_004', 'suv_005', 'truck_002', 'truck_003',
//       'bus_001', 'semi_trailer_truck_001', 'AdminVehicle_01',
//       'AdminVehicle_02', 'Bus_005', 'Ambulance', 'MPV_001', 'MPV_002', 'AIV_FullLoad_001', 'AIV_Empty_001',
//       'AIV_FullLoad_002', 'AIV_Empty_002', 'MIFA_01', 'Truck_with_cargobox', 'Truck_without_cargobox',
//       'MiningDumpTruck_001',
//     ],
//   },
//   { type: 'moto', list: ['moto_001'] },
//   { type: 'bike', list: ['bike_001', 'elecBike_001', 'tricycle_001', 'tricycle_002', 'vendingCar_001'] },
//   { type: 'pedestrian', list: ['human', 'child', 'oldman', 'woman', 'girl'] },
//   { type: 'animal', list: ['cat', 'dog'] },
//   {
//     type: 'obstacle', list: [
//       'Box', 'Cone', 'Sand', 'Sedan', 'Box_001', 'Stob_001', 'Stob_002',
//       'Stob_003', 'Stob_004', 'Stob_005', 'Stob_006', 'Stob_tran_001',
//       'Trash_001', 'Trash_002', 'Wheel_001', 'Wheel_002', 'Warning_001',
//       'Stone_001', 'Port_Crane_001', 'Port_Crane_002', 'Port_Container_001',
//       'Port_Container_002',
//     ],
//   },
//   {
//     type: 'machine', list: [
//       'Port_Crane_001', 'Port_Crane_002',
//       'Port_Crane_002_0_0', 'Port_Crane_002_0_2',
//       'Port_Crane_002_0_5', 'Port_Crane_002_1_0',
//       'Port_Crane_002_5_0',
//     ],
//   },
//   { type: 'signlight', list: ['signlight'] },
// ]

// const vehicleTypeMap = {
//   car: ['car', 'combination', 'bus', 'truck', 'semitrailer'],
//   moto: ['motorbike'],
//   bike: ['bicycle'],
//   machine: ['crane'],
// }
//
// const pedestrianTypeMap = {
//   pedestrian: ['pedestrian'],
//   animal: ['animal'],
// }
//
// const nonMotorVehicles = [
//   'elecBike_001', 'tricycle_001', 'tricycle_002', 'vendingCar_001',
// ]

export default {
  namespaced: true,
  state: {
    vehicleList: [],
    pedestrianList: [],
    obstacleList: [],
  },
  getters: {
    models (state) {
      const unusedElement = []
      const modelList = {
        car: [],
        moto: [],
        bike: [],
        pedestrian: [],
        animal: [],
        obstacle: [],
        machine: [],
      }

      const {
        vehicleList,
        pedestrianList,
        obstacleList,
      } = state
      vehicleList.forEach((v) => {
        const interfaceCategory = v.catalogParams[0]?.properties.interfaceCategory
        if (interfaceCategory && modelList[interfaceCategory]) {
          modelList[interfaceCategory].push(v)
        } else {
          // 收集未归类模型
          unusedElement.push(v)
        }
      })
      pedestrianList.forEach((p) => {
        const interfaceCategory = p.catalogParams?.properties.interfaceCategory
        if (interfaceCategory && modelList[interfaceCategory]) {
          modelList[interfaceCategory].push(p)
        } else {
          // 收集未归类模型
          unusedElement.push(p)
        }
      })
      obstacleList.forEach((o) => {
        const interfaceCategory = o.catalogParams?.properties.interfaceCategory
        if (interfaceCategory && modelList[interfaceCategory]) {
          modelList[interfaceCategory].push(o)
        } else {
          // 收集未归类模型
          unusedElement.push(o)
        }
      })
      if (unusedElement.length) {
        console.warn('There are unused catalogs:', unusedElement)
      }
      return modelList
    },
  },
  mutations: {
    updateList (state, payload) {
      ({
        vehicleList: state.vehicleList,
        pedestrianList: state.pedestrianList,
        obstacleList: state.obstacleList,
      } = payload)
    },
  },
  actions: {
    async saveModel ({ dispatch, rootGetters }, payload) {
      const { id, data } = payload
      await saveModelService(id, data)
      await dispatch('planners/getAllCatalogs', null, { root: true })
      if (rootGetters['scenario/presentScenario']) {
        editor.scenario.updateModel(data)
      }
    },
  },
}
