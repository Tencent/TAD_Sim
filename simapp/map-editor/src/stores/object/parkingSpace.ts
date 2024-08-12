// 停车位相关的方法，属于路面标识的特例

import type { Intersection } from 'three'
import { createDefaultObjectData, getLocationInRoad } from './common'
import type { IParkingSpaceConfig } from '@/config/parkingSpace'
import { genObjectId } from '@/utils/guid'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import { getCornerVerticesAttr } from '@/utils/business'
import { getLocationByST } from '@/utils/common3d'

// 创建一组通用停车位（默认为单个停车位）
export function createParkingSpace (params: {
  option: IParkingSpaceConfig
  intersection: Intersection
}) {
  const { option, intersection } = params

  const {
    name,
    showName,
    type,
    subtype,
    lineWidth,
    color,
    width,
    length,
    innerAngle,
    mainType,
  } = option
  const parkingSpaceData: biz.IParkingSpace = {
    ...createDefaultObjectData(),
    color,
    lineWidth,
    width,
    length,
    margin: length, // 初始的相邻停车位间距，默认为一个停车位的长度
    innerAngle, // 初始夹角
    count: 3, // 默认为 3 个停车位
    geoAttrs: [],
    cornerPoints: [],
    cornerSTPoints: [],
    sCoordinates: [],
  }
  parkingSpaceData.id = genObjectId()
  parkingSpaceData.mainType = mainType
  parkingSpaceData.name = name
  parkingSpaceData.showName = showName
  parkingSpaceData.type = type
  parkingSpaceData.subtype = subtype

  // 基于鼠标交互结构，更新停车位的空间属性
  const { object, point: intersectionPoint } = intersection

  // @ts-expect-error
  const { roadId, junctionId } = object
  if (roadId) {
    // 放置在道路上
    const road = getRoad(roadId)
    if (!road) return

    // 计算停车位放置点跟参考线相关的位置属性
    const locationRes = getLocationInRoad({
      point: intersectionPoint,
      roadId,
      useElevation: true,
    })

    if (!locationRes) return

    // 虚线的限时停车位
    const isDashed = name === 'Time_Limit_Parking_Space_Mark'

    const { count, margin, yaw = 0 } = parkingSpaceData
    const payload = {
      // 透传 biz.ICurve 类型的对象，在 webWorker 线程中没法引用
      curvePathPoints: road.keyPath.points,
      refLineLocation: locationRes,
      count,
      width,
      length,
      margin,
      lineWidth,
      yaw,
      innerAngle,
      isDashed,
    }
    if (road.elevationPath) {
      payload.elevationPathPoints = road.elevationPath.points
    }

    // 新增停车位的计算量相对来说较小，使用同步计算
    // 对于加载文件解析停车位，使用 webWorker 线程池来异步计算
    const { geoAttrs, cornerPoints, sCoordinates, cornerSTPoints } =
      getCornerVerticesAttr(payload)
    if (!geoAttrs || geoAttrs.length < 1) return

    // 跟道路绑定
    parkingSpaceData.roadId = roadId
    // 一组停车位的空间属性
    parkingSpaceData.sCoordinates = sCoordinates
    parkingSpaceData.geoAttrs = geoAttrs
    parkingSpaceData.cornerPoints = cornerPoints
    parkingSpaceData.cornerSTPoints = cornerSTPoints
    parkingSpaceData.t = locationRes.t
    // s 坐标默认为第一个停车位几何中心点的 s 坐标
    parkingSpaceData.s = sCoordinates[0]

    // 以第一个停车位几何中心点的坐标，作为一组重复停车位的基准位置
    parkingSpaceData.position = {
      x: intersectionPoint.x,
      y: intersectionPoint.y,
      z: intersectionPoint.z,
    }
  } else if (junctionId) {
    // 暂不支持在路口中放置停车位
  }

  setObject(parkingSpaceData.id, parkingSpaceData)

  return parkingSpaceData
}

