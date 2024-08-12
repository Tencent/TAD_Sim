const { join } = require('path')
const { readFileSync, writeFileSync, readFile, writeFile, existsSync } = require('fs')
const util = require('util')
const { shell } = require('electron')
const { UserDataPath } = require('../common/constants')
const { DefaultContent } = require('../common/default')

const readFileAsync = util.promisify(readFile)
const writeFileAsync = util.promisify(writeFile)

const MAX_SIZE = 100
let recentOpened = null

/**
 * @description 保存最近打开的文件
 */
function saveRecentOpened () {
  try {
    writeFileSync(
      UserDataPath.recentOpened,
      JSON.stringify(recentOpened, null, 2),
    ) // It is unsafe to use fs.writeFile() multiple times on the same file without waiting for the callback.
  } catch (err) {
    console.error(err)
  }
}

const recent = {
  /**
   * @description 获取最近打开的文件
   */
  get () {
    if (recentOpened === null) {
      try {
        const data = readFileSync(UserDataPath.recentOpened, 'utf8')

        recentOpened = JSON.parse(data)
      } catch (err) {
        console.error(err)
      }
    }

    return recentOpened
  },
  /**
   * @description 添加最近打开的文件
   * @param name
   */
  add (name) {
    if (!name) {
      return
    }

    if (recentOpened === null) {
      recentOpened = []
    }

    const index = recentOpened.indexOf(name)

    if (index !== -1) {
      recentOpened.splice(index, 1)
    }

    recentOpened.unshift(name)

    if (recentOpened.length > MAX_SIZE) {
      recentOpened.pop()
    }

    saveRecentOpened()
  },
  /**
   * @description 删除最近打开的文件
   * @param names
   */
  del (names) {
    if (!names || names.length === 0) {
      return
    }

    if (recentOpened === null) {
      recentOpened = []
    }

    names.forEach((name) => {
      const index = recentOpened.indexOf(name)

      if (index !== -1) {
        recentOpened.splice(index, 1)
      }
    })

    saveRecentOpened()
  },
}

const sensor = {
  config: null,
  /**
   * @description 加载传感器配置
   * @return {Promise<any>}
   */
  loadConfig () {
    if (sensor.config) {
      return Promise.resolve(sensor.config)
    }
    return readFileAsync(UserDataPath.sensorConfig, 'utf8')
      .then((data) => {
        try {
          data = JSON.parse(data)
        } catch (e) {
          data = DefaultContent.sensorConfigObj
        }
        return { ...DefaultContent.sensorConfigObj, ...data }
      })
      .catch(() => DefaultContent.sensorConfigObj)
      .then((data) => {
        sensor.config = data
        return sensor.config
      })
  },
  /**
   * @description 保存传感器配置
   * @param params
   * @return {Promise<Awaited<unknown>[]>}
   */
  saveConfig (params) {
    return sensor.loadConfig().then((config) => {
      // 只有 defaultSensorConfig 规定的字段才会合并到 config
      Object.keys(DefaultContent.sensorConfigObj).forEach((key) => {
        if (params[key] !== undefined) {
          config[key] = params[key]
        }
      })
      return Promise.all([
        writeFileAsync(
          UserDataPath.sensorConfig,
          JSON.stringify(config, null, 2),
        ),
        writeFileAsync(
          UserDataPath.systemSensorConfigBackup,
          JSON.stringify(config, null, 2),
        ),
      ])
    })
  },
}

const warning = {
  config: null,
  /**
   * @description 加载告警配置
   * @return {Promise<any>}
   */
  loadConfig () {
    if (warning.config) {
      return Promise.resolve(warning.config)
    }
    return readFileAsync(UserDataPath.warningConfig, 'utf8')
      .then((data) => {
        try {
          data = JSON.parse(data)
        } catch (e) {
          data = DefaultContent.warningConfigObj
        }
        return { ...DefaultContent.warningConfigObj, ...data }
      })
      .catch(() => DefaultContent.warningConfigObj)
      .then((data) => {
        warning.config = data
        return warning.config
      })
  },
  /**
   * @description 保存告警配置
   * @param params
   * @return {Promise<Awaited<unknown>[]>}
   */
  saveConfig (params) {
    return warning.loadConfig().then((config) => {
      // 只有 defaultWarningConfig 规定的字段才会合并到 config
      Object.keys(DefaultContent.warningConfigObj).forEach((key) => {
        if (params[key] !== undefined) {
          config[key] = params[key]
        }
      })
      return writeFileAsync(
        UserDataPath.warningConfig,
        JSON.stringify(config, null, 2),
      )
    })
  },
  /**
   * @description 重置告警配置
   * @return {Promise<unknown>}
   */
  resetAll () {
    return writeFileAsync(
      UserDataPath.warningConfig,
      JSON.stringify(DefaultContent.warningConfig, null, 2),
    )
  },
}

/**
 * @description 打开批量生成场景的文件夹
 * @param scenarioFolderName
 * @return {boolean}
 */
function openBatchGenerateScenarioPath (scenarioFolderName = '') {
  const scenarioFolderPath = join(UserDataPath.batchGenerateScenario, scenarioFolderName)

  // 优先判断要打开的路径是否存在
  if (!existsSync(scenarioFolderPath)) return false

  shell.showItemInFolder(scenarioFolderPath)
  return true
}

/**
 * @description 打开kpi报告文件夹
 * @param kpiReportFolderName
 */
function openKpiReportPath (kpiReportFolderName = '') {
  shell.showItemInFolder(join(UserDataPath.kpiReport, kpiReportFolderName))
}
module.exports = {
  recent,
  sensor,
  warning,
  openBatchGenerateScenarioPath,
  openKpiReportPath,
  writeFileSync,
}
