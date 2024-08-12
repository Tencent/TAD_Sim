/**
 * 数据服务汇总文件
 */
import axios from './axios'
import * as map from './map'
import * as modules from './modules'
import * as playlist from './playlist'
import * as kpi from './kpi'
import * as tapd from './tapd'
import * as planners from './planners'
import * as pbConfig from './pb-config'

/**
 * 获取所有传感器数据
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function fetchSensors () {
  return axios.get('http://127.0.0.1:9000/sensors/loadpreset').then((data) => {
    let { content } = data
    if (content) {
      content = JSON.parse(content)
    }
    return content
  })
}

/**
 * 保存传感器数据
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function saveSensors (payload) {
  return axios.post('http://127.0.0.1:9000/sensors/savepreset', {
    content: JSON.stringify(payload),
  }).then((data) => {
    let { content } = data
    if (content) {
      content = JSON.parse(content)
    }
    return content
  })
}

/*
* 获取全局传感器数据
 */
export function fetchGlobalSensors () {
  return axios.get('http://127.0.0.1:9000/sensors/loadglobal')
}

/**
 * 保存全局传感器数据
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function saveGlobalSensors (payload) {
  return axios.post('http://127.0.0.1:9000/sensors/saveglobal', payload).then((data) => {
    let { content } = data
    if (content) {
      content = JSON.parse(content)
    }
    return content
  })
}

/**
 * 获取全局环境数据
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function fetchGlobalEnvironment () {
  return axios.get('http://127.0.0.1:9000/sensors/loadglobalenvironment')
}

/**
 * 保存全局环境数据
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function saveGlobalEnvironment (payload) {
  return axios.post('http://127.0.0.1:9000/sensors/saveglobalenvironment', payload).then((data) => {
    let { content } = data
    if (content) {
      content = JSON.parse(content)
    }
    return content
  })
}

export default {
  fetchSensors,
  saveSensors,
  ...map,
  ...modules,
  ...playlist,
  ...kpi,
  ...tapd,
  ...planners,
  ...pbConfig,
}
