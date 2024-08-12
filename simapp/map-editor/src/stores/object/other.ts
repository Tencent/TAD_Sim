import type { Intersection } from 'three'
import {
  createDefaultObjectData,
  getAlongRoadFrontPoint,
  getLocationInRoad,
} from './common'
import type { IOtherConfig } from '@/config/other'
import {
  fixedPrecision,
  getLocationByST,
  getLookAtPointByAngle,
  getLookAtPointByYaw,
  getValidDegreeInRange,
} from '@/utils/common3d'
import { genObjectId } from '@/utils/guid'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import { Constant } from '@/utils/business'

// 更新人行天桥的跨度
export function updateBridgeSpan (params: { objectId: string, span: number }) {
  const { objectId, span } = params
  const bridgeData = getObject(objectId) as biz.IOther
  if (!bridgeData) return null

  bridgeData.span = fixedPrecision(span)

  setObject(bridgeData.id, bridgeData)

  return bridgeData
}

// 更新其他类型物体的尺寸
export function updateOtherSize (params: {
  objectId: string
  length: number
  width: number
  height?: number
}) {
  const { objectId, length, width, height } = params
  const otherData = getObject(objectId) as biz.IOther
  if (!otherData) return null

  otherData.length = fixedPrecision(length)
  otherData.width = fixedPrecision(width)
  if (height) {
    otherData.height = fixedPrecision(height)
  }

  setObject(otherData.id, otherData)

  return otherData
}

/**
 * 基于配置项，创建一个其他类型物体的数据结构
 * @param params
 */
export function createOther (params: {
  option: IOtherConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const {
    name,
    showName,
    type,
    subtype,
    isPlaneModel,
    initYaw = 0,
    imageRatio,
    basicSize,
  } = option

  const otherData: biz.IOther = {
    ...createDefaultObjectData(),
    isPlaneModel,
    basicLength: 0,
    basicWidth: 0,
    basicHeight: 0,
  }
  if (isPlaneModel && imageRatio && basicSize) {
    const size = [basicSize, basicSize / imageRatio]
    otherData.size = size
  }
  // 天桥需要增加跨度属性
  if (name === 'PedestrianBridge') {
    otherData.span = Constant.bridgeSpan
  }

  otherData.id = genObjectId()
  otherData.mainType = 'other'
  otherData.name = name
  otherData.showName = showName
  otherData.type = type
  otherData.subtype = subtype

  // 基于鼠标交互结果，更新物体的【空间属性】
  const { point, object } = intersection
  // @ts-expect-error
  const { roadId = '', junctionId = '' } = object

  // 将位置同步到鼠标射线检测到的位置
  otherData.position = {
    x: point.x,
    y: point.y,
    z: point.z,
  }

  // 基于交互的配置项，更新物体的一些属性
  if (roadId) {
    otherData.roadId = roadId
    // TODO 如果是跟道路关联，计算对应的 st 坐标
    // 默认道路上的物体，都沿着道路参考线摆放，会自动调整角度
    const locationData = getLocationInRoad({
      point,
      roadId,
      useElevation: true,
    })
    if (!locationData) return null
    const { s, t, percent, tangent, closestPointOnRefLine, normal } =
      locationData

    otherData.s = s
    otherData.t = t
    otherData.closestPoint = closestPointOnRefLine
    otherData.closestPointTangent = tangent
    otherData.closestPointPercent = percent
    otherData.projectNormal = normal

    // 判断是否有默认的初始偏航角
    if (initYaw === 0) {
      otherData.yaw = t <= 0 ? 0 : 180
      // 默认看向 s 轴切线的前方
      const frontPoint = getAlongRoadFrontPoint({
        tangent,
        point,
        isForward: t <= 0,
      })
      otherData.lookAtPoint = frontPoint
    } else {
      // 如果有初始偏航角
      const _yaw = t <= 0 ? initYaw : getValidDegreeInRange(initYaw + 180)
      otherData.yaw = _yaw
      const lookAtPoint = getLookAtPointByYaw({
        pointOnRefLine: closestPointOnRefLine,
        tangent,
        point,
        yaw: _yaw,
        useElevation: true,
      })
      otherData.lookAtPoint = lookAtPoint
    }
  }
  if (junctionId) {
    otherData.junctionId = junctionId
    // 世界坐标系下的角度默认为 0
    otherData.angle = 0
  }

  // 将新增的物体缓存到本地
  setObject(otherData.id, otherData)

  return otherData
}

// 移动其他类型物体
export function moveOther (params: {
  objectId: string
  point: common.vec3
  roadId?: string
  refLineLocation?: biz.IRefLineLocation // refLineLocation 跟 roadId 同时存在
}) {
  const {
    objectId,
    point: targetPoint,
    refLineLocation = null,
    roadId = '',
  } = params
  const otherData = getObject(objectId)
  if (!otherData) return

  // 放置在道路上时对应的定制化属性更新
  if (roadId) {
    const locationData =
      refLineLocation ||
      getLocationInRoad({
        point: targetPoint,
        roadId,
        useElevation: true,
      })

    if (!locationData) return null

    // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
    let yaw = otherData.yaw
    if (
      (otherData.t < 0 && locationData.t > 0) ||
      (otherData.t > 0 && locationData.t < 0)
    ) {
      yaw = getValidDegreeInRange(yaw + 180)
    }

    const { s, t, percent, tangent, closestPointOnRefLine, normal } =
      locationData

    otherData.s = s
    otherData.t = t
    otherData.closestPointTangent = tangent
    otherData.closestPointPercent = percent
    otherData.closestPoint = closestPointOnRefLine
    otherData.projectNormal = normal

    // 基于当前的偏航角，计算朝向
    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: closestPointOnRefLine,
      tangent,
      point: targetPoint,
      yaw,
      useElevation: true,
    })
    otherData.yaw = yaw
    otherData.lookAtPoint = lookAtPoint
  } else {
    // 针对路口中的物体，基于全局的角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle: otherData.angle,
    })
    otherData.lookAtPoint = lookAtPoint
  }

  otherData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }

  setObject(otherData.id, otherData)

  return otherData
}

