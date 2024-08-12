const { BrowserWindow } = require('electron')

function getFocusedBrowserWindow () {
  return BrowserWindow.getFocusedWindow()
}

module.exports = {
  /**
   * 关闭当前窗口
   */
  close () {
    const focusedBrowserWindow = getFocusedBrowserWindow()

    if (focusedBrowserWindow) {
      focusedBrowserWindow.close()
    }
  },
  /**
   * 最大化当前窗口
   */
  maximize () {
    const focusedBrowserWindow = getFocusedBrowserWindow()

    if (focusedBrowserWindow) {
      focusedBrowserWindow.maximize()
    }
  },
  /**
   * 取消最大化当前窗口
   */
  unmaximize () {
    const focusedBrowserWindow = getFocusedBrowserWindow()

    if (focusedBrowserWindow) {
      focusedBrowserWindow.unmaximize()
    }
  },
  /**
   * 判断当前窗口是否最大化
   * @return {boolean}
   */
  isMaximized () {
    const focusedBrowserWindow = getFocusedBrowserWindow()

    if (focusedBrowserWindow) {
      return focusedBrowserWindow.isMaximized()
    }
  },
  /**
   * 最小化当前窗口
   */
  minimize () {
    const focusedBrowserWindow = getFocusedBrowserWindow()

    if (focusedBrowserWindow) {
      focusedBrowserWindow.minimize()
    }
  },
  webContents: {
    /**
     * 关闭当前窗口的开发者工具
     */
    closeDevTools () {
      const focusedBrowserWindow = getFocusedBrowserWindow()

      if (focusedBrowserWindow) {
        const { webContents } = focusedBrowserWindow

        if (webContents) {
          webContents.closeDevTools()
        }
      }
    },
    /**
     * 当前窗口的开发者工具是否打开
     * @return {boolean}
     */
    isDevToolsOpened () {
      const focusedBrowserWindow = getFocusedBrowserWindow()

      if (focusedBrowserWindow) {
        const { webContents } = focusedBrowserWindow

        if (webContents) {
          return webContents.isDevToolsOpened()
        }
      }
    },
  },
}
