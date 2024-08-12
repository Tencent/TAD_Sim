import { throttle } from 'lodash-es'
import eventBus from '@/event-bus'

// 日志消息
let logMessage

export function getLogMessage () {
  return logMessage
}

export function setLogMessage (data) {
  logMessage = data
}

export function emitLogMessage () {
  eventBus.$emit('log-message', logMessage)
}

export function resetLogMessage () {
  logMessage = undefined
  emitLogMessage()
}

/**
 * 节流发送日志消息
 * @type {DebouncedFunc<(function(): void)|*>}
 */
export const throttleLogMessage = throttle(() => {
  emitLogMessage()
}, 1000, { trailing: true })
