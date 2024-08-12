import { ImgUrls, ModelUrls } from '../utils/urls'

let signalBoardConfig = []

export function initSignalBoardConfig () {
  // 标志牌【指示类型】
  const indicatorSignalBoard = [
    {
      mainType: 'signalBoard',
      name: 'goStraight', // 直行
      textureUrl: ImgUrls.signalBoard.goStraight,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300100002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'minimumSpeed60', // 最低限速60
      textureUrl: ImgUrls.signalBoard.minSpeed60,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010301500002413',
      subtype: '60',
    },
    {
      mainType: 'signalBoard',
      name: 'minimumSpeed50', // 最低限速50
      textureUrl: ImgUrls.signalBoard.minSpeed50,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010301500002413',
      subtype: '50',
    },
    {
      mainType: 'signalBoard',
      name: 'minimumSpeed40', // 最低限速40
      textureUrl: ImgUrls.signalBoard.minSpeed40,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010301500002413',
      subtype: '40',
    },
    {
      mainType: 'signalBoard',
      name: 'keepRight', // 靠右侧道路行驶
      textureUrl: ImgUrls.signalBoard.keepRight,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300700002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'keepLeft', // 靠左侧道路行驶
      textureUrl: ImgUrls.signalBoard.keepLeft,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300800002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'pedestrianCrossing', // 人行横道
      textureUrl: ImgUrls.signalBoard.pedestrianCrossing,
      modelUrl: ModelUrls.signalBoard.square,
      type: '1010301800002616',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'turnRight', // 向右转弯
      textureUrl: ImgUrls.signalBoard.turnRight,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300300002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'turnLeft', // 向左转弯
      textureUrl: ImgUrls.signalBoard.turnLeft,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300200002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'straightOrRightTurn', // 直行和向右转弯
      textureUrl: ImgUrls.signalBoard.straightOrRightTurn,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300500002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'straightOrLeftTurn', // 直行和向左转弯
      textureUrl: ImgUrls.signalBoard.straightOrLeftTurn,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300400002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'roundabout', // 环岛行驶
      textureUrl: ImgUrls.signalBoard.roundabout,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010301100002416',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'walk', // 步行
      textureUrl: ImgUrls.signalBoard.walk,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010301300002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'nonMotorVehiclesOnly', // 非机动车行驶
      textureUrl: ImgUrls.signalBoard.nonMotorVehiclesOnly,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010302014002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'motorVehiclesOnly', // 机动车行驶
      textureUrl: ImgUrls.signalBoard.motorVehiclesOnly,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010302012002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'proceedStraightAndTurnRightViaRampOfInterchange', // 立交直行和右转弯行驶
      textureUrl:
        ImgUrls.signalBoard.proceedStraightAndTurnRightViaRampOfInterchange,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010301000002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'proceedStraightAndTurnLeftViaRampOfCloverleafInterchange', // 立交直行和左转弯行驶
      textureUrl:
        ImgUrls.signalBoard
          .proceedStraightAndTurnLeftViaRampOfCloverleafInterchange,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300900002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'honk', // 鸣喇叭
      textureUrl: ImgUrls.signalBoard.honk,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010301400002413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'turnLeftOrRight', // 向左和向右转弯
      textureUrl: ImgUrls.signalBoard.turnLeftOrRight,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010300600002413',
      subtype: '-1',
    },
    {
      mainType: '',
      name: 'parkingSpace', // 停车位
      textureUrl: ImgUrls.signalBoard.parkingSpace,
      modelUrl: ModelUrls.signalBoard.square,
      type: '1010302111002416',
      subtype: '-1',
    },
  ]
  // 标志牌【警告类型】
  const warningSignalBoard = [
    {
      mainType: 'signalBoard',
      name: 'slowDown', // 慢行
      textureUrl: ImgUrls.signalBoard.slowDown,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103200001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'sharpCurveToLeft', // 向左急转弯
      textureUrl: ImgUrls.signalBoard.sharpCurveToLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100211001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'sharpCurveToRight', // 向右急转弯
      textureUrl: ImgUrls.signalBoard.sharpCurveToRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100212001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'crossIntersection', // 十字交叉
      textureUrl: ImgUrls.signalBoard.crossIntersection,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100111001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'tShapedIntersection1', // T形交叉
      textureUrl: ImgUrls.signalBoard.tShapedIntersection1,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100121001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'tShapedIntersection2', // T形交叉
      textureUrl: ImgUrls.signalBoard.tShapedIntersection2,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100122001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'tShapedIntersection3', // T形交叉
      textureUrl: ImgUrls.signalBoard.tShapedIntersection3,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100123001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'watchForChildren', // 注意儿童
      textureUrl: ImgUrls.signalBoard.watchForChildren,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101100001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'uphill', // 上坡路
      textureUrl: ImgUrls.signalBoard.uphill,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100511001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'downhill', // 下坡路
      textureUrl: ImgUrls.signalBoard.downhill,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100512001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'roadWorkAhead', // 施工
      textureUrl: ImgUrls.signalBoard.roadWorkAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103500001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'roadNarrowsOnLeft', // 左侧变窄
      textureUrl: ImgUrls.signalBoard.roadNarrowsOnLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100711001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'roadNarrowsOnRight', // 右侧变窄
      textureUrl: ImgUrls.signalBoard.roadNarrowsOnRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100712001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'roadNarrowsOnBothSides', // 两侧变窄
      textureUrl: ImgUrls.signalBoard.roadNarrowsOnBothSides,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100713001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'unguardedRailwayCrossing', // 无人看守铁路道口
      textureUrl: ImgUrls.signalBoard.unguardedRailwayCrossing,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102812001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'steepMountainRoadLeft', // 傍山险路-左
      textureUrl: ImgUrls.signalBoard.steepMountainRoadLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101811001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'steepMountainRoadRight', // 傍山险路-右
      textureUrl: ImgUrls.signalBoard.steepMountainRoadRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101812001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'village', // 村庄
      textureUrl: ImgUrls.signalBoard.village,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102000001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'embankmentRoadLeft', // 堤坝路-左
      textureUrl: ImgUrls.signalBoard.embankmentRoadLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101911001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'embankmentRoadRight', // 堤坝路-右
      textureUrl: ImgUrls.signalBoard.embankmentRoadRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101912001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'tJunctionWithDualCarriageway', // 丁字平面交叉
      textureUrl: ImgUrls.signalBoard.tJunctionWithDualCarriageway,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104012001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'ferry', // 渡口
      textureUrl: ImgUrls.signalBoard.ferry,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102200001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'fallingRocksLeft', // 注意落石-左
      textureUrl: ImgUrls.signalBoard.fallingRocksLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101511001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'fallingRocksRight', // 注意落石
      textureUrl: ImgUrls.signalBoard.fallingRocksRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101512001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'reverseCurveLeft', // 反向转弯-左
      textureUrl: ImgUrls.signalBoard.reverseCurveLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100311001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'reverseCurveRight', // 反向转弯-右
      textureUrl: ImgUrls.signalBoard.reverseCurveRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100312001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'lowWaterCrossing', // 过水路面
      textureUrl: ImgUrls.signalBoard.lowWaterCrossing,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102700001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'vehiclesMergingLeft', // 车辆汇入-左
      textureUrl: ImgUrls.signalBoard.vehiclesMergingLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100131001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'vehiclesMergingRight', // 车辆汇入-右
      textureUrl: ImgUrls.signalBoard.vehiclesMergingRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100132001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'circleRoad', // 环形交叉
      textureUrl: ImgUrls.signalBoard.circleRoad,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100141001111',
      subtype: '-1',
    },
    // TODO 有一种交叉路口在没有定义 type
    {
      mainType: 'signalBoard',
      name: 'crossroads2', // 交叉路口
      textureUrl: ImgUrls.signalBoard.crossroads2,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100112001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'crossroads3', // 交叉路口
      textureUrl: ImgUrls.signalBoard.crossroads3,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100134001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'windingRoadAhead', // 连续弯路
      textureUrl: ImgUrls.signalBoard.windingRoadAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100400001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'longDescent', // 连续下坡
      textureUrl: ImgUrls.signalBoard.longDescent,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100600001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'roughRoadAhead', // 路面不平
      textureUrl: ImgUrls.signalBoard.roughRoadAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102400001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'driveCarefullyInRainOrSnow', // 注意雨雪天气
      textureUrl: ImgUrls.signalBoard.driveCarefullyInRainOrSnow,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104312001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'yieldToPeopleWithDisabilities', // 注意残疾人
      textureUrl: ImgUrls.signalBoard.yieldToPeopleWithDisabilities,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103000001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'watchForWildAnimals', // 注意野生动物
      textureUrl: ImgUrls.signalBoard.watchForWildAnimals,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101300001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'accidentArea', // 事故易发路段
      textureUrl: ImgUrls.signalBoard.accidentArea,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103100001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'reversibleLaneAhead', // 注意潮汐车道
      textureUrl: ImgUrls.signalBoard.reversibleLaneAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103800001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'driveCarefullyInAdverseWeatherConditions', // 注意不利气象条件
      textureUrl: ImgUrls.signalBoard.driveCarefullyInAdverseWeatherConditions,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104314001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'lowLyingRoadAhead', // 路面低洼
      textureUrl: ImgUrls.signalBoard.lowLyingRoadAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102600001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'bumpyRoadAhead', // 路面高凸
      textureUrl: ImgUrls.signalBoard.bumpyRoadAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102500001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'queuesLikelyAhead', // 注意前方车辆排队
      textureUrl: ImgUrls.signalBoard.queuesLikelyAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104400001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'crossroadsWithDualCarriageway', // 十字平面交叉
      textureUrl: ImgUrls.signalBoard.crossroadsWithDualCarriageway,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104011001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'tunnel', // 隧道
      textureUrl: ImgUrls.signalBoard.tunnel,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102100001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'turnOnHeadlightsBeforeEnteringTunnel', // 隧道开车灯
      textureUrl: ImgUrls.signalBoard.turnOnHeadlightsBeforeEnteringTunnel,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103700001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'camelBackBridge', // 驼峰桥
      textureUrl: ImgUrls.signalBoard.camelBackBridge,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102300001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'watchForNonMotorVehicles', // 注意非机动车
      textureUrl: ImgUrls.signalBoard.watchForNonMotorVehicles,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010102900001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'slipperySurface', // 易滑
      textureUrl: ImgUrls.signalBoard.slipperySurface,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101700001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'trafficLightsAhead', // 注意信号灯
      textureUrl: ImgUrls.signalBoard.trafficLightsAhead,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101400001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'detourLeft', // 左侧绕行
      textureUrl: ImgUrls.signalBoard.detourLeft,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103312001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'detourRight', // 右侧绕行
      textureUrl: ImgUrls.signalBoard.detourRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103313001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'detourAheadLeftOrRight', // 左右绕行
      textureUrl: ImgUrls.signalBoard.detourAheadLeftOrRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103311001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'narrowBridge', // 窄桥
      textureUrl: ImgUrls.signalBoard.narrowBridge,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100800001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'keepDistance', // 注意保持车距
      textureUrl: ImgUrls.signalBoard.keepDistance,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103900001111',
      subtype: '-1',
    },
    // TODO 没有注意合流-左的 type
    {
      mainType: 'signalBoard',
      name: 'roadsMergeRight', // 注意合流-右
      textureUrl: ImgUrls.signalBoard.roadsMergeRight,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104111001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'bewareOfCrosswind', // 注意横风
      textureUrl: ImgUrls.signalBoard.bewareOfCrosswind,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101600001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'icyRoads', // 注意路面结冰
      textureUrl: ImgUrls.signalBoard.icyRoads,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104311001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'driveWithCaution', // 注意危险
      textureUrl: ImgUrls.signalBoard.driveWithCaution,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010103400001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'driveCarefullyInFoggyWeather', // 注意雾天
      textureUrl: ImgUrls.signalBoard.driveCarefullyInFoggyWeather,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010104313001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'watchForLivestock', // 注意牲畜
      textureUrl: ImgUrls.signalBoard.watchForLivestock,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101200001111',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'twoWayTraffic', // 双向交通
      textureUrl: ImgUrls.signalBoard.twoWayTraffic,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010100900001111',
      subtype: '-1',
    },
    {
      mainType: '',
      name: 'watchForPedestrians', // 注意行人
      textureUrl: ImgUrls.signalBoard.watchForPedestrians,
      modelUrl: ModelUrls.signalBoard.triangle,
      type: '1010101000001111',
      subtype: '-1',
    },
  ]
  // 标志牌【禁止类型】
  const prohibitionSignalBoard = [
    {
      mainType: 'signalBoard',
      name: 'noStraightThru', // 禁止直行
      textureUrl: ImgUrls.signalBoard.noStraightThru,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202400001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noMotorVehicles', // 禁止机动车驶入
      textureUrl: ImgUrls.signalBoard.noMotorVehicles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010200600001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit120', // 限速120
      textureUrl: ImgUrls.signalBoard.speedLimit120,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '120',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit100', // 限速100
      textureUrl: ImgUrls.signalBoard.speedLimit100,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '100',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit80', // 限速80
      textureUrl: ImgUrls.signalBoard.speedLimit80,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '80',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit70', // 限速70
      textureUrl: ImgUrls.signalBoard.speedLimit70,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '70',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit60', // 限速60
      textureUrl: ImgUrls.signalBoard.speedLimit60,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '60',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit50', // 限速50
      textureUrl: ImgUrls.signalBoard.speedLimit50,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '50',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit40', // 限速40
      textureUrl: ImgUrls.signalBoard.speedLimit40,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '40',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit30', // 限速30
      textureUrl: ImgUrls.signalBoard.speedLimit30,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '30',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit20', // 限速20
      textureUrl: ImgUrls.signalBoard.speedLimit20,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '20',
    },
    {
      mainType: 'signalBoard',
      name: 'speedLimit5', // 限速5
      textureUrl: ImgUrls.signalBoard.speedLimit5,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203800001413',
      subtype: '5',
    },
    {
      mainType: 'signalBoard',
      name: 'stop', // 停车让行
      textureUrl: ImgUrls.signalBoard.stop,
      modelUrl: ModelUrls.signalBoard.octagon, // 八角形
      type: '1010200100001914',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'heightLimit5', // 限高5m
      textureUrl: ImgUrls.signalBoard.heightLimit5,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203500001413',
      subtype: '5',
    },
    {
      mainType: 'signalBoard',
      name: 'heightLimit3dot5', // 限高3.5m
      textureUrl: ImgUrls.signalBoard.heightLimit3dot5,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203500001413',
      subtype: '3.5',
    },
    {
      mainType: 'signalBoard',
      name: 'noEntryForVehicularTraffic', // 禁止驶入
      textureUrl: ImgUrls.signalBoard.noEntryForVehicularTraffic,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010200500001513',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noMotorcycles', // 禁止二轮摩托车驶入
      textureUrl: ImgUrls.signalBoard.noMotorcycles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201400001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'weightLimit55', // 限重55吨
      textureUrl: ImgUrls.signalBoard.weightLimit55,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203600001413',
      subtype: '55',
    },
    {
      mainType: 'signalBoard',
      name: 'weightLimit40', // 限重40吨
      textureUrl: ImgUrls.signalBoard.weightLimit40,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203600001413',
      subtype: '40',
    },
    {
      mainType: 'signalBoard',
      name: 'weightLimit30', // 限重30吨
      textureUrl: ImgUrls.signalBoard.weightLimit30,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203600001413',
      subtype: '30',
    },
    {
      mainType: 'signalBoard',
      name: 'weightLimit20', // 限重20吨
      textureUrl: ImgUrls.signalBoard.weightLimit20,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203600001413',
      subtype: '20',
    },
    {
      mainType: 'signalBoard',
      name: 'weightLimit10', // 限重10吨
      textureUrl: ImgUrls.signalBoard.weightLimit10,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203600001413',
      subtype: '10',
    },
    {
      mainType: 'signalBoard',
      name: 'noHonking', // 禁止鸣喇叭
      textureUrl: ImgUrls.signalBoard.noHonking,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203300001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noTrucks', // 禁止载货汽车驶入
      textureUrl: ImgUrls.signalBoard.noTrucks,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010200700001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noStopping', // 禁止停车
      textureUrl: ImgUrls.signalBoard.noStopping,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203111001713',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'axleWeightLimit14', // 限制轴重14吨
      textureUrl: ImgUrls.signalBoard.axleWeightLimit14,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203700001413',
      subtype: '14',
    },
    {
      mainType: 'signalBoard',
      name: 'axleWeightLimit13', // 限制轴重13吨
      textureUrl: ImgUrls.signalBoard.axleWeightLimit13,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203700001413',
      subtype: '13',
    },
    {
      mainType: 'signalBoard',
      name: 'axleWeightLimit10', // 限制轴重10吨
      textureUrl: ImgUrls.signalBoard.axleWeightLimit10,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203700001413',
      subtype: '10',
    },
    {
      mainType: 'signalBoard',
      name: 'slowDownAndGiveWay', // 减速让行
      textureUrl: ImgUrls.signalBoard.slowDownAndGiveWay,
      modelUrl: ModelUrls.signalBoard.invertedTriangle, // 倒三角
      type: '1010200200002012',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noLeftTurn', // 禁止向左转弯
      textureUrl: ImgUrls.signalBoard.noLeftTurn,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202211001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noRightTurn', // 禁止向右转弯
      textureUrl: ImgUrls.signalBoard.noRightTurn,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202311001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noTurns', // 禁止向左向右转弯
      textureUrl: ImgUrls.signalBoard.noTurns,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202500001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noStraightThruOrLeftTurn', // 禁止直行和向左转弯
      textureUrl: ImgUrls.signalBoard.noStraightThruOrLeftTurn,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202600001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noStraightThruOrRightTurn', // 禁止直行和向右转弯
      textureUrl: ImgUrls.signalBoard.noStraightThruOrRightTurn,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202700001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'vehiclesCarryingHazardousMaterialsProhibited', // 禁止运输危险物品车辆驶入
      textureUrl:
        ImgUrls.signalBoard.vehiclesCarryingHazardousMaterialsProhibited,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010204100001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noTractors', // 禁止拖拉机驶入
      textureUrl: ImgUrls.signalBoard.noTractors,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201200001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noMotorTricycles', // 禁止三轮车机动车驶入
      textureUrl: ImgUrls.signalBoard.noMotorTricycles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201300001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noMinibuses', // 禁止小型客车驶入
      textureUrl: ImgUrls.signalBoard.noMinibuses,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201000001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noLargeBuses', // 禁止大型客车驶入
      textureUrl: ImgUrls.signalBoard.noLargeBuses,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010200900001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noHandcartsOrTricycles', // 禁止人力车驶入
      textureUrl: ImgUrls.signalBoard.noHandcartsOrTricycles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202000001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noHandcartsOrFreightTricycles', // 禁止人力货运三轮车驶入
      textureUrl: ImgUrls.signalBoard.noHandcartsOrFreightTricycles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201800001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noPassengerTricycles', // 禁止人力客运三轮车驶入
      textureUrl: ImgUrls.signalBoard.noPassengerTricycles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201900001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noEntry', // 禁止通行
      textureUrl: ImgUrls.signalBoard.noEntry,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010200400001213',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'giveWayToOncomingVehicles', // 会车让行
      textureUrl: ImgUrls.signalBoard.giveWayToOncomingVehicles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010200300002113',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noLeftTurnForMinibuses', // 禁止小客车向左转弯
      textureUrl: ImgUrls.signalBoard.noLeftTurnForMinibuses,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202212001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noRightTurnForMinibuses', // 禁止小客车向右转弯
      textureUrl: ImgUrls.signalBoard.noRightTurnForMinibuses,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202312001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noLeftTurnForTrucks', // 禁止载货汽车向左转弯
      textureUrl: ImgUrls.signalBoard.noLeftTurnForTrucks,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202215001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noRightTurnForTrucks', // 禁止载货汽车向右转弯
      textureUrl: ImgUrls.signalBoard.noRightTurnForTrucks,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202315001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noOvertaking', // 禁止超车
      textureUrl: ImgUrls.signalBoard.noOvertaking,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202900001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'endOfNoOvertakingZone', // 解除禁止超车
      textureUrl: ImgUrls.signalBoard.endOfNoOvertakingZone,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203000001613',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noAnimalDrawnCarts', // 禁止畜力车进入
      textureUrl: ImgUrls.signalBoard.noAnimalDrawnCarts,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201700001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noElectricTricycles', // 禁止电动三轮车驶入
      textureUrl: ImgUrls.signalBoard.noElectricTricycles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010200800001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noEntryForNonMotorVehicles', // 禁止非机动车驶入
      textureUrl: ImgUrls.signalBoard.noEntryForNonMotorVehicles,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201600001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noTrailers', // 禁止汽车拖、挂车驶入
      textureUrl: ImgUrls.signalBoard.noTrailers,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201100001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noPedestrians', // 禁止行人进入
      textureUrl: ImgUrls.signalBoard.noPedestrians,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202100001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noFreightVehiclesAndTractors', // 禁止某两种车驶入
      textureUrl: ImgUrls.signalBoard.noFreightVehiclesAndTractors,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010201500001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'widthLimit3', // 限制宽度3m
      textureUrl: ImgUrls.signalBoard.widthLimit3,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203400001413',
      subtype: '3',
    },
    {
      mainType: 'signalBoard',
      name: 'customs', // 海关
      textureUrl: ImgUrls.signalBoard.customs,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010204200001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'stopForInspection', // 停车检查
      textureUrl: ImgUrls.signalBoard.stopForInspection,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010204000001413',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'noParking', // 禁止车辆长时停放
      textureUrl: ImgUrls.signalBoard.noParking,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203200001713',
      subtype: '-1',
    },
    {
      mainType: 'signalBoard',
      name: 'endOfSpeedLimit40', // 解除限制速度 40
      textureUrl: ImgUrls.signalBoard.endOfSpeedLimit40,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010203900001613',
      subtype: '-1',
    },
    {
      mainType: '',
      name: 'noUTurn', // 禁止掉头
      textureUrl: ImgUrls.signalBoard.noUTurn,
      modelUrl: ModelUrls.signalBoard.circle,
      type: '1010202800001413',
      subtype: '-1',
    },
  ]

  signalBoardConfig = [
    {
      type: 'indicatorSignalBoard',
      list: indicatorSignalBoard,
    },
    {
      type: 'warningSignalBoard',
      list: warningSignalBoard,
    },
    {
      type: 'prohibitionSignalBoard',
      list: prohibitionSignalBoard,
    },
  ]
}

// 通过 type 和 subtype 获取标志牌配置
export function getSignalBoardConfig (type, subtype) {
  for (const basicType of signalBoardConfig) {
    for (const option of basicType.list) {
      if (option.type === type && option.subtype === subtype) {
        return option
      }
    }
  }

  return null
}
