import i18n from '@/locales'

/**
 * 基础属性
 * @type {{
 * RotationY: {unit: string, range: string, step: number, type: string, value: number},
 * RotationX: {unit: string, range: string, step: number, type: string, value: number},
 * LocationY: {unit: string, range: string, step: number, type: string, value: number},
 * LocationZ: {unit: string, range: string, step: number, type: string, value: number},
 * LocationX: {unit: string, range: string, step: number, type: string, value: number},
 * RotationZ: {unit: string, range: string, step: number, type: string, value: number}
 * }}
 */
export const baseInfo = {
  LocationX: {
    range: '[-2000,2000]',
    step: 0.01,
    value: 0,
    type: 'Number',
    unit: 'cm',
  },
  LocationY: {
    range: '[-2000,2000]',
    step: 0.01,
    value: 0,
    type: 'Number',
    unit: 'cm',
  },
  LocationZ: {
    range: '[0,2000]',
    step: 0.01,
    value: 171,
    type: 'Number',
    unit: 'cm',
  },
  RotationX: {
    range: '[-180,180]',
    step: 0.01,
    value: 0,
    type: 'Number',
    unit: '°',
  },
  RotationY: {
    range: '[-180,180]',
    step: 0.01,
    value: 0,
    type: 'Number',
    unit: '°',
  },
  RotationZ: {
    range: '[-180,180]',
    step: 0.01,
    value: 0,
    type: 'Number',
    unit: '°',
  },
}

/**
 * 相机高级属性
 * @type {{
 * Res_Horizontal: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Aperture: {name: string, range: string, step: number, type: string, value: number},
 * Noise_Intensity: {name: string, range: string, step: number, type: string, value: number},
 * InsideParamGroup: {name: string, type: string, value: number},
 * ColorTemperature: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * ExposureMode: {name: string, type: string, value: number},
 * Compensation: {name: string, range: string, step: number, type: string, value: number},
 * Exquisite: {name: string, range: string, step: number, type: string, value: number},
 * Vignette_Intensity: {name: string, range: string, step: number, type: string, value: number},
 * Intrinsic_Matrix: {name: string, range: string, step: number, type: string, value: number[]},
 * CCD_Width: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * Distortion_Parameters: {name: string, range: string, step: number, type: string, value: number[]},
 * ISO: {name: string, range: string, step: number, type: string, value: number},
 * FOV_Vertical: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * Blur_Intensity: {name: string, range: string, step: number, type: string, value: number},
 * LensFlares: {name: string, range: string, step: number, type: string, value: number},
 * Transmittance: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Res_Vertical: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * DisplayMode: {name: string, type: string, value: number},
 * FOV_Horizontal: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * Focal_Length: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * MotionBlur_Amount: {name: string, range: string, step: number, type: string, value: number},
 * Bloom: {name: string, range: string, step: number, type: string, value: number},
 * Frequency: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * ShutterSpeed: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * CCD_Height: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * WhiteHint: {name: string, range: string, step: number, type: string, value: number}
 * }}
 */
export const cameraAdvancedInfo = {
  Frequency: {
    range: '[0,1000]',
    step: 0.01,
    value: 25,
    unit: 'Hz',
    type: 'Number',
    name: 'sensor.Frequency',
  },
  DisplayMode: {
    /**
     * @property {0|1} value 0: color, 1: gray
     */
    value: 0,
    type: 'Number',
    name: 'sensor.DisplayMode',
  },
  Res_Horizontal: {
    range: '[1,10000]',
    step: 1,
    value: 1920,
    unit: 'px',
    type: 'Number',
    name: 'sensor.Res_Horizontal',
  },
  Res_Vertical: {
    range: '[1,10000]',
    step: 1,
    value: 1208,
    unit: 'px',
    type: 'Number',
    name: 'sensor.Res_Vertical',
  },
  Blur_Intensity: {
    range: '[0,1]',
    step: 0.01,
    value: 0,
    type: 'Number',
    name: 'sensor.Blur_Intensity',
  },
  MotionBlur_Amount: {
    range: '[0,1]',
    step: 0.01,
    value: 0,
    type: 'Number',
    name: 'sensor.MotionBlur_Amount',
  },
  Vignette_Intensity: {
    range: '[0,1]',
    step: 0.1,
    value: 0.4,
    type: 'Number',
    name: 'sensor.Vignette_Intensity',
  },
  Noise_Intensity: {
    range: '[0,1]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.Noise_Intensity',
  },
  Distortion_Parameters: {
    range: '[-65535,65535]',
    step: 0.000000001,
    value: [
      -0.5333680427825355,
      0.4577346209269804,
      -0.4836076527200365,
      -0.0023929586859060227,
      0.0018364297357014352,
    ],
    type: 'Array',
    name: 'sensor.Distortion_Parameters',
  },
  InsideParamGroup: {
    value: 0, // 0 - 矩阵，1 - fov，2 - 感光器
    type: 'Number',
    name: 'sensor.InsideParam',
  }, // 以下参数中的 InsideParamGroup === InsideParamGroup 时，可以进行编辑，否则disabled
  Intrinsic_Matrix: {
    range: '[-5000000,5000000]',
    step: 0.000000001,
    value: [1945.1674168728503, 0, 946.1188960408923, 0, 1938.137228006907, 619.7048547473978, 0, 0, 1],
    type: 'Array',
    name: 'sensor.Intrinsic_Matrix',
  },

  FOV_Horizontal: {
    range: '[0,170]',
    step: 0.01,
    value: 60,
    unit: '°',
    InsideParamGroup: 1,
    type: 'Number',
    name: 'sensor.FOV_Horizontal',
  },
  FOV_Vertical: {
    range: '[0,170]',
    step: 0.01,
    value: 36.28,
    unit: '°',
    InsideParamGroup: 1,
    type: 'Number',
    name: 'sensor.FOV_Vertical',
  },
  CCD_Width: {
    range: '(0,10000]',
    step: 0.01,
    value: 10,
    unit: 'mm',
    InsideParamGroup: 2,
    type: 'Number',
    name: 'sensor.CCD_Width',
  },
  CCD_Height: {
    range: '(0,10000]',
    step: 0.01,
    value: 10,
    unit: 'mm',
    InsideParamGroup: 2,
    type: 'Number',
    name: 'sensor.CCD_Height',
  },
  Focal_Length: {
    range: '(0,10000]',
    step: 0.01,
    value: 10,
    unit: 'mm',
    InsideParamGroup: 2,
    type: 'Number',
    name: 'sensor.Focal_Length',
  },
  Exquisite: {
    range: '[-5,5]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.exquisite',
  },
  LensFlares: {
    range: '[0,1]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.LensFlares',
  },
  Bloom: {
    range: '[0,1]',
    step: 0.001,
    value: 0.675,
    type: 'Number',
    name: 'sensor.Bloom',
  },
  ExposureMode: {
    value: 0,
    type: 'Number',
    name: 'sensor.ExposureMode',
  },
  Compensation: {
    range: '[-15,15]',
    step: 0.1,
    value: 1,
    type: 'Number',
    name: 'sensor.Compensation',
  },
  ShutterSpeed: {
    range: '[1,2000]',
    step: 0.1,
    value: 60,
    unit: '1/s',
    type: 'Number',
    name: 'sensor.ShutterSpeed',
  },
  ISO: {
    range: '[1,400000]',
    step: 0.1,
    value: 100,
    type: 'Number',
    name: 'sensor.ISO',
  },
  Aperture: {
    range: '[1,32]',
    step: 0.1,
    value: 4,
    type: 'Number',
    name: 'sensor.Aperture',
  },
  ColorTemperature: {
    range: '[1500,15000]',
    step: 1,
    value: 6500,
    unit: 'k',
    type: 'Number',
    name: 'sensor.ColorTemperature',
  },
  WhiteHint: {
    range: '[-1,1]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.WhiteHint',
  },
  Transmittance: {
    range: '[1,100]',
    step: 0.1,
    value: 98,
    unit: '%',
    type: 'Number',
    name: 'sensor.Transmittance',
  },
}

