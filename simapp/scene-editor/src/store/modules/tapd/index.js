import service from '@/api'
import { errorHandler, showErrorMessage } from '@/common/errorHandler'

// 从tapd导入测试计划列表
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    loading: false,
    // 测试计划列表
    testPlanList: [],
    // 测试用例列表
    tcaseNames: [],
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    /**
     * 获取测试计划列表
     * @param commit
     * @return {Promise<void>}
     */
    async getTestPlanList ({ commit }) {
      let testPlanList = []
      commit('updateState', { loading: true })
      try {
        const { status = 'unknown', info = 'unknown', data = [] } = await service.getTestPlanList()
        testPlanList = data.map((item) => {
          const { id, name } = item.TestPlan
          return {
            id,
            name,
          }
        })
        if (status !== 1) {
          await showErrorMessage(`[tapd.cn] get test plans status ${status}, info ${info}`)
        }
      } catch (e) {
        await errorHandler(e)
      } finally {
        commit('updateState', { testPlanList })
        commit('updateState', { loading: false })
      }
    },
    /**
     * 获取测试用例列表
     * @param commit
     * @param testPlanId
     * @return {Promise<void>}
     */
    async getTcaseNames ({ commit }, testPlanId) {
      const tcaseNames = []
      commit('updateState', { loading: true })
      try {
        if (testPlanId) {
          const { status = 'unknown', info = 'unknown', data = [] } = await service.getTestPlanTcaseList(testPlanId)
          if (status === 1) {
            const testPlanTcaseIds = data.map((item) => {
              return item.TestPlanStoryTcaseRelation.tcase_id
            })
            for (const testPlanTcaseId of testPlanTcaseIds) {
              const tcaseReponse = await service.getTcase(testPlanTcaseId)
              if (tcaseReponse.status === 1) {
                tcaseNames.push(tcaseReponse.data[0].Tcase.name)
              } else {
                await showErrorMessage(`[tapd.cn] get tcase status ${tcaseReponse.status}, info ${tcaseReponse.info}`)
              }
            }
          } else {
            await showErrorMessage(`[tapd.cn] get test plan tcases status ${status}, info ${info}`)
          }
        }
      } catch (e) {
        await errorHandler(e)
      } finally {
        commit('updateState', { tcaseNames })
        commit('updateState', { loading: false })
      }
    },
  },
}
