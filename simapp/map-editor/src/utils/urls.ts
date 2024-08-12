// 通过 './static/' 引用 public 中的静态资源（开发环境和编译后均能访问到对应静态资源）
const modelPath = './static/models'
const imgPath = './static/imgs'

// 三维模型的静态资源路径
export const ModelUrls = {
  signalBoard: {
    triangle: `${modelPath}/signalBoard/SM_TPP_A.fbx`,
    square: `${modelPath}/signalBoard/SM_TPP_B.fbx`,
    circle: `${modelPath}/signalBoard/SM_TPP_C.fbx`,
    octagon: `${modelPath}/signalBoard/SM_TPP_D.fbx`, // 八边形
    invertedTriangle: `${modelPath}/signalBoard/SM_TPP_E.fbx`, // 倒三角
  },
  trafficLight: {
    // 竖直三盏灯，对应 001 的 01-05 纹理
    verticalOmnidirectionalLight:
      `${modelPath}/trafficLight/sm_trli_001_01.FBX`,
    verticalLeftTurnLight: `${modelPath}/trafficLight/sm_trli_001_02.FBX`,
    verticalStraightLight: `${modelPath}/trafficLight/sm_trli_001_03.FBX`,
    verticalRightTurnLight: `${modelPath}/trafficLight/sm_trli_001_04.FBX`,
    verticalUTurnLight: `${modelPath}/trafficLight/sm_trli_001_05.FBX`,
    // 竖直两盏灯，对应 001 的 06 纹理
    verticalPedestrianLight: `${modelPath}/trafficLight/sm_trli_001_06.FBX`,
    // 水平三盏灯，对应 002 的 01-05 纹理
    horizontalOmnidirectionalLight:
      `${modelPath}/trafficLight/sm_trli_002_01.FBX`,
    horizontalLeftTurnLight: `${modelPath}/trafficLight/sm_trli_002_02.FBX`,
    horizontalStraightLight: `${modelPath}/trafficLight/sm_trli_002_03.FBX`,
    horizontalRightTurnLight: `${modelPath}/trafficLight/sm_trli_002_04.FBX`,
    horizontalUTurnLight: `${modelPath}/trafficLight/sm_trli_002_05.FBX`,

    // 双色灯
    twoColorIndicatorLight:
      `${modelPath}/trafficLight/sm_trli_shuangsedeng.fbx`,
    // 单车灯
    bicycleLight: `${modelPath}/trafficLight/sm_trli_danchedeng.fbx`,
  },
  other: {
    // 路面污损
    bump: `${modelPath}/other/sm_lumiantuqi.fbx`, // 路面凸起
    // 交通管理
    manholeCover: `${modelPath}/other/sm_obpg_cove_001.fbx`, // 井盖
    parkingLever: `${modelPath}/other/sm_tingchegan.fbx`, // 停车杆
    parkingPile: `${modelPath}/other/sm_tingchezhuang.fbx`, // 停车桩
    groundLock: `${modelPath}/other/sm_disuo.fbx`, // 地锁
    plasticCarStopper: `${modelPath}/other/sm_sujiaodangcheqi.fbx`, // 塑料挡车器
    uShapedCarStopper: `${modelPath}/other/sm_uxingdangcheqi.fbx`, // U型挡车器
    supportTypeCarStopper: `${modelPath}/other/sm_zhichengxingdangcheqi.fbx`, // 支撑型挡车器
    chargingStation: `${modelPath}/other/SM_ChongDianZhuang.fbx`, // 充电桩
    lamp: `${modelPath}/other/sm_ludeng.fbx`, // 路灯
    trafficBarrier: `${modelPath}/other/sm_weilan.fbx`, // 护栏
    roadCurb: `${modelPath}/other/sm_lubianshi.fbx`, // 路沿石
    reflectiveRoadSign: `${modelPath}/other/sm_fanguanglubiao.fbx`, // 反光路标
    trafficCone: `${modelPath}/other/sm_odr_stob_080b.fbx`, // 交通锥
    trafficHorse: `${modelPath}/other/sm_shuima.fbx`, // 水马
    obstacle: `${modelPath}/other/sm_stob_fangzhuangtong.fbx`, // 防撞桶
    garbageCan: `${modelPath}/other/sm_trash_002.fbx`, // 垃圾桶
    decelerationZone: `${modelPath}/other/sm_stob_jiansudai_d.fbx`, // 减速带
    // 植被
    tree: `${modelPath}/other/sm_tree.fbx`, // 树木
    shrub: `${modelPath}/other/dongqing_a02.fbx`, // 灌木
    grass: `${modelPath}/other/sm_grass_2_2.fbx`, // 草坪
    // 建筑
    residence: `${modelPath}/other/sm_bud.fbx`, // 居民楼
    busStop: `${modelPath}/other/SM_Busstop.fbx`, // 公交站
    // 天桥
    pedestrianBridge: `${modelPath}/other/sm_prbridge_10m.fbx`, // 人行天桥模型1
    pedestrianBridge1: `${modelPath}/other/sm_prbridge_10m1.fbx`, // 人行天桥模型2
    pedestrianBridge2: `${modelPath}/other/sm_prbridge_10m2.fbx`, // 人行天桥模型3
  },
  sensor: {
    camera: `${modelPath}/sensor/camera/sm_camera_001.fbx`, // 摄像头
    radar: `${modelPath}/sensor/hmbradar/sm_hmbradar_a.fbx`, // 毫米波雷达
    rsu: `${modelPath}/sensor/rsu/sm_rsu_01_a.fbx`, // rsu
    lidar: `${modelPath}/sensor/lidar/sm_leida.fbx`, // 激光雷达
  },
}

