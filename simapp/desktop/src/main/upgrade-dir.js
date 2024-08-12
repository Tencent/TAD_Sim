const path = require('path')
const fse = require('fs-extra')
const _ = require('lodash')
const { XMLParser, XMLBuilder } = require('fast-xml-parser')
const {
  CacheConfig,
  UserDataConfig,
  AppUserDataPath,
  SystemConfig,
  LogConfig,
  UpdateUserDataConfig,
  UserCatalogsPath,
  SysCatalogsPath,
  AppPath,
} = require('../common/constants')
const { DefaultContent } = require('../common/default')

function ensureCacheSync () {
  const cachePath = path.resolve(AppUserDataPath, './cache')
  fse.ensureDirSync(cachePath)
  // fse.emptyDirSync(cachePath)
}

// 初始化 cache 目录，如果在新的目录结构下已经存在对应的文件，则不需要替换
async function initCache () {
  try {
    // cache 目录下的文件，只要保证文件（文件夹）存在就行
    const initPromise = CacheConfig.map((config) => {
      const { target, sourceContent, isFile } = config
      if (isFile) {
        if (sourceContent && DefaultContent[sourceContent]) {
          !fse.existsSync(target) && fse.ensureFileSync(target)
          return fse.writeFile(target, DefaultContent[sourceContent], 'utf-8')
        }
      } else {
        if (!fse.existsSync(target)) {
          return fse.ensureDir(target)
        }
      }

      return Promise.resolve()
    })

    await Promise.all(initPromise)
  } catch (err) {
    console.log('init cache dir failed: ', err)
  }
}

// sys 目录，每次升级都是重新用安装目录的预设内容覆盖
async function initSystem () {
  try {
    const sysPath = path.resolve(AppUserDataPath, './sys')
    await fse.ensureDir(sysPath)

    const initPromise = SystemConfig.map((config) => {
      const { target, origin, source, sourceContent, isFile, shouldDelete } = config

      if (isFile) {
        if (shouldDelete && target && source && fse.existsSync(source)) {
          // 先备份，再删除
          fse.copySync(source, target)
          return fse.remove(source)
        }
        if (source && fse.existsSync(source)) {
          return fse.copy(source, target)
        } else if (sourceContent && DefaultContent[sourceContent]) {
          !fse.existsSync(target) && fse.ensureFileSync(target)
          return fse.writeFile(target, DefaultContent[sourceContent], 'utf-8')
        } else if (origin && fse.existsSync(origin)) {
          return fse.copy(origin, target)
        }
      } else {
        if (source && fse.existsSync(source)) {
          // 有source的会强制覆盖拷贝
          return fse.copy(source, target)
        } else if (origin && fse.existsSync(origin)) {
          return fse.copy(origin, target)
        } else {
          // 对于文件夹，如果source和target都不存在，只需要保证target目录存在就行
          return fse.ensureDir(target)
        }
      }

      return Promise.resolve()
    })

    await Promise.all(initPromise)
  } catch (err) {
    console.log('init system dir failed: ', err)
  }
}

