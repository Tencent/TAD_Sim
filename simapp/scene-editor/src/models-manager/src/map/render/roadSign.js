import {
  Box3,
  DoubleSide,
  Group,
  Mesh,
  MeshBasicMaterial,
  PlaneGeometry,
  Vector2,
  Vector3,
} from 'three'
import { getTexture } from '../../utils/loader'
import {
  PI,
  axisZ,
  getPositionNormal,
  halfPI,
  setRotateByDirNormal,
} from '../utils/common'

// 常规路面标线的材质
const basicMat = new MeshBasicMaterial({
  color: 0xFFFFFF,
  transparent: true,
  opacity: 1,
  side: DoubleSide,
  alphaTest: 0.1,
})
const basicGeo = new PlaneGeometry(1, 1)

// 路面标线模型的缓存
const roadSignMap = new Map()

// 需要针对旧版编辑器兼容的路面标线名称（基本都是路口元素）
const needUpdateRoadSignName = [
  'Turn_Left_Waiting', // 左转弯待转区
]

// 创建路面标识的模型
export async function createRoadSign (options) {
  const {
    data,
    config,
    parent,
    ignoreAlt = false,
    isOldMap = false,
    ignoreSize = false,
  } = options
  const { id: objectId, yaw, x, y, z, roadid: roadId } = data
  let { width, length } = data
  const { textureUrl, basicSize, name } = config
  if (!textureUrl) return null

  let fixedYaw = Number(yaw)
  if (isOldMap && needUpdateRoadSignName.includes(name)) {
    fixedYaw += PI
  }

  const _z = ignoreAlt ? 0 : z
  let group

  if (roadSignMap.has(name)) {
    // 如果有缓存，则直接使用缓存的 clone
    group = roadSignMap.get(name).clone()
    // 使用缓存资源时，也需要兼容旧版 v1.0 公交专用车道线尺寸
    if (isOldMap && name === 'Bus_Only_Lane_Line') {
      width = 0
      length = 0
    }
  } else {
    let ratio = 1
    // 兼容旧版 tadsim v1.0 公交专用车道线的尺寸
    if (isOldMap && name === 'Bus_Only_Lane_Line') {
      width = 0
      length = 0
      ratio = 0.077
    } else {
      ratio = Number(width) / Number(length)
    }
    const size = [basicSize, basicSize / ratio]

    const srcGroup = new Group()
    srcGroup.name = 'roadSignGroup'

    const texture = await getTexture(textureUrl)
    const geo = basicGeo.clone()
    const mat = basicMat.clone()
    mat.map = texture
    mat.needsUpdate = true

    const mesh = new Mesh(geo, mat)
    mesh.rotateOnAxis(axisZ, -halfPI)
    mesh.scale.set(size[0], size[1])

    srcGroup.add(mesh)

    roadSignMap.set(name, srcGroup)
    group = srcGroup.clone()
  }

  if (!group) return null

  // 更新所有网格绑定的id
  group.objectId = objectId
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'roadSign'
      child.objectId = objectId
    }
  })

  if (!ignoreSize) {
    const box = new Box3()
    box.expandByObject(group)
    // 路面标识原始尺寸的获取
    const initWidth = box.max.y - box.min.y
    const initLength = box.max.x - box.min.x

    // 如果当前尺寸存在，则应用当前的尺寸
    let scaleX = 1
    let scaleY = 1
    if (width && length) {
      scaleX = length / initLength
      scaleY = width / initWidth
    }
    group.scale.set(scaleX, scaleY, 1)
  }

  if (
    !ignoreAlt &&
    getPositionNormal &&
    typeof getPositionNormal === 'function'
  ) {
    // 如果是常规带有坡道的道路情况，则将路面标线沿道路坡道调整角度
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
    setRotateByDirNormal(group, directionOnXY, normal)
  } else {
    // 路面标识角度调整
    group.rotateOnAxis(axisZ, fixedYaw)
  }

  // 路面标识的位置
  group.position.set(x, y, _z)

  parent.add(group)
}