// 移动停车位
export function moveParkingSpace (params: {
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

  const parkingSpaceData = getObject(objectId) as biz.IParkingSpace
  if (!parkingSpaceData) return null

  // 目前先支持道路上的停车位移动，路口中的先不支持
  if (!roadId) return null
  const road = getRoad(roadId)
  if (!road) return null

  // 如果传入了已经计算好的参考线坐标系数据，则可直接用，无须重复计算
  const locationData =
    refLineLocation ||
    getLocationInRoad({
      point: targetPoint,
      roadId,
      useElevation: true,
    })
  if (!locationData) return null

  // 虚线的限时停车位
  const isDashed = parkingSpaceData.name === 'Time_Limit_Parking_Space_Mark'

  const { width, length, lineWidth, yaw, innerAngle, count, margin } =
    parkingSpaceData
  const { geoAttrs, cornerPoints, sCoordinates, cornerSTPoints } =
    getCornerVerticesAttr({
      curvePathPoints: road.keyPath.points,
      elevationPathPoints: road.elevationPath ?
        road.elevationPath.points :
        undefined,
      refLineLocation: locationData,
      count,
      width,
      length,
      margin,
      lineWidth,
      yaw,
      innerAngle,
      isDashed,
    })
  if (!geoAttrs) return

  parkingSpaceData.geoAttrs = geoAttrs
  parkingSpaceData.cornerPoints = cornerPoints
  parkingSpaceData.cornerSTPoints = cornerSTPoints
  parkingSpaceData.sCoordinates = sCoordinates
  parkingSpaceData.t = locationData.t
  // s 坐标默认为第一个停车位几何中心点的 s 坐标
  parkingSpaceData.s = sCoordinates[0]
  // 移动后重新保存停车位的基准定位点（第一个车位几何中心点）
  parkingSpaceData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }

  setObject(parkingSpaceData.id, parkingSpaceData)

  return parkingSpaceData
}

// 通过st坐标移动停车位
export function moveParkingSpaceByST (params: {
  objectId: string
  s: number
  t: number
}) {
  const { objectId, s, t } = params
  const parkingSpaceData = getObject(objectId) as biz.IParkingSpace
  if (!parkingSpaceData) return

  const {
    roadId = '',
    count,
    yaw,
    width,
    length,
    margin,
    lineWidth,
    innerAngle,
  } = parkingSpaceData

  // 目前先支持道路上停车位的移动，路口中的暂时不支持
  const road = getRoad(roadId)
  if (!road) return

  // 基于传入的 st 坐标来计算新的位置
  const {
    s: newS,
    refLinePoint,
    targetPoint,
    tangent,
    percent,
  } = getLocationByST({
    s,
    t,
    useElevationTangent: true,
    elevationPath: road.elevationPath,
    curvePath: road.keyPath,
  })

  // 虚线的限时停车位
  const isDashed = parkingSpaceData.name === 'Time_Limit_Parking_Space_Mark'

  const payload = {
    curvePathPoints: road.keyPath.points,
    refLineLocation: {
      s: newS,
      t,
      percent,
      tangent,
      closestPointOnRefLine: refLinePoint,
    },
    count,
    width,
    length,
    margin,
    lineWidth,
    yaw,
    innerAngle,
    isDashed,
  }
  if (road.elevationPath) {
    payload.elevationPathPoints = road.elevationPath.points
  }

  const { geoAttrs, cornerPoints, sCoordinates, cornerSTPoints } =
    getCornerVerticesAttr(payload)
  if (!geoAttrs || geoAttrs.length < 1) return

  parkingSpaceData.sCoordinates = sCoordinates
  parkingSpaceData.cornerPoints = cornerPoints
  parkingSpaceData.cornerSTPoints = cornerSTPoints
  parkingSpaceData.geoAttrs = geoAttrs
  parkingSpaceData.s = newS
  parkingSpaceData.t = t
  parkingSpaceData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }

  setObject(parkingSpaceData.id, parkingSpaceData)

  return parkingSpaceData
}

