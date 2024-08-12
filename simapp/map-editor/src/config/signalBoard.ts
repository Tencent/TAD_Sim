import i18n from '@/locales'
import { getPermission } from '@/utils/permission'
import { ImgUrls, ModelUrls } from '@/utils/urls'

export interface ISignalBoardConfig {
  mainType: string
  name: string
  showName: string
  iconUrl: string
  textureUrl: string
  modelUrl?: string
  type: string
  subtype: string
  placementAreaName: Array<string>
}

// 指示标志
export const indicatorSignalBoard: Array<ISignalBoardConfig> = [
  {
    mainType: '',
    name: 'goStraight', // 直行
    showName: i18n.global.t('desc.editObject.goStraight'),
    iconUrl: ImgUrls.signalBoard.goStraight,
    textureUrl: ImgUrls.signalBoard.goStraight,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300100002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'minimumSpeed60', // 最低限速60
    showName: `${i18n.global.t('desc.editObject.minimumSpeed')} 60`,
    iconUrl: ImgUrls.signalBoard.minSpeed60,
    textureUrl: ImgUrls.signalBoard.minSpeed60,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010301500002413',
    subtype: '60',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'minimumSpeed50', // 最低限速50
    showName: `${i18n.global.t('desc.editObject.minimumSpeed')} 50`,
    iconUrl: ImgUrls.signalBoard.minSpeed50,
    textureUrl: ImgUrls.signalBoard.minSpeed50,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010301500002413',
    subtype: '50',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'minimumSpeed40', // 最低限速40
    showName: `${i18n.global.t('desc.editObject.minimumSpeed')} 40`,
    iconUrl: ImgUrls.signalBoard.minSpeed40,
    textureUrl: ImgUrls.signalBoard.minSpeed40,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010301500002413',
    subtype: '40',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'keepRight', // 靠右侧道路行驶
    showName: i18n.global.t('desc.editObject.keepRight'),
    iconUrl: ImgUrls.signalBoard.keepRight,
    textureUrl: ImgUrls.signalBoard.keepRight,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300700002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'keepLeft', // 靠左侧道路行驶
    showName: i18n.global.t('desc.editObject.keepLeft'),
    iconUrl: ImgUrls.signalBoard.keepLeft,
    textureUrl: ImgUrls.signalBoard.keepLeft,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300800002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'pedestrianCrossing', // 人行横道
    showName: i18n.global.t('desc.editObject.pedestrianCrossing'),
    iconUrl: ImgUrls.signalBoard.pedestrianCrossing,
    textureUrl: ImgUrls.signalBoard.pedestrianCrossing,
    modelUrl: ModelUrls.signalBoard.square,
    type: '1010301800002616',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'turnRight', // 向右转弯
    showName: i18n.global.t('desc.editObject.turnRight'),
    iconUrl: ImgUrls.signalBoard.turnRight,
    textureUrl: ImgUrls.signalBoard.turnRight,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300300002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'turnLeft', // 向左转弯
    showName: i18n.global.t('desc.editObject.turnLeft'),
    iconUrl: ImgUrls.signalBoard.turnLeft,
    textureUrl: ImgUrls.signalBoard.turnLeft,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300200002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'straightOrRightTurn', // 直行和向右转弯
    showName: i18n.global.t('desc.editObject.straightOrRightTurn'),
    iconUrl: ImgUrls.signalBoard.straightOrRightTurn,
    textureUrl: ImgUrls.signalBoard.straightOrRightTurn,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300500002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'straightOrLeftTurn', // 直行和向左转弯
    showName: i18n.global.t('desc.editObject.straightOrLeftTurn'),
    iconUrl: ImgUrls.signalBoard.straightOrLeftTurn,
    textureUrl: ImgUrls.signalBoard.straightOrLeftTurn,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300400002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'roundabout', // 环岛行驶
    showName: i18n.global.t('desc.editObject.roundabout'),
    iconUrl: ImgUrls.signalBoard.roundabout,
    textureUrl: ImgUrls.signalBoard.roundabout,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010301100002416',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'walk', // 步行
    showName: i18n.global.t('desc.editObject.walk'),
    iconUrl: ImgUrls.signalBoard.walk,
    textureUrl: ImgUrls.signalBoard.walk,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010301300002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'nonMotorVehiclesOnly', // 非机动车行驶
    showName: i18n.global.t('desc.editObject.nonMotorVehiclesOnly'),
    iconUrl: ImgUrls.signalBoard.nonMotorVehiclesOnly,
    textureUrl: ImgUrls.signalBoard.nonMotorVehiclesOnly,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010302014002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'motorVehiclesOnly', // 机动车行驶
    showName: i18n.global.t('desc.editObject.motorVehiclesOnly'),
    iconUrl: ImgUrls.signalBoard.motorVehiclesOnly,
    textureUrl: ImgUrls.signalBoard.motorVehiclesOnly,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010302012002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'proceedStraightAndTurnRightViaRampOfInterchange', // 立交直行和右转弯行驶
    showName: i18n.global.t(
      'desc.editObject.proceedStraightAndTurnRightViaRampOfInterchange',
    ),
    iconUrl:
      ImgUrls.signalBoard.proceedStraightAndTurnRightViaRampOfInterchange,
    textureUrl:
      ImgUrls.signalBoard.proceedStraightAndTurnRightViaRampOfInterchange,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010301000002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'proceedStraightAndTurnLeftViaRampOfCloverleafInterchange', // 立交直行和左转弯行驶
    showName: i18n.global.t(
      'desc.editObject.proceedStraightAndTurnLeftViaRampOfCloverleafInterchange',
    ),
    iconUrl:
      ImgUrls.signalBoard
        .proceedStraightAndTurnLeftViaRampOfCloverleafInterchange,
    textureUrl:
      ImgUrls.signalBoard
        .proceedStraightAndTurnLeftViaRampOfCloverleafInterchange,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300900002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'honk', // 鸣喇叭
    showName: i18n.global.t('desc.editObject.honk'),
    iconUrl: ImgUrls.signalBoard.honk,
    textureUrl: ImgUrls.signalBoard.honk,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010301400002413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'turnLeftOrRight', // 向左和向右转弯
    showName: i18n.global.t('desc.editObject.turnLeftOrRight'),
    iconUrl: ImgUrls.signalBoard.turnLeftOrRight,
    textureUrl: ImgUrls.signalBoard.turnLeftOrRight,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010300600002413',
    subtype: '-1',
    placementAreaName: [],
  },
  // 【停车位】指示标志
  {
    mainType: '',
    name: 'parkingSpace', // 停车位
    showName: i18n.global.t('desc.editObject.parkingSpace'),
    iconUrl: ImgUrls.signalBoard.parkingSpace,
    textureUrl: ImgUrls.signalBoard.parkingSpace,
    modelUrl: ModelUrls.signalBoard.square,
    type: '1010302111002416',
    subtype: '-1',
    placementAreaName: [],
  },
]
// 警告标志
export const warningSignalBoard: Array<ISignalBoardConfig> = [
  {
    mainType: '',
    name: 'slowDown', // 慢行
    showName: i18n.global.t('desc.editObject.slowDown'),
    iconUrl: ImgUrls.signalBoard.slowDown,
    textureUrl: ImgUrls.signalBoard.slowDown,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103200001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'sharpCurveToLeft', // 向左急转弯
    showName: i18n.global.t('desc.editObject.sharpCurveToLeft'),
    iconUrl: ImgUrls.signalBoard.sharpCurveToLeft,
    textureUrl: ImgUrls.signalBoard.sharpCurveToLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100211001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'sharpCurveToRight', // 向右急转弯
    showName: i18n.global.t('desc.editObject.sharpCurveToRight'),
    iconUrl: ImgUrls.signalBoard.sharpCurveToRight,
    textureUrl: ImgUrls.signalBoard.sharpCurveToRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100212001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'crossIntersection', // 十字交叉
    showName: i18n.global.t('desc.editObject.crossIntersection'),
    iconUrl: ImgUrls.signalBoard.crossIntersection,
    textureUrl: ImgUrls.signalBoard.crossIntersection,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100111001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'tShapedIntersection1', // T形交叉
    showName: i18n.global.t('desc.editObject.tShapedIntersection'),
    iconUrl: ImgUrls.signalBoard.tShapedIntersection1,
    textureUrl: ImgUrls.signalBoard.tShapedIntersection1,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100121001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'tShapedIntersection2', // T形交叉
    showName: i18n.global.t('desc.editObject.tShapedIntersection'),
    iconUrl: ImgUrls.signalBoard.tShapedIntersection2,
    textureUrl: ImgUrls.signalBoard.tShapedIntersection2,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100122001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'tShapedIntersection3', // T形交叉
    showName: i18n.global.t('desc.editObject.tShapedIntersection'),
    iconUrl: ImgUrls.signalBoard.tShapedIntersection3,
    textureUrl: ImgUrls.signalBoard.tShapedIntersection3,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100123001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'watchForChildren', // 注意儿童
    showName: i18n.global.t('desc.editObject.watchForChildren'),
    iconUrl: ImgUrls.signalBoard.watchForChildren,
    textureUrl: ImgUrls.signalBoard.watchForChildren,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101100001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'uphill', // 上坡路
    showName: i18n.global.t('desc.editObject.uphill'),
    iconUrl: ImgUrls.signalBoard.uphill,
    textureUrl: ImgUrls.signalBoard.uphill,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100511001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'downhill', // 下坡路
    showName: i18n.global.t('desc.editObject.downhill'),
    iconUrl: ImgUrls.signalBoard.downhill,
    textureUrl: ImgUrls.signalBoard.downhill,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100512001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'roadWorkAhead', // 施工
    showName: i18n.global.t('desc.editObject.roadWorkAhead'),
    iconUrl: ImgUrls.signalBoard.roadWorkAhead,
    textureUrl: ImgUrls.signalBoard.roadWorkAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103500001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'roadNarrowsOnLeft', // 左侧变窄
    showName: i18n.global.t('desc.editObject.roadNarrowsOnLeft'),
    iconUrl: ImgUrls.signalBoard.roadNarrowsOnLeft,
    textureUrl: ImgUrls.signalBoard.roadNarrowsOnLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100711001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'roadNarrowsOnRight', // 右侧变窄
    showName: i18n.global.t('desc.editObject.roadNarrowsOnRight'),
    iconUrl: ImgUrls.signalBoard.roadNarrowsOnRight,
    textureUrl: ImgUrls.signalBoard.roadNarrowsOnRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100712001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'roadNarrowsOnBothSides', // 两侧变窄
    showName: i18n.global.t('desc.editObject.roadNarrowsOnBothSides'),
    iconUrl: ImgUrls.signalBoard.roadNarrowsOnBothSides,
    textureUrl: ImgUrls.signalBoard.roadNarrowsOnBothSides,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100713001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'unguardedRailwayCrossing', // 无人看守铁路道口
    showName: i18n.global.t('desc.editObject.unguardedRailwayCrossing'),
    iconUrl: ImgUrls.signalBoard.unguardedRailwayCrossing,
    textureUrl: ImgUrls.signalBoard.unguardedRailwayCrossing,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102812001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'steepMountainRoadLeft', // 傍山险路-左
    showName: i18n.global.t('desc.editObject.steepMountainRoadLeft'),
    iconUrl: ImgUrls.signalBoard.steepMountainRoadLeft,
    textureUrl: ImgUrls.signalBoard.steepMountainRoadLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101811001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'steepMountainRoadRight', // 傍山险路-右
    showName: i18n.global.t('desc.editObject.steepMountainRoadRight'),
    iconUrl: ImgUrls.signalBoard.steepMountainRoadRight,
    textureUrl: ImgUrls.signalBoard.steepMountainRoadRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101812001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'village', // 村庄
    showName: i18n.global.t('desc.editObject.village'),
    iconUrl: ImgUrls.signalBoard.village,
    textureUrl: ImgUrls.signalBoard.village,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102000001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'embankmentRoadLeft', // 堤坝路-左
    showName: i18n.global.t('desc.editObject.embankmentRoadLeft'),
    iconUrl: ImgUrls.signalBoard.embankmentRoadLeft,
    textureUrl: ImgUrls.signalBoard.embankmentRoadLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101911001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'embankmentRoadRight', // 堤坝路-右
    showName: i18n.global.t('desc.editObject.embankmentRoadRight'),
    iconUrl: ImgUrls.signalBoard.embankmentRoadRight,
    textureUrl: ImgUrls.signalBoard.embankmentRoadRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101912001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'tJunctionWithDualCarriageway', // 丁字平面交叉
    showName: i18n.global.t('desc.editObject.tJunctionWithDualCarriageway'),
    iconUrl: ImgUrls.signalBoard.tJunctionWithDualCarriageway,
    textureUrl: ImgUrls.signalBoard.tJunctionWithDualCarriageway,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104012001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'ferry', // 渡口
    showName: i18n.global.t('desc.editObject.ferry'),
    iconUrl: ImgUrls.signalBoard.ferry,
    textureUrl: ImgUrls.signalBoard.ferry,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102200001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'fallingRocksLeft', // 注意落石-左
    showName: i18n.global.t('desc.editObject.fallingRocksLeft'),
    iconUrl: ImgUrls.signalBoard.fallingRocksLeft,
    textureUrl: ImgUrls.signalBoard.fallingRocksLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101511001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'fallingRocksRight', // 注意落石
    showName: i18n.global.t('desc.editObject.fallingRocksRight'),
    iconUrl: ImgUrls.signalBoard.fallingRocksRight,
    textureUrl: ImgUrls.signalBoard.fallingRocksRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101512001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'reverseCurveLeft', // 反向转弯-左
    showName: i18n.global.t('desc.editObject.reverseCurveLeft'),
    iconUrl: ImgUrls.signalBoard.reverseCurveLeft,
    textureUrl: ImgUrls.signalBoard.reverseCurveLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100311001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'reverseCurveRight', // 反向转弯-右
    showName: i18n.global.t('desc.editObject.reverseCurveRight'),
    iconUrl: ImgUrls.signalBoard.reverseCurveRight,
    textureUrl: ImgUrls.signalBoard.reverseCurveRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100312001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'lowWaterCrossing', // 过水路面
    showName: i18n.global.t('desc.editObject.lowWaterCrossing'),
    iconUrl: ImgUrls.signalBoard.lowWaterCrossing,
    textureUrl: ImgUrls.signalBoard.lowWaterCrossing,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102700001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'vehiclesMergingLeft', // 车辆汇入-左
    showName: i18n.global.t('desc.editObject.vehiclesMergingLeft'),
    iconUrl: ImgUrls.signalBoard.vehiclesMergingLeft,
    textureUrl: ImgUrls.signalBoard.vehiclesMergingLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100131001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'vehiclesMergingRight', // 车辆汇入-右
    showName: i18n.global.t('desc.editObject.vehiclesMergingRight'),
    iconUrl: ImgUrls.signalBoard.vehiclesMergingRight,
    textureUrl: ImgUrls.signalBoard.vehiclesMergingRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100132001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'circleRoad', // 环形交叉
    showName: i18n.global.t('desc.editObject.circleRoad'),
    iconUrl: ImgUrls.signalBoard.circleRoad,
    textureUrl: ImgUrls.signalBoard.circleRoad,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100141001111',
    subtype: '-1',
    placementAreaName: [],
  },
  // TODO 有一种交叉路口在没有定义 type
  {
    mainType: '',
    name: 'crossroads2', // 交叉路口
    showName: i18n.global.t('desc.editObject.crossroads'),
    iconUrl: ImgUrls.signalBoard.crossroads2,
    textureUrl: ImgUrls.signalBoard.crossroads2,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100112001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'crossroads3', // 交叉路口
    showName: i18n.global.t('desc.editObject.crossroads'),
    iconUrl: ImgUrls.signalBoard.crossroads3,
    textureUrl: ImgUrls.signalBoard.crossroads3,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100134001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'windingRoadAhead', // 连续弯路
    showName: i18n.global.t('desc.editObject.windingRoadAhead'),
    iconUrl: ImgUrls.signalBoard.windingRoadAhead,
    textureUrl: ImgUrls.signalBoard.windingRoadAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100400001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'longDescent', // 连续下坡
    showName: i18n.global.t('desc.editObject.longDescent'),
    iconUrl: ImgUrls.signalBoard.longDescent,
    textureUrl: ImgUrls.signalBoard.longDescent,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100600001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'roughRoadAhead', // 路面不平
    showName: i18n.global.t('desc.editObject.roughRoadAhead'),
    iconUrl: ImgUrls.signalBoard.roughRoadAhead,
    textureUrl: ImgUrls.signalBoard.roughRoadAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102400001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'driveCarefullyInRainOrSnow', // 注意雨雪天气
    showName: i18n.global.t('desc.editObject.driveCarefullyInRainOrSnow'),
    iconUrl: ImgUrls.signalBoard.driveCarefullyInRainOrSnow,
    textureUrl: ImgUrls.signalBoard.driveCarefullyInRainOrSnow,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104312001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'yieldToPeopleWithDisabilities', // 注意残疾人
    showName: i18n.global.t('desc.editObject.yieldToPeopleWithDisabilities'),
    iconUrl: ImgUrls.signalBoard.yieldToPeopleWithDisabilities,
    textureUrl: ImgUrls.signalBoard.yieldToPeopleWithDisabilities,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103000001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'watchForWildAnimals', // 注意野生动物
    showName: i18n.global.t('desc.editObject.watchForWildAnimals'),
    iconUrl: ImgUrls.signalBoard.watchForWildAnimals,
    textureUrl: ImgUrls.signalBoard.watchForWildAnimals,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101300001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'accidentArea', // 事故易发路段
    showName: i18n.global.t('desc.editObject.accidentArea'),
    iconUrl: ImgUrls.signalBoard.accidentArea,
    textureUrl: ImgUrls.signalBoard.accidentArea,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103100001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'reversibleLaneAhead', // 注意潮汐车道
    showName: i18n.global.t('desc.editObject.reversibleLaneAhead'),
    iconUrl: ImgUrls.signalBoard.reversibleLaneAhead,
    textureUrl: ImgUrls.signalBoard.reversibleLaneAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103800001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'driveCarefullyInAdverseWeatherConditions', // 注意不利气象条件
    showName: i18n.global.t(
      'desc.editObject.driveCarefullyInAdverseWeatherConditions',
    ),
    iconUrl: ImgUrls.signalBoard.driveCarefullyInAdverseWeatherConditions,
    textureUrl: ImgUrls.signalBoard.driveCarefullyInAdverseWeatherConditions,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104314001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'lowLyingRoadAhead', // 路面低洼
    showName: i18n.global.t('desc.editObject.lowLyingRoadAhead'),
    iconUrl: ImgUrls.signalBoard.lowLyingRoadAhead,
    textureUrl: ImgUrls.signalBoard.lowLyingRoadAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102600001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'bumpyRoadAhead', // 路面高凸
    showName: i18n.global.t('desc.editObject.bumpyRoadAhead'),
    iconUrl: ImgUrls.signalBoard.bumpyRoadAhead,
    textureUrl: ImgUrls.signalBoard.bumpyRoadAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102500001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'queuesLikelyAhead', // 注意前方车辆排队
    showName: i18n.global.t('desc.editObject.queuesLikelyAhead'),
    iconUrl: ImgUrls.signalBoard.queuesLikelyAhead,
    textureUrl: ImgUrls.signalBoard.queuesLikelyAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104400001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'crossroadsWithDualCarriageway', // 十字平面交叉
    showName: i18n.global.t('desc.editObject.crossroadsWithDualCarriageway'),
    iconUrl: ImgUrls.signalBoard.crossroadsWithDualCarriageway,
    textureUrl: ImgUrls.signalBoard.crossroadsWithDualCarriageway,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104011001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'tunnel', // 隧道
    showName: i18n.global.t('desc.editObject.tunnel'),
    iconUrl: ImgUrls.signalBoard.tunnel,
    textureUrl: ImgUrls.signalBoard.tunnel,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102100001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'turnOnHeadlightsBeforeEnteringTunnel', // 隧道开车灯
    showName: i18n.global.t(
      'desc.editObject.turnOnHeadlightsBeforeEnteringTunnel',
    ),
    iconUrl: ImgUrls.signalBoard.turnOnHeadlightsBeforeEnteringTunnel,
    textureUrl: ImgUrls.signalBoard.turnOnHeadlightsBeforeEnteringTunnel,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103700001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'camelBackBridge', // 驼峰桥
    showName: i18n.global.t('desc.editObject.camelBackBridge'),
    iconUrl: ImgUrls.signalBoard.camelBackBridge,
    textureUrl: ImgUrls.signalBoard.camelBackBridge,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102300001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'watchForNonMotorVehicles', // 注意非机动车
    showName: i18n.global.t('desc.editObject.watchForNonMotorVehicles'),
    iconUrl: ImgUrls.signalBoard.watchForNonMotorVehicles,
    textureUrl: ImgUrls.signalBoard.watchForNonMotorVehicles,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010102900001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'slipperySurface', // 易滑
    showName: i18n.global.t('desc.editObject.slipperySurface'),
    iconUrl: ImgUrls.signalBoard.slipperySurface,
    textureUrl: ImgUrls.signalBoard.slipperySurface,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101700001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'trafficLightsAhead', // 注意信号灯
    showName: i18n.global.t('desc.editObject.trafficLightsAhead'),
    iconUrl: ImgUrls.signalBoard.trafficLightsAhead,
    textureUrl: ImgUrls.signalBoard.trafficLightsAhead,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101400001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'detourLeft', // 左侧绕行
    showName: i18n.global.t('desc.editObject.detourLeft'),
    iconUrl: ImgUrls.signalBoard.detourLeft,
    textureUrl: ImgUrls.signalBoard.detourLeft,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103312001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'detourRight', // 右侧绕行
    showName: i18n.global.t('desc.editObject.detourRight'),
    iconUrl: ImgUrls.signalBoard.detourRight,
    textureUrl: ImgUrls.signalBoard.detourRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103313001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'detourAheadLeftOrRight', // 左右绕行
    showName: i18n.global.t('desc.editObject.detourAheadLeftOrRight'),
    iconUrl: ImgUrls.signalBoard.detourAheadLeftOrRight,
    textureUrl: ImgUrls.signalBoard.detourAheadLeftOrRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103311001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'narrowBridge', // 窄桥
    showName: i18n.global.t('desc.editObject.narrowBridge'),
    iconUrl: ImgUrls.signalBoard.narrowBridge,
    textureUrl: ImgUrls.signalBoard.narrowBridge,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100800001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'keepDistance', // 注意保持车距
    showName: i18n.global.t('desc.editObject.keepDistance'),
    iconUrl: ImgUrls.signalBoard.keepDistance,
    textureUrl: ImgUrls.signalBoard.keepDistance,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103900001111',
    subtype: '-1',
    placementAreaName: [],
  },
  // TODO 没有注意合流-左的 type
  {
    mainType: '',
    name: 'roadsMergeRight', // 注意合流-右
    showName: i18n.global.t('desc.editObject.roadsMerge'),
    iconUrl: ImgUrls.signalBoard.roadsMergeRight,
    textureUrl: ImgUrls.signalBoard.roadsMergeRight,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104111001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'bewareOfCrosswind', // 注意横风
    showName: i18n.global.t('desc.editObject.bewareOfCrosswind'),
    iconUrl: ImgUrls.signalBoard.bewareOfCrosswind,
    textureUrl: ImgUrls.signalBoard.bewareOfCrosswind,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101600001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'icyRoads', // 注意路面结冰
    showName: i18n.global.t('desc.editObject.icyRoads'),
    iconUrl: ImgUrls.signalBoard.icyRoads,
    textureUrl: ImgUrls.signalBoard.icyRoads,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104311001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'driveWithCaution', // 注意危险
    showName: i18n.global.t('desc.editObject.driveWithCaution'),
    iconUrl: ImgUrls.signalBoard.driveWithCaution,
    textureUrl: ImgUrls.signalBoard.driveWithCaution,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010103400001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'driveCarefullyInFoggyWeather', // 注意雾天
    showName: i18n.global.t('desc.editObject.driveCarefullyInFoggyWeather'),
    iconUrl: ImgUrls.signalBoard.driveCarefullyInFoggyWeather,
    textureUrl: ImgUrls.signalBoard.driveCarefullyInFoggyWeather,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010104313001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'watchForLivestock', // 注意牲畜
    showName: i18n.global.t('desc.editObject.watchForLivestock'),
    iconUrl: ImgUrls.signalBoard.watchForLivestock,
    textureUrl: ImgUrls.signalBoard.watchForLivestock,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101200001111',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'twoWayTraffic', // 双向交通
    showName: i18n.global.t('desc.editObject.twoWayTraffic'),
    iconUrl: ImgUrls.signalBoard.twoWayTraffic,
    textureUrl: ImgUrls.signalBoard.twoWayTraffic,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010100900001111',
    subtype: '-1',
    placementAreaName: [],
  },
  // 【注意行人】的标志牌
  {
    mainType: '',
    name: 'watchForPedestrians', // 注意行人
    showName: i18n.global.t('desc.editObject.watchForPedestrians'),
    iconUrl: ImgUrls.signalBoard.watchForPedestrians,
    textureUrl: ImgUrls.signalBoard.watchForPedestrians,
    modelUrl: ModelUrls.signalBoard.triangle,
    type: '1010101000001111',
    subtype: '-1',
    placementAreaName: [],
  },
]
// 禁止标志
export const prohibitionSignalBoard: Array<ISignalBoardConfig> = [
  {
    mainType: '',
    name: 'noStraightThru', // 禁止直行
    showName: i18n.global.t('desc.editObject.noStraightThru'),
    iconUrl: ImgUrls.signalBoard.noStraightThru,
    textureUrl: ImgUrls.signalBoard.noStraightThru,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202400001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noMotorVehicles', // 禁止机动车驶入
    showName: i18n.global.t('desc.editObject.noMotorVehicles'),
    iconUrl: ImgUrls.signalBoard.noMotorVehicles,
    textureUrl: ImgUrls.signalBoard.noMotorVehicles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010200600001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit120', // 限速120
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 120`,
    iconUrl: ImgUrls.signalBoard.speedLimit120,
    textureUrl: ImgUrls.signalBoard.speedLimit120,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '120',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit100', // 限速100
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 100`,
    iconUrl: ImgUrls.signalBoard.speedLimit100,
    textureUrl: ImgUrls.signalBoard.speedLimit100,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '100',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit80', // 限速80
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 80`,
    iconUrl: ImgUrls.signalBoard.speedLimit80,
    textureUrl: ImgUrls.signalBoard.speedLimit80,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '80',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit70', // 限速70
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 70`,
    iconUrl: ImgUrls.signalBoard.speedLimit70,
    textureUrl: ImgUrls.signalBoard.speedLimit70,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '70',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit60', // 限速60
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 60`,
    iconUrl: ImgUrls.signalBoard.speedLimit60,
    textureUrl: ImgUrls.signalBoard.speedLimit60,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '60',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit50', // 限速50
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 50`,
    iconUrl: ImgUrls.signalBoard.speedLimit50,
    textureUrl: ImgUrls.signalBoard.speedLimit50,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '50',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit40', // 限速40
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 40`,
    iconUrl: ImgUrls.signalBoard.speedLimit40,
    textureUrl: ImgUrls.signalBoard.speedLimit40,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '40',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit30', // 限速30
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 30`,
    iconUrl: ImgUrls.signalBoard.speedLimit30,
    textureUrl: ImgUrls.signalBoard.speedLimit30,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '30',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit20', // 限速20
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 20`,
    iconUrl: ImgUrls.signalBoard.speedLimit20,
    textureUrl: ImgUrls.signalBoard.speedLimit20,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '20',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'speedLimit5', // 限速5
    showName: `${i18n.global.t('desc.editObject.speedLimit')} 5`,
    iconUrl: ImgUrls.signalBoard.speedLimit5,
    textureUrl: ImgUrls.signalBoard.speedLimit5,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203800001413',
    subtype: '5',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'stop', // 停车让行
    showName: i18n.global.t('desc.editObject.stop'),
    iconUrl: ImgUrls.signalBoard.stop,
    textureUrl: ImgUrls.signalBoard.stop,
    modelUrl: ModelUrls.signalBoard.octagon, // 八角形
    type: '1010200100001914',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'heightLimit5', // 限高5m
    showName:
      `${i18n.global.t('desc.editObject.heightLimit')
      } 5 ${
      i18n.global.t('desc.editObject.m')}`,
    iconUrl: ImgUrls.signalBoard.heightLimit5,
    textureUrl: ImgUrls.signalBoard.heightLimit5,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203500001413',
    subtype: '5',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'heightLimit3dot5', // 限高3.5m
    showName:
      `${i18n.global.t('desc.editObject.heightLimit')
      } 3.5 ${
      i18n.global.t('desc.editObject.m')}`,
    iconUrl: ImgUrls.signalBoard.heightLimit3dot5,
    textureUrl: ImgUrls.signalBoard.heightLimit3dot5,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203500001413',
    subtype: '3.5',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noEntryForVehicularTraffic', // 禁止驶入
    showName: i18n.global.t('desc.editObject.noEntryForVehicularTraffic'),
    iconUrl: ImgUrls.signalBoard.noEntryForVehicularTraffic,
    textureUrl: ImgUrls.signalBoard.noEntryForVehicularTraffic,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010200500001513',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noMotorcycles', // 禁止二轮摩托车驶入
    showName: i18n.global.t('desc.editObject.noMotorcycles'),
    iconUrl: ImgUrls.signalBoard.noMotorcycles,
    textureUrl: ImgUrls.signalBoard.noMotorcycles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201400001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'weightLimit55', // 限重55吨
    showName:
      `${i18n.global.t('desc.editObject.weightLimit')
      } 55 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.weightLimit55,
    textureUrl: ImgUrls.signalBoard.weightLimit55,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203600001413',
    subtype: '55',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'weightLimit40', // 限重40吨
    showName:
      `${i18n.global.t('desc.editObject.weightLimit')
      } 40 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.weightLimit40,
    textureUrl: ImgUrls.signalBoard.weightLimit40,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203600001413',
    subtype: '40',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'weightLimit30', // 限重30吨
    showName:
      `${i18n.global.t('desc.editObject.weightLimit')
      } 30 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.weightLimit30,
    textureUrl: ImgUrls.signalBoard.weightLimit30,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203600001413',
    subtype: '30',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'weightLimit20', // 限重20吨
    showName:
      `${i18n.global.t('desc.editObject.weightLimit')
      } 20 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.weightLimit20,
    textureUrl: ImgUrls.signalBoard.weightLimit20,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203600001413',
    subtype: '20',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'weightLimit10', // 限重10吨
    showName:
      `${i18n.global.t('desc.editObject.weightLimit')
      } 10 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.weightLimit10,
    textureUrl: ImgUrls.signalBoard.weightLimit10,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203600001413',
    subtype: '10',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noHonking', // 禁止鸣喇叭
    showName: i18n.global.t('desc.editObject.noHonking'),
    iconUrl: ImgUrls.signalBoard.noHonking,
    textureUrl: ImgUrls.signalBoard.noHonking,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203300001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noTrucks', // 禁止载货汽车驶入
    showName: i18n.global.t('desc.editObject.noTrucks'),
    iconUrl: ImgUrls.signalBoard.noTrucks,
    textureUrl: ImgUrls.signalBoard.noTrucks,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010200700001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noStopping', // 禁止停车
    showName: i18n.global.t('desc.editObject.noStopping'),
    iconUrl: ImgUrls.signalBoard.noStopping,
    textureUrl: ImgUrls.signalBoard.noStopping,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203111001713',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'axleWeightLimit14', // 限制轴重14吨
    showName:
      `${i18n.global.t('desc.editObject.axleWeightLimit')
      } 14 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.axleWeightLimit14,
    textureUrl: ImgUrls.signalBoard.axleWeightLimit14,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203700001413',
    subtype: '14',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'axleWeightLimit13', // 限制轴重13吨
    showName:
      `${i18n.global.t('desc.editObject.axleWeightLimit')
      } 13 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.axleWeightLimit13,
    textureUrl: ImgUrls.signalBoard.axleWeightLimit13,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203700001413',
    subtype: '13',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'axleWeightLimit10', // 限制轴重10吨
    showName:
      `${i18n.global.t('desc.editObject.axleWeightLimit')
      } 10 ${
      i18n.global.t('desc.editObject.t')}`,
    iconUrl: ImgUrls.signalBoard.axleWeightLimit10,
    textureUrl: ImgUrls.signalBoard.axleWeightLimit10,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203700001413',
    subtype: '10',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'slowDownAndGiveWay', // 减速让行
    showName: i18n.global.t('desc.editObject.slowDownAndGiveWay'),
    iconUrl: ImgUrls.signalBoard.slowDownAndGiveWay,
    textureUrl: ImgUrls.signalBoard.slowDownAndGiveWay,
    modelUrl: ModelUrls.signalBoard.invertedTriangle, // 倒三角
    type: '1010200200002012',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noLeftTurn', // 禁止向左转弯
    showName: i18n.global.t('desc.editObject.noLeftTurn'),
    iconUrl: ImgUrls.signalBoard.noLeftTurn,
    textureUrl: ImgUrls.signalBoard.noLeftTurn,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202211001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noRightTurn', // 禁止向右转弯
    showName: i18n.global.t('desc.editObject.noRightTurn'),
    iconUrl: ImgUrls.signalBoard.noRightTurn,
    textureUrl: ImgUrls.signalBoard.noRightTurn,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202311001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noTurns', // 禁止向左向右转弯
    showName: i18n.global.t('desc.editObject.noTurns'),
    iconUrl: ImgUrls.signalBoard.noTurns,
    textureUrl: ImgUrls.signalBoard.noTurns,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202500001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noStraightThruOrLeftTurn', // 禁止直行和向左转弯
    showName: i18n.global.t('desc.editObject.noStraightThruOrLeftTurn'),
    iconUrl: ImgUrls.signalBoard.noStraightThruOrLeftTurn,
    textureUrl: ImgUrls.signalBoard.noStraightThruOrLeftTurn,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202600001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noStraightThruOrRightTurn', // 禁止直行和向右转弯
    showName: i18n.global.t('desc.editObject.noStraightThruOrRightTurn'),
    iconUrl: ImgUrls.signalBoard.noStraightThruOrRightTurn,
    textureUrl: ImgUrls.signalBoard.noStraightThruOrRightTurn,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202700001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'vehiclesCarryingHazardousMaterialsProhibited', // 禁止运输危险物品车辆驶入
    showName: i18n.global.t(
      'desc.editObject.vehiclesCarryingHazardousMaterialsProhibited',
    ),
    iconUrl: ImgUrls.signalBoard.vehiclesCarryingHazardousMaterialsProhibited,
    textureUrl:
      ImgUrls.signalBoard.vehiclesCarryingHazardousMaterialsProhibited,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010204100001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noTractors', // 禁止拖拉机驶入
    showName: i18n.global.t('desc.editObject.noTractors'),
    iconUrl: ImgUrls.signalBoard.noTractors,
    textureUrl: ImgUrls.signalBoard.noTractors,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201200001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noMotorTricycles', // 禁止三轮车机动车驶入
    showName: i18n.global.t('desc.editObject.noMotorTricycles'),
    iconUrl: ImgUrls.signalBoard.noMotorTricycles,
    textureUrl: ImgUrls.signalBoard.noMotorTricycles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201300001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noMinibuses', // 禁止小型客车驶入
    showName: i18n.global.t('desc.editObject.noMinibuses'),
    iconUrl: ImgUrls.signalBoard.noMinibuses,
    textureUrl: ImgUrls.signalBoard.noMinibuses,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201000001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noLargeBuses', // 禁止大型客车驶入
    showName: i18n.global.t('desc.editObject.noLargeBuses'),
    iconUrl: ImgUrls.signalBoard.noLargeBuses,
    textureUrl: ImgUrls.signalBoard.noLargeBuses,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010200900001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noHandcartsOrTricycles', // 禁止人力车驶入
    showName: i18n.global.t('desc.editObject.noHandcartsOrTricycles'),
    iconUrl: ImgUrls.signalBoard.noHandcartsOrTricycles,
    textureUrl: ImgUrls.signalBoard.noHandcartsOrTricycles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202000001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noHandcartsOrFreightTricycles', // 禁止人力货运三轮车驶入
    showName: i18n.global.t('desc.editObject.noHandcartsOrFreightTricycles'),
    iconUrl: ImgUrls.signalBoard.noHandcartsOrFreightTricycles,
    textureUrl: ImgUrls.signalBoard.noHandcartsOrFreightTricycles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201800001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noPassengerTricycles', // 禁止人力客运三轮车驶入
    showName: i18n.global.t('desc.editObject.noPassengerTricycles'),
    iconUrl: ImgUrls.signalBoard.noPassengerTricycles,
    textureUrl: ImgUrls.signalBoard.noPassengerTricycles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201900001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noEntry', // 禁止通行
    showName: i18n.global.t('desc.editObject.noEntry'),
    iconUrl: ImgUrls.signalBoard.noEntry,
    textureUrl: ImgUrls.signalBoard.noEntry,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010200400001213',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'giveWayToOncomingVehicles', // 会车让行
    showName: i18n.global.t('desc.editObject.giveWayToOncomingVehicles'),
    iconUrl: ImgUrls.signalBoard.giveWayToOncomingVehicles,
    textureUrl: ImgUrls.signalBoard.giveWayToOncomingVehicles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010200300002113',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noLeftTurnForMinibuses', // 禁止小客车向左转弯
    showName: i18n.global.t('desc.editObject.noLeftTurnForMinibuses'),
    iconUrl: ImgUrls.signalBoard.noLeftTurnForMinibuses,
    textureUrl: ImgUrls.signalBoard.noLeftTurnForMinibuses,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202212001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noRightTurnForMinibuses', // 禁止小客车向右转弯
    showName: i18n.global.t('desc.editObject.noRightTurnForMinibuses'),
    iconUrl: ImgUrls.signalBoard.noRightTurnForMinibuses,
    textureUrl: ImgUrls.signalBoard.noRightTurnForMinibuses,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202312001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noLeftTurnForTrucks', // 禁止载货汽车向左转弯
    showName: i18n.global.t('desc.editObject.noLeftTurnForTrucks'),
    iconUrl: ImgUrls.signalBoard.noLeftTurnForTrucks,
    textureUrl: ImgUrls.signalBoard.noLeftTurnForTrucks,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202215001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noRightTurnForTrucks', // 禁止载货汽车向右转弯
    showName: i18n.global.t('desc.editObject.noRightTurnForTrucks'),
    iconUrl: ImgUrls.signalBoard.noRightTurnForTrucks,
    textureUrl: ImgUrls.signalBoard.noRightTurnForTrucks,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202315001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noOvertaking', // 禁止超车
    showName: i18n.global.t('desc.editObject.noOvertaking'),
    iconUrl: ImgUrls.signalBoard.noOvertaking,
    textureUrl: ImgUrls.signalBoard.noOvertaking,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202900001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'endOfNoOvertakingZone', // 解除禁止超车
    showName: i18n.global.t('desc.editObject.endOfNoOvertakingZone'),
    iconUrl: ImgUrls.signalBoard.endOfNoOvertakingZone,
    textureUrl: ImgUrls.signalBoard.endOfNoOvertakingZone,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203000001613',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noAnimalDrawnCarts', // 禁止畜力车进入
    showName: i18n.global.t('desc.editObject.noAnimalDrawnCarts'),
    iconUrl: ImgUrls.signalBoard.noAnimalDrawnCarts,
    textureUrl: ImgUrls.signalBoard.noAnimalDrawnCarts,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201700001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noElectricTricycles', // 禁止电动三轮车驶入
    showName: i18n.global.t('desc.editObject.noElectricTricycles'),
    iconUrl: ImgUrls.signalBoard.noElectricTricycles,
    textureUrl: ImgUrls.signalBoard.noElectricTricycles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010200800001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noEntryForNonMotorVehicles', // 禁止非机动车驶入
    showName: i18n.global.t('desc.editObject.noEntryForNonMotorVehicles'),
    iconUrl: ImgUrls.signalBoard.noEntryForNonMotorVehicles,
    textureUrl: ImgUrls.signalBoard.noEntryForNonMotorVehicles,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201600001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noTrailers', // 禁止汽车拖、挂车驶入
    showName: i18n.global.t('desc.editObject.noTrailers'),
    iconUrl: ImgUrls.signalBoard.noTrailers,
    textureUrl: ImgUrls.signalBoard.noTrailers,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201100001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noPedestrians', // 禁止行人进入
    showName: i18n.global.t('desc.editObject.noPedestrians'),
    iconUrl: ImgUrls.signalBoard.noPedestrians,
    textureUrl: ImgUrls.signalBoard.noPedestrians,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202100001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noFreightVehiclesAndTractors', // 禁止某两种车驶入
    showName: i18n.global.t('desc.editObject.noFreightVehiclesAndTractors'),
    iconUrl: ImgUrls.signalBoard.noFreightVehiclesAndTractors,
    textureUrl: ImgUrls.signalBoard.noFreightVehiclesAndTractors,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010201500001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'widthLimit3', // 限制宽度3m
    showName:
      `${i18n.global.t('desc.editObject.widthLimit')
      } 3 ${
      i18n.global.t('desc.editObject.m')}`,
    iconUrl: ImgUrls.signalBoard.widthLimit3,
    textureUrl: ImgUrls.signalBoard.widthLimit3,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203400001413',
    subtype: '3',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'customs', // 海关
    showName: i18n.global.t('desc.editObject.customs'),
    iconUrl: ImgUrls.signalBoard.customs,
    textureUrl: ImgUrls.signalBoard.customs,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010204200001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'stopForInspection', // 停车检查
    showName: i18n.global.t('desc.editObject.stopForInspection'),
    iconUrl: ImgUrls.signalBoard.stopForInspection,
    textureUrl: ImgUrls.signalBoard.stopForInspection,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010204000001413',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'noParking', // 禁止车辆长时停放
    showName: i18n.global.t('desc.editObject.noParking'),
    iconUrl: ImgUrls.signalBoard.noParking,
    textureUrl: ImgUrls.signalBoard.noParking,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203200001713',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'endOfSpeedLimit40', // 解除限制速度 40
    showName: `${i18n.global.t('desc.editObject.endOfSpeedLimit')} 40`,
    iconUrl: ImgUrls.signalBoard.endOfSpeedLimit40,
    textureUrl: ImgUrls.signalBoard.endOfSpeedLimit40,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010203900001613',
    subtype: '-1',
    placementAreaName: [],
  },
  // 【禁止掉头】
  {
    mainType: '',
    name: 'noUTurn', // 禁止掉头
    showName: i18n.global.t('desc.editObject.noUTurn'),
    iconUrl: ImgUrls.signalBoard.noUTurn,
    textureUrl: ImgUrls.signalBoard.noUTurn,
    modelUrl: ModelUrls.signalBoard.circle,
    type: '1010202800001413',
    subtype: '-1',
    placementAreaName: [],
  },
]

