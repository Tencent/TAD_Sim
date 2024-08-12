import {
  Box3,
  CubicBezierCurve,
  CubicBezierCurve3,
  MathUtils,
  Vector2,
  Vector3,
} from 'three'
import type { Group, Intersection, Line, Material, Mesh, Object3D, Points } from 'three'
import { genUuid } from './guid'
import { Constant } from './business'

export const worldCenter = new Vector3(0, 0, 0)
export const axisX = new Vector3(1, 0, 0)
export const axisY = new Vector3(0, 1, 0)
export const axisZ = new Vector3(0, 0, 1)
export const PI = Math.PI
export const halfPI = PI / 2

// 偏移量尽可能保持在同一高度，切换视角时画面展示的物体比例更接近
// 透视相机初始偏移量
export const pCameraOffset = {
  x: -10,
  y: 25,
  z: 0,
}
// 正交相机初始偏移量
export const oCameraOffset = {
  x: -1,
  y: 200,
  z: 0,
}

// 默认提供的统一颜色
export const DefaultColor = {
  White: {
    name: 'White',
    value: 0xF1F1F1,
    string: '#f1f1f1',
  },
  Yellow: {
    name: 'Yellow',
    value: 0xF1EA15,
    string: '#f1ea15',
  },
  Red: {
    name: 'Red',
    value: 0xFC2D2D,
    string: '#fc2d2d',
  },
  Green: {
    name: 'Green',
    value: 0x00D82C,
    string: '#00d82c',
  },
  Blue: {
    name: 'Blue',
    value: 0x0DC2EC,
    string: '#0dc2ec',
  },
}

type ICurveType = 'centripetal' | 'chordal' | 'catmullrom'
export const CurveType: ICurveType = 'centripetal'
export const CatmullromTension = '0.32'

// 从三维场景中计算物体的重心
export function getGravityCenter (params: { objectId: string, parent: Group }) {
  const { objectId, parent } = params
  const targetObject = parent.getObjectByProperty('objectId', objectId)
  if (!targetObject) return null

  // 通过包围盒计算对应物体的位置
  const box = new Box3()
  box.expandByObject(targetObject)
  const gravityCenter = new Vector3()
  box.getCenter(gravityCenter)

  return gravityCenter
}

// 从三维场景中获取杆上物体的世界坐标
export function getWorldLocation (params: { objectId: string, parent: Group }) {
  const { objectId, parent } = params
  const targetObject = parent.getObjectByProperty('objectId', objectId)
  if (!targetObject) return null

  targetObject.updateMatrixWorld()
  const worldPosition = new Vector3()
  targetObject.getWorldPosition(worldPosition)

  return worldPosition
}

// 针对 catmullrom 类型的道路线性，补充计算首和尾的虚拟控制点
export function addCatmullromVirtualPoint (
  controlPoints: Array<biz.ICommonControlPoint>,
) {
  // 对于 catmullrom 类型的曲线控制点，需要在首和尾添加2个辅助的虚拟控制点
  // 虚拟控制点到端点的距离，跟端点到相邻控制点是【等距】的
  // 首部的虚拟控制点
  const headHdg = controlPoints[0].hdg
  const head0 = new Vector3(
    controlPoints[0].x,
    controlPoints[0].y,
    controlPoints[0].z,
  )
  const head1 = new Vector3(
    controlPoints[1].x,
    controlPoints[1].y,
    controlPoints[1].z,
  )

  const headDirection = head0.clone().sub(head1).normalize()
  const headDist = head0.distanceTo(head1)
  const vHeadPoint = head0.clone().addScaledVector(headDirection, headDist)

  // 尾部的虚拟控制点
  const length = controlPoints.length
  const tailHdg = controlPoints[length - 1].hdg
  const tail0 = new Vector3(
    controlPoints[length - 1].x,
    controlPoints[length - 1].y,
    controlPoints[length - 1].z,
  )
  const tail1 = new Vector3(
    controlPoints[length - 2].x,
    controlPoints[length - 2].y,
    controlPoints[length - 2].z,
  )
  const tailDirection = tail0.clone().sub(tail1).normalize()
  const tailDist = tail0.distanceTo(tail1)
  const vTailPoint = tail0.clone().addScaledVector(tailDirection, tailDist)

  // 在首尾添加虚拟控制点
  controlPoints.unshift({
    x: vHeadPoint.x,
    y: vHeadPoint.y,
    z: vHeadPoint.z,
    hdg: headHdg,
  })
  controlPoints.push({
    x: vTailPoint.x,
    y: vTailPoint.y,
    z: vTailPoint.z,
    hdg: tailHdg,
  })
}

