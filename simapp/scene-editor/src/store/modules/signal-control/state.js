export default {
  // 信控配置数据，来自 sceneParse
  signlightPlans: {
    activePlan: '0',
    signlights: [],
    junctions: [],
    routesData: [],
  },
  currentPlanId: undefined,
  // 当前路口控制器
  currentJunctionId: undefined,
  // 当前 phase
  currentPhaseId: undefined,
  // 当前灯态
  currentLightStateId: undefined,
  // 临时灯，用于新增临时的 phase 相位
  tmpSignlightList: [],
  // 弹窗内编辑的信控与场景属性里的信控，不好联动，故而用这个临时变量
  signlightPlanUpdateFlag: 0,
  // 信控配置被修改的标志
  isDirty: false,
}
