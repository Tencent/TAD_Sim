import _ from 'lodash'
import { ElMessage } from 'element-plus'
import Signlight from 'sim-player/sceneeditor/Signlight'
import { editor } from '@/api/interface'
import {
  formatArrayToColors,
  formatColorsToArray,
  getSignlightColorData,
  hasColor,
} from '@/components/signal-control/common/utils'
import { messageBoxInfo } from '@/plugins/message-box'

// 获取列表中最大的 id 再 +1
function getMaxId (signlights, key) {
  const group = _.groupBy(signlights, key)
  const ids = Object.keys(group)
  const maxId = ids.reduce((res, id) => Math.max(id ? Number.parseInt(id, 10) : 0, res), 0)
  return `${maxId + 1}`
}

/**
 * 初始化信控配置项
 * @param state
 * @param payload
 */
export function initSignlightPlans ({ state }, payload) {
  state.signlightPlans = _.cloneDeep(payload)
  state.currentPlanId = undefined
  state.currentJunctionId = undefined
  state.currentPhaseId = undefined
  state.currentLightStateId = undefined
  state.tmpSignlightList = []
  state.isDirty = false
}

export function refreshSignlightPlans ({ state }) {
  // 偶现响应时组件尚未加载，需延迟一点再通知其获取数据
  setTimeout(() => {
    state.signlightPlanUpdateFlag = Math.random()
  }, 100)
}

/**
 * 重置信控配置项
 * @param state
 * @param dispatch
 */
export function resetSignalControl ({ state, dispatch }) {
  const signlightPlans = editor.scenario.getSignlightPlans()
  dispatch('initSignlightPlans', signlightPlans)
  state.currentPlanId = signlightPlans.activePlan || '0'
  dispatch('resetJunction')
}

/**
 * 选择信控配置项
 * @param state
 * @param dispatch
 * @param planId
 */
export function selectPlan ({ state, dispatch }, planId) {
  state.currentPlanId = planId

  dispatch('resetJunction')
}

/**
 * 克隆信控配置项
 * @param state
 * @param getters
 * @param dispatch
 * @param plan
 */
export function clonePlan ({ state, getters, dispatch }, plan) {
  const allSignlights = getters.allSignlights.filter(e => !e.isTemp)
  const oldPlanSignlights = getters.planList.find(e => e.id === plan.id).signlights.filter(e => !e.isTemp)
  const newPlanId = getMaxId(allSignlights, 'plan')
  let newSignlightId = getMaxId(allSignlights, 'id')
  // let newPhaseId = getMaxId(allSignlights, 'phaseNumber')
  const tmp = _.groupBy(oldPlanSignlights, 'phaseNumber')

  // 克隆该信控配置下的所有信号灯
  const newPlanSignlights = []
  Object.keys(tmp).forEach((phaseNumber) => {
    const signlights = tmp[phaseNumber]
    // const phaseId = (newPhaseId++) + ''
    signlights.forEach((signlight) => {
      const id = `${newSignlightId++}`
      // const templateSignlight = { ...signlight, id, phaseNumber: phaseId, plan: newPlanId }
      const templateSignlight = { ...signlight, id, plan: newPlanId }
      const newSignlight = new Signlight()
      newSignlight.copyNoModel(templateSignlight)
      newPlanSignlights.push(newSignlight)
    })
  })
  const signlights = allSignlights.concat(newPlanSignlights)

  // 临时相位也要克隆
  const newTmpSignlightList = []
  state.tmpSignlightList.forEach((signlight) => {
    const id = `${++newSignlightId}_tmp`
    const templateSignlight = { ...signlight, id, plan: newPlanId }
    const newSignlight = new Signlight()
    newSignlight.copyNoModel(templateSignlight)
    newSignlight.isTemp = true
    newTmpSignlightList.push(newSignlight)
  })
  state.tmpSignlightList = state.tmpSignlightList.concat(newTmpSignlightList)

  // 刷新视图
  dispatch('updateSignlightPlans', { signlights })

  state.isDirty = true
}

