import ajax from '@/api/axios'

/**
 * 获取场景列表
 * @returns {Promise} 返回一个包含场景列表的 Promise 对象
 */
export const getScenarioList = function () {
  return ajax.post('http://127.0.0.1:9000/api/scenarios/v2', {
    offset: 0,
    limit: 99999,
  })
}

/**
 * 获取指定 ID 的场景信息
 * @param {number} id - 场景的 ID
 * @returns {Promise} 返回一个包含指定 ID 场景信息的 Promise 对象
 */
export const getScenario = function (id) {
  return ajax.post(`http://127.0.0.1:9000/openscene/v2`, { id })
}

/**
 * 删除场景
 * @param {number[]} ids - 要删除的场景 ID 列表
 * @returns {Promise} 返回一个表示删除操作结果的 Promise 对象
 */
export const delScenarios = function (ids) {
  const list = ids.map((id) => {
    return {
      id,
    }
  })
  return ajax.post('http://127.0.0.1:9000/api/scenarios/delete', list)
}

/**
 * 复制场景
 * @param {Array} arr - 要复制的场景信息列表
 * @returns {Promise} 返回一个表示复制操作结果的 Promise 对象
 */
export const copyScenario = function (arr) { // todo
  return ajax.post('http://127.0.0.1:9000/api/scenarios/copy', arr)
}

/**
 * 导入场景
 * @param {object} payload - 导入的场景
 * @returns {Promise}
 */
export const importScenarios = function (payload) {
  payload.sources = payload.sources.map((name) => {
    return {
      name,
    }
  })
  return ajax.post('http://127.0.0.1:9000/api/scenarios/upload', payload)
}

/**
 * 导入子场景
 * @param {object} payload - 导入的子场景
 * @returns {Promise}
 */
export const importScenariosSub = function (payload) {
  payload.sources = payload.sources.map(({ name, replaceFlag }) => {
    return {
      name,
      replaceFlag,
    }
  })
  return ajax.post('http://127.0.0.1:9000/api/scenarios/upload', payload, { timeout: 0 })
}

/**
 * 导出场景
 * @param {object} payload - 导出场景
 * @returns {Promise}
 */
export const exportScenarios = function (payload) { // todo
  payload.sources = payload.sources.map((name) => {
    return {
      name,
    }
  })
  return ajax.post('http://127.0.0.1:9000/api/scenarios/download', payload, { timeout: 0 })
}

/**
 * 获取场景生成信息
 * @param {object} payload - 查询场景生成信息所需的参数
 * @returns {Promise} 返回一个包含场景生成信息的 Promise 对象
 */
export const getScenarioGenerationInfo = function (payload) {
  return ajax.post('http://127.0.0.1:9000/paramscenecount', payload)
}

/**
 * 重命名场景
 * @param {object} payload - 包含要重命名的场景 ID 和新名称的数据
 * @returns {Promise}
 */
export const renameScenario = function (payload) {
  return ajax.post('http://127.0.0.1:9000/api/scenarios/rename', payload)
}

/**
 * 查询进度信息
 * @param {object} payload - 查询进度信息所需的参数
 * @returns {Promise} 返回一个包含进度信息的 Promise 对象
 */
export const queryProgressInfo = function (payload) {
  return ajax.post('http://127.0.0.1:9000/api/scenarios/getprogressval', payload)
}

/**
 * 停止进度
 * @param {object} payload - 停止进度所需的参数
 * @returns {Promise}
 */
export const stopProgress = function (payload) {
  return ajax.post('http://127.0.0.1:9000/api/scenarios/stopprogress', payload)
}

/**
 * 查询场景生成信息
 * @param {object} payload - 查询场景生成信息所需的参数
 * @returns {Promise} 返回一个包含场景生成信息的 Promise 对象
 */
export const queryScenarioGenerationInfo = function (payload) {
  return ajax.post('http://127.0.0.1:9000/api/scenarios/getgenprogressval', payload)
}

/**
 * 停止场景生成
 * @param {object} payload - 停止场景生成所需的参数
 * @returns {Promise}
 */
export const stopScenarioGeneration = function (payload) {
  return ajax.post('http://127.0.0.1:9000/api/scenarios/stopgenprogress', payload)
}
