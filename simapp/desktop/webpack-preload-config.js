const { join } = require('path')
const commonConfig = require('./webpack-common-config')

/**
 * @description 预加载脚本webpack配置
 */
const preloadConfig = {
  ...commonConfig,
  entry: {
    'preload-main': join(__dirname, 'src/renderer/preload-main.js'),
    'preload-map-editor': join(__dirname, 'src/renderer/preload-map-editor.js'),
  },
  target: 'electron-preload',
}

module.exports = preloadConfig
