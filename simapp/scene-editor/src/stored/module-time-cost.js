import { throttle } from 'lodash-es'
import { deepFreezeObject } from '@/common/utils'

let moduleTimeCost = {}

/**
 * 获取模块时间花费
 * @returns {object} - 模块时间花费对象
 */
export function getModuleTimeCost () {
  return moduleTimeCost
}

/**
 * 设置模块时间成本
 * @param {Array} data - 包含模块名称和时间成本的数据数组
 */
export function setModuleTimeCost (data) {
  data.forEach((item) => {
    const { name, timeCost } = item

    moduleTimeCost[name] = timeCost
  })
}

/**
 * 提交模块时间成本到Vuex store
 * @param {object} commit - Vuex提交对象
 */
export function commitModuleTimeCost (commit) {
  commit('chart-bar/updateState', { moduleTimeCost: deepFreezeObject({ ...moduleTimeCost }) }, { root: true })
}

/**
 * 重置模块时间成本
 * @param {object} commit - Vuex提交对象
 */
export function resetModuleTimeCost (commit) {
  moduleTimeCost = {}
  commitModuleTimeCost(commit)
}

/**
 * 节流函数，用于控制提交模块时间成本的频率
 * @param {object} commit - Vuex提交对象
 * @param {boolean} setupReady - 是否准备就绪
 * @returns {Function} - 节流后的函数
 */
export const throttleModuleTimeCost = throttle((commit, setupReady) => {
  if (setupReady) {
    commitModuleTimeCost(commit)
  }
}, 100, { trailing: true })
