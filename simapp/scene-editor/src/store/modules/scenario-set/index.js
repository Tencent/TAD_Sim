import { forEach } from 'lodash-es'
import { ElMessage } from 'element-plus'
import {
  createScenarioSet,
  destroyScenarioSet,
  getScenarioSetList,
  updateScenarioSet,
} from '@/api/scenario-set'
import { errorHandler } from '@/common/errorHandler'
import i18n from '@/locales'

// 场景集状态管理
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    loading: false,
    exporting: false,
    keyword: '',
    scenarioSetList: [],
  },
  getters: {
    filteredScenarioSetList (state) {
      return state.scenarioSetList.filter(item => item.id !== 1 && item.name.includes(state.keyword))
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    /**
     * 获取场景集列表
     * @param commit
     * @return {Promise<void>}
     */
    async getScenarioSetList ({ commit }) {
      try {
        const { data: scenarioSetList = [] } = await getScenarioSetList()
        scenarioSetList.forEach((scenarioSet) => {
          const { scenes, id } = scenarioSet
          if (scenes) {
            const ids = scenes.split(',')
            const scenarioIds = []
            forEach(ids, (id) => {
              if (id) {
                scenarioIds.push(+id)
              }
            })
            scenarioSet.scenes = scenarioIds
          } else {
            scenarioSet.scenes = []
          }
          if (id === 1) {
            scenarioSet.label = i18n.t('scenario.buildinSceneSet')
            scenarioSet.name = i18n.t('scenario.ungroupedScenes')
          }
        })
        commit('updateState', { scenarioSetList })
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 创建场景集
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async createScenarioSet ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await createScenarioSet(payload)
        ElMessage.success(i18n.t('tips.createScenarioSetSuccess'))
        await dispatch('getScenarioSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 更新场景集
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async updateScenarioSet ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await updateScenarioSet(payload)
        ElMessage.success(i18n.t('tips.modifyScenarioSetSuccess'))
        await dispatch('getScenarioSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 删除场景集
     * @param commit
     * @param dispatch
     * @param id
     * @return {Promise<void>}
     */
    async destroyScenarioSet ({ commit, dispatch }, id) {
      commit('updateState', { loading: true })
      try {
        await destroyScenarioSet(id)
        ElMessage.success(i18n.t('tips.deleteScenarioSetSuccess'))
        await dispatch('getScenarioSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 从场景集删除场景
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async removeScenario ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await updateScenarioSet(payload)
        ElMessage.success(i18n.t('tips.deleteScenarioSuccess'))
        await dispatch('getScenarioSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 向场景集添加场景
     * @param commit
     * @param dispatch
     * @param payload
     * @return {Promise<void>}
     */
    async addScenario ({ commit, dispatch }, payload) {
      commit('updateState', { loading: true })
      try {
        await updateScenarioSet(payload)
        await dispatch('getScenarioSetList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
  },
}
