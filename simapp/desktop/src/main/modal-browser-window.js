const { writeFile } = require('fs')
const { promisify } = require('util')
const { BrowserWindow } = require('electron')
const { AppUserDataPath } = require('../common/constants')
const { getFilePath } = require('../common/util')

const { NODE_ENV = 'production' } = process.env

const writeSync = promisify(writeFile)
// 创建窗口的preload文件，注入原生能力
const preload = {
  development: getFilePath('../renderer/preload-main.js'),
  production: getFilePath('preload-main.js'),
}
const defaultBrowserWindowOptions = {
  width: 800,
  height: 480,
  resizable: false,
  title: 'TAD Sim',
  show: false,
  frame: false,
  parent: null,
  modal: true,
  backgroundColor: '#020202',
  webPreferences: {
    preload: preload[NODE_ENV],
    contextIsolation: true,
    additionalArguments: [AppUserDataPath],
  },
}
const defaultPrintToPDFOptions = {
  printBackground: true,
}

/**
 * @class ModalBrowserWindow
 * @description 模态窗口
 */
class ModalBrowserWindow {
  constructor (options) {
    this.browserWindow = new BrowserWindow({
      ...defaultBrowserWindowOptions,
      ...options,
    })
    this.webContents = this.browserWindow.webContents
    if (options.show !== false) {
      // 传入 show 参数为false时，不自动显示
      this.browserWindow.on('ready-to-show', () => {
        this.browserWindow.show()
      })
    }
  }

  /**
   * 加载URL
   * @param url
   * @return {Promise<void>}
   */
  loadURL (url) {
    const p = this.browserWindow.loadURL(url)

    if (NODE_ENV === 'development') {
      this.webContents.openDevTools({
        mode: 'undocked',
      })
    }
    return p
  }

  /**
   * 导出为PDF
   * @param browserWindow
   * @param file
   * @param options
   * @return {Promise<*|null>}
   */
  static async writeToPDF (browserWindow, file, options) {
    try {
      const data = await browserWindow.webContents.printToPDF({
        ...defaultPrintToPDFOptions,
        ...options,
      })
      await writeSync(file, data)

      return null
    } catch (err) {
      return err.message
    }
  }

  /**
   * 关闭窗口
   * @param browserWindow
   */
  static close (browserWindow) {
    const parentBrowserWindow = browserWindow.getParentWindow()

    if (parentBrowserWindow) {
      parentBrowserWindow.focus() // https://blog.walterlv.com/post/fix-owner-window-dropping-down-when-close-a-modal-child-window.html
    }
    browserWindow.close()
  }

  writeToPDF (file, options) {
    return ModalBrowserWindow.writeToPDF(this.browserWindow, file, options)
  }

  close () {
    return ModalBrowserWindow.close(this.browserWindow)
  }
}

module.exports = {
  createWindow (url, options, setParent = true) {
    const modalBrowserWindow = new ModalBrowserWindow({
      ...options,
      parent: setParent ? BrowserWindow.getFocusedWindow() : null,
    })
    if (url) {
      modalBrowserWindow.loadURL(url)
    }

    return modalBrowserWindow.browserWindow.id
  },
  writeToPDF (...args) {
    const modalBrowserWindow = BrowserWindow.getFocusedWindow()

    return ModalBrowserWindow.writeToPDF(modalBrowserWindow, ...args)
  },
  closeWindow (...args) {
    const modalBrowserWindow = BrowserWindow.getFocusedWindow()

    return ModalBrowserWindow.close(modalBrowserWindow, ...args)
  },
  closeWindowById (id, ...args) {
    const modalBrowserWindow = BrowserWindow.fromId(id)
    return ModalBrowserWindow.close(modalBrowserWindow, ...args)
  },
  loadURLById (id, url) {
    const modalBrowserWindow = BrowserWindow.fromId(id)
    return modalBrowserWindow.loadURL(url)
  },
  writeToPDFById (id, ...args) {
    const modalBrowserWindow = BrowserWindow.fromId(id)
    return ModalBrowserWindow.writeToPDF(modalBrowserWindow, ...args)
  },
}