// 创建圆弧效果的点
export function createArcPoints (params: {
  radius: number
  center: Vector3
  startAngle: number // 起始的角度
  endAngle: number // 截止的角度
  unit: number // 采样点的角度间隔
  isClockwise?: boolean // 是否是顺时针创建
}) {
  const {
    radius,
    center,
    startAngle,
    endAngle,
    unit,
    isClockwise = false,
  } = params
  const points = []
  // 是否将最后的边界点补上
  let addLast = false

  if (isClockwise) {
    // 顺时针是角度递减
    for (let i = startAngle; i >= endAngle; i -= unit) {
      if (i === endAngle) {
        addLast = true
      }
      const x = center.x + radius * Math.sin(i)
      const y = 0
      const z = center.z + radius * Math.cos(i)
      const p = new Vector3(x, y, z)
      points.push(p)
    }
  } else {
    // 逆时针是角度递增
    for (let i = startAngle; i <= endAngle; i += unit) {
      if (i === endAngle) {
        addLast = true
      }
      const x = center.x + radius * Math.sin(i)
      const y = 0
      const z = center.z + radius * Math.cos(i)
      const p = new Vector3(x, y, z)
      points.push(p)
    }
  }
  // 补齐最后一个边界点
  if (!addLast) {
    const x = center.x + radius * Math.sin(endAngle)
    const y = 0
    const z = center.z + radius * Math.cos(endAngle)
    points.push(new Vector3(x, y, z))
  }

  return points
}

// 获取一条曲线路径上，百分比位置坐标点的属性
export function getPointInfoOnCurve (curve: biz.ICurve3, percent: number) {
  const p = curve.getPointAt(percent)
  // 坐标点切线方向跟水平向右坐标轴的夹角
  const hdg = String(getWorldAngle(curve.getTangentAt(percent)))
  // 将坐标点转换成透传的形式
  const _p = transformOneCoordinateToCommon(p)
  return {
    ..._p,
    hdg,
  }
}

// 获取一个方向向量，跟水平向右坐标轴的夹角，弧度制，逆时针为正
export function getWorldAngle (direction: common.vec3) {
  const directionVec2 = new Vector2(direction.z, direction.x)
  const angle = directionVec2.angle()
  return getValidRadInRange(angle)
}

// 通过全局角度，计算物体看向的点
export function getLookAtPointByAngle (params: {
  point: common.vec3
  angle: number
}) {
  const { point, angle } = params

  const direction = new Vector3(0, 0, 1)
  const rad = MathUtils.degToRad(angle)
  direction.applyAxisAngle(axisY, rad).normalize()

  const lookAtPoint = new Vector3(point.x, point.y, point.z).addScaledVector(
    direction,
    1,
  )
  return lookAtPoint
}

// 通过偏航角和切线方向，计算物体看向的点
export function getLookAtPointByYaw (params: {
  pointOnRefLine?: common.vec3 // 在道路参考线上的投影点
  point: common.vec3
  tangent: common.vec3
  yaw: number // 偏航角是角度制
  useElevation?: boolean // 是否适用高程的切线分量
}) {
  const { tangent, point, yaw, useElevation = false, pointOnRefLine } = params
  const direction = new Vector3(tangent.x, tangent.y, tangent.z)
  // 如果是忽略道路高程（坡度），则以竖直朝上的方向为法向量来旋转
  let normal: Vector3 = axisY
  if (!useElevation) {
    // 如果不适用高程切线分量，则将高度分量设为0
    direction.y = 0
  } else if (pointOnRefLine) {
    // 如果道路有坡度，使用垂直于坡道的法向量
    // 先计算当前位置在道路参考线处道路平面内的向量
    const vertical = getVerticalVector(tangent).normalize()
    // 将原有的道路切线方向，绕上述向量旋转 90°
    const _tangent = direction.clone()
    _tangent.applyAxisAngle(vertical, halfPI).normalize()
    normal = _tangent
  }
  // 将角度制转换成弧度制
  const rad = MathUtils.degToRad(yaw)
  // 将切线方向绕 y 轴旋转指定的偏航角
  direction.applyAxisAngle(normal, rad).normalize()

  const lookAtPoint = new Vector3(point.x, point.y, point.z).addScaledVector(
    direction,
    1,
  )

  return lookAtPoint
}

