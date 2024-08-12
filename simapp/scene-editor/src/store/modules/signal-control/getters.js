import _ from 'lodash'
import {
  formatColorsToArray,
  getSignlightColorData,
} from '@/components/signal-control/common/utils'

// signlights 数据格式可见 https://doc.weixin.qq.com/doc/w3_AaEAnQbyAPYdoZyPFZYSmqE4sMtOy

export function allSignlights (state, getters, rootState, rootGetters) {
  const signlightPlans = state.signlightPlans
  const signlights = signlightPlans?.signlights || []
  const allSignlights = signlights.concat(state.tmpSignlightList)
  return allSignlights
}

// 按信控配置分组的语义灯
export function planSignlightsMap (state, getters, rootState, rootGetters) {
  const map = _.groupBy(getters.allSignlights, 'plan')
  return map
}

// 当前激活的信控配置 id
export function activePlanId (state, getters, rootState, rootGetters) {
  const signlightPlans = state.signlightPlans
  const { activePlan } = signlightPlans
  return activePlan
}

// 当前查看的信控配置 id
export function currentPlanId (state, getters, rootState, rootGetters) {
  return state.currentPlanId
}

// 是默认信控配置，将可不修改
export function isSystemPlan (state, getters, rootState, rootGetters) {
  return getters.currentPlanId === '0'
}

// 当前 信控配置下的 语义灯列表
export function currentPlanSignlights (state, getters, rootState, rootGetters) {
  const signlights = getters.planSignlightsMap[getters.currentPlanId]
  return signlights
}

// 信控配置列表
export function planList (state, getters, rootState, rootGetters) {
  const group = getters.planSignlightsMap
  const planList = Object.keys(group).map((id) => {
    const name = id === '0' ? '默认信控配置' : `信控配置 ${id}`
    return {
      id,
      name,
      signlights: group[id],
    }
  })
  return planList
}

// 当前信控配置
export function currentPlan (state, getters, rootState, rootGetters) {
  const current = getters.planList.find(e => e.id === getters.currentPlanId)
  return current || {}
}

// 当前信控配置下的路口列表
export function planJunctionTree (state, getters, rootState, rootGetters) {
  const signlights = getters.currentPlanSignlights
  const group = _.groupBy(signlights, 'junction')
  const junctions = Object.keys(group).map((id) => {
    const name = `junction controller ${id}`
    let cycleTime = 0 // 周期时长
    let crossingStatus = false // 启用状态
    const junctionSignlights = group[id]

    const tmp = _.groupBy(junctionSignlights, 'phaseNumber')
    delete tmp[''] // 未绑定相位的灯不显示在相位列表中
    const phases = Object.keys(tmp).map((phaseId) => {
      const signlights = tmp[phaseId]
      const common = signlights[0]
      const colorData = getSignlightColorData(common)
      const colors = formatColorsToArray(colorData)
      const total = colors.reduce((res, e) => res + e.value, 0)
      cycleTime = Math.max(cycleTime, total)
      crossingStatus = !crossingStatus ? common.enabled : crossingStatus
      return {
        id: `${id}+${phaseId}`,
        isTemp: common.isTemp,
        name: `phase${phaseId}`,
        enabled: common.enabled,
        colors,
        signalHead: common.signalHead,
        signlights: tmp[phaseId],
        junctionid: id,
        phaseNumber: phaseId,
      }
    })

    return {
      id,
      name,
      cycleTime,
      crossingStatus,
      phases,
      signlights: junctionSignlights,
    }
  })
  return junctions
}

// 当前信控配置下的路口控制器
export function currentJunction (state, getters, rootState, rootGetters) {
  const current = getters.planJunctionTree.find(e => e.id === state.currentJunctionId)
  return current || {}
}

// 当前信控配置下的路口控制器下的相位列表
export function planJunctionPhaseList (state, getters, rootState, rootGetters) {
  return getters.currentJunction?.phases || []
}

// 当前信控配置下的路口控制器下的相位
export function currentPhase (state, getters, rootState, rootGetters) {
  const current = getters.planJunctionPhaseList.find(e => e.id === state.currentPhaseId)
  return current || {}
}

// 当前 junction 路口控制器下，其 road 道路与 light 物理灯的 tree
export function roadLightTree (state, getters, rootState, rootGetters) {
  const signlightPlans = state.signlightPlans
  const { junctions = [] } = signlightPlans
  const currentJunctionData = junctions.find(e => e.id === state.currentJunctionId) // 当前路口下得所有物理灯
  const tafficlights = currentJunctionData?.tafficlights || []
  const group = _.groupBy(tafficlights, 'roadid') // 按道路分组
  const tree = Object.keys(group).map((id) => { // 道路 id
    const label = `road ${id}`
    const children = group[id].map((item) => {
      const id = item.id // 该道路下的物理灯 id
      const label = `light ${id}`
      return { id, label }
    })
    return { id: label, label, children }
  })
  return tree
}
