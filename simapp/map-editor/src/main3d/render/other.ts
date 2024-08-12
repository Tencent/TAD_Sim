import type {
  Object3D,
} from 'three'
import {
  Box3,
  DoubleSide,
  Group,
  Mesh,
  MeshLambertMaterial,
  MeshPhongMaterial,
  PlaneGeometry,
  Vector3,
} from 'three'
import { load3DModel, loadTexture } from '../loaders'
import { type IOtherConfig, getOtherConfig } from '@/config/other'
import { Constant, RenderOrder } from '@/utils/business'
import { PI, axisX, axisY, fixedPrecision, halfPI } from '@/utils/common3d'
import { ModelUrls } from '@/utils/urls'
import { getObject } from '@/utils/mapCache'

interface IRenderOther {
  data: biz.IOther
  parent: Group
}

// 各状态自发光强度的偏差
const emissiveIntensityOffset = {
  normal: 0,
  hovered: 0.3,
  selected: 0.01,
}

// 为了更好的渲染效果，给物体添加自发光效果
const emissiveIntensityMap = {
  // 地图污损类
  Pothole: -0.05,
  Patch: 0.1,
  Crack: 0.1,
  Asphalt_Line: 0.1,
  Rut_Track: 0.1,
  Stagnant_Water: 0.1,
  Protrusion: 0.1,
  // 交通管理类
  Well_Cover: 0.05,
  Deceleration_Zone: -0.1,
  Parking_Hole: 1,
  Parking_Lot: 0.1,
  Ground_Lock: 0.1,
  Plastic_Vehicle_Stopper: 0.1,
  Parking_Limit_Position_Pole_2m: 0.1,
  Support_Vehicle_Stopper: 0.1,
  Charging_Station: 0.1,
  Lamp: -0.1,
  Traffic_Barrier: 0.05,
  Road_Curb: 0.2,
  Reflective_Road_Sign: 0.5,
  Traffic_Cone: 0.2,
  Traffic_Horse: 2,
  Obstacle: 0.5,
  GarbageCan: 0.7,
  // 植被类
  Tree: 2,
  Shrub: 0.3,
  Grass: 0.1,
  // 建筑类
  Building: 0.2,
  BusStation: 0.3,
  // 天桥类
  PedestrianBridge: 0.5,
}

// 模型长和宽的朝向跟自然语言理解相反的模型，需要手动调整对应的长和宽
const fixSizeObjects = [
  'Parking_Hole', // 不能修改尺寸
  'Charging_Station', // 不能修改尺寸
  'Road_Curb', // 可以修改尺寸
]

