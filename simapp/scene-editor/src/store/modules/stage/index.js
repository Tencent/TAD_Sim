export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    // 画布尺寸
    clientRect: null,
    // 编辑器、播放器角度
    degreeEditor: 0,
    degreePlayer: 0,
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
}