// 将角度制的值，控制在 [-180, 180] 的有效区间内
export function getValidDegreeInRange (deg: number) {
  let _deg = deg % 360
  if (_deg > 180) {
    // 如果在 (180, 360] 区间
    _deg -= 360
  } else if (_deg < -180) {
    // 如果在 [-360,180) 区间
    _deg += 360
  }
  return _deg
}

// 将弧度制的值，控制在[-PI, PI]的有效区间内
export function getValidRadInRange (rad: number) {
  let _rad = rad % (PI * 2)
  if (_rad > PI) {
    _rad -= PI * 2
  } else if (_rad < -PI) {
    _rad += PI * 2
  }
  return _rad
}

// 计算路口 laneLink 参考线坐标系下的偏航角
export function getYawInJunction (params: {
  tangent: common.vec3
  // 世界坐标系下的角度
  angle: number
}) {
  const { angle, tangent } = params
  const tangentVec2 = new Vector2(tangent.z, tangent.x)
  // 计算 st 坐标切线方向向量基于水平横轴的角度
  const tangentDeg = MathUtils.radToDeg(tangentVec2.angle())
  // 用世界坐标系下的角度，减去切线的角度计算出偏航角
  const yaw = angle - tangentDeg

  // 将偏航角限制于 [-180, 180] 度的范围中
  const _yaw = getValidDegreeInRange(yaw)
  return _yaw
}

// 通过参考线坐标系的 st 坐标，计算实际笛卡尔坐标系下的空间位置
export function getLocationByST (params: {
  curvePath: biz.ICurve3
  elevationPath?: biz.ICurve3
  useElevationTangent?: boolean
  s: number
  t: number
  crossBorder?: boolean
}) {
  const {
    curvePath,
    elevationPath,
    useElevationTangent = false,
    s,
    t,
    crossBorder = false,
  } = params
  const length = curvePath.getLength()
  let normal: Vector3

  // 如果允许超越参考线的边界范围，则对于 s < 0 或 s > length 的情况，使用端点延长线辅助定位
  if (crossBorder && (s < 0 || s > length)) {
    const isTail = s > length

    // 切线方向
    const tangent = curvePath.getTangentAt(isTail ? 1 : 0).normalize()
    // 参考线的端点
    const endPoint = curvePath.getPointAt(isTail ? 1 : 0)
    // 如果 s < 0,偏移量为负数，即反方向
    const sOffset = isTail ? s - length : s
    // 投影点在切向方向延长线上
    const refLinePoint = endPoint.clone().addScaledVector(tangent, sOffset)
    // 如果 s < 0, 百分比也为负数，实际上无意义
    const percent = s / length
    // 垂直分量指向参考线右侧
    const vertical = getVerticalVector(tangent).normalize()
    // 参考线右侧的坐标，t值为负数
    const targetPoint = refLinePoint.clone().addScaledVector(vertical, -t)

    if (elevationPath) {
      const _percent = Math.max(0, Math.min(1, s / length))
      const elevationPoint = elevationPath.getPointAt(_percent)
      // 如果高程曲线存在，则使用高程曲线在s处的高度更新世界坐标系下的坐标
      targetPoint.y = elevationPoint.y
      // 道路参考线上最近投影点的高度也需要更新
      refLinePoint.y = elevationPoint.y
      if (useElevationTangent) {
        const elevationTangent = elevationPath.getTangentAt(_percent)
        // 先将原切向向量归一化
        tangent.normalize()
        // 计算高度分量跟s轴分量的占比（x不可能为0）
        const ratio = elevationTangent.y / elevationTangent.x
        tangent.y = ratio
        // 调整高度的分量后，再次归一化
        tangent.normalize()
      }
    }

    // 将切线方向，绕投影点旋转 90°得到此处的法向量（不管道路是否有高程坡度，都有该法向量）
    normal = tangent.clone()
    // 切线方向绕投影点的垂直方向旋转 90°
    normal.applyAxisAngle(vertical, halfPI).normalize()

    return {
      s,
      t,
      refLinePoint,
      targetPoint,
      tangent,
      percent,
      normal,
    }
  }

  // 否则都走下面约束在 [0, length] 范围内的逻辑
  // 将 s 值控制在有效范围内
  const _s = Math.max(0, Math.min(s, length))
  const percent = _s / length
  // 获取参考线在新比例下的采样点和切线
  const refLinePoint: Vector3 = curvePath.getPointAt(percent)
  const tangent = curvePath.getTangentAt(percent)

  const _targetPoint = refLinePoint.clone()
  if (elevationPath) {
    const percent = Math.max(0, Math.min(1, s / length))
    const elevationPoint = elevationPath.getPointAt(percent)
    // 如果高程曲线存在，则使用高程曲线在s处的高度更新世界坐标系下的坐标
    _targetPoint.y = elevationPoint.y
    // 道路参考线上最近投影点的高度也需要更新
    refLinePoint.y = elevationPoint.y
    if (useElevationTangent) {
      const elevationTangent = elevationPath.getTangentAt(percent)
      // 先将原切向向量归一化
      tangent.normalize()
      // 计算高度分量跟s轴分量的占比（x不可能为0）
      const ratio = elevationTangent.y / elevationTangent.x
      tangent.y = ratio
      // 调整高度的分量后，再次归一化
      tangent.normalize()
    }
  }

  // 基于切线方向计算同一高度的垂直方向分量（切线方向右侧）
  const vertical = getVerticalVector(tangent)

  // 将切线方向，绕投影点旋转 90°得到此处的法向量（不管道路是否有高程坡度，都有该法向量）
  normal = tangent.clone()
  // 切线方向绕投影点的垂直方向旋转 90°
  normal.applyAxisAngle(vertical, halfPI).normalize()

  // 特殊情况，t=0，说明在参考线上
  if (t === 0) {
    return {
      // 将校正过的 s 值返回
      s: _s,
      t,
      refLinePoint,
      targetPoint: _targetPoint,
      tangent,
      percent,
      normal,
    }
  }

  // t>0 说明目标点在参考线左侧，跟垂直分量方向相反
  // t<0 说明目标点在参考线右侧，跟垂直分量方向一致
  const offsetPoint: Vector3 = refLinePoint
    .clone()
    .addScaledVector(vertical, -t)

  if (elevationPath) {
    const percent = Math.max(0, Math.min(1, s / length))
    const elevationPoint = elevationPath.getPointAt(percent)
    // 如果高程曲线存在，则使用高程曲线在s处的高度更新世界坐标系下的坐标
    offsetPoint.y = elevationPoint.y
    // 道路参考线上最近投影点的高度也需要更新
    refLinePoint.y = elevationPoint.y

    if (useElevationTangent) {
      const elevationTangent = elevationPath.getTangentAt(percent)
      // 先将原切向向量归一化
      tangent.normalize()
      // 计算高度分量跟s轴分量的占比（x不可能为0）
      const ratio = elevationTangent.y / elevationTangent.x
      tangent.y = ratio
      // 调整高度的分量后，再次归一化
      tangent.normalize()
    }
  }
  return {
    s: _s,
    t,
    refLinePoint,
    targetPoint: offsetPoint,
    tangent,
    percent,
    normal,
  }
}

