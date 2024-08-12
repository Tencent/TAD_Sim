import { size, throttle } from 'lodash-es'
import eventBus from '@/event-bus'

// 最大kpi列表长度
const SIZE = 120
// 存储kpi列表
let gradingKPIList = []

export function getGradingKPIList () {
  return gradingKPIList
}

/**
 * 压入一帧kpi数据
 */
export function pushGradingKPI (data) {
  const gradingKPI = {
    timeStamp: data.timeStamp?.timeStamp,
    speed: data.speed?.speed,
    acceleration: data.acceleration?.acceleration,
    collision: size(data.collision),
  }

  gradingKPIList.push(gradingKPI)

  if (gradingKPIList.length > SIZE) {
    gradingKPIList.shift()
  }
}

export function emitGradingKPIList () {
  eventBus.$emit('grading-kpi', gradingKPIList)
}

/**
 * 重置kpi列表
 */
export function resetGradingKPIList () {
  gradingKPIList = []
  emitGradingKPIList()
}

/**
 * 节流kpi列表
 * 场景播放过程中，每100ms会触发一次节流，将kpi列表发送给前端
 * 场景播放完成后，会触发一次节流，将kpi列表发送给前端
 *
 * @type {DebouncedFunc<(function(*): void)|*>}
 */
export const throttleGradingKPI = throttle((playingStatus) => {
  if (playingStatus !== 'uninitialized') { // 由于节流的trailing，场景播放完成后还会push一帧，我们不需要这一帧，这里屏蔽一下。
    emitGradingKPIList()
  }
}, 100, { trailing: true })
