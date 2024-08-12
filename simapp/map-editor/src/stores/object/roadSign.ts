import { CatmullRomCurve3, type Intersection } from 'three'
import { useRoadStore } from '../road'
import {
  createDefaultObjectData,
  getAlongRoadFrontPoint,
  getClosestPointInLane,
  getLocationInRoad,
} from './common'
import { createCrosswalk, moveCrosswalk } from './customRoadSign/crosswalk'
import { createNonMotorVehicle } from './customRoadSign/nonMotorVehicleArea'
import { createStopLine, moveStopLine } from './customRoadSign/stopLine'
import {
  createGuideLaneLine,
  moveGuideLaneLine,
  updateGuideLaneLineSize,
} from './customRoadSign/guideLaneLine'
import {
  createVariableDirectionLaneLine,
  moveVariableDirectionLaneLine,
  updateVariableDirectionLaneLineSize,
} from './customRoadSign/variableDirectionLaneLine'
import {
  createCrosswalkWithSide,
  moveCrosswalkWithSide,
} from './customRoadSign/crosswalkWithSide'
import {
  createLongitudinalDeceleration,
  moveLongitudinalDeceleration,
  updateLongitudinalDecelerationSize,
} from './customRoadSign/longitudinalDecelerationMarking'
import {
  createLateralDeceleration,
  moveLateralDeceleration,
  updateLateralDecelerationSize,
} from './customRoadSign/lateralDecelerationMarking'
import {
  createSemicircleLine,
  moveSemicircleLine,
  updateSemicircleLineSize,
} from './customRoadSign/semicircleLine'
import {
  createIntersectionGuideLine,
  updateIntersectionGuideLine,
} from './customRoadSign/intersectionGuideLine'
import { getBoundaryById } from '@/stores/road/boundary'
import { getLaneById } from '@/stores/road/lane'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import { genObjectId } from '@/utils/guid'
import { getParallelCurveSamplePoints } from '@/utils/business'
import {
  CatmullromTension,
  CurveType,
  fixedPrecision,
  getClosestPointFromCurvePath,
  getLocationByST,
  getLookAtPointByAngle,
  getLookAtPointByYaw,
  getTValue,
  getValidDegreeInRange,
  transformVec3ByObject,
} from '@/utils/common3d'
import { type IRoadSignConfig, getRoadSignConfig } from '@/config/roadSign'

// 更新路口中特殊路面标线
export function updateRoadSignByJunction (objectId: string) {
  const roadSignData = getObject(objectId) as biz.IRoadSign
  if (!roadSignData) return

  const { name } = roadSignData
  if (name === 'Intersection_Guide_Line') {
    return updateIntersectionGuideLine(objectId)
  }

  return null
}