// 传感器图片路径
const sensorUrls = {
  // icon
  cameraIcon: `${imgPath}/sensor/ico_camera.png`, // 摄像头
  radarIcon: `${imgPath}/sensor/ico_reda.png`, // 毫米波雷达
  rsuIcon: `${imgPath}/sensor/icon_rsu.png`, // rsu
  lidarIcon: `${imgPath}/sensor/icon_leida.png`, // 激光雷达
  // 模型纹理
  camera: `${modelPath}/sensor/camera/t_camera_001_BC.png`, // 摄像头
  radar: `${modelPath}/sensor/hmbradar/t_HMBradar_A_BC.png`, // 毫米波雷达
  rsu: `${modelPath}/sensor/rsu/t_hik_camra_ai.png`, // rsu
  lidar: `${modelPath}/sensor/lidar/LD_01_Base_Color.png`, // 激光雷达
}

const otherTexturePath = `${imgPath}/other/`
const otherUrls = {
  // ------ icon ------
  // 地面污损
  potholeIcon: `${otherTexturePath}icon_sm_kengwa.png`, // 坑洼
  patchIcon: `${otherTexturePath}t_obpg_tran_004.png`, // 补丁
  crackIcon: `${otherTexturePath}icon_sm_liefeng.png`, // 裂缝
  asphaltLineIcon: `${otherTexturePath}icon_sm_liqing.png`, // 沥青线
  tireMarksIcon: `${otherTexturePath}icon_sm_chetaihenji.png`, // 车胎痕迹
  stagnantWaterIcon: `${otherTexturePath}icon_sm_jishui.png`, // 积水
  bumpIcon: `${otherTexturePath}icon_sm_lumiantuqi.png`, // 凸起
  // 交通管理
  manholeCoverIcon: `${otherTexturePath}t_obpg_cove_001.png`, // 井盖
  parkingLeverIcon: `${otherTexturePath}ico_tingchegan.png`, // 停车杆
  parkingPileIcon: `${otherTexturePath}ico_tingchezhuang.png`, // 停车桩
  groundLockIcon: `${otherTexturePath}ico_disuo.png`, // 地锁
  plasticCarStopperIcon: `${otherTexturePath}ico_xiangsudangcheqi.png`, // 塑料挡车器
  uShapedCarStopperIcon: `${otherTexturePath}ico_uxingdangcheqi.png`, // U型挡车器
  supportTypeCarStopperIcon: `${otherTexturePath}ico_zhichengxingdangcheqi.png`, // 支撑型挡车器
  chargingStationIcon: `${otherTexturePath}icon_chargingpile.png`, // 充电桩
  reflectiveRoadSignIcon: `${otherTexturePath}icon_sm_fanguanglubiao.png`, // 反光路标
  lampIcon: `${otherTexturePath}ico_ludeng.png`, // 路灯
  trafficBarrierIcon: `${otherTexturePath}ico_weilan.png`, // 护栏
  roadCurbIcon: `${otherTexturePath}ico_lubianshi.png`, // 路沿石
  trafficConeIcon: `${otherTexturePath}ico_odr_stob_080b.png`, // 交通锥
  trafficHorseIcon: `${otherTexturePath}icon_shuima.png`, // 水马
  obstacleIcon: `${otherTexturePath}ico_stob_fangzhuangtong.png`, // 防撞桶
  garbageCanIcon: `${otherTexturePath}ico_trash_002.png`, // 垃圾桶
  decelerationZoneIcon: `${otherTexturePath}icon_stob_jiansudai_d.png`, // 减速带
  // 植被
  treeIcon: `${otherTexturePath}ico_tree.png`, // 树木
  shrubIcon: `${otherTexturePath}ico_dongqing_a02.png`, // 灌木
  grassIcon: `${otherTexturePath}ico_grass_2_2.png`, // 草坪
  // 建筑
  residenceIcon: `${otherTexturePath}ico_bud.png`, // 居民楼
  busStopIcon: `${otherTexturePath}icon_busstop.png`, // 公交站
  // 人行天桥
  pedestrianBridgeIcon: `${otherTexturePath}icon_tianqiao_10m.png`, // 人行天桥
  // ------ model texture ------
  // 地面污损
  pothole: `${otherTexturePath}t_kengwa.png`, // 坑洼
  crack: `${otherTexturePath}t_liefeng.png`, // 裂缝
  asphaltLine: `${otherTexturePath}t_liqing.png`, // 沥青线
  tireMarks: `${otherTexturePath}t_chetaihenji.png`, // 车胎痕迹
  stagnantWater: `${otherTexturePath}t_jishui.png`, // 积水
  bump: `${otherTexturePath}t_lumiantuqi.png`, // 凸起
  // 交通管理
  reflectiveRoadSign: `${modelPath}/other/t_fanguanglubiao.png`, // 反光路标
  // 植被
  grass: `${modelPath}/other/T_Grass.png`, // 草坪
  // 其他元素的纹理
  arrow: `${otherTexturePath}ico_arrow.jpg`, // 箭头
  commonRoadIcon: `${otherTexturePath}icon_commonroad.png`, // 普通双向双车道 icon
  rampIcon: `${otherTexturePath}icon_ramp.png`, // 匝道 icon
  tunnelIcon: `${otherTexturePath}icon_tunnel.png`, // 隧道 icon
}

