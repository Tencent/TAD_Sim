// crg编辑模式下辅助元素的渲染
import type {
  Object3D,
} from 'three'
import {
  BufferAttribute,
  BufferGeometry,
  Color,
  DynamicDrawUsage,
  Mesh,
  MeshBasicMaterial,
} from 'three'
import { RenderOrder, getGeometryAttrBySamplePoints } from '@/utils/business'
import { disposeMaterial } from '@/utils/common3d'

export const roadMaskOpacity = 0.2

export const roadMaskColor = {
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

// 重置所有道路或路口蒙层网格的颜色
export function resetAllRoadMaskColor (parent: Object3D) {
  // 如果没有指定，则重做所有蒙层的颜色
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

// 更新道路蒙层网格的颜色
export function updateRoadMaskColor (params: {
  parent: Object3D
  status: common.objectStatus
  roadId: string | Array<string>
}) {
  const { parent, status } = params
  let { roadId = [] } = params
  if (!Array.isArray(roadId)) {
    roadId = [roadId]
  }
  if (roadId.length < 1) return

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
}

/**
 * 渲染道路上方辅助交互的蒙层网格
 * @param params
 */
export function renderRoadMask (params: {
  roadId: string
  leftPoints: Array<common.vec3>
  rightPoints: Array<common.vec3>
  parent: Object3D
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
 * 销毁道路辅助交互蒙层元素
 * @param parent
 */
export function disposeRoadMask (params: { parent: Object3D, roadId?: string }) {
  const { parent, roadId = '' } = params
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'roadMask') {
      if (!roadId || child.roadId === roadId) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}
