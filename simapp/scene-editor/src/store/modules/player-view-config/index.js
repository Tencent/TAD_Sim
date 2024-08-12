import { set } from 'lodash'
import { player } from '@/api/interface'

/**
 * 获取并初始化视图配置
 */
let viewConfig = localStorage.getItem('viewConfig')

/**
 * 默认视图配置
 * @returns {object} - 默认视图配置对象
 */
function defaultViewConfig () {
  return {
    cityMonitor: {
      simMainCar: {
        id: true,
        v: true,
        acc: true,
        lonlat: true,
      },
      simTraffic: {
        id: true,
        v: true,
        acc: true,
        vr: true,
        ar: true,
        lonlat: true,
      },
      others: {
        id: true,
      },
    },
    sceneViewConfig: {
      ego: true,
      trafficDynamic: true,
      signlight: true,
      obstacle: true,
    },
    mapViewConfig: {
      lane: true,
      lanelink: true,
      laneboundary: true,
      roadmesh: true,
      grid: true,
      road: true,
      mapobject: true,
    },
  }
}

try {
  viewConfig = JSON.parse(viewConfig)
} catch (e) {
  // pass
}

// 如果localStorage中没有视图配置或配置不完整，则使用默认配置
if (!viewConfig?.cityMonitor || !viewConfig?.sceneViewConfig || !viewConfig?.mapViewConfig) {
  viewConfig = defaultViewConfig()
}

/**
 * Vuex store状态
 */
const state = {
  ...viewConfig,
}

/**
 * Vuex store变更操作（mutations）
 */
const mutations = {
  /**
   * 更新视图配置
   * @param {object} state - Vuex store状态
   * @param {{ path: string, checked: boolean }} payload - 包含路径和选中状态的参数
   */
  updateViewConfig (state, { path, checked }) {
    set(state, path, checked)
    localStorage.setItem('viewConfig', JSON.stringify(state))
    const paths = path.split('.')
    if (paths[0] === 'sceneViewConfig' || paths[0] === 'mapViewConfig') {
      player.setObjectsVisible(paths[1])
    }
  },
  /**
   * 重置视图配置
   * @param {object} state - Vuex store状态
   */
  reset (state) {
    Object.assign(state, { ...defaultViewConfig() })
    localStorage.setItem('viewConfig', JSON.stringify(state))
    player.resetAllObjectVisible()
  },
}

/**
 * Vuex store操作（actions）
 */
const actions = {}

/**
 * Vuex store获取值（getters）
 */
const getters = {}

export default {
  namespaced: true,
  state,
  mutations,
  actions,
  getters,
}
