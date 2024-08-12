import { map } from 'lodash-es'
import state from './state'
import * as mutations from './mutations'
import * as actions from './actions'

export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state,
  getters: {
    // 当前选中的对象
    selectedObject (state) {
      return state.objects.find(obj => `${obj.type}.${obj.id}` === state.selectedObjectKey) || null
    },
    // 场景元素树
    tree (state, getter, rootState) {
      const objects = state.objects

      const tree = {
        planner: [],
        car: [],
        moto: [],
        bike: [],
        pedestrian: [],
        animal: [],
        obstacle: [],
        machine: [],
        signlight: [],
        sensor: [],
      }
      objects.forEach((obj) => {
        if (obj.type === 'planner') {
          const {
            planners: {
              plannerList,
              sensors: tSensors,
            },
          } = rootState
          // 挂载一下传感器
          obj.sensors = map(plannerList.find(p => p.variable === obj.name).sensor.group, (s) => {
            const _s = tSensors.find(ts => ts.idx === s.idx)
            return {
              ...s,
              ..._s,
            }
          })
        }
        if (!tree[obj.type]) {
          tree[obj.type] = []
        }
        tree[obj.type].push(obj)
      })
      Object.keys(tree).forEach((key) => {
        if (!tree[key].length) {
          delete tree[key]
        }
      })
      return tree
    },
    // 是否正在播放界面
    isPlaying (state) {
      return state.status === 'playing' || state.status === 'renderPlaying'
    },
    // 是否正在编辑界面
    isModifying (state) {
      return state.status === 'modifying'
    },
    // 是否正在批量播放界面
    batchPlaying (state) {
      return state.status === 'playing' && state.playingMode === 'list' && state.shouldPlayAfterSetup
    },
    // 正在编辑的场景和新建的场景
    presentScenario (state) {
      return state.currentScenario || state.newScenario
    },
    trafficAI (state) {
      return state.trafficAI
    },
    // 当前视图支持的视角类型
    currentCameraSupported (state) {
      return state.status === 'modifying' ? state.supportedPerspectivesModifying : state.supportedPerspectives
    },
    // 是否Logsim
    isLogSim (state) {
      const flag = state.currentScenario && state.currentScenario.traffictype === 'simrec'
      return !!flag
    },
  },
  mutations,
  actions,
}
