import i18n from '@/locales'

export interface IParkingSpaceConfig {
  mainType: 'parkingSpace'
  name: string
  showName: string
  iconUrl: string
  width: number
  length: number
  lineWidth: number
  color: common.colorType
  innerAngle: number
  type: string
  subtype: string
  placementAreaName: Array<string>
  fixedProperty: Array<'length' | 'innerAngle'> // 限制不能修改的属性
}

// 通用的停车位
export const customParkingSpace: IParkingSpaceConfig = {
  mainType: 'parkingSpace',
  name: 'Parking_Space_Mark',
  showName: i18n.global.t('desc.editObject.parkingSpotMarking'),
  iconUrl: '',
  width: 3,
  length: 6,
  lineWidth: 0.2,
  color: 'White',
  innerAngle: 90,
  type: 'parkingSpace',
  subtype: '',
  placementAreaName: ['lane', 'roadPlacementArea'],
  fixedProperty: [],
}

// 虚线的限时停车位
export const limitedTimeParkingSpace: IParkingSpaceConfig = {
  mainType: 'parkingSpace',
  name: 'Time_Limit_Parking_Space_Mark',
  showName: i18n.global.t('desc.editObject.limitedTimeParkingSpace'),
  iconUrl: '',
  width: 3,
  length: 6,
  lineWidth: 0.2,
  color: 'White',
  innerAngle: 90,
  type: 'parkingSpace',
  subtype: '',
  placementAreaName: ['lane', 'roadPlacementArea'],
  fixedProperty: [],
}

// 固定样式的停车位配置
export const fixedParkingSpace: Array<IParkingSpaceConfig> = [
  {
    mainType: 'parkingSpace',
    name: 'Parking_6m',
    showName: i18n.global.t('desc.editObject.parking6m'),
    iconUrl: '',
    width: 2.5,
    lineWidth: 0.2,
    color: 'White',
    length: 6,
    innerAngle: 90,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
    fixedProperty: ['length'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Parking_5m',
    showName: i18n.global.t('desc.editObject.parking5m'),
    iconUrl: '',
    width: 2.5,
    lineWidth: 0.2,
    color: 'White',
    length: 5,
    innerAngle: 90,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
    fixedProperty: ['length'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Parking_45deg',
    showName: i18n.global.t('desc.editObject.parking45deg'),
    iconUrl: '',
    width: 3.536,
    lineWidth: 0.2,
    color: 'White',
    length: 5.5,
    innerAngle: 45,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
    fixedProperty: ['innerAngle'],
  },
  {
    mainType: 'parkingSpace',
    name: 'Parking_60deg',
    showName: i18n.global.t('desc.editObject.parking60deg'),
    iconUrl: '',
    width: 2.888,
    lineWidth: 0.2,
    color: 'White',
    length: 6.004,
    innerAngle: 60,
    type: 'parkingSpace',
    subtype: '',
    placementAreaName: ['lane', 'roadPlacementArea'],
    fixedProperty: ['innerAngle'],
  },
]

// 通过名称获取配置
export function getParkingSpaceConfig (name: string) {
  if (name === customParkingSpace.name) {
    return customParkingSpace
  }

  if (name === limitedTimeParkingSpace.name) {
    return limitedTimeParkingSpace
  }

  for (const option of fixedParkingSpace) {
    if (option.name === name) {
      return option
    }
  }

  return null
}
