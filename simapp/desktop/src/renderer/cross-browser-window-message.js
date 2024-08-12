const { initiate, send } = require('./cross-browser-window-ipc-renderer')

const object = {}

/**
 * 注册事件
 * @param methodName
 * @param callback
 */
function on (methodName, callback) {
  object[methodName] = callback
}

/**
 * 触发事件
 * @param args
 */
function emit (...args) {
  send(args)
}

/**
 * 注销事件
 * @param methodName
 */
function off (methodName) {
  delete object[methodName]
}

/**
 * 注册一次性事件
 * @param methodName
 * @param callback
 */
function once (methodName, callback) {
  on(methodName, (...args) => {
    const result = callback(...args)
    off(methodName)
    return result
  })
}

initiate(object)

module.exports = {
  on,
  emit,
  off,
  once,
}