// 更新路面标线的尺寸
export function updateRoadSignSize (params: {
  objectId: string
  length: number
  width: number
}) {
  const { objectId, length, width } = params
  const roadSignData = getObject(objectId) as biz.IRoadSign
  if (!roadSignData) return null

  // 如果是基于 geoAttrs 创建的特殊路面标线，需要重新创建顶点属性
  switch (roadSignData.name) {
    case 'Road_Guide_Lane_Line':
      return updateGuideLaneLineSize(params)
    case 'Variable_Direction_Lane_Line':
      return updateVariableDirectionLaneLineSize(params)
    case 'Longitudinal_Deceleration_Marking':
      return updateLongitudinalDecelerationSize(params)
    case 'Lateral_Deceleration_Marking':
      return updateLateralDecelerationSize(params)
    case 'White_Semicircle_Line_Vehicle_Distance_Confirmation':
      return updateSemicircleLineSize(params)
    default:
      break
  }

  roadSignData.length = fixedPrecision(length)
  roadSignData.width = fixedPrecision(width)

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 基于车道结构的变换，更新车道上的路面标识
export function updateRoadSignByLane (params: {
  objectId: string
  type?: string
}) {
  const { objectId, type = '' } = params
  const roadSignData = getObject(objectId) as biz.IRoadSign
  if (!roadSignData) return

  const { s: originS, name } = roadSignData

  // 跟车道无关，仅跟道路总宽度有关的路面标线
  switch (name) {
    case 'Crosswalk_Line':
      // 由于人行横道横跨整条道路，在车道数量更新时，人行横道应该还是存在。除非删除对应的道路
      return moveCrosswalk({
        objectId,
        s: originS,
      })
    case 'Crosswalk_with_Left_and_Right_Side':
      return moveCrosswalkWithSide({
        objectId,
        s: originS,
      })
    case 'Stop_Line':
      // 停止线同理人行横道
      // 由于停止线的 s 坐标是固定不动的，可以基于 s 坐标重新计算停止线的属性
      return moveStopLine({
        objectId,
        s: originS,
      })
    default:
      break
  }

  // 跟车道紧密关联的特殊路面标线
  const { roadId, sectionId, laneId: originLaneId } = roadSignData

  // 判断是否需要更新标线关联的 laneId
  let targetLaneId
  if (type === 'remove') {
    // 判断正反向车道
    if (Number(originLaneId) > 0) {
      // 反向车道
      targetLaneId = String(Number(originLaneId) - 1)
    } else {
      // 正向车道
      targetLaneId = String(Number(originLaneId) + 1)
    }
  } else if (type === 'add') {
    // 新增一条车道，会导致目标车道外侧的车道 id 绝对值都 +1
    if (Number(originLaneId) > 0) {
      targetLaneId = String(Number(originLaneId) + 1)
    } else {
      targetLaneId = String(Number(originLaneId) - 1)
    }
  } else {
    targetLaneId = originLaneId
  }

  if (!roadId || !sectionId || !targetLaneId) return

  switch (name) {
    case 'Road_Guide_Lane_Line':
      // 移动导向车道线
      return moveGuideLaneLine({
        objectId,
        s: originS,
        laneId: targetLaneId,
      })
    case 'Variable_Direction_Lane_Line':
      // 移动可变导向车道线
      return moveVariableDirectionLaneLine({
        objectId,
        s: originS,
        laneId: targetLaneId,
      })
    case 'Longitudinal_Deceleration_Marking': {
      // 移动纵向减速标线
      return moveLongitudinalDeceleration({
        objectId,
        s: originS,
        laneId: targetLaneId,
      })
    }
    case 'Lateral_Deceleration_Marking': {
      // 移动横向减速标线
      return moveLateralDeceleration({
        objectId,
        s: originS,
        laneId: targetLaneId,
      })
    }
    case 'White_Semicircle_Line_Vehicle_Distance_Confirmation': {
      // 白色半圆状车距确认线
      return moveSemicircleLine({
        objectId,
        s: originS,
        laneId: targetLaneId,
      })
    }
    default:
      break
  }

  // 常规的路面标线
  const {
    position: originPosition,
    closestPoint: closestPointOnRefLine, // 距离道路参考线最近的点
    closestPointPercent, // 距离道路参考线最近点占当前道路参考线路径的比例
    closestPointTangent, // 距离道路参考线最近点的切线方向
  } = roadSignData

  if (!closestPointOnRefLine || !closestPointTangent) return

  const roadStore = useRoadStore()
  const queryLaneRes = roadStore.getLaneById(roadId, sectionId, targetLaneId)
  if (!queryLaneRes) return
  const { road, lane } = queryLaneRes

  // 计算路面标识原来的坐标，到目标车道中心线最近的点
  const { point: closestPoint } = getClosestPointInLane({
    lane,
    targetPoint: originPosition,
  })

  // 计算新位置的 st 坐标
  const s = fixedPrecision(road.length * closestPointPercent)
  const t = getTValue({
    tangent: closestPointTangent,
    refLinePoint: closestPointOnRefLine,
    targetPoint: closestPoint,
  })

  roadSignData.position = {
    x: closestPoint.x,
    y: closestPoint.y,
    z: closestPoint.z,
  }

  roadSignData.sectionId = sectionId
  roadSignData.laneId = targetLaneId
  roadSignData.s = s
  roadSignData.t = t

  // 手动控制路标看向的点为沿着道路前方的点
  const frontPoint = getAlongRoadFrontPoint({
    tangent: closestPointTangent,
    point: closestPoint,
    isForward: t <= 0,
  })
  roadSignData.lookAtPoint = frontPoint

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 计算鼠标射线交互点在道路中的计算后位置信息
export function calcRoadSignLocationInRoad (params: {
  point: common.vec3
  roadId: string
  sectionId: string
  laneId: string
}) {
  const { roadId, sectionId, laneId, point: intersectionPoint } = params
  if (!roadId || !sectionId || !laneId) return null

  const roadStore = useRoadStore()
  const queryLaneRes = roadStore.getLaneById(roadId, sectionId, laneId)
  if (!queryLaneRes) return null
  const { road, lane } = queryLaneRes
  // 获取路面标识到道路参考线最近的点
  const { keyPath, elevationPath } = road
  const {
    point: refLineClosestPoint,
    tangent,
    percent,
  } = getClosestPointFromCurvePath({
    curvePath: keyPath,
    point: intersectionPoint,
  })

  // 如果道路存在高程的路径
  if (elevationPath) {
    const elevationTangent = elevationPath.getTangentAt(percent)
    // 先将原切线向量归一化
    tangent.normalize()
    // 计算高度分量跟 s 轴分量的占比（x不可能为0）
    const ratio = elevationTangent.y / elevationTangent.x
    tangent.y = ratio
    // 调整高度分量后，再次归一化
    tangent.normalize()

    // // 道路参考线最近投影点的高度更新
    // const elevationPoint = elevationPath.getPointAt(percent)
    // refLineClosestPoint.y = elevationPoint.y
  }

  // 获取路面标识到车道中心线最近的点，将路面标识调整到车道线上最近的点处
  const { point: closestPoint } = getClosestPointInLane({
    lane,
    targetPoint: intersectionPoint,
  })

  // 获取 st 坐标
  const s = fixedPrecision(road.length * percent)
  const t = getTValue({
    tangent,
    refLinePoint: refLineClosestPoint,
    targetPoint: closestPoint,
  })

  return {
    s,
    t,
    tangent,
    percent,
    // 车道中心最近的点
    closestPointInLane: closestPoint,
    // 道路参考线上最近的点
    closestPointOnRefLine: refLineClosestPoint,
  }
}

// 关于路面标识的物体原子化操作方法
export function createRoadSign (params: {
  option: IRoadSignConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const {
    name,
    showName,
    type,
    subtype,
    imageRatio = 1,
    basicSize,
    freeMoveInRoad = false,
  } = option

  switch (name) {
    case 'Crosswalk_Line':
      // 人行横道
      return createCrosswalk(params)
    case 'Crosswalk_with_Left_and_Right_Side':
      // 行人左右分道的人行横道线
      return createCrosswalkWithSide(params)
    case 'Non_Motor_Vehicle_Area':
      // 非机动车禁驶区
      return createNonMotorVehicle(params)
    case 'Stop_Line':
      // 停止线
      return createStopLine(params)
    case 'Road_Guide_Lane_Line':
      // 导向车道线
      return createGuideLaneLine(params)
    case 'Variable_Direction_Lane_Line':
      // 可变导向车道线
      return createVariableDirectionLaneLine(params)
    case 'Longitudinal_Deceleration_Marking':
      // 纵向减速标线
      return createLongitudinalDeceleration(params)
    case 'Lateral_Deceleration_Marking':
      // 横向减速标线
      return createLateralDeceleration(params)
    case 'White_Semicircle_Line_Vehicle_Distance_Confirmation':
      // 横向减速标线
      return createSemicircleLine(params)
    case 'Intersection_Guide_Line':
      // 路口导向线【路口!】
      return createIntersectionGuideLine(params)
    default:
      break
  }

  const size = [basicSize, basicSize / imageRatio]
  const roadSignData: biz.IRoadSign = {
    ...createDefaultObjectData(),
    size,
    basicWidth: 0,
    basicLength: 0,
    basicHeight: 0,
  }
  roadSignData.id = genObjectId()
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype

  // 基于鼠标交互结果，更新物体的【空间属性】
  const { object, point: intersectionPoint } = intersection
  // @ts-expect-error
  const { roadId, sectionId, laneId, junctionId } = object
  if (freeMoveInRoad && roadId) {
    // 在车道上自由放置
    // 计算道路参考线相关的属性
    const locationData = getLocationInRoad({
      point: intersectionPoint,
      roadId,
      useElevation: true,
    })
    if (!locationData) return null
    const { s, t, percent, tangent, closestPointOnRefLine, normal } =
      locationData
    // 更新路面标识源数据的属性
    roadSignData.position = {
      x: intersectionPoint.x,
      y: intersectionPoint.y,
      z: intersectionPoint.z,
    }
    roadSignData.roadId = roadId
    roadSignData.s = s
    roadSignData.t = t
    roadSignData.closestPointTangent = tangent
    roadSignData.closestPointPercent = percent
    roadSignData.closestPoint = closestPointOnRefLine
    // 自由移动的路面标线，可以调节角度，需要 up 方向的调整
    roadSignData.projectNormal = normal

    // 默认的偏航角为 0
    // 区分正反向车道
    const yaw = t <= 0 ? 0 : 180
    roadSignData.yaw = yaw
    // 即默认看向 s 轴切线的前方
    const frontPoint = getAlongRoadFrontPoint({
      tangent,
      point: intersectionPoint,
      isForward: t <= 0,
    })
    roadSignData.lookAtPoint = frontPoint
  } else if (!freeMoveInRoad && roadId && sectionId && laneId) {
    // 放置在车道上
    const locationData = calcRoadSignLocationInRoad({
      point: intersection.point,
      roadId,
      sectionId,
      laneId,
    })
    if (!locationData) return null
    const {
      s,
      t,
      tangent,
      percent,
      closestPointInLane,
      closestPointOnRefLine,
    } = locationData

    // 更新路面标识源数据的属性
    roadSignData.position = {
      x: closestPointInLane.x,
      y: closestPointInLane.y,
      z: closestPointInLane.z,
    }
    roadSignData.roadId = roadId
    roadSignData.sectionId = sectionId
    roadSignData.laneId = laneId
    roadSignData.s = s
    roadSignData.t = t
    roadSignData.closestPoint = closestPointOnRefLine
    roadSignData.closestPointTangent = tangent
    roadSignData.closestPointPercent = percent

    // 默认的偏航角为 0
    // 区分正反向车道
    const yaw = t <= 0 ? 0 : 180
    roadSignData.yaw = yaw

    // 手动控制路标看向的点为沿着道路前方的点
    const frontPoint = getAlongRoadFrontPoint({
      tangent,
      point: closestPointInLane,
      isForward: t <= 0,
    })
    roadSignData.lookAtPoint = frontPoint
  } else if (junctionId) {
    // 放置在路口中

    // 更新路面标识源数据的属性
    roadSignData.position = {
      x: intersectionPoint.x,
      y: intersectionPoint.y,
      z: intersectionPoint.z,
    }
    roadSignData.junctionId = junctionId
  }

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 移动通过 geoAttrs 创建的自定义路面标线
export function moveCustomRoadSign (params: {
  objectId: string
  point: common.vec3
}) {
  const { objectId } = params
  const roadSignData = getObject(objectId)
  if (!roadSignData) return null
  const { name } = roadSignData
  // 找到路面标识的配置
  const config = getRoadSignConfig(name)
  if (!config) return null
  if (name === 'Road_Guide_Lane_Line') {
    // 导向车道线
    return moveGuideLaneLine(params)
  } else if (name === 'Variable_Direction_Lane_Line') {
    // 可变导向车道线
    return moveVariableDirectionLaneLine(params)
  } else if (name === 'Longitudinal_Deceleration_Marking') {
    // 纵向减速标线
    return moveLongitudinalDeceleration(params)
  } else if (name === 'Lateral_Deceleration_Marking') {
    // 横向减速标线
    return moveLateralDeceleration(params)
  } else if (name === 'White_Semicircle_Line_Vehicle_Distance_Confirmation') {
    // 白色半圆状车距确认线
    return moveSemicircleLine(params)
  }
}

// 移动路面标识
export function moveRoadSign (params: {
  objectId: string
  point: common.vec3
  roadId?: string
  refLineLocation?: biz.IRefLineLocation
}) {
  const {
    objectId,
    point: targetPoint,
    refLineLocation = null,
    roadId: targetRoadId = '',
  } = params
  const roadSignData = getObject(objectId)
  if (!roadSignData) return null
  const { name } = roadSignData
  // 找到路面标识的配置
  const config = getRoadSignConfig(name)
  if (!config) return null

  if (name === 'Stop_Line') {
    return moveStopLine({
      objectId,
      point: targetPoint,
    })
  } else if (name === 'Crosswalk_Line') {
    return moveCrosswalk({
      objectId,
      point: targetPoint,
    })
  } else if (name === 'Crosswalk_with_Left_and_Right_Side') {
    return moveCrosswalkWithSide({
      objectId,
      point: targetPoint,
    })
  }

  const { freeMoveInRoad = false } = config

  const { roadId, sectionId, laneId, junctionId } = roadSignData
  if (freeMoveInRoad && targetRoadId) {
    // 自由移动道路上的元素
    const locationData =
      refLineLocation ||
      getLocationInRoad({
        point: targetPoint,
        roadId,
        useElevation: true,
      })
    if (!locationData) return null

    // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
    let yaw = roadSignData.yaw
    if (
      (roadSignData.t < 0 && locationData.t > 0) ||
      (roadSignData.t > 0 && locationData.t < 0)
    ) {
      yaw = getValidDegreeInRange(yaw + 180)
    }

    const { s, t, percent, tangent, closestPointOnRefLine, normal } =
      locationData

    roadSignData.s = s
    roadSignData.t = t
    roadSignData.closestPointTangent = tangent
    roadSignData.closestPointPercent = percent
    roadSignData.closestPoint = closestPointOnRefLine
    roadSignData.projectNormal = normal

    // 基于当前的偏航角，计算朝向
    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: closestPointOnRefLine,
      tangent,
      point: targetPoint,
      yaw,
      useElevation: true,
    })
    roadSignData.yaw = yaw
    roadSignData.lookAtPoint = lookAtPoint

    roadSignData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
  } else if (!freeMoveInRoad && roadId && sectionId && laneId) {
    // 移动车道上的路面标识
    const locationData = calcRoadSignLocationInRoad({
      point: targetPoint,
      roadId,
      sectionId,
      laneId,
    })
    if (!locationData) return null
    const {
      s,
      t,
      tangent,
      percent,
      closestPointInLane,
      closestPointOnRefLine,
    } = locationData

    // 更新路面标识源数据的属性
    roadSignData.position = {
      x: closestPointInLane.x,
      y: closestPointInLane.y,
      z: closestPointInLane.z,
    }
    roadSignData.roadId = roadId
    roadSignData.laneId = laneId
    roadSignData.s = s
    roadSignData.t = t
    roadSignData.closestPoint = closestPointOnRefLine
    roadSignData.closestPointTangent = tangent
    roadSignData.closestPointPercent = percent

    // 手动控制路标看向的点为沿着道路前方的点
    const frontPoint = getAlongRoadFrontPoint({
      tangent,
      point: closestPointInLane,
      isForward: t <= 0,
    })
    roadSignData.lookAtPoint = frontPoint
  } else if (junctionId) {
    // 移动路口中的路面标识

    // 更新路面标识源数据的属性
    roadSignData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }

    // 针对于路口中的物体，基于全局角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle: roadSignData.angle,
    })
    roadSignData.lookAtPoint = lookAtPoint
  }

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 通过参考线坐标系的 st 值更新路面标识的位置
export function moveRoadSignByST (params: {
  objectId: string
  s: number
  t: number
}) {
  const { objectId, s, t } = params
  const roadSignData = getObject(objectId)
  if (!roadSignData) return

  switch (roadSignData.name) {
    case 'Stop_Line':
      return moveStopLine({
        objectId,
        s,
      })
    case 'Crosswalk_Line':
      return moveCrosswalk({
        objectId,
        s,
      })
    case 'Crosswalk_with_Left_and_Right_Side':
      return moveCrosswalkWithSide({
        objectId,
        s,
      })
    case 'Road_Guide_Lane_Line':
      return moveGuideLaneLine({
        objectId,
        s,
      })
    case 'Variable_Direction_Lane_Line':
      return moveVariableDirectionLaneLine({
        objectId,
        s,
      })
    case 'Longitudinal_Deceleration_Marking': {
      return moveLongitudinalDeceleration({
        objectId,
        s,
      })
    }
    case 'Lateral_Deceleration_Marking': {
      return moveLateralDeceleration({
        objectId,
        s,
      })
    }
    case 'White_Semicircle_Line_Vehicle_Distance_Confirmation': {
      return moveSemicircleLine({
        objectId,
        s,
      })
    }
    default:
      break
  }

  // 通过 st 修改物体的位置，说明 roadId 一定存在
  const { roadId } = roadSignData
  const roadData = getRoad(roadId)
  if (!roadData) return
  // 找到路面标识的配置
  const config = getRoadSignConfig(roadSignData.name)
  if (!config) return null
  const { freeMoveInRoad = false } = config

  // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
  let needUpdateYaw = false
  if (freeMoveInRoad) {
    if ((roadSignData.t < 0 && t > 0) || (roadSignData.t > 0 && t < 0)) {
      needUpdateYaw = true
    }
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
  roadSignData.s = newS
  roadSignData.t = t
  roadSignData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }
  roadSignData.closestPoint = refLinePoint
  roadSignData.closestPointPercent = percent
  roadSignData.closestPointTangent = tangent
  // 如果是可自由移动的标线，需要更新坡道的法向量
  if (freeMoveInRoad) {
    let _yaw = roadSignData.yaw
    if (needUpdateYaw) {
      _yaw = getValidDegreeInRange(_yaw + 180)
    }
    roadSignData.projectNormal = normal
    // 可自由移动的标线，需要根据yaw角调整朝向
    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: refLinePoint,
      tangent,
      point: targetPoint,
      yaw: _yaw,
      useElevation: true,
    })
    roadSignData.yaw = _yaw
    roadSignData.lookAtPoint = lookAtPoint
  } else {
    // 手动控制路标看向的点为沿着道路前方的点
    const frontPoint = getAlongRoadFrontPoint({
      tangent,
      point: targetPoint,
      isForward: t <= 0,
    })
    roadSignData.lookAtPoint = frontPoint
  }

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 更新路面标识的角度
export function rotateRoadSign (params: {
  objectId: string
  yaw?: number
  angle?: number
}) {
  const { objectId, yaw, angle } = params
  const roadSignData = getObject(objectId)
  if (!roadSignData) return null
  const { position } = roadSignData

  if (yaw !== undefined) {
    // 针对道路上可更新偏航角的路面标识
    const { closestPointTangent, closestPoint } = roadSignData
    if (!closestPointTangent || !closestPoint) return null

    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: closestPoint,
      tangent: closestPointTangent,
      point: position,
      yaw,
      useElevation: true,
    })
    roadSignData.yaw = yaw
    roadSignData.lookAtPoint = lookAtPoint
  }

  if (angle !== undefined) {
    // 针对于路口中的物体，基于全局角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: position,
      angle,
    })
    roadSignData.angle = angle
    roadSignData.lookAtPoint = lookAtPoint
  }

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

