import dict from '@/common/dict'
import i18n from '@/locales'

// 从 window 对象中获取 electron 对象
const { electron } = window

/**
 * 封装请求方法，用于调用 Electron 关于 playlist 相关 API。
 * @param {string} method - 要调用的方法名。
 * @param {...any} args - 传递给方法的参数。
 * @returns {Promise} 返回一个 Promise对象，根据状态码解析为成功或失败。
 */
function request (method, ...args) {
  return new Promise((resolve, reject) => {
    electron.player.playlist[method](...args, (status) => {
      const { retCode, data } = status
      if (retCode === 1) {
        resolve(data)
      } else {
        reject(new Error(i18n.t(dict.retCode[retCode]) || retCode))
      }
    })
  })
}

/**
 * 获取播放列表中所有的场景。
 * @returns {Promise<any>}
 */
export function getPlaylist () {
  return request('findOne')
}

/**
 * 更新某个播放场景
 * @param {number[]} ids - 场景 ID
 * @returns {Promise<void>}
 */
export function updatePlaylist (ids) {
  return request('update', ids)
}

/**
 * 更新某个播放场景
 * @param {number[]} ids - 场景 ID
 * @returns {Promise<void>}
 */
export function removeScenariosFromPlaylist (ids) {
  return request('removeScenarios', ids)
}
