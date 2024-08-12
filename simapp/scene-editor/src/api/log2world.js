import ajax from './axios'

/**
 * 保存log2world配置
 * @param params
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export async function setLog2WorldConfig (params) {
  const url = 'http://127.0.0.1:8302/api/config/l2w_params'
  return ajax.post(url, params)
}

/**
 * 获取log2world配置
 * @param scenePath
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export async function getLog2WorldConfig ({ scenePath }) {
  const url = `http://127.0.0.1:8302/api/config/l2w_params?scene_path=${scenePath}`
  const path = encodeURI(url)
  return ajax.get(path)
}
