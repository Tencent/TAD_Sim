const { sep, join } = require('path')
const fse = require('fs-extra')
const { contextBridge } = require('electron')
const { AppUserDataPath } = require('../common/constants')
const { DefaultContent, initDefaultContent } = require('../common/default')
const { invoke } = require('./ipc')
const crossBrowserWindowMessage = require('./cross-browser-window-message')
const tadsim = require('./tadsim')

const { versions, platform } = process
initDefaultContent()

/**
 * @description 暴露api到渲染进程
 * @return {Promise<*>}
 */
async function exposeApi () {
  const installPkg = fse.readJSONSync(join(AppUserDataPath, './sys/package.json'))
  const { language = 'en' } = installPkg

  try {
    const api = {
      versions,
      platform,
      path: {
        sep,
      },
      focusedBrowserWindow: {
        /**
         * @description 关闭当前窗口
         */
        close () {
          invoke('focusedBrowserWindow.close')
        },
        /**
         * @description 最大化当前窗口
         */
        maximize () {
          invoke('focusedBrowserWindow.maximize')
        },
        /**
         * @description 取消最大化当前窗口
         */
        unmaximize () {
          invoke('focusedBrowserWindow.unmaximize')
        },
        isMaximized () {
          return invoke('focusedBrowserWindow.isMaximized')
        },
        minimize () {
          invoke('focusedBrowserWindow.minimize')
        },
        webContents: {
          closeDevTools () {
            invoke('focusedBrowserWindow.webContents.closeDevTools')
          },
          isDevToolsOpened () {
            return invoke('focusedBrowserWindow.webContents.isDevToolsOpened')
          },
        },
      },
      dialog: {
        showOpenDialog (...args) {
          return invoke('dialog.showOpenDialog', ...args)
        },
      },
      crossBrowserWindowMessage,
      tadsim,
      // TODO: 后续正式环境可干掉
      webConfig: Object.freeze(DefaultContent.webConfig),
      getPermission: DefaultContent.getPermission,
      env: {
        TADSIM_LANGUAGE: language,
      },
    }
    contextBridge.exposeInMainWorld('electron', api)
  } catch (err) {
    console.error(err)
  }
}

console.log('process.versions: ', process.versions)

exposeApi()