// 初始化用户目录，如果在新的目录结构下已经存在对应的文件，则不需要替换
async function initUserData () {
  try {
    const dataPath = path.resolve(AppUserDataPath, './data')
    await fse.ensureDir(dataPath)

    const initPromise = UserDataConfig.map(async (config) => {
      const {
        target,
        source,
        origin,
        isFile,
        sourceContent,
        onlyUserData,
        isHadMap,
        targetBackup,
        originBackup,
      } = config
      if (isFile) {
        if (fse.existsSync(origin)) {
          // 如果存在旧目录结构的用户数据，拷贝到新的目录中
          return fse.copy(origin, target)
        } else {
          if (fse.existsSync(target)) {
            // 如果目标路径对应的文件已经存在，则不需要做处理
            return Promise.resolve()
          } else {
            if (source && fse.existsSync(source)) {
              // 如果安装目录中有用户数据的默认配置，则拷贝到新目录中
              return fse.copy(source, target)
            } else if (sourceContent && DefaultContent[sourceContent]) {
              // 如果存在初始文件内容，则直接写入到文件中
              !fse.existsSync(target) && fse.ensureFileSync(target)
              return fse.writeFile(
                target,
                DefaultContent[sourceContent],
                'utf-8',
              )
            } else {
              // 对于安装目录和预设值都不存在的情况，则无须处理
              return Promise.resolve()
            }
          }
        }
      } else {
        if (onlyUserData) {
          !fse.existsSync(target) && fse.ensureDirSync(target)

          if (isHadMap) {
            // 为了兼容后端，需要将 scenario/hadmap 全部拷贝到 data 目录，在用预设的 hadmap 覆盖
            if (origin && fse.existsSync(origin)) {
              const originFiles = await fse.readdir(origin)
              const copyOriginPromise = originFiles.map(filename =>
                fse.copy(
                  path.resolve(origin, filename),
                  path.resolve(target, filename),
                ),
              )
              await Promise.all(copyOriginPromise)
            }

            // 无论是原有的目录结构，还是新的目录结构，每次升级都需要把安装目录预设的 hadmap 重新拷贝到 data 目录中！
            if (source && fse.existsSync(source)) {
              const sourceFiles = await fse.readdir(source)
              const copySourcePromise = sourceFiles.map(filename =>
                fse.copy(
                  path.resolve(source, filename),
                  path.resolve(target, filename),
                ),
              )
              return Promise.all(copySourcePromise)
            }
          } else {
            if (
              origin &&
              source &&
              fse.existsSync(origin) &&
              fse.existsSync(source)
            ) {
              const originFiles = await fse.readdir(origin)
              const sourceFiles = await fse.readdir(source)
              // 对比原目录下的内容跟安装目录下的内容的差异部分，拷贝到用户目录
              const diffs = _.difference(originFiles, sourceFiles)
              const copyFilePromise = diffs.map((filename) => {
                return fse.copy(
                  path.resolve(origin, filename),
                  path.resolve(target, filename),
                )
              })
              return Promise.all(copyFilePromise)
            }
          }
        } else {
          // 普通的拷贝
          if (origin && fse.existsSync(origin)) {
            return fse.copy(origin, target)
          } else if (originBackup && fse.existsSync(originBackup)) {
            // windows 和 linux 系统的部分路径存在偏差，需要针对另一个 origin 路径做兼容
            return fse.copy(originBackup, targetBackup)
          } else {
            if (fse.existsSync(target)) {
              return Promise.resolve()
            } else {
              if (source && fse.existsSync(source)) {
                return fse.copy(source, target)
              } else {
                return fse.ensureDir(target)
              }
            }
          }
        }
      }
      return Promise.resolve()
    })
    await Promise.all(initPromise)
  } catch (err) {
    console.log('init user data dir failed: ', err)
  }
}

const xmlOptions = {
  ignoreAttributes: false,
  attributeNamePrefix: '$_',
  allowBooleanAttributes: true,
}
const readOption = { encoding: 'utf-8' }
const parser = new XMLParser(xmlOptions)
const builder = new XMLBuilder({
  ...xmlOptions,
  format: true,
  suppressBooleanAttributes: false,
  suppressEmptyNode: true,
})

/**
 * 在上一步确认用户文件之后，此步骤只需简单替换某些文件为最新版本即可
 * @returns {Promise<void>}
 */
