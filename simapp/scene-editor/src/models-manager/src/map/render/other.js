import {
  Box3,
  DoubleSide,
  Group,
  Mesh,
  MeshBasicMaterial,
  MeshLambertMaterial,
  MeshPhongMaterial,
  PlaneGeometry,
  Vector2,
  Vector3,
} from 'three'
import { getModel, getTexture } from '../../utils/loader'
import {
  axisZ,
  fixedPrecision,
  getPositionNormal,
  halfPI,
  setRotateByDirNormal,
} from '../utils/common'
import { ModelUrls } from '../utils/urls'

// 模型长和宽的朝向跟自然语言理解相反的模型，需要手动调整对应的长和宽
const fixSizeObjects = [
  'Deceleration_Zone', // 减速带
  'Ground_Lock', // 地锁
  'Plastic_Vehicle_Stopper', // 塑胶挡车器
  'Parking_Limit_Position_Pole_2m', // U型挡车器
  'Support_Vehicle_Stopper', // 支撑型挡车器
  'Building', // 居民楼
  'BusStation', // 公交车站
  'Traffic_Barrier', // 交通护栏
  'Lamp', // 路灯
  'Traffic_Horse', // 水马
  'GarbageCan', // 垃圾桶
]
const fixScaleObjects = [
  'Plastic_Vehicle_Stopper', // 塑胶挡车器
  'Parking_Limit_Position_Pole_2m', // U型挡车器
  'Support_Vehicle_Stopper', // 支撑型挡车器
  'Deceleration_Zone', // 减速带
  'Shrub', // 灌木
  'Traffic_Barrier', // 交通护栏
  'Lamp', // 路灯
  'Traffic_Horse', // 水马
  'GarbageCan', // 垃圾桶
]

// 为了更好的渲染效果，需要替换材质的物体名称
const replaceMaterialObjects = {
  // 地图污损类
  // 交通管理类
  Well_Cover: 0.05,
  Parking_Hole: 1,
  Parking_Lot: 0.1,
  Ground_Lock: 0.1,
  Plastic_Vehicle_Stopper: 0.1,
  Parking_Limit_Position_Pole_2m: 0.1,
  Support_Vehicle_Stopper: 0.1,
  Charging_Station: 0.3,
  Lamp: 0.1,
  Traffic_Barrier: 0.05,
  Road_Curb: 0.2,
  Reflective_Road_Sign: 0.8,
  Traffic_Cone: 1,
  Traffic_Horse: 2,
  Obstacle: 0.5,
  GarbageCan: 1,
  // 植被类
  Tree: 2,
  Shrub: 0.3,
  Grass: 1,
  // 建筑类
  Building: 0.2,
  BusStation: 0.3,
  // 天桥类
  PedestrianBridge: 0.5,
}

const otherMap = new Map()
const basicGeo = new PlaneGeometry(1, 1)
const basicMat = new MeshBasicMaterial({
  color: 0xFFFFFF,
  transparent: true,
  opacity: 1,
  side: DoubleSide,
  alphaTest: 0.1,
})

// 需要适配的其他类型物体的名称
const needUpdateOtherName = [
  'Protrusion', // 凸起
  'Reflective_Road_Sign', // 反光路标
  'Parking_Hole', // 停车杆
  'Ground_Lock', // 地锁
  'Plastic_Vehicle_Stopper', // 塑胶挡车器
  'Parking_Limit_Position_Pole_2m', // U型挡车器
  'Support_Vehicle_Stopper', // 支撑型挡车器
  'Charging_Station', // 充电桩
  'Tree', // 树木
  'Building', // 居民楼
]

// 调整模型的形状
function adjustOtherModelShape (options) {
  const { modelGroup, name } = options
  modelGroup.traverse((child) => {
    if (child instanceof Mesh) {
      // 尺寸相关的调整
      if (name === 'Protrusion') {
        child.scale.set(1, 1, 1)
        // 凸起模型圆心在几何中心，而不是地面中心，手动调整
        const box = new Box3()
        box.expandByObject(modelGroup)
        const height = box.max.y - box.min.y
        child.position.set(child.position.x, height / 2, child.position.z)
      } else if (name === 'Reflective_Road_Sign') {
        // 原始模型太大了，将 Mesh 的尺寸默认调整为 1
        child.scale.set(1, 1, 1)
      } else if (
        name === 'BusStation' ||
        name === 'Traffic_Barrier' ||
        name === 'Lamp' ||
        name === 'Traffic_Horse' ||
        name === 'GarbageCan'
      ) {
        // 部分模型需要调整90°方向
        child.rotateZ(halfPI)
      }
    }
  })
}