/**
 * @type {[
 * {label: string, value: string}
 * ]}
 */
export const NoiseIntensityOptions = [
  {
    value: 'Low',
    label: 'sensor.Low',
  },
  {
    value: 'Middle',
    label: 'sensor.Middle',
  },
  {
    value: 'High',
    label: 'sensor.High',
  },
  {
    value: 'Off',
    label: 'none',
  },
]

export const DistortionParameterNames = ['k1', 'k2', 'k3', 'p1', 'p2']

/**
 * @type {[
 * {readonly: boolean, name: string}
 * ]}
 */
export const IntrinsicMatrixProps = [
  {
    name: 'fx',
    readonly: false,
  },
  {
    name: 'skew',
    readonly: false,
  },
  {
    name: 'cx',
    readonly: false,
  },
  {
    name: '',
    readonly: true,
  },
  {
    name: 'fy',
    readonly: false,
  },
  {
    name: 'cy',
    readonly: false,
  },
  {
    name: '',
    readonly: true,
  },
  {
    name: '',
    readonly: true,
  },
  {
    name: '',
    readonly: true,
  },
]

/**
 * @type {{
 * uChannels: {name: string, range: string, step: number, type: string, value: number},
 * ReflectionType: {name: string, type: string, value: number},
 * uHorizontalResolution: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * uUpperFov: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * IP: {name: string, type: string, value: string},
 * Port: {name: string, range: string, step: number, type: string, value: number[]},
 * AngleDefinition: {name: string, type: string, value: string},
 * FovEnd: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * uLowerFov: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * ExtinctionCoe: {name: string, range: string, step: number, type: string, value: number},
 * DrawPoint: {name: string, type: string, value: boolean},
 * uRange: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * DrawRay: {name: string, type: string, value: boolean},
 * Model: {name: string, type: string, value: string},
 * Frequency: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Attenuation: {name: string, range: string, step: number, type: string, value: number},
 * FovStart: {unit: string, name: string, range: string, step: number, type: string, value: number}
 * }}
 */
