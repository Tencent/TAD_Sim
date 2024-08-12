import { ElMessage } from 'element-plus'
import service from '@/api'
import { errorHandler } from '@/common/errorHandler'
import i18n from '@/locales'

export default {
  strict: import.meta.env.MODE === 'development', // 开发模式下启用严格模式
  namespaced: true, // 模块命名空间隔离
  state: {
    loading: false, // 加载状态
    keyword: '', // 关键词
    scheme: null, // 方案信息
    currentModuleSetGroups: null, // 当前激活的模块集合组ID
  },
  getters: {
    /**
     * 模块集合列表
     * @param {object} state - 模块集合管理模块状态
     * @returns {Array} - 模块集合列表
     */
    moduleSetList (state) {
      return state.scheme ? state.scheme.schemes : []
    },
    // 全局模块集合的id
    globalModuleSetId (state, getters) {
      return 99999
    },
    // 全局模块
    globalModuleSet (state, getters) {
      const globalItem = getters.moduleSetList.find(e => e.id === getters.globalModuleSetId)
      return globalItem
    },
    // moduleSetActive (state, getters) {
    //   const { mainCarModuleSetList } = getters
    //   const { currentModuleSetId } = state
    //   if (mainCarModuleSetList?.length) {
    //     return mainCarModuleSetList.find(item => {
    //       return item.id === currentModuleSetId
    //     })
    //   } else {
    //     return null
    //   }
    // },
    filteredModuleSetList (state, getters) {
      const { scheme } = state
      if (scheme) {
        return scheme.schemes.filter(item => item.id !== getters.globalModuleSetId && item.name.includes(state.keyword))
      } else {
        return []
      }
    },
    mainCarModuleSetList (state, getters) {
      const { scheme } = state
      if (scheme) {
        return scheme.schemes.filter(item => item.id !== getters.globalModuleSetId)
      } else {
        return []
      }
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    // 获取模块集合列表
    async getModuleSetList ({ commit }) {
      try {
        const scheme = await service.getModuleSetList()
        scheme.schemes.forEach((item) => {
          item.category = item.category || 'L4'
        })
        commit('updateState', { scheme })
      } catch (e) {
        await errorHandler(e)
      }
    },
    // 创建模块集合
    async createModuleSet ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        console.log('addModuleConfig', payload)
        await service.createModuleSet(payload)
        ElMessage.success(i18n.t('tips.createModuleConfigSuccess'))
        await dispatch('getModuleSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    // 更新模块集合
    async updateModuleSet ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        console.log('updateModuleConfig', payload)
        await service.updateModuleSet(payload)
        ElMessage.success(i18n.t('tips.modifyModuleConfigSuccess'))
        await dispatch('getModuleSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    async destroyModuleSet ({ commit, dispatch }, id) {
      commit('updateState', { loading: true })
      try {
        await service.destroyModuleSet(id)
        ElMessage.success(i18n.t('tips.deleteModuleConfigSuccess'))
        await dispatch('getModuleSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    async activeModuleSet ({ commit, dispatch, state }, id) {
      // todo: 此处的判断需要重写
      if (id === state.currentModuleSetGroups) return
      commit('updateState', { loading: true })
      try {
        await service.activeModuleSet(id)
        commit('updateState', {
          currentModuleSetGroups: id,
        })
        await dispatch('getModuleSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    async importModuleSet ({ commit, dispatch }, filePath) {
      commit('updateState', { loading: true })
      try {
        await service.importModuleSet(filePath)
        ElMessage.success(i18n.t('tips.importModuleConfigSuccess'))
        await dispatch('sim-modules/getModuleList', null, { root: true })
        await dispatch('getModuleSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    async exportModuleSet ({ commit }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.exportModuleSet(payload)
        ElMessage.success(i18n.t('tips.exportModuleConfigSuccess'))
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    async removeModuleSet ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.updateModuleSet(payload)
        ElMessage.success(i18n.t('tips.deleteModuleSuccess'))
        await dispatch('getModuleSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    async addModuleSet ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await service.updateModuleSet(payload)
        await dispatch('getModuleSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
  },
}