/**
 * 删除信控配置项
 * @param state
 * @param dispatch
 * @param plan
 */
export function deletePlan ({ state, dispatch }, plan) {
  const oldSignlightPlans = state.signlightPlans
  const oldActivePlan = state.currentPlanId
  const { signlights: oldSignlights } = oldSignlightPlans
  // 排除掉绑定了该 plan 的所有信号灯
  const signlights = oldSignlights.filter(signlight => signlight.plan !== plan.id)
  // 临时相位也要删除
  state.tmpSignlightList = state.tmpSignlightList.filter(signlight => signlight.plan !== plan.id)
  // 若删除的是当前激活的 plan，则激活默认 plan
  const deleteCurrent = oldActivePlan === plan.id
  const activePlan = deleteCurrent ? '0' : oldActivePlan

  // 刷新视图
  dispatch('updateSignlightPlans', { activePlan, signlights })

  // 重置路口控制器
  dispatch('resetJunction')

  // 若删除的是当前激活的 plan，外层也要相应更改
  if (deleteCurrent) {
    editor.scenario.setSignlightPlans({ activePlan })
    state.signlightPlanUpdateFlag = Math.random()
  }

  state.isDirty = true
}

/**
 * 初始化路口控制器
 * @param state
 * @param getters
 * @return {default.watch.planJunctionTree}
 */
export function resetJunction ({ state, getters }) {
  const tree = getters.planJunctionTree
  if (tree[0]) {
    state.currentJunctionId = tree[0].id
    state.currentPhaseId = tree[0].phases[0]?.id
  }
  return tree
}

/**
 * 选择路口控制器
 * @param state
 * @param junctionId
 */
export function selectJunction ({ state }, junctionId) {
  state.currentJunctionId = junctionId
}

/**
 * 选择 phase
 * @param state
 * @param phaseId
 */
export function selectPhase ({ state }, phaseId) {
  if (state.currentPhaseId !== phaseId) {
    state.currentLightStateId = undefined
  }
  state.currentPhaseId = phaseId
}

/**
 * 获取灯态
 * @param state
 * @param lightStateId
 */
export function selectLightState ({ state }, lightStateId) {
  state.currentLightStateId = lightStateId
}

/**
 * 路口控制器 junction 数据变动
 * @param state
 * @param getters
 * @param dispatch
 * @param payload
 */
