import { throttle } from 'lodash-es'

/**
 * 定义一个全局时间戳变量
 * @type {number}
 */
let timestamp = -1

/**
 * 获取当前时间戳
 * @returns {number} - 当前时间戳
 */
export function getTimestamp () {
  return timestamp
}

/**
 * 设置当前时间戳
 * @param {number} data - 要设置的时间戳
 */
export function setTimestamp (data) {
  timestamp = data
}

/**
 * 重置当前时间戳
 * @param {Function} commit - Vuex提交函数
 */
export function resetTimestamp (commit) {
  timestamp = -1
  commitTimestamp(commit)
}

/**
 * 提交时间戳到Vuex状态管理
 * @param {Function} commit - Vuex提交函数
 */
export function commitTimestamp (commit) {
  commit('updateState', { timestamp })
}

/**
 * 节流后提交时间戳到Vuex状态管理
 * @param {Function} commit - Vuex提交函数
 * @returns {Function} - 节流后的提交函数
 */
export const throttleTimestamp = throttle((commit) => {
  commitTimestamp(commit)
}, 100, { trailing: true })