// 获取 st 坐标系下的 t 坐标，区分正负
export function getTValue (params: {
  tangent: common.vec3
  refLinePoint: common.vec3
  targetPoint: common.vec3
}) {
  const { tangent, refLinePoint, targetPoint } = params
  // 将三维场景对应的坐标，转换成平面坐标，即 zxy 取前两位作为 xy
  const targetVec2 = new Vector2(targetPoint.z, targetPoint.x)
  const refLinePointVec2 = new Vector2(refLinePoint.z, refLinePoint.x)
  const vec2 = targetVec2.clone().sub(refLinePointVec2)
  const tangentVec2 = new Vector2(tangent.z, tangent.x)
  const crossValue = vec2.cross(tangentVec2)
  const dist = targetVec2.distanceTo(refLinePointVec2)
  // 叉乘的结果，>0 说明目标点在右侧；<0 说明目标点在左侧
  // 沿参考线方向右侧，为负值；左侧，为正值
  return crossValue >= 0 ? -dist : dist
}

// 按照 renderOrder 从大到小的顺序排列
export function sortByRenderOrder (
  intersectionA: Intersection<Object3D>,
  intersectionB: Intersection<Object3D>,
) {
  const renderOrderA = intersectionA.object.renderOrder
  const renderOrderB = intersectionB.object.renderOrder

  if (renderOrderA > renderOrderB) {
    return -1
  } else if (renderOrderA < renderOrderB) {
    return 1
  } else {
    return 0
  }
}

export function transformArrayObjectToArray (obj: any) {
  const arr: Array<number> = []
  for (const key of Object.keys(obj)) {
    const index = Number(key)
    arr[index] = obj[key]
  }
  return arr
}

/**
 * 计算直角三角形的斜边长度
 * @param x
 * @param y
 */
export function calcHypotenuse (x: number, y: number) {
  return Math.sqrt(x * x + y * y)
}

/**
 * 在求曲线路径的近似点时，根据道路长度获取合理的采样间隔
 * @param length
 */
