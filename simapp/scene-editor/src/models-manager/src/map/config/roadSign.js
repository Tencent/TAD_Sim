import { ImgUrls } from '../utils/urls'

let roadSignConfig = []

export function initRoadSignConfig () {
  // 路面标识【指示类型】
  const indicatorRoadSign = [
    {
      mainType: 'roadSign',
      name: 'Arrow_Forward', // 前方直行
      textureUrl: ImgUrls.roadSign.straight,
      basicSize: 0.8,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Left', // 前方左转
      textureUrl: ImgUrls.roadSign.leftTurn,
      basicSize: 1.2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Right', // 前方右转
      textureUrl: ImgUrls.roadSign.rightTurn,
      basicSize: 1.2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Left_And_Forward', // 前方可直行或左转
      textureUrl: ImgUrls.roadSign.straightOrLeftTurn,
      basicSize: 1.8,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Right_And_Forward', // 前方可直行或右转
      textureUrl: ImgUrls.roadSign.straightOrRightTurn,
      basicSize: 1.8,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_U_Turns', // 前方掉头
      textureUrl: ImgUrls.roadSign.uTurn,
      basicSize: 1.45,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Forward_And_U_Turns', // 前方可直行或掉头
      textureUrl: ImgUrls.roadSign.straightOrUTurn,
      basicSize: 1.6,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Left_And_U_Turns', // 前方可左转或掉头
      textureUrl: ImgUrls.roadSign.leftTurnOrUTurn,
      basicSize: 1.45,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Left_And_Right', // 前方道路仅可左右转弯
      textureUrl: ImgUrls.roadSign.turnLeftOrRightOnly,
      basicSize: 1.9,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Arrow_Turn_And_Straight', // 转弯直行
      textureUrl: ImgUrls.roadSign.turnOrStraight,
      basicSize: 1.9,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Crosswalk_Warning_Line', // 人行横道预告线
      textureUrl: ImgUrls.roadSign.pedestrianCrosswalkWarningSigns,
      basicSize: 1.8,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'White_Broken_Line_Vehicle_Distance_Confirmation', // 白色折线车距确认线
      textureUrl: ImgUrls.roadSign.whiteBrokenLineVehicleDistanceConfirmation,
      basicSize: 1.8,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Turn_Left_Waiting', // 左转弯待转区
      textureUrl: ImgUrls.roadSign.leftTurningVehicleWaitingArea,
      basicSize: 3.3,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Turn_And_Merge_Left', // 前方道路有左弯或需向左合流
      textureUrl: ImgUrls.roadSign.leftBendAheadOrNeedsToMergeLeft,
      basicSize: 1.2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Turn_And_Merge_Right', // 前方道路有右弯或需向右合流
      textureUrl: ImgUrls.roadSign.rightBendAheadOrNeedsToMergeRight,
      basicSize: 1.2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Word_Mark_100_120', // 路面文字标识（100-120）
      textureUrl: ImgUrls.roadSign.pavementMarking100120,
      basicSize: 2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Word_Mark_80_100', // 路面文字标识（80-100）
      textureUrl: ImgUrls.roadSign.pavementMarking80100,
      basicSize: 2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Non_Motor_Vehicle', // 非机动车道
      textureUrl: ImgUrls.roadSign.bicycle,
      basicSize: 2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Disabled_Parking_Space_Road_Mark', // 残疾人专用停车位路面标记
      textureUrl: ImgUrls.roadSign.disabledParkingSpots,
      basicSize: 1.45,
      type: 'none',
      subtype: '',
    },
  ]

  // 路面标识【禁止类型】
  const prohibitionRoadSign = [
    {
      mainType: 'roadSign',
      name: 'Stop_To_Give_Way', // 停车让行线
      textureUrl: ImgUrls.roadSign.stopAndGiveWay,
      basicSize: 3,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Circular_Center_Circle', // 圆形中心圈
      textureUrl: ImgUrls.roadSign.circularCenterCircle,
      basicSize: 6,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Rhombus_Center_Circle', // 菱形中心圈
      textureUrl: ImgUrls.roadSign.diamondCenterCircle,
      basicSize: 6.6,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Mesh_Line', // 网状线
      textureUrl: ImgUrls.roadSign.meshLine,
      basicSize: 6.6,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Small_Cars_Lane_Line', // 小型车专用车道线
      textureUrl: ImgUrls.roadSign.smallCarLane,
      basicSize: 1.1,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Big_Cars_Lane_Line', // 小型车专用车道线
      textureUrl: ImgUrls.roadSign.largeCarLane,
      basicSize: 1.1,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Non_Motor_Vehicle_Line', // 非机动车道线
      textureUrl: ImgUrls.roadSign.nonMotorVehiclesOnlyMarkings,
      basicSize: 2.2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Bus_Only_Lane_Line', // 公交专用车道
      textureUrl: ImgUrls.roadSign.busOnlyLane,
      basicSize: 1.3,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Turning_Forbidden', // 禁止转弯
      textureUrl: ImgUrls.roadSign.turningForbidden,
      basicSize: 2.15,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'Slow_Down_To_Give_Way', // 减速让行线
      textureUrl: ImgUrls.roadSign.slowDownAndGiveWay,
      basicSize: 3,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'roadSign',
      name: 'U_Turning_Forbidden', // 禁止掉头
      textureUrl: ImgUrls.roadSign.uTurningForbidden,
      basicSize: 2.15,
      type: 'none',
      subtype: '',
    },
  ]

  // 路面标识【其他类型】
  const otherRoadSign = [
    {
      mainType: 'parkingSpace',
      name: 'Parking_Space_Mark', // 停车位
      textureUrl: ImgUrls.roadSign.parkingSpotMarkingIcon,
      type: 'parkingSpace',
      subtype: '',
    },
  ]

  roadSignConfig = [
    {
      type: 'indicatorRoadSign',
      list: indicatorRoadSign,
    },
    {
      type: 'prohibitionRoadSign',
      list: prohibitionRoadSign,
    },
    {
      type: 'otherRoadSign',
      list: otherRoadSign,
    },
  ]
}

// 通过名称查询路面标识的配置信息
export function getRoadSignConfig (name) {
  for (const basicType of roadSignConfig) {
    for (const option of basicType.list) {
      if (option.name === name) {
        return option
      }
    }
  }

  return null
}
