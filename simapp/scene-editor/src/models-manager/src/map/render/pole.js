import {
  Color,
  CylinderGeometry,
  Group,
  Mesh,
  MeshLambertMaterial,
  SphereGeometry,
} from 'three'
import { getPoleConfig } from '../config'
import { PI, axisZ, halfPI } from '../utils/common'

const poleMat = new MeshLambertMaterial({
  color: new Color(0.8, 0.8, 0.8),
})

// 杆模型的缓存
const poleMap = new Map()

// 创建竖杆的模型
function createVerticalPole (name) {
  const config = getPoleConfig(name)
  if (!config) return null
  const { vRadius, height, segment } = config.style

  // 外层容器
  const poleGroup = new Group()
  poleGroup.name = 'verticalPoleGroup'
  // 顶部半圆球
  const topGeo = new SphereGeometry(
    vRadius,
    segment,
    segment,
    0,
    PI * 2,
    0,
    halfPI,
  )

  // 杆体圆柱
  const verticalGeo = new CylinderGeometry(
    vRadius,
    vRadius,
    height,
    segment,
    1,
    true,
  )

  const verticalMesh = new Mesh(verticalGeo, poleMat)
  verticalMesh.position.set(0, height / 2, 0)
  verticalMesh.name = 'verticalPole'
  poleGroup.add(verticalMesh)

  const topMesh = new Mesh(topGeo, poleMat)
  topMesh.position.set(0, height - 0.01, 0)
  topMesh.name = 'verticalPole'
  poleGroup.add(topMesh)

  return poleGroup
}

// 创建横杆的模型
function createCrossPole (name) {
  const config = getPoleConfig(name)
  if (!config) return null
  const { vRadius, hRadius, height, length, segment } = config.style

  // 外层容器
  const poleGroup = new Group()
  poleGroup.name = 'crossPoleGroup'

  // 顶部半圆球
  const topGeo = new SphereGeometry(
    vRadius,
    segment,
    segment,
    0,
    PI * 2,
    0,
    halfPI,
  )

  // 杆体圆柱
  const verticalPole = new CylinderGeometry(
    vRadius,
    vRadius,
    height,
    segment,
    1,
    true,
  )
  // 水平延伸的部分
  const horizontalGeo = new CylinderGeometry(hRadius, hRadius, length, segment)

  // 顶部半圆球（不作为交互的区域）
  const topMesh = new Mesh(topGeo, poleMat)
  topMesh.position.set(0, height - 0.01, 0)
  topMesh.name = 'crossPole'
  poleGroup.add(topMesh)

  // 竖直部分的杆体
  const verticalMesh = new Mesh(verticalPole, poleMat)
  verticalMesh.position.set(0, height / 2, 0)
  verticalMesh.name = 'crossPole'
  poleGroup.add(verticalMesh)

  // 水平延伸部分的杆体
  const horizontalMesh = new Mesh(horizontalGeo, poleMat)
  // TODO 这个角度后续可能需要调整
  horizontalMesh
    .translateY(height - hRadius)
    .translateZ(length / 2 + vRadius)
    .rotateX(halfPI)
  horizontalMesh.name = 'crossPole'
  poleGroup.add(horizontalMesh)

  return poleGroup
}

// 创建杆的模型
export async function createPole (params) {
  const { data, parent, ignoreAlt = false, isOldMap = false } = params
  const { id: poleId, name, x, y, z, yaw } = data

  const _yaw = isOldMap ? Number(yaw) + halfPI : Number(yaw)

  const _z = ignoreAlt ? 0 : z
  let group

  if (poleMap.has(name)) {
    // 如果有缓存，则直接使用缓存的clone
    group = poleMap.get(name).clone()
  } else {
    // 创建要缓存的原始对象
    const srcGroup = new Group()
    srcGroup.name = 'poleGroup'

    let pole
    if (name === 'Cross_Pole' || name === 'Cantilever_Pole') {
      // 横杆
      pole = createCrossPole(name)
    } else {
      // 竖杆
      pole = createVerticalPole(name)
    }

    if (pole) {
      pole.rotateX(halfPI)
      srcGroup.add(pole)
    }

    // 第一次创建模型，则缓存到映射表中
    poleMap.set(name, srcGroup)

    // 使用的是源对象的 clone
    group = srcGroup.clone()
  }

  if (!group) return null

  group.objectId = poleId
  // 给杆中所有的网格绑定 id
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.objectId = poleId
    }
  })

  // 杆模型的位置和角度
  group.position.set(x, y, _z)
  group.rotateOnAxis(axisZ, _yaw + halfPI)

  parent.add(group)
}
