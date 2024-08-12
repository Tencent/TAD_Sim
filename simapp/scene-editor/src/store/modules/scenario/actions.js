import { cloneDeep, find, get, isEmpty, isNil, throttle } from 'lodash-es'
import { h } from 'vue'
import { ElMessage } from 'element-plus'
import {
  getModuleLogFileName,
  resetModuleLogFileName,
  setModuleLogFileName,
  throttleModuleLogFileName,
} from '@/stored/module-log-file-name'
import { resetModuleStatus, setModuleStatus, throttleModuleStatus } from '@/stored/module-status'
import { resetModuleTimeCost, setModuleTimeCost, throttleModuleTimeCost } from '@/stored/module-time-cost'
import { pushGradingKPI, resetGradingKPIList, throttleGradingKPI } from '@/stored/grading-kpi'
import { pushPbMessage, resetPbMessage, throttlePbMessage } from '@/stored/pb-message'
import { resetProtoMessage, setProtoMessage, throttleProtoMessage } from '@/stored/proto-message'
import { resetLogMessage, setLogMessage, throttleLogMessage } from '@/stored/log-message'
import { resetTimestamp, setTimestamp, throttleTimestamp } from '@/stored/timestamp'
import {
  delScenarios as delScenariosService,
  exportScenarios as exportScenariosService,
  getScenario,
  getScenarioList as getScenarioListService,
  importScenarios as importScenariosService,
  importScenariosSub as importScenariosServiceSub,
} from '@/api/scenario'
import dict from '@/common/dict'
import { errorHandler, showErrorMessage } from '@/common/errorHandler'
import { common, editor, player } from '@/api/interface'
import i18n from '@/locales'
import { delay, getNow } from '@/common/utils'
import simInfo1Message from '@/common/simInfo1Message'
import eventBus from '@/event-bus'
import { resetCarDashboard, setCarDashboard, throttleCarDashboard } from '@/stored/car-dashboard'
import filters from '@/filters'
import { getDecoder } from '@/common/proto-define'
import { messageBoxConfirm, messageBoxError, messageBoxInfo, messageBoxSuccess } from '@/plugins/message-box'

const { electron } = window
const { $itemName } = filters

/**
 * 检查场景对象中的触发器是否填写了正确的 targetelement 或 element_ref 字段
 * @param {Array} scenarioObjects - 场景对象数组
 * @returns {object} - 返回检查结果，包含 res（是否通过检查）和 unFilledItems（未填写的项目列表）
 */
function checkScenarioTriggers (scenarioObjects) {
  /*
  * 检查所有 objects 的 triggers 中应该填写的 targetelement/tartget_element 字段是否填写
  * (删除某 object 可能会导致其他某个 object 的 trigger 中 targetelement/target_element 字段被置空)
  * 检查所有 objects 的 triggers 中应该填写的 element_ref 字段是否填写(删除某 object 可能会导致其他某个 object 的 trigger 中 element_ref 字段被置空)
  * 不在 simPlayer 里检查的原因是这里方便使用 objects 查看所有 object, simPlayer 中需要查找对应字段寻找所有 object
  * */
  const unFilledItems = []
  scenarioObjects.forEach((object) => {
    const triggers = object.triggers || object.sceneevents
    const label = $itemName(object)
    // 普通 object 的所有触发信息存储在 triggers 字段下, 主车的存储在 sceneevents 字段下
    triggers && triggers.forEach((trigger) => {
      trigger.conditions && trigger.conditions.forEach((condition) => {
        if (['ttc_trigger', 'reach_position', 'distance_trigger', 'velocity_trigger', 'timeheadway_trigger'].includes(condition.type)) {
          if (!condition.targetelement) {
            unFilledItems.push(i18n.t('tips.triggerNeedUpdate', { objectLabel: label, triggerName: trigger.name }))
          }
        } else if (['element_state'].includes(condition.type)) {
          if (!condition.element_ref) {
            unFilledItems.push(i18n.t('tips.triggerNeedUpdate', { objectLabel: label, triggerName: trigger.name }))
          }
        }
      })

      trigger.action && trigger.action.forEach((action) => {
        if (['lateralDistance'].includes(action.actiontype)) {
          if (!action.multi.target_element) {
            unFilledItems.push(i18n.t('tips.triggerNeedUpdate', { objectLabel: label, triggerName: trigger.name }))
          }
        }
      })
    })
  })

  if (unFilledItems.length) {
    return { res: false, unFilledItems }
  } else {
    return { res: true }
  }
}

/**
 * 获取场景列表
 * @param {object} commit - Vuex commit 对象
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function getScenarioList ({ commit }) {
  try {
    const { list: scenarioList = [] } = await getScenarioListService()
    scenarioList.forEach((scenario) => {
      const scenarioSetIds = scenario.set.split(',')
      const scenarioSetList = []
      scenarioSetIds.forEach((scenarioSetId) => {
        if (scenarioSetId) {
          scenarioSetList.push(+scenarioSetId)
        }
      })
      scenario.set = scenarioSetList
    })
    scenarioList.sort((a, b) => {
      return a.name.localeCompare(b.name)
    })
    commit('updateState', { scenarioList })
  } catch (e) {
    await errorHandler(e)
  }
}

/**
 * 导入场景
 * @param {object} commit - Vuex commit 对象
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function importScenario ({ commit }) {
  try {
    const objects = await editor.scenario.importScenario('byl0601.sim')
    commit('updateState', {
      objects,
      isDirty: true,
    })
  } catch (e) {
    commit('updateState', { loading: false })
    await errorHandler(e)
  }
}

/**
 * 打开场景
 * @param {object} dispatch - Vuex dispatch 对象
 * @param {object} commit - Vuex commit 对象
 * @param {object} state - Vuex state 对象
 * @param {object} getters - Vuex getters 对象
 * @param {object} scenario - 要打开的场景对象
 * @returns {Promise<boolean>} - 返回一个布尔值，表示操作是否成功
 */
