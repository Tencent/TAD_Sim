import {
  Box3,
  Color,
  Material,
  MathUtils,
  Mesh,
  MeshPhongMaterial,
  Vector3,
} from 'three'
import type {
  Group,
  Object3D,
} from 'three'
import { load3DModel, loadTexture } from '../loaders'
import { cloneOriginMaterial } from './object'
import {
  type ITrafficLightConfig,
  getTrafficLightConfig,
} from '@/config/trafficLight'
import { RenderOrder } from '@/utils/business'
import { axisY, fixedPrecision, halfPI } from '@/utils/common3d'

interface IRenderTrafficLight {
  data: biz.ITrafficLight
  parent: Group
}

// 信号灯自发光贴图强度
const trafficLightEmissiveIntensity = {
  normal: 0.6,
  hovered: 1.2,
  selected: 0.61,
}

/**
 * 更新标志牌的颜色（标志牌更新自发光贴图的强度）
 */
export function updateTrafficLightColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object, status, isForced = false } = params
  object.traverse((child) => {
    if (child instanceof Mesh) {
      if (
        isForced ||
        child.material.emissiveIntensity !==
        trafficLightEmissiveIntensity.selected
      ) {
        child.material.emissiveIntensity = trafficLightEmissiveIntensity[status]
        child.material.needsUpdate = true
      }
    }
  })
}

// 将加载的三维模型资源做一定地调整，使之符合当前场景渲染的规则
async function adjustLightModel (
  modelGroup: Group,
  config: ITrafficLightConfig,
) {
  const { textureUrl } = config

  const mesh = modelGroup.children[0] as Mesh
  // 调整内部网格的角度
  mesh.rotateZ(halfPI)

  // 模型中网格尺寸调整为 1
  modelGroup.traverse((child) => {
    if (child instanceof Mesh) {
      child.scale.set(1, 1, 1)
    }
  })

  // 通过自发光贴图让交通灯模型更亮一些（指信号灯彩色的部分，黑色本来还是黑的没办法）
  if (
    mesh.material instanceof Material &&
    mesh.material instanceof MeshPhongMaterial
  ) {
    try {
      const texture = await loadTexture(textureUrl)
      if (texture) {
        // 如果加载纹理成功，则更新自发光贴图
        mesh.material.emissiveMap = texture
        mesh.material.emissiveIntensity = 0.8
      }
    } catch (err) {
      mesh.material.emissiveIntensity = 0.1
    }
    mesh.material.emissive = new Color(1, 1, 1)
    mesh.material.needsUpdate = true
  }
}

// 渲染交通信号灯
export async function createTrafficLight (params: IRenderTrafficLight) {
  const { data: trafficLightData, parent } = params
  const {
    id: objectId,
    poleId,
    position,
    angle,
    type,
    subtype,
  } = trafficLightData
  const _position = new Vector3(position.x, position.y, position.z)
  const configOption = getTrafficLightConfig(type, subtype)
  if (!configOption) return
  // 从外层容器中，找对应的 pole
  const poleContainer = parent.getObjectByProperty('objectId', poleId)
  if (!poleContainer) return

  const { modelUrl } = configOption
  if (!modelUrl) return
  // 加载当前信号灯对应的模型和纹理文件
  const model = await load3DModel(modelUrl)

  // 由于是直接加载 fbx 文件中对应的贴图材质，缓存模型后需要 clone 处理
  cloneOriginMaterial(model)

  // 调整模型自身网格属性
  await adjustLightModel(model, configOption)

  model.traverse((child) => {
    if (child instanceof Mesh) {
      // 更新对应网格的属性
      child.name = 'trafficLight'
      child.renderOrder = RenderOrder.trafficLight
      child.objectId = objectId
      child.poleId = poleId
    }
  })
  model.name = 'trafficLightGroup'
  model.objectId = objectId
  model.poleId = poleId

  const box = new Box3()
  box.expandByObject(model)
  const width = fixedPrecision(box.max.z - box.min.z)
  const height = fixedPrecision(box.max.y - box.min.y)
  const length = fixedPrecision(box.max.x - box.min.x)

  model.position.set(_position.x, _position.y, _position.z)

  // 将角度制转换成弧度制
  const rad = MathUtils.degToRad(angle)
  // 将信号灯按照竖直的 Y 轴旋转一定的角度
  model.setRotationFromAxisAngle(axisY, rad)

  poleContainer.add(model)

  return {
    width,
    height,
    length,
  }
}

// 移动杆上的交通灯
export function moveTrafficLight (params: IRenderTrafficLight) {
  const { data: trafficLightData, parent } = params
  const { id, position } = trafficLightData
  // 找到第一个属性符合的元素，也就是信号灯外层的容器
  const trafficLightGroup = parent.getObjectByProperty('objectId', id)
  if (!trafficLightGroup) return

  // 只需要调整位置
  trafficLightGroup.position.set(position.x, position.y, position.z)
}

// 旋转杆上的交通信号灯，是绕着竖杆调整角度
export function rotateTrafficLight (params: IRenderTrafficLight) {
  const { data: trafficLight, parent } = params
  const { id, position, angle } = trafficLight
  // 找到第一个属性符合的元素，也就是交通灯外层的容器
  const trafficLightGroup = parent.getObjectByProperty('objectId', id)
  if (!trafficLightGroup) return

  // 调整位置
  trafficLightGroup.position.set(position.x, position.y, position.z)

  // 将角度制转换成弧度制
  const rad = MathUtils.degToRad(angle)
  // 将信号灯按照竖直的 Y 轴旋转一定的角度
  trafficLightGroup.setRotationFromAxisAngle(axisY, rad)
}
