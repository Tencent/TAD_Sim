import { SolidStateLidarMap } from '@/store/modules/sensor/constants'

const { isNaN } = Number

/**
 * @description CCD参数换算FOV
 * @param {number} ccdWidth CCD宽度
 * @param {number} ccdHeight CCD 高度
 * @param {number} focalLength 焦距
 * @return {[number, number]} [横向fov，纵向fov]
 */
function ccdToFov (ccdWidth, ccdHeight, focalLength) {
  if (+focalLength === 0 || isNaN(+focalLength)) {
    console.warn('The focal length cannot be 0')
    return [0, 0]
  }
  const hFov = 2 * Math.atan(0.5 * ccdWidth / focalLength) * 180 / Math.PI
  const vFov = 2 * Math.atan(0.5 * ccdHeight / focalLength) * 180 / Math.PI
  return [hFov, vFov]
}

/**
 * @description CCD参数换算FOV
 * @param {number} resHorizontal Horizontal resolution
 * @param {number} resVertical 垂直分辨率
 * @param {number} fx 内参fx
 * @param {number} fy 内参fy
 * @return {[number, number]} [横向fov，纵向fov]
 */
function intrinsicMatrixToFov (resHorizontal, resVertical, fx, fy) {
  let hFov
  let vFov
  if (+fx === 0 || isNaN(+fx)) {
    console.warn('The fx,fy of intrinsic parameter cannot be 0')
    hFov = 0
  } else {
    hFov = 2 * Math.atan(0.5 * resHorizontal / fx) * 180 / Math.PI
  }
  if (+fy === 0 || isNaN(+fy)) {
    console.warn('The fx,fy of intrinsic parameter cannot be 0')
    vFov = 0
  } else {
    vFov = 2 * Math.atan(0.5 * resVertical / fy) * 180 / Math.PI
  }
  return [hFov, vFov]
}

/**
 * 获取传感器的上视场角
 * @param {object} sensor 传感器对象
 * @return {number} 上视场角值
 */
function getUpperFov (sensor) {
  const {
    type,
    param,
  } = sensor

  switch (type) {
    case 'Camera':
    case 'Fisheye':
    case 'Semantic':
    case 'Depth':{
      const {
        FOV_Vertical,
        IntrinsicParamType,
        Intrinsic_Matrix: [fx, , , , fy],
        CCD_Width,
        CCD_Height,
        Focal_Length,
        Res_Horizontal,
        Res_Vertical,
      } = param
      let fov = 0
      switch (+IntrinsicParamType) {
        case 0:
          fov = intrinsicMatrixToFov(Res_Horizontal, Res_Vertical, fx, fy)[1] / 2
          break
        case 1:
          fov = FOV_Vertical / 2 || 0
          break
        case 2:
          fov = ccdToFov(CCD_Width, CCD_Height, Focal_Length)[1] / 2
          break
        default:
      }
      return fov
    }
    case 'TraditionalLidar':
      if (param.Model in SolidStateLidarMap) {
        return SolidStateLidarMap[param.Model].upperFov
      }
      switch (param.Model) {
        case 'HS40':
          return 7
        case 'RS32':
          return 25
        case 'HS32':
        case 'RS16':
          return 15
        case 'Velodyne32':
          return 30
        case 'RSBpearl':
          return 90
        case 'RSHelios':
          return 55
        case 'HS64':
        case 'RSRuby':
        case 'HS128':
          return 25
        case 'RSM1':
        case 'RSM1P':
          return 12.5
        case 'HS128AT':
          return 12
        case 'HS64QT':
          return 52.1
        default:
          return param.uUpperFov || 0
      }
    case 'IMU':
    case 'GPS':
    case 'OBU':
      return 0
    // radar & truth
    case 'Ultrasonic':
      return param.FOV_Vertical / 2 || 0
    default:
      return param.vfov / 2 || 0
  }
}

/**
 * 获取传感器的下视场角
 * @param {object} sensor 传感器对象
 * @return {number} 下视场角值
 */
function getLowerFov (sensor) {
  const {
    type,
    param,
  } = sensor

  switch (type) {
    case 'Camera':
    case 'Fisheye':
    case 'Semantic':
    case 'Depth': {
      const {
        FOV_Vertical,
        IntrinsicParamType,
        Intrinsic_Matrix: [fx, , , , fy],
        CCD_Width,
        CCD_Height,
        Focal_Length,
        Res_Horizontal,
        Res_Vertical,
      } = param
      let fov = 0
      switch (+IntrinsicParamType) {
        case 0:
          fov = -intrinsicMatrixToFov(Res_Horizontal, Res_Vertical, fx, fy)[1] / 2
          break
        case 1:
          fov = -FOV_Vertical / 2 || 0
          break
        case 2:
          fov = -ccdToFov(CCD_Width, CCD_Height, Focal_Length)[1] / 2
          break
        default:
      }
      return fov
    }
    case 'TraditionalLidar':
      if (param.Model in SolidStateLidarMap) {
        return SolidStateLidarMap[param.Model].lowerFov
      }
      switch (param.Model) {
        case 'HS40':
          return -16
        case 'RS32':
          return -25
        case 'HS32':
        case 'RS16':
          return -15
        case 'Velodyne32':
          return -10
        case 'RSBpearl':
          return 0
        case 'RSHelios':
        case 'HS64':
        case 'RSRuby':
        case 'HS128':
          return -15
        case 'RSM1':
        case 'RSM1P':
          return -12.5
        case 'HS128AT':
          return -12
        case 'HS64QT':
          return -52.1
        default:
          return -param.uLowerFov || 0
      }
    case 'IMU':
    case 'GPS':
    case 'OBU':
      return 0
    // radar & truth
    case 'Ultrasonic':
      return -param.FOV_Vertical / 2 || 0
    default:
      return -param.vfov / 2 || 0
  }
}

