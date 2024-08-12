import { MathUtils, Matrix4, Quaternion, Vector2, Vector3 } from 'three'
import { getPoleConfig } from '../config'

export const PI = Math.PI
export const halfPI = PI / 2
export const worldCenter = new Vector3(0, 0, 0)
export const axisX = new Vector3(1, 0, 0)
export const axisY = new Vector3(0, 1, 0)
export const axisZ = new Vector3(0, 0, 1)

export const cacheCustomModelList = []
// 清空本地缓存的自定义模型列表
export function clearCustomModelList () {
  cacheCustomModelList.length = 0
}
// 注册并解析自定义模型列表
export function registryCustomModelList (list) {
  clearCustomModelList()
  for (const option of list) {
    const { name, variable, model3d, boundingBox } = option
    const { center, dimensions } = boundingBox
    const { x: posX, y: posY, z: posZ } = center
    const { length, width, height } = dimensions
    // 将路径分隔符统一成 webgl loader 可以加载的静态资源路径
    const validModelUrl = model3d.replaceAll('\\', '/')
    // 将配置项铺平，减少嵌套
    const _option = {
      name,
      variable,
      modelUrl: validModelUrl,
      posX,
      posY,
      posZ,
      length,
      width,
      height,
    }

    cacheCustomModelList.push(_option)
  }
}
// 通过模型名称查询对应的配置参数
export function queryCustomModelConfigByName (name) {
  for (const option of cacheCustomModelList) {
    if (name === option.variable) {
      return option
    }
  }
  return null
}

// 从外层调用入口中，注册获取道路中某一个位置的法向量的方法
// eslint-disable-next-line import/no-mutable-exports
export let getPositionNormal
export function registryGetPositionNormal (fn) {
  getPositionNormal = fn
}

export const defaultColor = {
  white: {
    name: 'white',
    value: 0xF1F1F1,
    string: '#f1f1f1',
  },
  yellow: {
    name: 'yellow',
    value: 0xF1EA15,
    string: '#f1ea15',
  },
  red: {
    name: 'red',
    value: 0xFC2D2D,
    string: '#fc2d2d',
  },
  green: {
    name: 'green',
    value: 0x00D82C,
    string: '#00d82c',
  },
  blue: {
    name: 'blue',
    value: 0x0DC2EC,
    string: '#0dc2ec',
  },
}

export function promisify (fn) {
  return new Promise((resolve, reject) => {
    fn.then((data) => {
      resolve(data)
    })
  })
}

// 相对的渲染层级
export const RenderOrder = {
  signalBoard: 5, // 信号标志牌
  trafficLight: 5, // 交通信号灯
  sensor: 5, // 传感器
  pole: 4, // 杆
  other: 4, // 其他类型物体
  parkingSpace: 3, // 停车位轮廓线
  parkingSpacePlaceholder: 2, // 停车位中间虚拟的占位区域
  roadSign: 1, // 路面标识
}

// 将形如 [{x: 1, y: 2, z: 3}] 的数值转换成 Array<Vector3> 类型的数组
export function transformVec3ByObject (points) {
  return points.map(p => new Vector3(p.x, p.y, p.z))
}

// 获取在 [-PI, PI] 范围内的有效弧度值
export function getValidRadInRange (rad) {
  let _rad = rad % (PI * 2)
  if (_rad > PI) {
    _rad -= PI * 2
  } else if (_rad < -PI) {
    _rad += PI * 2
  }
  return _rad
}

