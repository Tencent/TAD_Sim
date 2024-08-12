import type {
  Object3D,
} from 'three'
import {
  Color,
  CylinderGeometry,
  Group,
  Mesh,
  MeshBasicMaterial,
  MeshLambertMaterial,
  SphereGeometry,
  Vector3,
} from 'three'
import { getPoleConfig } from '@/config/pole'
import { Constant, RenderOrder } from '@/utils/business'
import { PI, halfPI } from '@/utils/common3d'

interface IRenderPole {
  data: biz.IPole
  parent: Group
}

const poleColor = {
  normal: 0xD8D8D8,
  hovered: 0xF1F1F1,
  selected: 0xD9D9D9,
}

/**
 * 重置所有杆的高亮交互效果
 * @param parent
 */
export function resetAllPoleColor (parent: Object3D) {
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      (child.name === 'verticalPole' ||
      child.name === 'horizontalPole' ||
      child.name === 'verticalPoleTop')
    ) {
      child.material.color.set(poleColor.normal)
      child.material.needsUpdate = true
    }
  })
}

/**
 * 通过 id  更新杆的高亮交互效果
 * @param params
 */
export function updatePoleColorById (params: {
  objectId: string
  parent: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { objectId, parent, status, isForced = false } = params
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.objectId === objectId &&
      (child.name === 'verticalPole' ||
      child.name === 'horizontalPole' ||
      child.name === 'verticalPoleTop')
    ) {
      if (
        isForced ||
        !new Color(poleColor.selected).equals(child.material.color)
      ) {
        child.material.color.set(poleColor[status])
        child.material.needsUpdate = true
      }
    }
  })
}

/**
 * 更新杆的高亮交互效果
 * @param params
 */
export function updatePoleColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object, status, isForced = false } = params
  if (!object) return
  let parentGroup: Object3D | null = object
  if (object instanceof Mesh) {
    // 如果是网格，先找父级的容器
    parentGroup = object.parent
  }
  if (!parentGroup) return

  // 再从容器中找杆实体部分的元素
  parentGroup.traverse((child) => {
    if (
      child instanceof Mesh &&
      (child.name === 'verticalPole' ||
      child.name === 'horizontalPole' ||
      child.name === 'verticalPoleTop')
    ) {
      if (
        isForced ||
        !new Color(poleColor.selected).equals(child.material.color)
      ) {
        child.material.color.set(poleColor[status])
        child.material.needsUpdate = true
      }
    }
  })
}

/**
 * 创建竖杆（可作为容器）
 */
export function createVerticalPole (params: {
  name: string
  objectId: string
  roadId?: string
  junctionId?: string
}) {
  const { name, objectId, roadId = '', junctionId = '' } = params
  const poleContainer = new Group()
  poleContainer.name = 'verticalPoleGroup'
  poleContainer.objectId = objectId
  poleContainer.roadId = roadId
  poleContainer.junctionId = junctionId

  const poleConfig = getPoleConfig(name)
  if (!poleConfig) return

  const { vRadius, height, segment } = poleConfig.style

  // 顶部半圆球
  const topGeo = new SphereGeometry(
    vRadius,
    segment,
    segment,
    0,
    PI * 2,
    0,
    halfPI,
  )

  // 杆体圆柱
  const poleGeo = new CylinderGeometry(
    vRadius,
    vRadius,
    height,
    segment,
    1,
    true,
  )

  // 柱体虚拟放置的交互区域
  const poleVirtualGeo = new CylinderGeometry(
    vRadius * Constant.polePlacementAreaRadiusRatio,
    vRadius * Constant.polePlacementAreaRadiusRatio,
    height,
    segment,
  )

  const poleMat = new MeshLambertMaterial({
    color: poleColor.normal,
  })
  const virtualPartMat = new MeshBasicMaterial({
    color: 0xFFFFFF,
    depthTest: false,
    transparent: true,
    opacity: 0,
  })
  const poleMesh = new Mesh(poleGeo, poleMat)
  poleMesh.position.set(0, height / 2, 0)
  poleMesh.name = 'verticalPole'
  // 杆容器中的每一个网格对象，单独设置 RenderOrder，保证鼠标射线检测到的元素按照 RenderOrder 排序结果正确
  poleMesh.renderOrder = RenderOrder.pole
  poleMesh.objectId = objectId
  poleMesh.roadId = roadId
  poleMesh.junctionId = junctionId
  poleContainer.add(poleMesh)

  const verticalVirtualMesh = new Mesh(poleVirtualGeo, virtualPartMat)
  verticalVirtualMesh.position.set(0, height / 2, 0)
  verticalVirtualMesh.name = 'verticalPolePlacementArea'
  verticalVirtualMesh.renderOrder = RenderOrder.pole
  verticalVirtualMesh.objectId = objectId
  verticalVirtualMesh.roadId = roadId
  verticalVirtualMesh.junctionId = junctionId
  poleContainer.add(verticalVirtualMesh)

  const topMesh = new Mesh(topGeo, poleMat)
  topMesh.position.set(0, height - 0.01, 0)
  topMesh.name = 'verticalPoleTop'
  topMesh.renderOrder = RenderOrder.pole
  // 顶部半圆球也响应交互
  topMesh.objectId = objectId
  topMesh.roadId = roadId
  topMesh.junctionId = junctionId
  poleContainer.add(topMesh)

  return poleContainer
}