export const lidarAdvancedInfo = {
  Frequency: {
    range: '[1,128]',
    step: 1,
    value: 5,
    unit: 'Hz',
    type: 'Number',
    name: 'sensor.Frequency',
  },
  DrawPoint: {
    value: true,
    type: 'Boolean',
    name: 'sensor.DrawPoint',
  },
  DrawRay: {
    value: true,
    type: 'Boolean',
    name: 'sensor.DrawRay',
  },
  Model: {
    /**
     * @property value 型号
     * 'RS32'|'Velodyne32'|'HS40'|'RS16'|'RSBpearl'|'RSHelios'|'RSRuby'|'HS128'|'HS64'|'RSM1'|'RSM1P'|'User'
     */
    value: 'RS32',
    type: 'String',
    name: 'sensor.Model',
  },
  // 以下当Model = User时可用
  uChannels: {
    range: '[1,320]',
    step: 1,
    value: 16,
    type: 'Number',
    name: 'sensor.uChannels',
  },
  uRange: {
    range: '(0,1000]',
    step: 0.01,
    value: 150,
    unit: 'm',
    type: 'Number',
    name: 'sensor.uRange',
  },
  uHorizontalResolution: {
    range: '(0,360]',
    step: 0.01,
    value: 1,
    unit: '°',
    type: 'Number',
    name: 'sensor.Res_Horizontal',
  },
  uUpperFov: {
    range: '(0,90]',
    step: 0.01,
    value: 10,
    unit: '°',
    type: 'Number',
    name: 'sensor.uUpperFov',
  },
  uLowerFov: {
    range: '(0,90]',
    step: 0.01,
    value: 10,
    unit: '°',
    type: 'Number',
    name: 'sensor.uLowerFov',
  },
  IP: {
    value: '127.0.0.1',
    type: 'String',
    name: 'sensor.IP',
  },
  Port: {
    range: '[1025,60000]',
    step: 1,
    type: 'Array',
    value: [2000, 2001],
    name: 'sensor.Port',
  },
  Attenuation: {
    range: '[0,1)',
    step: 0.00000001,
    value: 0,
    type: 'Number',
    name: 'sensor.Attenuation',
  },
  AngleDefinition: {
    value: '',
    type: 'File',
    name: 'sensor.AngleDefinition',
  },
  ExtinctionCoe: {
    range: '[0,1]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.ExtinctionCoe',
  },
  ReflectionType: {
    /**
     * @prototype {0:反射率|1:tag值|2:衰减值}
     */
    value: 0,
    type: 'Number',
    name: 'sensor.ReflectionType',
  },
  FovStart: {
    range: '(0,360]',
    step: 0.1,
    value: 0,
    unit: '°',
    type: 'Number',
    name: 'sensor.FovStart',
  },
  FovEnd: {
    range: '(0,360]',
    step: 0.1,
    value: 360,
    unit: '°',
    type: 'Number',
    name: 'sensor.FovEnd',
  },
}

/**
 * @type {[
 * {label: string, value: string}
 * ]}
 */
export const ModelOptions = [
  {
    value: 'HS32',
    label: 'HS32',
  },
  {
    value: 'HS40',
    label: 'HS40',
  },
  {
    value: 'HS64',
    label: 'HS64',
  },
  {
    value: 'HS64QT',
    label: 'HS64QT',
  },
  {
    value: 'HS128',
    label: 'HS128',
  },
  {
    value: 'HS128AT',
    label: 'HS128AT',
  },
  // {
  //   value: 'HS120FT',
  //   label: 'HS120FT',
  // },
  {
    value: 'RS16',
    label: 'RS16',
  },
  {
    value: 'RS32',
    label: 'RS32',
  },
  {
    value: 'RSBpearl',
    label: 'RSBpearl',
  },
  {
    value: 'RSHelios',
    label: 'RSHelios',
  },
  {
    value: 'RSM1',
    label: 'RSM1',
  },
  {
    value: 'RSM1P',
    label: 'RSM1P',
  },
  {
    value: 'RSRuby',
    label: 'RSRuby',
  },
  {
    value: 'Velodyne32',
    label: 'Velodyne32',
  },
  {
    value: 'User',
    label: i18n.t('indicator.Customize'),
  },
]

/**
 * @type {[{label: string, value: number}]}
 */
export const LidarFrequencyOptions = [
  {
    value: 5,
    label: '5',
  },
  {
    value: 10,
    label: '10',
  },
  {
    value: 20,
    label: '20',
  },
]

/*
* 固态激光雷达:
* RSM1，视角（垂直）：25°（-12.5°~+12.5°），视角（水平）：120°（-60.0°~+60.0°），测距：200m
* HS128AT，垂直视场角 25.4°（-12.5° ~ +12.9°），水平视场角 120°，测距能力 180 m
* */
export const SolidStateLidarMap = {
  RSM1: {
    upperFov: 12.5,
    lowerFov: -12.5,
    leftFov: 60,
    rightFov: -60,
    far: 200,
  },
  RSM1P: {
    upperFov: 12.5,
    lowerFov: -12.5,
    leftFov: 60,
    rightFov: -60,
    far: 200,
  },
  HS128AT: {
    upperFov: 12.5,
    lowerFov: -12.9,
    leftFov: 60,
    rightFov: -60,
    far: 180,
  },
}

/**
 * 毫米波雷达高级属性
 * @type {{
 * vfov: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * R_m: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * rcs: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * SNR_min_dB: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * F0_GHz: {unit: string, name: string, step: number, type: string, value: number},
 * Pt_dBm: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * hwidth: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * vwidth: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Gr_dBi: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Ts_K: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Gt_dBi: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * ANTENNA_ANGLE_path1: {name: string, type: string, value: string},
 * anne_tag: {name: string, type: string, value: number},
 * ANTENNA_ANGLE_path2: {name: string, type: string, value: string},
 * delay: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * hfov: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Fn_dB: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * weather: {name: string, type: string, value: number},
 * Frequency: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * tag: {name: string, type: string, value: number},
 * radar_angle: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * L0_dB: {unit: string, name: string, range: string, type: string, value: number}
 * }}
 */
