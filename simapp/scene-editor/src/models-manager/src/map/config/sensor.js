import { ImgUrls, ModelUrls } from '../utils/urls'

// 路侧传感器
let sensorConfig = []

export function initSensorConfig () {
  sensorConfig = [
    {
      mainType: 'sensor',
      name: 'Camera', // 摄像头
      textureUrl: ImgUrls.sensor.camera,
      modelUrl: ModelUrls.sensor.camera,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'sensor',
      name: 'Millimeter_Wave_Radar', // 毫米波雷达
      textureUrl: ImgUrls.sensor.radar,
      modelUrl: ModelUrls.sensor.radar,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'sensor',
      name: 'RSU', // RSU
      textureUrl: ImgUrls.sensor.rsu,
      modelUrl: ModelUrls.sensor.rsu,
      type: 'none',
      subtype: '',
    },
    {
      mainType: 'sensor',
      name: 'Lidar', // 激光雷达
      textureUrl: ImgUrls.sensor.lidar,
      modelUrl: ModelUrls.sensor.lidar,
      type: 'none',
      subtype: '',
    },
  ]
}

// 通过名称来获取路侧传感器的配置
export function getSensorConfig (name) {
  for (const option of sensorConfig) {
    if (option.name === name) {
      return option
    }
  }
  return null
}
