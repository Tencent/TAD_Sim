const process = require('process')
const path = require('path')
const fse = require('fs-extra')
const _ = require('lodash')
const { ProjectPath, UserDataPath } = require('./constants')
const { getFilePath } = require('./util')

const DefaultContent = {}
const { platform } = process

/**
 * 获取web-config配置
 * @return {{}|any}
 */
function getWebConfigData () {
  const webConfigPath = getFilePath('../../web-config.json')
  try {
    const data = fse.readJsonSync(webConfigPath)
    return data
  } catch (err) {
    console.error(err)
  }

  return {}
}

function getPermission (key) {
  const config = DefaultContent.webConfig || {}

  const has = _.has(config, key)
  if (!has) {
    // 无法区分 key 是写错了不存在，还是打包时剔除了，所以就不警告
    // console.warn(`请检查 ${key} 是否存在于 web-config.json 中`)
    return false
  }

  const res = _.get(config, key)
  return res
}
/**
 * 格式化service配置文件
 * @return {string}
 */
function formatServiceConfig () {
  let platformService = ProjectPath.service
  let platformServiceData = UserDataPath.serviceData
  let platformUserLog = UserDataPath.userLog

  if (platform === 'win32') {
    const regExp = new RegExp(`\\${path.sep}`, 'g')

    platformService = platformService.replace(regExp, path.sep + path.sep)
    platformServiceData = platformServiceData.replace(
      regExp,
      path.sep + path.sep,
    )
    platformUserLog = platformUserLog.replace(regExp, path.sep + path.sep)
  }

  try {
    let content = fse.readFileSync(
      path.join(platformService, 'local_service.config.in'),
      'utf8',
    )

    content = content.replace(/@\{InstallDirectory\}/g, platformService)
    content = content.replace(/@\{appDataPath\}/g, platformServiceData)
    content = content.replace(/@\{ModuleLogDirectory\}/g, platformUserLog)

    return content
  } catch (err) {
    console.error(err)
  }

  return ''
}

/**
 * 初始化默认配置
 */
function initDefaultContent () {
  DefaultContent.webConfig = getWebConfigData()
  DefaultContent.getPermission = getPermission
  DefaultContent.serviceConfig = formatServiceConfig()
  DefaultContent.recentOpened = JSON.stringify([], null, 2)
  const defaultSensorConfig = {
    sensorUseGlobal: false,
    sensorGlobalFilePath: 'sensor_global.xml',
    environmentUseGlobal: false,
    environmentGlobalFilePath: 'environment_global.xml',
  }
  DefaultContent.sensorConfigObj = defaultSensorConfig
  DefaultContent.sensorConfig = JSON.stringify(defaultSensorConfig, null, 2)
  const defaultWarningConfig = {
    vc2010: true,
    vc2015: true,
    nvidiaDriver: true,
  }
  DefaultContent.warningConfigObj = defaultWarningConfig
  DefaultContent.warningConfig = JSON.stringify(defaultWarningConfig, null, 2)
}

module.exports = { DefaultContent, initDefaultContent }
