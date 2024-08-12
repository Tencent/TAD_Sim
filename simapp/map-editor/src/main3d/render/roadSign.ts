import type {
  Object3D,
} from 'three'
import {
  Box3,
  Color,
  DoubleSide,
  Group,
  Mesh,
  MeshBasicMaterial,
  PlaneGeometry,
  Vector3,
} from 'three'
import { loadTexture } from '../loaders'
import { createNonMotorVehicleArea } from './customRoadSign/nonMotorVehicleArea'
import { createCrosswalkLine } from './customRoadSign/crosswalk'
import { createStopLine } from './customRoadSign/stopLine'
import { createCustomRoadSignByGeoAttrs } from './customRoadSign/index'
import { createCrosswalkWithSideLine } from './customRoadSign/crosswalkWithSide'
import { getRoadSignConfig } from '@/config/roadSign'
import { RenderOrder } from '@/utils/business'
import { PI, axisX, axisY, fixedPrecision, halfPI } from '@/utils/common3d'

// 常规路面标识的颜色
export const roadSignColor = {
  normal: 0xE8E8E8,
  hovered: 0xFFFFFF,
  selected: 0xE9E9E9,
}
// 黄色的自定义路面标识各交互状态颜色
export const yellowRoadSignColor = {
  normal: 0xE8E800,
  hovered: 0xFFFF00,
  selected: 0xE9E900,
}

export interface IRenderRoadSign {
  data: biz.IRoadSign
  parent: Group
}

// 没有贴图的自定义路面标线材质
export const customRoadSignMaterial = new MeshBasicMaterial({
  color: roadSignColor.normal,
  depthTest: false,
  side: DoubleSide,
})
// 占位区域的透明材质
export const transparentMaterial = new MeshBasicMaterial({
  color: roadSignColor.normal,
  transparent: true,
  opacity: 0,
  alphaTest: 0.1,
})

/**
 * 更新一个路面标线容器内所有网格的颜色（路面标线容器中，不一定只有一个网格！）
 * @param params
 */
function _updateRoadSignColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object: objectGroup, status, isForced = false } = params
  let colorMap = roadSignColor
  // 如果是没有贴图手动创建的带有颜色路面标线
  if (objectGroup.customColor && objectGroup.customColor === 'yellow') {
    colorMap = yellowRoadSignColor
  }
  // 先遍历第一遍，判断是否需要更新
  let needUpdate = false
  if (isForced) {
    needUpdate = true
  } else {
    objectGroup.traverse((child) => {
      if (needUpdate) return

      if (child instanceof Mesh) {
        if (!new Color(colorMap.selected).equals(child.material.color)) {
          needUpdate = true
        }
      }
    })
  }

  // 如果不需要更新，则直接返回
  if (!needUpdate) return

  objectGroup.traverse((child) => {
    if (child instanceof Mesh) {
      child.material.color.set(colorMap[status])
      child.material.needsUpdate = true
    }
  })
}

/**
 * 重置所有路面标线的默认颜色
 * @param parent
 */
export function resetAllRoadSignColor (parent: Object3D) {
  parent.traverse((child) => {
    if (child instanceof Group && child.name === 'roadSignGroup') {
      _updateRoadSignColor({
        object: child,
        status: 'normal',
        isForced: true,
      })
    }
  })
}

/**
 * 通过元素的 id 更新路面标线的颜色
 * @param params
 */
