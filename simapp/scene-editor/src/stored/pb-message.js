import { throttle } from 'lodash-es'
import eventBus from '@/event-bus'

const SIZE = 120 // 定义消息列表的最大长度
let pbMessageList = [] // 存储消息列表

/**
 * 获取消息列表
 * @returns {Array} - 返回消息列表
 */
export function getPbMessageList () {
  return pbMessageList
}

/**
 * 向消息列表中推送一条消息
 * @param {object} data - 要推送的消息数据
 */
export function pushPbMessage (data) {
  pbMessageList.push(data)

  // 如果消息列表长度超过最大长度，移除最早的一条消息
  if (pbMessageList.length > SIZE) {
    pbMessageList.shift()
  }
}

/**
 * 触发消息列表的事件
 */
export function emitPbMessage () {
  eventBus.$emit('pb-message', pbMessageList)
}

/**
 * 重置消息列表
 */
export function resetPbMessage () {
  pbMessageList = []
  emitPbMessage()
}

/**
 * 节流触发消息列表的事件
 * @param {string} playingStatus - 场景播放状态
 */
export const throttlePbMessage = throttle((playingStatus) => {
  if (playingStatus !== 'uninitialized') { // 由于节流的trailing，场景播放完成后还会push一帧，我们不需要这一帧，这里屏蔽一下。
    emitPbMessage()
  }
}, 100, { trailing: true })
