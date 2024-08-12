const { ipcRenderer } = require('electron')
const { CrossBrowserWindowMessageChannel } = require('../common/constants')

/**
 * 创建垮窗口消息通道
 * @param object
 */
function initiate (object) {
  ipcRenderer.on(CrossBrowserWindowMessageChannel, (event, args) => {
    const [methodName, ...rest] = args
    const fn = object[methodName]

    if (fn) {
      fn(...rest)
    }
  })
}

function send (args) {
  ipcRenderer.send(CrossBrowserWindowMessageChannel, args)
}

function destroy () {
  ipcRenderer.removeAllListeners(CrossBrowserWindowMessageChannel)
}

module.exports = {
  initiate,
  send,
  destroy,
}
