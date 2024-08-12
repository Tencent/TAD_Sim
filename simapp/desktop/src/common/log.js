const { createWriteStream } = require('fs')
const { join } = require('path')
const { Console } = require('console')
const { getProcessType } = require('./util')

/**
 * 创建一个 console 对象
 * @param filename
 * @return {console.Console}
 */
function create (filename) {
  const stream = createWriteStream(filename)

  return new Console(stream)
}

/**
 * 将 electron 的 console 对象赋值到 usingConsole
 * @param usingConsole
 * @param electronConsole
 */
function assign (usingConsole, electronConsole) {
  const { error, warn, log, info, debug } = usingConsole
  const original = {
    error,
    warn,
    log,
    info,
    debug,
  }

  Object.keys(original).forEach((key) => {
    usingConsole[key] = electronConsole[key]
  })

  usingConsole.original = original
}

/**
 * 初始化 console 对象
 * @param usingConsole
 * @param AppUserDataPath
 */
function init (usingConsole = {}, AppUserDataPath) {
  const type = getProcessType()
  const filename = join(AppUserDataPath, `./cache/debug_log/${type}.log`)
  const electronConsole = create(filename)

  assign(usingConsole, electronConsole)
}

module.exports = {
  init,
}