export async function openScenario ({ dispatch, commit, state, getters }, scenario) {
  const canOpen = await editor.scenario.canOpenScenario(scenario)
  if (!canOpen) {
    messageBoxError(i18n.t('tips.mapLoadFailed'))
    return false
  }

  if (state.currentScenario && scenario.id === state.currentScenario.id && !getters.isPlaying) {
    try {
      await messageBoxConfirm(i18n.t('tips.openWhileOpening'))
    } catch (e) {
      return false
    }
  } else if (state.isDirty) {
    try {
      await messageBoxConfirm(i18n.t('tips.openWithoutSaving'))
    } catch (e) {
      return false
    }
  }
  commit('updateState', { loading: true })
  commit('stage/updateState', { degree: 0 }, { root: true })
  // 重置一些状态
  commit('updateState', {
    currentPerspective: 'top',
    expandedKeys: [],
    selectedObjectKey: '',
  })
  dispatch('updateOperationMode', 1)
  editor.getSignals('updateRenderState').dispatch({
    key: 'useTopCamera',
    value: true,
  })
  try {
    const objects = await editor.scenario.open(scenario.id)
    const generateInfo = editor.scenario.getGenerateInfo()
    const uiStateInfo = editor.scenario.getUIStateInfo()
    const roadObj = editor.scenario.getRoadobj()
    const sceneevents = editor.scenario.getSceneevents()
    // todo: plannerEvents
    const signlightPlans = editor.scenario.getSignlightPlans()
    commit('updateState', {
      objects,
      currentScenario: scenario,
      generateInfo,
      sceneevents: sceneevents || [],
      newScenario: null,
      status: 'modifying',
      isDirty: false,
      factor: (uiStateInfo && uiStateInfo.factor) || 1,
      roadObj,
    })
    dispatch('signalControl/initSignlightPlans', signlightPlans, { root: true })
    dispatch('signalControl/refreshSignlightPlans', undefined, { root: true })
    // dispatch('sensor/getSensors', null, { root: true })
    dispatch('environment/getConfig', null, { root: true })
  } catch (e) {
    commit('updateState', { loading: false })
    await errorHandler(e)
    return false
  }
  dispatch('pushHistory', scenario.filename)
  commit('updateState', { loading: false })

  const trafficAI = editor.scenario.getTrafficAIConfig()
  commit('updateState', { trafficAI })
  return true
}