async function updateUserData () {
  for (let i = 0; i < UpdateUserDataConfig.length; i++) {
    // flag: 有更新才反写
    let flag = false
    const { target } = UpdateUserDataConfig[i]
    const xml = parser.parse(await fse.readFile(target, readOption))
    const catalog = xml.OpenSCENARIO.Catalog
    let tagName
    switch (catalog.$_name) {
      case 'VehicleCatalog':
        tagName = 'Vehicle'
        break
      case 'MiscObjectCatalog':
        tagName = 'MiscObject'
        break
      case 'PedestrianCatalog':
        tagName = 'Pedestrian'
        break
      default:
        console.error('catalog格式错误')
        return
    }
    if (_.isArray(catalog[tagName])) {
      // 多个 Vehicle，则删除preset为 true 的
      catalog[tagName] = _.filter(catalog[tagName], (v) => {
        if (_.isArray(v.Properties.Property)) {
          const preset = _.find(v.Properties.Property, p => p.$_name === 'Preset')
          if (preset && preset.$_value === 'true') {
            flag = true
            return false
          } else if (!v.$_name.startsWith('user_')) {
            // 老版本 vehicle 没有 preset 属性，但不以user_开头，直接删除
            flag = true
            return false
          }
        }
        return true
      })
    } else if (catalog[tagName] && catalog[tagName].Properties.Property) {
      // 如果是单个 vehicle，则判断preset为 true，则删除
      const preset = _.find(catalog[tagName].Properties.Property, p => p.$_name === 'Preset')
      if (preset && preset.$_value === 'true') {
        delete catalog[tagName]
        flag = true
      }
    }
    if (flag) {
      const xmlNew = builder.build(xml)
      await fse.writeFile(target, xmlNew, 'utf-8')
    }
  }
}

// 获取可用的sensorGroupId，目前取id最大值+1，后续可能改为找最小的未使用的id
function getValidSensorGroupId (groups, isPreset) {
  return groups.reduce((max, sg) => {
    const id = +sg.$_name
    // 如果是preset，则取id小于100000的
    if (isPreset && id >= 100000) {
      return max
    } else {
      return Math.max(max, id)
    }
  }, isPreset ? 1 : 100000) + 1
}

/**
 * 更新主车配置
 * @returns {Promise<void>}
 */
