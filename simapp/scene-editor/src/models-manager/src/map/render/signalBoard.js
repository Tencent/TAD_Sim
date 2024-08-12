import { DoubleSide, Group, Mesh, MeshBasicMaterial } from 'three'
import { getModel, getTexture } from '../../utils/loader'
import { PI, adjustLocationByPole, axisZ, halfPI } from '../utils/common'

// 标志牌的缓存
const signalBoardMap = new Map()
const basicMat = new MeshBasicMaterial({
  color: 0xFFFFFF,
  side: DoubleSide,
})
const backMat = new MeshBasicMaterial({
  color: 0xDFDFDF,
  side: DoubleSide,
})

// 克隆一份标志牌模型的备份
function cloneSignalBoardModelBackPart (modelGroup) {
  const mesh = modelGroup.children[0]
  const backMesh = mesh.clone()
  // 更新背面网格的材质
  backMesh.material = backMat
  backMesh.material.needsUpdate = true

  // 调整背面网格的距离
  backMesh.translateX(-0.01)
  modelGroup.add(backMesh)
}

// 创建标志牌
export async function createSignalBoard (params) {
  const {
    data,
    config,
    poleData,
    parent,
    ignoreAlt = false,
    isOldMap = false,
  } = params
  const { id: objectId, x, y, z, yaw } = data
  const { modelUrl, textureUrl, name } = config
  if (!modelUrl || !textureUrl) return null

  const fixedYaw = isOldMap ? Number(yaw) + PI : Number(yaw)

  const _z = ignoreAlt ? z - poleData.z : z
  // 外层的容器
  let group

  if (signalBoardMap.has(name)) {
    group = signalBoardMap.get(name).clone()
  } else {
    // 创建原始要缓存的组
    const srcGroup = new Group()
    srcGroup.name = 'signalBoardGroup'

    const model = await getModel(modelUrl)
    const texture = await getTexture(textureUrl)
    const mat = basicMat.clone()
    mat.map = texture
    mat.needsUpdate = true

    model.traverse((child) => {
      if (child instanceof Mesh) {
        child.material = mat
        child.material.needsUpdate = true
      }
    })
    // 在完成纹理赋值后，更新标志牌背面的面板
    cloneSignalBoardModelBackPart(model)

    model.rotateX(halfPI)
    srcGroup.add(model)

    // 第一次创建模型，缓存到映射表中
    signalBoardMap.set(name, srcGroup)

    // 使用的是源对象的 clone
    group = srcGroup.clone()
  }

  if (!group) return null

  group.objectId = objectId
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'signalBoard'
      child.objectId = objectId
    }
  })

  // 基于杆的位置，调整标志牌的位置和角度
  const locationRes = adjustLocationByPole({
    data,
    poleData,
    isOldMap,
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
