// 调试模块状态管理
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    collapse: true,
    currentTab: 0,
    currentName: '',
  },
  getters: {
    // 当前打开的日志文件名
    currentLogFileName (state, getters, rootState) {
      const { moduleLogFileName } = rootState['chart-bar']
      if (moduleLogFileName) {
        return moduleLogFileName[state.currentName]
      } else {
        return null
      }
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
    /**
     * 切换显示隐藏
     * @param state
     */
    toggleCollapse (state) {
      state.collapse = !state.collapse
    },
  },
}