function getApproximatePointSegment (length: number) {
  if (length < 0) return -1
  // 向上取整
  const _length = Math.ceil(length)
  // 最小的采样间隔，单位 m
  let segmentPerMeter: number
  if (_length <= 100) {
    return 1000
  } else if (_length <= 250) {
    segmentPerMeter = 5
  } else if (_length <= 500) {
    segmentPerMeter = 4
  } else if (_length <= 1000) {
    segmentPerMeter = 2
  } else {
    // > 1000m
    segmentPerMeter = 1
  }
  return _length * segmentPerMeter
}

/**
 * 获取一个点，到某一条曲线上最近的交点
 * @param params
 */
export function getClosestPointFromCurvePath (params: {
  curvePath: biz.ICurve3
  point: common.vec3 | Vector3
  segment?: number
}) {
  const { curvePath, point, segment } = params
  // 路径长度
  const curveLength = curvePath.getLength()
  // 根据实际情况调整判定的间隔
  const _segment = segment || getApproximatePointSegment(curveLength)
  // 曲线路径的密集采样点
  const curveSamplePoints: Array<Vector3> = curvePath.getSpacedPoints(_segment)
  // 将目标点转换成三维矢量
  const _point = new Vector3(point.x, point.y, point.z)

  let sampleIndex = 0
  let lastDist
  for (let i = 0; i < curveSamplePoints.length; i++) {
    const samplePoint = curveSamplePoints[i]
    // 获取目标点到曲线采样点的距离
    const dist = _point.distanceTo(samplePoint)
    // 如果缓存的上一次距离不存在，或当前新的距离小于上一次的距离，需要更新缓存的数据
    if (lastDist === undefined || dist < lastDist) {
      lastDist = dist
      sampleIndex = i
      continue
    }
  }

  // 最近交点的坐标
  const closestPoint = curveSamplePoints[sampleIndex]
  // 最近交点占当前车道的百分比
  const percent = sampleIndex / curveSamplePoints.length
  // 获取最近交点的切线方向
  const tangent: Vector3 = curvePath.getTangentAt(percent)
  return {
    point: closestPoint,
    tangent,
    percent,
  }
}

/**
 * 获取路径上某一个百分比的参考点右侧的法向量方向 offset 偏移量的点坐标
 * @param params
 */
export function getSamplePointByOffset (params: {
  keyPath: biz.ICurve3
  elevationPath?: biz.ICurve3
  percent: number
  offset: number
}) {
  const { keyPath, percent, offset, elevationPath } = params
  // 百分比不超过 1
  const _percent = Math.min(percent, 1)
  const refPoint = keyPath.getPointAt(_percent)
  const tangent = keyPath.getTangentAt(_percent)
  const vertical = getVerticalVector(tangent)
  const offsetPoint = refPoint.clone()
  offsetPoint.addScaledVector(vertical, offset)
  // 如果高程的曲线存在
  if (elevationPath) {
    const elevationPoint = elevationPath.getPointAt(percent)
    // 使用高程曲线的高度
    offsetPoint.y = elevationPoint.y
  }
  return {
    refPoint,
    offsetPoint,
  }
}

/**
 * 基于两个点获取偏移的补间点
 * @param params
 * @returns
 */
export function getOffsetTweenPoint (params: {
  v1: Vector3 | common.vec3 | common.vec3WithId
  v2: Vector3 | common.vec3 | common.vec3WithId
  angle: number
  fixed?: boolean
  newId?: boolean
  offsetDistance: number
}) {
  const { v1, v2, angle, fixed = false, newId = false, offsetDistance } = params
  // 计算中心点的初始坐标
  const _x = (v1.x + v2.x) / 2
  const _y = (v1.y + v2.y) / 2
  const _z = (v1.z + v2.z) / 2
  // 计算 xz 平面的旋转向量
  const offsetInXZ = new Vector2(v2.x - v1.x, v2.z - v1.z)
  // const length = offsetInXZ.length()
  const originPoint = new Vector2(0, 0)
  offsetInXZ
    .rotateAround(originPoint, angle)
    .normalize()
    // TODO 后续可以根据特殊情况再优化 length 的算法
    .setLength(offsetDistance)
  // 中心点坐标 + 旋转一定角度偏移量坐标
  let p: common.vec3 | common.vec3WithId = {
    x: _x + offsetInXZ.x,
    y: _y,
    z: _z + offsetInXZ.y,
  }

  if (fixed) {
    // 精度调整
  }
  if (newId) {
    p = {
      ...p,
      id: genUuid(),
    }
  }
  return p
}

