const { openSync, readFile } = require('fs')
const { join } = require('path')

const MAX_COUNT = 10240
const interval = 1000
let fds = {}
let messages = {}
let callback = null
let readLogFilesTimer = null

/**
 * 打开文件
 * @param dirname
 * @param name
 */
function openFile (dirname, name) {
  const path = join(dirname, name)

  fds[name] = openSync(path)
  messages[name] = ''
}

/**
 * 异步读取文件
 * @param name
 */
function readFileAsync (name) {
  readFile(fds[name], (err, data) => {
    if (err) {
      console.error(err)

      return
    }

    if (data) {
      messages[name] += data

      const length = messages[name].length

      if (length > MAX_COUNT) {
        messages[name] = messages[name].substring(length - MAX_COUNT)
      }

      if (callback) {
        callback(messages)
      }
    }
  })
}

/**
 * 异步读取文件
 */
function readFiles () {
  readLogFilesTimer = setInterval(() => {
    for (const name in fds) {
      readFileAsync(name)
    }
  }, interval)
}

/**
 * 打开文件
 * @param names
 * @param dirname
 * @param fn
 */
function openFiles (names, dirname, fn) {
  callback = fn
  fds = {}
  messages = {}

  names.forEach((name) => {
    openFile(dirname, name)
  })

  clearInterval(readLogFilesTimer)
  readFiles()
}

/**
 * 清空消息
 */
function clearMessage () {
  for (const prop in messages) {
    messages[prop] = ''
  }

  if (callback) {
    callback(messages)
  }
}

module.exports = {
  openFiles,
  clearMessage,
}
