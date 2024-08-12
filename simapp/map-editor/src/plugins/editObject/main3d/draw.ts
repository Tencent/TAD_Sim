// 物体编辑模式下，辅助放置交互区域的渲染逻辑
import type {
  Object3D,
  Quaternion,
} from 'three'
import {
  Box3,
  BufferAttribute,
  BufferGeometry,
  Color,
  DynamicDrawUsage,
  Group,
  Mesh,
  MeshBasicMaterial,
  Sprite,
  Vector3,
} from 'three'
import {
  Constant,
  RenderOrder,
  getGeometryAttrBySamplePoints,
} from '@/utils/business'
import {
  DefaultColor,
  disposeMaterial,
  getLookAtPointByYaw,
  getValidDegreeInRange,
} from '@/utils/common3d'
import root3d from '@/main3d/index'
import { showOriginObject, updateBox3Helper } from '@/main3d/render/object'
import { getValidLookAtPoint } from '@/stores/object/pole'
import { getObject } from '@/utils/mapCache'
import { createTextSprite } from '@/main3d/render/text'
import { customRoadSignMaterial } from '@/main3d/render/roadSign'
import { getCrosswalkGeo } from '@/main3d/render/customRoadSign/crosswalk'
import { createCrosswalkWithSideGroup } from '@/main3d/render/customRoadSign/crosswalkWithSide'

const roadHelperAreaMat = new MeshBasicMaterial({
  color: new Color(0.3, 0.3, 0.3),
  depthTest: false,
})
const junctionHelperAreaMat = new MeshBasicMaterial({
  color: new Color(0.3, 0.3, 0.3),
  depthTest: false,
})

export const junctionMaskColor = {
  normal: 0xFFFFFF,
  hovered: 0xFFFFFF,
  selected: 0xFFFF00,
}
export const junctionMaskOpacity = 0.3
const junctionMaskMat = new MeshBasicMaterial({
  color: junctionMaskColor.normal,
  depthTest: false,
  transparent: true,
  opacity: 0,
})

// 设置所有文字精灵隐藏还是展示
export function setAllTextSpriteVisible (params: {
  visible: boolean
  parent: Group
}) {
  const { visible, parent } = params
  parent.traverse((child) => {
    if (child instanceof Sprite && child.name === 'textSprite') {
      child.visible = visible
    }
  })
}

