import { ElMessage } from 'element-plus'
import service from '@/api'
import { errorHandler } from '@/common/errorHandler'
import i18n from '@/locales'

// 指标组状态管理
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    loading: false,
    keyword: '',
    activeKpiGroupId: 0,
    kpiGroupList: [],
  },
  getters: {
    /**
     * 用户搜索的指标列表结果
     * @param state
     * @return {*[]}
     */
    filteredKpiGroupList (state) {
      return state.kpiGroupList.filter(item => item.id !== -1 && item.name.includes(state.keyword))
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    /**
     * 获取指标组列表
     * @param commit
     * @return {Promise<void>}
     */
    async getKpiGroupList ({ commit }) {
      try {
        const { activeId: activeKpiGroupId, groups: kpiGroupList } = await service.getKpiGroupList()
        commit('updateState', { activeKpiGroupId, kpiGroupList })
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 创建指标组
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async createKpiGroup ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.createKpiGroup(payload)
        ElMessage.success(i18n.t('tips.createIndicatorGroupSuccess'))
        await dispatch('getKpiGroupList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 更新指标组
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async updateKpiGroup ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.updateKpiGroup(payload)
        ElMessage.success(i18n.t('tips.editIndicatorGroupSuccess'))
        await dispatch('getKpiGroupList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 删除指标组
     * @param commit
     * @param dispatch
     * @param id
     * @return {Promise<void>}
     */
    async destroyKpiGroup ({ commit, dispatch }, id) {
      commit('updateState', { loading: true })
      try {
        await service.destroyKpiGroup(id)
        ElMessage.success(i18n.t('tips.deleteIndicatorGroupSuccess'))
        await dispatch('getKpiGroupList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 激活指标组
     * @param commit
     * @param dispatch
     * @param id
     * @return {Promise<void>}
     */
    async activeKpiGroup ({ commit, dispatch }, id) {
      commit('updateState', { loading: true })
      try {
        await service.activeKpiGroup(id)
        ElMessage.success(i18n.t('tips.applyIndicatorGroupSuccess'))
        await dispatch('getKpiGroupList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 创建指标组下的指标
     * @param commit
     * @param payload
     * @return {Promise<void>}
     */
    async createKpisByKpiGroup ({ commit }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.createKpisByKpiGroup(payload)
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 获取指标组下的指标
     * @param context
     * @param id
     * @return {Promise<*|*[]>}
     */
    async getKpisByKpiGroup (context, id) {
      try {
        const { kpis } = await service.getKpisByKpiGroup(id)
        console.log('== getKpisByKpiGroup', kpis)
        return kpis
      } catch (e) {
        await errorHandler(e)
        return []
      }
    },
  },
}
