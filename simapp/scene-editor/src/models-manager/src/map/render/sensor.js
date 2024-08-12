import { getModel, getTexture } from '../../utils/loader'
import {
  adjustLocationByPole,
  axisZ,
  fixedPrecision,
  halfPI,
} from '../utils/common'
import { Box3, Color, Group, Material, Mesh, MeshPhongMaterial } from 'three'

const sensorMap = new Map()

// 传感器的默认，需要做一定的调整
/**
 * 调整传感器模型
 * @param {object} options - 包含模型组、名称和纹理信息的对象
 */
function adjustSensorModel (options) {
  const { modelGroup, name, texture } = options
  const mesh = modelGroup.children[0]
  if (name === 'RSU') {
    // RSU 模型的中心点不对，手动调整
    const box = new Box3()
    box.expandByObject(modelGroup)
    const length = fixedPrecision(box.max.x - box.min.x)
    mesh.translateX(length / 2)
  } else {
    mesh.rotateZ(halfPI)
  }

  // 通过自发光贴图让模型更亮一些
  if (
    mesh.material instanceof Material &&
    mesh.material instanceof MeshPhongMaterial
  ) {
    if (texture) {
      // 如果加载纹理成功，则更新自发光贴图
      mesh.material.emissiveMap = texture
      // 自发光的强度
      let intensity = 0.2
      switch (name) {
        case 'Camera':
          intensity = 0.8
          break
        case 'Millimeter_Wave_Radar':
          intensity = 0.2
          break
        case 'RSU':
          intensity = 0.3
          break
        default:
          break
      }
      mesh.material.emissiveIntensity = intensity
    } else {
      mesh.material.emissiveIntensity = 0.1
    }
    mesh.material.emissive = new Color(1, 1, 1)
    mesh.material.needsUpdate = true
  }
}

/**
 * 创建传感器模型
 * @param {object} params - 包含数据、配置、杆数据、父对象等信息的对象
 * @returns {Promise<Group|null>} - 创建的传感器模型组或null
 */
export async function createSensor (params) {
  const {
    data,
    config,
    poleData,
    parent,
    ignoreAlt = false,
    isOldMap = false,
  } = params
  const { id: objectId, yaw, x, y, z } = data
  const { modelUrl, textureUrl, name } = config
  if (!modelUrl || !textureUrl) return null

  const fixedYaw = Number(yaw)
  const _z = ignoreAlt ? z - poleData.z : z
  let group

  // 用map缓存模型
  if (sensorMap.has(name)) {
    group = sensorMap.get(name).clone()
  } else {
    // 创建原始要缓存的组
    const srcGroup = new Group()
    srcGroup.name = 'sensorGroup'

    const model = await getModel(modelUrl)
    const texture = await getTexture(textureUrl)
    adjustSensorModel({
      modelGroup: model,
      name,
      texture,
    })

    model.rotateX(halfPI)
    srcGroup.add(model)

    sensorMap.set(name, srcGroup)
    group = srcGroup.clone()
  }

  if (!group) return null

  group.objectId = objectId
  // 递归给子节点添加 objectId和name
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'sensor'
      child.objectId = objectId
    }
  })

  // 基于杆的位置，调整传感器的位置和角度
  const locationRes = adjustLocationByPole({
    data,
    poleData,
    isOldMap,
    yaw: fixedYaw,
  })
  const _yaw = locationRes && locationRes.matchYaw ?
    locationRes.matchYaw :
    fixedYaw
  if (locationRes && locationRes.pos) {
    const newZ = ignoreAlt ? locationRes.pos.z - poleData.z : locationRes.pos.z
    group.position.set(locationRes.pos.x, locationRes.pos.y, newZ)
  } else {
    group.position.set(x, y, _z)
  }
  group.rotateOnAxis(axisZ, _yaw)

  parent.add(group)
}
