import { throttle } from 'lodash-es'
import { deepFreezeObject } from '@/common/utils'

let moduleStatus = {}

/**
 * 获取模块状态
 * @returns {object} - 模块状态对象
 */
export function getModuleStatus () {
  return moduleStatus
}

/**
 * 设置模块状态
 * @param {Array} data - 包含模块信息的数组
 */
export function setModuleStatus (data) {
  data.forEach((item) => {
    const { name, initiated, connected } = item

    moduleStatus[name] = {
      initiated,
      connected,
    }
  })
}

/**
 * 提交模块状态到Vuex store
 * @param {Function} commit - Vuex的commit方法
 */
export function commitModuleStatus (commit) {
  commit('chart-bar/updateState', { moduleStatus: deepFreezeObject({ ...moduleStatus }) }, { root: true })
}

/**
 * 重置模块状态
 * @param {Function} commit - Vuex的commit方法
 */
export function resetModuleStatus (commit) {
  moduleStatus = {}
  commitModuleStatus(commit)
}

/**
 * 节流后的模块状态提交函数
 * @param {Function} commit - Vuex的commit方法
 * @param {boolean} setupReady - 是否准备就绪
 */
export const throttleModuleStatus = throttle((commit, setupReady) => {
  if (setupReady) {
    commitModuleStatus(commit)
  }
}, 100, { trailing: true })