// 在路口中，渲染文字提示辅助精灵
export function createTextSpriteInJunction (params: {
  junction: biz.IJunction
  content: string
  parent: Group
}) {
  const { junction, content, parent } = params
  const { id: junctionId } = junction

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
  // 默认不展示
  textSprite.visible = false
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
  mesh.renderOrder = RenderOrder.rsuControlJunctionMask
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

// 更新道路或路口蒙层网格的颜色
export function updateJunctionMaskColor (params: {
  parent: Group
  status: common.objectStatus
  junctionId: string | Array<string>
}) {
  const { junctionId = '', parent, status } = params
  if (!junctionId) return
  let _junctionIds: Array<string> = []
  if (Array.isArray(junctionId)) {
    _junctionIds = junctionId
  } else {
    _junctionIds = [junctionId]
  }

  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'junctionMask' &&
      _junctionIds.includes(child.junctionId)
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

/**
 * 销毁路口辅助交互蒙层元素
 * @param parent
 */
export function disposeJunctionMask (params: {
  parent: Object3D
  junctionId?: string
}) {
  const { parent, junctionId } = params
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'junctionMask') {
      if (!junctionId || child.junctionId === junctionId) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 渲染预览辅助交互的停车位
 * @param params
 */
export function renderVirtualParkingSpace (params: {
  objectId: string
  color: common.colorType
  parent: Object3D
  geoAttrs: Array<biz.IGeoAttr>
  roadId?: string
  refLineLocation?: biz.IRefLineLocation
  intersectionPoint?: common.vec3
}) {
  const {
    objectId,
    parent,
    geoAttrs,
    color,
    roadId = '',
    refLineLocation = null,
    intersectionPoint = null,
  } = params
  const parkingSpaceGroup = new Group()
  parkingSpaceGroup.name = 'virtualParkingSpace'
  parkingSpaceGroup.objectId = objectId
  // 当前预览的停车位放置的道路（dragEnd 时用）
  parkingSpaceGroup.roadId = roadId
  // 当前交互点在道路参考线坐标系的坐标属性（dragEnd 时用）
  parkingSpaceGroup.refLineLocation = refLineLocation
  // 当前鼠标跟道路交互区域的交互坐标（dragEnd 时用）
  parkingSpaceGroup.intersectionPoint = intersectionPoint
  parent.add(parkingSpaceGroup)

  const basicColor = DefaultColor[color].value
  const lineMat = new MeshBasicMaterial({
    color: basicColor,
  })

  // 向停车位的容器中添加网格
  geoAttrs.forEach((geoAttr) => {
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

    // 描边的线网格
    const lineMesh = new Mesh(geo, lineMat)
    lineMesh.name = 'parkingSpace'
    lineMesh.objectId = objectId
    lineMesh.renderOrder = RenderOrder.parkingSpace
    lineMesh.matrixAutoUpdate = false
    lineMesh.updateMatrix()
    parkingSpaceGroup.add(lineMesh)
  })
}

/**
 * 渲染预览辅助交互的自定义导入模型物体
 * @param params
 */
export function renderVirtualCustomModel (params: {
  objectId: string
  parent: Group
  position: common.vec3
  roadId?: string
  refLineLocation?: biz.IRefLineLocation
}) {
  const {
    objectId,
    parent,
    position,
    roadId = '',
    refLineLocation = null,
  } = params
  const mapContainer = root3d.mapElementsContainer
  let virtualCustomModel = parent.getObjectByName('virtualCustomModel')
  if (!virtualCustomModel) {
    const originCustomModel = mapContainer.getObjectByProperty(
      'objectId',
      objectId,
    )
    if (!originCustomModel) return
    virtualCustomModel = originCustomModel.clone()
    virtualCustomModel.name = 'virtualCustomModel'
    virtualCustomModel.visible = true
    // 自定义属性需要手动拷贝
    virtualCustomModel.objectId = objectId
    parent.add(virtualCustomModel)
  }

  // 放置到道路和路口中，都需要调整位置属性
  virtualCustomModel.position.set(position.x, position.y, position.z)
  // 由于道路上放置其他物体时，是实时计算放置区域的有效性，有可能在完成放置时，目标点的位置是无效的
  // 此处通过三维虚拟放置的元素缓存上一次有效的位置信息，在完成放置时，从辅助元素取值
  if (roadId) {
    const objectData = getObject(objectId)
    if (refLineLocation && objectData) {
      // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
      let yaw = objectData.yaw
      if (
        (objectData.t < 0 && refLineLocation.t > 0) ||
        (objectData.t > 0 && refLineLocation.t < 0)
      ) {
        yaw = getValidDegreeInRange(yaw + 180)
      }
      const { normal = null } = refLineLocation
      if (normal) {
        // 调整预览放置元素的 up 朝向
        virtualCustomModel.up.set(normal.x, normal.y, normal.z)
      }
      const lookAtPoint = getLookAtPointByYaw({
        pointOnRefLine: refLineLocation.closestPointOnRefLine,
        tangent: refLineLocation.tangent,
        point: position,
        yaw,
        useElevation: true,
      })
      virtualCustomModel.lookAt(lookAtPoint)
    }

    virtualCustomModel.roadId = roadId
    virtualCustomModel.refLineLocation = refLineLocation
  }

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualCustomModel) {
    updateBox3Helper(virtualCustomModel)
  }
}

/**
 * 渲染预览辅助交互的其他类型物体
 * @param params
 */
export function renderVirtualOther (params: {
  objectId: string
  parent: Group
  position: common.vec3
  roadId?: string
  refLineLocation?: biz.IRefLineLocation
}) {
  const {
    objectId,
    parent,
    position,
    roadId = '',
    refLineLocation = null,
  } = params
  const mapContainer = root3d.mapElementsContainer
  let virtualOther = parent.getObjectByName('virtualOther')
  if (!virtualOther) {
    const originOther = mapContainer.getObjectByProperty('objectId', objectId)
    if (!originOther) return
    virtualOther = originOther.clone()
    virtualOther.name = 'virtualOther'
    virtualOther.visible = true
    // 自定义属性需要手动拷贝
    virtualOther.objectId = objectId
    parent.add(virtualOther)
  }

  // 放置到道路和路口中，都需要调整位置属性
  virtualOther.position.set(position.x, position.y, position.z)
  // 由于道路上放置其他物体时，是实时计算放置区域的有效性，有可能在完成放置时，目标点的位置是无效的
  // 此处通过三维虚拟放置的元素缓存上一次有效的位置信息，在完成放置时，从辅助元素取值
  if (roadId) {
    const objectData = getObject(objectId)
    if (refLineLocation && objectData) {
      // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
      let yaw = objectData.yaw
      if (
        (objectData.t < 0 && refLineLocation.t > 0) ||
        (objectData.t > 0 && refLineLocation.t < 0)
      ) {
        yaw = getValidDegreeInRange(yaw + 180)
      }
      const { normal = null } = refLineLocation
      if (normal) {
        // 调整预览放置元素的 up 朝向
        virtualOther.up.set(normal.x, normal.y, normal.z)
      }
      const lookAtPoint = getLookAtPointByYaw({
        pointOnRefLine: refLineLocation.closestPointOnRefLine,
        tangent: refLineLocation.tangent,
        point: position,
        yaw,
        useElevation: true,
      })
      virtualOther.lookAt(lookAtPoint)
    }

    virtualOther.roadId = roadId
    virtualOther.refLineLocation = refLineLocation
  }

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualOther) {
    updateBox3Helper(virtualOther)
  }
}

/**
 * 渲染杆的预览辅助元素
 * @param params
 */
export function renderVirtualPole (params: {
  objectId: string
  parent: Group
  position: common.vec3
  roadId?: string
  refLineLocation?: biz.IRefLineLocation
}) {
  const {
    objectId,
    parent,
    position,
    roadId = '',
    refLineLocation = null,
  } = params
  const mapContainer = root3d.mapElementsContainer
  let virtualPole = parent.getObjectByName('virtualPole')
  if (!virtualPole) {
    const originPole = mapContainer.getObjectByProperty('objectId', objectId)
    if (!originPole) return
    virtualPole = originPole.clone()
    virtualPole.name = 'virtualPole'
    virtualPole.visible = true
    // 自定义属性需要手动拷贝
    virtualPole.objectId = objectId
    parent.add(virtualPole)
  }

  // 放置到道路和路口中，都需要调整位置属性
  virtualPole.position.set(position.x, position.y, position.z)

  // 由于在道路上放置杆时，是实时计算放置区域的有效性，有可能在完成放置时，目标点的位置是无效的
  // 此处通过三维虚拟放置的元素缓存上一次有效的位置信息，在完成放置时，从辅助元素取值
  if (roadId) {
    if (refLineLocation) {
      const { t, tangent } = refLineLocation
      const lookAtPoint = getValidLookAtPoint({
        tangent,
        position,
        t,
      })
      virtualPole.lookAt(lookAtPoint)
    }

    virtualPole.roadId = roadId
    virtualPole.refLineLocation = refLineLocation
  }

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualPole) {
    updateBox3Helper(virtualPole)
  }
}

