// 其他类型物体相关的配置
import i18n from '@/locales'
import { getPermission } from '@/utils/permission'
import { ImgUrls, ModelUrls } from '@/utils/urls'

export interface IOtherConfig {
  mainType: 'other'
  name: string
  showName: string
  iconUrl: string
  modelUrl?: string
  textureUrl?: string
  type: string
  subtype: string
  placementAreaName: Array<string>
  initYaw?: number // 由于模型默认的朝向不一定都是沿着道路行驶的方向，需要手动提供一个默认偏航角
  isPlaneModel: boolean // 是否是平面模型
  imageRatio?: number // 如果是平面模型，由于纹理图片不是icon图片了，在 textureLoader 加载纹理时无法第一时间获取到纹理的长宽比，需要手动定义
  basicSize?: number // 如果时平面模型，提供初始的尺寸
  adjustSize?: common.adjustSizeMode // 尺寸编辑模式
  isBridge?: boolean // 是否是天桥
}

// 地面污损类
const roadDamageOrDefectConfig: Array<IOtherConfig> = [
  {
    mainType: 'other',
    name: 'Pothole', // 坑洼
    showName: i18n.global.t('desc.editObject.pothole'),
    iconUrl: ImgUrls.other.potholeIcon,
    textureUrl: ImgUrls.other.pothole,
    isPlaneModel: true,
    imageRatio: 1,
    basicSize: 2,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
  {
    mainType: 'other',
    name: 'Patch', // 补丁
    showName: i18n.global.t('desc.editObject.patch'),
    iconUrl: ImgUrls.other.patchIcon,
    // 补丁用 icon 作为纹理
    textureUrl: ImgUrls.other.patchIcon,
    isPlaneModel: true,
    imageRatio: 0.5,
    basicSize: 0.4,
    type: 'patch',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
  {
    mainType: 'other',
    name: 'Crack', // 裂缝
    showName: i18n.global.t('desc.editObject.crack'),
    iconUrl: ImgUrls.other.crackIcon,
    textureUrl: ImgUrls.other.crack,
    isPlaneModel: true,
    imageRatio: 0.16,
    basicSize: 0.4,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
  {
    mainType: 'other',
    name: 'Asphalt_Line', // 沥青线
    showName: i18n.global.t('desc.editObject.asphaltLine'),
    iconUrl: ImgUrls.other.asphaltLineIcon,
    textureUrl: ImgUrls.other.asphaltLine,
    isPlaneModel: true,
    imageRatio: 0.22,
    basicSize: 0.4,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
  {
    mainType: 'other',
    name: 'Rut_Track', // 车轮痕迹
    showName: i18n.global.t('desc.editObject.tireMarks'),
    iconUrl: ImgUrls.other.tireMarksIcon,
    textureUrl: ImgUrls.other.tireMarks,
    isPlaneModel: true,
    imageRatio: 0.133,
    basicSize: 0.24,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
  {
    mainType: 'other',
    name: 'Stagnant_Water', // 积水
    showName: i18n.global.t('desc.editObject.stagnantWater'),
    iconUrl: ImgUrls.other.stagnantWaterIcon,
    textureUrl: ImgUrls.other.stagnantWater,
    isPlaneModel: true,
    imageRatio: 1,
    basicSize: 2,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
  {
    mainType: 'other',
    name: 'Protrusion', // 凸起
    showName: i18n.global.t('desc.editObject.bump'),
    iconUrl: ImgUrls.other.bumpIcon,
    textureUrl: ImgUrls.other.bump,
    modelUrl: ModelUrls.other.bump,
    isPlaneModel: false,
    type: 'obstacle',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lwh',
  },
]

// 交通管理类
const _trafficManagementConfig: Array<IOtherConfig> = [
  {
    mainType: 'other',
    name: 'Well_Cover', // 井盖
    showName: i18n.global.t('desc.editObject.manholeCover'),
    iconUrl: ImgUrls.other.manholeCoverIcon,
    // 井盖的模型没有默认的纹理，使用 icon 的图片作为纹理
    textureUrl: ImgUrls.other.manholeCoverIcon,
    modelUrl: ModelUrls.other.manholeCover,
    isPlaneModel: false,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Deceleration_Zone', // 减速带
    showName: i18n.global.t('desc.editObject.decelerationZone'),
    iconUrl: ImgUrls.other.decelerationZoneIcon,
    modelUrl: ModelUrls.other.decelerationZone,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    initYaw: 90,
    adjustSize: 'l',
  },
  {
    mainType: 'other',
    name: 'Reflective_Road_Sign', // 反光路标
    showName: i18n.global.t('desc.editObject.reflectiveRoadSign'),
    iconUrl: ImgUrls.other.reflectiveRoadSignIcon,
    // 反光路标没有默认的纹理，手动赋值
    textureUrl: ImgUrls.other.reflectiveRoadSign,
    modelUrl: ModelUrls.other.reflectiveRoadSign,
    isPlaneModel: false,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Parking_Hole', // 停车杆
    showName: i18n.global.t('desc.editObject.parkingLever'),
    iconUrl: ImgUrls.other.parkingLeverIcon,
    modelUrl: ModelUrls.other.parkingLever,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Parking_Lot', // 停车桩
    showName: i18n.global.t('desc.editObject.parkingPile'),
    iconUrl: ImgUrls.other.parkingPileIcon,
    modelUrl: ModelUrls.other.parkingPile,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Ground_Lock', // 地锁
    showName: i18n.global.t('desc.editObject.groundLock'),
    iconUrl: ImgUrls.other.groundLockIcon,
    modelUrl: ModelUrls.other.groundLock,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Plastic_Vehicle_Stopper', // 塑胶挡车器
    showName: i18n.global.t('desc.editObject.plasticCarStopper'),
    iconUrl: ImgUrls.other.plasticCarStopperIcon,
    modelUrl: ModelUrls.other.plasticCarStopper,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
  {
    mainType: 'other',
    name: 'Parking_Limit_Position_Pole_2m', // U型挡车器
    showName: i18n.global.t('desc.editObject.uShapedCarStopper'),
    iconUrl: ImgUrls.other.uShapedCarStopperIcon,
    modelUrl: ModelUrls.other.uShapedCarStopper,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'l',
  },
  {
    mainType: 'other',
    name: 'Support_Vehicle_Stopper', // 支撑型挡车器
    showName: i18n.global.t('desc.editObject.supportTypeCarStopper'),
    iconUrl: ImgUrls.other.supportTypeCarStopperIcon,
    modelUrl: ModelUrls.other.supportTypeCarStopper,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'l',
  },
  {
    mainType: 'other',
    name: 'Charging_Station', // 充电桩
    showName: i18n.global.t('desc.editObject.chargingStation'),
    iconUrl: ImgUrls.other.chargingStationIcon,
    modelUrl: ModelUrls.other.chargingStation,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Lamp', // 路灯
    showName: i18n.global.t('desc.editObject.lamp'),
    iconUrl: ImgUrls.other.lampIcon,
    modelUrl: ModelUrls.other.lamp,
    isPlaneModel: false,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    initYaw: 90,
  },
  {
    mainType: 'other',
    name: 'Traffic_Barrier', // 交通护栏
    showName: i18n.global.t('desc.editObject.trafficBarrier'),
    iconUrl: ImgUrls.other.trafficBarrierIcon,
    modelUrl: ModelUrls.other.trafficBarrier,
    isPlaneModel: false,
    type: 'obstacle',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'l',
  },
  {
    mainType: 'other',
    name: 'Road_Curb', // 路沿石
    showName: i18n.global.t('desc.editObject.roadCurb'),
    iconUrl: ImgUrls.other.roadCurbIcon,
    modelUrl: ModelUrls.other.roadCurb,
    isPlaneModel: false,
    type: 'barrier',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lwh',
  },
  {
    mainType: 'other',
    name: 'Traffic_Cone', // 交通锥
    showName: i18n.global.t('desc.editObject.trafficCone'),
    iconUrl: ImgUrls.other.trafficConeIcon,
    modelUrl: ModelUrls.other.trafficCone,
    isPlaneModel: false,
    type: 'obstacle',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Traffic_Horse', // 水马
    showName: i18n.global.t('desc.editObject.trafficHorse'),
    iconUrl: ImgUrls.other.trafficHorseIcon,
    modelUrl: ModelUrls.other.trafficHorse,
    isPlaneModel: false,
    type: 'obstacle',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'l',
  },
  {
    mainType: 'other',
    name: 'GarbageCan', // 垃圾桶
    showName: i18n.global.t('desc.editObject.garbageCan'),
    iconUrl: ImgUrls.other.garbageCanIcon,
    modelUrl: ModelUrls.other.garbageCan,
    isPlaneModel: false,
    type: 'none',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
  },
  {
    mainType: 'other',
    name: 'Obstacle', // 障碍物
    showName: i18n.global.t('desc.editObject.obstacle'),
    iconUrl: ImgUrls.other.obstacleIcon,
    modelUrl: ModelUrls.other.obstacle,
    isPlaneModel: false,
    type: 'obstacle',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lwh',
  },
]
const trafficManagementConfig = _trafficManagementConfig.filter((item) => {
  if (item.name === 'Lamp') {
    return getPermission('action.mapEditor.models.other.lamp.view.enable')
  }
  if (item.name === 'Traffic_Barrier') {
    return getPermission('action.mapEditor.models.other.barrier.view.enable')
  }
  if (item.name === 'Road_Curb') {
    return getPermission('action.mapEditor.models.other.roadCurb.view.enable')
  }
  return getPermission(
    'action.mapEditor.models.other.trafficManagement.view.enable',
  )
})

// 植被类
const vegetationConfig: Array<IOtherConfig> = [
  {
    mainType: 'other',
    name: 'Tree', // 树木
    showName: i18n.global.t('desc.editObject.tree'),
    iconUrl: ImgUrls.other.treeIcon,
    modelUrl: ModelUrls.other.tree,
    isPlaneModel: false,
    type: 'tree',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'ratio3',
  },
  {
    mainType: 'other',
    name: 'Shrub', // 灌木
    showName: i18n.global.t('desc.editObject.shrub'),
    iconUrl: ImgUrls.other.shrubIcon,
    modelUrl: ModelUrls.other.shrub,
    isPlaneModel: false,
    type: 'vegetation',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lwh',
  },
  {
    mainType: 'other',
    name: 'Grass', // 草坪
    showName: i18n.global.t('desc.editObject.grass'),
    iconUrl: ImgUrls.other.grassIcon,
    textureUrl: ImgUrls.other.grass,
    isPlaneModel: true,
    imageRatio: 1,
    basicSize: 2,
    type: 'vegetation',
    subtype: '',
    placementAreaName: [
      'lane',
      'junction',
      'roadPlacementArea',
      'junctionPlacementArea',
    ],
    adjustSize: 'lw',
  },
]

// 建筑类
const structureConfig: Array<IOtherConfig> = [
  {
    mainType: 'other',
    name: 'Building', // 居民楼
    showName: i18n.global.t('desc.editObject.residence'),
    iconUrl: ImgUrls.other.residenceIcon,
    modelUrl: ModelUrls.other.residence,
    isPlaneModel: false,
    type: 'building',
    subtype: '',
    placementAreaName: ['roadPlacementArea', 'junctionPlacementArea'],
    adjustSize: 'ratio3',
  },
  {
    mainType: 'other',
    name: 'BusStation', // 公交站
    showName: i18n.global.t('desc.editObject.busStop'),
    iconUrl: ImgUrls.other.busStopIcon,
    modelUrl: ModelUrls.other.busStop,
    isPlaneModel: false,
    type: 'none',
    subtype: '',
    placementAreaName: ['roadPlacementArea', 'junctionPlacementArea'],
    adjustSize: 'ratio3',
  },
]

// 天桥类
const bridgeConfig: Array<IOtherConfig> = [
  {
    mainType: 'other',
    name: 'PedestrianBridge', // 人行天桥
    showName: i18n.global.t('desc.editObject.straddlePedestrianBridge'),
    iconUrl: ImgUrls.other.pedestrianBridgeIcon,
    modelUrl: ModelUrls.other.pedestrianBridge,
    isPlaneModel: false,
    type: 'none',
    subtype: '',
    placementAreaName: ['roadPlacementArea'],
    isBridge: true,
  },
]

export const _otherConfig = [
  {
    name: i18n.global.t('desc.editObject.roadDamageOrDefect'),
    type: 'roadDamageOrDefect',
    list: roadDamageOrDefectConfig,
  },
  {
    name: i18n.global.t('desc.editObject.trafficManagement'),
    type: 'trafficManagement',
    list: trafficManagementConfig,
  },
  {
    name: i18n.global.t('desc.editObject.vegetation'),
    type: 'vegetation',
    list: vegetationConfig,
  },
  {
    name: i18n.global.t('desc.editObject.structure'),
    type: 'structure',
    list: structureConfig,
  },
  {
    name: i18n.global.t('desc.editObject.bridge'),
    type: 'bridge',
    list: bridgeConfig,
  },
]
export const otherConfig = _otherConfig.filter((item) => {
  if (item.type === 'roadDamageOrDefect') {
    return getPermission(
      'action.mapEditor.models.other.roadDamageOrDefect.view.enable',
    )
  }
  if (item.type === 'trafficManagement') {
    const hasChildShow =
      getPermission('action.mapEditor.models.other.roadCurb.view.enable') ||
      getPermission('action.mapEditor.models.other.lamp.view.enable') ||
      getPermission('action.mapEditor.models.other.barrier.view.enable')
    return (
      getPermission(
        'action.mapEditor.models.other.trafficManagement.view.enable',
      ) || hasChildShow
    )
  }
  if (item.type === 'vegetation') {
    return getPermission('action.mapEditor.models.other.vegetation.view.enable')
  }
  if (item.type === 'structure') {
    return getPermission('action.mapEditor.models.other.structure.view.enable')
  }
  if (item.type === 'bridge') {
    return getPermission('action.mapEditor.models.other.bridge.view.enable')
  }
  return true
})

// 通过名称获取配置
export function getOtherConfig (name: string) {
  for (const otherType of otherConfig) {
    for (const option of otherType.list) {
      if (option.name === name) {
        return option
      }
    }
  }
  return null
}