/**
 * 从三维顶点数据中，找跟目标点最接近的一个点，可接受误差
 * @param targetPoint
 * @param points
 */
export function findClosestPoint (params: {
  targetPoint: Vector3 | common.vec3 | common.vec3WithId
  points: Array<Vector3 | common.vec3 | common.vec3WithId>
  enableDeviation?: boolean
  newId?: boolean
}) {
  const { targetPoint, points, enableDeviation = true, newId = false } = params
  let index = -1
  let closestPoint: common.vec3 | common.vec3WithId | null = null
  // 如果不接受误差，那么误差值为 0
  const deviation = enableDeviation ? undefined : 0
  for (let i = 0; i < points.length; i++) {
    const point = points[i]
    const isOk = compareTwoVector3InDeviation(targetPoint, point, deviation)
    if (isOk) {
      closestPoint = {
        x: point.x,
        y: point.y,
        z: point.z,
      }
      if (newId) {
        // 如果需要 id 属性，则重新赋值，保证类型检测的准确性
        closestPoint = {
          x: point.x,
          y: point.y,
          z: point.z,
          id: genUuid(),
        }
      }
      index = i
      break
    }
  }
  return {
    point: closestPoint,
    index,
  }
}

/**
 * 判断两个三维向量是否在可接受的误差范围内（可以理解成是否过于接近），默认误差为 0.005，对应 5 毫米
 * @param vecA
 * @param vecB
 * @param deviation
 */
export function compareTwoVector3InDeviation (
  vecA: common.vec3,
  vecB: common.vec3,
  deviation = 0.005,
) {
  const xIsEqual = compareTwoNumberInDeviation(vecA.x, vecB.x, deviation)
  const yIsEqual = compareTwoNumberInDeviation(vecA.y, vecB.y, deviation)
  const zIsEqual = compareTwoNumberInDeviation(vecA.z, vecB.z, deviation)
  return xIsEqual && yIsEqual && zIsEqual
}

/**
 * 判断两个数是否在可接受的误差范围内，默认误差为 0.005，对应 5 毫米
 * @param num1
 * @param num2
 * @param deviation
 * @returns
 */
export function compareTwoNumberInDeviation (
  num1: number,
  num2: number,
  deviation = 0.005,
) {
  const absRes = Math.abs(num1 - num2)
  return absRes <= deviation
}

export function getCenterLineByTwoLines (
  line1: Array<common.vec3 | Vector3>,
  line2: Array<common.vec3 | Vector3>,
  fixed?: boolean,
  isReverse?: boolean, // 是否是反向的
) {
  const length = Math.min(line1.length, line2.length)
  const points = []
  for (let i = 0; i < length; i++) {
    const center = getCenterByTwoVec3({
      v1: line1[i],
      v2: line2[i],
      fixed,
    })
    if (!isReverse) {
      points.push(center)
    } else {
      points.unshift(center)
    }
  }
  return points
}

/**
 * 调整数字的精度
 * @param num
 * @param precision
 * @returns
 */
export function fixedPrecision (num: number, precision: number = 3) {
  return Number(num.toFixed(precision))
}

/**
 * 调整矢量数据的精度【Warning：该方法频繁同步调用，会有性能问题！！！】
 * @param vec3
 * @param precision 默认3位小数点，对应精确到 1cm 级别，精度足够用
 * 在 threejs curve.getPonits 获取的采样点中，3位小数点的精度能够保持稳定，避免因为过多的小数点引发的采样点 diff 判断错误
 */
export function fixedVec3Precision (
  vec3: common.vec3 | Vector3,
  precision: number = 3,
) {
  vec3.x = Number(vec3.x.toFixed(precision))
  vec3.y = Number(vec3.y.toFixed(precision))
  vec3.z = Number(vec3.z.toFixed(precision))
}

/**
 * 获取两个矢量点的中点坐标
 * @param params
 * @returns
 */
export function getCenterByTwoVec3 (params: {
  v1: Vector3 | common.vec3 | common.vec3WithId
  v2: Vector3 | common.vec3 | common.vec3WithId
  fixed?: boolean
  newId?: boolean
}) {
  const { v1, v2, fixed = false, newId = false } = params
  const _x = (v1.x + v2.x) / 2
  const _y = (v1.y + v2.y) / 2
  const _z = (v1.z + v2.z) / 2
  let center: common.vec3 | common.vec3WithId = {
    x: _x,
    y: _y,
    z: _z,
  }
  if (fixed) {
    // 精度调整
  }
  if (newId) {
    center = {
      ...center,
      id: genUuid(),
    }
  }

  return center
}