export const radarAdvancedInfo = {
  Frequency: {
    range: '(0,200]',
    step: 0.01,
    value: 10,
    unit: 'Hz',
    type: 'Number',
    name: 'sensor.Frequency',
  },
  F0_GHz: {
    value: 77, // 77|79|24
    step: 0.01,
    unit: 'GHz',
    type: 'Number',
    name: 'sensor.F0_GHz',
  },
  Pt_dBm: {
    range: '[5,10]',
    step: 0.01,
    value: 10,
    unit: 'dBm',
    type: 'Number',
    name: 'sensor.Pt_dBm',
  },
  Gt_dBi: {
    range: '[10,15]',
    step: 0.01,
    value: 15,
    unit: 'dBi',
    type: 'Number',
    name: 'sensor.Gt_dBi',
  },
  Gr_dBi: {
    range: '[10,15]',
    step: 0.01,
    value: 15,
    unit: 'dBi',
    type: 'Number',
    name: 'sensor.Gr_dBi',
  },
  Ts_K: {
    range: '[290,310]',
    step: 0.01,
    value: 300,
    unit: 'K',
    type: 'Number',
    name: 'sensor.Ts_K',
  },
  Fn_dB: {
    range: '[0,45]',
    step: 0.01,
    value: 12,
    unit: 'dB',
    type: 'Number',
    name: 'sensor.Fn_dB',
  },
  L0_dB: {
    range: '[5,10]',
    value: 10, // 联动F0_GHz 77、79：[5,10]/ 24:[15,30]
    unit: 'dB',
    type: 'Number',
    name: 'sensor.L0_dB',
  },
  SNR_min_dB: {
    range: '[15,22]',
    step: 0.01,
    value: 18,
    unit: 'dB',
    type: 'Number',
    name: 'sensor.SNR_min_dB',
  },
  radar_angle: {
    range: '[-10,10]',
    step: 0.01,
    value: 0,
    unit: '°',
    type: 'Number',
    name: 'sensor.radar_angle',
  },
  R_m: {
    range: '[0,1000]',
    step: 0.01,
    value: 50.00,
    unit: 'm',
    type: 'Number',
    name: 'sensor.R_m',
  },
  rcs: {
    range: '[0,1000]',
    step: 0.01,
    value: 7.9,
    unit: '㎡',
    type: 'Number',
    name: 'sensor.rcs',
  },
  weather: {
    value: 0,
    type: 'Number',
    name: 'sensor.weather',
  },
  tag: {
    value: 0, // 0-MFSK波形，1-FC波形
    type: 'Number',
    name: 'sensor.tag',
  },
  anne_tag: {
    value: 1, // 0-文件，1-fov(hwidth,hwidth)
    type: 'Number',
    name: 'sensor.anne_tag',
  },
  hwidth: {
    range: '[1,10]',
    step: 0.01,
    value: 3,
    unit: '°',
    type: 'Number',
    name: 'sensor.hwidth',
  },
  vwidth: {
    range: '[1,10]',
    step: 0.01,
    value: 3,
    unit: '°',
    type: 'Number',
    name: 'sensor.vwidth',
  },
  ANTENNA_ANGLE_path1: {
    value: '',
    type: 'String',
    name: 'sensor.ANTENNA_ANGLE_path1',
  },
  ANTENNA_ANGLE_path2: {
    value: '',
    type: 'String',
    name: 'sensor.ANTENNA_ANGLE_path2',
  },
  vfov: {
    range: '[0,160]',
    step: 0.01,
    value: 10,
    unit: '°',
    type: 'Number',
    name: 'sensor.FOV_Vertical',
  },
  hfov: {
    range: '[0,160]',
    step: 0.01,
    value: 20,
    unit: '°',
    type: 'Number',
    name: 'sensor.FOV_Horizontal',
  },
  delay: {
    range: '[0,5000]',
    step: 1,
    value: 0,
    unit: 'ms',
    type: 'Number',
    name: 'sensor.delay',
  },
}

/**
 * @type {[{label: string, value: number, key: number}]}
 */
export const weathers = [
  {
    key: 0,
    label: 'sensor.weathers_a',
    value: 0.62,
  },
  {
    key: 1,
    label: 'sensor.weathers_b',
    value: 2.28,
  },
  {
    key: 2,
    label: 'sensor.weathers_c',
    value: 6.66,
  },
  {
    key: 3,
    label: 'sensor.weathers_d',
    value: 11.02,
  },
  {
    key: 4,
    label: 'sensor.weathers_e',
    value: 18.52,
  },
  {
    key: 5,
    label: 'sensor.weathers_f',
    value: 2.04,
  },
  {
    key: 6,
    label: 'sensor.weathers_g',
    value: 5.85,
  },
  {
    key: 7,
    label: 'sensor.weathers_h',
    value: 9.12,
  },
  {
    key: 8,
    label: 'sensor.weathers_i',
    value: 3.18,
  },
  {
    key: 9,
    label: 'sensor.weathers_j',
    value: 8.47,
  },
  {
    key: 10,
    label: 'sensor.weather_k',
    value: 13.72,
  },
  {
    key: 11,
    label: 'sensor.weather_l',
    value: 1.16,
  },
]

/**
 * @type {[{label: string, value: number}]}
 */
export const radarF0Options = [
  {
    label: '77',
    value: 77,
  },
  {
    label: '79',
    value: 79,
  },
  {
    label: '24',
    value: 24,
  },
]

/**
 * 真值传感器高级参数
 * @type {{
 * vfov: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * drange: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * hfov: {unit: string, name: string, range: string, step: number, type: string, value: number}
 * }}
 */
export const truthAdvancedInfo = {
  vfov: {
    range: '(0,180]',
    step: 0.01,
    value: 10,
    unit: '°',
    type: 'Number',
    name: 'sensor.FOV_Vertical',
  },
  hfov: {
    range: '(0,360]',
    step: 0.01,
    value: 20,
    unit: '°',
    type: 'Number',
    name: 'sensor.FOV_Horizontal',
  },
  drange: {
    // 保存到文件里面要转换成cm
    range: '(0,1000]',
    step: 0.01,
    value: 200,
    unit: 'm',
    type: 'Number',
    name: 'sensor.drange',
  },
}

