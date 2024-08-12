const path = require('path')
const net = require('net')
const { exec } = require('child_process')
const {
  app,
  BrowserWindow,
  BrowserView,
  ipcMain,
  protocol,
} = require('electron')
const { getFilePath, getFileProtocolPath } = require('../common/util')
const {
  AppPath,
  AppUserDataPath,
  ProjectPath,
  UserDataPath,
  CrossBrowserWindowMessageChannel,
} = require('../common/constants')
const { upgradePresetSensor } = require('../main/upgradePresetSensor')
const { DefaultContent, initDefaultContent } = require('../common/default')
const {
  upgradeService,
  checkVersionSync,
  isNeedToUpgrade,
  compareInstallVersion,
} = require('./upgrade-service')
const { startService, stopService } = require('./service-manager')
require('./ipc')
const { upgradeDir, ensureCacheSync } = require('./upgrade-dir')
const ensureBasicUserData = require('./user-data')

const { platform } = process
const { NODE_ENV = 'production' } = process.env

let _compareInstallVersionRes = null

/**
 * 主窗口配置
 * @type {{
 * skeleton: null,
 * skeletonUrl: {development: string, production: string},
 * name: string, window: null,
 * preload: {development: string | *, production: string | *},
 * mainUrl: {development: string, production: string}
 * }}
 */
const mainEditor = {
  name: 'main',
  window: null,
  skeleton: null,
  preload: {
    development: getFilePath('../renderer/preload-main.js'),
    production: getFilePath('preload-main.js'),
  },
  mainUrl: {
    development: 'http://127.0.0.1:8080',
    production: getFileProtocolPath('index.html'),
  },
  skeletonUrl: {
    development: getFileProtocolPath('../renderer/skeleton-main.html'),
    production: getFileProtocolPath('skeleton-main.html'),
  },
}
// 地图编辑器配置
const mapEditor = {
  name: 'map-editor',
  window: null,
  skeleton: null,
  preload: {
    development: getFilePath('../renderer/preload-map-editor.js'),
    production: getFilePath('preload-map-editor.js'),
  },
  mainUrl: {
    development: 'http://127.0.0.1:8081',
    production: getFileProtocolPath('map-editor/index.html'),
  },
  skeletonUrl: {
    development: getFileProtocolPath('../renderer/skeleton-map-editor.html'),
    production: getFileProtocolPath('skeleton-map-editor.html'),
  },
}

// tadsim 后端服务 host 和 port
const serviceHost = '127.0.0.1'
const servicePort = 9000

// 定时检测端口服务是否完全启动
function checkServicePortReady (ms) {
  let timer = null
  let sock = null
  let isReady = false
  let isConnected = false
  let count = 0
  const totalWaitTime = 30000
  const totalCheckCount = totalWaitTime / ms
  return new Promise((resolve, reject) => {
    timer = setInterval(() => {
      sock && sock.destroy()
      if (isReady && isConnected) {
        if (timer) {
          clearInterval(timer)
          timer = null
        }
        count = null
        resolve()
      } else {
        // 如果启动时间过长，则直接视为后台服务挂了，保证前端界面能够打开
        if (count > totalCheckCount) {
          if (timer) {
            clearInterval(timer)
            timer = null
          }
          count = null
          reject(new Error('start tadsim service timed out.'))
        }
        isConnected = false
        isReady = false
        sock = new net.Socket()
        sock.on('connect', () => {
          isConnected = true
        }).on('ready', () => {
          isReady = true
        }).on('error', () => {
          isConnected = false
          isReady = false
        }).connect(servicePort, serviceHost)
      }
      count++
    }, ms)
  })
}

// TODO 根据用户调整的宽高来初始化视图尺寸
const browserWindowOptions = {
  width: 1200,
  height: 720,
  minWidth: 1200,
  minHeight: 720,
  title: 'TADSim',
  icon: getFilePath('../../build/icons/256x256.png'),
  show: false,
  frame: false,
  backgroundColor: '#020202',
}

/**
 * 创建浏览器窗口
 * @param isMain
 * @return {Promise<void>}
 */
