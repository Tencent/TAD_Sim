/* 用来定义一些常量和枚举的文件 */
import MinAveragerTimeHeadway from '@/components/popups/kpi-form/min-averager-time-headway.vue'
import MinDistanceFromCar from '@/components/popups/kpi-form/min-distance-from-car.vue'
import MaxAccelerationV from '@/components/popups/kpi-form/max-acceleration-v.vue'
import MaxDecelerationV from '@/components/popups/kpi-form/max-deceleration-v.vue'
import MaxTurningAccelerate from '@/components/popups/kpi-form/max-turning-accelerate.vue'
import MaxSpeedV from '@/components/popups/kpi-form/max-speed-v.vue'
import Collision from '@/components/popups/kpi-form/collision.vue'
import CrossSolidLine from '@/components/popups/kpi-form/cross-solid-line.vue'
import RunTrafficLight from '@/components/popups/kpi-form/run-traffic-light.vue'
import EndPoint from '@/components/popups/kpi-form/end-point.vue'
import CustomReachEndRadius from '@/components/popups/kpi-form/custom-reach-end-radius.vue'
import MaxPosErrorH from '@/components/popups/kpi-form/max-pos-error-h.vue'
import MaxPosErrorHL from '@/components/popups/kpi-form/max-pos-error-h-l.vue'
import MaxPosErrorHR from '@/components/popups/kpi-form/max-pos-error-h-r.vue'
import MaxAveragePosErrorH from '@/components/popups/kpi-form/max-average-pos-error-h.vue'
import CustomMaxPosError from '@/components/popups/kpi-form/custom-max-pos-error.vue'
import TotalTime from '@/components/popups/kpi-form/total-time.vue'

// 根据请求中 retCode 返回对应的提示信息
const retCode = {
  '0': '',
  '1': 'tips.executionSucceeded',
  '2': 'tips.sceneEndedSuccessfully',
  '3': 'tips.commandRejected',
  '4': 'tips.executionFailed',
  '5': 'tips.systemError',
  '6': 'tips.illegalTopicReleases',
  '7': 'tips.sceneFileParsingError',
  '8': 'tips.sceneReachedMaxTime',
  '9': 'tips.sharedMemoryError',
  '11': 'tips.sameModuleConfExists',
  '13': 'tips.moduleConfUpdated',
  // todo: 翻译
  '14': 'Logsim场景没有找到回放模块',
  '-1': 'tips.clientConnectionTimeout',
  '-2': 'tips.clientInitParameterError',
}

// 根据 code 返回对应的提示信息
const moduleCode = {
  0: 'tips.Success',
  1: 'tips.reqStop',
  2: 'tips.execTimeout',
  3: 'tips.runError',
  4: 'tips.incomVer',
  5: 'tips.systemError',
  6: 'tips.reinitReq',
  7: 'tips.processExit',
}
// 主车算法类型
const moduleCategoryList = [
  { id: 110, name: 'module.Location' }, // 定位
  { id: 101, name: 'module.Perception' }, // 感知
  { id: 102, name: 'module.Prediction' }, // 预测
  { id: 103, name: 'module.Planning' }, // 规划
  { id: 104, name: 'module.Control' }, // 控制
  { id: 111, name: 'module.Driver' }, // 驾驶员模型
  { id: 112, name: 'module.Sensor' }, // 传感器
  { id: 105, name: 'module.VehicleDynamics' }, // 车辆动力学
  { id: 51, name: 'module.Evaluating' }, // 评测
  { id: 113, name: 'module.XIL' }, // XIL
  // { id: 0, name: 'module.Other' }, // 其他
  // { id: 106, name: 'module.Planner' },
]
// 主车控制器分类
const moduleSetCategoryList = [
  { id: 'L4', name: 'module.L4' },
  { id: 'ACC', name: 'module.ACC' },
  { id: 'CSC', name: 'module.CSC' },
  { id: 'ISLC', name: 'module.ISLC' },
  { id: 'ICA', name: 'module.ICA' },
  { id: 'NDA', name: 'module.NDA' },
  { id: 'HWNDA', name: 'module.HWNDA' },
  { id: 'CityNDA', name: 'module.CityNDA' },
  { id: 'AEB', name: 'module.AEB' },
  { id: 'FCW', name: 'module.FCW' },
  { id: 'LDW', name: 'module.LDW' },
  { id: 'LDP', name: 'module.LDP' },
  { id: 'ELKA', name: 'module.ELKA' },
  { id: 'TSR', name: 'module.TSR' },
  { id: 'IHBC', name: 'module.IHBC' },
  { id: 'BSD', name: 'module.BSD' },
  { id: 'DOW', name: 'module.DOW' },
  { id: 'RCTA', name: 'module.RCTA' },
  { id: 'RAW', name: 'module.RAW' },
  { id: 'FCTA', name: 'module.FCTA' },
  { id: 'FAPA', name: 'module.FAPA' },
  { id: 'RPA', name: 'module.RPA' },
  { id: 'HPA', name: 'module.HPA' },
  { id: 'RA', name: 'module.RA' },
  { id: 'RAEB', name: 'module.RAEB' },
  { id: 'AVP', name: 'module.AVP' },
]
// 全局模块类型
const moduleGlobalCategoryList = [
  { id: 1, name: 'module.TruthValue' }, // 真值
  { id: 1001, name: 'module.Tool' }, // 工具
  { id: 0, name: 'module.Other' }, // 其他
]

