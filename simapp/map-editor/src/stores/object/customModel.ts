// 自定义导入模型的数据层逻辑
import type { Intersection } from 'three'
import {
  createDefaultObjectData,
  getAlongRoadFrontPoint,
  getLocationInRoad,
} from './common'
import type { ICustomModelConfig } from '@/config/custom'
import { genObjectId } from '@/utils/guid'
import { getObject, getRoad, setObject } from '@/utils/mapCache'
import {
  fixedPrecision,
  getLocationByST,
  getLookAtPointByAngle,
  getLookAtPointByYaw,
  getValidDegreeInRange,
} from '@/utils/common3d'
import { useModelCacheStore } from '@/stores/object/modelCache'

// 创建自定义模型
export function createCustomModel (params: {
  option: ICustomModelConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const { name } = option

  const modelCacheStore = useModelCacheStore()
  const configOption = modelCacheStore.getCustomModelConfigByName(name)
  if (!configOption) return null

  const { length, width, height } = configOption

  const modelData: biz.ICustomModel = {
    ...createDefaultObjectData(),
    // 将自定义模型的尺寸同步到状态数据中
    basicWidth: width,
    basicLength: length,
    basicHeight: height,
  }

  modelData.id = genObjectId()
  modelData.mainType = 'customModel'
  // 自定义模型的 name 和 showName 属性为同一个值
  modelData.name = name
  modelData.showName = name

  // 基于鼠标交互结果，更新物体的空间属性
  const { point, object } = intersection
  // @ts-expect-error
  const { roadId = '', junctionId = '' } = object

  // 将位置同步到鼠标射线检测到的位置
  modelData.position = {
    x: point.x,
    y: point.y,
    z: point.z,
  }

  if (roadId) {
    // 如果自定义模型放置在道路上
    modelData.roadId = roadId

    const locationData = getLocationInRoad({
      point,
      roadId,
      useElevation: true,
    })
    if (!locationData) return null
    const { s, t, percent, tangent, closestPointOnRefLine, normal } =
      locationData

    const isForward = t <= 0

    modelData.s = s
    modelData.t = t
    modelData.closestPoint = closestPointOnRefLine
    modelData.closestPointTangent = tangent
    modelData.closestPointPercent = percent
    modelData.projectNormal = normal
    modelData.yaw = isForward ? 0 : 180
    // 默认看向 s 轴切线的方向
    const frontPoint = getAlongRoadFrontPoint({
      tangent,
      point,
      isForward,
    })
    modelData.lookAtPoint = frontPoint
  }
  if (junctionId) {
    modelData.junctionId = junctionId
    // 世界坐标系下的角度默认为 0
    modelData.angle = 0
  }

  setObject(modelData.id, modelData)

  return modelData
}

// 移动自定义模型
export function moveCustomModel (params: {
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
  const modelData = getObject(objectId)
  if (!modelData) return

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
    let yaw = modelData.yaw
    if (
      (modelData.t < 0 && locationData.t > 0) ||
      (modelData.t > 0 && locationData.t < 0)
    ) {
      yaw = getValidDegreeInRange(yaw + 180)
    }

    const { s, t, percent, tangent, closestPointOnRefLine, normal } =
      locationData

    modelData.s = s
    modelData.t = t
    modelData.closestPointTangent = tangent
    modelData.closestPointPercent = percent
    modelData.closestPoint = closestPointOnRefLine
    modelData.projectNormal = normal

    // 基于当前的偏航角，计算朝向
    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: closestPointOnRefLine,
      tangent,
      point: targetPoint,
      yaw,
      useElevation: true,
    })
    modelData.yaw = yaw
    modelData.lookAtPoint = lookAtPoint
  } else {
    // 针对路口中的物体，基于全局的角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle: modelData.angle,
    })
    modelData.lookAtPoint = lookAtPoint
  }

  modelData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }

  setObject(modelData.id, modelData)

  return modelData
}

// 通过 st 坐标移动自定义模型
export function moveCustomModelByST (params: {
  objectId: string
  s: number
  t: number
}) {
  const { objectId, s, t } = params

  const modelData = getObject(objectId)
  if (!modelData) return

  // 通过 st 修改物体的位置，说明 roadId 一定存在
  const { roadId } = modelData
  const roadData = getRoad(roadId)
  if (!roadData) return

  // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
  let yaw = modelData.yaw
  if ((modelData.t < 0 && t > 0) || (modelData.t > 0 && t < 0)) {
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
  modelData.s = newS
  modelData.t = t
  modelData.position = {
    x: targetPoint.x,
    y: targetPoint.y,
    z: targetPoint.z,
  }
  modelData.closestPoint = refLinePoint
  modelData.closestPointPercent = percent
  modelData.closestPointTangent = tangent
  modelData.projectNormal = normal

  // 基于当前的偏航角，计算朝向
  const lookAtPoint = getLookAtPointByYaw({
    pointOnRefLine: refLinePoint,
    tangent,
    point: targetPoint,
    yaw,
    useElevation: true,
  })
  modelData.yaw = yaw
  modelData.lookAtPoint = lookAtPoint

  setObject(modelData.id, modelData)

  return modelData
}

// 旋转自定义模型的角度
export function rotateCustomModel (params: {
  objectId: string
  yaw?: number
  angle?: number
}) {
  const { objectId, yaw, angle } = params
  const modelData = getObject(objectId)
  if (!modelData) return

  const { position } = modelData
  if (yaw !== undefined) {
    // 针对道路上的物体，基于偏航角来更新物体的角度
    const { closestPointTangent, closestPoint } = modelData
    if (!closestPointTangent || !closestPoint) return null

    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: closestPoint,
      tangent: closestPointTangent,
      point: position,
      yaw,
      useElevation: true,
    })

    modelData.yaw = yaw
    modelData.lookAtPoint = lookAtPoint
  }

  if (angle !== undefined) {
    // 针对路口中的物体，基于全局的角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: position,
      angle,
    })
    modelData.angle = angle
    modelData.lookAtPoint = lookAtPoint
  }

  setObject(modelData.id, modelData)

  return modelData
}

// 更新自定义模型的尺寸
export function updateCustomModelSize (params: {
  objectId: string
  length: number
  width: number
  height: number
}) {
  const { objectId, length, width, height } = params
  const modelData = getObject(objectId) as biz.ICustomModel
  if (!modelData) return null

  modelData.length = fixedPrecision(length)
  modelData.width = fixedPrecision(width)
  modelData.height = fixedPrecision(height)

  setObject(modelData.id, modelData)

  return modelData
}