async function updateDriverCatalogs () {
  const userDefaultXoscPath = path.resolve(UserCatalogsPath, './Vehicles/default.xosc')
  const userSensorXoscPath = path.resolve(UserCatalogsPath, './Sensors/SensorCatalog.xosc')
  const sysDefaultXoscPath = path.resolve(SysCatalogsPath, './Vehicles/default.xosc')
  const userDynamicsPath = path.resolve(UserCatalogsPath, './Dynamics')
  const sysDynamicsPath = path.resolve(SysCatalogsPath, './Dynamics')

  const userDefaultXosc = await fse.readFile(userDefaultXoscPath, readOption)
  const userDefaultConfig = parser.parse(userDefaultXosc)
  const userSensorXosc = await fse.readFile(userSensorXoscPath, readOption)
  const userSensorConfig = parser.parse(userSensorXosc)
  const sysDefaultXosc = await fse.readFile(sysDefaultXoscPath, readOption)
  const sysDefaultConfig = parser.parse(sysDefaultXosc)

  const userEgoCars = userDefaultConfig.OpenSCENARIO.Catalog[0].Vehicle
  const userCombinationCars = userDefaultConfig.OpenSCENARIO.Catalog[1].Vehicle
  const userSensorGroup = userSensorConfig.TADSim.Catalog[0].SensorGroup

  let sysVehicleCars = sysDefaultConfig.OpenSCENARIO.Catalog[0].Vehicle
  const sysCombinationCars = sysDefaultConfig.OpenSCENARIO.Catalog[1].Vehicle
  // 用flag表示是否有更新，如果没有新增则不复写文件，避免操作用户数据
  let flag = false

  for (let i = 0; i < userEgoCars.length; i++) {
    // 修复用户主车sensorGroup配置，用户的sensorgroupid +100000
    const uv = userEgoCars[i]
    const presetProperty = uv.Properties.Property.find(p => p.$_name === 'Preset')
    const sensorGroupProperty = uv.Properties.Property.find(p => p.$_name === 'SensorGroup')
    if (sensorGroupProperty) {
      let newId
      const id = +sensorGroupProperty.$_value
      if (id < 100000 && (presetProperty && presetProperty.$_value === 'false')) {
        // 如果id小于十万，则加上十万，然后修复sensorgroupid
        newId = `${id + 100000}`
        sensorGroupProperty.$_value = newId
        const group = userSensorGroup.find(sg => sg.$_name === id)
        if (group) {
          group.$_name = newId
        } else {
          userSensorGroup.push({
            $_name: newId,
          })
        }
        flag = true
      }
    } else {
      // 若没有sensorGroupId，则添加一个
      const maxSensorGroupName = getValidSensorGroupId(userSensorGroup, false)
      uv.Properties.Property.push({
        $_name: 'SensorGroup',
        $_value: `${maxSensorGroupName}`,
      })
      userSensorGroup.push({
        $_name: maxSensorGroupName,
      })
      flag = true
    }

    // 修复用户主车的动力学配置，用户的dynamicsid +100000
    const dynamicProperty = uv.Properties.Property.find(p => p.$_name === 'Dynamic')
    if (dynamicProperty && dynamicProperty.$_value !== '') {
      let id = +dynamicProperty.$_value
      const filePath = path.resolve(userDynamicsPath, `./dynamic_${id}.json`)
      if (await fse.pathExists(filePath)) {
        if (id < 100000 && presetProperty.$_value === 'false') {
          id = id + 100000
          dynamicProperty.$_value = `${id}`
          await fse.rename(filePath, path.resolve(userDynamicsPath, `./dynamic_${id}.json`))
          flag = true
        }
      } else {
        // 动力学文件不存在，应该报错
        console.error(`发现用户主车动力学配置但没有动力学配置文件：${id}`)
      }
    }
  }

  let maxSensorGroupName = getValidSensorGroupId(userSensorGroup, true)
  // 拷贝新加的主车
  if (!Array.isArray(sysVehicleCars)) {
    sysVehicleCars = [sysVehicleCars]
  }
  for (let i = 0; i < sysVehicleCars.length; i++) {
    const sc = sysVehicleCars[i]
    if (!userEgoCars.some(uc => uc.$_name === sc.$_name)) {
      flag = true
      const sensorGroupProperty = sc.Properties.Property.find(p => p.$_name === 'SensorGroup')
      // 如果用户没有这辆车，就加入进去。sensorGroup要修复一下
      if (sensorGroupProperty) {
        sensorGroupProperty.$_value = `${maxSensorGroupName}`
      } else {
        sc.Properties.Property.push({
          $_name: 'SensorGroup',
          $_value: `${maxSensorGroupName}`,
        })
      }
      userEgoCars.push(sc)
      // 记得同步创建sensorGroup
      userSensorGroup.push({
        $_name: `${maxSensorGroupName}`,
      })
      maxSensorGroupName++
      // 检查并拷贝动力学
      const dynamicProperty = sc.Properties.Property.find(p => p.$_name === 'Dynamic')
      if (dynamicProperty && dynamicProperty.$_value !== '') {
        const id = +dynamicProperty.$_value
        const filePath = path.resolve(sysDynamicsPath, `./dynamic_${id}.json`)
        if (await fse.pathExists(filePath)) {
          const newFilePath = path.resolve(userDynamicsPath, `./dynamic_${id}.json`)
          await fse.copyFile(filePath, newFilePath)
        } else {
          // 动力学文件不存在，应该报错
          console.error(`发现预设主车动力学配置但没有动力学配置文件：${id}`)
        }
      }
    }
  }

  // 相同逻辑，补充卡车
  for (let i = 0; i < sysCombinationCars.length; i++) {
    const sc = sysCombinationCars[i]
    if (!userCombinationCars.some(uc => uc.$_name === sc.$_name)) {
      flag = true
      const sensorGroupProperty = sc.Properties.Property.find(p => p.$_name === 'SensorGroup')
      // 如果用户没有这辆车，就加入进去。sensorGroup要修复一下
      if (sensorGroupProperty) {
        sensorGroupProperty.$_value = `${maxSensorGroupName}`
      } else {
        sc.Properties.Property.push({
          $_name: 'SensorGroup',
          $_value: `${maxSensorGroupName}`,
        })
      }
      userCombinationCars.push(sc)
      // 记得同步创建sensorGroup
      userSensorGroup.push({
        $_name: `${maxSensorGroupName}`,
      })
      maxSensorGroupName++
      // 检查并拷贝动力学
      const dynamicProperty = sc.Properties.Property.find(p => p.$_name === 'Dynamic')
      if (dynamicProperty && dynamicProperty.$_value !== '') {
        const id = +dynamicProperty.$_value
        const filePath = path.resolve(sysDynamicsPath, `./dynamic_${id}.json`)
        if (await fse.pathExists(filePath)) {
          const newFilePath = path.resolve(userDynamicsPath, `./dynamic_${id}.json`)
          await fse.copyFile(filePath, newFilePath)
        } else {
          // 动力学文件不存在，应该报错
          console.error(`发现预设主车动力学配置但没有动力学配置文件：${id}`)
        }
      }
    }
  }

  if (flag) {
    const userDefaultConfigNew = builder.build(userDefaultConfig)
    const userSensorConfigNew = builder.build(userSensorConfig)
    await fse.writeFile(userDefaultXoscPath, userDefaultConfigNew)
    await fse.writeFile(userSensorXoscPath, userSensorConfigNew)
  }

  // 复制所有主车模型文件
  const modelPaths = new Set()
  const totalCars = userEgoCars.concat(userCombinationCars)
  for (let i = 0; i < totalCars.length; i++) {
    const car = totalCars[i]
    let modelPath = car.Properties.Property.find(p => p.$_name === 'model3d')
    if (modelPath) {
      modelPath = modelPath.$_value
    } else {
      modelPath = car.$_model3d
    }
    if (modelPath) {
      const dir = modelPath.split('/')[0]
      if (dir) {
        modelPaths.add(dir)
      }
    }
  }
  for (const dir of modelPaths) {
    const sourceDir = path.resolve(AppPath, `./build/electron/assets/models/${dir}`)
    const targetDir = path.resolve(AppUserDataPath, `./data/scenario/models/${dir}`)
    if (!await fse.pathExists(targetDir) && await fse.pathExists(sourceDir)) {
      await fse.copy(sourceDir, targetDir)
    }
  }
}

