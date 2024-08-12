import axios from './axios'

function parseJSON (str) {
  if (!str) return str
  try {
    return JSON.parse(str)
  } catch (err) {
    return null
  }
}

// 获取 pb 配置列表
export function getPbConfigList () {
  return axios.post('http://127.0.0.1:8302/api/config/get_pb_info', { field: '' })
    .then(({ code, msg }) => {
      if (code !== 0) return Promise.reject(new Error(msg))
      return parseJSON(msg)
    })
}

// 获取 pb 配置详情
export function getPbConfigDetail (field) {
  return axios.post('http://127.0.0.1:8302/api/config/get_pb_info', { field })
    .then(({ code, msg }) => {
      if (code !== 0) return Promise.reject(new Error(msg))
      return parseJSON(msg)
    })
}

// 删除 pb 配置
export function deletePbConfig (field) {
  return axios.post('http://127.0.0.1:8302/api/config/del_pb_info', { field })
    .then(({ code, msg }) => {
      if (code !== 0) return Promise.reject(new Error(msg))
      return parseJSON(msg)
    })
}

// 创建 pb 配置
export function createPbConfig (data) {
  return axios.post('http://127.0.0.1:8302/api/config/update_pb_info', { source_field: '', ...data })
    .then(({ code, msg }) => {
      if (code !== 0) return Promise.reject(new Error(msg))
      return parseJSON(msg)
    })
}

// 修改 pb 配置
export function updatePbConfig (oldField, data) {
  return axios.post('http://127.0.0.1:8302/api/config/update_pb_info', { source_field: oldField, ...data })
    .then(({ code, msg }) => {
      if (code !== 0) return Promise.reject(new Error(msg))
      return parseJSON(msg)
    })
}