export function resetAllOtherObject (parent: Object3D) {
  parent.traverse((child) => {
    // 其他物体的容器
    if (child instanceof Group && child.name === 'otherGroup') {
      // 强制更新物体的交互效果为初始状态
      updateOtherObjectColor({
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
function updateOtherObjectColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object: objectGroup, status, isForced = false } = params
  // @ts-expect-error
  const otherData = getObject(objectGroup.objectId)
  if (!otherData) return

  const name = otherData.name
  // @ts-expect-error
  const defaultIntensity = emissiveIntensityMap[name]
  if (defaultIntensity === undefined) return

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
export function updateOtherObjectById (params: {
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
      child.name === 'otherGroup' &&
      child.objectId === objectId
    ) {
      objectGroup = child
    }
  })
  if (!objectGroup) return

  // 提供颜色的高亮
  updateOtherObjectColor({
    object: objectGroup,
    status,
    isForced,
  })
}

/**
 * 其他类型物体的交互效果
 */
export function updateOtherObject (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean // 是否强制更新（主要用于选中和取消选中时透传）
}) {
  const { object, status, isForced = false } = params
  let objectGroup: Object3D | null = null
  if (object instanceof Mesh) {
    objectGroup = object.parent
  } else {
    objectGroup = object
  }
  if (!objectGroup || objectGroup.name !== 'otherGroup') return

  // 提供颜色的高亮
  updateOtherObjectColor({
    object: objectGroup,
    status,
    isForced,
  })
}

/**
 * 更新其他类型模型的尺寸
 * @param params
 */
export function updateOtherSize (params: IRenderOther) {
  const { data: otherData, parent } = params
  const {
    id: objectId,
    width,
    length,
    height,
    name,
    basicWidth,
    basicLength,
    basicHeight,
  } = otherData
  // 从 parent 中找到对应的物体
  const targetObject = parent.getObjectByProperty('objectId', objectId)
  if (!targetObject) return

  // 基于模型最初的尺寸来调节 scale 比例
  let scaleZ = length / basicLength
  let scaleX = width / basicWidth
  let scaleY = 1

  // 如果是由于模型轴向跟自然语言理解的长宽相反，则对调两个值
  if (fixSizeObjects.includes(name)) {
    const temp = scaleZ
    scaleZ = scaleX
    scaleX = temp
  }
  // 平面模型不存在有效高度
  if (height && basicHeight) {
    scaleY = height / basicHeight
  }
  targetObject.scale.set(scaleX, scaleY, scaleZ)
}

// 调整部分模型的属性
function adjustOtherModel (params: {
  modelGroup: Group
  data: biz.IOther
  config: IOtherConfig
}) {
  const { modelGroup, data, config } = params
  const { name: objectName } = data
  modelGroup.traverse((child) => {
    if (child instanceof Mesh) {
      // 尺寸相关的调整
      if (objectName === 'Protrusion') {
        child.scale.set(1, 1, 1)
        // 凸起模型圆心在几何中心，而不是地面中心，手动调整
        const box = new Box3()
        box.expandByObject(modelGroup)
        const height = fixedPrecision(box.max.y - box.min.y, 4)
        child.position.set(child.position.x, height / 2, child.position.z)
      } else if (objectName === 'Reflective_Road_Sign') {
        // 原始模型太大了，将 Mesh 的尺寸默认调整为 1
        child.scale.set(1, 1, 1)
      } else if (
        objectName === 'BusStation' ||
        objectName === 'Traffic_Barrier' ||
        objectName === 'Lamp' ||
        objectName === 'Traffic_Horse' ||
        objectName === 'GarbageCan'
      ) {
        // 部分模型需要调整90°方向
        child.rotateZ(halfPI)
      }

      // 材质相关的调整
      // @ts-expect-error
      const emissiveIntensity = emissiveIntensityMap[objectName]
      if (emissiveIntensity) {
        // 如果一些模型在场景中的渲染效果不理想，则主动替换材质
        const basicColor = 0xFFFFFF
        const emissiveColor = 0xFFFFFF
        const alphaTest = 0.1

        if (Array.isArray(child.material)) {
          const newMats = []
          for (const originMat of child.material) {
            if (
              originMat instanceof MeshPhongMaterial ||
              originMat instanceof MeshLambertMaterial
            ) {
              const { map, transparent, depthTest, depthWrite, name } =
                originMat
              const opacity = transparent ? 0.98 : 1
              const newMat = new MeshLambertMaterial({
                alphaTest,
                name,
                color: basicColor,
                map,
                transparent,
                opacity,
                depthTest,
                depthWrite,
                emissive: emissiveColor,
                emissiveMap: map,
                emissiveIntensity,
                side: DoubleSide,
              })
              newMats.push(newMat)
            }
          }
          child.material = newMats
        } else {
          const originMat = child.material
          if (
            originMat instanceof MeshPhongMaterial ||
            originMat instanceof MeshLambertMaterial
          ) {
            const { map, depthTest, depthWrite, name } = originMat
            let { transparent } = originMat
            if (objectName === 'Shrub') {
              // 灌木模型材质默认不是透明的，手动修改
              transparent = true
            }
            const opacity = transparent ? 0.98 : 1
            const newMat = new MeshLambertMaterial({
              alphaTest,
              name,
              color: basicColor,
              map,
              transparent,
              opacity,
              depthTest,
              depthWrite,
              emissive: emissiveColor,
              emissiveMap: map,
              emissiveIntensity,
              side: DoubleSide,
            })
            child.material = newMat
          }
        }
      }
    }
  })
}

/**
 * 创建人行天桥
 * @param params
 */
async function createBridge (params: IRenderOther) {
  const { data: otherData, parent } = params
  const {
    id: objectId,
    name,
    position,
    roadId = '',
    junctionId = '',
    lookAtPoint,
    projectNormal,
  } = otherData

  const configOption = getOtherConfig(name)
  if (!configOption) return

  // 人行天桥由 3 个模型手动组装而成
  const modelUrl0 = ModelUrls.other.pedestrianBridge
  const modelUrl1 = ModelUrls.other.pedestrianBridge1
  const modelUrl2 = ModelUrls.other.pedestrianBridge2

  // 模型加载完成后，自带Group
  const modelGroup0 = await load3DModel(modelUrl0)
  const modelGroup1 = await load3DModel(modelUrl1)
  const modelGroup2 = await load3DModel(modelUrl2)

  // 提取每一个group中的有效网格元素
  const model0 = modelGroup0.children[0]
  const model1 = modelGroup1.children[0]
  const model2 = modelGroup2.children[0]

  // 计算中间部分模型的尺寸
  const centerBox = new Box3()
  centerBox.expandByObject(model0)
  const centerW = fixedPrecision(centerBox.max.x - centerBox.min.x)

  // 容器
  const bridgeGroup = new Group()

  // 下坡模型的宽度
  const slopeHalfWidth = 2.45
  // 中间部分离地高度
  const heightOffset = 9.98
  // 获取天桥的跨度
  const { span = Constant.bridgeSpan } = otherData

  // 根据天桥的跨度，将中间模型拷贝多份
  /**
   * 0-15m => 1份
   * 15-25m => 2份
   * 25-35m => 3份
   * 35-50m => 4份
   */
  let count = 1
  if (span > 15 && span <= 25) {
    count = 2
  } else if (span > 25 && span <= 35) {
    count = 3
  } else if (span > 35) {
    count = 4
  }
  // 每一份中间模型的尺寸
  const perWidth = span / count

  const centerModels = []
  for (let i = 0; i < count; i++) {
    const _model = model0.clone()
    _model.scale.set(perWidth / centerW, 1, 1)
    // 中间部分的偏移量
    _model.translateX(perWidth * i - perWidth * (count / 2 - 1 / 2))
    // 中间部分抬离地面
    _model.translateZ(heightOffset)
    centerModels.push(_model)
  }

  // 左右两侧坡道的偏移
  model1.translateX(-(span / 2 + slopeHalfWidth))
  model2.translateX(span / 2 + slopeHalfWidth)

  // 天桥容器的子集，保持是 Mesh 类型的数组
  bridgeGroup.add(...centerModels, model1, model2)

  // 调整天桥的渲染效果
  adjustOtherModel({
    modelGroup: bridgeGroup,
    data: otherData,
    config: configOption,
  })

  bridgeGroup.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'other'
      child.renderOrder = RenderOrder.other
      child.objectId = objectId
      child.roadId = roadId
      child.junctionId = junctionId
      // 由于天桥是由3个模型拼装而成，每一个单独模型在 group 中存在偏移量，需要提供标识来辅助拖拽
      child.isBridge = true
    }
  })
  bridgeGroup.name = 'otherGroup'
  bridgeGroup.objectId = objectId
  bridgeGroup.roadId = roadId
  bridgeGroup.junctionId = junctionId
  bridgeGroup.isBridge = true

  const box = new Box3()
  box.expandByObject(bridgeGroup)
  let width = fixedPrecision(box.max.x - box.min.x)
  const height = fixedPrecision(box.max.y - box.min.y)
  let length = fixedPrecision(box.max.z - box.min.z)
  // 如果是由于模型轴向跟自然语言理解的长宽相反，则对调两个值
  if (fixSizeObjects.includes(name)) {
    const temp = length
    length = width
    width = temp
  }

  bridgeGroup.position.set(position.x, position.y, position.z)

  // 针对道路上的物体调整指向
  if (projectNormal) {
    bridgeGroup.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    bridgeGroup.lookAt(_lookAtPoint)
  }

  parent.add(bridgeGroup)

  return {
    width,
    height,
    length,
  }
}

