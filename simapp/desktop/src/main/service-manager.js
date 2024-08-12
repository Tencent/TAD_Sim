const path = require('path')
const { spawn } = require('child_process')
const fse = require('fs-extra')
const { dialog, app } = require('electron')
const {
  AppPath,
  AppUserDataPath,
  ProjectPath,
  UserDataPath,
} = require('../common/constants')

const { platform } = process
const processList = []

/**
 * 调起子进程
 * @param command
 * @param args
 * @param options
 */
function spawnService (command, args, options) {
  const proc = spawn(command, args, options)

  proc.on('exit', async (code, signal) => {
    if (code) {
      console.error(
        `child process ${command} exited with code ${code} and signal ${signal}`,
      )
      processList.splice(processList.indexOf(proc), 1)
    }
    if (code === 100) {
      // 之前的进程没有退出, 2秒之后递归重试
      setTimeout(() => {
        spawnService(command, args, options)
      }, 2000)
    }
    if (code === 101) {
      await dialog.showMessageBox({
        type: 'error',
        title: '授权过期',
        message: '您的软件授权已过期。',
        buttons: [
          '退出',
        ],
        defaultId: 0,
        noLink: true,
      })
      app.quit()
    }
  })

  proc.on('error', (err) => {
    console.error(err)
  })

  processList.push(proc)
}

/**
 * 调起sim service进程
 */
function startSimService () {
  const env = {
    ...process.env,
  }
  let command = ''

  if (platform === 'win32') {
    command = path.join(ProjectPath.service, 'txsim-local-service.exe')
  } else {
    command = path.join(ProjectPath.service, 'txsim-local-service')
    env.LD_LIBRARY_PATH = ProjectPath.serviceDeps
  }

  const args = [
    `--root=${AppUserDataPath}`,
    `--logdir=${UserDataPath.debugLog}`,
  ]
  const options = {
    env,
  }

  spawnService(command, args, options)
}

/**
 * 调起web后台
 */
function startScenarioService () {
  // 确保服务端写入日志的文件存在
  !fse.existsSync(UserDataPath.debugLogScenarioService) && fse.ensureFileSync(UserDataPath.debugLogScenarioService)
  const fd = fse.openSync(UserDataPath.debugLogScenarioService, 'w')
  const env = {
    ...process.env,
  }
  let command = ''

  if (platform === 'win32') {
    command = path.join(ProjectPath.service, 'txSimService/txSimService.exe')
  } else {
    command = path.join(ProjectPath.service, 'txSimService')
    env.LD_LIBRARY_PATH = ProjectPath.serviceDeps
  }

  const args = [
    `--service_dir=${ProjectPath.serviceScenario}`,
    `--scenario_dir=${UserDataPath.scenario}`,
    `--app_dir=${AppPath}`,
  ]
  const options = {
    env,
    stdio: [0, fd, fd],
  }

  spawnService(command, args, options)
}

async function startService () {
  startSimService()
  startScenarioService()
}

/**
 * 退出所有子进程
 */
function stopService () {
  processList.forEach((proc) => {
    proc.kill()
  })
}

module.exports = {
  startService,
  stopService,
}
