const useCustomChart = true // 版本功能：自定义指标图表

export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    // 是否使用自定义指标图表
    useCustomChart,
    // 各种图表显示状态
    accelerationChartVisible: false,
    collisionChartVisible: false,
    speedChartVisible: false,
    moduleChartVisible: true,
    v2xWarningVisible: false,
    dashboardVisible: false,
    viewConfigVisible: false,
    moduleStatus: {},
    moduleTimeCost: {},
    moduleLogFileName: {},
    customChartVisible: false,
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
    /**
     * 切换图表显示状态
     * @param state
     * @param key
     */
    toggleChartsVisible (state, key) {
      if (['accelerationChartVisible', 'viewConfigVisible', 'collisionChartVisible', 'speedChartVisible', 'moduleChartVisible', 'dashboardVisible', 'v2xWarningVisible', 'customChartVisible'].includes(key)) {
        state[key] = !state[key]
      }
    },
  },
}
