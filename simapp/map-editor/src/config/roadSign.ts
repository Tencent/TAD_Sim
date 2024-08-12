import i18n from '@/locales'
import { getPermission } from '@/utils/permission'
import { ImgUrls } from '@/utils/urls'

export interface IRoadSignConfig {
  mainType: string
  name: string
  showName: string
  iconUrl: string
  textureUrl: string
  modelUrl?: string
  imageRatio?: number
  freeMoveInRoad?: boolean
  basicSize: number
  type: string
  subtype: string
  placementAreaName: Array<string>
  adjustSize?: common.adjustSizeMode
}

export const indicatorRoadSign: Array<IRoadSignConfig> = [
  {
    mainType: 'roadSign',
    name: 'Arrow_Forward', // 前方直行
    showName: i18n.global.t('desc.editObject.goStraight'),
    iconUrl: ImgUrls.roadSign.straight,
    textureUrl: ImgUrls.roadSign.straight,
    basicSize: 0.8,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Left', // 前方左转
    showName: i18n.global.t('desc.editObject.leftTurn'),
    iconUrl: ImgUrls.roadSign.leftTurn,
    textureUrl: ImgUrls.roadSign.leftTurn,
    basicSize: 1.2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Right', // 前方右转
    showName: i18n.global.t('desc.editObject.rightTurn'),
    iconUrl: ImgUrls.roadSign.rightTurn,
    textureUrl: ImgUrls.roadSign.rightTurn,
    basicSize: 1.2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Left_And_Forward', // 前方可直行或左转
    showName: i18n.global.t('desc.editObject.straightOrLeftTurn'),
    iconUrl: ImgUrls.roadSign.straightOrLeftTurn,
    textureUrl: ImgUrls.roadSign.straightOrLeftTurn,
    basicSize: 1.8,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Right_And_Forward', // 前方可直行或右转
    showName: i18n.global.t('desc.editObject.straightOrRightTurn'),
    iconUrl: ImgUrls.roadSign.straightOrRightTurn,
    textureUrl: ImgUrls.roadSign.straightOrRightTurn,
    basicSize: 1.8,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_U_Turns', // 前方掉头
    showName: i18n.global.t('desc.editObject.uTurn'),
    iconUrl: ImgUrls.roadSign.uTurn,
    textureUrl: ImgUrls.roadSign.uTurn,
    basicSize: 1.45,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Forward_And_U_Turns', // 前方可直行或掉头
    showName: i18n.global.t('desc.editObject.straightOrUTurn'),
    iconUrl: ImgUrls.roadSign.straightOrUTurn,
    textureUrl: ImgUrls.roadSign.straightOrUTurn,
    basicSize: 1.6,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Left_And_U_Turns', // 前方可左转或掉头
    showName: i18n.global.t('desc.editObject.leftTurnOrUTurn'),
    iconUrl: ImgUrls.roadSign.leftTurnOrUTurn,
    textureUrl: ImgUrls.roadSign.leftTurnOrUTurn,
    basicSize: 1.45,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Left_And_Right', // 前方道路仅可左右转弯
    showName: i18n.global.t('desc.editObject.turnLeftOrRightOnly'),
    iconUrl: ImgUrls.roadSign.turnLeftOrRightOnly,
    textureUrl: ImgUrls.roadSign.turnLeftOrRightOnly,
    basicSize: 1.9,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Arrow_Turn_And_Straight', // 转弯直行
    showName: i18n.global.t('desc.editObject.turnOrStraight'),
    iconUrl: ImgUrls.roadSign.turnOrStraight,
    textureUrl: ImgUrls.roadSign.turnOrStraight,
    basicSize: 1.9,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Crosswalk_Warning_Line', // 人行横道预告线
    showName: i18n.global.t('desc.editObject.pedestrianCrosswalkWarningSigns'),
    iconUrl: ImgUrls.roadSign.pedestrianCrosswalkWarningSigns,
    textureUrl: ImgUrls.roadSign.pedestrianCrosswalkWarningSigns,
    basicSize: 1.8,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'White_Broken_Line_Vehicle_Distance_Confirmation', // 白色折线车距确认线
    showName: i18n.global.t(
      'desc.editObject.whiteBrokenLineVehicleDistanceConfirmation',
    ),
    iconUrl: ImgUrls.roadSign.whiteBrokenLineVehicleDistanceConfirmation,
    textureUrl: ImgUrls.roadSign.whiteBrokenLineVehicleDistanceConfirmation,
    basicSize: 1.8,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  // {
  //   mainType: 'roadSign',
  //   name: 'White_Semicircle_Line_Vehicle_Distance_Confirmation', // 白色半圆状车距确认线
  //   showName: i18n.global.t(
  //     'desc.editObject.whiteSemiCircleLineVehicleDistanceConfirmation'
  //   ),
  //   iconUrl: ImgUrls.roadSign.whiteSemicircleLineVehicleDistanceConfirmation,
  //   textureUrl: '',
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['lane'],
  //   adjustSize: 'l', // 只能控制长度
  // },
  {
    mainType: 'roadSign',
    name: 'Turn_Left_Waiting', // 左转弯待转区
    showName: i18n.global.t('desc.editObject.leftTurningVehicleWaitingArea'),
    iconUrl: ImgUrls.roadSign.leftTurningVehicleWaitingArea,
    textureUrl: ImgUrls.roadSign.leftTurningVehicleWaitingArea,
    basicSize: 3.3,
    type: 'none',
    subtype: '',
    placementAreaName: ['junction'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Turn_And_Merge_Left', // 前方道路有左弯或需向左合流
    showName: i18n.global.t('desc.editObject.leftBendAheadOrNeedsToMergeLeft'),
    iconUrl: ImgUrls.roadSign.leftBendAheadOrNeedsToMergeLeft,
    textureUrl: ImgUrls.roadSign.leftBendAheadOrNeedsToMergeLeft,
    basicSize: 1.2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Turn_And_Merge_Right', // 前方道路有右弯或需向右合流
    showName: i18n.global.t(
      'desc.editObject.rightBendAheadOrNeedsToMergeRight',
    ),
    iconUrl: ImgUrls.roadSign.rightBendAheadOrNeedsToMergeRight,
    textureUrl: ImgUrls.roadSign.rightBendAheadOrNeedsToMergeRight,
    basicSize: 1.2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Word_Mark_100_120', // 路面文字标识（100-120）
    showName: i18n.global.t('desc.editObject.pavementMarking'),
    iconUrl: ImgUrls.roadSign.pavementMarking100120,
    textureUrl: ImgUrls.roadSign.pavementMarking100120,
    basicSize: 2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Word_Mark_80_100', // 路面文字标识（80-100）
    showName: i18n.global.t('desc.editObject.pavementMarking'),
    iconUrl: ImgUrls.roadSign.pavementMarking80100,
    textureUrl: ImgUrls.roadSign.pavementMarking80100,
    basicSize: 2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Non_Motor_Vehicle', // 非机动车道
    showName: i18n.global.t('desc.editObject.nonMotorVehiclesOnlyMarkings'),
    iconUrl: ImgUrls.roadSign.bicycle,
    textureUrl: ImgUrls.roadSign.bicycle,
    basicSize: 2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Disabled_Parking_Space_Road_Mark', // 残疾人专用停车位
    showName: i18n.global.t('desc.editObject.disabledParkingSpots'),
    iconUrl: ImgUrls.roadSign.disabledParkingSpots,
    textureUrl: ImgUrls.roadSign.disabledParkingSpots,
    basicSize: 1.45,
    freeMoveInRoad: true,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
    adjustSize: 'ratio2',
  },
  // {
  //   mainType: 'roadSign',
  //   name: 'Crosswalk_Line', // 人行横道线
  //   showName: i18n.global.t('desc.editObject.crosswalk'),
  //   iconUrl: ImgUrls.roadSign.crosswalkLineIcon,
  //   textureUrl: '',
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   // 人行横道虽然可以放置在道路上，也需要支持拖拽放置在道路首尾延长线的区域
  //   placementAreaName: ['lane'],
  //   adjustSize: 'lw', // 长和宽单独控制，没有比例
  //   freeMoveInRoad: true,
  // },
  // {
  //   mainType: 'roadSign',
  //   name: 'Crosswalk_with_Left_and_Right_Side', // 行人左右分道的人行横道线
  //   showName: i18n.global.t('desc.editObject.crosswalkWithLeftAndRightSide'),
  //   iconUrl: ImgUrls.roadSign.crosswalkWithLeftAndRightSide,
  //   textureUrl: '',
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   // 人行横道虽然可以放置在道路上，也需要支持拖拽放置在道路首尾延长线的区域
  //   placementAreaName: ['lane'],
  //   adjustSize: 'lw', // 长和宽单独控制，没有比例
  //   freeMoveInRoad: true,
  // },
  // {
  //   mainType: 'roadSign',
  //   name: 'Road_Guide_Lane_Line', // 导向车道线
  //   showName: i18n.global.t('desc.editObject.guideLaneLine'),
  //   iconUrl: ImgUrls.roadSign.guideLaneLine,
  //   textureUrl: '',
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['lane'],
  //   adjustSize: 'l', // 只能控制长度
  // },
  // {
  //   mainType: 'roadSign',
  //   name: 'Variable_Direction_Lane_Line', // 可变导向车道线
  //   showName: i18n.global.t('desc.editObject.variableDirectionLaneLine'),
  //   iconUrl: ImgUrls.roadSign.variableDirectionLaneLine,
  //   textureUrl: '',
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['lane'],
  //   adjustSize: 'l', // 只能控制长度
  // },
  // {
  //   mainType: 'roadSign',
  //   name: 'Intersection_Guide_Line', // 路口导向线（不能修改尺寸和位置）
  //   showName: i18n.global.t('desc.editObject.intersectionGuideLine'),
  //   iconUrl: ImgUrls.roadSign.intersectionGuideLine,
  //   textureUrl: '',
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['junction'],
  // },
]
// 警告标线
export const warningRoadSign: Array<IRoadSignConfig> = [
  {
    mainType: 'roadSign',
    name: 'Longitudinal_Deceleration_Marking', // 车行道纵向减速标线
    showName: i18n.global.t('desc.editObject.longitudinalDecelerationMarking'),
    iconUrl: ImgUrls.roadSign.longitudinalDecelerationMarking,
    textureUrl: '',
    basicSize: 0,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'l', // 只能控制长度
  },
  {
    mainType: 'roadSign',
    name: 'Lateral_Deceleration_Marking', // 车行道横向减速标线
    showName: i18n.global.t('desc.editObject.lateralDecelerationMarking'),
    iconUrl: ImgUrls.roadSign.lateralDecelerationMarking,
    textureUrl: '',
    basicSize: 0,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'l', // 只能控制长度
  },
]
// 禁止标线
export const prohibitionRoadSign: Array<IRoadSignConfig> = [
  {
    mainType: 'roadSign',
    name: 'Stop_To_Give_Way', // 停车让行线
    showName: i18n.global.t('desc.editObject.stopAndGiveWay'),
    iconUrl: ImgUrls.roadSign.stopAndGiveWay,
    textureUrl: ImgUrls.roadSign.stopAndGiveWay,
    basicSize: 3,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  // {
  //   mainType: 'roadSign',
  //   name: 'Slow_Down_To_Give_Way', // 减速让行线
  //   showName: i18n.global.t('desc.editObject.slowDownAndGiveWay'),
  //   iconUrl: ImgUrls.roadSign.slowDownAndGiveWay,
  //   textureUrl: ImgUrls.roadSign.slowDownAndGiveWay,
  //   basicSize: 3,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['lane'],
  //   adjustSize: 'ratio2',
  // },
  // {
  //   mainType: 'roadSign',
  //   name: 'Stop_Line', // 停止线
  //   showName: i18n.global.t('desc.editObject.stopLine'),
  //   iconUrl: ImgUrls.roadSign.stopLineIcon,
  //   textureUrl: ImgUrls.roadSign.stopLineIcon,
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['lane'],
  // },
  {
    mainType: 'roadSign',
    name: 'Circular_Center_Circle', // 圆形中心圈
    showName: i18n.global.t('desc.editObject.circularCenterCircle'),
    iconUrl: ImgUrls.roadSign.circularCenterCircle,
    textureUrl: ImgUrls.roadSign.circularCenterCircle,
    basicSize: 6,
    type: 'none',
    subtype: '',
    placementAreaName: ['junction'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Rhombus_Center_Circle', // 菱形中心圈
    showName: i18n.global.t('desc.editObject.diamondCenterCircle'),
    iconUrl: ImgUrls.roadSign.diamondCenterCircle,
    textureUrl: ImgUrls.roadSign.diamondCenterCircle,
    basicSize: 6.6,
    type: 'none',
    subtype: '',
    placementAreaName: ['junction'],
    adjustSize: 'lw',
  },
  {
    mainType: 'roadSign',
    name: 'Mesh_Line', // 网状线
    showName: i18n.global.t('desc.editObject.meshLine'),
    iconUrl: ImgUrls.roadSign.meshLine,
    textureUrl: ImgUrls.roadSign.meshLine,
    basicSize: 6.6,
    type: 'none',
    subtype: '',
    placementAreaName: ['junction'],
    adjustSize: 'lw',
  },
  {
    mainType: 'roadSign',
    name: 'Small_Cars_Lane_Line', // 小型车专用车道线
    showName: i18n.global.t('desc.editObject.smallCarLane'),
    iconUrl: ImgUrls.roadSign.smallCarLane,
    textureUrl: ImgUrls.roadSign.smallCarLane,
    basicSize: 1.1,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Big_Cars_Lane_Line', // 小型车专用车道线
    showName: i18n.global.t('desc.editObject.largeCarLane'),
    iconUrl: ImgUrls.roadSign.largeCarLane,
    textureUrl: ImgUrls.roadSign.largeCarLane,
    basicSize: 1.1,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Non_Motor_Vehicle_Line', // 非机动车道线
    showName: i18n.global.t('desc.editObject.nonMotorVehiclesOnlyMarkings'),
    iconUrl: ImgUrls.roadSign.nonMotorVehiclesOnlyMarkings,
    textureUrl: ImgUrls.roadSign.nonMotorVehiclesOnlyMarkings,
    basicSize: 2.2,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Bus_Only_Lane_Line', // 公交专用车道
    showName: i18n.global.t('desc.editObject.busLane'),
    iconUrl: ImgUrls.roadSign.busOnlyLane,
    textureUrl: ImgUrls.roadSign.busOnlyLane,
    basicSize: 1.3,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  {
    mainType: 'roadSign',
    name: 'Turning_Forbidden', // 禁止转弯
    showName: i18n.global.t('desc.editObject.turningForbidden'),
    iconUrl: ImgUrls.roadSign.turningForbidden,
    textureUrl: ImgUrls.roadSign.turningForbidden,
    basicSize: 2.15,
    type: 'none',
    subtype: '',
    placementAreaName: ['lane'],
    adjustSize: 'ratio2',
  },
  // {
  //   mainType: 'roadSign',
  //   name: 'U_Turning_Forbidden', // 禁止掉头
  //   showName: i18n.global.t('desc.editObject.uTurningForbidden'),
  //   iconUrl: ImgUrls.roadSign.uTurningForbidden,
  //   textureUrl: ImgUrls.roadSign.uTurningForbidden,
  //   basicSize: 2.15,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['lane'],
  //   adjustSize: 'ratio2',
  // },
  // {
  //   mainType: 'roadSign',
  //   name: 'Non_Motor_Vehicle_Area', // 非机动车禁驶区标线
  //   showName: i18n.global.t('desc.editObject.nonMotorVehicle'),
  //   iconUrl: ImgUrls.roadSign.nonMotorVehicleIcon,
  //   textureUrl: '',
  //   basicSize: 0,
  //   type: 'none',
  //   subtype: '',
  //   placementAreaName: ['junction'],
  //   adjustSize: 'lw',
  // },
]
export const otherRoadSign: Array<IRoadSignConfig> = [
  {
    mainType: 'parkingSpace',
    name: 'Parking_Space_Mark', // 停车位
    showName: i18n.global.t('desc.editObject.parkingSpotMarking'),
    iconUrl: ImgUrls.roadSign.parkingSpotMarkingIcon,
    textureUrl: ImgUrls.roadSign.parkingSpotMarkingIcon,
    basicSize: 3.3,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Time_Limit_Parking_Space_Mark', // 限时停车位（虚线）
    showName: i18n.global.t('desc.editObject.limitedTimeParkingSpace'),
    iconUrl: ImgUrls.roadSign.limitedTimeParkingSpaceIcon,
    textureUrl: ImgUrls.roadSign.limitedTimeParkingSpaceIcon,
    basicSize: 3.3,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Parking_6m', // 6米停车位
    showName: i18n.global.t('desc.editObject.parking6m'),
    iconUrl: ImgUrls.roadSign.parking6mIcon,
    textureUrl: ImgUrls.roadSign.parking6mIcon,
    basicSize: 3.3,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Parking_5m', // 5米停车位
    showName: i18n.global.t('desc.editObject.parking5m'),
    iconUrl: ImgUrls.roadSign.parking5mIcon,
    textureUrl: ImgUrls.roadSign.parking5mIcon,
    basicSize: 3.3,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Parking_45deg', // 45°停车位
    showName: i18n.global.t('desc.editObject.parking45deg'),
    iconUrl: ImgUrls.roadSign.parking45degIcon,
    textureUrl: ImgUrls.roadSign.parking45degIcon,
    basicSize: 3.3,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Parking_60deg', // 60°停车位
    showName: i18n.global.t('desc.editObject.parking60deg'),
    iconUrl: ImgUrls.roadSign.parking60degIcon,
    textureUrl: ImgUrls.roadSign.parking60degIcon,
    basicSize: 3.3,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
  },
]

export const _roadSignConfig = [
  {
    name: i18n.global.t('desc.editObject.indicatorLine'),
    type: 'indicatorRoadSign',
    list: indicatorRoadSign,
  },
  {
    name: i18n.global.t('desc.editObject.warningLine'),
    type: 'warningRoadSign',
    list: warningRoadSign,
  },
  {
    name: i18n.global.t('desc.editObject.prohibitionLine'),
    type: 'prohibitionRoadSign',
    list: prohibitionRoadSign,
  },
  {
    name: i18n.global.t('desc.editObject.otherLine'),
    type: 'otherRoadSign',
    list: otherRoadSign,
  },
]
export const roadSignConfig = _roadSignConfig.filter((item) => {
  if (item.type === 'indicatorRoadSign') {
    return getPermission(
      'action.mapEditor.models.roadSign.indicator.view.enable',
    )
  }
  if (item.type === 'warningRoadSign') {
    return getPermission('action.mapEditor.models.roadSign.warning.view.enable')
  }
  if (item.type === 'prohibitionRoadSign') {
    return getPermission(
      'action.mapEditor.models.roadSign.prohibition.view.enable',
    )
  }
  if (item.type === 'otherRoadSign') {
    return getPermission('action.mapEditor.models.roadSign.other.view.enable')
  }
  return true
})

// 通过名称获取配置
export function getRoadSignConfig (name: string) {
  for (const roadSignType of roadSignConfig) {
    for (const option of roadSignType.list) {
      if (option.name === name) {
        return option
      }
    }
  }
  return null
}