/**
 * 获取目标 section 中指定 id 的车道左右两侧边界线全量采样点
 */
function getAllLaneBoundaryPoints (params: {
  section: biz.ISection
  laneId: string
}) {
  const { section, laneId } = params
  const lane = getLaneById(section.lanes, laneId)
  if (!lane) return null
  const { lbid, rbid } = lane
  const lb = getBoundaryById(section.boundarys, lbid)
  const rb = getBoundaryById(section.boundarys, rbid)
  if (!lb || !rb) return
  return {
    left: lb.samplePoints,
    right: rb.samplePoints,
  }
}

/**
 * 获取目标 section 中指定 id 的车道左右两侧边界线部分采样点
 * @param params
 */
function getPartLaneBoundaryPoints (params: {
  section: biz.ISection
  laneId: string
  startPercent: number
  endPercent: number
}) {
  const { section, laneId, startPercent, endPercent } = params
  const { pStart, pEnd } = section
  const lane = getLaneById(section.lanes, laneId)
  if (!lane) return null
  const { lbid, rbid } = lane
  const lb = getBoundaryById(section.boundarys, lbid)
  const rb = getBoundaryById(section.boundarys, rbid)
  if (!lb || !rb) return

  // 区间内的采样点集合
  const lPoints = []
  const rPoints = []

  // 当前 section 比例的跨度
  const percentSpan = pEnd - pStart
  // 按照比例区间来定义分段数
  const segment = Math.ceil(
    (lb.samplePoints.length * (endPercent - startPercent)) / percentSpan,
  )

  // 由于 startPercent 和 endPercent 是基于整条道路参考线的比例
  // 该方法基于车道边界线采样点重新创建一条路径，那么起始和截至对应的比例也需要重新调整
  const _startPercent = (startPercent - pStart) / percentSpan
  const _endPercent = (endPercent - pStart) / percentSpan

  // 基于完整的车道边界线采样点，创建路径重新采样
  let leftPath: biz.ICurve3 | null = null
  leftPath = new CatmullRomCurve3(transformVec3ByObject(lb.samplePoints))
  leftPath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    leftPath.tension = CatmullromTension
  }
  // 获取边界线采样点形成的路径上，区间 [startPercent, endPercent] 范围中的采样点
  const leftPartPoints = getParallelCurveSamplePoints({
    keyPath: leftPath,
    offset: 0,
    segment,
    pStart: _startPercent,
    pEnd: _endPercent,
  })
  lPoints.push(...leftPartPoints.refPoints)

  // 右侧边界线的逻辑同上
  let rightPath: biz.ICurve3 | null = null
  rightPath = new CatmullRomCurve3(transformVec3ByObject(rb.samplePoints))
  rightPath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    rightPath.tension = CatmullromTension
  }
  const rightPartPoints = getParallelCurveSamplePoints({
    keyPath: rightPath,
    offset: 0,
    segment,
    pStart: _startPercent,
    pEnd: _endPercent,
  })
  rPoints.push(...rightPartPoints.refPoints)

  // 手动释放
  leftPath = null
  rightPath = null

  return {
    left: lPoints,
    right: rPoints,
  }
}

