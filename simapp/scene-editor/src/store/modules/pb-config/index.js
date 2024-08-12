import {
  getPbConfigList,
} from '@/api/pb-config'
import dict from '@/common/dict'

export default {
  namespaced: true,
  state: {
    topicList: [], // pb topic 列表
    pbConfigList: [], // pb config 列表
    chosenPbConfigList: [], // 实时数据图表中选中的 pb config
  },
  getters: {
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    refreshTopicList ({ commit }) {
      const topicList = Object.keys(dict.MessageProtoMap).map(topic => ({ label: topic, value: topic }))
      commit('updateState', { topicList })
    },
    // 刷新 pb 配置列表
    async refreshPbConfigList ({ commit }) {
      const pbConfigList = await getPbConfigList()
      console.log('pb config list:', pbConfigList)
      commit('updateState', { pbConfigList })
    },
    // 重置
    resetChosenPbConfigList ({ commit }, chosen) {
      localStorage.setItem('pb-config-operated', false)
      localStorage.removeItem('pb-config-chosen')
      commit('updateState', { chosenPbConfigList: [] })
    },
    // 通过实际返回 pb 数值的层级结构，获取对应的 pb 配置
    updateChosenPbConfigList ({ commit }, chosen) {
      localStorage.setItem('pb-config-operated', true)
      if (chosen?.length) {
        commit('updateState', { chosenPbConfigList: chosen })
        localStorage.setItem('pb-config-chosen', JSON.stringify(chosen))
      } else {
        commit('updateState', { chosenPbConfigList: [] })
        localStorage.removeItem('pb-config-chosen')
      }
    },
  },
}
