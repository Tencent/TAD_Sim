import { delay } from '@/common/utils'
import dict from '@/common/dict'
import i18n from '@/locales'
import { messageBoxConfirm, messageBoxError } from '@/plugins/message-box'

export default {
  strict: import.meta.env.MODE === 'development', // 开发模式下启用严格模式
  namespaced: true, // 模块命名空间隔离
  state: {
    currentMission: null, // 当前任务
    progressPercentage: 0, // 进度百分比
    progressVisible: false, // 进度条是否可见
    progressTitle: '', // 进度条标题
    missionProps: {}, // 任务属性
  },
  getters: {},
  mutations: {
    /**
     * 更新任务状态
     * @param {object} state - 任务模块状态
     * @param {object} payload - 需要更新的状态数据
     */
    updateState (state, payload) {
      Object.assign(state, payload)
    },
    /**
     * 开始任务
     * @param {object} state - 任务模块状态
     * @param {string | object} payload - 任务名称或包含任务名称和属性的对象
     */
    // 打开一个弹窗, 如果有弹窗已打开, 则不打开
    startMission (state, payload) {
      let name, props
      if (typeof payload === 'string') {
        name = payload
      } else {
        ({ name, props } = payload)
      }
      if (!state.currentMission) {
        state.currentMission = name
        state.missionProps = props
      }
    },
  },
  actions: {
    // 新建场景窗口
    async openCreateNewScenario ({ commit, state, rootState }) {
      if (rootState.scenario.scenarioList.length + 1 > dict.maxScenarioCount) {
        messageBoxError(i18n.t('tips.scenarioExceededCannotCreate', { maxScenarioCount: dict.maxScenarioCount }))
        return
      }
      if (state.currentMission) return
      if (rootState.scenario.isDirty) {
        try {
          await messageBoxConfirm(i18n.t('tips.addWithoutSaving'))
        } catch (e) {
          return
        }
      }
      commit('startMission', 'CreateNewScenario')
    },
    // 进度条
    async progress ({ commit }, { title, time, promise }) {
      commit('updateState', {
        progressTitle: title,
        progressVisible: true,
        progressPercentage: 0,
      })

      time = Math.max(time, 1000)

      const length = Math.floor(Math.random() * 5) + 8
      const timeout = time / length

      async function p () {
        for (let i = 0; i < length; i++) {
          commit('updateState', {
            progressPercentage: Math.floor(timeout * i / time * 100),
          })
          await delay(timeout * (Math.random() + 0.7))
        }
      }

      try {
        await Promise.race([promise, p()])
      } catch (err) {
        console.error(err)
        throw err
      } finally {
        commit('updateState', {
          progressPercentage: 100,
          progressVisible: false,
        })
      }
    },
  },
}