/**
 * 渲染杆上的预览辅助元素（交通灯+标志牌+传感器）
 * @param params
 */
export function renderVirtualElementOnPole (params: {
  objectId: string
  parent: Group
  position: common.vec3
  quaternion: Quaternion
  name: 'virtualSignalBoard' | 'virtualTrafficLight' | 'virtualSensor'
}) {
  // 在当前编辑模式的容器中创建+添加预览辅助的元素
  const { objectId, parent, position, quaternion, name } = params
  const mapContainer = root3d.mapElementsContainer
  // 从编辑模式的容器中先找预览辅助元素
  let virtualElement = parent.getObjectByName(name)
  if (!virtualElement) {
    // 如果没找到，则需要基于原始地图元素容器中的物体，clone 一份新的
    // 此处找到的为外层的容器
    const originGroup = mapContainer.getObjectByProperty('objectId', objectId)
    if (!originGroup) return

    virtualElement = originGroup.clone()
    virtualElement.name = name
    // 只需要在创建时调整旋转角度
    virtualElement.quaternion.copy(quaternion)
    // 由于原始物体已经隐藏，需要在 clone 时将预览放置的元素设置成可见
    virtualElement.visible = true
    // 自定义属性需要手动拷贝
    virtualElement.objectId = objectId
    parent.add(virtualElement)
  }

  // 每次都需要更新位置属性
  virtualElement.position.set(position.x, position.y, position.z)

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualElement) {
    updateBox3Helper(virtualElement)
  }
}

