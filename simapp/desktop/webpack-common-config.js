const { join } = require('path')

/**
 * @description 通用webpack配置
 */
const commonConfig = {
  mode: 'production',
  node: {
    __dirname: false,
    __filename: false,
  },
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: join(__dirname, 'build/electron'),
  },
  resolve: {
    extensions: ['.js', '.json'],
  },
}

module.exports = commonConfig
