import { ElMessage } from 'element-plus'
import './simInfo1Message.less'
import { createApp, h } from 'vue'

/**
 * ElMessage + h函数 创建并显示一个自定义样式的消息提示框
 * @param {object} options - 配置选项
 * @param {string} options.title - 消息标题
 * @param {string | Array} options.message - 消息内容，可以是字符串或字符串数组
 * @return {Promise<any>} 返回一个 Promise，在消息提示框消失时 resolve
 */
export default function ({ title, message }) {
  let vueInstance = createApp()

  const messageInstance = ElMessage({
    message: h(
      'div',
      { class: 'container' },
      [
        h('p', { class: 'title' }, [
          h('i', { class: 'el-icon-warning' }),
          h('span', { }, title),
        ]),
        Array.isArray(message) ?
          message.map(msg => h('p', { class: 'info' }, [msg])) :
          h('p', { class: 'info' }, message),
      ],
    ),
    duration: 30000,
    showClose: true,
    customClass: 'sim-info-1',
    center: true,
    iconClass: 'hidden-icon',
    offset: 40,
  })
  vueInstance = null
  return messageInstance
}