// 转换 moduleGlobalCategoryList，变为一个对象，key 为 id，value 为 name
const moduleCategory = {}
moduleCategoryList.forEach((category) => {
  moduleCategory[category.id] = category.name
})

const kpiCategoryList = [
  { id: 'General', name: 'indicator.General' },
  { id: 'Perception', name: 'module.Perception' },
  { id: 'Prediction', name: 'module.Prediction' },
  { id: 'Planning', name: 'module.Planning' },
  { id: 'Control', name: 'module.Control' },
  { id: 'L4', name: 'indicator.L4' },
  { id: 'ACC', name: 'indicator.ACC' },
  { id: 'CSC', name: 'indicator.CSC' },
  { id: 'ISLC', name: 'indicator.ISLC' },
  { id: 'ICA', name: 'indicator.ICA' },
  { id: 'NDA', name: 'indicator.NDA' },
  { id: 'HWNDA', name: 'indicator.HWNDA' },
  { id: 'CityNDA', name: 'indicator.CityNDA' },
  { id: 'AEB', name: 'indicator.AEB' },
  { id: 'FCW', name: 'indicator.FCW' },
  { id: 'LDW', name: 'indicator.LDW' },
  { id: 'LDP', name: 'indicator.LDP' },
  { id: 'ELKA', name: 'indicator.ELKA' },
  { id: 'TSR', name: 'indicator.TSR' },
  { id: 'IHBC', name: 'indicator.IHBC' },
  { id: 'BSD', name: 'indicator.BSD' },
  { id: 'DOW', name: 'indicator.DOW' },
  { id: 'RCTA', name: 'indicator.RCTA' },
  { id: 'RAW', name: 'indicator.RAW' },
  { id: 'FCTA', name: 'indicator.FCTA' },
  { id: 'FAPA', name: 'indicator.FAPA' },
  { id: 'RPA', name: 'indicator.RPA' },
  { id: 'HPA', name: 'indicator.HPA' },
  { id: 'RA', name: 'indicator.RA' },
  { id: 'RAEB', name: 'indicator.RAEB' },
  { id: 'AVP', name: 'indicator.AVP' },
  { id: 'Other', name: 'indicator.Other' },
  // { id: 'Custom', name: 'indicator.Customize' },
  // { id: 'SECURITY', name: 'indicator.Safety' },
  // { id: 'COMFORTABILITY', name: 'indicator.Comfort' },
  // { id: 'COMPLIANCE', name: 'indicator.Compliance' },
  // { id: 'STATISTICS', name: 'indicator.Statistics' },
  // { id: 'EFFICIENCY', name: 'indicator.Efficiency' },
  // { id: 'ACCURACY', name: 'indicator.Accuracy' },
  // { id: 'Parking', name: 'indicator.Parking' },
]
const kpiLabelsList = [
  { id: 'ACCURACY', name: 'indicator.Accuracy' },
  { id: 'COMFORTABILITY', name: 'indicator.Comfort' },
  { id: 'COMPLIANCE', name: 'indicator.Compliance' },
  { id: 'EFFICIENCY', name: 'indicator.Efficiency' },
  { id: 'SECURITY', name: 'indicator.Safety' },
  { id: 'STATISTICS', name: 'indicator.Statistics' },
  { id: 'INTELLIGENCE', name: 'indicator.Intelligence' },
  { id: 'THEORY', name: 'indicator.Theory' },
  { id: 'TAKEOVER', name: 'indicator.TakeOver' },
  { id: 'OTHER', name: 'indicator.Customize' },
  // { id: 'Custom', name: 'indicator.Customize' },
  // { id: 'Parking', name: 'indicator.Parking' },
]

