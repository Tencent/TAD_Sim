import { isEmpty, merge, throttle } from 'lodash-es'
import eventBus from '@/event-bus'

/**
 * 主车仪表盘，用于显示主车状态，性能原因，避免使用vuex
 * @type {{}}
 */
let carDashboard = {}
export function getCarDashboard () {
  return carDashboard
}

export function setCarDashboard (data) {
  if (carDashboard) {
    carDashboard = merge(carDashboard, data)
  } else {
    carDashboard = data
  }
}

export function resetCarDashboard () {
  carDashboard = {}
  eventBus.$emit('car-dashboard', { cmd: 'reset' })
}

export const throttleCarDashboard = throttle(() => {
  if (!isEmpty(carDashboard)) {
    eventBus.$emit('car-dashboard', carDashboard)
  }
}, 100, { trailing: true })