/**
 * 支持由小到大的排序
 * @param a
 * @param b
 * @returns
 */
export function orderByAsc (a: number | string, b: number | string) {
  const _a = typeof a === 'string' ? Number(a) : a
  const _b = typeof b === 'string' ? Number(b) : b
  if (_a > _b) {
    return 1
  } else if (_a < _b) {
    return -1
  } else {
    return 0
  }
}

interface ISortItem {
  id: string | number
}
/**
 * 按照 id 由小到大排序
 * @param itemA
 * @param itemB
 * @returns
 */
export function sortById (itemA: ISortItem, itemB: ISortItem) {
  const idA = Number(itemA.id)
  const idB = Number(itemB.id)
  return orderByAsc(idA, idB)
}

/**
 * 按照 id 绝对值由小到大排序
 * @param itemA
 * @param itemB
 * @returns
 */
export function sortByAbsId (itemA: ISortItem, itemB: ISortItem) {
  const idA = Math.abs(Number(itemA.id))
  const idB = Math.abs(Number(itemB.id))
  return orderByAsc(idA, idB)
}

/**
 * 线性获取两个矢量点的插值
 * @param params
 */
export function getLinearValueByTwoPoints (params: {
  point1: common.vec3 | Vector3
  point2: common.vec3 | Vector3
  ratio: number // ratio 是 [0, 1] 区间的任意比例
}) {
  const { point1, point2, ratio } = params
  // 由于不确定输入的是否是 Vector3 类型的点，手动创一个
  const v1 = new Vector3(point1.x, point1.y, point1.z)
  const v2 = new Vector3(point2.x, point2.y, point2.z)

  const tweenPoint = new Vector3(0, 0, 0)
    .addScaledVector(v1, ratio)
    .addScaledVector(v2, 1 - ratio)
  return tweenPoint
}

/**
 * 通过 2 个端点，以及控制点跟端点形成的方向，创建【二维】的贝塞尔曲线
 * 控制点到端点的距离，跟两端点之间距离成固定比例
 * @param params
 */
export function getBezier2WithDirection (params: {
  point1: common.vec2
  direction1: common.vec2
  direction2: common.vec2
  point2: common.vec2
  ratio?: number
}) {
  const {
    point1,
    point2,
    direction1,
    direction2,
    ratio = Constant.junctionBCPRatio,
  } = params
  const _point1 = new Vector2(point1.x, point1.y)
  const _point2 = new Vector2(point2.x, point2.y)
  const _direction1 = new Vector2(direction1.x, direction1.y)
  const _direction2 = new Vector2(direction2.x, direction2.y)
  // 两个端点之间的距离
  const dist = _point1.distanceTo(_point2)

  // 端点的控制点，跟端点在同一平面上
  const controlPoint1 = _point1.clone().add(
    _direction1
      .clone()
      .normalize()
      .multiplyScalar(dist * ratio),
  )
  const controlPoint2 = _point2.clone().add(
    _direction2
      .clone()
      .normalize()
      .multiplyScalar(dist * ratio),
  )
  const bezierCurve = new CubicBezierCurve(
    _point1,
    controlPoint1,
    controlPoint2,
    _point2,
  )
  return bezierCurve
}

/**
 * 通过 2 个端点，以及控制点跟端点形成的方向，创建【三维】的贝塞尔曲线
 * 控制点到端点的距离，跟两端点之间距离成固定比例
 * @param params
 * @returns
 */
export function getBezier3WithDirection (params: {
  point1: common.vec3
  direction1: common.vec3
  direction2: common.vec3
  point2: common.vec3
  ratio?: number
}) {
  const {
    point1,
    point2,
    direction1,
    direction2,
    ratio = Constant.junctionBCPRatio,
  } = params
  const _point1 = new Vector3(point1.x, point1.y, point1.z)
  const _point2 = new Vector3(point2.x, point2.y, point2.z)
  const _direction1 = new Vector3(direction1.x, direction1.y, direction1.z)
  const _direction2 = new Vector3(direction2.x, direction2.y, direction2.z)
  // 两个端点之间的距离
  const dist = _point1.distanceTo(_point2)

  // 端点的控制点，跟端点在同一平面上
  const controlPoint1 = _point1.clone().add(
    _direction1
      .clone()
      .normalize()
      .multiplyScalar(dist * ratio),
  )
  const controlPoint2 = _point2.clone().add(
    _direction2
      .clone()
      .normalize()
      .multiplyScalar(dist * ratio),
  )
  const bezierCurve = new CubicBezierCurve3(
    _point1,
    controlPoint1,
    controlPoint2,
    _point2,
  )
  // 连带控制点一起返回
  return {
    curve: bezierCurve,
    cp1: controlPoint1,
    cp2: controlPoint2,
  }
}

