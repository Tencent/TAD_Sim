// 路侧传感器相关配置

import i18n from '@/locales'
import { getPermission } from '@/utils/permission'
import { ImgUrls, ModelUrls } from '@/utils/urls'

export interface ISensorConfig {
  mainType: string
  name: string
  showName: string
  iconUrl: string
  textureUrl: string
  modelUrl: string
  type: string
  subtype: string
  placementAreaName: Array<string>
}

export const _sensorConfig: Array<ISensorConfig> = [
  {
    mainType: 'sensor',
    name: 'Camera', // 摄像头
    showName: i18n.global.t('desc.editObject.camera'),
    iconUrl: ImgUrls.sensor.cameraIcon,
    textureUrl: ImgUrls.sensor.camera,
    modelUrl: ModelUrls.sensor.camera,
    type: 'none',
    subtype: '',
    placementAreaName: [],
  },
  {
    mainType: 'sensor',
    name: 'Millimeter_Wave_Radar', // 毫米波雷达
    showName: i18n.global.t('desc.editObject.millimeterWaveRadar'),
    iconUrl: ImgUrls.sensor.radarIcon,
    textureUrl: ImgUrls.sensor.radar,
    modelUrl: ModelUrls.sensor.radar,
    type: 'none',
    subtype: '',
    placementAreaName: [],
  },
  {
    mainType: 'sensor',
    name: 'Lidar', // 激光雷达
    showName: i18n.global.t('desc.editObject.lidar'),
    iconUrl: ImgUrls.sensor.lidarIcon,
    textureUrl: ImgUrls.sensor.lidar,
    modelUrl: ModelUrls.sensor.lidar,
    type: 'none',
    subtype: '',
    placementAreaName: [],
  },
  {
    mainType: 'sensor',
    name: 'RSU', // 通信单元
    showName: i18n.global.t('desc.editObject.rsu'),
    iconUrl: ImgUrls.sensor.rsuIcon,
    textureUrl: ImgUrls.sensor.rsu,
    modelUrl: ModelUrls.sensor.rsu,
    type: 'none',
    subtype: '',
    placementAreaName: [],
  },
]
export const sensorConfig = _sensorConfig.filter((item) => {
  if (item.name === 'Camera') {
    return getPermission('action.mapEditor.models.sensor.camera.view.enable')
  }
  if (item.name === 'Millimeter_Wave_Radar') {
    return getPermission('action.mapEditor.models.sensor.radar.view.enable')
  }
  if (item.name === 'Lidar') {
    return getPermission('action.mapEditor.models.sensor.lidar.view.enable')
  }
  if (item.name === 'RSU') {
    return getPermission('action.mapEditor.models.sensor.rsu.view.enable')
  }
  return true
})

// 通过名称来获取配置
export function getSensorConfig (name: string) {
  for (const option of sensorConfig) {
    if (option.name === name) {
      return option
    }
  }
  return null
}

// 初始化预设的参数
function initPresetConfig () {
  for (const option of sensorConfig) {
    // 只能放置在杆上
    option.placementAreaName = [
      'horizontalPole',
      'verticalPole',
      'horizontalPolePlacementArea',
      'verticalPolePlacementArea',
    ]
  }
}
initPresetConfig()

// 每一类传感器的默认设备参数
// rsu
export function getRsuParams () {
  return {
    // 通用的属性
    JunctionIDs: [], // 关联的路口 id，支持多选
    // 云端版涉及的属性
    V2X_idx: '', // 绑定的云平台通信单元配置
    // 单机版涉及的属性
    FrequencySPAT: '10', // SPAT发送频率（Hz）
    FrequencyRSM: '10', // RSM发送频率（Hz）
    FrequencyRSI: '10', // RSI发送频率（Hz）
    FrequencyMAP: '10', // MAP发送频率（Hz）
    DistanceCommu: '300', // 通信距离（m）
    Band: '5905-5925', // 频段
    Mbps: '100', // 传输速率（Mbps）
    MaxDelay: '100', // 最大延时（ms）
    MapRadius: '1000', // 地图半径（m）
    TriggerImmediately: 'True', // 关键事件立即触发
    CongestionRegulation: 'True', // 拥堵调节
    V2N: 'False', // 云协同(V2N)
    V2I: 'False', // 车路协同(V2I)
    PreMSG: '', // 消息预设
  }
}

// 摄像头
export function getCameraParams () {
  return {
    // 通用属性
    BelongRSU: '', // 绑定的 rsu
    // 云端版涉及的属性
    V2X_idx: '', // 绑定的云平台摄像头配置
    // 单机版涉及的属性
    Frequency: '25', // 频率（Hz）
    ResHorizontal: '1920', // 水平分辨率
    ResVertical: '1208', // 垂直分辨率
    Distortion: {
      k1: '0',
      k2: '0',
      k3: '0',
      p1: '0',
      p2: '0',
    }, // 畸变参数
    IntrinsicType: '0', // 内参形式
    IntrinsicMat: {
      fx: '1945',
      skew: '0',
      cx: '946',
      param4: '0',
      fy: '1938',
      cy: '619',
      param7: '0',
      param8: '0',
      param9: '1',
    }, // 内参矩阵
    FovHorizontal: '60', // 水平Fov（°）
    FovVertical: '36.28', // 垂直Fov（°）
    CcdWidth: '10', // 感光器宽度（mm）
    CcdHeight: '10', // 感光器高度（mm）
    CcdFocal: '10', // 镜头焦距（mm）
    MaxDistance: '100', // 感知距离（m）
    Completeness: '50', // 遮挡最小比例（%）
    MinArea: '20', // 最小像素面积
  }
}

// 激光雷达
export function getLidarParams () {
  return {
    // 通用属性
    BelongRSU: '', // 绑定的 rsu
    // 云端版涉及的属性
    V2X_idx: '', // 绑定的云平台激光雷达配置
    // 单机版涉及的属性
    RayNum: '16', // 线数
    Radius: '150', // 半径范围（m）
    HorizontalRes: '1', // 水平分辨率
    FovUp: '10', // 上仰角（°）
    FovDown: '10', // 下仰角（°）
    FovStart: '0', // 起点角度（°）
    FovEnd: '360', // 结束角度（°）
    Completeness: '30', // 遮挡最小比例（%）
    MinHitNum: '5', // 最小光数
  }
}

// 毫米波雷达
export function getRadarParams () {
  return {
    // 通用属性
    BelongRSU: '', // 绑定的 rsu
    // 云端版涉及的属性
    V2X_idx: '', // 绑定的云平台毫米波雷达配置
    // 单机版涉及的属性
    F0_GHz: '77', // 雷达载频 77|24
    Pt_dBm: '10', // 发射功率（dBm）
    Gt_dBi: '15', // 发射天线增益（dBi）
    Gr_dBi: '15', // 接收天线增益（dBi）
    Ts_K: '300', // 等效噪声温度（K）
    Fn_dB: '12', // 接收机噪声系数（dB）
    L0_dB: '10', // 系统衰减常数（dB）
    SNR_min_dB: '18', // 最小可检测信噪比（dB）
    Delay: '0', // 目标延迟参数（ms）
    FovVertical: '10', // 垂直fov（°）
    FovHorizontal: '20', // 水平fov（°）
    ResHorizontal: '3', // 水平角度分辨率（°）
    ResVertical: '3', // 水平角度分辨率（°）
  }
}