// 通过 st 坐标移动其他类型物体
export function moveOtherByST (params: {
  objectId: string
  s: number
  t: number
}) {
  const { objectId, s, t } = params
  const otherData = getObject(objectId)
  if (!otherData) return

  // 通过 st 修改物体的位置，说明 roadId 一定存在
  const { roadId } = otherData
  const roadData = getRoad(roadId)
  if (!roadData) return

  // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
  let yaw = otherData.yaw
  if ((otherData.t < 0 && t > 0) || (otherData.t > 0 && t < 0)) {
    yaw = getValidDegreeInRange(yaw + 180)
  }

  // 返回的 s 是校正后的值
  const {
    s: newS,
    refLinePoint,
    targetPoint,
    tangent,
    percent,
    normal,
  } = getLocationByST({
    s,
    t,
    useElevationTangent: true,
    elevationPath: roadData.elevationPath,
    curvePath: roadData.keyPath,
  })

  // 更新源数据属性
  otherData.s = newS
  otherData.t = t
  otherData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }
  otherData.closestPoint = refLinePoint
  otherData.closestPointPercent = percent
  otherData.closestPointTangent = tangent
  otherData.projectNormal = normal

  // 基于当前的偏航角，计算朝向
  const lookAtPoint = getLookAtPointByYaw({
    pointOnRefLine: refLinePoint,
    tangent,
    point: targetPoint,
    yaw,
    useElevation: true,
  })
  otherData.yaw = yaw
  otherData.lookAtPoint = lookAtPoint

  setObject(otherData.id, otherData)

  return otherData
}

// 旋转其他类型物体的角度
export function rotateOther (params: {
  objectId: string
  yaw?: number
  angle?: number
}) {
  const { objectId, yaw, angle } = params
  const otherData = getObject(objectId)
  if (!otherData) return

  const { position } = otherData
  if (yaw !== undefined) {
    // 针对道路上的物体，基于偏航角来更新物体的角度
    const { closestPointTangent, closestPoint } = otherData
    if (!closestPointTangent || !closestPoint) return null

    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: closestPoint,
      tangent: closestPointTangent,
      point: position,
      yaw,
      useElevation: true,
    })

    otherData.yaw = yaw
    otherData.lookAtPoint = lookAtPoint
  }

  if (angle !== undefined) {
    // 针对路口中的物体，基于全局的角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: position,
      angle,
    })
    otherData.angle = angle
    otherData.lookAtPoint = lookAtPoint
  }

  setObject(otherData.id, otherData)

  return otherData
}
