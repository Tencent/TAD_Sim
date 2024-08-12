import { ElMessage } from 'element-plus'
import service from '@/api'
import { errorHandler } from '@/common/errorHandler'
import i18n from '@/locales'
import { messageBoxInfo } from '@/plugins/message-box'

export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    loading: false,
    modules: [],
    scheme: null,
  },
  getters: {
    /**
     * 当前激活的模块列表
     * @param state
     * @param getters
     * @param rootState
     * @param rootGetters
     * @return {*[]}
     */
    modulesActive (state, getters, rootState, rootGetters) {
      const { scheme, currentModuleSetGroups } = rootState['module-set']
      const globalModuleSetId = rootGetters['module-set/globalModuleSetId']

      const globalModuleSet = rootGetters['module-set/globalModuleSet']
      // 构建多主车 modules
      const activeModules = []
      if (currentModuleSetGroups?.length) {
        currentModuleSetGroups.filter(c => c && c.scheme_id !== globalModuleSetId).forEach((m) => {
          const { schemes } = scheme
          const mSet = schemes.find(s => s.id === m.scheme_id)
          const modules = mSet?.modules || []
          modules.forEach((md) => {
            activeModules.push({
              ...md,
              name: `${m.group_name}-${md.name}`,
              groupName: m.group_name,
            })
          })
        })
      }
      const globalModules = globalModuleSet ? globalModuleSet.modules : []
      globalModules.forEach((m) => {
        m.groupName = 'global'
      })
      return [...activeModules, ...globalModules]
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
    /**
     * 上移模块
     * @param state
     * @param payload
     */
    moveModuleUp (state, payload) {
      const { index, item } = payload
      if (index === 0) {
        return
      }
      const targetIndex = index - 1
      state.modules.splice(index, 1)
      state.modules.splice(targetIndex, 0, item)
    },
    /**
     * 下移模块
     * @param state
     * @param payload
     */
    moveModuleDown (state, payload) {
      const { index, item } = payload
      if (index === state.modules.length - 1) {
        return
      }
      const targetIndex = index + 1
      state.modules.splice(index, 1)
      state.modules.splice(targetIndex, 0, item)
    },
  },
  actions: {
    /**
     * 获取模块列表
     * @param commit
     * @return {Promise<void>}
     */
    async getModuleList ({ commit }) {
      try {
        const modules = await service.getModuleList()
        commit('updateState', { modules })
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 创建模块
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async createModule ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.createModule(payload)
        ElMessage.success(i18n.t('tips.createModuleSuccess', { moduleName: payload.name }))
        await dispatch('getModuleList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 更新模块
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async updateModule ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.updateModule(payload)
        ElMessage.success(i18n.t('tips.modifyModuleSuccess', { moduleName: payload.name }))
        await dispatch('getModuleList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 删除模块
     * @param commit
     * @param dispatch
     * @param name
     * @return {Promise<void>}
     */
    async destroyModule ({ commit, dispatch }, name) {
      commit('updateState', { loading: true })
      try {
        await service.destroyModule(name)
        ElMessage.success(i18n.t('tips.deleteModuleSuccessWithName', { moduleName: name }))
        await dispatch('getModuleList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 重置模块
     * @param commit
     * @param dispatch
     * @param rootGetters
     * @param rootState
     * @return {Promise<void>}
     */
    async resetModules ({ commit, dispatch, rootGetters, rootState }) {
      const isPlaying = rootGetters['scenario/isPlaying']
      const { playingStatus, currentScenario } = rootState.scenario
      if (playingStatus === 'ready' && isPlaying) {
        await messageBoxInfo(i18n.t('tips.reloadScenarioAfterChangeConfig'))
        commit('updateState', { loading: true })
        await dispatch('scenario/resetScenario', currentScenario, { root: true })
        commit('updateState', { loading: false })
      }
    },
  },
}