async function createBrowserWindow (isMain) {
  // 获取不同 window 和 env 对应的配置
  const current = isMain ? mainEditor : mapEditor
  const preload = current.preload[NODE_ENV]
  const mainUrl = current.mainUrl[NODE_ENV]
  const skeletonUrl = current.skeletonUrl[NODE_ENV]
  // init BrowserWindow
  const browserWindow = new BrowserWindow({
    ...browserWindowOptions,
    webPreferences: {
      // nativeWindowOpen: true,
      preload,
      webSecurity: false,
      contextIsolation: true,
      additionalArguments: [AppUserDataPath, `--need-to-upgrade=${isNeedToUpgrade()}`],
    },
  })

  // 初始化骨架屏
  const skeletonView = new BrowserView()
  skeletonView.setBackgroundColor(browserWindowOptions.backgroundColor)
  skeletonView.setBounds({
    x: 0,
    y: 0,
    width: browserWindowOptions.width,
    height: browserWindowOptions.height,
  })
  skeletonView.webContents.loadURL(skeletonUrl)

  // 在 dom-ready 前，展示骨架屏
  browserWindow.setBrowserView(skeletonView)

  current.window = browserWindow
  current.skeleton = skeletonView

  const mainProcessParams = {
    AppPath,
    AppUserDataPath,
    UserDataPath,
    ProjectPath,
    DefaultContent,
  }
  // 主进程向渲染进程，同步主进程的变量
  browserWindow.webContents.send(
    'sync-main-process-params',
    JSON.stringify(mainProcessParams),
  )

  try {
    if (isMain) {
      // 在主进程启动时，设置环境变量
      process.env.TADSIM = AppPath
      process.env.TADSIM_DATA = AppUserDataPath
      if (platform === 'win32') {
        await exec(`setx "TADSIM" "${AppPath}"`)
        await exec(`setx "TADSIM_DATA" "${AppUserDataPath}"`)
      }

      // 每次启动后台服务前，先保证前端必要文件存在
      await ensureBasicUserData()

      // 如果有版本更新，则执行后端 service 更新
      if (isNeedToUpgrade()) {
        await upgradeService()
      }

      startService()

      // 等待后台服务完全启动，再打开前端页面
      await checkServicePortReady(500)
    }
  } catch (err) {
    console.log('start service failed: ', err)
  }

  browserWindow.show()
  browserWindow.loadURL(mainUrl)
}

/**
 * 添加 window 事件监听
 * @param isMain
 */
function addWindowListener (isMain) {
  const current = isMain ? mainEditor : mapEditor
  const { window: curWindow, name, skeleton } = current

  if (NODE_ENV === 'development') {
    curWindow.webContents.openDevTools({
      mode: 'right',
    })
  }

  curWindow.webContents.once('dom-ready', () => {
    if (skeleton) {
      curWindow.removeBrowserView(skeleton)
      skeleton.webContents.destroy()
    }
  })

  curWindow.webContents.on('did-fail-load', () => {
    console.error(`${name} browser window did-fail-load`)
  })
  curWindow.webContents.on('unresponsive', () => {
    console.error(`${name} browser window unresponsive`)
  })
  curWindow.webContents.on('responsive', () => {
    console.error(`${name} browser window responsive`)
  })

  curWindow.on('closed', () => {
    current.window = null
    if (current.name === 'main' && mapEditor.window) {
      mapEditor.window.close()
    }
    if (current.name === 'map-editor' && mainEditor.window) {
      // 向主编辑器发送地图编辑器 close 的事件
      mainEditor.window.webContents.send(CrossBrowserWindowMessageChannel, [
        'map-editor-closed',
        true,
      ])
    }
  })
}

/**
 * 创建主窗口
 */
function createMain () {
  createBrowserWindow(true)
  addWindowListener(true)
}

/**
 * 创建地图编辑器窗口
 */
function createMapEditor () {
  createBrowserWindow(false)
  addWindowListener(false)
}