/**
 * 获取绕 Y 轴顺时针旋转 90 度的单位向量
 * @param vec3
 * @returns
 */
export function getVerticalVector (vec3: common.vec3) {
  return new Vector3(-vec3.z, 0, vec3.x).normalize()
}

/**
 * 获取 Curve 曲线某一侧的端点切线方向的向量在 xz 平面的分量
 * 路径结尾处为同方向，路径开头处要调整为反方向
 * @param path
 * @param isTail
 */
export function getEndPointAlongVec3 (path: biz.ICurve3, isTail: boolean) {
  let tangent = new Vector3(0, 0, 0)
  tangent = path.getTangentAt(isTail ? 1 : 0, tangent)
  const _tangent = new Vector3(0, 0, 0)
  _tangent.x = isTail ? tangent.x : -tangent.x
  _tangent.z = isTail ? tangent.z : -tangent.z
  _tangent.normalize()
  return _tangent
}

/**
 * 获取 Curve 曲线首尾端点切线方向的垂直向量在 xz 平面上的分量
 * @param path
 * @returns
 */
export function getCurveEndPointVerticalVec3 (path: biz.ICurve3) {
  let tHead = new Vector3(1, 0, 0)
  let tTail = new Vector3(1, 0, 0)
  tHead = path.getTangentAt(0, tHead)
  tTail = path.getTangentAt(1, tTail)
  const vHead = getVerticalVector(tHead)
  const vTail = getVerticalVector(tTail)
  return [vHead, vTail]
}

/**
 * 地图文件中，xy 平面表示地平面，z 轴为高程信息
 * threejs 中，zx 平面表示地平面，y 轴为高程信息
 * 将采样点坐标在两种格式间做转换
 * @param point
 * @returns
 */
// 单个坐标
export function transformOneCoordinateToCommon (point: common.vec3) {
  return { x: point.z, y: point.x, z: point.y }
}
// 一组坐标
export function transformCoordinateToCommon (points: Array<common.vec3>) {
  return points.map(p => ({
    x: p.z,
    y: p.x,
    z: p.y,
  }))
}

export function transformCoordinateToThree (
  points: Array<common.vec3>,
): Array<common.vec3> {
  return points.map(p => ({
    z: p.x,
    x: p.y,
    y: p.z,
  }))
}

/**
 * 将 threejs 三维矢量的对象，还原成普通对象属性
 * @param points
 * @returns
 */
export function transformVec3ToObject (points: Array<Vector3>) {
  return points.map(p => ({
    x: p.x,
    y: p.y,
    z: p.z,
  }))
}

/**
 * 将符合 xyz 坐标形式的对象，转换成 threejs 自带的 Vector3 对象
 * @param vec3
 * @returns
 */
export function transformVec3 (vec3: common.vec3 | common.vec3WithId) {
  return new Vector3(vec3.x, vec3.y, vec3.z)
}

/**
 * 将符合 xyz 坐标形式的数组，转换成 threejs 自带的 Vector3 对象
 * @param points
 * @returns
 */
export function transformVec3ByArray (points: Array<common.vec3Arr>) {
  return points.map(p => new Vector3(p[0], p[1], p[2]))
}

/**
 * 将符合 xyz 坐标形式的对象，转换成 threejs 自带的 Vector3 对象
 * @param points
 * @returns
 */
export function transformVec3ByObject (points: Array<common.vec3>) {
  return points.map(p => new Vector3(p.x, p.y, p.z))
}

/**
 * 将扁平结构的 number 类型数据，按照每 3 个一组的形式组装成三维顶点 Vector3 数组
 * @param points
 */
export function transformVec3ByFlatArray (points: Array<number>) {
  const _points = []
  for (let i = 0; i < points.length; i += 3) {
    const p = new Vector3(points[i], points[i + 1], points[i + 2])
    _points.push(p)
  }
  return _points
}

/**
 * 遍历销毁 mesh 中的 material
 * @param mesh
 */
export function disposeMaterial (mesh: Mesh | Line | Points) {
  if (Array.isArray(mesh.material)) {
    mesh.material.forEach((mat: Material) => mat.dispose())
  } else {
    mesh.material.dispose()
  }
}
