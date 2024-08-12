const { close, maximize, unmaximize, isMaximized, minimize, webContents: { closeDevTools, isDevToolsOpened } } = require('./focused-browser-window')
const { createWindow, writeToPDF, closeWindow, closeWindowById, loadURLById, writeToPDFById } = require('./modal-browser-window')

/**
 * 主进程与渲染进程通信的通道
 */
const channels = {
  'focusedBrowserWindow.close': close,
  'focusedBrowserWindow.maximize': maximize,
  'focusedBrowserWindow.unmaximize': unmaximize,
  'focusedBrowserWindow.isMaximized': isMaximized,
  'focusedBrowserWindow.minimize': minimize,
  'focusedBrowserWindow.webContents.closeDevTools': closeDevTools,
  'focusedBrowserWindow.webContents.isDevToolsOpened': isDevToolsOpened,
  'ModalBrowserWindow.createWindow': createWindow,
  'ModalBrowserWindow.writeToPDF': writeToPDF,
  'ModalBrowserWindow.closeWindow': closeWindow,
  'ModalBrowserWindow.closeWindowById': closeWindowById,
  'ModalBrowserWindow.loadURLById': loadURLById,
  'ModalBrowserWindow.writeToPDFById': writeToPDFById,
}

module.exports = {
  channels,
}
