import axios from 'axios'

const ajax = axios.create()

// 默认超时时间
ajax.defaults.timeout = 30 * 1000
// 默认请求头格式
ajax.defaults.headers['Content-Type'] = 'application/json; charset=UTF-8'

// 默认返回处理钩子
ajax.interceptors.response.use((response) => {
  return response.data
}, (error) => {
  if (error.response) {
    const { message, statusText } = error.response
    if (!message) {
      error.response.message = statusText
    }
  }
  return Promise.reject(error)
})

export default ajax