// 基于杆的位置和角度，调整杆上物体的空间属性
export function adjustLocationByPole (params) {
  const { data, poleData, isOldMap = false, yaw } = params
  // 投影到水平面的坐标
  const projectPos = new Vector2(data.x, data.y)
  const projectPolePos = new Vector2(poleData.x, poleData.y)
  const _poleYaw = isOldMap ?
    Number(poleData.yaw) + halfPI :
    Number(poleData.yaw)

  // 判断在横杆部分还是竖杆部分的误差
  const distDeviation = 0.5 // 距离误差1m
  const radDeviation = MathUtils.degToRad(10) // 角度误差10度

  // 杆的角度，跟杆上物体的角度，差
  const fixedAngle = halfPI

  // 目标物体是否在竖杆上
  const onVerticalPole = poleData.name === 'Vertical_Pole'

  const config = getPoleConfig(poleData.name)
  if (!config) return null
  const { vRadius, height } = config.style

  if (onVerticalPole) {
    // 如果在竖杆上，基于偏航角计算物体在杆上的位置
    const unit = new Vector2(vRadius * Math.cos(yaw), vRadius * Math.sin(yaw))
    const newPos = new Vector3(poleData.x + unit.x, poleData.y + unit.y, data.z)
    return {
      pos: newPos,
    }
  } else {
    const calcDist = projectPos.distanceTo(projectPolePos)
    const deltaDist = Math.abs(calcDist - vRadius)

    if (deltaDist <= distDeviation) {
      // 在横杆的竖直部分中
      const unit = new Vector2(vRadius * Math.cos(yaw), vRadius * Math.sin(yaw))
      const newPos = new Vector3(
        poleData.x + unit.x,
        poleData.y + unit.y,
        data.z,
      )
      return {
        pos: newPos,
      }
    } else {
      // 在横杆的水平延伸部分
      const deltaRad1 = Math.abs(
        getValidRadInRange(_poleYaw + fixedAngle - yaw),
      )
      const deltaRad2 = Math.abs(
        getValidRadInRange(_poleYaw - fixedAngle - yaw),
      )
      const deltaHeight = Math.abs(height + poleData.z - data.z)

      if (
        (deltaRad1 <= radDeviation || deltaRad2 <= radDeviation) &&
        deltaHeight <= distDeviation
      ) {
        // 如果角度和高度处于横杆水平延伸部分的误差范围中
        const onLeft = deltaRad1 <= radDeviation
        const matchYaw = onLeft ? _poleYaw + fixedAngle : _poleYaw - fixedAngle

        // 从杆指向物体方向的向量
        const direction = projectPos.clone().sub(projectPolePos)
        const angle = direction.angle()
        const theta = Math.abs(_poleYaw - angle)
        const length = direction.length() * Math.cos(theta)
        const poleDirection = new Vector2(
          Math.cos(_poleYaw),
          Math.sin(_poleYaw),
        )
        const pointOnHorizontal = projectPolePos.addScaledVector(
          poleDirection,
          length,
        )
        // 跟横杆垂直的方向
        const verticalDirection = new Vector2(
          Math.cos(matchYaw),
          Math.sin(matchYaw),
        )
        // 横杆的半径
        const { hRadius, height } = config.style
        const newPosVec2 = pointOnHorizontal
          .clone()
          .addScaledVector(verticalDirection, hRadius)

        // 在横杆上校正后的高度
        const fixedHeight = poleData.z + height - hRadius
        const newPos = new Vector3(newPosVec2.x, newPosVec2.y, fixedHeight)

        return {
          pos: newPos,
          matchYaw,
        }
      }
    }
  }

  return null
}

// 调整数字的精度
export function fixedPrecision (num, precision = 3) {
  return Number(num.toFixed(precision))
}

// 调整物体跟道路坡道之间的贴合角度
export function setRotateByDirNormal (mesh, dirOnXY, normal) {
  if (normal.z < 0) {
    normal.negate()
  }
  const theta = Math.atan2(dirOnXY.y, dirOnXY.x)
  const matrix = new Matrix4()
  matrix.makeRotationAxis(axisZ, theta)
  const quaternionZ = new Quaternion()
  quaternionSetFromUnitVectors(quaternionZ, axisZ, normal)
  const matrixZ = new Matrix4()
  matrixZ.makeRotationFromQuaternion(quaternionZ)
  matrixZ.multiply(matrix)
  mesh.rotation.setFromRotationMatrix(matrixZ)
}

function quaternionSetFromUnitVectors (quaternion, vFrom, vTo) {
  // assumes direction vectors vFrom and vTo are normalized
  let r = vFrom.dot(vTo) + 1

  if (r < Number.EPSILON) {
    // vFrom and vTo point in opposite directions
    r = 0

    if (Math.abs(vFrom.x) > Math.abs(vFrom.z)) {
      quaternion._x = -vFrom.y
      quaternion._y = vFrom.x
      quaternion._z = 0
      quaternion._w = r
    } else {
      quaternion._x = 0
      quaternion._y = -vFrom.z
      quaternion._z = vFrom.y
      quaternion._w = r
    }
  } else {
    // crossVectors( vFrom, vTo ); // inlined to avoid cyclic dependency on Vector3
    quaternion._x = vFrom.y * vTo.z - vFrom.z * vTo.y
    quaternion._y = vFrom.z * vTo.x - vFrom.x * vTo.z
    quaternion._z = vFrom.x * vTo.y - vFrom.y * vTo.x
    quaternion._w = r
  }

  return quaternion.normalize()
}
