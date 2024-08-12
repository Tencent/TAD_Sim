import { ElMessage } from 'element-plus'
import service from '@/api'
import { errorHandler } from '@/common/errorHandler'
import i18n from '@/locales'
import { delay } from '@/common/utils'

// 测评报告模块状态管理
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    loading: false,
    // 用户搜索关键词
    keyword: '',
    // 报告列表
    kpiReportList: [],
    polling: false,
    // 批量导出中
    exporting: false,
  },
  getters: {
    filteredKpiReportList (state) {
      return state.kpiReportList.filter(item => item.name.includes(state.keyword))
    },
    hasTmpFile (state) {
      return state.kpiReportList.findIndex(report => report.paths.findIndex(path => path.endsWith('.tmp')) > -1) > -1
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    /**
     * 获取报告列表
     * @param commit
     * @return {Promise<void>}
     */
    async getKpiReportList ({ commit }) {
      try {
        const kpiReportList = await service.getKpiReportList()
        commit('updateState', { kpiReportList })
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 删除报告
     * @param commit
     * @param dispatch
     * @param ids
     * @return {Promise<void>}
     */
    async destroyKpiReport ({ commit, dispatch }, ids) {
      commit('updateState', { loading: true })
      try {
        await service.destroyKpiReport(ids)
        ElMessage.success(i18n.t('tips.deleteEvaluationReportSuccess'))
        await dispatch('getKpiReportList')
      } catch (e) {
        await errorHandler(e)
      }
      commit('updateState', { loading: false })
    },
    /**
     * 获取报告
     * @param context
     * @param id
     * @return {Promise<unknown>}
     */
    async getKpiReport (context, id) {
      try {
        return service.getKpiReport(id)
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 开始轮询
     * @param state
     * @param dispatch
     * @param getters
     * @return {Promise<void>}
     */
    async startPolling ({ state, dispatch, getters }) {
      state.polling = true
      await dispatch('getKpiReportList')
      await delay(1000)
      while (state.polling) {
        if (getters.hasTmpFile) {
          await dispatch('getKpiReportList')
        }
        await delay(1000)
      }
    },
    /**
     * 结束轮询
     * @param state
     */
    endPolling ({ state }) {
      state.polling = false
    },
    /**
     * 上传报告截图
     * @param context
     * @param params
     * @return {Promise<unknown>}
     */
    async uploadReportImage (context, params) {
      try {
        return service.uploadReportImage(params)
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 删除报告截图
     * @param context
     * @param params
     * @return {Promise<unknown>}
     */
    async deleteReportImage (context, params) {
      try {
        return service.deleteReportImage(params)
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 获取报告截图
     * @param context
     * @param params
     * @return {Promise<unknown>}
     */
    async getReportImage (context, params) {
      try {
        return service.getReportImage(params)
      } catch (e) {
        await errorHandler(e)
      }
    },
    /**
     * 获取报告截图base64
     * @param context
     * @param params
     * @return {Promise<unknown>}
     */
    async getImageBase64 (context, params) {
      try {
        return service.getImageBase64(params)
      } catch (e) {
        await errorHandler(e)
      }
    },
  },
}