/**
 * 卸载场景
 * @param {object} dispatch - Vuex dispatch 对象
 * @param {object} commit - Vuex commit 对象
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function unloadScenario ({ dispatch, commit }) {
  dispatch('resetCameraState')

  try {
    // 初始化视角
    dispatch('resetCameraState')

    await editor.scenario.unload()

    commit('updateState', {
      objects: [],
      currentScenario: null,
      generateInfo: null,
      newScenario: null,
      status: 'modifying',
      isDirty: false,
      factor: 1,
      roadObj: {},
    })
    await dispatch('sensor/resetScenarioSensorGroup', null, { root: true })
  } catch (e) {
    await errorHandler(e)
  }

  const trafficAI = editor.scenario.getTrafficAIConfig()
  commit('updateState', { trafficAI })
}

/**
 * 创建场景
 * @param {object} commit - Vuex commit 对象
 * @param {object} dispatch - Vuex dispatch 对象
 * @param {object} payload - 创建场景所需的数据
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function createScenario ({ commit, dispatch }, payload) {
  try {
    // 每次新建场景都初始化视角
    dispatch('resetCameraState')

    await editor.scenario.add(payload)

    const uiStateInfo = editor.scenario.getUIStateInfo()
    const roadObj = editor.scenario.getRoadobj()
    const signlightPlans = editor.scenario.getSignlightPlans()
    commit('updateState', {
      objects: [],
      currentScenario: null,
      generateInfo: null,
      newScenario: {
        ...payload,
        preset: '0', // 对于新建的场景，添加非预设的字段值
      },
      sceneevents: [],
      // plannerEvents: [],
      status: 'modifying',
      isDirty: true,
      factor: (uiStateInfo && uiStateInfo.factor) || 1,
      roadObj,
    })
    dispatch('signalControl/initSignlightPlans', signlightPlans, { root: true })
    dispatch('signalControl/refreshSignlightPlans', undefined, { root: true })
    // 这个方法可以创建一套默认的环境配置
    await dispatch('environment/getConfig', null, { root: true })
  } catch (e) {
    await errorHandler(e)
  }

  const trafficAI = editor.scenario.getTrafficAIConfig()
  commit('updateState', { trafficAI })
}

/**
 * 选择对象
 * @param {object} commit - Vuex commit 对象
 * @param {object} payload - 选择的对象数据
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function selectObject ({ commit }, payload) {
  // payload null || { type: multiple, id: 'idx', index: -1 } || object
  let object = payload
  if (payload && payload.type === 'multiple' && payload.id === 'ids' && payload.index === -1) {
    object = null
  }
  editor.object.select(object)
  commit('selectObject', payload)
}

/**
 * 删除对象
 * @param {object} commit - Vuex commit 对象
 * @param {object} state - Vuex state 对象
 * @param {object} getters - Vuex getters 对象
 * @param {object} obj - 要删除的对象
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function removeObject ({ commit, state, getters }, obj) {
  let target = obj
  if (!target) {
    target = getters.selectedObject
  }
  if (!target) {
    return
  }

  let editSceneevents
  if (target.eventId) {
    editSceneevents = cloneDeep(state.sceneevents)
    target.eventId.split(',').forEach((id) => {
      editSceneevents.splice(editSceneevents.findIndex(_ => _.id === id), 1)
    })
  }

  // 如果其他 object 内触发中有对象选择字段, 且对象选择字段的值为该待删除 object , 则置空. 同时修改 originSceneevent 中对应 event 的 对象字段
  // 如果待删除 object 中存在其他 object 的故事板事件触发的引用行为, 则置空. 同时修改 originSceneevent 中对应 event 的 引用行为字段
  const deleteObjAbbrevName = target.type === 'planner' ? `ego_${target.id}` : `${target.type === 'car' ? 'v' : 'p'}_${target.id}`
  const deleteObjRefActions = []

  // 普通 object 的所有触发信息存储在 triggers 字段下, 主车的存储在 sceneevents 字段下
  const objectTriggers = target.type === 'planner' ? target.sceneevents : target.triggers
  objectTriggers && objectTriggers.forEach((trigger) => {
    trigger.action && trigger.action.forEach((action) => {
      deleteObjRefActions.push(`${trigger.id}_${action.actionid}`)
    })
  })

  state.objects.forEach((_obj) => {
    if (Number(_obj.id) !== Number(target.id) || _obj.type !== target.type) {
      let ifUpdate = false
      const triggerPath = _obj.type === 'planner' ? 'sceneevents' : 'triggers'
      const objTriggers = _obj[triggerPath] ? cloneDeep(_obj[triggerPath]) : []
      objTriggers.forEach((trigger) => {
        trigger.conditions && trigger.conditions.forEach((condition, cIndex, cArr) => {
          // 其他 object 内触发中的 conditions 的某些 condition 有对象选择字段, 且该对象字段为待删除 object
          if (condition.targetelement === deleteObjAbbrevName) {
            // 修改 Sceneevents 对应元素
            if (!editSceneevents) {
              editSceneevents = cloneDeep(state.sceneevents)
            }
            let sceneevents
            if (_obj.type === 'planner') {
              sceneevents = _obj.sceneevents
            } else {
              sceneevents = editSceneevents
            }
            const sceneevent = sceneevents.find(item => Number(item.id) === Number(trigger.id))
            const targetCondition = sceneevent &&
              sceneevent.conditions.find(condition => condition.targetelement === deleteObjAbbrevName)
            if (targetCondition) {
              targetCondition.targetelement = ''
            }

            // 修改 object.triggers 里对应元素
            ifUpdate = true
            cArr[cIndex].targetelement = ''
          }

          // 待删除 object 中存在其他同类型(主车类型/非主车类型) object 的故事板事件触发的引用行为
          if (
            condition.type === 'element_state' &&
            deleteObjRefActions.includes(condition.element_ref) &&
            ((_obj.type === 'planner') === (target.type === 'planner')) // 判断是否同类型
          ) {
            // 修改 Sceneevents 对应元素
            if (!editSceneevents) {
              editSceneevents = cloneDeep(state.sceneevents)
            }
            let sceneevents
            if (_obj.type === 'planner') {
              sceneevents = _obj.sceneevents
            } else {
              sceneevents = editSceneevents
            }
            const sceneevent = sceneevents.find(item => Number(item.id) === Number(trigger.id))
            const targetCondition = sceneevent &&
              sceneevent.conditions.find(condition => deleteObjRefActions.includes(condition.element_ref))
            if (targetCondition) {
              targetCondition.element_ref = ''
            }

            // 修改 object.triggers 里对应元素
            ifUpdate = true
            cArr[cIndex].element_ref = ''
          }
        })

        trigger.action && trigger.action.forEach((action, aIndex, aArr) => {
          // 其他 object 内触发中的 action 的某些元素中有对象选择字段, 且该对象字段为待删除 object
          if (action.multi && action.multi.target_element === deleteObjAbbrevName) {
            // 修改 Sceneevents 对应元素
            if (!editSceneevents) {
              editSceneevents = cloneDeep(state.sceneevents)
            }
            let sceneevents
            if (_obj.type === 'planner') {
              sceneevents = _obj.sceneevents
            } else {
              sceneevents = editSceneevents
            }
            const sceneevent = sceneevents.find(item => Number(item.id) === Number(trigger.id))
            const targetAction = sceneevent &&
              sceneevent.action.find(action => action.multi.target_element === deleteObjAbbrevName)
            if (targetAction) {
              targetAction.multi.target_element = ''
            }

            // 修改 object.triggers 里对应元素
            ifUpdate = true
            aArr[aIndex].multi.target_element = ''
          }
        })
      })
      if (ifUpdate) {
        if (_obj.type === 'planner') {
          _obj.sceneevents = objTriggers
        } else {
          _obj.triggers = objTriggers
        }
        const payload = {
          type: _obj.type,
          data: _obj,
        }
        editor.object.update(payload)
        commit('updateObject', payload)
      }
    }
  })

  if (editSceneevents) {
    commit('updateSceneevents', editSceneevents)
  }
  // commit('updatePlannerEvents', editPlannerEvents)
  // if (target.type === 'planner') { commit('updatePlannerEvents', []) } // 如果删除对象是主车，则置空 plannerEvents (若是多主车则需要重新考虑)

  {
    // 当删除对象是主车或者交通车辆时
    // 且删除的对象被设置为其他 car 的跟车跟随对象，则将其他 car 的跟随对象置空
    const { type, id } = target
    if (['planner', 'car'].includes(type)) {
      state.objects.forEach((object) => {
        if (object.follow && object.type === 'car') {
          const delFollowName = filters.$followName({ id, type })
          if (object.follow === delFollowName) {
            object.follow = ''
          }
        }
      })
    }
  }

  try {
    await editor.object.del(target)
    commit('deleteObject', target)
  } catch (e) {
    await errorHandler(e)
  }
}

/**
 * 保存场景
 * @param {object} commit - Vuex commit 对象
 * @param {object} state - Vuex state 对象
 * @param {object} dispatch - Vuex dispatch 对象
 * @returns {Promise<boolean>} - 返回一个布尔值，表示操作是否成功
 */
export async function saveScenario ({ commit, state, dispatch }) {
  if (!state.isDirty) return false
  let scenario = state.currentScenario || state.newScenario
  const id = scenario.id || -1
  const { result, message } = editor.scenario.validate()
  const errorMessage = `${i18n.t('tips.scenarioSavedFail')}:${message}`
  if (!result) {
    messageBoxError(errorMessage)
    return false
  }

  const { res, unFilledItems } = checkScenarioTriggers(state.objects)
  if (!res) {
    simInfo1Message({ title: i18n.t('tips.cantSaveOrPlayBecauseEventNeedUpdate'), message: unFilledItems })
    return
  }

  try {
    scenario = await editor.scenario.save(id, scenario.name, scenario.type)
    const generateInfo = editor.scenario.getGenerateInfo()
    if (state.newScenario) {
      await dispatch('pushHistory', scenario.filename)
      await dispatch('getScenarioList')
      await dispatch('scenario-set/getScenarioSetList', null, { root: true })
    }
    commit('updateState', {
      isDirty: false,
      currentScenario: scenario,
      generateInfo,
      newScenario: null,
    })
    messageBoxSuccess(i18n.t('tips.scenarioSavedSuccess'))
    return true
  } catch (e) {
    await errorHandler(e)
    return false
  }
}