/**
 * 渲染预览的辅助导向车道线
 * @param params
 */
export function renderVirtualCustomRoadSign (params: {
  objectId: string
  parent: Object3D
  geoAttrs: Array<biz.IGeoAttr>
  roadId: string
  sectionId: string
  laneId: string
  point: common.vec3
}) {
  const {
    objectId,
    parent,
    geoAttrs,
    roadId,
    sectionId,
    laneId,
    point = null,
  } = params
  const group = new Group()
  group.name = 'virtualRoadSign'
  group.objectId = objectId
  // 当前预览的停车位放置的道路（dragEnd 时用）
  group.roadId = roadId
  group.sectionId = sectionId
  group.laneId = laneId
  // 当前鼠标跟道路交互区域的交互坐标（dragEnd 时用）
  group.intersectionPoint = point
  parent.add(group)

  const mat = customRoadSignMaterial.clone()

  // 向容器中添加网格
  geoAttrs.forEach((geoAttr) => {
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

    const mesh = new Mesh(geo, mat)
    mesh.name = 'roadSign'
    mesh.objectId = objectId
    mesh.roadId = roadId
    mesh.sectionId = sectionId
    mesh.laneId = laneId
    mesh.renderOrder = RenderOrder.roadSign
    mesh.matrixAutoUpdate = false
    mesh.updateMatrix()
    group.add(mesh)
  })

  // 拖拽过程中，实时更新包围盒外框的位置
  if (group) {
    updateBox3Helper(group)
  }
}

/**
 * 渲染预览的辅助交互行人左右分道的人行横道（会跟随道路的宽度调整长度）
 * @param params
 */
export function renderVirtualCrosswalkWithSide (params: {
  objectId: string
  parent: Object3D
  point?: common.vec3
  roadId: string
  refLineLocation?: biz.IRefLineLocation
  length: number
}) {
  const {
    objectId,
    parent,
    point,
    roadId = '',
    refLineLocation = null,
    length,
  } = params
  // 如果当前编辑模式中没有对应的预览辅助元素，需要创建到编辑模式的容器中
  const roadSignData = getObject(objectId)
  if (!roadSignData) return

  let virtualRoadSign = parent.getObjectByName('virtualRoadSign')

  if (!virtualRoadSign) {
    // 外层的容器
    virtualRoadSign = new Group()
    virtualRoadSign.name = 'virtualRoadSign'
    virtualRoadSign.objectId = objectId
    virtualRoadSign.roadId = roadId
    parent.add(virtualRoadSign)
  }

  // 先销毁容器内部的网格
  virtualRoadSign.traverse((child) => {
    if (child instanceof Mesh) {
      child.geometry.dispose()
      disposeMaterial(child)
    }
  })
  virtualRoadSign.clear()

  // 在虚拟的容器中，添加辅助预览的网格元素
  createCrosswalkWithSideGroup({
    length,
    // 原来人行横道的宽度
    width: roadSignData.width,
    group: virtualRoadSign,
  })
  virtualRoadSign.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'roadSign'
    }
  })

  // 将创建好的辅助交互预览的人行横道，调整位置和朝向
  // 根据 roadId 是否存在，来判断物体在道路上是自由移动还是限制于车道中
  if (roadId) {
    if (point) {
      // 道路上自由移动的路面标识
      virtualRoadSign.position.set(point.x, point.y, point.z)

      const roadSignData = getObject(objectId)
      if (refLineLocation && roadSignData) {
        const { normal = null } = refLineLocation
        if (normal) {
          virtualRoadSign.up.set(normal.x, normal.y, normal.z)
        }
        const lookAtPoint = getLookAtPointByYaw({
          pointOnRefLine: refLineLocation.closestPointOnRefLine,
          tangent: refLineLocation.tangent,
          point,
          yaw: roadSignData.yaw,
          useElevation: true,
        })
        virtualRoadSign.lookAt(lookAtPoint)
      }

      virtualRoadSign.roadId = roadId
      virtualRoadSign.refLineLocation = refLineLocation
    }
  }

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualRoadSign) {
    updateBox3Helper(virtualRoadSign)
  }
}

