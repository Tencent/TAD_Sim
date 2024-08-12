import type {
  Material,
  Object3D,
} from 'three'
import {
  Box3,
  Box3Helper,
  Color,
  Group,
  Mesh,
  Vector3,
} from 'three'
import { updateSignalBoardColor } from './signalBoard'
import { updateTrafficLightColor } from './trafficLight'
import { updateSensorColor } from './sensor'
import { disposeMaterial } from '@/utils/common3d'
import { RenderOrder } from '@/utils/business'
import root3d from '@/main3d/index'

type IObjectOnPoleType = 'signalBoard' | 'trafficLight' | 'sensor'

let cloneObject: Object3D | null = null
let containerBox: Box3 | null = null
let box3Helper: Box3Helper | null = null
let box3HelperContainer: Group | null = null

/**
 * 通过物体的 id 更新物体的包围盒外框
 * @param id
 */
export function updateBox3HelperById (id: string) {
  const targetObject = root3d.getObjectById(id)
  if (!targetObject) return

  updateBox3Helper(targetObject)
}

/**
 * 更新选中物体是的包围盒外边框（唯一）
 */
export function updateBox3Helper (object: Object3D) {
  // 始终将包围盒外边框渲染在核心地图元素的容器中，从而避免放置在各个容器中难以管理
  const parent = root3d.mapElementsContainer
  const groupName = object.name

  // 先销毁原先的包围盒外边框
  disposeBox3Helper()

  // 创建新的包围盒外边框
  containerBox = new Box3()
  cloneObject = object.clone()

  if (!containerBox || !cloneObject) return

  // 计算物体重心的坐标（物体重心不一定在中心，会出现包围盒偏移的情况）
  const originBox = new Box3()
  originBox.expandByObject(object)
  const gravityCenter = new Vector3()
  originBox.getCenter(gravityCenter)

  // 基于原始的物体容器克隆备份
  // 如果是竖杆和横杆（由于自身也是容器，需要只拷贝杆元素实体网格本身）
  if (
    groupName === 'verticalPoleGroup' ||
    groupName === 'horizontalPoleGroup' ||
    groupName === 'virtualPole' // 拖拽虚拟杆时的名称
  ) {
    // 重新创建一个容器
    cloneObject = new Group()
    object.traverse((child) => {
      if (
        child instanceof Mesh &&
        (child.name === 'verticalPole' ||
        child.name === 'horizontalPole' ||
        child.name === 'verticalPoleTop' ||
        child.name === 'horizontalPoleTop')
      ) {
        // 将实体克隆到容器中
        cloneObject.add(child.clone())
      }
    })
  }

  // 将位置和角度重置（scale使用物体当前尺寸）
  cloneObject.position.set(0, 0, 0)
  cloneObject.rotation.set(0, 0, 0)
  // 基于克隆未添加到场景中初始坐标初始角度的物体进行包围盒的检测
  containerBox.expandByObject(cloneObject)

  // 获取物体在世界坐标系下的坐标
  const wp = new Vector3()
  object.getWorldPosition(wp)

  // 计算物体重心和坐标中心的水平偏移量
  const offset = new Vector3(wp.x - gravityCenter.x, 0, wp.z - gravityCenter.z)

  // 创建包围盒外框的实体元素
  box3Helper = new Box3Helper(containerBox, new Color(0xFFFF00))
  // @ts-expect-error
  box3Helper.material.depthTest = false

  box3Helper.renderOrder = RenderOrder.boxHelper
  // 包围盒实体元素需要跟克隆物体内的网格一样设定偏移
  box3Helper.position.set(offset.x, offset.y, offset.z)

  // 将包围盒容器应用物体的空间矩阵
  box3HelperContainer = new Group()
  box3HelperContainer.name = 'box3HelperGroup'
  box3HelperContainer.add(box3Helper)
  // 容器仅使用物体的世界坐标和角度
  box3HelperContainer.applyMatrix4(object.matrixWorld)
  // 手动将外层容器的尺寸重置为 1
  box3HelperContainer.scale.set(1, 1, 1)

  parent.add(box3HelperContainer)
}

/**
 * 销毁当前场景中的包围盒外边框
 */
export function disposeBox3Helper () {
  if (box3Helper && box3HelperContainer) {
    box3Helper.geometry.dispose()
    disposeMaterial(box3Helper)
    if (box3Helper.parent) {
      box3Helper.parent.remove(box3Helper)
    }
    if (box3HelperContainer.parent) {
      box3HelperContainer.parent.remove(box3HelperContainer)
    }

    // 重置变量
    box3Helper = null
    box3HelperContainer = null
    containerBox = null
    cloneObject = null
  }
}

/**
 * 重置所有杆上物体的交互效果
 */
