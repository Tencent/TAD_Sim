import * as actions from './actions'
import * as mutations from './mutations'

// 环境配置状态管理
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    visible: false,
    loading: false,
    globalMode: false,
    // 场景环境分组
    scenarioEvnGroup: [],
    // 全局环境分组
    globalEnvGroup: [],
  },
  getters: {},
  mutations,
  actions,
}
