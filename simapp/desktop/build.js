const { EOL } = require('os')
const { join } = require('path')
const { spawn } = require('child_process')
const { copySync, emptyDirSync, removeSync } = require('fs-extra')
const webpack = require('webpack')
const webpackMainConfig = require('./webpack-main-config')
const webpackPreloadConfig = require('./webpack-preload-config')

const NPM = process.platform === 'win32' ? 'npm.cmd' : 'npm'
const DEST = 'build/electron'

/**
 * 格式化输出
 * @param buffer
 * @return {string}
 */
function format (buffer) {
  const str = buffer.toString()
  const reg = new RegExp(`${EOL}$`)

  return str.replace(reg, '')
}

/**
 * 清理产物目录
 * @return {Promise<void>}
 */
function clean () {
  emptyDirSync('build')
  console.log(`> [SUCCESS] rm -rf build/*`)
}

/**
 * 构建 webpack配置
 * @param config
 * @return {Promise<unknown>}
 */
function buildWebPack (config) {
  return new Promise((resolve, reject) => {
    webpack(config, (err, stats) => {
      if (err) {
        reject(err)
      } else {
        const message = stats.toString({
          chunks: false,
          colors: true,
        })

        if (stats.hasErrors()) {
          reject(new Error(message))
        } else {
          console.log(`> [SUCCESS] webpack ${config.target}`)
          resolve()
        }
      }
    })
  })
}

/**
 * 构建 renderer 文件
 * @return {Promise<unknown>}
 */
function buildRenderer () {
  return new Promise((resolve, reject) => {
    try {
      copySync('node_modules/regedit/vbs', `${DEST}/vbs`)
      console.log(`> [SUCCESS] cp -r regedit/vbs ${DEST}/vbs`)
      copySync('src/renderer/skeleton-main.html', `${DEST}/skeleton-main.html`)
      copySync('src/renderer/skeleton-map-editor.html', `${DEST}/skeleton-map-editor.html`)
      console.log(`> [SUCCESS] cp src/renderer/*.html ${DEST}`)
      copySync('src/icons', join(DEST, '..', 'icons'))
      console.log(`> [SUCCESS] cp src/icons, ${join(DEST, '..', 'icons')}`)
      resolve()
    } catch (err) {
      reject(err)
    }
  })
}

/**
 * 构建
 * @return {Promise<void>}
 */
async function build () {
  try {
    await buildWebPack(webpackMainConfig)
    await buildWebPack(webpackPreloadConfig)
    await buildRenderer()
  } catch (err) {
    console.error(`> [ERROR] ${err}`)
    process.exit(1)
  }
}

clean()
build()
