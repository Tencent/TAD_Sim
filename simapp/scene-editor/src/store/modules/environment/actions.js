import { cloneDeep } from 'lodash-es'
import { createDefaultEnv } from './factory'
import { fetchGlobalEnvironment, saveGlobalEnvironment } from '@/api'
import { editor } from '@/api/interface'
import { errorHandler } from '@/common/errorHandler'
import mockElectron from '@/api/mock-electron'

const { electron = mockElectron } = window

/**
 * 获取配置
 * @param {object} context - Vuex 上下文对象，包含 dispatch 和 commit 方法
 * @returns {Promise<void>}
 */
export async function getConfig ({ dispatch, commit }) {
  await dispatch('getGlobalConfig')
  let scenarioEvnGroup
  try {
    scenarioEvnGroup = await editor.scenario.getCurrentEnvironment()
    if (!scenarioEvnGroup) {
      scenarioEvnGroup = [createDefaultEnv()]
      commit('updateState', {
        scenarioEvnGroup,
      })
      // 预设一个环境配置时保存一下，触发场景的isDirty
      dispatch('saveConfig')
    } else {
      scenarioEvnGroup = cloneDeep(scenarioEvnGroup)
      commit('updateState', {
        scenarioEvnGroup,
      })
    }
  } catch (e) {
    scenarioEvnGroup = [createDefaultEnv()]
    commit('updateState', {
      scenarioEvnGroup,
    })
    // 预设一个环境配置时保存一下，触发场景的isDirty
    dispatch('saveConfig')
  }
}

/**
 * 获取全局配置
 * @param {object} context - Vuex 上下文对象，包含 commit 方法
 * @returns {Promise<void>}
 */
export async function getGlobalConfig ({ commit }) {
  let globalEnvGroup
  try {
    globalEnvGroup = await fetchGlobalEnvironment()
    // 如果没有配置，则使用创建一套默认配置
    if (!Array.isArray(globalEnvGroup) || !globalEnvGroup.length) {
      globalEnvGroup = [createDefaultEnv()]
    }
  } catch (e) {
    globalEnvGroup = [createDefaultEnv()]
  }

  let environmentUseGlobal
  try {
    ({ environmentUseGlobal } = await electron.editor.sensor.loadConfig())
  } catch (e) {
    environmentUseGlobal = false
  }

  commit('updateState', {
    globalMode: environmentUseGlobal,
    globalEnvGroup,
  })
}

/**
 * 保存配置
 * @param {object} context - Vuex 上下文对象，包含 commit 和 state 方法
 * @returns {Promise<void>}
 */
export async function saveConfig ({ commit, state }) {
  commit('updateState', { loading: true })
  try {
    commit('scenario/updateState', { isDirty: true }, { root: true })
    await saveGlobalEnvironment(cloneDeep(state.globalEnvGroup))
    await electron.editor.sensor.saveConfig({ environmentUseGlobal: state.globalMode })
    await editor.scenario.setCurrentEnvironment(cloneDeep(state.scenarioEvnGroup))
  } catch (e) {
    await errorHandler(e)
  }
  commit('updateState', { loading: false })
}