/**
 * IMU高级参数
 * @type {{Quaternion: {name: string, range: string, type: string, value: number[]}}}
 */
export const IMUAdvancedInfo = {
  Quaternion: {
    range: '[-1,1]',
    value: [0, 0, -0.70710678118, 0.70710678118],
    type: 'Array',
    name: 'sensor.Quaternion',
  },
}

/**
 * GPS高级参数
 * @type {{Quaternion: {name: string, range: string, type: string, value: number[]}}}
 */
export const GPSAdvancedInfo = {
  Quaternion: {
    range: '[-1,1]',
    value: [0, 0, 0, 0],
    type: 'Array',
    name: 'sensor.Quaternion',
  },
}

/**
 * 超声波高级参数
 * @type {{
 * Radius: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * PulseMoment: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * AttachmentType: {name: string, type: string, value: string},
 * NoiseFactor: {name: string, range: string, step: number, type: string, value: number},
 * FOV_Vertical: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * IndirectDistance: {name: string, type: string, value: number},
 * PollTurn: {name: string, type: StringConstructor, value: string},
 * AttachmentRange: {name: string, range: string, step: number, type: string, value: number},
 * FOV_Horizontal: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * dBmin: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * NoiseStd: {name: string, range: string, step: number, type: string, value: number},
 * PulsePeriod: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Frequency: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Distance: {unit: string, name: string, range: string, step: number, type: string, value: number}
 * }}
 */
export const ultrasonicAdvancedInfo = {
  Frequency: {
    range: '[4,250000]',
    step: 1,
    value: 40000,
    unit: 'Hz',
    type: 'Number',
    name: 'sensor.Frequency',
  },
  FOV_Horizontal: {
    range: '[0,160]',
    step: 0.01,
    value: 60,
    unit: '°',
    InsideParamGroup: 1,
    type: 'Number',
    name: 'sensor.FOV_Horizontal',
  },
  FOV_Vertical: {
    range: '[0,160]',
    step: 0.01,
    value: 30,
    unit: '°',
    InsideParamGroup: 1,
    type: 'Number',
    name: 'sensor.FOV_Vertical',
  },
  dBmin: {
    range: '[-1000,0]',
    step: 0.01,
    value: -6,
    unit: 'dB',
    type: 'Number',
    name: 'sensor.dBmin',
  },
  Radius: {
    range: '[0.001,0.3]',
    step: 0.001,
    value: 0.01,
    unit: 'm',
    type: 'Number',
    name: 'sensor.Radius',
  },
  NoiseFactor: {
    range: '(0.2,10]',
    step: 0.1,
    value: 5,
    type: 'Number',
    name: 'sensor.NoiseFactor',
  },
  NoiseStd: {
    range: '[0.2,10]',
    step: 0.1,
    value: 5,
    type: 'Number',
    name: 'sensor.NoiseStd',
  },
  AttachmentType: {
    /**
     * @property {'None'|'Water'|'Mud'} value 附着物类型
     */
    value: 'None',
    type: 'String',
    name: 'sensor.AttachmentType',
  },
  AttachmentRange: {
    range: '[0,1]',
    step: 0.1,
    value: 0.5,
    type: 'Number',
    name: 'sensor.AttachmentRange',
  },
  Distance: {
    range: '[0.001,100]',
    step: 0.001,
    value: 5,
    unit: 'm',
    type: 'Number',
    name: 'sensor.drange',
  },
  IndirectDistance: {
    value: 0,
    type: 'Number',
    name: 'sensor.IndirectDistance',
  },
  PulseMoment: {
    range: '[0,100]',
    step: 0.1,
    value: 0,
    unit: 'ms',
    type: 'Number',
    name: 'sensor.PulseMoment',
  },
  PulsePeriod: {
    range: '[0.1,100]',
    step: 0.1,
    value: 100,
    unit: 'ms',
    type: 'Number',
    name: 'sensor.PulsePeriod',
  },
  PollTurn: {
    value: '',
    type: String,
    name: 'sensor.PollTurn',
  },
}

/**
 * 附着物类型
 * @type {[{label: string, value: string},{label: string, value: string},{label: string, value: string}]}
 */
export const AttachmentTypeOptions = [
  {
    value: 'None',
    label: 'none',
  },
  {
    value: 'Water',
    label: 'Water',
  },
  {
    value: 'Mud',
    label: 'Mud',
  },
]

/**
 * 鱼眼相机高级参数
 * @type {{
 * ISO: {name: string, range: string, step: number, type: string, value: number},
 * Blur_Intensity: {name: string, range: string, step: number, type: string, value: number},
 * Res_Horizontal: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Aperture: {name: string, range: string, step: number, type: string, value: number},
 * LensFlares: {name: string, range: string, step: number, type: string, value: number},
 * Transmittance: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Res_Vertical: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Noise_Intensity: {name: string, range: string, step: number, type: string, value: number},
 * InsideParamGroup: {name: string, type: string, value: number},
 * DisplayMode: {name: string, type: string, value: number},
 * ColorTemperature: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * ExposureMode: {name: string, type: string, value: number},
 * Compensation: {name: string, range: string, step: number, type: string, value: number},
 * MotionBlur_Amount: {name: string, range: string, step: number, type: string, value: number},
 * Bloom: {name: string, range: string, step: number, type: string, value: number},
 * Frequency: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Exquisite: {name: string, range: string, step: number, type: string, value: number},
 * ShutterSpeed: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Vignette_Intensity: {name: string, range: string, step: number, type: string, value: number},
 * Intrinsic_Matrix: {name: string, range: string, step: number, type: string, value: number[]},
 * WhiteHint: {name: string, range: string, step: number, type: string, value: number},
 * Distortion_Parameters: {name: string, range: string, step: number, type: string, value: number[]}
 * }}
 */