export function changeJunctionValue ({ state, getters, dispatch }, payload) {
  const { id, key, value } = payload

  // 修改 signlights 数据
  const allSignlights = getters.allSignlights
  const oldSignlights = getters.currentJunction.signlights.filter(e => e.junction === id)
  const ids = oldSignlights.map(signlight => signlight.id)
  let signlights = allSignlights
  switch (key) {
    case 'crossingStatus': {
      signlights = allSignlights.map((signlight) => {
        if (ids.includes(signlight.id)) {
          const newSignlight = signlight
          Object.assign(newSignlight, {
            enabled: value,
          })
          return newSignlight
        }
        return signlight
      })
      break
    }
    case 'cycleTime': {
      const offset = value - getters.currentJunction.cycleTime
      getters.planJunctionPhaseList.forEach((phase) => {
      // 该路口下所有相位的灯态时长需变化
        let colors = phase.colors.slice(0)
        if (offset > 0) {
        // 如果总时长增加，则最后的灯态拉长
          const last = colors[colors.length - 1]
          if (last) {
            last.value += offset
          }
        } else {
        // 如果总时长减少，则保留前半部分
          let count = 0
          colors = colors.reduce((res, item) => {
            let current = item.value
            if (count < value) {
              if (count + current > value) {
                current = value - count
              }
              res.push({ ...item, value: current })
              count += item.value
            }
            return res
          }, [])
        }
        const tmp = formatArrayToColors(colors)
        Object.keys(tmp).forEach(key => (tmp[key] = String(tmp[key]))) // 所有数据转为字符串
        const { startTime, green: timeGreen, yellow: timeYellow, red: timeRed } = tmp
        const ids = phase.signlights.map(e => e.id)
        signlights = allSignlights.map((signlight) => {
          if (ids.includes(signlight.id)) {
            const newSignlight = signlight
            Object.assign(newSignlight, {
              startTime,
              timeGreen,
              timeYellow,
              timeRed,
            })
            return newSignlight
          }
          return signlight
        })
      })
      break
    }
    case 'disabled': {
      signlights = allSignlights.map((signlight) => {
        if (ids.includes(signlight.id)) {
          const newSignlight = signlight
          Object.assign(newSignlight, {
            enabled: false,
          })
          return newSignlight
        }
        return signlight
      })
      break
    }
    case 'enabled': {
      signlights = allSignlights.map((signlight) => {
        if (ids.includes(signlight.id)) {
          const newSignlight = signlight
          Object.assign(newSignlight, {
            enabled: true,
          })
          return newSignlight
        }
        return signlight
      })
      break
    }
    case 'default': {
      signlights = allSignlights.map((signlight) => {
        if (ids.includes(signlight.id)) {
          const newSignlight = signlight
          Object.assign(newSignlight, {
            enabled: true,
          })
          return newSignlight
        }
        return signlight
      })
      break
    }
    case 'allGreen': {
      signlights = allSignlights.map((signlight) => {
        if (ids.includes(signlight.id)) {
          const newSignlight = signlight
          Object.assign(newSignlight, {
            enabled: true,
            startTime: '1',
            timeGreen: getters.currentJunction.cycleTime,
            timeYellow: '0',
            timeRed: '0',
          })
          return newSignlight
        }
        return signlight
      })
      break
    }
    case 'allRed': {
      signlights = allSignlights.map((signlight) => {
        if (ids.includes(signlight.id)) {
          const newSignlight = signlight
          Object.assign(newSignlight, {
            enabled: true,
            startTime: '1',
            timeGreen: '0',
            timeYellow: '0',
            timeRed: getters.currentJunction.cycleTime,
          })
          return newSignlight
        }
        return signlight
      })
      break
    }
    default:
  }

  // 刷新视图
  dispatch('updateSignlightPlans', { signlights })

  state.isDirty = true
}

/**
 * 新增相位
 * @param state
 * @param getters
 */
export function addPhase ({ state, getters }) {
  const allSignlights = getters.allSignlights
  const junctionSignlights = getters.currentJunction.signlights
  const newSignlightId = getMaxId(allSignlights, 'id')
  const newPhaseId = getMaxId(junctionSignlights, 'phaseNumber')

  const colorData = {
    startTime: 1,
    timeGreen: 3,
    timeYellow: 2,
    timeRed: 0,
  }
  if (junctionSignlights?.length > 0) { // 默认添加 绿3黄2红剩，不足则向前扣除
    const hasColorItem = junctionSignlights.find(signlight => hasColor(signlight))
    const otherColorData = getSignlightColorData(hasColorItem)
    const otherTotalTime = otherColorData.green + otherColorData.yellow + otherColorData.red
    if (otherTotalTime > 5) colorData.timeRed = otherTotalTime - 5
    else if (otherTotalTime > 3) colorData.timeYellow = otherTotalTime - 3
    else if (otherTotalTime > 0) {
      colorData.timeYellow = 0
      colorData.timeGreen = otherTotalTime
    }
  }
  Object.keys(colorData).forEach((key) => {
    colorData[key] = String(colorData[key])
  })

  const templateSignlight = {
    ...junctionSignlights[0],
    id: `${newSignlightId}_tmp`,
    enabled: getters.currentJunction.crossingStatus,
    ...colorData,
    phaseNumber: newPhaseId,
    signalHead: [],
  }
  const newSignlight = new Signlight()
  newSignlight.copyNoModel(templateSignlight)
  newSignlight.isTemp = true // 标记为临时灯，会转为临时相位（重要）
  state.tmpSignlightList.push(newSignlight)

  state.isDirty = true
}

/**
 * 删除相位
 * @param state
 * @param getters
 * @param dispatch
 */
