import type {
  Object3D,
} from 'three'
import {
  BufferAttribute,
  BufferGeometry,
  Color,
  DoubleSide,
  DynamicDrawUsage,
  Group,
  Mesh,
  MeshBasicMaterial,
  Vector2,
  Vector3,
} from 'three'
import { createTextMesh } from './text'
import { useObjectInteractionStore } from '@/plugins/editObject/store/interaction'
import { RenderOrder } from '@/utils/business'
import { DefaultColor, halfPI } from '@/utils/common3d'
import { globalConfig } from '@/utils/preset'

const parkingSpaceOpacity = {
  normal: 0,
  hovered: 0.2,
  selected: 0.2,
}
const parkingSpaceColor = {
  normal: 0xFFFFFF,
  hovered: 0xFFFFFF,
  selected: 0xFFFF00,
}

/**
 * 重置所有停车位的默认颜色
 * @param parent
 */
export function resetAllParkingSpaceColor (parent: Object3D) {
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'parkingSpace' &&
      child.isPlaceHolder === true
    ) {
      // 如果停车位的占位区域不为默认的颜色，则更新颜色
      if (
        !new Color(parkingSpaceColor.normal).equals(child.material.color)
      ) {
        child.material.opacity = parkingSpaceOpacity.normal
        child.material.color.set(parkingSpaceColor.normal)
        child.material.needsUpdate = true
      }
    }
  })
}

/**
 * 通过 id 找到停车位更新交互效果
 * @param params
 */
export function updateParkingSpaceColorById (params: {
  objectId: string
  parent: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { objectId, parent, status, isForced = false } = params
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'parkingSpace' &&
      child.objectId === objectId &&
      child.isPlaceHolder === true
    ) {
      // 从容器中找到停车位占位区域子集
      if (
        isForced ||
        !new Color(parkingSpaceColor.selected).equals(child.material.color)
      ) {
        child.material.opacity = parkingSpaceOpacity[status]
        child.material.color.set(parkingSpaceColor[status])
        child.material.needsUpdate = true
      }
    }
  })
}

/**
 * 更新停车位交互效果
 */
export function updateParkingSpaceColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object, status, isForced = false } = params
  if (!object) return
  if (object instanceof Mesh) {
    // 如果是网格
    // 先判断是否是停车位占位区域
    let _object = object
    if (object.name === 'parkingSpace' && !object.isPlaceHolder) {
      // 如果不是占位区域，需要找到同容器内的相邻占位子集
      if (object.parent) {
        const targetObject = object.parent.getObjectByProperty(
          'isPlaceHolder',
          true,
        )
        if (targetObject) {
          _object = targetObject as Mesh
        }
      }
    }

    // 修改占位区域的透明度和颜色
    if (
      isForced ||
      !new Color(parkingSpaceColor.selected).equals(_object.material.color)
    ) {
      _object.material.opacity = parkingSpaceOpacity[status]
      _object.material.color.set(parkingSpaceColor[status])
      _object.material.needsUpdate = true
    }
  } else if (object instanceof Group) {
    // 如果是容器
    const _object = object.getObjectByProperty('isPlaceHolder', true) as Mesh
    if (_object) {
      // 修改占位区域的透明度和颜色
      const material = _object.material as MeshBasicMaterial
      if (
        isForced ||
        !new Color(parkingSpaceColor.selected).equals(material.color)
      ) {
        material.opacity = parkingSpaceOpacity[status]
        material.color.set(parkingSpaceColor[status])
        material.needsUpdate = true
      }
    }
  }
}

// 基于停车位的4个角坐标，创建用于辅助选取的占位几何体
export function createPlaceHolderGeo (points: Array<common.vec3>) {
  const vertices = []
  const indices = []
  // 按照[左上，右上，右下，左下]的顺序
  const [p1, p2, p3, p4] = points
  // 顶点坐标
  vertices.push(p1.x, p1.y, p1.z)
  vertices.push(p2.x, p2.y, p2.z)
  vertices.push(p3.x, p3.y, p3.z)
  vertices.push(p4.x, p4.y, p4.z)
  // 索引顺序
  indices.push(0, 2, 1)
  indices.push(0, 3, 2)

  const _vertices = new Float32Array(vertices)

  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  return geo
}