// 转换 kpiCategoryList，变为一个对象，key 为 id，value 为 name
const kpiCategory = {}
kpiCategoryList.forEach((category) => {
  kpiCategory[category.id] = category.name
})
const kpi = {
  MinAveragerTimeHeadway: {
    name: 'indicator.kpi0-name',
    definition: 'indicator.kpi0-def',
    calculation: 'indicator.kpi0-calc',
    component: MinAveragerTimeHeadway,
  },
  MinDistanceFromCar: {
    name: 'indicator.kpi1-name',
    definition: 'indicator.kpi1-def',
    calculation: 'indicator.kpi1-calc',
    component: MinDistanceFromCar,
  },
  MaxAcceleration_V: {
    name: 'indicator.kpi2-name',
    definition: 'indicator.kpi2-def',
    calculation: 'indicator.kpi2-calc',
    component: MaxAccelerationV,
  },
  MaxDeceleration_V: {
    name: 'indicator.kpi3-name',
    definition: 'indicator.kpi3-def',
    calculation: 'indicator.kpi3-calc',
    component: MaxDecelerationV,
  },
  MaxTurningAccelerate: {
    name: 'indicator.kpi4-name',
    definition: 'indicator.kpi4-def',
    calculation: 'indicator.kpi4-calc',
    component: MaxTurningAccelerate,
  },
  MaxSpeed_V: {
    name: 'indicator.kpi5-name',
    definition: 'indicator.kpi5-def',
    calculation: 'indicator.kpi5-calc',
    component: MaxSpeedV,
  },
  Collision: {
    name: 'indicator.kpi6-name',
    definition: 'indicator.kpi6-def',
    calculation: 'indicator.kpi6-calc',
    component: Collision,
  },
  CrossSolidLine: {
    name: 'indicator.kpi7-name',
    definition: 'indicator.kpi7-def',
    calculation: 'indicator.kpi7-calc',
    component: CrossSolidLine,
  },
  RunTrafficLight: {
    name: 'indicator.kpi8-name',
    definition: 'indicator.kpi8-def',
    calculation: 'indicator.kpi8-calc',
    component: RunTrafficLight,
  },
  EndPoint: {
    name: 'indicator.kpi9-name',
    definition: 'indicator.kpi9-def',
    calculation: 'indicator.kpi9-calc',
    component: EndPoint,
  },
  Custom_ReachEndRadius: {
    name: 'indicator.kpi10-name',
    definition: 'indicator.kpi10-def',
    calculation: 'indicator.kpi10-calc',
    component: CustomReachEndRadius,
  },
  MaxPosError_H: {
    name: 'indicator.kpi11-name',
    definition: 'indicator.kpi11-def',
    calculation: 'indicator.kpi11-calc',
    component: MaxPosErrorH,
  },
  MaxPosError_H_L: {
    name: 'indicator.kpi12-name',
    definition: 'indicator.kpi12-def',
    calculation: 'indicator.kpi12-calc',
    component: MaxPosErrorHL,
  },
  MaxPosError_H_R: {
    name: 'indicator.kpi13-name',
    definition: 'indicator.kpi13-def',
    calculation: 'indicator.kpi13-calc',
    component: MaxPosErrorHR,
  },
  MaxAveragePosError_H: {
    name: 'indicator.kpi14-name',
    definition: 'indicator.kpi14-def',
    calculation: 'indicator.kpi14-calc',
    component: MaxAveragePosErrorH,
  },
  Custom_MaxPosError: {
    name: 'indicator.kpi15-name',
    definition: 'indicator.kpi15-def',
    calculation: 'indicator.kpi15-calc',
    component: CustomMaxPosError,
  },
  TotalTime: {
    name: 'indicator.kpi16-name',
    definition: 'indicator.kpi16-def',
    calculation: 'indicator.kpi16-calc',
    component: TotalTime,
  },
}