export function deletePhase ({ state, getters, dispatch }) {
  if (state.currentPhaseId === undefined) {
    ElMessage.warning('请先选择一个 phase')
    return
  }

  if (
    getters.currentPhase.colors.length > 0 &&
    getters.currentJunction.phases.filter(e => e.colors.length > 0).length <= 1
  ) {
    ElMessage.warning('该路口控制器下至少需要一个有灯态的 phase')
    return
  }

  const allSignlights = getters.allSignlights
  const phaseSignlights = getters.currentPhase.signlights
  const isTemp = getters.currentPhase.isTemp
  const ids = phaseSignlights.map(signlight => signlight.id)
  // 删除相位，实则清除该灯与相位的绑定关系
  const signlights = allSignlights.map((signlight) => {
    if (ids.includes(signlight.id)) {
      const newSignlight = signlight
      Object.assign(newSignlight, {
        enabled: false,
        phaseNumber: '',
        startTime: '1',
        timeGreen: '0',
        timeYellow: '0',
        timeRed: '0',
        signalHead: [],
      })
      return newSignlight
    }
    return signlight
  })

  // 删的临时相位的话，临时灯也可去掉了
  if (isTemp) {
    state.tmpSignlightList = state.tmpSignlightList.filter(e => !ids.includes(e.id))
  }

  // 刷新视图
  dispatch('updateSignlightPlans', { signlights })

  // 清除相位的选中态
  state.currentPhaseId = undefined

  state.isDirty = true
}

/**
 * 相位 phase 数据变动，主要是 colors 和 signalHead
 * @param state
 * @param getters
 * @param dispatch
 * @param payload
 */
export function changePhaseValue ({ state, getters, dispatch }, payload) {
  const { id: phaseId, key, value } = payload
  const phase = getters.planJunctionPhaseList.find(e => e.id === phaseId)

  // 修改 signlights 数据
  const allSignlights = getters.allSignlights
  const oldSignlights = phase.signlights
  const ids = oldSignlights.map(signlight => signlight.id)
  const signlights = allSignlights
  if (key === 'colors') {
    if (value.length === 0) {
      if (
        getters.currentPhase.colors.length > 0 &&
        getters.currentJunction.phases.filter(e => e.colors.length > 0).length <= 1
      ) {
        ElMessage.warning('该路口控制器下至少需要一个有灯态的 phase')
        return
      }
    }
    const tmp = formatArrayToColors(value)
    Object.keys(tmp).forEach(key => (tmp[key] = String(tmp[key]))) // 所有数据转为字符串
    const { startTime, green: timeGreen, yellow: timeYellow, red: timeRed } = tmp
    allSignlights.map((signlight) => {
      if (ids.includes(signlight.id)) {
        const newSignlight = signlight
        Object.assign(newSignlight, {
          startTime,
          timeGreen,
          timeYellow,
          timeRed,
        })
        return newSignlight
      }
      return signlight
    })
  } else if (key === 'signalHead') {
    allSignlights.map((signlight) => {
      if (ids.includes(signlight.id)) {
        // 当前相位的物理灯，直接修改
        const newSignlight = signlight
        Object.assign(newSignlight, {
          [key]: value,
        })
        return newSignlight
      } else if (signlight.plan === getters.currentPlanId) {
        // 其他相位，若也勾选了这些物理灯，需剔除
        const newSignlight = signlight
        const newSignalHead = _.difference(signlight.signalHead, value)
        Object.assign(newSignlight, {
          [key]: newSignalHead,
        })
        return newSignlight
      }
      return signlight
    })
  } else {
    allSignlights.map((signlight) => {
      if (ids.includes(signlight.id)) {
        const newSignlight = signlight
        Object.assign(newSignlight, {
          [key]: value,
        })
        return newSignlight
      }
      return signlight
    })
  }

  // 刷新视图
  dispatch('updateSignlightPlans', { signlights })

  state.isDirty = true
}

/**
 * 合并相邻且灯色相同的灯态
 * @param colors
 * @return {*}
 */