export const _signalBoardConfig = [
  {
    name: i18n.global.t('desc.editObject.indicatorSign'),
    type: 'indicatorSignalBoard',
    list: indicatorSignalBoard,
  },
  {
    name: i18n.global.t('desc.editObject.warningSign'),
    type: 'warningSignalBoard',
    list: warningSignalBoard,
  },
  {
    name: i18n.global.t('desc.editObject.prohibitionSign'),
    type: 'prohibitionSignalBoard',
    list: prohibitionSignalBoard,
  },
]
export const signalBoardConfig = _signalBoardConfig.filter((item) => {
  if (item.type === 'indicatorSignalBoard') {
    return getPermission(
      'action.mapEditor.models.signalBoard.indicator.view.enable',
    )
  }
  if (item.type === 'warningSignalBoard') {
    return getPermission(
      'action.mapEditor.models.signalBoard.warning.view.enable',
    )
  }
  if (item.type === 'prohibitionSignalBoard') {
    return getPermission(
      'action.mapEditor.models.signalBoard.prohibition.view.enable',
    )
  }
  return true
})

// signal 类型通过 type 和 subtype 获取配置
export function getSignalBoardConfig (type: string, subtype: string) {
  for (const signalBoardType of signalBoardConfig) {
    for (const option of signalBoardType.list) {
      if (option.type === type && option.subtype === subtype) {
        return option
      }
    }
  }
  return null
}

// 初始化预设的参数
function initPresetConfig () {
  for (const signalType of signalBoardConfig) {
    for (const signalOption of signalType.list) {
      signalOption.mainType = 'signalBoard'
      signalOption.placementAreaName = [
        'horizontalPole',
        'verticalPole',
        'horizontalPolePlacementArea',
        'verticalPolePlacementArea',
      ]
    }
  }
}
initPresetConfig()
