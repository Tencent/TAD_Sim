import { h } from 'vue'
import { ElProgress } from 'element-plus'
import './progress-message.less'

// 进度条组件的默认配置
const progressProps = {
  percentage: 0,
  type: 'circle',
  strokeWidth: 2,
  width: 18,
  showText: false,
  color: '#00FAFF',
}

/**
 * 创建一个带有进度条的消息组件
 * @param {object} options - 配置选项
 * @returns {object} messageInstance - 返回一个包含更新进度方法的消息组件实例
 */
function progressMessage (options) {
  const progress = h('div', {
    attrs: {
      class: 'progress-message',
    },
  }, [
    h(ElProgress, {
      props: progressProps,
    }),
    h('span', {
      attrs: {
        class: 'progress-message-text',
      },
    }, options.message),
  ])

  // 创建消息组件实例
  const messageInstance = this.$message.info({
    duration: 0,
    showClose: true,
    iconClass: '',
    ...options,
    message: progress,
  })

  // 更新进度条方法
  messageInstance.updateProgress = (percentage) => {
    messageInstance.$slots.default = [
      h('div', {
        attrs: {
          class: 'progress-message',
        },
      }, [
        h(ElProgress, {
          props: {
            ...progressProps,
            percentage,
          },
        }),
        h('span', {
          attrs: {
            class: 'progress-message-text',
          },
        }, options.message),
      ]),
    ]
    messageInstance.$forceUpdate()
  }

  return messageInstance
}

export default {
  install (app) {
    /**
     * 将 progressMessage 方法添加到 Vue3 实例的全局属性中
     * @param {object} app - Vue 应用实例
     */
    app.config.globalProperties.$progressMessage = progressMessage
  },
}