// 调整模型的材质
function adjustOtherModelMaterial (options) {
  const { modelGroup, name: objectName } = options
  modelGroup.traverse((child) => {
    if (child instanceof Mesh) {
      // 材质相关的调整
      const emissiveIntensity = replaceMaterialObjects[objectName]
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
              const {
                map,
                transparent,
                depthTest,
                depthWrite,
                name,
              } = originMat
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
async function createBridge (params) {
  const { data, config, parent, ignoreAlt = false } = params
  const { id: objectId, x, y, z, yaw, userData = [], roadid: roadId } = data

  const fixedYaw = Number(yaw)
  const _z = ignoreAlt ? 0 : z

  let span = 18
  for (const _userData of userData) {
    // 如果透传的数据中有天桥的跨度
    const _span = _userData.bridge_span
    if (_span) {
      span = _span
    }
  }

  // 天桥模型有三个部分组成
  // 每一个部分的静态资源路径
  const urls = [
    ModelUrls.other.pedestrianBridge,
    ModelUrls.other.pedestrianBridge1,
    ModelUrls.other.pedestrianBridge2,
  ]

  // 每一个部分在缓存中的命名
  const modelName = [
    'PedestrianBridge0',
    'PedestrianBridge1',
    'PedestrianBridge2',
  ]

  const loadModelPromises = []
  for (const index in modelName) {
    const name = modelName[index]
    if (otherMap.has(name)) {
      loadModelPromises.push(Promise.resolve(otherMap.get(name).clone()))
    } else {
      const url = urls[index]
      loadModelPromises.push(
        (() => {
          return new Promise((resolve) => {
            getModel(url).then((data) => {
              otherMap.set(name, data)
              resolve(data.clone())
            })
          })
        })(),
      )
    }
  }

  const models = await Promise.all(loadModelPromises)

  // 计算中间部分模型的尺寸
  const centerBox = new Box3()
  centerBox.expandByObject(models[0])
  const centerW = fixedPrecision(centerBox.max.x - centerBox.min.x)

  // 容器
  const bridgeGroup = new Group()
  // 下坡模型的宽度
  const slopeHalfWidth = 2.45
  // 中间部分离地高度
  const heightOffset = 9.98

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
    const _model = models[0].clone()
    _model.scale.set(perWidth / centerW, 1, 1)
    // 中间部分的偏移量
    _model.translateX(perWidth * i - perWidth * (count / 2 - 1 / 2))
    // 中间部分抬离地面
    _model.translateZ(heightOffset)
    _model.rotateX(halfPI)
    centerModels.push(_model)
  }

  // 左右两侧坡道的偏移
  models[1].translateX(-(span / 2 + slopeHalfWidth))
  models[1].rotateX(halfPI)
  models[2].translateX(span / 2 + slopeHalfWidth)
  models[2].rotateX(halfPI)

  bridgeGroup.add(...centerModels, models[1], models[2])

  // 更新模型的材质
  adjustOtherModelMaterial({
    modelGroup: bridgeGroup,
    name: config.name,
  })

  bridgeGroup.traverse((child) => {
    if (child instanceof Mesh) {
      // 更新对应网格的属性
      child.name = 'other'
      child.objectId = objectId
    }
  })
  bridgeGroup.objectId = objectId
  bridgeGroup.position.set(x, y, _z)

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
      Math.cos(fixedYaw + halfPI),
      Math.sin(fixedYaw + halfPI),
    ).normalize()
    setRotateByDirNormal(bridgeGroup, directionOnXY, normal)
  } else {
    // 如果没有跟随坡度调整的方法，则使用默认的角度调整
    bridgeGroup.rotateOnAxis(axisZ, fixedYaw + halfPI)
  }

  parent.add(bridgeGroup)
}

// 创建其他类型的物体
export async function createOther (params) {
  const {
    data,
    config,
    parent,
    ignoreAlt = false,
    isOldMap = false,
    ignoreSize = false,
  } = params
  const {
    id: objectId,
    length: originLength,
    width: originWidth,
    height,
    x,
    y,
    z,
    yaw,
    roadid: roadId,
  } = data
  const { modelUrl = '', textureUrl = '', name } = config

  // 如果是人行天桥，需要特殊处理
  if (modelUrl === ModelUrls.other.pedestrianBridge) {
    await createBridge(params)
    return
  }

  let length = originLength
  let width = originWidth
  if (name === 'Building' || name === 'BusStation') {
    // 长宽异常情况，需要手动调整
    if (width > length) {
      const temp = length
      length = width
      width = temp
    }
  }

  let fixedYaw = Number(yaw)
  if (isOldMap && needUpdateOtherName.includes(name)) {
    fixedYaw += halfPI
  }
  const _z = ignoreAlt ? 0 : z

  let group

  if (otherMap.has(name)) {
    group = otherMap.get(name).clone()
    // 如果没有模型，只有纹理贴图
    if (!modelUrl && textureUrl) {
      group.objectId = objectId
      group.traverse((child) => {
        if (child instanceof Mesh) {
          child.name = 'other'
          child.objectId = objectId
        }
      })
      if (!ignoreSize) {
        adjustSize({
          group,
          name,
          width,
          length,
          height,
        })
      }

      // 设置 otherGroup 的位置和角度
      group.position.set(x, y, _z)
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
        setRotateByDirNormal(group, directionOnXY, normal)
      } else {
        // 角度
        group.rotateOnAxis(axisZ, fixedYaw)
      }

      parent.add(group)
      return
    }
  } else {
    // 没有模型，只有纹理贴图
    if (!modelUrl && textureUrl) {
      const srcGroup = new Group()
      srcGroup.name = 'otherGroup'

      const ratio = Number(width) / Number(length)
      const size = [config.basicSize, config.basicSize / ratio]

      const texture = await getTexture(textureUrl)
      const geo = basicGeo.clone()
      const mat = basicMat.clone()
      mat.map = texture
      mat.needsUpdate = true

      const mesh = new Mesh(geo, mat)
      mesh.rotateOnAxis(axisZ, -halfPI)
      mesh.scale.set(size[0], size[1])
      srcGroup.add(mesh)

      otherMap.set(name, srcGroup)
      group = srcGroup.clone()

      group.objectId = objectId
      group.traverse((child) => {
        if (child instanceof Mesh) {
          child.name = 'other'
          child.objectId = objectId
        }
      })

      if (!ignoreSize) {
        adjustSize({
          group,
          name,
          width,
          length,
          height,
        })
      }

      // 设置 otherGroup 的位置和角度
      group.position.set(x, y, _z)
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
        setRotateByDirNormal(group, directionOnXY, normal)
      } else {
        // 角度
        group.rotateOnAxis(axisZ, fixedYaw)
      }

      parent.add(group)
      return
    }

    // 如果既没有指定纹理，也没有模型资源，则无法渲然
    if (!modelUrl) return null

    const srcGroup = new Group()
    srcGroup.name = 'otherGroup'

    const model = await getModel(modelUrl)
    let texture
    if (textureUrl) {
      texture = await getTexture(textureUrl)
      model.traverse((child) => {
        if (child instanceof Mesh) {
          child.material.map = texture
          child.material.needsUpdate = true
        }
      })
    }
    // 调整模型的属性
    adjustOtherModelShape({
      modelGroup: model,
      name,
    })
    adjustOtherModelMaterial({
      modelGroup: model,
      name,
    })

    model.traverse((child) => {
      if (child instanceof Mesh) {
        // 更新对应网格的属性
        child.name = 'other'
        child.objectId = objectId
      }
    })
    model.rotateX(halfPI)
    srcGroup.add(model)

    otherMap.set(name, srcGroup)
    group = srcGroup.clone()
  }
  group.objectId = objectId
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'other'
      child.objectId = objectId
    }
  })

  if (!ignoreSize) {
    adjustSize({
      group,
      name,
      width,
      length,
      height,
    })
  }

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
      Math.cos(fixedYaw + halfPI),
      Math.sin(fixedYaw + halfPI),
    ).normalize()
    setRotateByDirNormal(group, directionOnXY, normal)
  } else {
    // 角度
    group.rotateOnAxis(axisZ, fixedYaw + halfPI)
  }

  // 设置物体的位置
  group.position.set(x, y, _z)

  parent.add(group)
}

// 调整物体的尺寸
function adjustSize (params) {
  const { group, name, width, length, height } = params
  // 获取模型原始尺寸
  const box = new Box3()
  box.expandByObject(group)
  let initLength = box.max.x - box.min.x
  let initWidth = box.max.y - box.min.y
  const initHeight = box.max.z - box.min.z
  // 如果是由于模型轴向跟自然语言理解的长宽相反，则对调两个值
  if (fixSizeObjects.includes(name)) {
    const temp = initLength
    initLength = initWidth
    initWidth = temp
  }

  // 如果当前尺寸存在，则应用当前的尺寸
  let scaleX = 1
  let scaleY = 1
  let scaleZ = 1
  if (length && initLength) {
    scaleX = length / initLength
  }
  if (width && initWidth) {
    scaleY = width / initWidth
  }
  if (height && initHeight) {
    scaleZ = height / initHeight
  }
  // 如果是特殊处理，且可以修改尺寸的物体
  if (fixScaleObjects.includes(name)) {
    const temp = scaleY
    scaleY = scaleX
    scaleX = temp
  }
  group.scale.set(scaleX, scaleY, scaleZ)
}