const kpiResultMap = {
  UN_DEFINED: 'indicator.UN_DEFINED',
  PASS: 'indicator.PASS',
  SKIPPED: 'indicator.SKIPPED',
  FAIL: 'indicator.FAIL',
}

// 触发位置偏移选项
const triggerDirectionOptionList = [
  { id: 'static', name: 'none' },
  { id: 'left', name: 'scenario.moveToTheLeftLane' },
  { id: 'right', name: 'scenario.moveToTheRightLane' },
  { id: 'laneleft', name: 'scenario.moveToTheLeftOfTheLane' },
  { id: 'laneright', name: 'scenario.moveToTheRightOfTheLane' },
]

// 触发条件类型
const conditionTypeOptionList = [
  { id: 'timeRelative', name: 'TTC' },
  { id: 'distanceRelative', name: 'scenario.distanceToEgoVehicle' },
]

// 触发中的距离类型
const distanceModeOptionList = [
  { id: 'euclideandistance', name: 'scenario.euclideanDistance' },
  { id: 'laneprojection', name: 'scenario.roadCoordinateDistance' },
]

// 挡位列表
const gearList = [
  {
    id: 0,
    name: 'D',
  },
  {
    id: 1,
    name: 'P',
  },
  {
    id: 2,
    name: 'R',
  },
  {
    id: 3,
    name: 'N',
  },
  {
    id: 4,
    name: 'M1',
  },
  {
    id: 5,
    name: 'M2',
  },
  {
    id: 6,
    name: 'M3',
  },
  {
    id: 7,
    name: 'M4',
  },
  {
    id: 8,
    name: 'M5',
  },
  {
    id: 9,
    name: 'M6',
  },
  {
    id: 10,
    name: 'M7',
  },
]

// 播放消息的 topic 对照表，比如消息为 GRADING，实际 proto 中为 Grading
const MessageProtoMap = {
  Location: [
    'LOCATION',
    'LOCATION_REPLAY',
    'LOCATION_TRAILER',
    'LOCATION_TRAILER_REPLAY',
  ],
  Trajectory: [
    'TRAJECTORY',
    'TRAJECTORY_1',
    'TRAJECTORY_2',
    'TRAJECTORY_3',
    'TRAJECTORY_4',
    'TRAJECTORY_5',
    'TRAJECTORY_REPLAY',
  ],
  Traffic: [
    'TRAFFIC',
    'TRAFFIC_REPLAY',
  ],
  Grading: [
    'GRADING',
  ],
  Control: [
    'CONTROL',
  ],
  VehicleState: [
    'VEHICLE_STATE',
  ],
  TrajectoryFollow: [
    'TRAJECTORY_FOLLOW',
  ],
  V2xEarlyWarning: [
    'V2XEARLYWARNING',
  ],
  Control_V2: [
    'CONTROL_V2',
  ],
  DebugMessage: [
    'DebugMessage',
  ],
}
// 根据 pb 返回的 TOPIC ，返回对应名称， 如 GRADING TOPIC 找到 pb 配置中的 Grading
export function getProtoName (pbKeyframeTopic) {
  return Object.keys(MessageProtoMap).find(topic => MessageProtoMap[topic].includes(pbKeyframeTopic))
}

const dict = {
  retCode,
  moduleCode,
  moduleCategoryList,
  moduleSetCategoryList,
  moduleCategory,
  moduleGlobalCategoryList,
  kpiCategoryList,
  kpiLabelsList,
  kpiCategory,
  kpi,
  kpiResultMap,
  maxScenarioCount: 2000,
  triggerDirectionOptionList,
  conditionTypeOptionList,
  distanceModeOptionList,
  gearList,
  MessageProtoMap,
  getProtoName,
}

export default dict
