import { throttle } from 'lodash-es'
import { deepFreezeObject } from '@/common/utils'

let moduleLogFileName = {}

/**
 * 获取模块日志文件名
 * @returns {object} - 模块日志文件名对象
 */
export function getModuleLogFileName () {
  return moduleLogFileName
}

/**
 * 设置模块日志文件名
 * @param {Array} data - 包含模块名称和日志文件名的数组
 */
export function setModuleLogFileName (data) {
  data.forEach((item) => {
    const { name, logFileName } = item

    moduleLogFileName[name] = logFileName
  })
}

/**
 * 提交模块日志文件名到Vuex store
 * @param {Function} commit - Vuex store的commit方法
 */
export function commitModuleLogFileName (commit) {
  commit('chart-bar/updateState', { moduleLogFileName: deepFreezeObject({ ...moduleLogFileName }) }, { root: true })
}

/**
 * 重置模块日志文件名
 * @param {Function} commit - Vuex store的commit方法
 */
export function resetModuleLogFileName (commit) {
  moduleLogFileName = {}
  commitModuleLogFileName(commit)
}

/**
 * 节流提交模块日志文件名到Vuex store
 * @param {Function} commit - Vuex store的commit方法
 * @param {boolean} setupReady - 是否准备就绪
 * @returns {Function} - 节流函数
 */
export const throttleModuleLogFileName = throttle((commit, setupReady) => {
  if (setupReady) {
    commitModuleLogFileName(commit)
  }
}, 100, { trailing: true })
