import { Color, Group, Mesh, type Object3D, Vector3 } from 'three'
import { load3DModel } from '../loaders'
import { cloneOriginMaterial, disposeOneObject } from './object'
import { useModelCacheStore } from '@/stores/object/modelCache'
import { RenderOrder } from '@/utils/business'
import { getObject } from '@/utils/mapCache'

interface IRenderCustomModel {
  data: biz.ICustomModel
  parent: Object3D
}

// 各状态自发光强度的偏差
const emissiveIntensityOffset = {
  normal: 0,
  hovered: 0.3,
  selected: 0.01,
}

/**
 * 由于自定义模型配置的改变，单纯更新渲染层自定义模型渲染效果
 */
export async function reRenderCustomModel (params: IRenderCustomModel) {
  const { data: modelData, parent } = params

  // 先销毁原有的模型
  disposeOneObject({
    objectId: modelData.id,
    parent,
  })

  // 重新渲染对应的物体
  await createCustomModel(params)
}

/**
 * 更新自定义模型的尺寸
 * @param params
 */
export function updateCustomModelSize (params: IRenderCustomModel) {
  const { data: modelData, parent } = params
  const { id: objectId, length, basicLength } = modelData
  // 从 parent 中找到对应的物体
  const targetObject = parent.getObjectByProperty('objectId', objectId)
  if (!targetObject) return

  // 基于模型最初的尺寸来调节 scale 比例
  const scale = length / basicLength
  targetObject.scale.set(scale, scale, scale)
}

export function resetAllCustomModel (parent: Object3D) {
  parent.traverse((child) => {
    // 其他物体的容器
    if (child instanceof Group && child.name === 'customModelGroup') {
      // 强制更新物体的交互效果为初始状态
      updateCustomModelColor({
        object: child,
        status: 'normal',
        isForced: true,
      })
    }
  })
}

/**
 * 更新其他类型物体的颜色
 */
function updateCustomModelColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object: objectGroup, status, isForced = false } = params
  // @ts-expect-error
  const modelData = getObject(objectGroup.objectId)
  if (!modelData) return

  const defaultIntensity = 0

  // 先遍历第一遍，判断是否需要更新
  let needUpdate = false
  if (isForced) {
    needUpdate = true
  } else {
    objectGroup.traverse((child) => {
      // 如果已经遍历过确认需要更新了，则无须执行后续的重复遍历
      if (needUpdate) return

      if (child instanceof Mesh) {
        if (Array.isArray(child.material)) {
          // 如果网格的材质是数组
          const mat0 = child.material[0]
          if (
            mat0.emissiveIntensity !==
            defaultIntensity + emissiveIntensityOffset.selected
          ) {
            needUpdate = true
          }
        } else {
          // 如果网格的材质是单个材质实例
          if (
            child.material.emissiveIntensity !==
            defaultIntensity + emissiveIntensityOffset.selected
          ) {
            needUpdate = true
          }
        }
      }
    })
  }

  // 如果不需要更新，则直接返回
  if (!needUpdate) return

  // 目标的自发光强度
  const targetIntensity = defaultIntensity + emissiveIntensityOffset[status]

  objectGroup.traverse((child) => {
    if (child instanceof Mesh) {
      if (Array.isArray(child.material)) {
        // 如果是材质数组
        child.material.forEach((mat) => {
          mat.emissiveIntensity = targetIntensity
          mat.needsUpdate = true
        })
      } else {
        child.material.emissiveIntensity = targetIntensity
        child.material.needsUpdate = true
      }
    }
  })
}

/**
 * 通过 id 找到物体并更新对应交互效果
 */
export function updateCustomModelById (params: {
  objectId: string
  parent: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { objectId, parent, status, isForced = false } = params
  let objectGroup: Object3D | null = null
  parent.traverse((child) => {
    if (
      child instanceof Group &&
      child.name === 'customModelGroup' &&
      child.objectId === objectId
    ) {
      objectGroup = child
    }
  })
  if (!objectGroup) return

  // 提供颜色的高亮
  updateCustomModelColor({
    object: objectGroup,
    status,
    isForced,
  })
}

/**
 * 其他类型物体的交互效果
 */
export function updateCustomModel (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean // 是否强制更新（主要用于选中和取消选中时透传）
}) {
  const { object, status, isForced = false } = params
  let objectGroup: Object3D | null = null
  if (object instanceof Mesh) {
    // 自定义导入的模型，存在2层的group
    objectGroup = object.parent?.parent
  } else {
    objectGroup = object
  }
  if (!objectGroup || objectGroup.name !== 'customModelGroup') return

  // 提供颜色的高亮
  updateCustomModelColor({
    object: objectGroup,
    status,
    isForced,
  })
}