/**
 * 另存为场景
 * @param {object} commit - Vuex commit 对象
 * @param {object} state - Vuex state 对象
 * @param {object} dispatch - Vuex dispatch 对象
 * @param {object} getters - Vuex getters 对象
 * @param {object} payload - 另存为场景所需的数据
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function saveAs ({ commit, state, dispatch, getters }, payload) {
  if (state.status !== 'modifying' || !getters.presentScenario) return false
  const { name, ext } = payload
  let scenario = state.currentScenario || state.newScenario
  // const id = scenario.id || -1
  const { result, message } = editor.scenario.validate()
  const errorMessage = `${i18n.t('tips.scenarioSavedFail')}:${message}`
  if (!result) {
    messageBoxError(errorMessage)
    return
  }
  const { res, unFilledItems } = checkScenarioTriggers(state.objects)
  if (!res) {
    simInfo1Message({ title: i18n.t('tips.cantSaveOrPlayBecauseEventNeedUpdate'), message: unFilledItems })
    return
  }
  try {
    scenario = await editor.scenario.saveAs(name, ext)
    const generateInfo = editor.scenario.getGenerateInfo()
    // if (state.newScenario) {
    dispatch('pushHistory', `${name}.${ext}`)
    // }
    await dispatch('getScenarioList')
    await dispatch('scenario-set/getScenarioSetList', null, { root: true })
    commit('updateState', {
      isDirty: false,
      currentScenario: scenario,
      generateInfo,
      newScenario: null,
    })
    messageBoxSuccess(i18n.t('tips.scenarioSavedSuccess'))
  } catch (e) {
    await errorHandler(e)
  }
}

/**
 * 重置场景
 * @param {object} dispatch - Vuex dispatch 对象
 * @param {object} commit - Vuex commit 对象
 * @param {object} state - Vuex state 对象
 * @param {object} payload - 重置场景所需的数据
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export async function resetScenario ({ dispatch, commit, state }, payload) {
  if (state.playingStatus === 'playing') {
    await messageBoxInfo(i18n.t('tips.currentScenarioIsPlaying'))
    return
  }

  const hasMap = await common.hasMap(payload)
  if (!hasMap) {
    messageBoxError(i18n.t('tips.mapLoadFailed'))
    return
  }

  commit('updateState', { loading: true })
  let objects = []
  try {
    objects = await player.setScenario(payload.id)
    resetGradingKPIList()
    resetPbMessage()
    dispatch('pb-config/resetChosenPbConfigList', undefined, { root: true })
  } catch (e) {
    commit('updateState', { loading: false })
    await errorHandler(e)
    return
  }
  commit('updateState', { loading: false })

  try {
    const generateInfo = editor.scenario.getGenerateInfo()

    commit('updateState', {
      currentScenario: payload,
      objects,
      generateInfo,
      newScenario: null,
      selectedObjectKey: '',
      isDirty: false,
      status: 'playing',
      // factor: 1,  // 保持上一次的缩放位置
    })

    dispatch('setup', payload)
  } catch (e) {
    await errorHandler(e)
  }
}

/**
 * 放大场景
 * @param {object} state - Vuex state 对象
 * @returns {Promise<void>} - 返回一个空 Promise
 */
export function zoomIn ({ state }) {
  switch (state.status) {
    case 'playing':
      try {
        return player.zoom(1.1)
      } catch (e) {
        errorHandler(e)
      }
      break
    case 'modifying':
      return editor.scenario.zoom(1.1)
  }
}

export function zoomOut ({ state }) {
  switch (state.status) {
    case 'playing':
      try {
        return player.zoom(0.9)
      } catch (e) {
        errorHandler(e)
      }
      break
    case 'modifying':
      return editor.scenario.zoom(0.9)
  }
}

export function setEnd (context, payload) {
  const { type, id } = payload
  return editor.scenario.setEnd(type, id)
}

export async function selectPathEnd (context, payload) {
  const conversion = {
    planner: 'egopath_end',
    car: 'vehiclepath_end',
    moto: 'motopath_end',
    bike: 'bikepath_end',
    pedestrian: 'pedestrianpath_end',
    animal: 'animalpath_end',
    machine: 'machinepath_end',
  }
  let { type } = payload
  type = conversion[type]
  if (type) {
    editor.object.select({
      ...payload,
      type,
    })
  }
}

export async function removePathPoint ({ commit }, payload) {
  // return when not editing
  if (editor.scenario.getState() !== 0) {
    return
  }

  const { type, id, index, endPosArr } = payload
  try {
    await editor.scenario.removePoint(type, { id, index })
    let newPosArr = []
    if (index === -1) {
      if (endPosArr.length >= 1) {
        newPosArr.push(endPosArr[0])
      }
    } else {
      newPosArr = endPosArr.filter((item, i) => i !== index)
    }
    commit('updateObject', {
      type,
      data: {
        id,
        endPosArr: newPosArr,
      },
    })
  } catch (e) {
    await errorHandler(e)
  }
}

export function switchPerspective ({ state, commit, dispatch }, payload) {
  switch (state.status) {
    case 'modifying':
      editor.setCamera(payload)
      commit('updateState', {
        currentPerspective: payload,
      })
      if (payload === 'top') {
        dispatch('updateOperationMode', 1)
      }
      break
    case 'playing':
      player.setCamera(payload)
      commit('updateState', {
        currentPerspectivePlaying: payload,
      })
      break
    default:
  }
}

export async function backToPlanner ({ state, dispatch, getters }, id) {
  switch (state.status) {
    case 'modifying': {
      const planners = get(getters, 'tree.planner', [])
      let planner
      if (id === undefined) {
        planner = planners[0]
      } else {
        planner = planners.find(p => p.id === id)
      }
      if (planner) {
        await dispatch('selectObject', planner)
      }
    }
      break
    case 'playing':
    // todo:
      player.locateObject({ type: 'planner', id })
      break
    default:
  }
}

