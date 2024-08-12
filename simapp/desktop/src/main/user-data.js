const path = require('path')
const fse = require('fs-extra')
const { DefaultContent } = require('../common/default')
const { AppUserDataPath } = require('../common/constants')

/**
 * 每次启动，都需要确保必要的用户数据存在
 * @return {Promise<void>}
 */
async function ensureBasicUserData () {
  try {
    // 确保用户目录下 ./sys/local_service.config 文件内容路径模版字段已经替换
    const serviceConfigTarget = path.resolve(
      AppUserDataPath,
      './sys/local_service.config',
    )
    const defaultServiceConfig = DefaultContent.serviceConfig
    !fse.existsSync(serviceConfigTarget) && fse.ensureFileSync(serviceConfigTarget)
    return fse.writeFile(serviceConfigTarget, defaultServiceConfig, 'utf-8')
  } catch (err) {
    console.log('ensure basic user data error: ', err)
  }
}

module.exports = ensureBasicUserData
