import i18n from '@/locales'
import { getPermission } from '@/utils/permission'
import { ImgUrls, ModelUrls } from '@/utils/urls'

export interface ITrafficLightConfig {
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

export const _trafficLightConfig: Array<ITrafficLightConfig> = [
  {
    mainType: '',
    name: 'verticalOmnidirectionalLight',
    showName: i18n.global.t('desc.editObject.verticalOmnidirectionalLight'),
    iconUrl: ImgUrls.trafficLight.verticalOmnidirectionalLightIcon,
    textureUrl: ImgUrls.trafficLight.verticalOmnidirectionalLight,
    modelUrl: ModelUrls.trafficLight.verticalOmnidirectionalLight,
    type: '1000001',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'verticalStraightRoundLight',
    showName: i18n.global.t('desc.editObject.verticalStraightRoundLight'),
    iconUrl: ImgUrls.trafficLight.verticalStraightRoundLightIcon,
    textureUrl: ImgUrls.trafficLight.verticalStraightRoundLight,
    modelUrl: ModelUrls.trafficLight.verticalOmnidirectionalLight,
    type: '1000011',
    subtype: '60',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'verticalLeftTurnLight',
    showName: i18n.global.t('desc.editObject.verticalLeftTurnLight'),
    iconUrl: ImgUrls.trafficLight.verticalLeftTurnLightIcon,
    textureUrl: ImgUrls.trafficLight.verticalLeftTurnLight,
    modelUrl: ModelUrls.trafficLight.verticalLeftTurnLight,
    type: '1000011',
    subtype: '10',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'verticalStraightLight',
    showName: i18n.global.t('desc.editObject.verticalStraightLight'),
    iconUrl: ImgUrls.trafficLight.verticalStraightLightIcon,
    textureUrl: ImgUrls.trafficLight.verticalStraightLight,
    modelUrl: ModelUrls.trafficLight.verticalStraightLight,
    type: '1000011',
    subtype: '30',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'verticalRightTurnLight',
    showName: i18n.global.t('desc.editObject.verticalRightTurnLight'),
    iconUrl: ImgUrls.trafficLight.verticalRightTurnLightIcon,
    textureUrl: ImgUrls.trafficLight.verticalRightTurnLight,
    modelUrl: ModelUrls.trafficLight.verticalRightTurnLight,
    type: '1000011',
    subtype: '20',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'verticalUTurnLight',
    showName: i18n.global.t('desc.editObject.verticalUTurnLight'),
    iconUrl: ImgUrls.trafficLight.verticalUTurnLightIcon,
    textureUrl: ImgUrls.trafficLight.verticalUTurnLight,
    modelUrl: ModelUrls.trafficLight.verticalUTurnLight,
    type: '1000011',
    subtype: '70',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'verticalPedestrianLight',
    showName: i18n.global.t('desc.editObject.verticalPedestrianLight'),
    iconUrl: ImgUrls.trafficLight.verticalPedestrianLightIcon,
    textureUrl: ImgUrls.trafficLight.verticalPedestrianLight,
    modelUrl: ModelUrls.trafficLight.verticalPedestrianLight,
    type: '1000002',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'horizontalOmnidirectionalLight',
    showName: i18n.global.t('desc.editObject.horizontalOmnidirectionalLight'),
    iconUrl: ImgUrls.trafficLight.horizontalOmnidirectionalLightIcon,
    textureUrl: ImgUrls.trafficLight.horizontalOmnidirectionalLight,
    modelUrl: ModelUrls.trafficLight.horizontalOmnidirectionalLight,
    type: '1000003',
    subtype: '-1',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'horizontalStraightRoundLight',
    showName: i18n.global.t('desc.editObject.horizontalStraightRoundLight'),
    iconUrl: ImgUrls.trafficLight.horizontalStraightRoundLightIcon,
    textureUrl: ImgUrls.trafficLight.horizontalStraightRoundLight,
    modelUrl: ModelUrls.trafficLight.horizontalOmnidirectionalLight,
    type: '1000021',
    subtype: '60',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'horizontalLeftTurnLight',
    showName: i18n.global.t('desc.editObject.horizontalLeftTurnLight'),
    iconUrl: ImgUrls.trafficLight.horizontalLeftTurnLightIcon,
    textureUrl: ImgUrls.trafficLight.horizontalLeftTurnLight,
    modelUrl: ModelUrls.trafficLight.horizontalLeftTurnLight,
    type: '1000021',
    subtype: '10',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'horizontalStraightLight',
    showName: i18n.global.t('desc.editObject.horizontalStraightLight'),
    iconUrl: ImgUrls.trafficLight.horizontalStraightLightIcon,
    textureUrl: ImgUrls.trafficLight.horizontalStraightLight,
    modelUrl: ModelUrls.trafficLight.horizontalStraightLight,
    type: '1000021',
    subtype: '30',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'horizontalRightTurnLight',
    showName: i18n.global.t('desc.editObject.horizontalRightTurnLight'),
    iconUrl: ImgUrls.trafficLight.horizontalRightTurnLightIcon,
    textureUrl: ImgUrls.trafficLight.horizontalRightTurnLight,
    modelUrl: ModelUrls.trafficLight.horizontalRightTurnLight,
    type: '1000021',
    subtype: '20',
    placementAreaName: [],
  },
  {
    mainType: '',
    name: 'horizontalUTurnLight',
    showName: i18n.global.t('desc.editObject.horizontalUTurnLight'),
    iconUrl: ImgUrls.trafficLight.horizontalUTurnLightIcon,
    textureUrl: ImgUrls.trafficLight.horizontalUTurnLight,
    modelUrl: ModelUrls.trafficLight.horizontalUTurnLight,
    type: '1000021',
    subtype: '70',
    placementAreaName: [],
  },
  // 单车指示灯
  {
    mainType: '',
    name: 'bicycleLight',
    showName: i18n.global.t('desc.editObject.bicycleLight'),
    iconUrl: ImgUrls.trafficLight.bicycleLightIcon,
    textureUrl: ImgUrls.trafficLight.bicycleLight,
    modelUrl: ModelUrls.trafficLight.bicycleLight,
    type: '1000013',
    subtype: '-1',
    placementAreaName: [],
  },
  // 双色指示灯
  {
    mainType: '',
    name: 'twoColorIndicatorLight',
    showName: i18n.global.t('desc.editObject.twoColorIndicatorLight'),
    iconUrl: ImgUrls.trafficLight.twoColorIndicatorLightIcon,
    textureUrl: ImgUrls.trafficLight.twoColorIndicatorLight,
    modelUrl: ModelUrls.trafficLight.twoColorIndicatorLight,
    type: '1000009',
    subtype: '-1',
    placementAreaName: [],
  },
]
export const trafficLightConfig = _trafficLightConfig.filter((item) => {
  const isHorizontal = [
    'horizontalOmnidirectionalLight',
    'horizontalStraightRoundLight',
    'horizontalLeftTurnLight',
    'horizontalStraightLight',
    'horizontalRightTurnLight',
    'horizontalUTurnLight',
  ]
  const isVertical = [
    'verticalOmnidirectionalLight',
    'verticalStraightRoundLight',
    'verticalLeftTurnLight',
    'verticalStraightLight',
    'verticalRightTurnLight',
    'verticalUTurnLight',
    'verticalPedestrianLight',
    'bicycleLight',
    'twoColorIndicatorLight',
  ]

  if (isHorizontal.includes(item.name)) {
    return getPermission(
      'action.mapEditor.models.trafficLight.horizontal.view.enable',
    )
  }
  if (isVertical.includes(item.name)) {
    return getPermission(
      'action.mapEditor.models.trafficLight.vertical.view.enable',
    )
  }
  return true
})

// 通过 name 获取配置
export function getTrafficLightConfigByName (name: string) {
  for (const option of trafficLightConfig) {
    if (option.name === name) {
      return option
    }
  }
  return null
}

// 通过 type 和 subtype 获取信号灯配置
export function getTrafficLightConfig (type: string, subtype: string) {
  for (const option of trafficLightConfig) {
    if (option.type === type && option.subtype === subtype) {
      return option
    }
  }
  return null
}

// 初始化预设的参数
function initPresetConfig () {
  for (const option of trafficLightConfig) {
    option.mainType = 'trafficLight'
    option.placementAreaName = [
      'horizontalPole',
      'verticalPole',
      'horizontalPolePlacementArea',
      'verticalPolePlacementArea',
    ]
  }
}
initPresetConfig()