function colorsRemoveSame (colors) {
  return colors.reduce((res, e) => {
    const prev = res[res.length - 1]
    if (prev && prev.color === e.color) {
      prev.value += e.value
      return res
    }
    return res.concat(e)
  }, [])
}

/**
 * 新增灯态
 * @param state
 * @param getters
 * @param dispatch
 * @param color
 */
export function addLightState ({ state, getters, dispatch }, color) {
  if (state.currentPhaseId === undefined) {
    ElMessage.warning('请先选择一个 phase')
    return
  }

  const colors = getters.currentPhase.colors.slice(0)

  if (colors.length === 1 && colors[0].color === color) return // 灯态为纯色且与新增颜色一致，无变化

  const getTimeKey = (key) => {
    if (key === 'startTime') return 'startTime'
    if (key === 'green') return 'timeGreen'
    if (key === 'yellow') return 'timeYellow'
    if (key === 'red') return 'timeRed'
  }
  const colorData = {
    startTime: '1',
    timeGreen: '0',
    timeYellow: '0',
    timeRed: '0',
  }
  if (colors.length < 1) {
    // 若全灰，直接填满该颜色
    const restTime = `${getters.currentJunction.cycleTime}`
    const key = getTimeKey(color)
    colorData[key] = restTime
  } else {
    // 若已有颜色，则从其他颜色中扣除 5s（不足 5s 则扣到 1s 为止）, 并填充该颜色
    const oldColorData = formatArrayToColors(colors)
    const total = oldColorData.green + oldColorData.yellow + oldColorData.red
    const newColorData = { ...oldColorData }
    const tmpColors = formatColorsToArray({ startTime: 1, ..._.omit(oldColorData, ['startTime']) })
    const maxColor = _.maxBy(tmpColors.filter(e => e.color !== color), 'value')
    if (maxColor) {
      const needReduceValue = maxColor.value > 5 ? 5 : (maxColor.value - 1)
      newColorData[maxColor.color] -= needReduceValue
      newColorData[color] += needReduceValue
      if (newColorData.startTime !== 1 && color !== 'red') {
        const newStartTime = newColorData.startTime + needReduceValue
        newColorData.startTime = newStartTime > total ? 1 : newStartTime
      }
    }
    Object.keys(newColorData).forEach((simpleKey) => {
      const key = getTimeKey(simpleKey)
      colorData[key] = String(newColorData[simpleKey])
    })
  }

  // 修改 signlights 数据
  const allSignlights = getters.allSignlights
  const oldSignlights = getters.currentPhase.signlights
  const ids = oldSignlights.map(signlight => signlight.id)
  const { startTime, timeGreen, timeYellow, timeRed } = colorData
  const signlights = allSignlights.map((signlight) => {
    if (ids.includes(signlight.id)) {
      const newSignlight = signlight
      Object.assign(newSignlight, {
        startTime,
        timeGreen,
        timeYellow,
        timeRed,
      })
      return newSignlight
    }
    return signlight
  })

  // 刷新视图
  dispatch('updateSignlightPlans', { signlights })

  state.isDirty = true
}

/**
 * 删除灯态
 * @param getters
 * @param state
 * @param dispatch
 */
