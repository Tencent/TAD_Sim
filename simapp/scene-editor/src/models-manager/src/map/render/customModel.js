import { Box3, Group, Mesh, Vector2, Vector3 } from 'three'
import { getModel } from '../../utils/loader'
import {
  axisZ,
  getPositionNormal,
  halfPI,
  setRotateByDirNormal,
} from '../utils/common'

const customModelMap = new Map()

// 创建自定义导入模型的物体
export async function createCustomModel (params) {
  const { data, config, parent, ignoreAlt = false } = params

  const {
    id: objectId,
    length,
    width,
    height,
    x,
    y,
    z,
    yaw,
    roadid: roadId,
    name,
  } = data

  const { modelUrl, posX, posY, posZ } = config
  const fixedYaw = Number(yaw)
  const _z = ignoreAlt ? 0 : z

  let group
  if (customModelMap.has(name)) {
    // 如果有缓存
    group = customModelMap.get(name).clone()
  } else {
    const srcGroup = new Group()
    srcGroup.name = 'customModelProxyGroup'

    const model = await getModel(modelUrl)
    model.rotateX(halfPI)
    srcGroup.add(model)

    customModelMap.set(name, srcGroup)
    group = srcGroup.clone()
  }

  group.objectId = objectId
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'customModel'
      child.objectId = objectId
    }
  })

  // 由于需要应用自定义模型的原点坐标偏移，嵌套2层Group
  const wrapGroup = new Group()
  wrapGroup.name = 'customModelGroup'
  wrapGroup.objectId = objectId

  // 基于数据中的 length，width，height 调整物体尺寸
  const { x: scaleX, y: scaleY, z: scaleZ } = getAdjustSize({
    group,
    width,
    length,
    height,
  })

  // 应用原点坐标偏移量
  group.position.set(posY, -posX, posZ)
  wrapGroup.add(group)
  wrapGroup.scale.set(scaleX, scaleY, scaleZ)

  if (
    !ignoreAlt &&
    getPositionNormal &&
    typeof getPositionNormal === 'function'
  ) {
    // 如果是常规带有坡道的道路情况，则将物体沿道路坡道调整角度
    const _p = new Vector3(x, y, z)
    let normal
    if (roadId) {
      // 如果是放置在道路上的元素，需要在请求当前位置法向量时，透传物体对应的 roadId
      normal = getPositionNormal(_p, roadId)
    } else {
      normal = getPositionNormal(_p)
    }
    const directionOnXY = new Vector2(
      Math.cos(fixedYaw),
      Math.sin(fixedYaw),
    ).normalize()
    setRotateByDirNormal(wrapGroup, directionOnXY, normal)
  } else {
    // 角度
    wrapGroup.rotateOnAxis(axisZ, fixedYaw)
  }

  // 设置物体的位置
  wrapGroup.position.set(x, y, _z)

  parent.add(wrapGroup)
}

// 调整物体的尺寸
function getAdjustSize (params) {
  const { group, width, length, height } = params
  // 获取模型原始尺寸
  const box = new Box3()
  box.expandByObject(group)

  // 由于场景编辑器和地图编辑器坐标系的差别，length 和 width 属性对应的坐标轴不一样
  const initWidth = box.max.x - box.min.x
  const initLength = box.max.y - box.min.y
  const initHeight = box.max.z - box.min.z

  // 如果当前尺寸存在，则应用当前的尺寸
  let scaleX = 1
  let scaleY = 1
  let scaleZ = 1
  if (width && initWidth) {
    scaleX = width / initWidth
  }
  if (length && initLength) {
    scaleY = length / initLength
  }
  if (height && initHeight) {
    scaleZ = height / initHeight
  }

  return {
    x: scaleX,
    y: scaleY,
    z: scaleZ,
  }
}