// 初始化 app listener 的方法必须是同步的
function init () {
  initDefaultContent()

  const enableHardwareAcceleration = DefaultContent.getPermission && DefaultContent.getPermission('config.app.hardwareAcceleration.enable')
  if (!enableHardwareAcceleration) {
    app.disableHardwareAcceleration()
  }

  process.env.no_proxy = 'localhost,127.0.0.1,::1'
  process.env.NO_PROXY = 'localhost,127.0.0.1,::1'

  if (platform !== 'win32') {
    process.env.LD_LIBRARY_PATH = ProjectPath.serviceNodeDeps
  }

  app.commandLine.appendSwitch('enable-unsafe-es3-apis')
  app.commandLine.appendSwitch('disable-features', 'OutOfBlinkCors')

  const gotTheLock = app.requestSingleInstanceLock()

  if (!gotTheLock) {
    app.quit()
  }

  // 在覆写用户目录 pkg 配置文件之前，先判断用户目录 pkg version。此次判断的是低于 2.55.xxx 的版本
  _compareInstallVersionRes = compareInstallVersion('2.55.9999', '<=')

  // 优先判断是否需要做版本升级
  checkVersionSync()
  isNeedToUpgrade() && ensureCacheSync()

  // 调整 userData 目录
  const originUserDataPath = app.getPath('userData')
  const newUserDataPath = path.resolve(AppUserDataPath, './cache')
  if (originUserDataPath !== newUserDataPath) {
    app.setPath('userData', newUserDataPath)
  }

  initIpcMainListener()
  initAppListener()
  initProcessListener()
}

function initIpcMainListener () {
  ipcMain.handle('map-editor', (event, methodName, ...args) => {
    if (methodName === 'open') {
      if (mapEditor.window) {
        if (mapEditor.window.isMinimized()) {
          mapEditor.window.restore()
        }

        mapEditor.window.focus()
      } else {
        createMapEditor()
      }
    } else if (methodName === 'focus') {
      mapEditor.window.focus()
    } else if (methodName === 'isOpened') {
      return mapEditor.window !== null
    }
  })

  ipcMain.on(CrossBrowserWindowMessageChannel, (event, args) => {
    const { sender } = event
    const allWindows = BrowserWindow.getAllWindows()
    allWindows.forEach((win) => {
      if (win.webContents !== sender) {
        win.webContents.send(CrossBrowserWindowMessageChannel, args)
      }
    })
    // const { window: mainWindow } = mainEditor
    // const { window: mapEditorWindow } = mapEditor
    // if (mainWindow && mainWindow.webContents !== sender) {
    //   mainWindow.webContents.send(CrossBrowserWindowMessageChannel, args)
    // }
    // if (mapEditorWindow && mapEditorWindow.webContents !== sender) {
    //   mapEditorWindow.webContents.send(CrossBrowserWindowMessageChannel, args)
    // }
  })
}

function initAppListener () {
  app.once('ready', async () => {
    // 判断版本更新后，升级目录
    if (isNeedToUpgrade()) {
      await upgradeDir()
    }

    if (_compareInstallVersionRes) {
      upgradePresetSensor()
    }

    protocol.registerFileProtocol('file', (request, callback) => {
      let requestUrl = request.url.replace('file:///', '')

      requestUrl = decodeURI(requestUrl)
      callback(requestUrl)
    })
    // 创建主视图
    createMain()
  })
  app.on('window-all-closed', () => {
    if (platform !== 'darwin') {
      app.quit()
    }
  })
  app.on('will-quit', () => {
    stopService()
  })
  app.on('activate', () => {
    if (mainEditor.window === null) {
      createMain()
    }
  })
  app.on('second-instance', () => {
    if (mainEditor.window) {
      if (mainEditor.window.isMinimized()) {
        mainEditor.window.restore()
      }

      mainEditor.window.focus()
    }
  })
  app.on('gpu-process-crashed', (event, killed) => {
    console.error('gpu-process-crashed')
    console.error(event)
    console.error(killed)
  })
  app.on('renderer-process-crashed', (event, webContents, killed) => {
    console.error('renderer-process-crashed')
    console.error(event)
    console.error(killed)
  })
}

function initProcessListener () {
  process.on('unhandledRejection', (reason, promise) => {
    console.error('unhandledRejection:')
    console.error(reason)
    console.error(promise)
  })
  process.on('uncaughtException', (err) => {
    console.error('uncaughtException:')
    console.error(err)
  })
}

init()
