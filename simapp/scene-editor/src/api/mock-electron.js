/**
 * window.electron 对象在云端环境中不存在,
 * 因此模拟 Electron api，
 * 用于兼容某些文件在云端环境中调用 window.electron 对象报错的情况
 * 应尽可能的完善该 mock 对象 或者 避免使用
 */
export default {
  modules: {
    create () {
      return {}
    },
    update () {
      return {
        code: 0,
      }
    },
    destroy () {
      return {}
    },
    findOne () {
      return Promise.resolve({})
    },
    findAll () {
      return Promise.resolve([])
    },
  },
  player: {
    setup () {
    },
    reset () {
    },
    play () {
    },
    pause () {
    },
    step () {
    },
    stop () {
    },
  },
  checkPathExist () {
    return true
  },
  webConfig: {
    sensor: {},
  },
}
