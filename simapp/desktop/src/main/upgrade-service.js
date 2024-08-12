const path = require('path')
const { spawn } = require('child_process')
const fse = require('fs-extra')
const { compare } = require('compare-versions')
const {
  AppPath,
  AppUserDataPath,
  UserDataPath,
  ProjectPath,
} = require('../common/constants')

const { platform } = process
let _isNeedToUpgrade = false

/**
 * 比较版本号
 * @param version
 * @param condition
 * @return {boolean}
 */
function compareInstallVersion (version, condition) {
  const originPackagePath = path.resolve(AppUserDataPath, './package.json')
  const newPackagePath = path.resolve(AppUserDataPath, './sys/package.json')
  const installPackagePath = fse.existsSync(originPackagePath) ?
    originPackagePath :
    newPackagePath

  if (!fse.existsSync(installPackagePath)) return false

  let installedContent = fse.readJsonSync(installPackagePath)
  try {
    // todo: 兼容错误package.json格式代码，后续版本删掉
    installedContent = JSON.parse(installedContent)
  } catch (e) {
    console.log('package.json format checked.')
    return false
  }
  const { version: installVersion } = installedContent
  if (compare(installVersion, version, condition)) return true

  return false
}

// 暂时先用一个方法来获取该 module 私有变量
// 后续最好把配置相关的内容，单独整理出一个 config 文件统一保存
function isNeedToUpgrade () {
  return _isNeedToUpgrade
}

function checkVersionSync () {
  try {
    const { name, version } = fse.readJsonSync(
      path.resolve(AppPath, './package.json'),
    )
    const originPackagePath = path.resolve(AppUserDataPath, './package.json')
    const newPackagePath = path.resolve(AppUserDataPath, './sys/package.json')
    const installPackagePath = fse.existsSync(originPackagePath) ?
      originPackagePath :
      newPackagePath

    // 默认语言为中文
    let installLanguage = 'zh-CN'

    if (fse.existsSync(installPackagePath)) {
      const installPackageContent = fse.readJsonSync(installPackagePath)
      const { version: installVersion, language = 'zh-CN' } =
        installPackageContent
      // 如果已经安装的版本中保存了语言的配置，则同步到新的配置文件中
      installLanguage = language

      if (!installVersion || compare(version, installVersion, '>')) {
        // 有了新版本
        _isNeedToUpgrade = true
      }
    } else {
      // 如果用户目录下的 package 文件不存在，则视为需要更新
      _isNeedToUpgrade = true
    }

    if (_isNeedToUpgrade) {
      // 每次版本更新，都需要覆盖用户目录的 package
      const packagePath = path.resolve(AppUserDataPath, './sys/package.json')
      !fse.existsSync(packagePath) && fse.ensureFileSync(packagePath)
      const config = {
        name,
        version,
        language: installLanguage,
      }
      fse.writeJsonSync(packagePath, config)
    }
  } catch (err) {
    console.log('compare package version failed: ', err)
  }
}

async function runServiceUpgrade () {
  const command =
    platform === 'win32' ?
      path.join(ProjectPath.serviceUpgradeTools, 'run.bat') :
      path.join(ProjectPath.serviceUpgradeTools, 'run.sh')
  const fd = await fse.open(UserDataPath.upgradeLog, 'w')
  const options = {
    stdio: [0, fd, fd],
  }

  return new Promise((resolve, reject) => {
    let proc = null
    if (platform === 'win32') {
      // 基于服务端启动 bat 升级脚本，对命令行做了适配
      const _cmd = `""${command}" "${AppPath}" "${AppUserDataPath}""`
      options.windowsVerbatimArguments = true
      proc = spawn('cmd.exe', ['/c', _cmd], options)
    } else {
      proc = spawn(command, [AppPath, AppUserDataPath], options)
    }
    proc.on('exit', (code) => {
      // if (code === 0) {
      // TODO 暂时先用升级脚本退出后，也能打开界面的逻辑。后续需要优化，如果服务端升级不成功，前端提示然后退出应用
      resolve()
      // }
    })
    proc.on('error', (err) => {
      console.error(err)
      reject(err)
    })
  })
}

async function upgradeService () {
  try {
    await runServiceUpgrade()
  } catch (err) {
    console.log('run upgrade service failed: ', err)
  }
}

module.exports = {
  checkVersionSync,
  upgradeService,
  isNeedToUpgrade,
  compareInstallVersion,
}
