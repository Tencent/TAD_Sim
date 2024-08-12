import { throttle } from 'lodash-es'
import eventBus from '@/event-bus'

// 定义一个全局变量 protoMessage 用于存储协议消息数据
let protoMessage

/**
 * 获取协议消息数据
 * @returns {object | undefined} - 返回协议消息数据
 */
export function getProtoMessage () {
  return protoMessage
}

/**
 * 设置协议消息数据
 * @param {object} data - 要设置的协议消息数据
 */
export function setProtoMessage (data) {
  protoMessage = data
}

/**
 * 触发协议消息事件
 */
export function emitProtoMessage () {
  eventBus.$emit('proto-message', protoMessage)
}

/**
 * 重置协议消息数据并触发事件
 */
export function resetProtoMessage () {
  protoMessage = undefined
  emitProtoMessage()
}

/**
 * 节流触发协议消息事件，1000毫秒内最多触发一次
 */
export const throttleProtoMessage = throttle(() => {
  emitProtoMessage()
}, 1000, { trailing: true })
