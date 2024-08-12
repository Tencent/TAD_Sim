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
import { type ISensorConfig, getSensorConfig } from '@/config/sensor'
import { RenderOrder } from '@/utils/business'
import { axisY, fixedPrecision, halfPI } from '@/utils/common3d'
import { getObject } from '@/utils/mapCache'

interface IRenderSensor {
  data: biz.ISensor
  parent: Group
}

type ISensorType = 'Camera' | 'Millimeter_Wave_Radar' | 'RSU' | 'Lidar'

// 各状态自发光强度的偏差
const sensorEmissiveIntensityOffset = {
  normal: 0,
  hovered: 0.2,
  selected: 0.01,
}
// 传感器默认的自发光贴图的强度
const sensorDefaultIntensity = {
  Camera: 0.8,
  Millimeter_Wave_Radar: 0.1,
  RSU: 0.3,
  Lidar: 0.3,
}

/**
 * 更新传感器的颜色
 * @param params
 */
export function updateSensorColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object, status, isForced = false } = params
  // 由于不同的传感器的自发光程度不同，需要根据配置中的name属性判断具体是哪一个
  // @ts-expect-error
  const objectData = getObject(object.objectId)
  if (!objectData) return

  const name = objectData.name as ISensorType
  object.traverse((child) => {
    if (child instanceof Mesh) {
      if (
        isForced ||
        child.material.emissiveIntensity !==
        sensorDefaultIntensity[name] +
        sensorEmissiveIntensityOffset.selected
      ) {
        child.material.emissiveIntensity =
          sensorDefaultIntensity[name] + sensorEmissiveIntensityOffset[status]
        child.material.needsUpdate = true
      }
    }
  })
}

// 将加载的三维模型资源做一定的调整
async function adjustSensorModel (params: {
  modelGroup: Group
  config: ISensorConfig
}) {
  const { modelGroup, config } = params
  const mesh = modelGroup.children[0] as Mesh
  if (config.name === 'RSU') {
    // RSU 模型的中心点不对，手动调整
    const box = new Box3()
    box.expandByObject(modelGroup)
    const length = fixedPrecision(box.max.x - box.min.x)
    mesh.translateX(length / 2)
  } else if (config.name === 'Lidar') {
    // 激光雷达，不需要调整角度
  } else {
    mesh.rotateZ(halfPI)
  }

  const { textureUrl, name } = config
  // 通过自发光贴图让模型更亮一些
  if (
    textureUrl &&
    mesh.material instanceof Material &&
    mesh.material instanceof MeshPhongMaterial
  ) {
    try {
      const texture = await loadTexture(textureUrl)
      if (texture) {
        // 如果加载纹理成功，则更新自发光贴图
        mesh.material.emissiveMap = texture
        // 配置中默认的自发光强度
        const defaultIntensity = sensorDefaultIntensity[name as ISensorType]
        // 自发光的强度
        const intensity = defaultIntensity || 0.2
        mesh.material.emissiveIntensity = intensity
      }
    } catch (err) {
      mesh.material.emissiveIntensity = 0.1
    }
    mesh.material.emissive = new Color(1, 1, 1)
    mesh.material.needsUpdate = true
  }
}

// 创建传感器模型
export async function createSensor (params: IRenderSensor) {
  const { data: sensorData, parent } = params
  const { id: objectId, poleId, name, position, angle } = sensorData
  const _position = new Vector3(position.x, position.y, position.z)
  const configOption = getSensorConfig(name)
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
  await adjustSensorModel({
    modelGroup: model,
    config: configOption,
  })

  model.traverse((child) => {
    if (child instanceof Mesh) {
      // 更新对应网格的属性
      child.name = 'sensor'
      child.renderOrder = RenderOrder.sensor
      child.objectId = objectId
      child.poleId = poleId
    }
  })
  model.name = 'sensorGroup'
  model.objectId = objectId
  model.poleId = poleId

  // 在模型调整完后，再检测尺寸
  const box = new Box3()
  box.expandByObject(model)
  const width = fixedPrecision(box.max.z - box.min.z)
  const height = fixedPrecision(box.max.y - box.min.y)
  const length = fixedPrecision(box.max.x - box.min.x)

  model.position.set(_position.x, _position.y, _position.z)

  // 将角度制转换成弧度制
  const rad = MathUtils.degToRad(angle)
  // 将传感器按照竖直的 Y 轴旋转一定的角度
  model.setRotationFromAxisAngle(axisY, rad)

  poleContainer.add(model)

  return {
    width,
    height,
    length,
  }
}

// 移动杆上的传感器
export function moveSensor (params: IRenderSensor) {
  const { data: sensorData, parent } = params
  const { id, position } = sensorData
  // 找到第一个属性符合的元素，也就是传感器外层的容器
  const sensorGroup = parent.getObjectByProperty('objectId', id)
  if (!sensorGroup) return

  // 只需要调整位置
  sensorGroup.position.set(position.x, position.y, position.z)
}

// 旋转杆上的传感器，是绕着竖杆调整角度
// 【TODO】 路侧设备需要设置俯仰角
export function rotateSensor (params: IRenderSensor) {
  const { data: sensorData, parent } = params
  const { id, position, angle } = sensorData
  // 找到第一个属性符合的元素，也就是传感器外层的容器
  const sensorGroup = parent.getObjectByProperty('objectId', id)
  if (!sensorGroup) return

  // 调整位置
  sensorGroup.position.set(position.x, position.y, position.z)

  // 将角度制转换成弧度制
  const rad = MathUtils.degToRad(angle)
  // 将传感器按照竖直的 Y 轴旋转一定的角度
  sensorGroup.setRotationFromAxisAngle(axisY, rad)
}
