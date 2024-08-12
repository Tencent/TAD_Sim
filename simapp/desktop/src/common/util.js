const { join, isAbsolute } = require('path')
const { format } = require('url')

function noop () {}

// __dirname 是基于当前 index.js 位置
// AppPath = join(__dirname, '../..')，相当于获取到 desktop 的根目录
function getFilePath (...paths) {
  return join(__dirname, ...paths) // index.js __dirname, not current file
}

/**
 * 获取文件协议路径
 * @param path
 * @return {string}
 */
function getFileProtocolPath (path) {
  let pathname = ''

  if (!isAbsolute(path)) {
    pathname = getFilePath(path)
  }

  return format({
    protocol: 'file',
    slashes: true,
    pathname,
  })
}

/**
 * 获取当前进程类型
 * @return {*}
 */
function getProcessType () {
  let { type } = process
  if (type) {
    if (type === 'browser') {
      type = 'main'
    }
  } else {
    throw new Error('process type is unknown')
  }

  return type
}

/**
 * 等待Promise
 * @param time
 * @return {Promise<unknown>}
 */
function wait (time) {
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      resolve()
    }, time)
  })
}

module.exports = {
  noop,
  getFilePath,
  getFileProtocolPath,
  getProcessType,
  wait,
}