export function updateRoadSignColorById (params: {
  objectId: string
  parent: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { parent, objectId, status, isForced = false } = params
  parent.traverse((child) => {
    if (
      child instanceof Group &&
      child.name === 'roadSignGroup' &&
      child.objectId === objectId
    ) {
      _updateRoadSignColor({
        object: child,
        status,
        isForced,
      })
    }
  })
}

/**
 * 更新常规路面标线的颜色
 */
export function updateRoadSignColor (params: {
  object: Object3D // 要更新颜色的网格或容器
  status: common.objectStatus
  isForced?: boolean // 是否要强制更新颜色（忽略已经选中的高亮）
}) {
  const { object, status, isForced = false } = params
  if (!object) return
  let objectGroup = object
  if (object instanceof Mesh && object.parent) {
    objectGroup = object.parent
  }

  if (!objectGroup) return

  _updateRoadSignColor({
    object: objectGroup,
    status,
    isForced,
  })
}

/**
 * 更新路面标识模型的尺寸
 * @param params
 */
export function updateRoadSignSize (params: IRenderRoadSign) {
  const { data: roadSignData, parent } = params
  const { id: objectId, width, length, basicWidth, basicLength } = roadSignData
  // 从 parent 中找到对应的物体
  const targetObject = parent.getObjectByProperty('objectId', objectId)
  if (!targetObject) return

  // 基于模型最初的尺寸来调节 scale 比例
  const scaleZ = length / basicLength
  const scaleX = width / basicWidth

  targetObject.scale.set(scaleX, 1, scaleZ)
}

/**
 * 创建路面标线并渲染到场景中
 * @param params
 */
export async function createRoadSign (params: IRenderRoadSign) {
  const { data: roadSignData, parent } = params
  const {
    id: objectId,
    name,
    position,
    roadId,
    sectionId,
    laneId,
    junctionId,
    lookAtPoint,
    projectNormal,
    size,
    width: currentWidth,
    length: currentLength,
  } = roadSignData

  switch (name) {
    case 'Crosswalk_Line':
      // 人行横道
      return createCrosswalkLine(params)
    case 'Crosswalk_with_Left_and_Right_Side':
      // 行人左右分道的人行横道线
      return createCrosswalkWithSideLine(params)
    case 'Non_Motor_Vehicle_Area':
      // 非机动车禁驶区
      return createNonMotorVehicleArea(params)
    case 'Stop_Line':
      // 停止线
      return createStopLine(params)
    case 'Road_Guide_Lane_Line':
    case 'Variable_Direction_Lane_Line':
    case 'Longitudinal_Deceleration_Marking':
    case 'Lateral_Deceleration_Marking':
    case 'White_Semicircle_Line_Vehicle_Distance_Confirmation':
    case 'Intersection_Guide_Line':
      // 通过 geoAttrs 创建的跟车道关联的特殊路面标线，使用统一的创建方法
      // 包括:导向车道线、可变导向车道线、车行道纵向减速标线、白色半圆状车距确认线、路口导向线
      return createCustomRoadSignByGeoAttrs(params)
    default:
      break
  }

  const roadSignConfig = getRoadSignConfig(name)
  if (!roadSignConfig) return
  const roadSignTexture = await loadTexture(roadSignConfig.textureUrl)
  const geo = new PlaneGeometry(size[0], size[1])
  const mat = new MeshBasicMaterial({
    color: roadSignColor.normal,
    map: roadSignTexture,
    transparent: true,
    opacity: 1,
    alphaTest: 0.1,
  })

  const roadSignGroup = new Group()
  roadSignGroup.name = 'roadSignGroup'
  roadSignGroup.objectId = objectId
  roadSignGroup.roadId = roadId
  roadSignGroup.sectionId = sectionId
  roadSignGroup.laneId = laneId
  roadSignGroup.junctionId = junctionId

  const mesh = new Mesh(geo, mat)
  mesh.rotateOnAxis(axisY, PI)
  mesh.rotateOnAxis(axisX, -halfPI)
  mesh.name = 'roadSign'
  // 路面标识的容器不设置 RenderOrder，只给网格设置 RenderOrder
  mesh.renderOrder = RenderOrder.roadSign
  mesh.objectId = objectId
  mesh.roadId = roadId
  mesh.sectionId = sectionId
  mesh.laneId = laneId
  mesh.junctionId = junctionId
  roadSignGroup.add(mesh)

  const box = new Box3()
  box.expandByObject(roadSignGroup)
  // 路面标识尺寸的获取
  const width = fixedPrecision(box.max.x - box.min.x)
  const height = fixedPrecision(box.max.y - box.min.y)
  const length = fixedPrecision(box.max.z - box.min.z)

  // 如果当前尺寸存在，则应用当前的尺寸
  let scaleZ = 1
  let scaleX = 1
  if (currentWidth && currentLength) {
    scaleZ = currentLength / length
    scaleX = currentWidth / width
  }
  roadSignGroup.scale.set(scaleX, 1, scaleZ)

  roadSignGroup.position.set(position.x, position.y, position.z)

  // 道路上和路口中的路面标识，都使用 lookAtPoint 控制旋转角度
  if (projectNormal) {
    roadSignGroup.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    roadSignGroup.lookAt(_lookAtPoint)
  }

  parent.add(roadSignGroup)

  return {
    width,
    height,
    length,
  }
}

// 更新道路标识（容器+网格）的属性
export function moveRoadSign (params: IRenderRoadSign) {
  const { data: roadSignData, parent } = params
  const {
    id: objectId,
    position,
    roadId,
    lookAtPoint,
    projectNormal,
  } = roadSignData

  // 从 parent 中，找对应的路面标识元素
  let targetRoadSignGroup: Group
  parent.traverse((child) => {
    if (targetRoadSignGroup) return
    if (
      child instanceof Group &&
      child.name === 'roadSignGroup' &&
      child.objectId === objectId
    ) {
      targetRoadSignGroup = child
    }
  })

  if (!targetRoadSignGroup) return

  // 更新外层容器的属性，道路和路口中的路面标识均有效
  targetRoadSignGroup.position.set(position.x, position.y, position.z)

  // 如果是放置在道路上，还需要调整默认的标识朝向
  if (projectNormal) {
    targetRoadSignGroup.up.set(
      projectNormal.x,
      projectNormal.y,
      projectNormal.z,
    )
  }
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    targetRoadSignGroup.lookAt(_lookAtPoint)
  }
}

// 更新路面标识的角度
export function rotateRoadSign (params: IRenderRoadSign) {
  const { data: roadSignData, parent } = params
  const { id: objectId, lookAtPoint } = roadSignData

  // 从 parent 中，找对应的路面标识元素
  let targetRoadSignGroup: Group
  parent.traverse((child) => {
    if (targetRoadSignGroup) return
    if (
      child instanceof Group &&
      child.name === 'roadSignGroup' &&
      child.objectId === objectId
    ) {
      targetRoadSignGroup = child
    }
  })

  if (!targetRoadSignGroup) return

  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    targetRoadSignGroup.lookAt(_lookAtPoint)
  }
}
