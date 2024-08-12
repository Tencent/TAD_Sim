import { Color, Group, Material, Mesh, MeshPhongMaterial } from 'three'
import { getModel, getTexture } from '../../utils/loader'
import { PI, adjustLocationByPole, axisZ, halfPI } from '../utils/common'

const trafficLightMap = new Map()

/**
 * 调整加载的三维模型资源，使之符合当前场景渲染的规则
 * @param {object} options - 包含模型组和纹理的选项
 */
function adjustLightModel (options) {
  const { modelGroup, texture } = options
  const mesh = modelGroup.children[0]
  mesh.rotateZ(halfPI)

  // 模型中网格尺寸调整为 1
  modelGroup.traverse((child) => {
    if (child instanceof Mesh) {
      child.scale.set(1, 1, 1)
    }
  })

  if (
    mesh.material instanceof Material &&
    mesh.material instanceof MeshPhongMaterial
  ) {
    if (texture) {
      // 如果纹理加载成功，则更新自发光题图
      mesh.material.emissiveMap = texture
      mesh.material.emissiveIntensity = 0.8
    } else {
      mesh.material.emissiveIntensity = 0.1
    }
    mesh.material.emissive = new Color(1, 1, 1)
    mesh.material.needsUpdate = true
  }
}

/**
 * 创建信号灯的模型
 * @param {object} params - 包含信号灯数据、配置、杆数据、父对象等参数的对象
 * @returns {Promise<Group>} - 返回一个包含信号灯模型的Promise对象
 */
export async function createTrafficLight (params) {
  const {
    data,
    config,
    poleData,
    parent,
    ignoreAlt = false,
    isOldMap = false,
  } = params
  const { id: lightId, x, y, z, yaw } = data
  const { modelUrl, textureUrl, name } = config
  if (!modelUrl || !textureUrl) return null

  const fixedYaw = isOldMap ? Number(yaw) + PI : Number(yaw)

  const _z = ignoreAlt ? z - poleData.z : z
  // 创建外层的容器，用于添加到外层的场景中
  let group

  if (trafficLightMap.has(name)) {
    group = trafficLightMap.get(name).clone()
  } else {
    const srcGroup = new Group()
    srcGroup.name = 'trafficLightGroup'

    const model = await getModel(modelUrl)
    const texture = await getTexture(textureUrl)
    // 调整信号灯模型的渲染效果
    adjustLightModel({
      modelGroup: model,
      texture,
    })

    model.rotateX(halfPI)
    srcGroup.add(model)

    trafficLightMap.set(name, srcGroup)
    group = srcGroup.clone()
  }

  if (!group) return null

  group.objectId = lightId
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'trafficLight'
      child.objectId = lightId
    }
  })

  // 基于杆的位置，调整信号灯的位置和角度
  const locationRes = adjustLocationByPole({
    data,
    poleData,
    isOldMap,
    // 往校正杆上物体的方法中，透传旧版地图问题的调整后角度
    yaw: fixedYaw,
  })
  const _yaw =
    locationRes && locationRes.matchYaw ? locationRes.matchYaw : fixedYaw
  if (locationRes && locationRes.pos) {
    const newZ = ignoreAlt ? locationRes.pos.z - poleData.z : locationRes.pos.z
    group.position.set(locationRes.pos.x, locationRes.pos.y, newZ)
  } else {
    group.position.set(x, y, _z)
  }
  group.rotateOnAxis(axisZ, _yaw)

  parent.add(group)
}
