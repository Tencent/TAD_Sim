// 信控配置编辑模式下辅助元素的渲染
import type {
  Group,
} from 'three'
import {
  Box3,
  BufferAttribute,
  BufferGeometry,
  Color,
  DynamicDrawUsage,
  Mesh,
  MeshBasicMaterial,
  Sprite,
  Vector3,
} from 'three'
import { RenderOrder, getGeometryAttrBySamplePoints } from '@/utils/business'
import { disposeMaterial } from '@/utils/common3d'
import { disposeOneObject } from '@/main3d/render/object'
import { createTrafficLight } from '@/main3d/render/trafficLight'
import { getJunction } from '@/utils/mapCache'
import root3d from '@/main3d/index'
import { createTextSprite } from '@/main3d/render/text'

export const roadMaskOpacity = 0.2
export const junctionMaskOpacity = 0.3

export const roadMaskColor = {
  normal: 0xFFFFFF,
  hovered: 0xFFFFFF,
  selected: 0xFFFF00,
}
export const junctionMaskColor = {
  normal: 0xFFFFFF,
  hovered: 0xFFFFFF,
  selected: 0xFFFF00,
}

const roadMaskMat = new MeshBasicMaterial({
  color: roadMaskColor.normal,
  depthTest: false,
  transparent: true,
  opacity: 0,
})
const junctionMaskMat = new MeshBasicMaterial({
  color: junctionMaskColor.normal,
  depthTest: false,
  transparent: true,
  opacity: 0,
})

// 渲染层更新三维场景中的信号灯的灯态类型
export async function updateTrafficLightType (params: {
  data: biz.ITrafficLight
  parent: Group
}) {
  const { data: lightData, parent } = params

  // 将原来的信号灯模型，从杆容器中销毁
  disposeOneObject({
    objectId: lightData.id,
    parent,
  })

  // 重新创建一个信号灯
  await createTrafficLight({
    data: lightData,
    parent,
  })
}

// 在道路跟路口连接处，渲染文字提示辅助精灵
export function createTextSpriteInRoad (params: {
  content: string
  position: common.vec3
  parent: Group
}) {
  const { position, content, parent } = params
  const textSprite = createTextSprite({
    content,
    scale: 2,
  })
  if (!textSprite) return
  textSprite.position.set(position.x, position.y, position.z)
  parent.add(textSprite)
}

// 在路口中，渲染文字提示辅助精灵
export function createTextSpriteInJunction (params: {
  junctionId: string
  content: string
  parent: Group
}) {
  const { junctionId, content, parent } = params
  const junction = getJunction(junctionId)
  if (!junction) return

  const mapContainer = root3d.mapElementsContainer
  let junctionMesh: Mesh
  // 从地图元素容器中，找对应的路口
  mapContainer.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'junction' &&
      child.junctionId === junctionId
    ) {
      junctionMesh = child
    }
  })

  // @ts-expect-error
  if (!junctionMesh) return

  // 计算路口几何体的包围盒
  junctionMesh.geometry.computeBoundingBox()
  if (!junctionMesh.geometry.boundingBox) return

  const box = new Box3()
  box
    .copy(junctionMesh.geometry.boundingBox)
    .applyMatrix4(junctionMesh.matrixWorld)
  // 获取包围盒的中心点
  const junctionCenter = new Vector3()
  box.getCenter(junctionCenter)

  // 创建路口的文字精灵
  const textSprite = createTextSprite({
    content,
    scale: 2,
  })
  if (!textSprite) return
  textSprite.position.set(junctionCenter.x, junctionCenter.y, junctionCenter.z)
  // 将文字精灵加到当前编辑模式的容器中
  parent.add(textSprite)
}