// 交通信号灯的路径(单独的纹理和icon)
const lightIconPath = `${imgPath}/trafficLight/`
const lightTexturePath = `${modelPath}/trafficLight/`
const trafficLightUrls = {
  //  ---------- icon ----------
  verticalOmnidirectionalLightIcon: `${lightIconPath}ico_sm_trli_001_01.png`, // 竖排全方位灯
  verticalStraightRoundLightIcon: `${lightIconPath}ico_sm_trli_001_01.png`, // 竖排直行圆灯
  verticalLeftTurnLightIcon: `${lightIconPath}ico_sm_trli_001_02.png`, // 竖排左转灯
  verticalStraightLightIcon: `${lightIconPath}ico_sm_trli_001_03.png`, // 竖排直行灯
  verticalRightTurnLightIcon: `${lightIconPath}ico_sm_trli_001_04.png`, // 竖排右转灯
  verticalUTurnLightIcon: `${lightIconPath}ico_sm_trli_001_05.png`, // 竖排掉头灯
  verticalPedestrianLightIcon: `${lightIconPath}ico_sm_trli_001_06.png`, // 竖排人行灯
  horizontalOmnidirectionalLightIcon: `${lightIconPath}ico_sm_trli_002_01.png`, // 横排全方位灯
  horizontalStraightRoundLightIcon: `${lightIconPath}ico_sm_trli_002_01.png`, // 横排直行圆灯
  horizontalLeftTurnLightIcon: `${lightIconPath}ico_sm_trli_002_02.png`, // 横排左转灯
  horizontalStraightLightIcon: `${lightIconPath}ico_sm_trli_002_03.png`, // 横排直行灯
  horizontalRightTurnLightIcon: `${lightIconPath}ico_sm_trli_002_04.png`, // 横排右转灯
  horizontalUTurnLightIcon: `${lightIconPath}ico_sm_trli_002_05.png`, // 横排掉头灯
  bicycleLightIcon: `${lightIconPath}ico_trli_danchedeng.png`, // 单车灯
  twoColorIndicatorLightIcon: `${lightIconPath}ico_trli_shuangsedeng.png`, // 双色灯
  //  ---------- texture ----------
  verticalOmnidirectionalLight: `${lightTexturePath}t_sm_trli_001_01.png`, // 竖排全方位灯
  verticalStraightRoundLight: `${lightTexturePath}t_sm_trli_001_01.png`, // 竖排行圆灯
  verticalLeftTurnLight: `${lightTexturePath}t_sm_trli_001_02.png`, // 竖排左转灯
  verticalStraightLight: `${lightTexturePath}t_sm_trli_001_03.png`, // 竖排直行灯
  verticalRightTurnLight: `${lightTexturePath}t_sm_trli_001_04.png`, // 竖排右转灯
  verticalUTurnLight: `${lightTexturePath}t_sm_trli_001_05.png`, // 竖排掉头灯
  verticalPedestrianLight: `${lightTexturePath}t_sm_trli_001_06.png`, // 竖排人行灯
  horizontalOmnidirectionalLight: `${lightTexturePath}t_sm_trli_002_01.png`, // 横排全方位灯
  horizontalStraightRoundLight: `${lightTexturePath}t_sm_trli_002_01.png`, // 横排直行圆灯
  horizontalLeftTurnLight: `${lightTexturePath}t_sm_trli_002_02.png`, // 横排左转灯
  horizontalStraightLight: `${lightTexturePath}t_sm_trli_002_03.png`, // 横排直行灯
  horizontalRightTurnLight: `${lightTexturePath}t_sm_trLi_002_04.png`, // 横排右转灯
  horizontalUTurnLight: `${lightTexturePath}t_sm_trLi_002_05.png`, // 横排掉头灯
  bicycleLight: `${lightTexturePath}t_trli_danchedeng.png`, // 单车灯
  twoColorIndicatorLight: `${lightTexturePath}t_trli_shuangsedeng.png`, // 双色灯
}

