import {
  createDefaultSensorGroup,
  createDefaultSensorGroupFromValue,
  createDefaultSensorGroups,
  createSensorGroupFromValue,
} from '@/store/modules/sensor/factory'
import { fetchGlobalSensors, fetchSensors } from '@/api'
import { editor } from '@/api/interface'
import { errorHandler } from '@/common/errorHandler'
import { getPermission } from '@/utils/permission'

const { electron } = window
const sensorConfig = getPermission('action.sensors.view.list')

/**
 * 根据传感器类型过滤传感器数组 （教育版或其他特殊情况屏蔽某一种类传感器）
 * @param {object} sensor - 传感器对象
 * @returns {boolean} - 如果传感器类型在允许的配置中，则返回true，否则返回false
 */
export function sensorFilter (sensor) {
  return sensorConfig.includes(sensor.type)
}

/**
 * 获取传感器数据并进行本地化处理
 * @param {object} context - Vuex的上下文对象，包含dispatch和commit方法
 * @returns {Promise<void>} - 返回一个Promise，异步操作完成后resolve
 */
export async function getSensors ({ dispatch, commit }) {
  commit('updateState', { loading: true })
  const savedSensor = await editor.scenario.getCurrentSensor()
  let mainSensorGroup = savedSensor ? createSensorGroupFromValue(savedSensor) : createDefaultSensorGroup()
  mainSensorGroup = mainSensorGroup.filter(sensorFilter)

  commit('updateSensorGroup', { index: 0, group: mainSensorGroup })
  await dispatch('getGlobalSensors')
  commit('updateState', { loading: false })
}

/**
 * 获取全局传感器数据并进行本地化处理
 * @param {object} context - Vuex的上下文对象，包含commit方法
 * @returns {Promise<void>} - 返回一个Promise，异步操作完成后resolve
 */
export async function getGlobalSensors ({ commit }) {
  let defaultSensorGroups = createDefaultSensorGroups()
  try {
    const defaultGroups = await fetchSensors()

    if (defaultGroups) {
      defaultSensorGroups = defaultGroups.map(group => createDefaultSensorGroupFromValue(group))
    }
  } catch (e) {
    await errorHandler(e)
  }
  let globalSensorGroup
  try {
    globalSensorGroup = createSensorGroupFromValue(await fetchGlobalSensors())
  } catch (e) {
    globalSensorGroup = createDefaultSensorGroup()
  }

  let sensorUseGlobal
  try {
    ({ sensorUseGlobal } = await electron.editor.sensor.loadConfig())
  } catch (e) {
    sensorUseGlobal = false
  }

  defaultSensorGroups = defaultSensorGroups.map(group => group.filter(sensorFilter))
  globalSensorGroup = globalSensorGroup.filter(sensorFilter)
  commit('acceptDefaultGroups', defaultSensorGroups)
  commit('acceptGlobalGroups', globalSensorGroup)
  commit('updateState', { globalMode: sensorUseGlobal })
}
