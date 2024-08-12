const { app } = require('electron')
const {
  default: installExtension,
  VUEJS_DEVTOOLS,
} = require('electron-devtools-installer')

process.env.NODE_ENV = 'development'

/**
 * 开发模式的启动入口
 */
app.whenReady().then(() => {
  installExtension(VUEJS_DEVTOOLS)
    .then((name) => {
      console.log(`add extension: ${name}`)
    })
    .catch((err) => {
      console.log('devtools error: ', err)
    })
})

require('./index')