/**
 * 创建物体并渲染到场景中
 * @param params
 */
export async function createOther (params: IRenderOther) {
  const { data: otherData, parent } = params
  const {
    id: objectId,
    name,
    position,
    roadId = '',
    junctionId = '',
    lookAtPoint,
    size = [],
    width: currentWidth,
    length: currentLength,
    height: currentHeight,
    projectNormal,
  } = otherData

  const configOption = getOtherConfig(name)
  if (!configOption) return

  const { modelUrl, textureUrl = '' } = configOption

  // 如果是人行天桥，需要特殊处理
  if (modelUrl === ModelUrls.other.pedestrianBridge) {
    // 将创建完人行天桥的尺寸透传返回
    const res = await createBridge(params)
    return res
  }

  // 没有模型，只有纹理贴图
  if (!modelUrl && textureUrl && size) {
    // 是需要手动创建平面作为载体模型
    const texture = await loadTexture(textureUrl)
    const geo = new PlaneGeometry(size[0], size[1])
    const mat = new MeshLambertMaterial({
      color: 0xFFFFFF,
      map: texture,
      transparent: true,
      opacity: 1,
      alphaTest: 0.1,
    })
    const otherGroup = new Group()
    otherGroup.name = 'otherGroup'
    otherGroup.objectId = objectId
    otherGroup.roadId = roadId
    otherGroup.junctionId = junctionId

    const mesh = new Mesh(geo, mat)
    mesh.rotateOnAxis(axisY, PI)
    mesh.rotateOnAxis(axisX, -halfPI)
    mesh.name = 'other'
    // 路面标识的容器不设置 RenderOrder，只给网格设置 RenderOrder
    // 平面类型的地图污损，可以使用路面表示的 renderOrder
    mesh.renderOrder = RenderOrder.other
    mesh.objectId = objectId
    mesh.roadId = roadId
    mesh.junctionId = junctionId
    otherGroup.add(mesh)

    adjustOtherModel({
      modelGroup: otherGroup,
      data: otherData,
      config: configOption,
    })

    const box = new Box3()
    box.expandByObject(otherGroup)
    let width = fixedPrecision(box.max.x - box.min.x)
    const height = fixedPrecision(box.max.y - box.min.y)
    let length = fixedPrecision(box.max.z - box.min.z)
    // 如果是由于模型轴向跟自然语言理解的长宽相反，则对调两个值
    if (fixSizeObjects.includes(name)) {
      const temp = length
      length = width
      width = temp
    }

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
    // 如果是特殊处理，且可以修改尺寸的物体
    if (fixSizeObjects.includes(name) && configOption.adjustSize) {
      const temp = scaleZ
      scaleZ = scaleX
      scaleX = temp
    }
    otherGroup.scale.set(scaleX, scaleY, scaleZ)

    otherGroup.position.set(position.x, position.y, position.z)

    if (projectNormal) {
      otherGroup.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
    }
    // 道路上和路口中，都是通过调整物体看向目标点实现角度的旋转
    if (lookAtPoint) {
      const _lookAtPoint = new Vector3(
        lookAtPoint.x,
        lookAtPoint.y,
        lookAtPoint.z,
      )
      otherGroup.lookAt(_lookAtPoint)
    }

    parent.add(otherGroup)
    return {
      width,
      height,
      length,
    }
  }

  // 如果既没有指定纹理，也没有模型资源，则无法渲染
  if (!modelUrl) return

  // 加载当前信号灯对应的模型和纹理文件
  const model = await load3DModel(modelUrl)

  // 如果模型没有默认的纹理，且配置中有纹理的路径，需要手动给模型赋值纹理
  if (textureUrl) {
    const texture = await loadTexture(textureUrl)
    model.traverse((child) => {
      if (child instanceof Mesh) {
        child.material.map = texture
        child.material.needsUpdate = true
      }
    })
  }

  // 调整模型属性
  adjustOtherModel({
    modelGroup: model,
    data: otherData,
    config: configOption,
  })

  model.traverse((child) => {
    if (child instanceof Mesh) {
      // 更新对应网格的属性
      child.name = 'other'
      child.renderOrder = RenderOrder.other
      child.objectId = objectId
      child.roadId = roadId
      child.junctionId = junctionId
    }
  })
  model.name = 'otherGroup'
  model.objectId = objectId
  model.roadId = roadId
  model.junctionId = junctionId

  const box = new Box3()
  box.expandByObject(model)
  let width = fixedPrecision(box.max.x - box.min.x)
  const height = fixedPrecision(box.max.y - box.min.y)
  let length = fixedPrecision(box.max.z - box.min.z)
  // 如果是由于模型轴向跟自然语言理解的长宽相反，则对调两个值
  if (fixSizeObjects.includes(name)) {
    const temp = length
    length = width
    width = temp
  }

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
  // 如果是特殊处理，且可以修改尺寸的物体
  if (fixSizeObjects.includes(name) && configOption.adjustSize) {
    const temp = scaleZ
    scaleZ = scaleX
    scaleX = temp
  }

  model.scale.set(scaleX, scaleY, scaleZ)

  model.position.set(position.x, position.y, position.z)

  if (projectNormal) {
    // 如果存在上方向的更新，在 lookAt 属性更新前，更新 up 方向
    model.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  // 针对道路上的物体调整指向
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    model.lookAt(_lookAtPoint)
  }

  parent.add(model)

  return {
    width,
    height,
    length,
  }
}

