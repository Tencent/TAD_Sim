import { cloneDeep, merge } from 'lodash-es'
import { createDefaultEnv } from './factory'

export function updateState (state, payload) {
  Object.assign(state, payload)
}

/**
 *
 * @param state
 * @param {'scenarioEvnGroup'|'globalEnvGroup'} type
 * @param {number} index
 * @param {object} data
 */
export function updateEnv (state, { type, index, data }) {
  const target = state[type][index]
  const params = cloneDeep(data)
  if (index > 0 && 'TimeStamp' in data) {
    const {
      Date: {
        Day,
        Month,
        Year,
      },
      Time: {
        Hour,
        MilliSecond = 0,
        Minute,
        Second,
      },
    } = state[type][0]
    const startDateTime = new Date(Year, Month - 1, Day, Hour, Minute, Second, MilliSecond)
    const targetDateTime = new Date(startDateTime.getTime() + data.TimeStamp)
    params.Date = {}
    params.Date.Year = targetDateTime.getFullYear()
    params.Date.Month = targetDateTime.getMonth() + 1
    params.Date.Day = targetDateTime.getDay()

    params.Time = {}
    params.Time.Hour = targetDateTime.getHours()
    params.Time.Minute = targetDateTime.getMinutes()
    params.Time.Second = targetDateTime.getSeconds()
    params.Time.Millisecond = targetDateTime.getMilliseconds()
  }
  state[type].splice(index, 1, merge(target, params))
}

export function addEnv (state, type) {
  state[type].push(createDefaultEnv())
}

export function removeEnv (state, { type, index }) {
  state[type].splice(index, 1)
}