// 创建一个完整的停车位网格
export function createParkingSpace (params: {
  data: biz.IParkingSpace
  parent: Group
}) {
  const { data: parkingSpaceData, parent } = params

  const {
    id: objectId,
    color,
    geoAttrs,
    roadId = '',
    cornerPoints,
  } = parkingSpaceData
  if (!geoAttrs || geoAttrs.length < 1) return

  const basicColor = DefaultColor[color].value
  // 停车位边线的材质
  const lineMat = new MeshBasicMaterial({ color: basicColor, side: DoubleSide })
  // 停车位中间占位区域的透明材质
  const transparentMat = new MeshBasicMaterial({
    color: parkingSpaceColor.normal,
    transparent: true,
    opacity: 0,
    alphaTest: 0.1,
  })
  // 由于停车位每一次更新都是销毁旧的网格，重新创建新网格，所以针对停车位的创建，不得不耦合当前选中交互的状态
  const objectInteractionStore = useObjectInteractionStore()
  const { currentParkingSpace } = objectInteractionStore
  if (currentParkingSpace && currentParkingSpace.id === objectId) {
    // 如果当前创建的停车位为选中状态
    transparentMat.opacity = parkingSpaceOpacity.selected
    transparentMat.color.set(parkingSpaceColor.selected)
    transparentMat.needsUpdate = true
  }

  const parkingGroup = new Group()
  parkingGroup.name = 'parkingSpaceGroup'
  // 停车位的容器绑定物体 id，便于查找
  parkingGroup.objectId = objectId
  parkingGroup.roadId = roadId
  parent.add(parkingGroup)

  geoAttrs.forEach((geoAttr, index) => {
    const { vertices, indices } = geoAttr
    const _vertices =
      vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
    const geo = new BufferGeometry()
    const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
      DynamicDrawUsage,
    )
    geo.setIndex(indices)
    geo.setAttribute('position', positionAttr)
    geo.computeVertexNormals()

    // 每一个停车位的4个角点
    const _cornerPoints = cornerPoints[index]

    // 辅助选取的透明占位网格
    const placeholderGeo = createPlaceHolderGeo(_cornerPoints)
    const placeholder = new Mesh(placeholderGeo, transparentMat)
    placeholder.name = 'parkingSpace'
    // 由于 name 属性跟边界线同名，无法区分，新增字段区分中间的透明占位网格
    placeholder.isPlaceHolder = true
    placeholder.objectId = objectId
    placeholder.roadId = roadId
    placeholder.renderOrder = RenderOrder.parkingSpace
    placeholder.matrixAutoUpdate = false
    placeholder.updateMatrix()
    parkingGroup.add(placeholder)

    // 根据项目配置文件，决定是否要显示这个停车位标识
    if (globalConfig.exposeModule.function.showParkingSpaceMark) {
      // 基于四个角点，创建每一个单独停车位的标识
      // 停车位元素的 id，乘以 100，再按照停车位的顺序叠加
      const markId = Number(objectId) * 100 + index
      // 计算世界坐标系每一个停车位的中心点
      const center = new Vector3(0, 0, 0)
      _cornerPoints.forEach((p) => {
        center.x += p.x
        center.y += p.y
        center.z += p.z
      })
      center.multiplyScalar(1 / _cornerPoints.length)
      // 基于前两个点角点来计算停车位延伸的方向
      const p0 = _cornerPoints[0]
      const p1 = _cornerPoints[1]
      const direction = new Vector2(p1.z - p0.z, p1.x - p0.x)
      const angle = direction.angle()
      const textMesh = createTextMesh({
        content: String(markId),
        color: basicColor,
      })
      if (textMesh) {
        textMesh.objectId = objectId
        textMesh.roadId = roadId
        textMesh.renderOrder = RenderOrder.parkingSpace
        textMesh.rotateX(-halfPI)
        textMesh.rotateZ(angle - halfPI)
        textMesh.position.set(center.x, center.y, center.z)
        parkingGroup.add(textMesh)
      }
    }

    // 描边的线网格
    const lineMesh = new Mesh(geo, lineMat)
    lineMesh.name = 'parkingSpace'
    lineMesh.objectId = objectId
    lineMesh.roadId = roadId
    lineMesh.renderOrder = RenderOrder.parkingSpace
    lineMesh.matrixAutoUpdate = false
    lineMesh.updateMatrix()
    parkingGroup.add(lineMesh)
  })
}