export const fisheyeAdvancedInfo = {
  Frequency: {
    range: '[0,1000]',
    step: 0.01,
    value: 25,
    unit: 'Hz',
    type: 'Number',
    name: 'sensor.Frequency',
  },
  DisplayMode: {
    /**
     * @property {0|1} value 0: color, 1: gray
     */
    value: 0,
    type: 'Number',
    name: 'sensor.DisplayMode',
  },
  Res_Horizontal: {
    range: '[1,10000]',
    step: 1,
    value: 1200,
    unit: 'px',
    type: 'Number',
    name: 'sensor.Res_Horizontal',
  },
  Res_Vertical: {
    range: '[1,10000]',
    step: 1,
    value: 1080,
    unit: 'px',
    type: 'Number',
    name: 'sensor.Res_Vertical',
  },
  Blur_Intensity: {
    range: '[0,1]',
    step: 0.01,
    value: 0,
    type: 'Number',
    name: 'sensor.Blur_Intensity',
  },
  MotionBlur_Amount: {
    range: '[0,1]',
    step: 0.01,
    value: 0,
    type: 'Number',
    name: 'sensor.MotionBlur_Amount',
  },
  Vignette_Intensity: {
    range: '[0,1]',
    step: 0.01,
    value: 0.4,
    type: 'Number',
    name: 'sensor.Vignette_Intensity',
  },
  Noise_Intensity: {
    // value: 'Low',
    range: '[0,1]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.Noise_Intensity',
  },
  Distortion_Parameters: {
    range: '[-65535,65535]',
    step: 0.000000001,
    value: [0.00298329, 0.04234368, -0.00231007, -0.0028992],
    type: 'Array',
    name: 'sensor.Distortion_Parameters',
  },
  InsideParamGroup: {
    value: 0, // 0 - 矩阵
    type: 'Number',
    name: 'sensor.InsideParam',
  },
  // 以下参数中的 InsideParamGroup === InsideParamGroup 时，可以进行编辑，否则disabled
  Intrinsic_Matrix: {
    range: '[-5000000,5000000]',
    step: 0.000000001,
    value: [337.14139734, 0, 641.86019172, 0, 336.24194116, 543.23107903, 0, 0, 1],
    type: 'Array',
    name: 'sensor.Intrinsic_Matrix',
  },
  Exquisite: {
    range: '[-5,5]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.exquisite',
  },
  LensFlares: {
    range: '[0,1]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.LensFlares',
  },
  Bloom: {
    range: '[0,1]',
    step: 0.001,
    value: 0.675,
    type: 'Number',
    name: 'sensor.Bloom',
  },
  ExposureMode: {
    /**
     * @prototy: {0:自动|1:手动}
     */
    value: 0,
    type: 'Number',
    name: 'sensor.ExposureMode',
  },
  Compensation: {
    range: '[-15,15]',
    step: 0.1,
    value: 1,
    type: 'Number',
    name: 'sensor.Compensation',
  },
  ShutterSpeed: {
    range: '[1,2000]',
    step: 0.1,
    value: 60,
    unit: '1/s',
    type: 'Number',
    name: 'sensor.ShutterSpeed',
  },
  ISO: {
    range: '[1,400000]',
    step: 0.1,
    value: 100,
    type: 'Number',
    name: 'sensor.ISO',
  },
  Aperture: {
    range: '[1,32]',
    step: 0.1,
    value: 4,
    type: 'Number',
    name: 'sensor.Aperture',
  },
  ColorTemperature: {
    range: '[1500,15000]',
    step: 1,
    value: 6500,
    unit: 'k',
    type: 'Number',
    name: 'sensor.ColorTemperature',
  },
  WhiteHint: {
    range: '[-1,1]',
    step: 0.1,
    value: 0,
    type: 'Number',
    name: 'sensor.WhiteHint',
  },
  Transmittance: {
    range: '[1,100]',
    step: 0.1,
    value: 98,
    unit: '%',
    type: 'Number',
    name: 'sensor.Transmittance',
  },
}

export const FisheyeDistortionParameterNames = ['k1', 'k2', 'k3', 'k4']

export const QuaternionNames = ['qx', 'qy', 'qz', 'qw']

/**
 * 设备选项
 * @type {[{label: TranslateResult, value: string}]}
 */
export const deviceOptions = [
  {
    value: '',
    label: i18n.t('sensor.frontGpu'),
  },
  {
    value: '0',
    label: 'GPU 0',
  },
  {
    value: '1',
    label: 'GPU 1',
  },
  {
    value: '2',
    label: 'GPU 2',
  },
  {
    value: '3',
    label: 'GPU 3',
  },
  {
    value: '4',
    label: 'GPU 4',
  },
  {
    value: '5',
    label: 'GPU 5',
  },
  {
    value: '6',
    label: 'GPU 6',
  },
  {
    value: '7',
    label: 'GPU 7',
  },
]

/**
 * OBU 高级选项
 * @type {{
 * PreBSM: {name: string, type: string, value: string},
 * TriggerImmediately: {name: string, type: string, value: string},
 * SystemDelay: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * NoTeam: {name: string, type: string, value: string},
 * PosAccuracy: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * FrequencyBSM: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Band: {name: string, type: string, value: string},
 * CommuDelay: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * DistanceCity: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Mbps: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * DistanceOpen: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * DisableRSU: {name: string, type: string, value: string}
 * }}
 */
