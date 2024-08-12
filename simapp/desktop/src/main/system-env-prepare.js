const { promisify } = require('util')
const { spawn } = require('child_process')
const path = require('path')
const regedit = require('regedit')
const { dialog } = require('electron')
const { invoke } = require('lodash')
const { warning } = require('../renderer/editor')

const list = promisify(regedit.list)

// const VC2010Key = 'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\10.0\\VC\\VCRedist\\x64'
const VC2022Key = 'HKLM\\SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\14.0\\VC\\Runtimes\\x64'

/**
 * 测试环境是否安装了指定版本的 VC++ Redistributable
 * @param key
 * @return {Promise<*|string>}
 */
async function testEnv (key) {
  try {
    const result = await list(key)
    const {
      [key]: {
        values: {
          Version,
        },
      },
    } = result
    return Version.value
  } catch (e) {
    return ''
  }
}

/**
 * 检测 Windows 环境是否安装了指定版本的 VC++ Redistributable
 */
async function testAndInstallWin32 () {
  const warningConfig = await warning.loadConfig()

  // // 检测 VC 2010 x64 的安装
  // const vc2010Version = await testEnv(VC2010Key)
  // if (!vc2010Version.startsWith('v10.')) {
  //   try {
  //     // 尝试安装 vc 2010
  //     await new Promise((resolve, reject) => {
  //       try {
  //         const installation = spawn('.\\vcredist_x64_2010.exe', ['Start', '/passive', '/promptrestart'], {
  //           cwd: path.join(__dirname, '../../service/deps/win32/vc++'),
  //         })

  //         installation.on('close', (code) => {
  //           if (code === 0) {
  //             resolve()
  //           } else {
  //             reject(new Error(`vc2010 安装失败, code: ${code}`))
  //           }
  //         })
  //       } catch (e) {
  //         reject(new Error(`vc2010 安装失败: ${e.massage}`))
  //       }
  //     })
  //   } catch (e) {
  //     if (warningConfig.vc2010) {
  //       // 提示用户缺失依赖
  //       const { response, checkboxChecked } = await dialog.showMessageBox({
  //         type: 'warning',
  //         title: '缺失依赖程序',
  //         message: 'TAD Sim 需要依赖 Microsoft Visual C++ 2010 x64 Redistributable 才能正常运行。',
  //         checkboxLabel: '不再提示',
  //         buttons: [
  //           '仍然运行',
  //           '退出',
  //         ],
  //         cancelId: 0,
  //         defaultId: 1,
  //         noLink: true,
  //       })
  //       if (checkboxChecked) {
  //         await warning.saveConfig({ vc2010: false })
  //       }
  //       if (response !== 0) {
  //         return Promise.reject(new Error('用户退出应用'))
  //       }
  //     }
  //   }
  // }

  // 检测 VC 2022 x64 的安装
  const vc2022Version = await testEnv(VC2022Key)
  if (!vc2022Version.startsWith('v14.')) {
    try {
      await new Promise((resolve, reject) => {
        try {
          const installation = spawn('.\\vcredist_x64_2022.exe', ['/install', '/passive'], {
            cwd: path.join(__dirname, '../../service/deps/win32/vc++'),
          })

          installation.on('close', (code) => {
            if (code === 0) {
              resolve()
            } else {
              reject(new Error(`vc2022 安装失败, code: ${code}`))
            }
          })
        } catch (e) {
          reject(new Error(`vc2022 安装失败: ${e.massage}`))
        }
      })
    } catch (e) {
      if (warningConfig.vc2022) {
        const { response, checkboxChecked } = await dialog.showMessageBox({
          type: 'warning',
          title: '缺失依赖程序',
          message: 'TAD Sim 需要依赖 Microsoft Visual C++ 2022 x64 Redistributable 才能正常运行。',
          checkboxLabel: '不再提示',
          buttons: [
            '仍然运行',
            '退出',
          ],
          cancelId: 0,
          defaultId: 1,
          noLink: true,
        })
        if (checkboxChecked) {
          await warning.saveConfig({ vc2022: false })
        }
        if (response !== 0) {
          return Promise.reject(new Error('用户退出应用'))
        }
      }
    }
  }
}

/**
 * 查找显示设备
 * @param devicesInfo
 * @return {*[]}
 */
function findDisplayDevices (devicesInfo) {
  const displayDevices = []
  findDisplay(devicesInfo.children || [])

  function findDisplay (deviceArr) {
    deviceArr.forEach((device) => {
      if (device.id === 'display') {
        displayDevices.push(device)
      }
      if (device.children && device.children.length) {
        findDisplay(device.children)
      }
    })
  }

  return displayDevices
}

/**
 * 检测 Linux 环境是否安装了 NVIDIA 显卡驱动
 * @return {Promise<never>}
 */
async function testAndInstallLinux () {
  // 获取 Linux 所有设备信息
  const displayDevices = await new Promise((resolve, reject) => {
    const lshw = spawn('lshw', ['-json'])
    let output = ''
    lshw.stdout.on('data', (data) => {
      output += data.toString()
    })
    lshw.on('close', () => {
      try {
        output = JSON.parse(output)
      } catch (error) {
        output = {}
      }
      const displayDevices = findDisplayDevices(output)
      resolve(displayDevices)
    })
  })

  // 查询是否有非官方驱动的 NVIDIA 显卡
  const noOfficialNvidiaDriver = testNvidiaDriver(displayDevices)
  if (noOfficialNvidiaDriver && (await warning.loadConfig()).nvidiaDriver) {
    const { response, checkboxChecked } = await dialog.showMessageBox({
      type: 'warning',
      title: '非官方显卡驱动',
      message: '您的 NVIDIA 显卡安装的非官方驱动，可能会使 TAD Sim 运行缓慢，或者 Display 模块无法运行。',
      checkboxLabel: '不再提示',
      buttons: [
        '仍然运行',
        '退出',
      ],
      cancelId: 0,
      defaultId: 1,
      noLink: true,
    })
    if (checkboxChecked) {
      await warning.saveConfig({ nvidiaDriver: false })
    }
    if (response !== 0) {
      return Promise.reject(new Error('用户退出应用'))
    }
  }
}

/**
 * 检测 NVIDIA 显卡没有装官方驱动
 * @param {Array} displayDevices
 * @return {boolean}
 */
function testNvidiaDriver (displayDevices) {
  const noOfficialDriver = displayDevices.find((d) => {
    return (
      d.id === 'display' &&
      invoke(d, 'vendor.toLowerCase', '') === 'nvidia corporation' &&
      invoke(d, 'configuration.driver.toLowerCase', '') !== 'nvidia'
    )
  })
  return !!noOfficialDriver
}

function testAndInstall () {
  if (process.platform === 'win32') {
    return testAndInstallWin32()
  } else if (process.platform === 'linux') {
    return testAndInstallLinux()
  } else {
    return Promise.resolve()
  }
}

module.exports = {
  testAndInstall,
}
