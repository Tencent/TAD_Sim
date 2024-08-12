import ajax from '@/api/axios'

const baseUrl = 'http://127.0.0.1:9000/api/scenarioset'

/**
 * 获取场景集列表
 * @returns {Promise} 返回一个包含场景集列表的 Promise 对象
 */
export const getScenarioSetList = function () {
  return ajax.post(baseUrl, {
    offset: 0,
    limit: 99999,
  })
}

/**
 * 创建场景集
 * @param {object} payload - 场景集的数据
 * @returns {Promise}
 */
export const createScenarioSet = function (payload) {
  return ajax.post(`${baseUrl}/add`, payload)
}

/**
 * 更新场景集
 * @param {object} payload - 场景集的数据，包括要更新的字段和值
 * @returns {Promise}
 */
export const updateScenarioSet = function (payload) {
  return ajax.post(`${baseUrl}/update`, payload)
}

/**
 * 删除场景集
 * @param {number} id - 要删除的场景集的 ID
 * @returns {Promise}
 */
export const destroyScenarioSet = function (id) {
  return ajax.post(`${baseUrl}/delete`, {
    id,
  })
}