export const OBUAdvancedInfo = {
  FrequencyBSM: {
    range: '[0,100]',
    step: 1,
    value: 10,
    unit: 'Hz',
    type: 'Number',
    name: 'sensor.FrequencyBSM',
  },
  DistanceOpen: {
    range: '[100,1000]',
    step: 1,
    value: 800,
    unit: 'm',
    type: 'Number',
    name: 'sensor.DistanceOpen',
  },
  DistanceCity: {
    range: '[10,500]',
    step: 1,
    value: 300,
    unit: 'm',
    type: 'Number',
    name: 'sensor.DistanceCity',
  },
  Band: {
    value: '5905-5925',
    type: 'String',
    name: 'sensor.Band',
  },
  Mbps: {
    range: '[0.1,1000]',
    step: 0.1,
    value: 10,
    unit: 'Mbps',
    type: 'Number',
    name: 'sensor.Mbps',
  },
  SystemDelay: {
    range: '[0,1000]',
    step: 0.1,
    value: 10,
    unit: 'ms',
    type: 'Number',
    name: 'sensor.SystemDelay',
  },
  CommuDelay: {
    range: '[0,1000]',
    step: 0.1,
    value: 20,
    unit: 'ms',
    type: 'Number',
    name: 'sensor.CommuDelay',
  },
  TriggerImmediately: {
    value: 'True',
    type: 'String',
    name: 'sensor.TriggerImmediately',
  },
  PosAccuracy: {
    range: '[0.1,10]',
    step: 0.1,
    value: 0.5,
    unit: 'm',
    type: 'Number',
    name: 'sensor.PosAccuracy',
  },
  NoTeam: {
    value: 'True',
    type: 'String',
    name: 'sensor.NoTeam',
  },
  DisableRSU: {
    value: 'True',
    type: 'String',
    name: 'sensor.DisableRSU',
  },
  PreBSM: {
    value: '',
    type: 'String',
    name: 'sensor.PreBSM',
  },
  // Frequency: {
  //   range: '[1,1000000]',
  //   step: 1,
  //   value: 30,
  //   unit: 'Hz',
  //   type: 'Number',
  //   name: 'sensor.Frequency',
  // },
  // ConnectionDistance: {
  //   range: '[100,300]',
  //   step: 1,
  //   value: 300,
  //   unit: 'm',
  //   type: 'Number',
  //   name: 'sensor.ConnectionDistance',
  // },
  // v2x_loss_type: {
  //   /**
  //    * @property {0|1|2} value
  //    */
  //   value: 0,
  //   type: 'Number',
  //   name: 'sensor.v2x_loss_type',
  // },
  // v2x_loss_rand_prob: {
  //   range: '[0,100]',
  //   step: 0.01,
  //   value: 1,
  //   unit: '%',
  //   type: 'Number',
  //   name: 'sensor.v2x_loss_rand_prob',
  // },
  // v2x_loss_burs_prob: {
  //   range: '[0,100]',
  //   step: 0.01,
  //   value: 1,
  //   unit: '%',
  //   type: 'Number',
  //   name: 'sensor.v2x_loss_burs_prob',
  // },
  // v2x_loss_burs_min: {
  //   range: '[0,1000000000]',
  //   step: 1,
  //   value: 1,
  //   type: 'Number',
  //   name: 'sensor.v2x_loss_burs_min',
  // },
  // v2x_loss_burs_max: {
  //   range: '[0,1000000000]',
  //   step: 1,
  //   value: 5,
  //   type: 'Number',
  //   name: 'sensor.v2x_loss_burs_max',
  // },
  // v2x_bandwidth: {
  //   range: '[0,100]',
  //   step: 1,
  //   value: 10,
  //   unit: 'MHz',
  //   type: 'Number',
  //   name: 'sensor.v2x_bandwidth',
  // },
  // v2x_freq_channel: {
  //   range: '[0,100]',
  //   step: 0.1,
  //   value: 5.9,
  //   unit: 'GHz',
  //   type: 'Number',
  //   name: 'sensor.v2x_freq_channel',
  // },
  // v2x_broad_speed: {
  //   range: '[0,10]',
  //   step: 0.01,
  //   value: 1,
  //   unit: 'Mbps',
  //   type: 'Number',
  //   name: 'sensor.v2x_broad_speed',
  // },
  // v2x_delay_type: {
  //   /**
  //    * @property {0|1|2|3} value
  //    */
  //   value: 0,
  //   type: 'Number',
  //   name: 'sensor.v2x_delay_type',
  // },
  // v2x_delay_fixed_time: {
  //   range: '[0,1000000000]',
  //   step: 0.01,
  //   value: 45,
  //   unit: 'ms',
  //   type: 'Number',
  //   name: 'sensor.v2x_delay_fixed_time',
  // },
  // v2x_delay_uniform_min: {
  //   range: '[0,99999999.99]',
  //   step: 0.01,
  //   value: 40,
  //   unit: 'ms',
  //   type: 'Number',
  //   name: 'sensor.v2x_delay_uniform_min',
  // },
  // v2x_delay_uniform_max: {
  //   range: '[0.01,1000000000]',
  //   step: 0.01,
  //   value: 45,
  //   unit: 'ms',
  //   type: 'Number',
  //   name: 'sensor.v2x_delay_uniform_max',
  // },
  // v2x_delay_normal_mean: {
  //   range: '[0,1000000000]',
  //   step: 0.01,
  //   value: 45,
  //   unit: 'ms',
  //   type: 'Number',
  //   name: 'sensor.v2x_delay_normal_mean',
  // },
  // v2x_delay_normal_deviation: {
  //   range: '[0,1000000000]',
  //   step: 0.01,
  //   value: 5,
  //   unit: 'ms',
  //   type: 'Number',
  //   name: 'sensor.v2x_delay_normal_deviation',
  // },
}