const roadSignBasicPath = `${imgPath}/roadSign/`
// 路面标识的静态资源路径
const roadSignUrls = {
  // 指示标线
  straight: `${roadSignBasicPath}ZS_11.png`,
  leftTurn: `${roadSignBasicPath}ZS_13.png`,
  rightTurn: `${roadSignBasicPath}ZS_14.png`,
  straightOrLeftTurn: `${roadSignBasicPath}ZS_12.png`,
  straightOrRightTurn: `${roadSignBasicPath}ZS_15.png`,
  uTurn: `${roadSignBasicPath}ZS_16.png`,
  straightOrUTurn: `${roadSignBasicPath}ZS_17.png`,
  leftTurnOrUTurn: `${roadSignBasicPath}ZS_18.png`,
  turnLeftOrRightOnly: `${roadSignBasicPath}ZS_19.png`,
  leftBendAheadOrNeedsToMergeLeft: `${roadSignBasicPath}ZS_20.png`,
  rightBendAheadOrNeedsToMergeRight: `${roadSignBasicPath}ZS_21.png`,
  pavementMarking80100: `${roadSignBasicPath}ZS_22_2.png`,
  pavementMarking100120: `${roadSignBasicPath}ZS_22.png`,
  bicycle: `${roadSignBasicPath}ZS_23.png`,
  disabledParkingSpots: `${roadSignBasicPath}ZS_24.png`,
  leftTurningVehicleWaitingArea: `${roadSignBasicPath}ZS_01.png`, // 左转弯待转区
  pedestrianCrosswalkWarningSigns: `${roadSignBasicPath}ZS_05.png`,
  whiteBrokenLineVehicleDistanceConfirmation: `${roadSignBasicPath}ZS_07.png`, // 白色折线车距确认线
  whiteSemicircleLineVehicleDistanceConfirmation:
    `${roadSignBasicPath
    }icon_White_Semicircle_Line_Vehicle_Distance_Confirmation.png`, // 白色半圆状车距确认线
  turnOrStraight: `${roadSignBasicPath}arrow_straightleftright.png`, // 转弯直行
  crosswalkLineIcon: `${roadSignBasicPath}icon_Crosswalk_Line.png`, // 人行横道 icon
  guideLaneLine: `${roadSignBasicPath}icon_road_guide_lane_line.png`, // 导向车道线
  variableDirectionLaneLine:
    `${roadSignBasicPath}icon_Variable direction lane line.png`, // 可变导向车道线
  crosswalkWithLeftAndRightSide:
    `${roadSignBasicPath}icon_crosswalk_with_side.png`, // 行人左右分道的人行横道线
  intersectionGuideLine: `${roadSignBasicPath}icon_Intersection_guide_line.png`, // 路口导向线
  // 警告标线
  longitudinalDecelerationMarking:
    `${roadSignBasicPath}icon_Longitudinal_Deceleration_ Marking.png`, // 车行道纵向减速标线
  lateralDecelerationMarking:
    `${roadSignBasicPath}icon_Lateral_Deceleration_Marking.png`, // 车行道横线减速标线
  // 禁止标线
  stopLineIcon: `${roadSignBasicPath}icon_Stop_Line.png`, // 停止线
  stopAndGiveWay: `${roadSignBasicPath}JZ_26.png`, // 停车让行线
  slowDownAndGiveWay: `${roadSignBasicPath}icon_slow_down_To_Give_Way.png`, // 减速让行线
  circularCenterCircle: `${roadSignBasicPath}JZ_29.png`, // 圆形中心圈
  diamondCenterCircle: `${roadSignBasicPath}JZ_30.png`, // 菱形中心圈
  meshLine: `${roadSignBasicPath}JZ_31.png`, // 网状线
  smallCarLane: `${roadSignBasicPath}JZ_33.png`, // 小型车专用车道线
  largeCarLane: `${roadSignBasicPath}JZ_34.png`, // 大型车专用车道线
  nonMotorVehiclesOnlyMarkings: `${roadSignBasicPath}JZ_35.png`, // 非机动车道线
  turningForbidden: `${roadSignBasicPath}JZ_37.png`, // 禁止转弯
  uTurningForbidden: `${roadSignBasicPath}JZ_38.png`, // 禁止掉头
  busOnlyLane: `${roadSignBasicPath}icon_gongjiao.png`, // 公交专用车道
  nonMotorVehicleIcon: `${roadSignBasicPath}icon_Non_Motor_Vehicle.png`, // 非机动车禁驶区标线
  // 其他标线
  parkingSpotMarkingIcon: `${roadSignBasicPath}r1.png`, // 停车位标线 icon
  limitedTimeParkingSpaceIcon: `${roadSignBasicPath}r2.png`, // 限时停车位标线 icon（虚线）
  parking6mIcon: `${roadSignBasicPath}icon_parking_6m.png`, // 6米停车位
  parking5mIcon: `${roadSignBasicPath}icon_parking_5m.png`, // 5米停车位
  parking60degIcon: `${roadSignBasicPath}icon_parking_60deg.png`, // 60度停车位
  parking45degIcon: `${roadSignBasicPath}icon_parking_45deg.png`, // 45度停车位
}