export function deleteLightState ({ getters, state, dispatch }) {
  if (state.currentPhaseId === undefined) {
    ElMessage.warning('请先选择一个 phase')
    return
  }
  if (state.currentLightStateId === undefined) {
    ElMessage.warning('请先选择一个灯态')
    return
  }

  let colors = getters.currentPhase.colors.slice(0)
  if (colors.length <= 1) {
    // 若其为最后的灯，则全灰
    if (
      getters.currentPhase.colors.length > 0 &&
      getters.currentJunction.phases.filter(e => e.colors.length > 0).length <= 1
    ) {
      ElMessage.warning('该路口控制器下至少需要一个有灯态的 phase')
      return
    }
    colors = []
  } else {
    // 将该灯删除，其时长分配给其他颜色中最长且靠前的
    const currentLight = colors[state.currentLightStateId]
    const color = currentLight.color
    const maxColor = _.maxBy(colors.filter(e => e.color !== color), 'value')
    const oldTotal = colors.reduce((res, e) => res + e.value, 0)
    colors = colors.filter(e => e.color !== color)
    const newTotal = colors.reduce((res, e) => res + e.value, 0)
    const needReduceValue = oldTotal - newTotal
    if (maxColor) {
      maxColor.value += needReduceValue
    }
  }
  colors = colorsRemoveSame(colors)

  // 修改 signlights 数据
  const allSignlights = getters.allSignlights
  const oldSignlights = getters.currentPhase.signlights
  const ids = oldSignlights.map(signlight => signlight.id)
  const tmp = formatArrayToColors(colors)
  Object.keys(tmp).forEach(key => (tmp[key] = String(tmp[key]))) // 所有数据转为字符串
  const { startTime, green: timeGreen, yellow: timeYellow, red: timeRed } = tmp
  const signlights = allSignlights.map((signlight) => {
    if (ids.includes(signlight.id)) {
      const newSignlight = signlight
      Object.assign(newSignlight, {
        startTime,
        timeGreen,
        timeYellow,
        timeRed,
      })
      return newSignlight
    }
    return signlight
  })

  // 刷新视图
  dispatch('updateSignlightPlans', { signlights })

  // 若该索引无内容，则重置索引
  if (!colors[state.currentLightStateId]) {
    state.currentLightStateId = colors.length > 0 ? 0 : undefined
  }

  state.isDirty = true
}

/**
 * 相位与方向灯向绑定
 * @param getters
 * @param state
 * @param dispatch
 * @param payload
 */
export function bindSignlight ({ getters, state, dispatch }, payload) {
  if (state.currentPhaseId === undefined) {
    ElMessage.warning('请先选择一个 phase')
    return
  }

  const { id: signlightId } = payload
  const allSignlights = getters.allSignlights
  const triggerSignlight = allSignlights.find(e => e.id === signlightId)

  if (getters.currentPhase.junctionid !== triggerSignlight.junction) {
    ElMessage.warning(`路口 ${getters.currentPhase.junctionid} 的相位不可操作路口 ${triggerSignlight.junction} 的语义灯`)
    return
  }

  const unBind = triggerSignlight.phaseNumber === getters.currentPhase.phaseNumber // 想解绑，否则是绑定或换绑

  // 拼凑数据
  let newSignlightData
  if (unBind) {
    // 解除相位时，相位的数据需从方向灯中移除
    newSignlightData = {
      enabled: false,
      phaseNumber: '',
      signalHead: [],
    }
    // 另外，若解绑的相位为临时相位，则其相位数据不可丢
    if (getters.currentPhase.isTemp) {
      newSignlightData = undefined
    }
    // 另外，被解绑的相位，若全部解绑，需将此相位改为临时相位
    const needChangeToTmp = !getters.currentPhase.isTemp && getters.currentPhase.signlights.length === 1
    if (needChangeToTmp) {
      const newSignlightId = getMaxId(allSignlights, 'id')
      const thisLight = getters.currentPhase.signlights[0]
      const templateSignlight = {
        ...thisLight,
        id: `${newSignlightId}_tmp`,
        phaseNumber: thisLight.phaseNumber,
      }
      const newSignlight = new Signlight()
      newSignlight.copyNoModel(templateSignlight)
      newSignlight.isTemp = true // 标记为临时灯，转为临时相位（重要）
      state.tmpSignlightList.push(newSignlight)
    }
  } else {
    // 绑定相位时，相位的数据需覆盖到该方向灯上
    const newPhaseId = getters.currentPhase.phaseNumber
    const tmpSignlight = getters.currentPhase.signlights[0]
    newSignlightData = {
      ..._.pick(tmpSignlight, [
        'enabled',
        'startTime',
        'timeGreen',
        'timeYellow',
        'timeRed',
        'signalHead',
      ]),
      phaseNumber: newPhaseId,
    }
    // 另外，若被绑的相位为临时相位，则可将该临时相位删除了
    if (getters.currentPhase.isTemp) {
      const ids = getters.currentPhase.signlights.map(e => e.id)
      state.tmpSignlightList = state.tmpSignlightList.filter(e => !ids.includes(e.id))
    }
    // 另外，原灯所在的相位，若全部解绑，需将此相位改为临时相位
    const isSamePhase = e => (
      e.plan === triggerSignlight.plan &&
      e.junction === triggerSignlight.junction &&
      e.phaseNumber === triggerSignlight.phaseNumber
    )
    const triggerPhaseSignlights = getters.allSignlights.filter(isSamePhase)
    if (triggerPhaseSignlights.length === 1 && triggerPhaseSignlights[0].phaseNumber) {
      const newSignlightId = getMaxId(allSignlights, 'id')
      const thisLight = triggerPhaseSignlights[0]
      const templateSignlight = {
        ...thisLight,
        id: `${newSignlightId}_tmp`,
        phaseNumber: thisLight.phaseNumber,
      }
      const newSignlight = new Signlight()
      newSignlight.copyNoModel(templateSignlight)
      newSignlight.isTemp = true // 标记为临时灯，转为临时相位（重要）
      state.tmpSignlightList.push(newSignlight)
    }
  }

  // 修改 signlights 数据
  const ids = [signlightId]
  const signlights = allSignlights.map((signlight) => {
    if (ids.includes(signlight.id)) {
      const newSignlight = signlight
      Object.assign(newSignlight, {
        ...newSignlightData,
      })
      return newSignlight
    }
    return signlight
  })

  // 刷新视图
  dispatch('updateSignlightPlans', { signlights })

  state.isDirty = true
}

