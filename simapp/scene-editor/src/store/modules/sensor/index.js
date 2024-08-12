import {
  DistortionParameterNames,
  FisheyeDistortionParameterNames,
  IntrinsicMatrixProps,
  LidarFrequencyOptions,
  ModelOptions,
  NoiseIntensityOptions,
  radarF0Options,
  v2xDelayTypeOptions,
  v2xLossTypeOptions,
  weathers,
} from './constants'
import * as actions from './actions'
import * as mutations from './mutations'

/**
 * 传感器状态定义
 */
export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    visible: false,
    loading: false,
    currentGroupIndex: '0',
    currentSensorIndex: 0,
    sensorGroups: [
      [],
      [],
      [],
      [],
      [],
      [],
    ],
    globalMode: false,
  },
  getters: {
    currentGroup (state) {
      return state.sensorGroups[state.currentGroupIndex] || []
    },
    currentSensor (state, { currentGroup }) {
      return currentGroup && currentGroup[state.currentSensorIndex]
    },
    // 噪声强度选项
    NoiseIntensityOptions: () => NoiseIntensityOptions,
    // 畸变参数名称
    DistortionParameterNames: () => DistortionParameterNames,
    // 鱼眼畸变参数名称
    FisheyeDistortionParameterNames: () => FisheyeDistortionParameterNames,
    // 内参矩阵属性
    IntrinsicMatrixProps: () => IntrinsicMatrixProps,
    // 传感器模型选项
    ModelOptions: () => ModelOptions,
    // 雷达频率选项
    LidarFrequencyOptions: () => LidarFrequencyOptions,
    // 天气参数选项
    weathers: () => weathers,
    // 雷达F0选项
    radarF0Options: () => radarF0Options,
    // V2X 丢包率和延迟类型选项
    v2xLossTypeOptions: () => v2xLossTypeOptions,
    v2xDelayTypeOptions: () => v2xDelayTypeOptions,
  },
  mutations,
  actions,
}