/**
 * 渲染预览的辅助交互人行横道（会跟随道路的宽度调整长度）
 * @param params
 */
export function renderVirtualCrosswalk (params: {
  objectId: string
  parent: Object3D
  point?: common.vec3
  roadId: string
  refLineLocation?: biz.IRefLineLocation
  length: number
}) {
  const {
    objectId,
    parent,
    point,
    roadId = '',
    refLineLocation = null,
    length,
  } = params
  // 如果当前编辑模式中没有对应的预览辅助元素，需要创建到编辑模式的容器中
  const roadSignData = getObject(objectId)
  if (!roadSignData) return
  let virtualRoadSign = parent.getObjectByName('virtualRoadSign')

  if (!virtualRoadSign) {
    // 外层的容器
    virtualRoadSign = new Group()
    virtualRoadSign.name = 'virtualRoadSign'
    virtualRoadSign.objectId = objectId
    virtualRoadSign.roadId = roadId
    parent.add(virtualRoadSign)
  }

  // 先销毁容器内部的网格
  virtualRoadSign.traverse((child) => {
    if (child instanceof Mesh) {
      child.geometry.dispose()
      disposeMaterial(child)
      if (child.parent) {
        child.parent.remove(child)
      }
    }
  })

  // 获取原来的人行横道的宽度
  const { width } = roadSignData
  // 创建一个新的辅助人行横道
  const { main: mainGeo } = getCrosswalkGeo({
    length,
    width,
  })
  const mesh = new Mesh(mainGeo, customRoadSignMaterial.clone())
  mesh.name = 'roadSign'
  mesh.renderOrder = RenderOrder.roadSign
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  virtualRoadSign.add(mesh)

  // 将创建好的辅助交互预览的人行横道，调整位置和朝向
  // 根据 roadId 是否存在，来判断物体在道路上是自由移动还是限制于车道中
  if (roadId) {
    if (point) {
      // 道路上自由移动的路面标识
      virtualRoadSign.position.set(point.x, point.y, point.z)

      const roadSignData = getObject(objectId)
      if (refLineLocation && roadSignData) {
        const { normal = null } = refLineLocation
        if (normal) {
          virtualRoadSign.up.set(normal.x, normal.y, normal.z)
        }
        const lookAtPoint = getLookAtPointByYaw({
          pointOnRefLine: refLineLocation.closestPointOnRefLine,
          tangent: refLineLocation.tangent,
          point,
          yaw: roadSignData.yaw,
          useElevation: true,
        })
        virtualRoadSign.lookAt(lookAtPoint)
      }

      virtualRoadSign.roadId = roadId
      virtualRoadSign.refLineLocation = refLineLocation
    }
  }

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualRoadSign) {
    updateBox3Helper(virtualRoadSign)
  }
}

/**
 * 渲染预览的辅助交互停止线（会跟随道路的宽度调整长度）
 * @param params
 */
