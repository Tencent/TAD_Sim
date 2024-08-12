const { ipcRenderer } = require('electron')

/**
 * 调用 electron 方法
 * @param path
 * @param args
 * @return {Promise<any>}
 */
function invoke (path, ...args) {
  return ipcRenderer.invoke('invokeMainMethod', path, args)
}

module.exports = {
  invoke,
}
