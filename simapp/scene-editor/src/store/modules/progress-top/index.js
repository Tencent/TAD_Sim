import { queryProgressInfoMap as queryProgressInfoMapService, stopProgressMap as stopProgressMapService } from '../../../api/map'
import { queryProgressInfo as queryProgressInfoService, queryScenarioGenerationInfo, stopProgress as stopProgressService, stopScenarioGeneration } from '../../../api/scenario'
import {
  queryProgressInfo as queryProgressInfoSemanticService,
  stopgenprogressval,
} from '@/api/semantic'
import { messageBoxConfirm } from '@/plugins/message-box'
import i18n from '@/locales'

const { electron } = window

// 批量导入导出场景进度
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,

  state: {
    progressInfo: {
      mode: 0,
      status: false, // 是否启动
      title: 'Scene importing',
      progress: 0,
      dir: '',
    },
  },

  mutations: {
    updateProgressInfo (state, payload) {
      Object.assign(state.progressInfo, payload)
    },
  },

  actions: {
    /**
     * 查询进度
     * @param commit
     * @param state
     * @return {Promise<void>}
     */
    async queryProgressInfo ({ commit, state }) {
      const oldValue = state.progressInfo.progress
      let value = {}
      const { mode } = state.progressInfo
      try {
        if (mode === 0 || mode === 1) { // 场景导入导出
          value = await queryProgressInfoService()
        } else if (mode === 10 || mode === 11) { // 地图导入导出
          value = await queryProgressInfoMapService()
        } else if (mode === 20) { // 场景生成
          value = await queryScenarioGenerationInfo()
        } else if (mode === 30) { // 场景生成
          value = await queryProgressInfoSemanticService()
        }
        if (mode !== 30) {
          const { code, message } = value
          if (code === 0) {
            const messageNum = Number.parseFloat(message) * 100
            commit('updateProgressInfo', {
              progress: Math.round(messageNum * 10) / 10,
            })
          }
        }
      } catch (error) {
        if (oldValue === 100) {
          commit('updateProgressInfo', {
            status: false,
          })
        } else {
          commit('updateProgressInfo', {
            progress: oldValue + 10,
          })
        }
      }
    },
    /**
     * 停止进度
     * @param commit
     * @param state
     * @return {Promise<void>}
     */
    async stopProgress ({ commit, state }) {
      try {
        const { mode, dir } = state.progressInfo
        if (mode === 0 || mode === 1) {
          await stopProgressService()
        } else if (mode === 10 || mode === 11) {
          await stopProgressMapService()
        } else if (mode === 20) {
          await stopScenarioGeneration()
        } else if (mode === 30) {
          messageBoxConfirm(i18n.t('tips.generateScenariosSuccess'), i18n.t('tips.tips'), {
            cancelButtonText: i18n.t('tips.openTheFolder'),
            closeOnPressEscape: false,
            closeOnClickModal: false,
          }).catch(() => {
            electron.userLog.openUserLogFolder(dir)
          })
          await stopgenprogressval()
        }
        commit('updateProgressInfo', {
          status: false,
        })
      } catch (error) {
        commit('updateProgressInfo', {
          status: false,
        })
      }
    },
  },
}