/**
 * 计算跟车道相关的系列属性
 * @param params
 */
export function calcLaneAttr (params: {
  roadId: string
  sectionId: string
  laneId: string
  length: number
  percent: number
}) {
  const { roadId, sectionId, laneId, length, percent } = params
  const roadStore = useRoadStore()
  const queryLaneRes = roadStore.getLaneById(roadId, sectionId, laneId)
  if (!queryLaneRes) return null

  const { road, section: targetSection, lane: targetLane } = queryLaneRes
  // 遍历当前 section 中的车道宽度，获取对应的 t 坐标
  let newT = 0
  targetSection.lanes.forEach((lane) => {
    if (Math.abs(Number(lane.id)) < Math.abs(Number(laneId))) {
      // 内侧车道
      newT += lane.normalWidth
    } else if (lane.id === laneId) {
      // 目标车道
      newT += lane.normalWidth / 2
    }
  })

  // 导向车道线的长度，可能需要根据比例来调整最后的长度
  let newLength = length
  // 计算目标长度，对应的 percent 比例范围
  let startPercent = 0
  let endPercent = 1
  const percentRange = length / road.length
  if (percentRange >= 1) {
    // 如果车道线长度完全超出当前道路的长度，则范围为 1，需要重新调整 length 长度
    newLength = fixedPrecision(road.length)
  } else {
    // 如果是在道路长度区间内
    if (percent >= 0.5) {
      // 如果目标点的比例处于后半段
      if (percent + percentRange / 2 > 1) {
        endPercent = 1
        startPercent = endPercent - percentRange
      } else {
        startPercent = percent - percentRange / 2
        endPercent = percent + percentRange / 2
      }
    } else {
      // 如果目标点的比例处于前半段
      if (percent - percentRange / 2 < 0) {
        startPercent = 0
        endPercent = startPercent + percentRange
      } else {
        startPercent = percent - percentRange / 2
        endPercent = percent + percentRange / 2
      }
    }
  }
  const newPercent = (startPercent + endPercent) / 2
  const newS = fixedPrecision(road.length * newPercent)

  // 左右边界线的采样点集合（可能来自于多个 section 的多条车道边界线）
  const leftBoundaryPoints: Array<common.vec3> = []
  const rightBoundaryPoints: Array<common.vec3> = []
  for (const section of road.sections) {
    const { pStart, pEnd } = section
    // 判断section是否在导向车道线跨度范围中
    if (pEnd < startPercent || pStart > endPercent) continue

    if (pStart <= startPercent && pEnd < endPercent) {
      // 导向车道线的【前侧】有一部分在section中
      const res = getPartLaneBoundaryPoints({
        section,
        laneId,
        startPercent,
        endPercent: pEnd,
      })
      if (!res) continue
      // 将边界线采样点添加到缓存数组中
      leftBoundaryPoints.push(...res.left)
      rightBoundaryPoints.push(...res.right)
      continue
    }

    if (pStart > startPercent && pEnd >= endPercent) {
      // 导向车道线的【后侧】在一部分的section中
      const res = getPartLaneBoundaryPoints({
        section,
        laneId,
        startPercent: pStart,
        endPercent,
      })
      if (!res) continue
      leftBoundaryPoints.push(...res.left)
      rightBoundaryPoints.push(...res.right)
      continue
    }

    if (pStart > startPercent && pEnd < endPercent) {
      // 导向车道线【完全跨域】了一整个section
      const res = getAllLaneBoundaryPoints({
        section,
        laneId,
      })
      if (!res) continue
      leftBoundaryPoints.push(...res.left)
      rightBoundaryPoints.push(...res.right)
      continue
    }

    if (pStart <= startPercent && pEnd >= endPercent) {
      // 导向车道线【完全包含】在某一个section区间内
      const res = getPartLaneBoundaryPoints({
        section,
        laneId,
        startPercent,
        endPercent,
      })
      if (!res) continue
      leftBoundaryPoints.push(...res.left)
      rightBoundaryPoints.push(...res.right)
      continue
    }
  }

  return {
    leftPoints: leftBoundaryPoints,
    rightPoints: rightBoundaryPoints,
    length: newLength,
    width: targetLane.normalWidth, // 车道的宽度即为导向车道线的宽度
    s: newS,
    // 在正向车道，t 值始终未负值
    t: -newT,
    percent: newPercent,
  }
}