/**
 * 创建横杆（可作为容器）
 */
export function createHorizontalPole (params: {
  name: string
  objectId: string
  roadId?: string
  junctionId?: string
}) {
  const { name, objectId, roadId = '', junctionId = '' } = params
  const poleContainer = new Group()
  poleContainer.name = 'horizontalPoleGroup'
  poleContainer.objectId = objectId
  poleContainer.roadId = roadId
  poleContainer.junctionId = junctionId

  const poleConfig = getPoleConfig(name)
  if (!poleConfig) return

  const { vRadius, hRadius, height, length, segment } = poleConfig.style
  if (!hRadius || !length) return
  // 虚拟放置区域相对于杆实体的半径比例
  const ratio = Constant.polePlacementAreaRadiusRatio

  // 顶部半圆球
  const topGeo = new SphereGeometry(
    vRadius,
    segment,
    segment,
    0,
    PI * 2,
    0,
    halfPI,
  )

  // 杆体圆柱
  const poleGeo = new CylinderGeometry(
    vRadius,
    vRadius,
    height,
    segment,
    1,
    true,
  )

  // 竖直部分柱体虚拟放置的交互区域
  const vVirtualGeo = new CylinderGeometry(
    vRadius * ratio,
    vRadius * ratio,
    height,
    segment,
  )

  // 水平延伸的部分
  const horizontalGeo = new CylinderGeometry(hRadius, hRadius, length, segment)
  // 水平延伸柱体的虚拟放置交互区域
  const hVirtualGeo = new CylinderGeometry(
    hRadius * ratio,
    hRadius * ratio,
    length,
    segment,
  )

  const poleMat = new MeshLambertMaterial({
    color: poleColor.normal,
  })
  const virtualPartMat = new MeshBasicMaterial({
    color: 0xFFFFFF,
    depthTest: false,
    transparent: true,
    opacity: 0,
  })

  // 顶部半圆球（不作为交互的区域）
  const topMesh = new Mesh(topGeo, poleMat)
  topMesh.position.set(0, height - 0.01, 0)
  topMesh.name = 'horizontalPoleTop'
  topMesh.renderOrder = RenderOrder.pole
  // 顶部半圆球也响应交互
  topMesh.objectId = objectId
  topMesh.roadId = roadId
  topMesh.junctionId = junctionId
  poleContainer.add(topMesh)

  // 竖直部分的杆体
  const verticalMesh = new Mesh(poleGeo, poleMat)
  verticalMesh.position.set(0, height / 2, 0)
  verticalMesh.name = 'verticalPole'
  verticalMesh.renderOrder = RenderOrder.pole
  verticalMesh.objectId = objectId
  verticalMesh.roadId = roadId
  verticalMesh.junctionId = junctionId
  poleContainer.add(verticalMesh)

  // 竖直部分的虚拟放置区域
  const vVirtualMesh = new Mesh(vVirtualGeo, virtualPartMat)
  vVirtualMesh.position.set(0, height / 2, 0)
  vVirtualMesh.name = 'verticalPolePlacementArea'
  vVirtualMesh.renderOrder = RenderOrder.pole
  vVirtualMesh.objectId = objectId
  vVirtualMesh.roadId = roadId
  vVirtualMesh.junctionId = junctionId
  poleContainer.add(vVirtualMesh)

  // 水平延伸部分的杆体
  const horizontalMesh = new Mesh(horizontalGeo, poleMat)
  // TODO 这个角度后续可能需要调整
  horizontalMesh
    .translateY(height - hRadius)
    .translateZ(length / 2 + vRadius)
    .rotateX(halfPI)
  horizontalMesh.name = 'horizontalPole'
  horizontalMesh.renderOrder = RenderOrder.pole
  horizontalMesh.objectId = objectId
  horizontalMesh.roadId = roadId
  horizontalMesh.junctionId = junctionId
  poleContainer.add(horizontalMesh)

  const hVirtualMesh = new Mesh(hVirtualGeo, virtualPartMat)
  hVirtualMesh
    .translateY(height - hRadius)
    .translateZ(length / 2 + vRadius)
    .rotateX(halfPI)
  hVirtualMesh.name = 'horizontalPolePlacementArea'
  hVirtualMesh.renderOrder = RenderOrder.pole
  hVirtualMesh.objectId = objectId
  hVirtualMesh.roadId = roadId
  hVirtualMesh.junctionId = junctionId
  poleContainer.add(hVirtualMesh)

  return poleContainer
}