// // 调整停车位的偏航角
export function rotateParkingSpace (params: { objectId: string, yaw: number }) {
  const { objectId, yaw } = params
  const parkingSpaceData = getObject(objectId) as biz.IParkingSpace
  if (!parkingSpaceData) return null

  const {
    roadId = '',
    count,
    width,
    length,
    margin,
    lineWidth,
    innerAngle,
    position,
  } = parkingSpaceData

  // 目前先支持道路上停车位的移动，路口中的暂时不支持
  const road = getRoad(roadId)
  if (!road) return

  // 计算停车位放置点跟参考线相关的位置属性
  const locationRes = getLocationInRoad({
    point: position,
    roadId,
    useElevation: true,
  })

  if (!locationRes) return

  // 虚线的限时停车位
  const isDashed = parkingSpaceData.name === 'Time_Limit_Parking_Space_Mark'

  const payload = {
    // 透传 biz.ICurve 类型的对象，在 webWorker 线程中没法引用
    curvePathPoints: road.keyPath.points,
    refLineLocation: locationRes,
    count,
    width,
    length,
    margin,
    lineWidth,
    yaw,
    innerAngle,
    isDashed,
  }
  if (road.elevationPath) {
    payload.elevationPathPoints = road.elevationPath.points
  }

  // 新增停车位的计算量相对来说较小，使用同步计算
  // 对于加载文件解析停车位，使用 webWorker 线程池来异步计算
  const { geoAttrs, cornerPoints, sCoordinates, cornerSTPoints } =
    getCornerVerticesAttr(payload)
  if (!geoAttrs || geoAttrs.length < 1) return

  // 旋转偏航角，只修改停车位顶点相关的属性
  parkingSpaceData.sCoordinates = sCoordinates
  parkingSpaceData.geoAttrs = geoAttrs
  parkingSpaceData.cornerPoints = cornerPoints
  parkingSpaceData.cornerSTPoints = cornerSTPoints
  parkingSpaceData.yaw = yaw

  setObject(parkingSpaceData.id, parkingSpaceData)

  return parkingSpaceData
}

// 检查添加重复停车位的可能性【每一次修改重复属性前，都需要调用这个方法】
export function checkRepeatValidation (params: {
  objectId: string
  count: number
}) {
  const { objectId, count } = params
  const parkingSpaceData = getObject(objectId) as biz.IParkingSpace
  if (!parkingSpaceData) return false

  const { roadId, sCoordinates, margin } = parkingSpaceData

  // 始终是沿着道路行驶方向延伸重复的停车位
  const road = getRoad(roadId)
  if (!road) return false
  const currentCount = sCoordinates.length
  // 如果减少重复的个数，说明肯定可以放置
  if (count < currentCount) return true

  // 获取最后一个停车位的几何中心点的 s 坐标
  const lastS = sCoordinates[sCoordinates.length - 1]
  // 由于是等距（停车位几何中心间距）放置重复的停车位
  // 如果后续新增的重复停车位几何中心点，则视为无效
  const diffCount = count - currentCount
  const isValid = lastS + diffCount * margin <= road.length
  return isValid
}

// 更新停车位的属性【长度、宽度、间距、重复数量、线颜色、线宽、车位内夹角】
// 更新任意一种属性，都需要重新计算停车位的几何体数据（除了颜色以外）
export type ParkingSpacePropertyType =
  | 'length'
  | 'width'
  | 'margin'
  | 'lineWidth'
  | 'color'
  | 'count'
  | 'innerAngle'

