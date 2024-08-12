import ajax from '@/api/axios'

/**
 * 启动语义分析执行
 * @param {object} payload - 启动语义分析所需的参数
 * @returns {Promise}
 */
export const startexe = function (payload) {
  return ajax.post('http://127.0.0.1:9000/api/semantic/startexe', payload, {
    timeout: 7200000
  })
}

/**
 * 生成语义分析任务
 * @returns {Promise}
 */
export const generateSemantic = function () {
  return ajax.get('http://127.0.0.1:9000/api/semantic/generate')
}

/**
 * 获取语义分析状态
 * @param {object} payload - 查询语义分析状态所需的参数
 * @returns {Promise} 返回一个包含语义分析状态的 Promise 对象
 */
export const semanticStatu = function (payload) {
  return ajax.post('http://127.0.0.1:9000/api/semantic/statu', payload)
}

/**
 * 查询语义分析进度信息
 * @returns {Promise} 返回一个包含语义分析进度信息的 Promise 对象
 */
export const queryProgressInfo = function () {
  return ajax.get('http://127.0.0.1:9000/api/semantic/getgenprogressval')
}

/**
 * 停止语义分析进度
 * @returns {Promise}
 */
export const stopgenprogressval = function () {
  return ajax.get('http://127.0.0.1:9000/api/semantic/stopgenprogressval', { timeout: 0 })
}
