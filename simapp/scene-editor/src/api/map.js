import ajax from './axios'

/**
 * 获取地图列表
 * @return {*}
 */
export function getMapList () {
  return ajax('http://127.0.0.1:9000/hadmaplist')
}

/**
 * 删除地图
 * @param names
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export const delMaps = function (names) {
  const payload = names.map((name) => {
    return {
      name,
    }
  })
  return ajax.post('http://127.0.0.1:9000/hadmaps/delete', payload)
}

/**
 * 导入地图
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export const importMaps = function (payload) {
  payload.sources = payload.sources.map((name) => {
    return {
      name,
    }
  })
  return ajax.post('http://127.0.0.1:9000/hadmaps/upload', payload)
}

/**
 * 导入地图
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export const importMapsSub = function (payload) {
  payload.sources = payload.sources.map(({ name, replaceFlag }) => {
    return {
      name,
      replaceFlag,
    }
  })
  return ajax.post('http://127.0.0.1:9000/hadmaps/upload', payload, { timeout: 0 })
}

/**
 * 导出地图
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export const exportMaps = function (payload) {
  payload.sources = payload.sources.map((name) => {
    return {
      name,
    }
  })
  return ajax.post('http://127.0.0.1:9000/hadmaps/download', payload, { timeout: 0 })
}

/**
 * 查询导入地图进度
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export const queryProgressInfoMap = function (payload) {
  return ajax.post('http://127.0.0.1:9000/hadmaps/getprogressval', payload)
}

/**
 * 停止导入地图进度
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export const stopProgressMap = function (payload) {
  return ajax.post('http://127.0.0.1:9000/hadmaps/stopprogress', payload)
}
