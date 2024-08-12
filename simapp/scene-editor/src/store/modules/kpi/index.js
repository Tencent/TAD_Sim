import { uniq } from 'lodash-es'
import { ElMessage } from 'element-plus'
import service from '@/api'
import { errorHandler } from '@/common/errorHandler'
import i18n from '@/locales'
import dict from '@/common/dict'

const { kpiLabelsList: systemKpiLabelsList } = dict

export default {
  strict: import.meta.env.MODE === 'development', // 开发模式下启用严格模式
  namespaced: true, // 模块命名空间隔离
  state: {
    loading: false, // 加载状态
    kpiList: [], // 指标列表
    // { labelKey, labelDescription, system }
    kpiLabelsList: [], // KPI标签列表，包含系统预定义和自定义标签
  },
  mutations: {
    /**
     * 更新模块状态
     * @param {object} state - 模块状态
     * @param {object} payload - 需要更新的状态数据
     */
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    /**
     * 获取指标列表
     * @param {object} context - Vuex上下文对象
     * @returns {Promise<void>} - 返回一个空Promise
     */
    async getKpiList ({ commit }) {
      try {
        const { kpis: kpiList } = await service.getKpiList()
        commit('updateState', { kpiList })
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 刷新KPI标签列表
     * @param {object} context - Vuex上下文对象
     * @returns {Promise<void>} - 返回一个空Promise
     */
    async refreshKpiLabelsList ({ commit }) {
      try {
        const res = await service.getGradingLabelDescriptions()
        const defaultKpiLabelsList = systemKpiLabelsList
          .map(({ id, name }) => ({ labelKey: id, labelDescription: i18n.t(name), system: true }))
        const kpiLabelsList = uniq([...defaultKpiLabelsList, ...res])
        console.log('refreshKpiLabelsList', kpiLabelsList)
        commit('updateState', { kpiLabelsList })
      } catch (e) {
        console.log(e)
        await errorHandler(e)
      }
    },
    /**
     * 更新指标配置
     * @param {object} context - Vuex上下文对象
     * @param {object} payload - 更新的指标配置数据
     * @returns {Promise<void>} - 返回一个空Promise
     */
    async updateKpi ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.updateKpi(payload)
        ElMessage.success(i18n.t('tips.modifyIndicatorConfigSuccess'))
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 删除一个指标配置
     * @param {object} context - Vuex上下文对象
     * @param {number} id - 要删除的指标ID
     * @returns {Promise<void>} - 返回一个空Promise
     */
    async destroyKpi ({ commit, dispatch }, id) {
      commit('updateState', { loading: true })
      try {
        await service.destroyKpi(id)
        ElMessage.success(i18n.t('tips.deleteIndicatorConfigSuccess'))
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
  },
}
