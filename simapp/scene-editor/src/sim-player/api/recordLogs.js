import axios from 'axios'

/**
 * 获取日志数据
 * @param info
 * @return {Promise<AxiosResponse<any>> | *}
 * @constructor
 */
export function LogInfo (info) {
  const data = `[FRONT]:${info}`
  return axios({
    method: 'post',
    url: 'http://localhost:9000/api/record/log/info',
    withCredentials: true,
    headers: {
      'Content-Type': 'text/plain',
    },
    data,
  })
}

/**
 * 获取错误日志数据
 * @param info
 * @return {Promise<AxiosResponse<any>> | *}
 * @constructor
 */
export function LogError (info) {
  const data = `[FRONT]:${info}`
  return axios({
    method: 'post',
    url: 'http://localhost:9000/api/record/log/info',
    withCredentials: true,
    headers: {
      'Content-Type': 'text/plain',
    },
    data,
  })
}