export function renderVirtualStopLine (params: {
  objectId: string
  parent: Object3D
  point?: common.vec3
  // 只有在道路上移动标识，才有相对于参考线坐标系的位置信息
  roadId?: string
  refLineLocation?: biz.IRefLineLocation
  length: number // 停止线的实时长度
}) {
  const {
    objectId,
    parent,
    point,
    roadId = '',
    refLineLocation = null,
    length,
  } = params
  // 如果当前编辑模式中没有对应的预览辅助元素，则需要创建+添加到编辑模式容器中
  let virtualRoadSign = parent.getObjectByName('virtualRoadSign')
  if (!virtualRoadSign) {
    // 外层的容器
    virtualRoadSign = new Group()
    virtualRoadSign.name = 'virtualRoadSign'
    virtualRoadSign.objectId = objectId
    virtualRoadSign.roadId = roadId
    parent.add(virtualRoadSign)
  }

  // 先销毁容器内部的网格
  virtualRoadSign.traverse((child) => {
    if (child instanceof Mesh) {
      child.geometry.dispose()
      disposeMaterial(child)
      if (child.parent) {
        child.parent.remove(child)
      }
    }
  })

  // 创建一个尺寸为 1*1 的网格，通过缩放来实线预览的效果
  const vertices = []
  const indices = []
  vertices.push(0.5, 0, 0.5)
  vertices.push(0.5, 0, -0.5)
  vertices.push(-0.5, 0, -0.5)
  vertices.push(-0.5, 0, 0.5)
  indices.push(0, 1, 2)
  indices.push(2, 3, 0)

  const _vertices = new Float32Array(vertices)
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  const mat = customRoadSignMaterial.clone()
  const mesh = new Mesh(geo, mat)
  mesh.name = 'roadSign'
  mesh.renderOrder = RenderOrder.roadSign
  mesh.scale.set(length, 1, Constant.stopLineSize.width)
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  virtualRoadSign.add(mesh)

  // 根据 roadId 是否存在，来判断物体在道路上是自由移动还是限制于车道中
  if (roadId) {
    if (point) {
      // 道路上自由移动的路面标识
      virtualRoadSign.position.set(point.x, point.y, point.z)

      const roadSignData = getObject(objectId)
      if (refLineLocation && roadSignData) {
        const { normal = null } = refLineLocation
        if (normal) {
          virtualRoadSign.up.set(normal.x, normal.y, normal.z)
        }
        const lookAtPoint = getLookAtPointByYaw({
          pointOnRefLine: refLineLocation.closestPointOnRefLine,
          tangent: refLineLocation.tangent,
          point,
          yaw: roadSignData.yaw,
          useElevation: true,
        })
        virtualRoadSign.lookAt(lookAtPoint)
      }

      virtualRoadSign.roadId = roadId
      virtualRoadSign.refLineLocation = refLineLocation
    }
  }

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualRoadSign) {
    updateBox3Helper(virtualRoadSign)
  }
}

/**
 * 渲染预览的辅助交互路面标识
 * @param params
 */
export function renderVirtualRoadSign (params: {
  objectId: string
  parent: Object3D
  point?: common.vec3
  // 只有在道路上移动标识，才有相对于参考线坐标系的位置信息
  roadId?: string
  refLineLocation?: biz.IRefLineLocation
}) {
  const {
    objectId,
    parent,
    point,
    roadId = '',
    refLineLocation = null,
  } = params
  // 如果当前编辑模式中没有对应的预览辅助元素，则需要创建+添加到编辑模式容器中
  const mapContainer = root3d.mapElementsContainer
  let virtualRoadSign = parent.getObjectByName('virtualRoadSign')
  if (!virtualRoadSign) {
    const originRoadSign = mapContainer.getObjectByProperty(
      'objectId',
      objectId,
    )
    if (!originRoadSign) return
    virtualRoadSign = originRoadSign.clone()
    // 给预览的辅助元素设置唯一标识
    virtualRoadSign.name = 'virtualRoadSign'
    // 由于原始物体已经隐藏，需要在 clone 时将预览放置的元素设置成可见
    virtualRoadSign.visible = true
    // 自定义属性需要手动拷贝
    virtualRoadSign.objectId = objectId
    parent.add(virtualRoadSign)
  }

  // 根据 roadId 是否存在，来判断物体在道路上是自由移动还是限制于车道中
  if (roadId) {
    if (point) {
      // 道路上自由移动的路面标识
      virtualRoadSign.position.set(point.x, point.y, point.z)

      const roadSignData = getObject(objectId)
      if (refLineLocation && roadSignData) {
        // 如果拖拽自由移动的虚拟物体，所属的 t 坐标前后两次不同，则需要调整朝向
        let yaw = roadSignData.yaw
        if (
          (roadSignData.t < 0 && refLineLocation.t > 0) ||
          (roadSignData.t > 0 && refLineLocation.t < 0)
        ) {
          yaw = getValidDegreeInRange(yaw + 180)
        }
        const { normal = null } = refLineLocation
        if (normal) {
          virtualRoadSign.up.set(normal.x, normal.y, normal.z)
        }
        const lookAtPoint = getLookAtPointByYaw({
          pointOnRefLine: refLineLocation.closestPointOnRefLine,
          tangent: refLineLocation.tangent,
          point,
          yaw,
          useElevation: true,
        })
        virtualRoadSign.lookAt(lookAtPoint)
      }

      virtualRoadSign.roadId = roadId
      virtualRoadSign.refLineLocation = refLineLocation
    }
  } else {
    // 基于新的位置信息调整预览物体的属性
    if (refLineLocation) {
      // 限制于车道中的路面标识
      const { t, closestPointInLane, tangent } = refLineLocation
      const _position = new Vector3(
        closestPointInLane.x,
        closestPointInLane.y,
        closestPointInLane.z,
      )
      const _tangent = new Vector3(tangent.x, tangent.y, tangent.z)
      virtualRoadSign.position.set(_position.x, _position.y, _position.z)
      const frontVec3 = _position
        .clone()
        // 根据朝向属性调整渲染方向
        .addScaledVector(_tangent, t > 0 ? -1 : 1)
      virtualRoadSign.lookAt(frontVec3)
    } else if (point) {
      // 路口中虚拟放置的路面标识
      virtualRoadSign.position.set(point.x, point.y, point.z)
    }
  }

  // 拖拽过程中，实时更新包围盒外框的位置
  if (virtualRoadSign) {
    updateBox3Helper(virtualRoadSign)
  }
}

