import state from './state'
import * as getters from './getters'
import * as actions from './actions'

/**
 * 信控配置状态管理
 */
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state,
  getters,
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions,
}
