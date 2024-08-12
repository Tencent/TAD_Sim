/**
 * 升级旧版传感器数据
 * @author marsyu
 * @date 2020/08/24
 */
const path = require('path')
const fse = require('fs-extra')
const { UserDataPath } = require('../common/constants')

const nameMap = {
  camera: 'sensor.camera',
  fisheye: 'sensor.fisheye',
  semantic: 'sensor.semantic',
  depth: 'sensor.depth',
  lidar: 'sensor.lidar',
  radar: 'sensor.radar',
  truth: 'sensor.truth',
  imu: 'sensor.imu',
  gps: 'sensor.gps',
  ultrasonic: 'sensor.ultrasonic',
  obu: 'sensor.obu',
}

/**
 * 处理传感器名称
 * @param sensor
 */
function processName (sensor) {
  if (sensor.type in nameMap) {
    sensor.name = nameMap[sensor.type]
  }
}

const advancedInfoNameMap = {
  Frequency: 'sensor.Frequency',
  DisplayMode: 'sensor.DisplayMode',
  Res_Horizontal: 'sensor.Res_Horizontal',
  Res_Vertical: 'sensor.Res_Vertical',
  Blur_Intensity: 'sensor.Blur_Intensity',
  MotionBlur_Amount: 'sensor.MotionBlur_Amount',
  Vignette_Intensity: 'sensor.Vignette_Intensity',
  Noise_Intensity: 'sensor.Noise_Intensity',
  Distortion_Parameters: 'sensor.Distortion_Parameters',
  InsideParamGroup: 'sensor.InsideParam',
  Intrinsic_Matrix: 'sensor.Intrinsic_Matrix',
  FOV_Horizontal: 'sensor.FOV_Horizontal',
  FOV_Vertical: 'sensor.FOV_Vertical',
  CCD_Width: 'sensor.CCD_Width',
  CCD_Height: 'sensor.CCD_Height',
  Focal_Length: 'sensor.Focal_Length',
  DrawPoint: 'sensor.DrawPoint',
  DrawRay: 'sensor.DrawRay',
  Model: 'sensor.Model',
  uChannels: 'sensor.uChannels',
  uRange: 'sensor.uRange',
  uHorizontalResolution: 'sensor.Res_Horizontal',
  uUpperFov: 'sensor.uUpperFov',
  uLowerFov: 'sensor.uLowerFov',
  F0_GHz: 'sensor.F0_GHz',
  Pt_dBm: 'sensor.Pt_dBm',
  Gt_dBi: 'sensor.Gt_dBi',
  Gr_dBi: 'sensor.Gr_dBi',
  Ts_K: 'sensor.Ts_K',
  Fn_dB: 'sensor.Fn_dB',
  L0_dB: 'sensor.L0_dB',
  SNR_min_dB: 'sensor.SNR_min_dB',
  radar_angle: 'sensor.radar_angle',
  R_m: 'sensor.R_m',
  rcs: 'sensor.rcs',
  weather: 'sensor.weather',
  tag: 'sensor.tag',
  anne_tag: 'sensor.anne_tag',
  hwidth: 'sensor.hwidth',
  vwidth: 'sensor.vwidth',
  ANTENNA_ANGLE_path1: 'sensor.ANTENNA_ANGLE_path1',
  ANTENNA_ANGLE_path2: 'sensor.ANTENNA_ANGLE_path2',
  vfov: 'sensor.FOV_Vertical',
  hfov: 'sensor.FOV_Horizontal',
  delay: 'sensor.delay',
  drange: 'sensor.drange',
  Quaternion: 'sensor.Quaternion',
  dBmin: 'sensor.dBmin',
  Radius: 'sensor.Radius',
  NoiseFactor: 'sensor.NoiseFactor',
  NoiseStd: 'sensor.NoiseStd',
  AttachmentType: 'sensor.AttachmentType',
  AttachmentRange: 'sensor.AttachmentRange',
  ScanDistance: 'sensor.drange',
  ConnectionDistance: 'sensor.ConnectionDistance',
}

/**
 * 处理传感器高级属性
 * @param advancedInfo
 */
function processAdvancedInfo (advancedInfo) {
  Object.keys(advancedInfo).forEach((key) => {
    if (key in advancedInfoNameMap) {
      advancedInfo[key].name = advancedInfoNameMap[key]
    }
  })
}

/**
 * 升级预设的传感器配置
 */
function upgradePresetSensor () {
  const filePath = path.join(UserDataPath.scenario, './sensor_preset')
  if (fse.existsSync(filePath)) {
    try {
      const { content } = fse.readJsonSync(filePath)
      const presetSensorGroups = JSON.parse(content)
      presetSensorGroups.forEach((group) => {
        group.forEach((sensor) => {
          processName(sensor)
          if (sensor.advancedInfo) {
            processAdvancedInfo(sensor.advancedInfo)
          }
        })
      })
      fse.writeJsonSync(filePath, { content: JSON.stringify(presetSensorGroups) })
    } catch (e) {
      console.warn(e)
    }
  }
}

module.exports = {
  upgradePresetSensor,
}