// 销毁预览的路面标识元素
export function disposeVirtualObject (params: {
  parent: Group
  hasGroup?: boolean // 元素外层是否有容器
  isAll?: boolean
}) {
  const { parent, hasGroup = false, isAll = false } = params
  // 符合删除要求的 parent 直接子集
  const matchGroup: Array<Group> = []
  parent.traverse((child) => {
    if (hasGroup || isAll) {
      // 带外层容器的网格销毁
      if (
        child instanceof Group &&
        (child.name === 'virtualRoadSign' ||
        child.name === 'virtualSignalBoard' ||
        child.name === 'virtualTrafficLight' ||
        child.name === 'virtualPole' ||
        child.name === 'virtualOther' ||
        child.name === 'virtualParkingSpace' ||
        child.name === 'virtualSensor' ||
        child.name === 'virtualCustomModel')
      ) {
        // @ts-expect-error
        const { objectId } = child
        if (objectId) {
          // 如果拖拽的虚拟物体销毁时，原物体还处于隐藏状态，需要将原物体还原显示
          showOriginObject(objectId)
        }
        matchGroup.push(child)
      }
    }

    if (!hasGroup || isAll) {
      // 不带外层容器的网格销毁
    }
  })

  for (const oneGroup of matchGroup) {
    const matchMesh: Array<Mesh> = []
    oneGroup.traverse((child) => {
      if (child instanceof Mesh) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchMesh.push(child)
      }
    })
    oneGroup.remove(...matchMesh)
    oneGroup.clear()
  }

  parent.remove(...matchGroup)
}

/**
 * 渲染道路周边的辅助放置区域
 * @param params
 */
export function renderRoadHelperArea (params: {
  roadId: string
  leftPoints: Array<Vector3>
  rightPoints: Array<Vector3>
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

  const mesh = new Mesh(geo, roadHelperAreaMat)
  mesh.name = 'roadPlacementArea'
  mesh.roadId = roadId
  mesh.renderOrder = RenderOrder.placementArea
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

/**
 * 渲染路口周边的辅助放置区域
 * @param params
 */
export function renderJunctionHelperArea (params: {
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
  const mesh = new Mesh(geo, junctionHelperAreaMat)
  mesh.name = 'junctionPlacementArea'
  mesh.junctionId = junctionId
  mesh.renderOrder = RenderOrder.placementArea
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

/**
 * 销毁道路辅助放置区域元素
 * @param parent
 */
export function disposeRoadHelperArea (params: {
  parent: Object3D
  roadId?: string
}) {
  const { parent, roadId = '' } = params
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'roadPlacementArea') {
      if (!roadId || child.roadId === roadId) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 销毁路口辅助放置区域元素
 * @param parent
 */
export function disposeJunctionHelperArea (params: {
  parent: Object3D
  junctionId?: string
}) {
  const { parent, junctionId = '' } = params
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'junctionPlacementArea') {
      if (!junctionId || child.junctionId === junctionId) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}