export async function switchStatus ({ state, commit, dispatch, rootGetters }, targetStatus) {
  if (state.status === targetStatus) return
  switch (targetStatus) {
    case 'playing':
      if (state.isDirty && (state.currentScenario || state.newScenario)) { // 不打开场景时不提示  fix 5894
        if (state.currentScenario && state.currentScenario.preset === '') {
          try {
            await messageBoxConfirm(i18n.t('tips.playWithoutSaving'))
            commit('mission/startMission', 'SaveAs', { root: true })
          } catch (e) {
          // pass
          }
          return
        } else {
          try {
            await messageBoxConfirm(i18n.t('tips.playWithoutSaving'))
            const result = await dispatch('saveScenario')
            if (!result) {
              return
            }
          } catch (e) {
            return
          }
        }
      }

      commit('playlist/updateState', { keyword: '' }, { root: true })

      if (state.currentScenario) {
        const hasMap = await common.hasMap(state.currentScenario)
        if (!hasMap) {
          messageBoxError(i18n.t('tips.mapLoadFailed'))
          return
        }

        const playIdsList = rootGetters['playlist/playIdsList']
        if (!playIdsList.includes(state.currentScenario.id)) {
          dispatch('playlist/updatePlaylist', [
            state.currentScenario.id,
            ...playIdsList,
          ], { root: true })
        }
        try {
          await player.scenario.updateHadmapInfo()
          await dispatch('resetScenario', state.currentScenario)
          commit('updateState', { startPlaylistScenarioId: state.currentScenario.id })
        } catch (e) {
          await errorHandler(e)
        }
      }

      commit('updateState', {
        newScenario: null,
        selectedObjectKey: '',
        isDirty: false,
        status: 'playing',
        factor: 1,
      })

      dispatch('switchPerspective', state.currentPerspectivePlaying)

      break
    case 'modifying':
      if (state.status === 'playing') {
        const { playingStatus } = state
        if (playingStatus === 'playing') {
          await messageBoxInfo(i18n.t('tips.currentScenarioIsPlaying'))
          return
        }
      }

      if (state.currentScenario && state.currentScenario.traffictype === 'simrec') {
        await dispatch('unloadScenario')
      } else {
        if (state.currentScenario) {
          await dispatch('openScenario', state.currentScenario)
        } else {
        // commit('updateState', {
        //   currentScenario: rootState.playlist.prevPlayingScenario,
        // })
          await dispatch('unloadScenario')
        }
      }
      commit('updateState', { status: 'modifying' })
      commit('updateState', { shouldPlayAfterSetup: false })
      dispatch('switchPerspective', 'top')
      break
    case 'renderPlaying':
      if (state.isDirty) {
        try {
          await messageBoxConfirm(i18n.t('tips.playWithoutSaving'))
          await dispatch('saveScenario')
        } catch (e) {
          return
        }
      }
      try {
        await player.setScenario(state.currentScenario.id)
        resetGradingKPIList()
        resetPbMessage()
      } catch (e) {
        await errorHandler(e)
        return
      }
      console.log('render play')
      // todo: play场景并启动display
      break
    case 'mapEditor':
      break
    case 'static':
      break
    default:
  }
}

export function pushHistory ({ dispatch }, history) {
  electron.editor.recent.add(history)
  dispatch('getHistories')
}

export function delHistories ({ dispatch }, history) {
  electron.editor.recent.del(history)
  dispatch('getHistories')
}

export function getHistories ({ commit }) {
  const list = electron.editor.recent.get() || []
  commit('updateState', {
    openHistory: list.slice(0, 5),
  })
}

export async function copyScenario ({ dispatch }, scene) {
  try {
    const arr = [{
      name: `${scene}.sim`,
      newName: `${scene}(1).sim`,
    }]
    await editor.scenario.copy(arr)
    await dispatch('getScenarioList')
    await dispatch('scenario-set/getScenarioSetList', null, { root: true })
  } catch (e) {
    await errorHandler(e)
  }
}

export async function delScenarios ({ commit, dispatch, state }, ids) {
  try {
    await delScenariosService(ids)
    await dispatch('playlist/removeScenariosFromPlaylist', ids, { root: true })
    await dispatch('getScenarioList')
    await dispatch('scenario-set/getScenarioSetList', null, { root: true })
    if (state.currentScenario && ids.includes(state.currentScenario.id)) {
      commit('updateState', {
        newScenario: state.currentScenario,
        currentScenario: null,
        generateInfo: null,
        isDirty: true,
      })
      ElMessage.info(i18n.t('tips.deletedScenariosIncludeOpen'))
    }
    await dispatch('delHistories', ids)
  } catch (e) {
    await errorHandler(e)
  }
}

export async function importScenarios ({ dispatch }, payload) {
  try {
    const { code, message } = await importScenariosService(payload)
    await dispatch('getScenarioList')
    await dispatch('scenario-set/getScenarioSetList', null, { root: true })
    if (code === 0) {
      ElMessage.success(i18n.t('tips.completeScenarioImport'))
    } else {
      ElMessage.error({
        message,
      })
    }
  } catch (e) {
    await errorHandler(e)
  }
}

// 分部分导入场景列表
export async function importScenariosSub ({ dispatch }, payload) {
  try {
    const { code, message = '' } = await importScenariosServiceSub(payload)
    await dispatch('getScenarioList')
    await dispatch('scenario-set/getScenarioSetList', null, { root: true })
    if (code === 0) {
      ElMessage.success(i18n.t('tips.importScenarioSuccess'))
      return true
    } else if (code === -1) {
      const result = message.split(';') || []
      const task = setInterval(() => {
        const msg = result.shift()
        if (msg) {
          ElMessage.error(msg)
        }
        if (result.length === 0) {
          clearInterval(task)
        }
      }, 1000)
      return false
    }
  } catch (e) {
    await errorHandler(e)
    return false
  }
}

export async function exportScenarios (context, payload) {
  try {
    const { code, message } = await exportScenariosService(payload)
    if (code === 0) {
      ElMessage.success(i18n.t('tips.exportScenarioSuccess'))
    } else {
      ElMessage.error({
        message,
      })
    }
  } catch (e) {
    await errorHandler(e)
  }
}

export function saveTrafficAI ({ commit }, payload) {
  // todo: 保存配置
  commit('updateTraffic', payload)
  commit('updateState', {
    isDirty: true,
  })
  editor.scenario.applyTrafficAIConfig(payload)
}

function showSuccessMessage (message) {
  return ElMessage.success({
    message,
  })
}

export async function paramScene ({ dispatch }, payload) {
  const { id, prefix, time } = payload
  delete payload.id
  delete payload.prefix
  delete payload.time
  const promise = editor.scenario.paramScene(id, prefix, payload)
  await dispatch('mission/progress', {
    time,
    title: i18n.t('tips.SceneImporting'),
    promise,
  }, { root: true })
  const { dir: scenarioFolderName } = await promise
  try {
    await messageBoxConfirm(i18n.t('tips.generateScenariosSuccess'), {
      cancelButtonText: i18n.t('tips.openTheFolder'),
      closeOnPressEscape: false,
      closeOnClickModal: false,
    })
  } catch (e) {
    electron.editor.openBatchGenerateScenarioPath(scenarioFolderName)
  }
}