// 销毁所有的文字精灵
export function disposeAllTextSprite (parent: Group) {
  const matchChildren: Array<Sprite> = []
  parent.traverse((child) => {
    if (child instanceof Sprite && child.name === 'textSprite') {
      child.geometry.dispose()
      child.material.dispose()
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}

// 重置所有道路或路口蒙层网格的颜色
export function resetAllMaskColor (params: { parent: Group, type?: string }) {
  const { parent, type = '' } = params
  // 如果没有指定，则重做所有蒙层的颜色
  if (!type || type === 'road') {
    parent.traverse((child) => {
      if (child instanceof Mesh && child.name === 'roadMask') {
        const { material } = child
        if (
          !new Color(roadMaskColor.normal).equals(material.color) ||
          material.opacity !== 0
        ) {
          material.color.set(roadMaskColor.normal)
          // 常规状态为完全透明
          material.opacity = 0
          material.needsUpdate = true
        }
      }
    })
  }

  if (!type || type === 'junction') {
    parent.traverse((child) => {
      if (child instanceof Mesh && child.name === 'junctionMask') {
        const { material } = child
        if (
          !new Color(junctionMaskColor.normal).equals(material.color) ||
          material.opacity !== 0
        ) {
          material.color.set(junctionMaskColor.normal)
          // 常规状态为完全透明
          material.opacity = 0
          material.needsUpdate = true
        }
      }
    })
  }
}

// 更新道路或路口蒙层网格的颜色
export function updateMaskColor (params: {
  parent: Group
  status: common.objectStatus
  roadId?: string | Array<string>
  junctionId?: string
}) {
  const { junctionId = '', parent, status } = params
  let { roadId = [] } = params
  if (!Array.isArray(roadId)) {
    roadId = [roadId]
  }
  if (!junctionId && roadId.length < 1) return

  if (roadId.length > 0) {
    parent.traverse((child) => {
      if (
        child instanceof Mesh &&
        child.name === 'roadMask' &&
        roadId.includes(child.roadId)
      ) {
        child.material.color.set(roadMaskColor[status])
        if (status === 'normal') {
          // 常规状态时，为透明不显示效果
          child.material.opacity = 0
        } else {
          // hovered 和 selected 状态，为一定透明度的蒙层
          child.material.opacity = roadMaskOpacity
        }
        child.material.needsUpdate = true
      }
    })
  } else if (junctionId) {
    parent.traverse((child) => {
      if (
        child instanceof Mesh &&
        child.name === 'junctionMask' &&
        child.junctionId === junctionId
      ) {
        child.material.color.set(junctionMaskColor[status])
        if (status === 'normal') {
          // 常规状态时，为透明不显示效果
          child.material.opacity = 0
        } else {
          // hovered 和 selected 状态，为一定透明度的蒙层
          child.material.opacity = junctionMaskOpacity
        }
        child.material.needsUpdate = true
      }
    })
  }
}

/**
 * 渲染道路上方辅助交互的蒙层网格
 * @param params
 */
export function renderRoadMask (params: {
  roadId: string
  leftPoints: Array<common.vec3>
  rightPoints: Array<common.vec3>
  parent: Group
}) {
  const { roadId, leftPoints, rightPoints, parent } = params
  const { vertices, indices } = getGeometryAttrBySamplePoints(
    leftPoints,
    rightPoints,
  )
  const _vertices =
    vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
  const _indices =
    indices instanceof Float32Array ?
      Array.prototype.slice.call(indices) :
      indices
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(_indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  const mesh = new Mesh(geo, roadMaskMat.clone())
  mesh.name = 'roadMask'
  mesh.roadId = roadId
  mesh.renderOrder = RenderOrder.roadMask
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

/**
 * 渲染路口上方辅助交互的蒙层网格
 * @param params
 */
export function renderJunctionMask (params: {
  junctionId: string
  vertices: Array<number>
  indices: Array<number>
  parent: Group
}) {
  const { junctionId, vertices, indices, parent } = params
  const geo = new BufferGeometry()
  const _vertices =
    vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
  const _indices =
    indices instanceof Float32Array ?
      Array.prototype.slice.call(indices) :
      indices

  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(_indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()
  const mesh = new Mesh(geo, junctionMaskMat.clone())
  mesh.name = 'junctionMask'
  mesh.junctionId = junctionId
  mesh.renderOrder = RenderOrder.junctionMask
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

/**
 * 销毁道路辅助交互蒙层元素
 * @param parent
 */
export function disposeRoadMask (parent: Group) {
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'roadMask') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 销毁路口辅助交互蒙层元素
 * @param parent
 */
export function disposeJunctionMask (parent: Group) {
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'junctionMask') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}
