const { join } = require('path')
const commonConfig = require('./webpack-common-config')

/**
 * @description 主进程webpack配置
 */
const mainConfig = {
  ...commonConfig,
  entry: {
    main: join(__dirname, 'src/main/index.js'),
  },
  target: 'electron-main',
}

module.exports = mainConfig