// 三维场景中创建自定义模型
export async function createCustomModel (params: IRenderCustomModel) {
  const { data: modelData, parent } = params
  const {
    id: modelId,
    name,
    position,
    roadId = '',
    junctionId = '',
    lookAtPoint,
    width: currentWidth,
    length: currentLength,
    height: currentHeight,
    projectNormal,
  } = modelData

  // 获取对应模型静态资源的引用路径
  const modelCacheStore = useModelCacheStore()
  const configOption = modelCacheStore.getCustomModelConfigByName(name)
  if (!configOption) return

  const { modelUrl, posX, posY, posZ, length, width, height } = configOption
  if (!modelUrl) return

  // 加载对应自定义模型
  const model = await load3DModel(modelUrl)

  const emissiveColor = 0xFFFFFF

  // 由于是直接加载 fbx 文件中对应的贴图材质，缓存模型后需要 clone 处理
  cloneOriginMaterial(model)

  // 如果当前尺寸存在，则应用当前的尺寸
  let scaleX = 1
  let scaleY = 1
  let scaleZ = 1
  if (currentLength && length) {
    scaleZ = currentLength / length
  }
  if (currentWidth && width) {
    scaleX = currentWidth / width
  }
  if (currentHeight && height) {
    scaleY = currentHeight / height
  }

  model.scale.set(scaleX, scaleY, scaleZ)

  // 由于模型需要有原点坐标偏移效果，提供2层group容器
  const group = new Group()
  group.name = 'customModelGroup'
  group.objectId = modelId
  group.roadId = roadId
  group.junctionId = junctionId

  model.traverse((child) => {
    if (child instanceof Mesh) {
      // 更新对应网格的属性
      child.name = 'customModel'
      child.renderOrder = RenderOrder.customModel
      child.objectId = modelId
      child.roadId = roadId
      child.junctionId = junctionId

      // 设置将所有网格的材质自发光属性（用户后续鼠标交互状态联动）
      if (Array.isArray(child.material)) {
        for (const mat of child.material) {
          const { map } = mat
          mat.emissive = new Color(emissiveColor)
          mat.emissiveMap = map
          mat.emissiveIntensity = 0
          mat.needsUpdate = true
        }
      } else {
        const { map } = child.material
        child.material.emissive = new Color(emissiveColor)
        child.material.emissiveMap = map
        child.material.emissiveIntensity = 0
        child.material.needsUpdate = true
      }
    }
  })
  model.name = 'customModelProxyGroup'
  model.objectId = modelId
  model.roadId = roadId
  model.junctionId = junctionId

  model.position.set(posY, posZ, posX)

  // 和场景编辑器保证一致
  model.rotateY((-90 * Math.PI) / 180)

  group.add(model)
  group.position.set(position.x, position.y, position.z)

  if (projectNormal) {
    // 如果存在上方向的更新，在 lookAt 属性更新前，更新 up 方向
    group.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  // 针对道路上的物体调整朝向
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    group.lookAt(_lookAtPoint)
  }

  parent.add(group)

  return {
    width,
    length,
    height,
  }
}

// 移动物体
export function moveCustomModel (params: IRenderCustomModel) {
  const { data, parent } = params
  const {
    id: modelId,
    roadId = '',
    position,
    lookAtPoint,
    projectNormal,
  } = data
  // 从容器中找到对应其他类型物体的
  let targetGroup: Group
  parent.traverse((child) => {
    if (targetGroup) return
    if (
      child instanceof Group &&
      child.objectId === modelId &&
      child.name === 'customModelGroup'
    ) {
      targetGroup = child
    }
  })

  if (!targetGroup) return

  // 道路中和路口中物体位置更新
  targetGroup.position.set(position.x, position.y, position.z)

  // 仅限于需要跟随道路坡度调整的物体，才需要调整 up 方向和朝向
  if (projectNormal) {
    // 如果存在上方向，则在 lookAt 之前先调整上方向
    targetGroup.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    targetGroup.lookAt(_lookAtPoint)
  }
}

// 按照一定的角度旋转自定义模型
export function rotateCustomModel (params: IRenderCustomModel) {
  const { data, parent } = params
  const { id: modelId, lookAtPoint } = data
  // 从容器中找到对应其他类型物体的
  let targetGroup: Group
  parent.traverse((child) => {
    if (targetGroup) return
    if (
      child instanceof Group &&
      child.objectId === modelId &&
      child.name === 'customModelGroup'
    ) {
      targetGroup = child
    }
  })

  if (!targetGroup) return

  // 道路上和路口中，都是通过调整物体看向目标点实现角度的旋转
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    targetGroup.lookAt(_lookAtPoint)
  }
}
