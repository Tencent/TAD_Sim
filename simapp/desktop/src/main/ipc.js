const electron = require('electron')
const { channels } = require('./ipc-channel')

const { ipcMain } = electron

/**
 * 获取 electron 方法
 * @param object
 * @param path
 * @return {*}
 */
function getMethod (object, path) {
  if (!path) {
    return
  }

  const paths = path.split('.')
  let parent = object

  for (const currentPath of paths) {
    if (parent[currentPath]) {
      parent = parent[currentPath]
    } else {
      return
    }
  }

  return parent
}

/**
 * 调用 electron 方法
 * @param path
 * @param args
 * @return {*}
 */
function invokeMethod (path, args) {
  const customMethod = channels[path]
  const electronMethod = getMethod(electron, path)
  const method = customMethod || electronMethod

  if (method) {
    return method(...args)
  } else {
    throw new Error(`there is no method corresponding to the path '${path}' in main process`)
  }
}

ipcMain.handle('invokeMainMethod', (event, path, args) => {
  return invokeMethod(path, args)
})
