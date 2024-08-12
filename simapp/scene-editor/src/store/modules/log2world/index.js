import { ElMessage } from 'element-plus'
import { getLog2WorldConfig as getLog2WorldConfigService } from '../../../api/log2world'

// log2world状态管理
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,

  state: {
    // 配置信息
    form: {
      egoSwitch: true, // 主车Log2World切换开关
      trafficSwitch: true, // 交通流Log2World切换开关
      switchTime: 0, // 单位毫秒
      switchType: 1, // EVENT，SCENE。其中 1: SCENE代表Log2World切换时机：场景第x秒; 2: EVENT代表Log2World切换时机：第一个事件点前x秒;

      switchTimeScene: 0,
      time: 0, // 场景切换时间，来显示状态是否切换
    },
    // 当前场景事件列表
    events: {
      events: [],
      duration: 1000,
      startTimestamp: 0,
    },

    realSwitchTime: 0, // 相对时间

    // 提交的配置信息，用于保存
    committedForm: {
      egoSwitch: true,
      trafficSwitch: true,
      switchTime: 0,
      switchType: 1,

      switchTimeScene: 0,
      time: 0,
    },

  },

  getters: {
    // 当前是否开启Log2World
    isLog2World (state) {
      const { egoSwitch, trafficSwitch } = state.committedForm || {}
      return egoSwitch || trafficSwitch
    },
  },

  mutations: {
    updateState (state, payload) {
      Object.assign(state.form, payload)
    },

    updateEvent (state, payload) {
      Object.assign(state.events, payload)
    },

    updateRealSwitchTime (state, payload) {
      state.realSwitchTime = payload
    },

    updateCommittedForm (state, payload) {
      Object.assign(state.committedForm, payload)
    },
  },

  actions: {
    // 获取Log2World配置信息
    async getLog2WorldConfig ({ commit }, { scenePath }) {
      try {
        const { code, msg, config, events } = await getLog2WorldConfigService({
          scenePath,
        })
        if (code === 100) { // 成功
          const { switchTime, switchType } = config
          if (switchType === 1) {
            config.switchTimeScene = switchTime
            config.switchTime = 0
          } else {
            config.switchTimeScene = 0
          }

          commit('updateState', config)
          commit('updateCommittedForm', config)

          const {
            events: innerEvents,
          } = events
          for (const e of innerEvents) {
            e.time = e.ts
          }

          commit('updateEvent', events)

          let realTime = switchTime
          if (switchType === 2 && innerEvents.length) {
            realTime = innerEvents[0].ts - switchTime
          }
          commit('updateRealSwitchTime', realTime)
        } else {
          ElMessage.error(msg)
        }
      } catch (error) {
        ElMessage.error(error.message)
      }
    },
  },

}
