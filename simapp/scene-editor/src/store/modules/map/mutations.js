/**
 * 更新状态
 * @param {Object} state - 当前状态对象
 * @param {Object} payload - 需要更新的状态数据
 */
export function updateState (state, payload) {
  Object.assign(state, payload)
}