export function resetAllObjectOnPole (parent: Object3D) {
  parent.traverse((child) => {
    if (child instanceof Group) {
      if (child.name === 'signalBoardGroup') {
        // 标志牌
        updateSignalBoardColor({
          object: child,
          status: 'normal',
          isForced: true,
        })
      } else if (child.name === 'trafficLightGroup') {
        // 信号灯
        updateTrafficLightColor({
          object: child,
          status: 'normal',
          isForced: true,
        })
      } else if (child.name === 'sensorGroup') {
        // 传感器
        updateSensorColor({
          object: child,
          status: 'normal',
          isForced: true,
        })
      }
    }
  })
}

/**
 * 通过 id 找到物体，更新杆上物体的交互效果
 * @param params
 */
export function updateObjectOnPoleById (params: {
  type: IObjectOnPoleType
  objectId: string
  parent: Object3D
  status: common.objectStatus
  isForced?: boolean // 是否强制更新
}) {
  const { type, objectId, parent, status, isForced = false } = params
  parent.traverse((child) => {
    // 找到杆上物体网格的外层容器Group
    if (child instanceof Group && child.objectId === objectId) {
      if (type === 'signalBoard') {
        // 标志牌
        updateSignalBoardColor({
          object: child,
          status,
          isForced,
        })
      } else if (type === 'trafficLight') {
        // 信号灯
        updateTrafficLightColor({
          object: child,
          status,
          isForced,
        })
      } else if (type === 'sensor') {
        // 传感器
        updateSensorColor({
          object: child,
          status,
          isForced,
        })
      }
    }
  })
}

/**
 * 杆上物体选中的交互效果
 */
export function updateObjectOnPole (params: {
  type: IObjectOnPoleType
  object: Object3D
  status: common.objectStatus
  isForced?: boolean // 是否强制更新
}) {
  const { type, object, status, isForced = false } = params
  let objectGroup: Object3D | null = null
  if (object instanceof Mesh) {
    objectGroup = object.parent
  } else {
    objectGroup = object
  }
  // 如果模型外层的容器不存在，或者容器的name属性不包含Group字
  if (!objectGroup || objectGroup.name !== `${type}Group`) return

  if (type === 'signalBoard') {
    // 标志牌
    updateSignalBoardColor({
      object: objectGroup,
      status,
      isForced,
    })
  } else if (type === 'trafficLight') {
    // 信号灯
    updateTrafficLightColor({
      object: objectGroup,
      status,
      isForced,
    })
  } else if (type === 'sensor') {
    // 传感器
    updateSensorColor({
      object: objectGroup,
      status,
      isForced,
    })
  }
}

// 隐藏原始的物体（容器 or 网格）
export function hideOriginObject (objectId: string) {
  const parent = root3d.mapElementsContainer
  const targetObject = parent.getObjectByProperty('objectId', objectId)
  if (!targetObject) return
  if (targetObject.visible === false) return
  targetObject.visible = false
}

// 显示原始的物体（容器 or 网格）
export function showOriginObject (objectId: string) {
  const parent = root3d.mapElementsContainer
  const targetObject = parent.getObjectByProperty('objectId', objectId)
  if (!targetObject) return
  if (targetObject.visible === true) return
  targetObject.visible = true
}

// 销毁一个实际的物体元素
export function disposeOneObject (params: {
  objectId: string
  parent: Object3D
}) {
  const { objectId, parent } = params
  const matchGroup: Array<Group> = []
  // 先找物体外层的容器
  parent.traverse((child) => {
    if (child instanceof Group && child.objectId === objectId) {
      matchGroup.push(child)
    }
  })

  // 再将容器中所有的网格删除
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

  // group 的外层容器，不一定是 mapElementContainer，有可能是 poleContainer
  // 使用容器的直接父级来删除目标元素
  if (matchGroup.length > 0) {
    const groupParent = matchGroup[0].parent
    if (groupParent) {
      groupParent.remove(...matchGroup)
    }
  }
}

// 对于部分直接从 fbx 文件中渲染材质的模型，为了保证 hover 时材质不是同一个引用，需要手动 clone 对应的材质
export function cloneOriginMaterial (group: Object3D) {
  // 区分单独的材质对象和材质数组
  group.traverse((child) => {
    if (child instanceof Mesh) {
      if (Array.isArray(child.material)) {
        const cloneMats: Array<Material> = []
        for (const mat of child.material) {
          const cloneMat = mat.clone()
          cloneMat.needsUpdate = true
          cloneMats.push(cloneMat)
        }
        child.material = cloneMats
      } else {
        const cloneMat = child.material.clone()
        child.material = cloneMat
        child.material.needsUpdate = true
      }
    }
  })
}
