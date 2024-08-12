import { ElMessage } from 'element-plus'
import { showErrorMessage } from '@/common/errorHandler'
import dict from '@/common/dict'
import i18n from '@/locales'

const { electron } = window

const EDIT_CONFIG_STORAGE_KEY = 'editConfig'

/**
 * 放入localStorage的编辑器配置
 */
let storagedEditConfig
const storagedEditConfigStr = localStorage.getItem(EDIT_CONFIG_STORAGE_KEY)

try {
  // 可能没有配置
  storagedEditConfig = JSON.parse(storagedEditConfigStr)
} catch (e) {
}

if (!storagedEditConfig) {
  // 默认编辑器配置
  storagedEditConfig = {
    // 自动跟随道路方向
    autoFollowRoadDirection: true,
  }
}

export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    /**
     * 编辑器配置
     */
    simulation: {
      playControlRate: 100,
      scenarioTimeLimit: 60,
      coordinationMode: 1,
      addingInitialLocationMsg: true,
      autoReset: true,
      overrideUserLog: true,
      gradingFeedbackProcess: '',
    },
    /**
     * 天气
     */
    weather: {
      type: null,
      strength: 0,
    },
    sunlight: null,
    /**
     * 指标配置
     */
    kpi: {
      timeDistance: 86,
      accelerationRange: [2, 12],
      speedRange: [45, 87],
      collision: true,
      runLight: false,
      crossStopLine: true,
      crossSolidLine: true,
    },
    editor: storagedEditConfig,
  },
  getters: {},
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    /**
     * 获取配置
     * @param commit
     */
    getConfig ({ commit }) {
      electron.system.getConfig(async (status) => {
        const { retCode, data } = status
        if (retCode === 1) {
          commit('updateState', { simulation: data })
        } else {
          await showErrorMessage(i18n.t(dict.retCode[retCode]) || retCode)
        }
        console.log(status)
      })
    },
    /**
     * 保存配置
     * @param commit
     * @param dispatch
     * @param payload
     */
    saveConfig ({ commit, dispatch }, payload) {
      const { simulation, editor } = payload
      electron.system.setConfig(simulation, async (status) => {
        const { retCode } = status
        if (retCode === 1) {
          commit('updateState', { simulation })
          ElMessage.success(i18n.t('tips.saveSimulationSetSuccess'))
        } else {
          await showErrorMessage(i18n.t(dict.retCode[retCode]) || retCode)
        }
        console.log(status)
      })
      dispatch('setEditConfig', editor)
    },
    /**
     * 设置编辑器配置
     * @param commit
     * @param payload
     */
    setEditConfig ({ commit }, payload) {
      commit('updateState', { editor: payload })
      localStorage.setItem(EDIT_CONFIG_STORAGE_KEY, JSON.stringify(payload))
    },
    /**
     * 恢复默认配置
     * @param dispatch
     */
    restoreDefaultConfig ({ dispatch }) {
      electron.system.restoreDefaultConfig(async (status) => {
        const { retCode } = status
        if (retCode === 1) {
          dispatch('getConfig')
          ElMessage.success(i18n.t('tips.restoreSystemDefaultSetSuccess'))
        } else {
          await showErrorMessage(i18n.t(dict.retCode[retCode]) || retCode)
        }
        console.log(status)
      })
    },
    async saveKpi ({ commit }, payload) {
      // todo: 保存配置
    },
  },
}