const signalBoardBasicPath = `${imgPath}/signalBoard/`
// 信号标识牌的静态资源路径
const signalBoardUrls = {
  // 警告标志（三角形）
  slowDown: `${signalBoardBasicPath}T_tpp_001_02.png`, // 慢行
  sharpCurveToLeft: `${signalBoardBasicPath}T_tpp_001_03.png`, // 向左急转弯
  sharpCurveToRight: `${signalBoardBasicPath}T_tpp_001_04.png`, // 向右急转弯
  tShapedIntersection1: `${signalBoardBasicPath}T_tpp_001_05.png`, // T形交叉
  tShapedIntersection2: `${signalBoardBasicPath}T_tpp_001_06.png`,
  tShapedIntersection3: `${signalBoardBasicPath}T_TPP_00000002_BC_09.png`,
  crossIntersection: `${signalBoardBasicPath}T_tpp_001_07.png`, // 十字交叉
  watchForChildren: `${signalBoardBasicPath}T_tpp_001_08.png`, // 注意儿童
  uphill: `${signalBoardBasicPath}T_tpp_001_57.png`, // 上坡路
  roadWorkAhead: `${signalBoardBasicPath}T_tpp_001_58.png`, // 施工
  roadNarrowsOnLeft: `${signalBoardBasicPath}T_tpp_001_59.png`, // 左侧变窄
  roadNarrowsOnRight: `${signalBoardBasicPath}T_TPP_00000003_BC_35.png`, // 右侧变窄
  roadNarrowsOnBothSides: `${signalBoardBasicPath}T_TPP_00000002_BC_01.png`, // 两侧变窄
  unguardedRailwayCrossing: `${signalBoardBasicPath}T_TPP_00000002_BC_02.png`, // 无人看守铁路道路
  steepMountainRoadLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_01.png`, // 傍山险路
  steepMountainRoadRight: `${signalBoardBasicPath}T_TPP_00000003_BC_02.png`,
  village: `${signalBoardBasicPath}T_TPP_00000003_BC_03.png`, // 村庄
  embankmentRoadLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_04.png`, // 堤坝路
  embankmentRoadRight: `${signalBoardBasicPath}T_TPP_00000003_BC_05.png`,
  tJunctionWithDualCarriageway:
    `${signalBoardBasicPath}T_TPP_00000003_BC_06.png`, // 丁字平面交叉
  ferry: `${signalBoardBasicPath}T_TPP_00000003_BC_07.png`, // 渡口
  fallingRocksRight: `${signalBoardBasicPath}T_TPP_00000003_BC_08.png`, // 注意落石
  reverseCurveLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_09.png`, // 反向弯路
  reverseCurveRight: `${signalBoardBasicPath}T_TPP_00000003_BC_10.png`,
  lowWaterCrossing: `${signalBoardBasicPath}T_TPP_00000003_BC_11.png`, // 过水路面
  crossroads1: `${signalBoardBasicPath}T_TPP_00000003_BC_12.png`, // 交叉路口
  crossroads2: `${signalBoardBasicPath}T_TPP_00000003_BC_15.png`,
  crossroads3: `${signalBoardBasicPath}T_TPP_00000003_BC_17.png`,
  vehiclesMergingLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_13.png`, // 车辆汇入
  vehiclesMergingRight: `${signalBoardBasicPath}T_TPP_00000003_BC_14.png`,
  circleRoad: `${signalBoardBasicPath}T_TPP_00000003_BC_16.png`, // 环形交叉
  windingRoadAhead: `${signalBoardBasicPath}T_TPP_00000003_BC_18.png`, // 连续弯路
  longDescent: `${signalBoardBasicPath}T_TPP_00000003_BC_19.png`, // 连续下坡
  roughRoadAhead: `${signalBoardBasicPath}T_TPP_00000003_BC_20.png`, // 路面不平
  driveCarefullyInRainOrSnow: `${signalBoardBasicPath}T_TPP_00000003_BC_21.png`, // 注意雨雪天气
  yieldToPeopleWithDisabilities:
    `${signalBoardBasicPath}T_TPP_00000003_BC_22.png`, // 注意残疾人
  watchForWildAnimals: `${signalBoardBasicPath}T_TPP_00000003_BC_23.png`, // 注意野生动物
  accidentArea: `${signalBoardBasicPath}T_TPP_00000003_BC_24.png`, // 事故易发路段
  reversibleLaneAhead: `${signalBoardBasicPath}T_TPP_00000003_BC_25.png`, // 潮汐车道
  driveCarefullyInAdverseWeatherConditions:
    `${signalBoardBasicPath}T_TPP_00000003_BC_26.png`, // 注意不利气象条件
  lowLyingRoadAhead: `${signalBoardBasicPath}T_TPP_00000003_BC_27.png`, // 路面低洼
  bumpyRoadAhead: `${signalBoardBasicPath}T_TPP_00000003_BC_28.png`, // 路面高凸
  downhill: `${signalBoardBasicPath}T_TPP_00000003_BC_29.png`, // 下坡路
  queuesLikelyAhead: `${signalBoardBasicPath}T_TPP_00000003_BC_30.png`, // 注意前方车辆排队
  crossroadsWithDualCarriageway:
    `${signalBoardBasicPath}T_TPP_00000003_BC_31.png`, // 十字平面交叉
  tunnel: `${signalBoardBasicPath}T_TPP_00000003_BC_32.png`, // 隧道
  turnOnHeadlightsBeforeEnteringTunnel:
    `${signalBoardBasicPath}T_TPP_00000003_BC_33.png`, // 隧道开车灯
  camelBackBridge: `${signalBoardBasicPath}T_TPP_00000003_BC_34.png`, // 驼峰桥
  watchForNonMotorVehicles: `${signalBoardBasicPath}T_TPP_00000003_BC_36.png`, // 注意非机动车
  slipperySurface: `${signalBoardBasicPath}T_TPP_00000003_BC_37.png`, // 易滑
  trafficLightsAhead: `${signalBoardBasicPath}T_TPP_00000003_BC_38.png`, // 注意信号灯
  detourRight: `${signalBoardBasicPath}T_TPP_00000003_BC_39.png`, // 右侧绕行
  narrowBridge: `${signalBoardBasicPath}T_TPP_00000003_BC_40.png`, // 窄桥
  keepDistance: `${signalBoardBasicPath}T_TPP_00000003_BC_41.png`, // 注意保持车距
  roadsMergeLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_42.png`, // 注意合流
  roadsMergeRight: `${signalBoardBasicPath}T_TPP_00000003_BC_43.png`,
  bewareOfCrosswind: `${signalBoardBasicPath}T_TPP_00000003_BC_44.png`, // 注意横风
  icyRoads: `${signalBoardBasicPath}T_TPP_00000003_BC_45.png`, // 注意路面结冰
  fallingRocksLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_46.png`,
  driveWithCaution: `${signalBoardBasicPath}T_TPP_00000003_BC_47.png`, // 注意危险
  driveCarefullyInFoggyWeather:
    `${signalBoardBasicPath}T_TPP_00000003_BC_48.png`, // 注意雾天
  watchForLivestock: `${signalBoardBasicPath}T_TPP_00000003_BC_49.png`, // 注意牲畜
  detourLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_50.png`, // 左侧绕行
  detourAheadLeftOrRight: `${signalBoardBasicPath}T_TPP_00000003_BC_51.png`, // 左右绕行
  twoWayTraffic: `${signalBoardBasicPath}T_TPP_00000003_BC_52.png`, // 双向交通
  watchForPedestrians: `${signalBoardBasicPath}T_TPP_002_BC_18.png`, // 注意行人
  // 禁令标志（大多数圆形，少量八边形和倒三角形）
  noStraightThru: `${signalBoardBasicPath}T_tpp_001_09.png`, // 禁止直行
  noMotorVehicles: `${signalBoardBasicPath}T_tpp_001_10.png`, // 禁止机动车驶入
  speedLimit120: `${signalBoardBasicPath}T_tpp_001_11.png`, // 限速120
  speedLimit100: `${signalBoardBasicPath}T_tpp_001_12.png`, // 限速100
  speedLimit80: `${signalBoardBasicPath}T_tpp_001_13.png`, // 限速80
  speedLimit70: `${signalBoardBasicPath}T_TPP_00000004_BC_38.png`, // 限速70
  speedLimit60: `${signalBoardBasicPath}T_tpp_001_14.png`, // 限速60
  speedLimit50: `${signalBoardBasicPath}T_TPP_00000004_BC_37.png`, // 限速50
  speedLimit40: `${signalBoardBasicPath}T_tpp_001_15.png`, // 限速40
  speedLimit30: `${signalBoardBasicPath}T_tpp_001_16.png`, // 限速30
  speedLimit20: `${signalBoardBasicPath}T_tpp_001_23.png`, // 限速20
  speedLimit5: `${signalBoardBasicPath}T_tpp_001_24.png`, // 限速5
  stop: `${signalBoardBasicPath}T_tpp_001_21.png`, // 停车让行
  heightLimit5: `${signalBoardBasicPath}T_tpp_001_22.png`, // 限高5米
  heightLimit3dot5: `${signalBoardBasicPath}T_TPP_00000004_BC_28.png`, // 限高3.5m
  noEntryForVehicularTraffic: `${signalBoardBasicPath}T_tpp_001_29.png`, // 禁止驶入
  noMotorcycles: `${signalBoardBasicPath}T_tpp_001_30.png`, // 禁止二轮摩托车驶入
  weightLimit55: `${signalBoardBasicPath}T_tpp_001_31.png`, // 限重55
  weightLimit40: `${signalBoardBasicPath}T_tpp_001_48.png`, // 限重40
  weightLimit30: `${signalBoardBasicPath}T_tpp_001_39.png`, // 限重30
  weightLimit20: `${signalBoardBasicPath}T_tpp_001_32.png`, // 限重20
  weightLimit10: `${signalBoardBasicPath}T_tpp_001_40.png`, // 限重10
  noHonking: `${signalBoardBasicPath}T_tpp_001_37.png`, // 禁止鸣喇叭
  noTrucks: `${signalBoardBasicPath}T_tpp_001_38.png`, // 禁止载货汽车驶入
  noStopping: `${signalBoardBasicPath}T_tpp_001_45.png`, // 禁止停车
  axleWeightLimit14: `${signalBoardBasicPath}T_tpp_001_46.png`, // 限制轴重14
  axleWeightLimit13: `${signalBoardBasicPath}T_tpp_001_47.png`, // 限制轴重13
  slowDownAndGiveWay: `${signalBoardBasicPath}T_TPP_00000002_BC_03.png`, // 减速让行
  noLeftTurn: `${signalBoardBasicPath}T_TPP_00000002_BC_05.png`, // 禁止向左转弯
  heightLimit4dot5: `${signalBoardBasicPath}T_TPP_00000002_BC_06.png`, // 限高4.5
  vehiclesCarryingHazardousMaterialsProhibited:
    `${signalBoardBasicPath}T_TPP_00000002_BC_13.png`, // 禁止运输危险物品车辆驶入
  noLargeBuses: `${signalBoardBasicPath}T_TPP_00000002_BC_21.png`, // 禁止大型客车驶入
  noTractors: `${signalBoardBasicPath}T_TPP_00000002_BC_29.png`, // 禁止拖拉机驶入
  noEntryForNonMotorVehicles: `${signalBoardBasicPath}T_TPP_00000002_BC_37.png`, // 禁止非机动车驶入
  noPedestrians: `${signalBoardBasicPath}T_TPP_00000002_BC_45.png`, // 禁止行人进入
  noMotorTricycles: `${signalBoardBasicPath}T_TPP_00000002_BC_53.png`, // 禁止三轮机动车驶入
  noMinibuses: `${signalBoardBasicPath}T_TPP_00000004_BC_01.png`, // 禁止小型客车驶入
  noStraightThruOrLeftTurn: `${signalBoardBasicPath}T_TPP_00000004_BC_02.png`, // 禁止直行和向左转弯
  noHandcartsOrTricycles: `${signalBoardBasicPath}T_TPP_00000004_BC_03.png`, // 禁止人力车进入
  noHandcartsOrFreightTricycles:
    `${signalBoardBasicPath}T_TPP_00000004_BC_04.png`, // 禁止人力货运三轮车进入
  noPassengerTricycles: `${signalBoardBasicPath}T_TPP_00000004_BC_05.png`, // 禁止人力客运三轮车进入
  noRightTurn: `${signalBoardBasicPath}T_TPP_00000004_BC_06.png`, // 禁止向右转弯
  noTurns: `${signalBoardBasicPath}T_TPP_00000004_BC_07.png`, // 禁止向左向右转弯
  noStraightThruOrRightTurn: `${signalBoardBasicPath}T_TPP_00000004_BC_08.png`, // 禁止直行和向右转弯
  noEntry: `${signalBoardBasicPath}T_TPP_00000004_BC_09.png`, // 禁止通行
  giveWayToOncomingVehicles: `${signalBoardBasicPath}T_TPP_00000004_BC_10.png`, // 会车让行
  noRightTurnForMinibuses: `${signalBoardBasicPath}T_TPP_00000004_BC_12.png`, // 禁止小型客车右转
  noRightTurnForTrucks: `${signalBoardBasicPath}T_TPP_00000004_BC_13.png`, // 禁止载货汽车右转
  doNotCycleDownhill: `${signalBoardBasicPath}T_TPP_00000004_BC_14.png`, // 禁止骑自行车下坡
  doNotCycleUphill: `${signalBoardBasicPath}T_TPP_00000004_BC_15.png`, // 禁止骑自行车上坡
  endOfNoOvertakingZone: `${signalBoardBasicPath}T_TPP_00000004_BC_16.png`, // 解除禁止超车
  noLeftTurnForMinibuses: `${signalBoardBasicPath}T_TPP_00000004_BC_17.png`, // 禁止小型客车左转
  noOvertaking: `${signalBoardBasicPath}T_TPP_00000004_BC_18.png`, // 禁止超车
  noAnimalDrawnCarts: `${signalBoardBasicPath}T_TPP_00000004_BC_19.png`, // 禁止畜力车驶入
  noElectricTricycles: `${signalBoardBasicPath}T_TPP_00000004_BC_21.png`, // 禁止电动三轮车驶入
  noLeftTurnForTrucks: `${signalBoardBasicPath}T_TPP_00000004_BC_23.png`, // 禁止载货汽车左转
  noTrailers: `${signalBoardBasicPath}T_TPP_00000004_BC_24.png`, // 禁止汽车拖、挂车驶入
  noFreightVehiclesAndTractors:
    `${signalBoardBasicPath}T_TPP_00000004_BC_27.png`, // 禁止某两种车驶入
  widthLimit3: `${signalBoardBasicPath}T_TPP_00000004_BC_29.png`, // 限宽3m
  axleWeightLimit10: `${signalBoardBasicPath}T_TPP_00000004_BC_30.png`, // 限制轴重10
  customs: `${signalBoardBasicPath}T_TPP_00000004_BC_31.png`, // 海关
  stopForInspection: `${signalBoardBasicPath}T_TPP_00000004_BC_32.png`, // 停车检查
  noParking: `${signalBoardBasicPath}T_TPP_00000004_BC_33.png`, // 禁止长时停车
  endOfSpeedLimit40: `${signalBoardBasicPath}T_TPP_00000004_BC_34.png`, // 解除限速40
  noUTurn: `${signalBoardBasicPath}T_TPP_002_BC_61.png`, // 禁止掉头
  // 指示标志（大多数圆形，少量正方形）
  goStraight: `${signalBoardBasicPath}T_tpp_001_17.png`, // 直行
  minSpeed60: `${signalBoardBasicPath}T_tpp_001_19.png`, // 最低限速60
  minSpeed50: `${signalBoardBasicPath}T_TPP_00000003_BC_56.png`, // 最低限速50
  minSpeed40: `${signalBoardBasicPath}T_tpp_001_20.png`, // 最低限速40
  keepRight: `${signalBoardBasicPath}T_tpp_001_34.png`, // 靠右侧道路行驶
  keepLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_60.png`, // 靠左侧道路行驶
  pedestrianCrossing: `${signalBoardBasicPath}T_TPP_00000002_BC_07.png`, // 人行横道
  turnRight: `${signalBoardBasicPath}T_TPP_00000002_BC_28.png`, // 向右转弯
  turnLeft: `${signalBoardBasicPath}T_TPP_00000003_BC_53.png`, // 向左转弯
  roundabout: `${signalBoardBasicPath}T_TPP_00000002_BC_36.png`, // 环岛行驶
  straightOrRightTurn: `${signalBoardBasicPath}T_TPP_00000003_BC_54.png`, // 直行和向右转弯
  straightOrLeftTurn: `${signalBoardBasicPath}T_TPP_00000003_BC_55.png`, // 直行和向左转弯
  walk: `${signalBoardBasicPath}T_TPP_00000003_BC_57.png`, // 步行
  nonMotorVehiclesOnly: `${signalBoardBasicPath}T_TPP_00000003_BC_58.png`, // 非机动车行驶
  motorVehiclesOnly: `${signalBoardBasicPath}T_TPP_00000003_BC_59.png`, // 机动车行驶
  proceedStraightAndTurnRightViaRampOfInterchange:
    `${signalBoardBasicPath}T_TPP_00000003_BC_61.png`, // 立交直行和右转弯行驶
  proceedStraightAndTurnLeftViaRampOfCloverleafInterchange:
    `${signalBoardBasicPath}T_TPP_00000003_BC_62.png`, // 立交直行和左转弯行驶
  honk: `${signalBoardBasicPath}T_TPP_00000003_BC_63.png`, // 鸣喇叭
  turnLeftOrRight: `${signalBoardBasicPath}T_TPP_00000003_BC_64.png`, // 向左和向右转弯
  parkingSpace: `${signalBoardBasicPath}T_tpp_001_43.png`, // 停车位
}

const poleUrls = {
  verticalPole: `${imgPath}/pole/pole1.png`,
  horizontalPole: `${imgPath}/pole/pole2.png`,
}

// 路面标识的静态资源路径
export const ImgUrls = {
  pole: poleUrls,
  roadSign: roadSignUrls,
  signalBoard: signalBoardUrls,
  trafficLight: trafficLightUrls,
  other: otherUrls,
  sensor: sensorUrls,
}