export const OBUBandOptions = [
  {
    value: '5905-5915',
    label: '5905-5915',
  },
  {
    value: '5915-5925',
    label: '5915-5925',
  },
  {
    value: '5905-5925',
    label: '5905-5925',
  },
]

export const v2xLossTypeOptions = [
  {
    value: 0,
    label: 'sensor.noLoss',
  },
  {
    value: 1,
    label: 'sensor.randomLoss',
  },
  {
    value: 2,
    label: 'sensor.burstLoss',
  },
]

export const v2xDelayTypeOptions = [
  {
    value: 0,
    label: 'sensor.noDelay',
  },
  {
    value: 1,
    label: 'sensor.fixedDelay',
  },
  {
    value: 2,
    label: 'sensor.uniformDelay',
  },
  {
    value: 3,
    label: 'sensor.normalDelay',
  },
]

export const nameMap = {
  Camera: 'sensor.camera',
  Fisheye: 'sensor.fisheye',
  Semantic: 'sensor.semantic',
  Depth: 'sensor.depth',
  TraditionalLidar: 'sensor.lidar',
  Radar: 'sensor.radar',
  Truth: 'sensor.truth',
  IMU: 'sensor.imu',
  GPS: 'sensor.gps',
  Ultrasonic: 'sensor.ultrasonic',
  OBU: 'sensor.obu',
}

export const ReflectionTypeOption = [
  {
    value: 0,
    label: 'sensor.Reflectivity',
  },
  {
    value: 1,
    label: 'sensor.Tag',
  },
  {
    value: 2,
    label: 'sensor.AttenuationVal',
  },
]

/**
 * 语义相机和深度相机高级参数
 * @type {{
 * FOV_Horizontal: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * Focal_Length: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * FOV_Vertical: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * Frequency: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * Intrinsic_Matrix: {name: string, range: string, step: number, type: string, value: number[]},
 * Res_Horizontal: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * CCD_Width: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * },
 * CCD_Height: {
 * unit: string, name: string, range: string, step: number, type: string, value: number, InsideParamGroup: number
 * }, Res_Vertical: {unit: string, name: string, range: string, step: number, type: string, value: number},
 * InsideParamGroup: {name: string, type: string, value: number},
 * Distortion_Parameters: {name: string, range: string, step: number, type: string, value: number[]}
 * }}
 */
export const semanticAndDepthAdvancedInfo = {
  Frequency: {
    range: '[0,1000]',
    step: 0.01,
    value: 25,
    unit: 'Hz',
    type: 'Number',
    name: 'sensor.Frequency',
  },
  Res_Horizontal: {
    range: '[1,10000]',
    step: 1,
    value: 1920,
    unit: 'px',
    type: 'Number',
    name: 'sensor.Res_Horizontal',
  },
  Res_Vertical: {
    range: '[1,10000]',
    step: 1,
    value: 1208,
    unit: 'px',
    type: 'Number',
    name: 'sensor.Res_Vertical',
  },
  Distortion_Parameters: {
    range: '[-65535,65535]',
    step: 0.000000001,
    value: [
      -0.5333680427825355,
      0.4577346209269804,
      -0.4836076527200365,
      -0.0023929586859060227,
      0.0018364297357014352,
    ],
    type: 'Array',
    name: 'sensor.Distortion_Parameters',
  },
  InsideParamGroup: {
    value: 0, // 0 - 矩阵，1 - fov，2 - 感光器
    type: 'Number',
    name: 'sensor.InsideParam',
  }, // 以下参数中的 InsideParamGroup === InsideParamGroup 时，可以进行编辑，否则disabled
  Intrinsic_Matrix: {
    range: '[-5000000,5000000]',
    step: 0.000000001,
    value: [1945.1674168728503, 0, 946.1188960408923, 0, 1938.137228006907, 619.7048547473978, 0, 0, 1],
    type: 'Array',
    name: 'sensor.Intrinsic_Matrix',
  },

  FOV_Horizontal: {
    range: '[0,170]',
    step: 0.01,
    value: 60,
    unit: '°',
    InsideParamGroup: 1,
    type: 'Number',
    name: 'sensor.FOV_Horizontal',
  },
  FOV_Vertical: {
    range: '[0,170]',
    step: 0.01,
    value: 36.28,
    unit: '°',
    InsideParamGroup: 1,
    type: 'Number',
    name: 'sensor.FOV_Vertical',
  },
  CCD_Width: {
    range: '(0,10000]',
    step: 0.01,
    value: 10,
    unit: 'mm',
    InsideParamGroup: 2,
    type: 'Number',
    name: 'sensor.CCD_Width',
  },
  CCD_Height: {
    range: '(0,10000]',
    step: 0.01,
    value: 10,
    unit: 'mm',
    InsideParamGroup: 2,
    type: 'Number',
    name: 'sensor.CCD_Height',
  },
  Focal_Length: {
    range: '(0,10000]',
    step: 0.01,
    value: 10,
    unit: 'mm',
    InsideParamGroup: 2,
    type: 'Number',
    name: 'sensor.Focal_Length',
  },
}

export const sensorTypeMap = {
  camera: 'Camera',
  lidar: 'TraditionalLidar',
  radar: 'Radar',
  truth: 'Truth',
  imu: 'IMU',
  gps: 'GPS',
  ultrasonic: 'Ultrasonic',
  fisheye: 'Fisheye',
  semantic: 'Semantic',
  depth: 'Depth',
  obu: 'OBU',
}