/**
 * 触发视图刷新
 * @param state
 * @param payload
 */
export function updateSignlightPlans ({ state }, payload) {
  const { activePlan: propActivePlan, signlights: propSignlights } = payload

  if (propActivePlan) {
    state.currentPlanId = propActivePlan
    state.signlightPlans.activePlan = propActivePlan
  }
  if (propSignlights) {
    const tmpIds = state.tmpSignlightList.map(e => e.id)
    const signlights = propSignlights.filter((e) => {
      if (tmpIds.includes(e.id)) return false
      if (e.id.slice(-3) === 'tmp') return false
      return true
    })
    state.signlightPlans.signlights = signlights
  }
}

/**
 * 校验信控配置，所有相位都必须关联语义灯
 * @param state
 * @param dispatch
 * @return {boolean}
 */
export function validateSignlightPlans ({ state, dispatch }) {
  const single = state.tmpSignlightList[0]
  if (single) {
    const { plan, junction, phaseNumber } = single
    dispatch('selectPlan', plan)
    dispatch('selectJunction', junction)
    messageBoxInfo(`phase ${phaseNumber} 未关联语义灯`)
    return false
  }
  return true
}

/**
 * 信控配置注入进 sceneParse，保存是保存配置但不切换，应用则还需切换
 * @param state
 * @param commit
 * @param dispatch
 * @return {Promise<void>}
 */
export async function saveSignlightPlans ({ state, commit, dispatch }) {
  const { signlights } = state.signlightPlans

  editor.scenario.setSignlightPlans({ signlights })

  // 通知外部，信控已刷新
  dispatch('refreshSignlightPlans')

  if (state.isDirty) {
    commit('scenario/updateState', { isDirty: true }, { root: true })
  }

  state.isDirty = false
}

/**
 * 当前信控配置 id 注入进 sceneParse
 * @param getters
 * @param state
 * @param commit
 * @param dispatch
 * @param planId
 */
export function applySignlightPlans ({ getters, state, commit, dispatch }, planId) {
  const { signlights } = state.signlightPlans

  if (getters.activePlanId !== planId) {
    state.isDirty = true
  }

  editor.scenario.setSignlightPlans({ activePlan: planId, signlights })

  // 刷新视图
  dispatch('updateSignlightPlans', { activePlan: planId, signlights })

  // 通知外部，信控已刷新
  dispatch('refreshSignlightPlans')

  if (state.isDirty) {
    commit('scenario/updateState', { isDirty: true }, { root: true })
  }

  state.isDirty = false
}
