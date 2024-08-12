import { ImgUrls, ModelUrls } from '../utils/urls'

// 交通信号灯
let trafficLightConfig = []

export function initTrafficLightConfig () {
  trafficLightConfig = [
    {
      mainType: 'trafficLight',
      name: 'verticalOmnidirectionalLight', // 竖排全方位灯
      textureUrl: ImgUrls.trafficLight.verticalOmnidirectionalLight,
      modelUrl: ModelUrls.trafficLight.verticalOmnidirectionalLight,
      type: '1000001',
      subtype: '-1',
    },
    {
      mainType: 'trafficLight',
      name: 'verticalStraightRoundLight', // 竖排直行圆灯
      textureUrl: ImgUrls.trafficLight.verticalStraightRoundLight,
      modelUrl: ModelUrls.trafficLight.verticalOmnidirectionalLight,
      type: '1000011',
      subtype: '60',
    },
    {
      mainType: 'trafficLight',
      name: 'verticalLeftTurnLight', // 竖排左转灯
      textureUrl: ImgUrls.trafficLight.verticalLeftTurnLight,
      modelUrl: ModelUrls.trafficLight.verticalLeftTurnLight,
      type: '1000011',
      subtype: '10',
    },
    {
      mainType: 'trafficLight',
      name: 'verticalStraightLight', // 竖排直行灯
      textureUrl: ImgUrls.trafficLight.verticalStraightLight,
      modelUrl: ModelUrls.trafficLight.verticalStraightLight,
      type: '1000011',
      subtype: '30',
    },
    {
      mainType: 'trafficLight',
      name: 'verticalRightTurnLight', // 竖排右转灯
      textureUrl: ImgUrls.trafficLight.verticalRightTurnLight,
      modelUrl: ModelUrls.trafficLight.verticalRightTurnLight,
      type: '1000011',
      subtype: '20',
    },
    {
      mainType: 'trafficLight',
      name: 'verticalUTurnLight', // 竖排掉头灯
      textureUrl: ImgUrls.trafficLight.verticalUTurnLight,
      modelUrl: ModelUrls.trafficLight.verticalUTurnLight,
      type: '1000011',
      subtype: '70',
    },
    {
      mainType: 'trafficLight',
      name: 'verticalPedestrianLight', // 竖排人行灯
      textureUrl: ImgUrls.trafficLight.verticalPedestrianLight,
      modelUrl: ModelUrls.trafficLight.verticalPedestrianLight,
      type: '1000002',
      subtype: '-1',
    },
    {
      mainType: 'trafficLight',
      name: 'horizontalOmnidirectionalLight', // 横排全方位灯
      textureUrl: ImgUrls.trafficLight.horizontalOmnidirectionalLight,
      modelUrl: ModelUrls.trafficLight.horizontalOmnidirectionalLight,
      type: '1000003',
      subtype: '-1',
    },
    {
      mainType: 'trafficLight',
      name: 'horizontalStraightRoundLight', // 横排直行圆灯
      textureUrl: ImgUrls.trafficLight.horizontalStraightRoundLight,
      modelUrl: ModelUrls.trafficLight.horizontalOmnidirectionalLight,
      type: '1000021',
      subtype: '60',
    },
    {
      mainType: 'trafficLight',
      name: 'horizontalLeftTurnLight', // 横排左转灯
      textureUrl: ImgUrls.trafficLight.horizontalLeftTurnLight,
      modelUrl: ModelUrls.trafficLight.horizontalLeftTurnLight,
      type: '1000021',
      subtype: '10',
    },
    {
      mainType: 'trafficLight',
      name: 'horizontalStraightLight', // 横排直行灯
      textureUrl: ImgUrls.trafficLight.horizontalStraightLight,
      modelUrl: ModelUrls.trafficLight.horizontalStraightLight,
      type: '1000021',
      subtype: '30',
    },
    {
      mainType: 'trafficLight',
      name: 'horizontalRightTurnLight', // 横排右转灯
      textureUrl: ImgUrls.trafficLight.horizontalRightTurnLight,
      modelUrl: ModelUrls.trafficLight.horizontalRightTurnLight,
      type: '1000021',
      subtype: '20',
    },
    {
      mainType: 'trafficLight',
      name: 'horizontalUTurnLight', // 横排掉头灯
      textureUrl: ImgUrls.trafficLight.horizontalUTurnLight,
      modelUrl: ModelUrls.trafficLight.horizontalUTurnLight,
      type: '1000021',
      subtype: '70',
    },

    {
      mainType: 'trafficLight',
      name: 'bicycleLight', // 单车指示灯
      textureUrl: ImgUrls.trafficLight.bicycleLight,
      modelUrl: ModelUrls.trafficLight.bicycleLight,
      type: '1000013',
      subtype: '-1',
    },
    {
      mainType: 'trafficLight',
      name: 'twoColorIndicatorLight', // 双色指示灯
      textureUrl: ImgUrls.trafficLight.twoColorIndicatorLight,
      modelUrl: ModelUrls.trafficLight.twoColorIndicatorLight,
      type: '1000009',
      subtype: '-1',
    },
  ]
}

// 通过 type 和 subtype 获取信号灯的配置
export function getTrafficLightConfig (type, subtype) {
  for (const option of trafficLightConfig) {
    if (option.type === type && option.subtype === subtype) {
      return option
    }
  }
  return null
}
