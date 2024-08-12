import dict from '@/common/dict'
import mockElectron from '@/api/mock-electron'
import i18n from '@/locales'

// 从 window 对象中获取 electron 对象 ( 云端环境下没有 electron 对象, 用 mockElectron 对象来兼容 )
const { electron: { modules } = mockElectron } = window

/**
 * 封装请求方法，用于调用 Electron 关于 modules 相关 API。
 * @param {object} target - 目标对象，如 modules或 modules.scheme。
 * @param {string} method - 要调用的方法名。
 * @param {...any} args - 传递给方法的参数。
 * @returns {Promise} 返回一个 Promise对象，根据状态码解析为成功或失败。
 */
function request (target, method, ...args) {
  return new Promise((resolve, reject) => {
    target[method](...args, (status) => {
      const { retCode, data } = status
      if (retCode === 1) {
        resolve(data)
      } else {
        reject(new Error(i18n.t(dict.retCode[retCode]) || retCode))
      }
    })
  })
}

// modules 相关方法

// 获取模块列表
export function getModuleList () {
  return request(modules, 'findAll')
}

// 创建模块
export function createModule (payload) {
  return request(modules, 'create', payload)
}

// 更新模块
export function updateModule (payload) {
  return request(modules, 'update', payload)
}

// 删除模块
export function destroyModule (name) {
  return request(modules, 'destroy', name)
}

// schemes 相关方法

// 获取模块组列表
export function getModuleSetList () {
  return request(modules.scheme, 'findAll')
}

// 创建模块组
export function createModuleSet (payload) {
  return request(modules.scheme, 'create', payload)
}

// 更新模块组
export function updateModuleSet (payload) {
  return request(modules.scheme, 'update', payload)
}

// 删除模块组
export function destroyModuleSet (id) {
  return request(modules.scheme, 'destroy', id)
}

export function activeModuleSet (goups) {
  return request(modules.scheme, 'active', {
    active_multiego: goups,
  })
}

export function focusPlanner (group) {
  return request(modules.scheme, 'focus', group)
}

// 导入模块组
export function importModuleSet (filePath) {
  return request(modules.scheme, 'importScheme', filePath)
}

// 导出模块组
export function exportModuleSet (payload) {
  return request(modules.scheme, 'exportScheme', payload)
}
