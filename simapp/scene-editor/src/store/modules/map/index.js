import * as actions from './actions'
import * as mutations from './mutations'

// 地图模块state定义
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    mapList: [],
    // 是否正在导出
    exporting: false,
  },
  getters: {},
  mutations,
  actions,
}
