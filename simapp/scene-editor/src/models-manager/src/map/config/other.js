import { ImgUrls, ModelUrls } from '../utils/urls'

let otherConfig = []

// 初始化资源资源配置
export function initOtherConfig () {
  // 其他物体【地面污损】
  const roadDamageOrDefectConfig = [
    {
      mainType: 'other',
      name: 'Pothole', // 坑洼
      textureUrl: ImgUrls.other.pothole,
      isPlaneModel: true,
      imageRatio: 1,
      basicSize: 2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Patch', // 补丁
      // 补丁用 icon 作为纹理
      textureUrl: ImgUrls.other.patchIcon,
      isPlaneModel: true,
      imageRatio: 0.5,
      basicSize: 0.4,
      type: 'patch',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Crack', // 裂缝
      textureUrl: ImgUrls.other.crack,
      isPlaneModel: true,
      imageRatio: 0.16,
      basicSize: 0.4,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Asphalt_Line', // 沥青线
      textureUrl: ImgUrls.other.asphaltLine,
      isPlaneModel: true,
      imageRatio: 0.22,
      basicSize: 0.4,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Rut_Track', // 车轮痕迹
      textureUrl: ImgUrls.other.tireMarks,
      isPlaneModel: true,
      imageRatio: 0.133,
      basicSize: 0.24,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Stagnant_Water', // 积水
      textureUrl: ImgUrls.other.stagnantWater,
      isPlaneModel: true,
      imageRatio: 1,
      basicSize: 2,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Protrusion', // 凸起
      textureUrl: ImgUrls.other.bump,
      modelUrl: ModelUrls.other.bump,
      isPlaneModel: false,
      type: 'obstacle',
      subtype: '',
    },
  ]

  // 其他物体【交通管理】
  const trafficManagementConfig = [
    {
      mainType: 'other',
      name: 'Well_Cover', // 井盖
      // 井盖的模型没有默认的纹理，使用 icon 的图片作为纹理
      textureUrl: ImgUrls.other.manholeCoverIcon,
      modelUrl: ModelUrls.other.manholeCover,
      isPlaneModel: false,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Deceleration_Zone', // 减速带
      modelUrl: ModelUrls.other.decelerationZone,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
      initYaw: 90,
    },
    {
      mainType: 'other',
      name: 'Reflective_Road_Sign', // 反光路标
      textureUrl: ImgUrls.other.reflectiveRoadSign,
      modelUrl: ModelUrls.other.reflectiveRoadSign,
      isPlaneModel: false,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Parking_Hole', // 停车杆
      modelUrl: ModelUrls.other.parkingLever,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Parking_Lot', // 停车桩
      modelUrl: ModelUrls.other.parkingPile,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Ground_Lock', // 地锁
      modelUrl: ModelUrls.other.groundLock,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Plastic_Vehicle_Stopper', // 塑胶挡车器
      modelUrl: ModelUrls.other.plasticCarStopper,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Parking_Limit_Position_Pole_2m', // U型挡车器
      modelUrl: ModelUrls.other.uShapedCarStopper,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Support_Vehicle_Stopper', // 支撑型挡车器
      modelUrl: ModelUrls.other.supportTypeCarStopper,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Charging_Station', // 充电桩
      modelUrl: ModelUrls.other.chargingStation,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Lamp', // 路灯
      modelUrl: ModelUrls.other.lamp,
      isPlaneModel: false,
      type: 'none',
      subtype: '',
      initYaw: 90,
    },
    {
      mainType: 'other',
      name: 'Traffic_Barrier', // 交通护栏
      modelUrl: ModelUrls.other.trafficBarrier,
      isPlaneModel: false,
      type: 'obstacle',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Road_Curb', // 路沿石
      modelUrl: ModelUrls.other.roadCurb,
      isPlaneModel: false,
      type: 'barrier',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Traffic_Cone', // 交通锥
      modelUrl: ModelUrls.other.trafficCone,
      isPlaneModel: false,
      type: 'obstacle',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Traffic_Horse', // 水马
      modelUrl: ModelUrls.other.trafficHorse,
      isPlaneModel: false,
      type: 'obstacle',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'GarbageCan', // 垃圾桶
      modelUrl: ModelUrls.other.garbageCan,
      isPlaneModel: false,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Obstacle', // 障碍物
      modelUrl: ModelUrls.other.obstacle,
      isPlaneModel: false,
      type: 'obstacle',
      subtype: '',
    },
  ]

  // 其他物体【植被】
  const vegetationConfig = [
    {
      mainType: 'other',
      name: 'Tree', // 树木
      modelUrl: ModelUrls.other.tree,
      isPlaneModel: false,
      type: 'tree',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Shrub', // 灌木
      modelUrl: ModelUrls.other.shrub,
      isPlaneModel: false,
      type: 'vegetation',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'Grass', // 草坪
      textureUrl: ImgUrls.other.grass,
      isPlaneModel: true,
      imageRatio: 1,
      basicSize: 2,
      type: 'vegetation',
      subtype: '',
    },
  ]

  // 其他物体【建筑】
  const structureConfig = [
    {
      mainType: 'other',
      name: 'Building', // 居民楼
      modelUrl: ModelUrls.other.residence,
      isPlaneModel: false,
      type: 'building',
      subtype: '',
    },
    {
      mainType: 'other',
      name: 'BusStation', // 公交站
      modelUrl: ModelUrls.other.busStop,
      isPlaneModel: false,
      type: 'none',
      subtype: '',
    },
  ]

  // 其他物体【天桥类】
  const bridgeConfig = [
    {
      mainType: 'other',
      name: 'PedestrianBridge', // 人行天桥
      modelUrl: ModelUrls.other.pedestrianBridge,
      isPlaneModel: false,
      type: 'none',
      subtype: '',
      isBridge: true,
    },
  ]

  otherConfig = [
    {
      type: 'roadDamageOrDefect',
      list: roadDamageOrDefectConfig,
    },
    {
      type: 'trafficManagement',
      list: trafficManagementConfig,
    },
    {
      type: 'vegetation',
      list: vegetationConfig,
    },
    {
      type: 'structure',
      list: structureConfig,
    },
    {
      type: 'bridge',
      list: bridgeConfig,
    },
  ]
}

// 通过名称获取其他物体的配置
export function getOtherConfig (name) {
  for (const basicType of otherConfig) {
    for (const option of basicType.list) {
      if (option.name === name) {
        return option
      }
    }
  }
  return null
}
