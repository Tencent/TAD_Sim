import { isBoolean } from 'lodash'

// 获取第一个字母大写的字符串
export function uppercaseFirstChar (str: string) {
  const firstChar = str.charAt(0).toUpperCase()
  return firstChar + str.substring(1)
}

// 获取文件路径中的文件目录
export function getDirname (path: string, sep: string) {
  const _path = path.split(sep)
  _path.pop()
  return _path.join(sep)
}

// 获取文件路径中的文件名称
export function getFilename (path: string, sep: string) {
  const _path = path.split(sep)
  return _path.pop()
}

// 判断地图名称是否太长
export function isNameTooLong (name: string) {
  return !/^[a-z0-9][\w+\-.()]{0,70}$/i.test(name)
}

// 判断地图名称是否符合规范
export function isNameInvalidated (name: string) {
  return !/^[a-z0-9][\w+\-.()]*$/i.test(name)
}

// 判断当前地图名称是否是 .xodr 后缀（忽略大小写）
export function isXodr (name: string) {
  return /\.xodr$/i.test(name)
}

// 判断用户上传的自定义模型的名称是否符合规范
// 支持中文、英文字母、数字
export function isCustomModelNameValidated (name: string) {
  return /^[a-z0-9\u4E00-\u9FA5]*$/i.test(name)
}

// 通过文件大小计算对应的占用空间
export function getFileSize (value: number) {
  if (!value) return '0KB'
  if (value < 1024) {
    return `${value}B`
  } else if (value < 1024 * 1024) {
    return `${(value / 1024).toFixed(1)}KB`
  } else if (value < 1024 * 1024 * 1024) {
    return `${(value / 1024 / 1024).toFixed(1)}MB`
  } else if (value < 1024 * 1024 * 1024 * 1024) {
    return `${(value / 1024 / 1024 / 1024).toFixed(1)}GB`
  } else {
    return `${(value / 1024 / 1024 / 1024 / 1024).toFixed(1)}TB`
  }
}

// 计算字符串长度
export function calcStrLength (str: string) {
  const chats = str.split('')
  let len = 0
  chats.forEach((chat) => {
    if (/[\u4E00-\u9FA5]/.test(chat)) {
      len += 2
    } else {
      len += 1
    }
  })
  return len
}

// 等待函数
export async function sleep (ms: number) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms)
  })
}

// 将三维矢量的数组进行去重
export function uniqVec3Array (points: Array<common.vec3>) {
  const _points: Array<common.vec3> = []
  const pointSet = new Set()
  points.forEach((p) => {
    const { x, y, z } = p
    const key = `${x}_${y}_${z}`
    // 添加不重复的三维矢量到数组副本种
    if (!pointSet.has(key)) {
      pointSet.add(key)
      _points.push(p)
    }
  })
  pointSet.clear()
  return _points
}

// 用于存储跟操作记录相关状态对应的新建初始值工厂函数
const _storeInitValueFactory = new Map()

export function clearStoreFactory () {
  _storeInitValueFactory.clear()
}

/**
 * 注册 store 创建初始值的工厂函数
 * @param name
 * @param fn
 */
export function registryStore (name: string, fn: Function) {
  if (_storeInitValueFactory.has(name)) return

  _storeInitValueFactory.set(name, fn)
}

/**
 * 获取 store 对应的初始值
 * @param name
 * @returns
 */
export function getInitValueByName (name: string) {
  if (!_storeInitValueFactory.has(name)) return {}

  const factoryFn = _storeInitValueFactory.get(name)
  return factoryFn()
}

// 当前加载的地图文件内容缓存
let _originMapCache: null | biz.ICommonMapElements = null

export function setOriginMapCache (content: null | biz.ICommonMapElements) {
  if (!content) {
    _originMapCache = content
  } else {
    // 提供默认值
    const { roads = [], lanelinks = [], objects = [] } = content
    _originMapCache = {
      roads,
      lanelinks,
      objects,
    }
  }
}

export function getOriginMapCache (): null | biz.ICommonMapElements {
  return _originMapCache
}

/**
 * 通过 id 从数组中寻找元素
 * @param elements
 * @param id
 * @returns
 */
export function findElementById (
  elements: Array<biz.ICommonElement>,
  id: string,
) {
  return elements.find(ele => ele.id === id)
}

// 三维场景更新属性的 loading 状态，避免频繁触发大量数据的计算逻辑
let _isLoading = false

/**
 * 获取 loading 状态
 * @returns
 */
export function getLoading () {
  return _isLoading
}

/**
 * 切换 loading 状态
 * @param enabled
 * @returns
 */
export function toggleLoading (enabled?: boolean) {
  if (isBoolean(enabled)) {
    if (_isLoading === enabled) return
    _isLoading = enabled
  } else {
    _isLoading = !_isLoading
  }
}

// 本地缓存加载的插件配置
const _pluginConfigs: Array<common.pluginConfig> = []
/**
 * 清空插件配置缓存
 */
export function clearPluginConfig () {
  _pluginConfigs.length = 0
}
/**
 * 缓存插件的配置
 * @param pluginName
 * @param value
 * @returns
 */
export function cachePluginConfig (config: common.pluginConfig) {
  for (const _config of _pluginConfigs) {
    // 如果待缓存的插件配置名称，跟已缓存的插件配置重名，则不保存
    if (_config.name === config.name) return
  }
  _pluginConfigs.push(config)
}

/**
 * 通过插件名称获取指定插件的配置
 * @param pluginName
 * @returns
 */
export function getPluginConfigByName (pluginName: biz.IPluginName) {
  if (!pluginName) return null
  for (const config of _pluginConfigs) {
    if (config.name === pluginName) return config
  }

  return null
}

/**
 * 获取所有插件的配置
 * @returns
 */
export function getAllPluginConfig () {
  return _pluginConfigs
}

// 每一次启动，请求一个新的 session
let sessionId: number = -1
export function getServiceSessionId () {
  return sessionId
}
export function saveServiceSessionId (id: number) {
  sessionId = id
}

// 支持放置拖拽元素的容器元素（拖拽图片，放置到三维场景对应的 div 中，实现添加效果）
export const dropElements: Set<HTMLElement> = new Set()

// 是否是 electron 启动运行的（tadsim 单机版会给 window 对象注入 electron 的属性）
export function ifElectron () {
  const { electron = {} } = window as any
  return Object.keys(electron).length > 0
}

// 生产环境 or 开发环境
export function ifDev () {
  return import.meta.env.DEV
}

// 是云端版
export function ifCloud () {
  // 读取环境变量中当前发版的项目配置
  const { VITE_APP_RUNTIME_MODE = '' } = import.meta.env
  const isCloud = VITE_APP_RUNTIME_MODE === 'cloud'
  const isElectron = ifElectron()
  // 云端版跟单机版应该是互斥的，即单机版有 electron 全局变量，云端版必定没有
  if (!isElectron && isCloud === false) return true
  return isCloud
}