export async function paramSceneNew ({ commit }, payload) {
  const { id, prefix } = payload
  delete payload.id
  delete payload.prefix
  delete payload.time
  const promise = editor.scenario.paramScene(id, prefix, payload)

  commit('progressInfo/updateProgressInfo', {
    mode: 20, // 场景生成
    status: true,
    progress: 0,
  }, { root: true })

  const { dir: scenarioFolderName, err } = await promise

  commit('progressInfo/updateProgressInfo', {
    status: false,
  }, { root: true })

  if (err === 0) {
    try {
      await messageBoxConfirm(i18n.t('tips.generateScenariosSuccess'), {
        cancelButtonText: i18n.t('tips.openTheFolder'),
        closeOnPressEscape: false,
        closeOnClickModal: false,
      })
    } catch (e) {
      const isOpened = electron.editor.openBatchGenerateScenarioPath(scenarioFolderName)
      !isOpened && ElMessage.error(i18n.t('tips.openGeneratedScenesFolderFail'))
    }
  }
}

function showOpenKpiReportMessage ({ commit }, message) {
  ElMessage.success({
    message: h('div', {
      style: {
        display: 'flex',
        justifyContent: 'space-between',
        width: '100%',
        fontSize: '12px',
        color: '#67c23a',
      },
    }, [
      h('span', { style: { flex: 1 } }, message),
      h('span', {
        style: {
          display: 'inline-block',
          cursor: 'pointer',
          textDecoration: 'underline',
        },
        onClick () {
          commit('mission/startMission', 'KpiReportManagement', { root: true })
        },
      }, i18n.t('tips.viewReport')),
    ]),
    offset: 50,
  })
}

async function showErrorList (list) {
  for (const item of list) {
    const { name, code, err } = item
    const messages = [`[${name}]`, i18n.t(dict.moduleCode[code]) || code]
    if (err) {
      messages.push(err)
    }
    await showErrorMessage(messages.join(' '))
  }
}

async function showPlayCompletedMessage (context, list) {
  await showOpenKpiReportMessage(context, i18n.t('tips.playbackScenarioComplete'))
  for (const item of list) {
    const { name, msg } = item
    if (msg) {
      await showSuccessMessage(`${i18n.t('tips.reasonForFinishedPlay')}: [${name}] ${msg}`)
    }
  }
}

let setupReady = false

const throttleSendV2x = throttle((content) => {
  eventBus.$emit('v2x-warning-info', content)
}, 4000)

function renderFrame (data, context) {
  const { cmdStatus, msgStatus } = data
  const { commit, state, rootState } = context
  const { timestamp = -1, groups = [] } = msgStatus
  const messages = []
  msgStatus.messages = messages
  groups.forEach((g) => {
    if (g.name.startsWith('Ego')) {
      messages.push(...g.messages.map(m => ({ ...m, topic: `${g.name}/${m.topic}` })))
    } else if (g.name === 'common') {
      messages.push(...g.messages)
    }
  })
  messages.forEach((message) => {
    const { topic, content } = message
    const decoder = getDecoder(topic)
    if (decoder) {
      const buffer = new Uint8Array(content)
      const kb = buffer.length * Uint8Array.BYTES_PER_ELEMENT / 1024
      const t0 = performance.now()

      message.content = decoder.decode(buffer, buffer.length)

      // if (topic.endsWith('TRAJECTORY')) {
      //   console.log(buffer, message.content, 'trajectory')
      // }

      const t1 = performance.now()
      const ms = t1 - t0
      const output = `${getNow()}: deserialize ${topic} ${kb.toFixed(2)}KB ${ms.toFixed(2)}ms`

      if (kb >= 200 || ms >= 10) {
        console.error(output)
      } else if (kb >= 100 || ms >= 5) {
        console.warn(output)
      }
    }
  })
  const { followingPlannerId } = rootState.scenario
  let followingPlannerPrefix = ''
  if (!isNil(followingPlannerId)) {
    followingPlannerPrefix = `Ego_${`${followingPlannerId}`.padStart(3, '0')}/`
  }
  setTimestamp(timestamp)
  throttleTimestamp(commit)
  const grading = find(messages, { topic: `${followingPlannerPrefix}GRADING` })
  const v2xWarning = find(messages, { topic: `${followingPlannerPrefix}V2XEARLYWARNING` })
  const vehicleState = find(messages, { topic: `${followingPlannerPrefix}VEHICLE_STATE` })
  const controlV2 = find(messages, { topic: `${followingPlannerPrefix}CONTROL_V2` })
  const location = find(messages, { topic: `${followingPlannerPrefix}LOCATION` })
  const carDashboard = {}
  if (messages?.length) {
    pushPbMessage(msgStatus)
    throttlePbMessage(state.playingStatus)
  }
  delete msgStatus.groups
  if (grading?.content) {
    pushGradingKPI(grading.content)
    throttleGradingKPI(state.playingStatus)
    carDashboard.distHeadway = grading.content?.DistHeadway?.dist_to_fellow
  }
  if (cmdStatus?.length) {
    setModuleTimeCost(cmdStatus)
    throttleModuleTimeCost(commit, setupReady)
  }
  if (v2xWarning?.content) {
    // 该 TOPIC 发送消息频率不高, 不需要节流
    // test todo: delete throttle
    // eventBus.$emit('v2x-warning-info', v2xWarning.content)
    throttleSendV2x(v2xWarning.content)
  }
  if (vehicleState?.content) {
    carDashboard.brakePedalPos = vehicleState.content?.chassis_state?.BrakePedalPos
    carDashboard.accpedalPosition = vehicleState.content?.powertrain_state?.accpedal_position
    carDashboard.steeringWheelAngle = vehicleState.content?.chassis_state?.SteeringWheelAngle
    carDashboard.leftTurnSigLampSts = vehicleState.content?.body_state?.LeftTurnSigLampSts
    carDashboard.rightTurnSigLampSts = vehicleState.content?.body_state?.RightTurnSigLampSts
    carDashboard.geadMode = vehicleState.content?.powertrain_state?.gead_mode
    // carDashboard.speed = vehicleState.content?.powertrainState?.['speed_kph']
  }
  if (location?.content) {
    carDashboard.acceleration = location.content.acceleration
    carDashboard.rpy = location.content.rpy
    carDashboard.speed = location.content.velocity
  }
  if (controlV2?.content) {
    carDashboard.highBeam = controlV2.content?.body_command?.high_beam
    carDashboard.lowBeam = controlV2.content?.body_command?.low_beam
    carDashboard.rearFogLamp = controlV2.content?.body_command?.rear_fog_lamp
    carDashboard.frontFogLamp = controlV2.content?.body_command?.front_fog_lamp
    carDashboard.positionLamp = controlV2.content?.body_command?.position_lamp
    carDashboard.hazardLight = controlV2.content?.body_command?.hazard_light
  }

  if (!isEmpty(carDashboard)) {
    setCarDashboard(carDashboard)
    throttleCarDashboard()
  }

  // todo show module error message
  // todo feedback
  // todo grading feedback
  if (msgStatus) {
    try {
      if (state.currentScenario.traffictype === 'simrec') {
        messages.forEach((item) => {
          item.afterSwitch = rootState.log2world.realSwitchTime <= (msgStatus.timestamp || 0)
        })
      }
    } catch (error) {
      console.log(error.message)
    }
    setProtoMessage(msgStatus)
    throttleProtoMessage()
    player.scenario.render(msgStatus)
  }
}