// 版本升级只需要确保 log 目录存在
async function initLog () {
  try {
    const logPath = path.resolve(AppUserDataPath, './log')
    await fse.ensureDir(logPath)

    const initPromise = LogConfig.map((config) => {
      const { target, origin, isFile } = config
      if (isFile) {
        return Promise.resolve()
      } else {
        if (origin && fse.existsSync(origin)) {
          return fse.copy(origin, target)
        } else {
          return fse.ensureDir(target)
        }
      }
    })

    await Promise.all(initPromise)
  } catch (err) {
    console.log('init log dir failed: ', err)
  }
}

// 删除原有旧版本的目录结构
async function removeOrigin () {
  try {
    const userDataFiles = await fse.readdir(AppUserDataPath)
    const upgradeDirs = ['cache', 'Cache', 'sys', 'data', 'log', 'lockfile']
    const diffs = _.difference(userDataFiles, upgradeDirs)
    const removePromise = diffs.map(name =>
      fse.remove(path.resolve(AppUserDataPath, name)),
    )
    await Promise.all(removePromise)
  } catch (err) {
    console.log('remove origin dir failed: ', err)
  }
}
// 总体升级逻辑
async function upgradeDir () {
  await initCache()
  await initSystem()
  await initUserData()
  await updateUserData()
  await updateDriverCatalogs()
  await initLog()
  await removeOrigin()
}

module.exports = {
  upgradeDir,
  ensureCacheSync,
}
