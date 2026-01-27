const { sep, join, resolve } = require('path')
const fse = require('fs-extra')
const { contextBridge, ipcRenderer, shell } = require('electron')
const {
  AppPath,
  AppUserDataPath,
  UserDataPath,
} = require('../common/constants')
const { DefaultContent, initDefaultContent } = require('../common/default')
const { invoke } = require('./ipc')
const crossBrowserWindowMessage = require('./cross-browser-window-message')
const serviceNode = require('./service-node')
const userLog = require('./user-log')
const editor = require('./editor')
const tadsim = require('./tadsim')

const { versions, platform } = process
const { system, modules, player, kpi } = serviceNode

let moduleNames = []
initDefaultContent()

/**
 * @description 设置模块名称
 */
function setModuleNames () {
  try {
    const data = fse.readFileSync(UserDataPath.serviceConfig, 'utf8')
    const serviceConfig = JSON.parse(data)

    moduleNames = serviceConfig.configs.map(item => item.name)
  } catch (err) {
    console.error('set module names error: ', err)
  }
}

/**
 * @description 判断是否为预置模块
 * @param moduleName
 * @return {boolean}
 */
function isPreset (moduleName) {
  return modules.isPreset(moduleNames, moduleName)
}

/**
 * @description 打开用户日志文件
 * @param names
 * @param callback
 */
function openUserLogFiles (names, callback) {
  userLog.openFiles(names, UserDataPath.userLog, callback)
}

/**
 * @description 在文件管理器中显示文件
 * @param name
 */
function showItemInFolder (name) {
  shell.showItemInFolder(join(UserDataPath.userLog, name))
}

/**
 * @description 打开用户日志文件夹
 */
function openUserLogFolder (path = UserDataPath.userLog) {
  shell.openPath(path)
}

function copyFile (destination) {
  const source = resolve(AppUserDataPath, './sys/scenario/场景语义生成模板.xlsx')
  fse.copyFileSync(source, destination)
}

/**
 * @description 暴露api到渲染进程
 * @return {Promise<any|boolean>}
 */
async function exposeApi () {
  try {
    const installPkg = fse.readJSONSync(join(AppUserDataPath, './sys/package.json'))
    const { language = 'en' } = installPkg

    const api = {
      versions,
      platform,
      path: {
        sep,
      },
      app: {
        userDataPath: AppUserDataPath,
      },
      focusedBrowserWindow: {
        close () {
          invoke('focusedBrowserWindow.close')
        },
        maximize () {
          invoke('focusedBrowserWindow.maximize')
        },
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
        showSaveDialog (...args) {
          return invoke('dialog.showSaveDialog', ...args)
        },
      },
      ModalBrowserWindow: {
        createWindow (...args) {
          return invoke('ModalBrowserWindow.createWindow', ...args)
        },
        writeToPDF (...args) {
          return invoke('ModalBrowserWindow.writeToPDF', ...args)
        },
        closeWindow () {
          return invoke('ModalBrowserWindow.closeWindow')
        },
        closeWindowById (...args) {
          return invoke('ModalBrowserWindow.closeWindowById', ...args)
        },
        loadURLById (...args) {
          return invoke('ModalBrowserWindow.loadURLById', ...args)
        },
        writeToPDFById (...args) {
          return invoke('ModalBrowserWindow.writeToPDFById', ...args)
        },
      },
      crossBrowserWindowMessage,
      modules: {
        ...modules,
        isPreset,
      },
      editor,
      player,
      mapEditor: {
        open () {
          ipcRenderer.invoke('map-editor', 'open')
        },
        focus () {
          ipcRenderer.invoke('map-editor', 'focus')
        },
        isOpened () {
          return ipcRenderer.invoke('map-editor', 'isOpened')
        },
      },
      kpi,
      system,
      userLog: {
        ...userLog,
        openFiles: openUserLogFiles,
        showItemInFolder,
        openUserLogFolder,
        copyFile,
      },
      tadsim,
      // TODO: 后续正式环境可干掉
      webConfig: Object.freeze(DefaultContent.webConfig),
      getPermission: DefaultContent.getPermission,
      env: {
        TADSIM_LANGUAGE: language,
      },
      checkPathExist (path) {
        return fse.pathExistsSync(join(AppUserDataPath, path))
      },
    }
    contextBridge.exposeInMainWorld('electron', api)
  } catch (err) {
    console.error(err)
  }
}

console.log('process.versions: ', process.versions)

system.setAppPath(AppUserDataPath, AppPath)
setModuleNames()
exposeApi()