// 移动物体
export function moveOther (params: IRenderOther) {
  const { data, parent } = params
  const {
    id: otherId,
    roadId = '',
    position,
    lookAtPoint,
    projectNormal,
  } = data
  // 从容器中找到对应其他类型物体的
  let targetOtherGroup: Group
  parent.traverse((child) => {
    if (targetOtherGroup) return
    if (
      child instanceof Group &&
      child.objectId === otherId &&
      child.name === 'otherGroup'
    ) {
      targetOtherGroup = child
    }
  })

  if (!targetOtherGroup) return

  // 道路中和路口中物体位置更新
  targetOtherGroup.position.set(position.x, position.y, position.z)

  // 仅限于需要跟随道路坡度调整的物体，才需要调整 up 方向和朝向
  if (projectNormal) {
    // 如果存在上方向，则在 lookAt 之前先调整上方向
    targetOtherGroup.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    targetOtherGroup.lookAt(_lookAtPoint)
  }
}

// 按照一定的角度旋转物体
export function rotateOther (params: IRenderOther) {
  const { data, parent } = params
  const { id: otherId, lookAtPoint } = data
  // 从容器中找到对应其他类型物体的
  let targetOtherGroup: Group
  parent.traverse((child) => {
    if (targetOtherGroup) return
    if (
      child instanceof Group &&
      child.objectId === otherId &&
      child.name === 'otherGroup'
    ) {
      targetOtherGroup = child
    }
  })

  if (!targetOtherGroup) return

  // 道路上和路口中，都是通过调整物体看向目标点实现角度的旋转
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    targetOtherGroup.lookAt(_lookAtPoint)
  }
}