/**
 * 创建杆并渲染到场景中
 * @param params
 */
export function createPole (params: IRenderPole) {
  const { data: poleData, parent } = params
  const {
    id: objectId,
    name,
    position,
    roadId,
    junctionId,
    lookAtPoint,
  } = poleData
  // pole 是 group 包 mesh 的组合体
  let pole
  if (name === 'Cross_Pole' || name === 'Cantilever_Pole') {
    // 横杆
    pole = createHorizontalPole({
      name,
      objectId,
      roadId,
      junctionId,
    })
  } else {
    // 竖杆
    pole = createVerticalPole({
      name,
      objectId,
      roadId,
      junctionId,
    })
  }
  if (!pole) return

  pole.position.set(position.x, position.y, position.z)
  // 针对放置于道路上的指向调整
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    pole.lookAt(_lookAtPoint)
  } else {
    // 没有指向，则默认朝水平向右的方向（调整角度基于 xyz 笛卡尔坐标系）
  }

  parent.add(pole)
}

/**
 * 移动杆的位置
 * @param params
 */
export function movePole (params: IRenderPole) {
  const { data, parent } = params
  const { id: poleId, roadId = '', position, lookAtPoint } = data
  // 从容器中找对应的杆
  let targetPoleGroup: Group
  parent.traverse((child) => {
    if (targetPoleGroup) return
    if (
      child instanceof Group &&
      child.objectId === poleId &&
      (child.name === 'horizontalPoleGroup' ||
      child.name === 'verticalPoleGroup')
    ) {
      targetPoleGroup = child
    }
  })

  if (!targetPoleGroup) return

  // 道路和路口中的杆位置更新
  targetPoleGroup.position.set(position.x, position.y, position.z)

  if (roadId) {
    // 针对放置于道路上的指向调整
    if (lookAtPoint) {
      const _lookAtPoint = new Vector3(
        lookAtPoint.x,
        lookAtPoint.y,
        lookAtPoint.z,
      )
      targetPoleGroup.lookAt(_lookAtPoint)
    }
  }
}

// 按照一定的角度旋转杆
export function rotatePole (params: IRenderPole) {
  const { data: poleData, parent } = params
  const { id: poleId, lookAtPoint } = poleData
  // 从容器中找对应的杆
  let targetPoleGroup: Group
  parent.traverse((child) => {
    if (targetPoleGroup) return
    if (
      child instanceof Group &&
      child.objectId === poleId &&
      (child.name === 'horizontalPoleGroup' ||
      child.name === 'verticalPoleGroup')
    ) {
      targetPoleGroup = child
    }
  })

  if (!targetPoleGroup) return

  // 道路上和路口中，都是通过调整物体看向的目标点实现角度的旋转
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    targetPoleGroup.lookAt(_lookAtPoint)
  }
}