/**
 * 获取传感器的左视场角
 * @param {object} sensor 传感器对象
 * @return {number} 左视场角值
 */
function getLeftFov (sensor) {
  const {
    type,
    param,
  } = sensor

  switch (type) {
    case 'Camera':
    case 'Fisheye':
    case 'Semantic':
    case 'Depth': {
      const {
        FOV_Horizontal,
        IntrinsicParamType,
        Intrinsic_Matrix: [fx, , , , fy],
        CCD_Width,
        CCD_Height,
        Focal_Length,
        Res_Horizontal,
        Res_Vertical,
      } = param
      let fov = 0
      switch (+IntrinsicParamType) {
        case 0:
          fov = intrinsicMatrixToFov(Res_Horizontal, Res_Vertical, fx, fy)[0] / 2
          break
        case 1:
          fov = FOV_Horizontal / 2 || 0
          break
        case 2:
          fov = ccdToFov(CCD_Width, CCD_Height, Focal_Length)[0] / 2
          break
        default:
      }
      return fov
    }
    case 'TraditionalLidar':
      if (param.Model in SolidStateLidarMap) {
        return SolidStateLidarMap[param.Model].leftFov
      }
      return 180
    case 'IMU':
    case 'GPS':
    case 'OBU':
      return 0
    // radar & truth
    case 'Ultrasonic':
      return param.FOV_Horizontal / 2 || 0
    default:
      return param.hfov / 2 || 0
  }
}

/**
 * 获取传感器的右视场角
 * @param {object} sensor 传感器对象
 * @return {number} 右视场角值
 */
function getRightFov (sensor) {
  const {
    type,
    param,
  } = sensor

  switch (type) {
    case 'Camera':
    case 'Fisheye':
    case 'Semantic':
    case 'Depth': {
      const {
        FOV_Horizontal,
        IntrinsicParamType,
        Intrinsic_Matrix: [fx, , , , fy],
        CCD_Width,
        CCD_Height,
        Focal_Length,
        Res_Horizontal,
        Res_Vertical,
      } = sensor.param
      let fov = 0
      switch (+IntrinsicParamType) {
        case 0:
          fov = -intrinsicMatrixToFov(Res_Horizontal, Res_Vertical, fx, fy)[0] / 2
          break
        case 1:
          fov = -FOV_Horizontal / 2 || 0
          break
        case 2:
          fov = -ccdToFov(CCD_Width, CCD_Height, Focal_Length)[0] / 2
          break
        default:
      }
      return fov
    }
    case 'TraditionalLidar':
      if (param.Model in SolidStateLidarMap) {
        return SolidStateLidarMap[param.Model].rightFov
      }
      return -180
    case 'IMU':
    case 'GPS':
    case 'OBU':
      return 0
    // radar & truth
    case 'Ultrasonic':
      return -param.FOV_Horizontal / 2 || 0
    default:
      return -param.hfov / 2 || 0
  }
}

/**
 * 格式化传感器参数
 * @param {object} sensor 传感器对象
 * @return {object} 格式化后的传感器参数
 */
function formatSensorParams (sensor) {
  const {
    LocationX,
    LocationY,
    LocationZ,
    RotationX,
    RotationY,
    RotationZ,
  } = sensor
  const upperFov = getUpperFov(sensor)
  const lowerFov = getLowerFov(sensor)
  const leftFov = getLeftFov(sensor)
  const rightFov = getRightFov(sensor)

  return {
    x: LocationX,
    y: LocationY,
    z: LocationZ,
    rotateX: RotationX,
    rotateY: RotationY,
    rotateZ: RotationZ,
    upperFov,
    lowerFov,
    leftFov,
    rightFov,
  }
}

/**
 * 获取传感器的最大距离
 * @param {object} sensor 传感器对象
 * @return {number} 最大距离值
 */
function getMaxDistance (sensor) {
  const {
    type,
    param,
  } = sensor
  switch (type) {
    case 'Radar':
      return +param.R_m
    case 'Truth':
      return +param.drange
    case 'TraditionalLidar':
      if (param.Model in SolidStateLidarMap) {
        return SolidStateLidarMap[param.Model].far
      }
      return +param.uRange
    case 'Ultrasonic':
      return +param.Distance
    default:
      return 16
  }
}

export { formatSensorParams, getMaxDistance }