export function updateParkingSpaceProperty (params: {
  objectId: string
  type: ParkingSpacePropertyType
  value: string | number
}) {
  const { objectId, type, value } = params
  const parkingSpaceData = getObject(objectId) as biz.IParkingSpace
  if (!parkingSpaceData) return null

  // 默认需要更新结构
  let needUpdateGeo = true

  switch (type) {
    case 'color': {
      parkingSpaceData.color = value as common.colorType
      // 只需要修改颜色，几何结构不变
      needUpdateGeo = false
      break
    }
    case 'count': {
      // 一组停车位中重复的数量
      parkingSpaceData.count = value as number
      break
    }
    case 'width': {
      // 单个停车位宽度
      parkingSpaceData.width = value as number
      break
    }
    case 'length': {
      // 单个停车位长度
      parkingSpaceData.length = value as number
      // 需要联动修改相邻车位的间距
      // 此时为特殊情况，没有车位内夹角和偏航角
      parkingSpaceData.margin = parkingSpaceData.length
      break
    }
    case 'lineWidth': {
      // 停车位的描边线宽
      parkingSpaceData.lineWidth = value as number
      break
    }
    case 'innerAngle': {
      // 单个停车位内部夹角
      parkingSpaceData.innerAngle = value as number
      break
    }
    case 'margin': {
      // 相邻停车位的几何中心点的间距
      parkingSpaceData.margin = value as number
      break
    }
    default:
      break
  }

  // 更新一组停车位的几何结构
  if (needUpdateGeo) {
    const {
      count,
      width,
      length,
      lineWidth,
      yaw,
      innerAngle,
      position,
      margin,
      roadId = '',
    } = parkingSpaceData

    const road = getRoad(roadId)
    if (!road) return null

    // 基于第一个停车位几何中心点的坐标，重新计算一组新的参考线坐标系属性
    const refLineLocation = getLocationInRoad({
      point: position,
      roadId,
      useElevation: true,
    })
    if (!refLineLocation) return null

    // 虚线的限时停车位
    const isDashed = parkingSpaceData.name === 'Time_Limit_Parking_Space_Mark'

    const { geoAttrs, cornerPoints, sCoordinates, cornerSTPoints } =
      getCornerVerticesAttr({
        curvePathPoints: road.keyPath.points,
        elevationPathPoints: road.elevationPath ?
          road.elevationPath.points :
          undefined,
        refLineLocation,
        count,
        width,
        length,
        margin,
        lineWidth,
        yaw,
        innerAngle,
        isDashed,
      })
    if (!geoAttrs) return

    // 对于不需要更新基准点的情况，只需要更新每一个停车位自身的属性
    parkingSpaceData.geoAttrs = geoAttrs
    parkingSpaceData.cornerPoints = cornerPoints
    parkingSpaceData.cornerSTPoints = cornerSTPoints
    parkingSpaceData.sCoordinates = sCoordinates
  }

  setObject(parkingSpaceData.id, parkingSpaceData)

  return parkingSpaceData
}

// 基于一个已有的停车位复制一个同属性的新的停车位
export function copyParkingSpace (objectId: string) {
  const originData = getObject(objectId) as biz.IParkingSpace
  if (!originData) return null

  // 获取原来的停车位的属性
  const {
    width,
    color,
    lineWidth,
    length,
    margin,
    innerAngle,
    count,
    s,
    t,
    name,
    mainType,
    showName,
    type,
    subtype,
    roadId,
    yaw,
  } = originData

  const road = getRoad(roadId)
  if (!road) return null

  // 创建一个新的停车位的数据结构
  const copyData: biz.IParkingSpace = {
    ...createDefaultObjectData(),
    color,
    width,
    length,
    lineWidth,
    innerAngle,
    margin,
    count,
    geoAttrs: [],
    cornerPoints: [],
    cornerSTPoints: [],
    sCoordinates: [],
  }
  // 生成新的停车位 id
  copyData.id = genObjectId()
  copyData.mainType = mainType
  copyData.name = name
  copyData.showName = showName
  copyData.type = type
  copyData.subtype = subtype
  copyData.roadId = roadId
  copyData.yaw = yaw

  // 基于传入的 st 坐标来计算新的位置
  const {
    s: newS,
    t: newT,
    refLinePoint,
    targetPoint,
    tangent,
    percent,
  } = getLocationByST({
    // 复制的停车位参考线坐标，偏移半个停车位
    s: s + length / 2,
    t: t - width / 2,
    useElevationTangent: true,
    elevationPath: road.elevationPath,
    curvePath: road.keyPath,
  })

  // 虚线的限时停车位
  const isDashed = name === 'Time_Limit_Parking_Space_Mark'

  const payload = {
    curvePathPoints: road.keyPath.points,
    refLineLocation: {
      s: newS,
      t: newT,
      percent,
      tangent,
      closestPointOnRefLine: refLinePoint,
    },
    count,
    width,
    length,
    margin,
    lineWidth,
    yaw,
    innerAngle,
    isDashed,
  }
  if (road.elevationPath) {
    payload.elevationPathPoints = road.elevationPath.points
  }

  const { geoAttrs, cornerPoints, sCoordinates, cornerSTPoints } =
    getCornerVerticesAttr(payload)
  if (!geoAttrs || geoAttrs.length < 1) return

  copyData.sCoordinates = sCoordinates
  copyData.cornerPoints = cornerPoints
  copyData.cornerSTPoints = cornerSTPoints
  copyData.geoAttrs = geoAttrs
  copyData.s = newS
  copyData.t = newT
  copyData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }
  setObject(copyData.id, copyData)

  return copyData
}
