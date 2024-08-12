import { ElMessage } from 'element-plus'

// 全局错误提示
export function errorHandler (error) {
  console.error(error)
  return ElMessage.error({
    message: error.message || error,
  })
}
// 全局错误提示，强提示，需手动关闭
export function showErrorMessage (message) {
  console.error(message)
  return ElMessage.error({
    message,
    duration: 0,
    offset: 32,
    showClose: true,
  })
}