async function afterPlayFinished (context, mode) {
  const { state, rootState, commit, dispatch, rootGetters } = context
  const { playingTimes, times, startPlaylistScenarioId } = state
  let { currentScenario } = state
  let shouldPlayAfterSetup = false
  let shouldReset = false

  if (currentScenario === null) {
    return
  }
  // logsim播放完成之后需要重置一下主车、轨迹等的位置，不然下次播放的时候会有残留
  // todo: 2023/09/06 会造成播放完成后无法观察最后一帧，这里再做打算
  // player.scenario.resetScene()

  if (mode === 'auto') {
    if (state.playingMode === 'single') {
      // pass
    } else if (state.playingMode === 'list') {
      const filteredPlaylist = rootGetters['playlist/filteredPlaylist']
      const prevIndex = filteredPlaylist.findIndex((item) => {
        return item.id === state.currentScenario.id
      })
      // 每次播完延迟几秒再切，给用户看清楚结束画面的时间
      await delay(3000)
      if (prevIndex !== -1) {
        const { length } = filteredPlaylist
        const index = (prevIndex + 1) % length
        const startPlaylistScenarioIndex = filteredPlaylist.findIndex(item => item.id === startPlaylistScenarioId)
        let skipped = false
        if (times === playingTimes && index === startPlaylistScenarioIndex) {
          skipped = true
        }
        if (skipped) {
          ElMessage.success({
            message: i18n.t('tips.playbackScenariosComplete'),
          })
        } else {
          currentScenario = filteredPlaylist[index]
          shouldPlayAfterSetup = true
          shouldReset = true
          if (index === startPlaylistScenarioIndex) {
            commit('updateState', { times: times + 1 })
          }
        }
      }
    }
  } else if (mode === 'manual') {
    // pass
  }

  commit('updateState', { shouldPlayAfterSetup })

  if (shouldReset) {
    await dispatch('resetScenario', currentScenario)
  } else {
    await dispatch('system/getConfig', null, { root: true })
    const { autoReset } = rootState.system.simulation
    if (autoReset) {
      await dispatch('setup', currentScenario)
    }
  }
}

// electron.modules
export async function setup (context, scenario) {
  const { state, commit, dispatch } = context
  const { path: scenarioPath, id: scenarioId } = scenario
  setupReady = false
  resetModuleLogFileName(commit)
  resetModuleStatus(commit)
  resetModuleTimeCost(commit)
  resetGradingKPIList()
  resetPbMessage()
  resetCarDashboard()
  eventBus.$emit('v2x-warning-info', 'reset') // resetV2xWarningInfo
  resetProtoMessage()
  resetLogMessage()
  resetTimestamp(commit)
  commit('updateState', { loading: true })
  commit('updateState', { isPaused: false })
  await dispatch('module-set/getModuleSetList', null, { root: true })
  await dispatch('switchScenarioModuleSet', scenarioId)
  dispatch('pb-config/resetChosenPbConfigList', undefined, { root: true })
  electron.modules.setup(scenarioPath, async (status, err) => {
    if (state.loading === false) {
      // 其他请求可能会导致loading为false
      commit('updateState', { loading: true })
    }
    if (setupReady === false) {
      setupReady = true
    }
    if (err?.length) {
      await showErrorList(err)
    }
    const { retCode, initStatus, cmdStatus } = status
    const retMessage = dict.retCode[retCode] ? i18n.t(dict.retCode[retCode]) : retCode
    if (retCode === 0) {
      const { updatedModules = [] } = status
      if (updatedModules.length) {
        await showSuccessMessage(`${updatedModules.join(',')} ${i18n.t('tips.modulesUpdateAvailable')}`)
      }
      if (initStatus?.length) {
        setModuleLogFileName(initStatus)
        throttleModuleLogFileName(commit, setupReady)
        setModuleStatus(initStatus)
        throttleModuleStatus(commit, setupReady)
        electron.userLog.openFiles(Object.values(getModuleLogFileName()), (payload) => {
          setLogMessage(payload)
          throttleLogMessage()
        })
      }
    } else if (retCode === 1) {
      commit('updateState', { playingStatus: 'ready' })
      commit('updateState', { loading: false })
      if (cmdStatus?.length) {
        setModuleTimeCost(cmdStatus)
        throttleModuleTimeCost(commit, setupReady)
      }
      if (state.shouldPlayAfterSetup) {
        play(context)
      }
      // 初始化完成，请求事件列表
      dispatch('log2world/getLog2WorldConfig', { scenePath: scenarioPath }, { root: true })
      dispatch('pb-config/refreshPbConfigList', undefined, { root: true })
    } else if (retCode === 3) {
      console.log(retMessage)
      await showErrorMessage(retMessage)
      commit('updateState', { playingStatus: 'uninitialized' })
      commit('updateState', { loading: false })
    } else {
      if (state.playingMode === 'list' && state.shouldPlayAfterSetup) {
        commit('updateState', { shouldPlayAfterSetup: false })
      }
      commit('updateState', { playingStatus: 'uninitialized' })
      commit('updateState', { loading: false })
      if (retCode === 13) {
        await showSuccessMessage(retMessage)
      } else {
        await showErrorMessage(retMessage)
      }
    }
  })
}

export function unSetup () {
  electron.modules.unSetup(async (status, err) => {
    if (err?.length) {
      await showErrorList(err)
    }
    const { retCode } = status
    const retMessage = i18n.t(dict.retCode[retCode]) || retCode
    if (retCode === 1) {
      // '执行成功'
    } else if (retCode === 2) {
      // '场景成功结束'
    } else if (retCode === 3) {
      console.log(retMessage)
    } else {
      await showErrorMessage(retMessage)
    }
    console.log(status)
  })
}

export async function switchScenarioModuleSet (context, scenarioId) {
  const {
    dispatch,
    rootState: {
      planners: {
        plannerList,
      },
      'module-set': {
        scheme: {
          schemes,
        },
      },
    },
  } = context
  const data = await getScenario(scenarioId)
  const {
    sim: {
      planner: {
        egolist,
      },
    },
  } = data
  const ids = egolist.map((planner) => {
    const ego = plannerList.find(p => p.variable === planner.name)
    if (ego) {
      let controlName = ego.catalogParams[0].properties.controller
      if (!controlName) {
        controlName = 'L4_Planning'
      }
      const moduleSet = schemes.find(s => s.name === controlName)
      if (!moduleSet) {
        console.error('未找到模组')
        return undefined
      }
      // todo:
      console.log('dispatch', 'sim-modules/resetModules')
      return {
        group_name: planner.group,
        scheme_id: moduleSet.id,
        group_type: 1,
      }
      // if (!moduleSet) {
      //   console.error('未找到模组或控制器配置为None')
      //   await dispatch('module-set/activeModuleSet', 99999, { root: true })
      // } else {
      //   await dispatch('module-set/activeModuleSet', moduleSet.id, { root: true })
      // }
    } else {
      console.error('未找到主车配置')
    }
    return undefined
  }).filter(Boolean)
  ids.push({
    group_name: 'Scene',
    scheme_id: schemes.find(s => s.name === 'TADSIM_GLOBAL').id,
    group_type: 0,
  })
  await dispatch('module-set/activeModuleSet', ids, { root: true })
}

// electron.player
export function play (context) {
  const { state, commit } = context
  let frameNumber = 0
  commit('updateState', { loading: true })
  commit('updateState', { isPaused: false })
  electron.player.play(async (status, err) => {
    // console.log('status', cloneDeep(status))
    if (err?.length) {
      await showErrorList(err)
    }
    const { retCode, cmdStatus } = status
    const retMessage = i18n.t(dict.retCode[retCode]) || retCode
    frameNumber++
    if (retCode === 0) { // callback may be invoked after pause or stop even setup for many times
      if (frameNumber === 1) {
        commit('updateState', { playingStatus: 'playing' })
        commit('updateState', { loading: false }) // play loading
      }
      renderFrame(status, context)
    } else if (retCode === 1) {
      commit('updateState', { loading: false }) // pause loading
    } else if (retCode === 2) {
      renderFrame(status, context)
      player.scenario.playerLastFrame()
      commit('updateState', { playingStatus: 'uninitialized' })
      commit('updateState', { loading: false }) // play loading
      await showPlayCompletedMessage(context, cmdStatus)
      await afterPlayFinished(context, 'auto')
    } else if (retCode === 3) {
      console.log(retMessage)
      renderFrame(status, context)
    } else {
      renderFrame(status, context)
      commit('updateState', { playingStatus: 'uninitialized' })
      commit('updateState', { loading: false }) // play loading
      if (retCode === 8) {
        player.scenario.playerLastFrame()
        await showSuccessMessage(retMessage)
        await afterPlayFinished(context, 'auto')
      } else {
        if (state.playingMode === 'list' && state.shouldPlayAfterSetup) {
          commit('updateState', { shouldPlayAfterSetup: false })
        }
        await showErrorMessage(retMessage)
      }
    }
  })
}

export function step (context) {
  const { commit } = context
  commit('updateState', { loading: true })
  electron.player.step(async (status, err) => {
    console.log('status', cloneDeep(status))
    if (err?.length) {
      await showErrorList(err)
    }
    const { retCode, cmdStatus } = status
    const retMessage = i18n.t(dict.retCode[retCode]) || retCode
    if (retCode === 1) {
      // pass
    } else if (retCode === 2) {
      commit('updateState', { playingStatus: 'uninitialized' })
      await showPlayCompletedMessage(context, cmdStatus)
      // await afterPlayFinished(context, 'manual')
    } else if (retCode === 3) {
      console.log(retMessage)
    } else {
      commit('updateState', { playingStatus: 'uninitialized' })
      await showErrorMessage(retMessage)
    }
    commit('updateState', { loading: false })
    renderFrame(status, context)
  })
}

export function pause (context, stop = false) {
  const { commit, dispatch } = context
  commit('updateState', { loading: true })
  electron.player.pause(async (status) => {
    const { retCode } = status
    const retMessage = i18n.t(dict.retCode[retCode]) || retCode
    if (retCode === 1) { // when play callback retCode === 1, set loading = false
      commit('updateState', { playingStatus: 'ready' })
      commit('updateState', { isPaused: true })
    } else if (retCode === 3) {
      console.log(retMessage)
    } else {
      commit('updateState', { playingStatus: 'uninitialized' })
      commit('updateState', { loading: false })
      if (!stop) {
        await showErrorMessage(retMessage)
      }
    }
    if (stop) {
      dispatch('stop')
    }
  })
}

export function stop (context, mode = 'manual') {
  const { commit } = context
  commit('updateState', { loading: true })
  electron.player.stop(async (status, err) => {
    if (err?.length) {
      await showErrorList(err)
    }
    const { retCode } = status
    const retMessage = i18n.t(dict.retCode[retCode]) || retCode
    if (retCode === 3) {
      console.log(retMessage)
    } else if (retCode !== 2) {
      await showErrorMessage(retMessage)
    }
    // todo feedback
    commit('updateState', { playingStatus: 'uninitialized' })
    commit('updateState', { loading: false })
    console.log('stop', status)
    await afterPlayFinished(context, mode)
  })
}

export function updateOperationMode ({ commit, state }, mode) {
  if (state.operationMode === mode) {
    return
  }
  // if (mode === 3) { // 旋转时切换到自由视角
  //   dispatch('switchPerspective', 'debugging')
  // }
  commit('updateOperationMode', mode)
  editor.getSignals('operationMode').dispatch(mode)
}

export function resetCameraState ({ commit, dispatch }) {
  commit('stage/updateState', { degree: 0 }, { root: true })
  dispatch('switchPerspective', 'top')
  dispatch('updateOperationMode', 1)
}
