import { isArray, isEmpty, isFunction, memoize } from 'lodash-es'
import { signlightArrowImgs, signlightURLs } from './scene/constants'
import { fileURLToPath } from './utils/loader'
import { findCarInCatalog, findPedestrianInCatalog } from './catalogs/utils'
import {
  BoxGeometry,
  BufferGeometry,
  DoubleSide,
  Float32BufferAttribute,
  Group,
  Mesh,
  MeshBasicMaterial,
  MeshLambertMaterial,
  PlaneGeometry,
  TextureLoader,
} from 'three'
import { FBXLoader } from 'three/examples/jsm/loaders/FBXLoader'

const LinearEncoding = 3000
const sRGBEncoding = 3001

const halfPI = Math.PI / 2
// fbx加载器
const fbxLoader = new FBXLoader()
// 统一模型加载方法
const loadModel = memoize(
  url => new Promise((resolve, reject) => {
    fbxLoader.load(url, resolve, undefined, reject)
  }).then((mesh) => {
    mesh.rotation.x = halfPI
    return mesh
  }),
)
// 纹理加载器
const textureLoader = new TextureLoader()
// 统一纹理加载方法
const loadTexture = memoize(url => new Promise((resolve, reject) => {
  textureLoader.load(url, resolve, undefined, reject)
}))
// 交通灯箭头
const signArrowGeometry = new PlaneGeometry(1.5, 3)

/**
 * 加载一个箭头，用于信控配置
 * @param {string} textureUrl - 箭头纹理的URL
 * @returns {Promise<object>} - 返回一个包含材质和透明材质的对象
 */
async function _loadSignlightArrow (textureUrl) {
  const texture = await loadTexture(textureUrl)
  const material = new MeshBasicMaterial({
    map: texture,
    side: DoubleSide,
  })
  const transparentMaterial = material.clone()
  transparentMaterial.opacity = 0.5
  return {
    material,
    transparentMaterial,
  }
}

const loadSignArrow = memoize(_loadSignlightArrow)
// 盒子共享几何体（单位立方体，按真实包围盒尺寸缩放）
const boxGeo = new BoxGeometry(1, 1, 1)

// 播放(player)模式下所有 actor 都渲染为「按真实包围盒尺寸缩放」的彩色盒子，并按类型着色以便区分
const BOX_COLORS = {
  car: 0x16D1F3, // 青蓝 - 车辆
  pedestrian: 0xDEE048, // 黄 - 动态障碍物如行人、自行车
  obstacle: 0x615E66, // 灰色 - 静态障碍物
}
// 共享几何体，按颜色生成各类型盒子网格
function createColoredBox (color) {
  return new Mesh(boxGeo, new MeshLambertMaterial({ color, side: DoubleSide }))
}
const boxMeshCar = createColoredBox(BOX_COLORS.car)
const boxMeshPedestrian = createColoredBox(BOX_COLORS.pedestrian)
const boxMeshObstacle = createColoredBox(BOX_COLORS.obstacle)

// 朝向指示箭头：共享几何体与材质（性能优先，所有 box 复用同一份资源，
// 每个实例只创建一个轻量 Mesh 并设置 scale/position，不增加每帧开销）
// 单位尺寸、位于 XY 平面、指向 +X（即 box 的车头/朝向方向）的扁平三角形
const headingArrowGeo = new BufferGeometry()
headingArrowGeo.setAttribute('position', new Float32BufferAttribute([
  0.45, 0, 0, // 箭头尖端，指向前方 +X
  -0.25, 0.28, 0, // 左后角
  -0.25, -0.28, 0, // 右后角
], 3))
headingArrowGeo.computeVertexNormals()
// 白色、双面、不受光照（MeshBasic）以便在各种颜色 box 上都清晰可见
const headingArrowMat = new MeshBasicMaterial({ color: 0xFFFFFF, side: DoubleSide })

/**
 * 创建一个指示 box 朝向的扁平箭头（指向 +X，即 box 前向），贴在 box 顶面之上。
 * 复用共享几何体与材质，仅按各自包围盒尺寸设置 scale 与位置，性能开销极低。
 * @param {object} boundingBox - 含 center 与 dimensions(length/width/height)
 * @returns {Mesh}
 */
function createHeadingArrow (boundingBox) {
  const { center = { x: 0, y: 0, z: 0 }, dimensions } = boundingBox || {}
  const length = +dimensions?.length > 0 ? +dimensions.length : 1
  const width = +dimensions?.width > 0 ? +dimensions.width : 1
  const height = +dimensions?.height > 0 ? +dimensions.height : 1
  const arrow = new Mesh(headingArrowGeo, headingArrowMat)
  // 扁平三角形按 box 的长宽缩放（z 不缩放），尺寸与 box 顶面匹配
  arrow.scale.set(length, width, 1)
  // 放在顶面略上方，避免与顶面 z-fighting
  arrow.position.set(+center.x, +center.y, +center.z + height / 2 + 0.05)
  arrow.renderOrder = 1
  return arrow
}

/**
 * 修复材质编码
 * @param {object} object - 包含材质的对象
 */
// function fixMaterialEncoding (object) {
  // if (object.material) {
  //   if (isArray(object.material)) {
  //     object.material.forEach((m) => {
  //       if (m.map && m.map.encoding === sRGBEncoding) {
  //         m.map.encoding = LinearEncoding
  //       }
  //     })
  //   } else {
  //     if (object.material.map && object.material.map.encoding === sRGBEncoding) {
  //       object.material.map.encoding = LinearEncoding
  //     }
  //   }
  // }
  // if (isArray(object.children)) {
  //   object.children.forEach(fixMaterialEncoding)
  // }
// }

/**
 * 设置模型透明度
 * @param {Group|Mesh|Object3D} model - 要设置透明度的模型
 * @param {boolean} [transparent] - 是否透明
 */
function setTransparent (model, transparent = true) {
  if (!model) return
  // mesh.clone为浅复制。一些地方修改了material后，所有model都会受到影响。所以加此语句。其他地方的clone必要时也需要加以下。
  model.traverse((child) => {
    if (child.isMesh) {
      if (Array.isArray(child.material)) {
        child.material = child.material.map(material => material.clone())
      } else {
        child.material = child.material.clone()
      }
    }
  })
  if (model?.isMesh) {
    if (Array.isArray(model.material)) {
      model.material.forEach((ma) => {
        ma.transparent = true
        ma.opacity = transparent ? Math.min(0.3, ma.opacity) : 1
      })
    } else {
      model.material.transparent = true
      model.material.opacity = transparent ? Math.min(0.3, model.material.opacity) : 1
    }
  }
  for (const child of model.children) {
    setTransparent(child, transparent)
  }
}

const loadCar = Symbol('loadCarModel')
const loadPedestrian = Symbol('loadPedestrian')
const loadObstacle = Symbol('loadObstacle')
const loadPlanner = Symbol('loadPlanner')
const loadSignlight = Symbol('loadSignlight')
const loadingSignlightPromise = Symbol('loadingSignlightPromise')

/**
 * ScenarioModelsManager 类，用于管理场景中的模型
 */
class ScenarioModelsManager {
  /**
   * constructor
   * @param {object} options
   * @param {'editor'|'player'} options.type
   * @param {object | Function} options.catalogs
   * @param {string} options.prefix
   */
  constructor (options) {
    const { type, catalogs, prefix, replayPureBoxRender } = options
    this.type = type
    this._catalogs = catalogs
    this.prefix = prefix
    this._replayPureBoxRender = replayPureBoxRender ?? false
    this.modelsMapObstacle = {}
    this.modelsMapObstacleLogsim = {}
    this.modelsMapPedestrian = {}
    this.modelsMapPedestrianLogsim = {}
    this.modelsMapCar = {}
    this.modelsMapCarLogsim = {}
    this.modelsMapPlanner = {}
    this.modelsMapPlannerLogsim = {}
    this.modelsMapSignlight = {}
    this.signArrowObjects = null
    this[loadingSignlightPromise] = null

    // 初始化时即加载箭头，因图片不大，对整体性能影响比较轻微
    this.loadSignArrowImgsPromise = Promise.all(Object.keys(signlightArrowImgs).map(async (key) => {
      const url = `${prefix}/${signlightArrowImgs[key]}`
      const {
        material,
        transparentMaterial,
      } = await loadSignArrow(url)
      return {
        name: key,
        material,
        transparentMaterial,
      }
    })).then((arr) => {
      this.signArrowObjects = {
        geometry: signArrowGeometry,
      }
      arr.forEach(({ name, material, transparentMaterial }) => {
        material.transparent = true
        transparentMaterial.transparent = true
        this.signArrowObjects[name] = {
          material,
          transparentMaterial,
        }
      })
    })
  }

  /**
   * 获取 catalogs
   * @returns {Function|Array} - 返回 catalogs 函数或数组
   */
  get catalogs () {
    if (isFunction(this._catalogs)) {
      return this._catalogs()
    }
    return this._catalogs
  }

  /**
   * 获取 replayPureBoxRender 开关状态（支持函数动态读取）
   * @returns {boolean}
   */
  get replayPureBoxRender () {
    if (isFunction(this._replayPureBoxRender)) {
      return this._replayPureBoxRender()
    }
    return this._replayPureBoxRender
  }

  /**
   * 依据当前 pure box 模式返回 forceBox，并在模式翻转时清空车辆/行人/障碍物模型缓存。
   * 解决：回放过程中切换系统“纯 box 渲染”开关后，已缓存的模型仍沿用旧形态
   * （纯 box 缓存不被刷新，需重启 tadsim 才能恢复）的状态不一致问题。
   * @returns {boolean}
   */
  syncForceBoxCache () {
    const forceBox = this.type === 'player' && this.replayPureBoxRender
    if (this._lastForceBox !== undefined && this._lastForceBox !== forceBox) {
      this.clearModelCache('car')
      this.clearModelCache('pedestrian')
      this.clearModelCache('obstacle')
    }
    this._lastForceBox = forceBox
    return forceBox
  }

  /**
   * 获取模型最终路径
   * @param {object} catalog - 目录对象
   * @param {string} model3d - 模型3D路径
   * @returns {string} - 返回模型最终路径
   */
  getModelFinalPath (catalog, model3d) {
    if (catalog.catalogCategory === 'ego' || catalog.variable.startsWith('user_')) {
      return fileURLToPath(model3d)
    } else {
      return `${this.prefix}/${model3d}`
    }
  }

  [loadCar] (code, boundingBox, logsim) {
    const {
      type: runMode,
      catalogs,
      modelsMapCar,
      modelsMapCarLogsim,
    } = this
    let catalog
    // 播放模式下强制车辆渲染为按真实包围盒尺寸缩放的彩色盒子
    const forceBox = this.syncForceBoxCache()
    if (runMode === 'player') {
      // player模式把 traffic code转成 type
      catalog = findCarInCatalog(catalogs, code)
    } else {
      catalog = catalogs.vehicleList.find(v => v.variable === code)
    }

    const type = catalog?.variable || code
    const model = new Group()
    const map = logsim ? modelsMapCarLogsim : modelsMapCar
    let promise
    // 同步可用的 mesh（缓存命中或纯色盒子分支）。用于让调用方同步挂载，
    // 避免 promise.then 的微任务晚于本帧 renderScene 执行，导致回放时盒子闪烁/消失。
    let syncMesh = null
    if (map[type]) {
      const mesh = map[type]
      syncMesh = mesh
      promise = Promise.resolve(mesh)
    } else {
      if (forceBox || !catalog) {
        if (!catalog) console.warn(`未找到 vehicle 定义：${code}。使用默认盒子模型。`)
        const {
          center,
          dimensions: {
            height,
            length,
            width,
          },
        } = boundingBox
        const mesh = boxMeshCar.clone()
        mesh.scale.set(length, width, height)
        modelsMapCar[type] = mesh
        const meshTransparent = mesh.clone()
        modelsMapCarLogsim[type] = meshTransparent
        setTransparent(meshTransparent)
        mesh.position.set(+center.x, +center.y, +center.z)
        meshTransparent.position.set(+center.x, +center.y, +center.z)
        syncMesh = logsim ? meshTransparent : mesh
        promise = Promise.resolve(logsim ? meshTransparent : mesh)
      } else {
        const [{
          boundingBox: {
            center,
          },
          model3d,
        }] = catalog.catalogParams
        promise = loadModel(this.getModelFinalPath(catalog, model3d)).then((mesh) => {
          mesh = mesh.clone()
          // fixMaterialEncoding(mesh)
          const meshTransparent = mesh.clone()
          modelsMapCar[type] = mesh
          modelsMapCarLogsim[type] = meshTransparent
          setTransparent(meshTransparent)
          mesh.position.set(+center.x, +center.y, +center.z)
          meshTransparent.position.set(+center.x, +center.y, +center.z)
          if (logsim) {
            return meshTransparent
          } else {
            return mesh
          }
        })
      }
    }
    model.ignoreScale = true
    return { model, promise, syncMesh }
  }

  /**
   * 加载车辆模型
   * @param {string} code - 车辆代码
   * @param {object} boundingBox - 边界框信息
   * @param {boolean} logsim - 是否为 logsim
   * @returns {object} - 返回包含模型和Promise的对象
   */
  async loadCarModel (car, logsim = false) {
    const { type: code, boundingBox } = car
    const { model, promise } = this[loadCar](`${code}`, boundingBox, logsim)
    const mesh = await promise
    model.add(mesh.clone())
    return model
  }

  /**
   * 异步加载车辆模型
   * @param {object} car - 车辆信息对象
   * @param {boolean} logsim - 是否为logsim
   * @returns {Promise<Group>} - 返回一个包含车辆模型的Promise
   */
  loadCarModelSync (car, logsim = false) {
    const { type: code, boundingBox } = car
    const { model, promise, syncMesh } = this[loadCar](`${code}`, boundingBox, logsim)
    // 盒子/缓存 mesh 可同步挂载，确保本帧 renderScene 前模型已就绪（修复回放闪烁）；
    // 仅 GLTF 真异步加载时才走 promise。
    if (syncMesh) {
      model.add(syncMesh.clone())
    } else {
      promise.then((mesh) => {
        model.add(mesh.clone())
      })
    }
    // 播放模式下 box 渲染，叠加朝向指示箭头，便于直观判断车头方向
    if (this.type === 'player') {
      model.add(createHeadingArrow(boundingBox))
    }
    return model
  }

  // 加载行人内部方法
  [loadPedestrian] (code, boundingBox, logsim = false) {
    const {
      type: runMode,
      catalogs,
      modelsMapPedestrian,
      modelsMapPedestrianLogsim,
    } = this
    let type
    let catalog
    // 播放模式下强制行人渲染为按真实包围盒尺寸缩放的彩色盒子
    const forceBox = this.syncForceBoxCache()
    if (runMode === 'player') {
      // player模式把 traffic code转成 type
      catalog = findPedestrianInCatalog(catalogs, code)
      type = catalog?.variable || code
    } else {
      type = code
    }
    const map = logsim ? modelsMapPedestrianLogsim : modelsMapPedestrian

    const model = new Group()
    let promise
    let syncMesh = null
    if (map[type]) {
      const mesh = map[type]
      syncMesh = mesh
      promise = Promise.resolve(mesh)
    } else {
      let center
      let model3d
      // 行人定义里找
      let catalog = catalogs.pedestrianList.find(p => p.variable === type)
      if (catalog) {
        ({
          catalogParams: {
            boundingBox: {
              center,
            },
            model3d,
          },
        } = catalog)
      } else {
        // 交通车里面也有行人定义
        catalog = catalogs.vehicleList.find(v => v.variable === type)
        if (catalog) {
          ({
            catalogParams: [{
              boundingBox: {
                center,
              },
              model3d,
            }],
          } = catalog)
        }
      }

      if (forceBox || !catalog) {
        if (!catalog) console.warn(`未找到 pedestrian: ${code}`)
        const {
          center,
          dimensions: {
            height,
            length,
            width,
          },
        } = boundingBox
        const mesh = boxMeshPedestrian.clone()
        mesh.scale.set(length, width, height)
        modelsMapPedestrian[type] = mesh
        const meshTransparent = mesh.clone()
        modelsMapPedestrianLogsim[type] = meshTransparent
        setTransparent(meshTransparent)
        mesh.position.set(+center.x, +center.y, +center.z)
        meshTransparent.position.set(+center.x, +center.y, +center.z)
        syncMesh = logsim ? meshTransparent : mesh
        promise = Promise.resolve(logsim ? meshTransparent : mesh)
      } else {
        promise = loadModel(this.getModelFinalPath(catalog, model3d)).then((mesh) => {
          mesh = mesh.clone()
          // fixMaterialEncoding(mesh)
          const meshTransparent = mesh.clone()
          modelsMapPedestrian[type] = mesh
          modelsMapPedestrianLogsim[type] = meshTransparent
          setTransparent(meshTransparent)
          mesh.position.set(+center.x, center.y, +center.z)
          meshTransparent.position.set(+center.x, +center.y, +center.z)
          return logsim ? meshTransparent : mesh
        })
      }
    }
    return { model, promise, syncMesh }
  }

  /**
   * 加载行人模型
   * @param {string} ped - 行人代码
   * @param {boolean} logsim - 是否为 logsim
   * @returns {object} - 返回包含模型和Promise的对象
   */
  async loadPedestrianModel (ped, logsim) {
    const { subType: code, boundingBox } = ped
    const { model, promise } = this[loadPedestrian](`${code}`, boundingBox, logsim)
    const mesh = await promise
    model.add(mesh.clone())
    return model
  }

  /**
   * 同步加载行人模型
   * @param {object} ped - 行人信息对象
   * @param {boolean} logsim - 是否为logsim
   * @returns {Group} - 返回一个包含行人模型的Group对象
   */
  loadPedestrianModelSync (ped, logsim) {
    const { subType: code, boundingBox } = ped
    const { model, promise, syncMesh } = this[loadPedestrian](`${code}`, boundingBox, logsim)
    if (syncMesh) {
      model.add(syncMesh.clone())
    } else {
      promise.then((mesh) => {
        model.add(mesh.clone())
      })
    }
    // 播放模式下 box 渲染，叠加朝向指示箭头
    if (this.type === 'player') {
      model.add(createHeadingArrow(boundingBox))
    }
    return model
  }

  [loadObstacle] (code, boundingBox, logsim = false) {
    const {
      type: runMode,
      catalogs,
      modelsMapObstacle,
      modelsMapObstacleLogsim,
    } = this
    let catalog
    // 播放模式下强制障碍物渲染为按真实包围盒尺寸缩放的彩色盒子
    const forceBox = this.syncForceBoxCache()
    if (runMode === 'player') {
      // player模式把 traffic code转成 type
      catalog = catalogs.obstacleList.find(v => v.catalogParams.properties.modelId === code)
    } else {
      catalog = catalogs.obstacleList.find(v => v.variable === code)
    }

    const type = catalog?.variable || code
    const model = new Group()
    const map = logsim ? modelsMapObstacleLogsim : modelsMapObstacle
    let promise
    let syncMesh = null
    if (map[type]) {
      const mesh = map[type]
      syncMesh = mesh
      promise = Promise.resolve(mesh)
    } else {
      if (forceBox || !catalog) {
        if (!catalog) console.warn(`未找到 obstacle 定义：${code}`)
        const {
          center,
          dimensions: {
            height,
            length,
            width,
          },
        } = boundingBox
        const mesh = boxMeshObstacle.clone()
        mesh.scale.set(length, width, height)
        modelsMapObstacle[type] = mesh
        const meshTransparent = mesh.clone()
        modelsMapObstacleLogsim[type] = meshTransparent
        setTransparent(meshTransparent)
        mesh.position.set(+center.x, +center.y, +center.z)
        meshTransparent.position.set(+center.x, +center.y, +center.z)
        syncMesh = logsim ? meshTransparent : mesh
        promise = Promise.resolve(logsim ? meshTransparent : mesh)
      } else {
        const { boundingBox: { center }, model3d } = catalog.catalogParams
        promise = loadModel(this.getModelFinalPath(catalog, model3d)).then((mesh) => {
          mesh = mesh.clone()
          // fixMaterialEncoding(mesh)
          const meshTransparent = mesh.clone()
          modelsMapObstacle[type] = mesh
          modelsMapObstacleLogsim[type] = meshTransparent
          setTransparent(meshTransparent)
          mesh.position.set(+center.x, +center.y, +center.z)
          meshTransparent.position.set(+center.x, +center.y, +center.z)
          if (logsim) {
            return meshTransparent
          } else {
            return mesh
          }
        })
      }
    }
    return { model, promise, syncMesh }
  }

  /**
   * 加载障碍物模型
   * @param {string} code - 障碍物代码
   * @param {object} boundingBox - 边界框信息
   * @param {boolean} logsim - 是否为 logsim
   * @returns {object} - 返回包含模型和Promise的对象
   */
  async loadObstacleModel (obs, logsim) {
    const { type: code, boundingBox } = obs
    const { model, promise } = this[loadObstacle](`${code}`, boundingBox, logsim)
    const mesh = await promise
    model.add(mesh.clone())
    return model
  }

  loadObstacleModelSync (obs, logsim) {
    const { type: code, boundingBox } = obs
    const { model, promise, syncMesh } = this[loadObstacle](`${code}`, boundingBox, logsim)
    if (syncMesh) {
      model.add(syncMesh.clone())
    } else {
      promise.then((mesh) => {
        model.add(mesh.clone())
      })
    }
    // 静态障碍物不需要朝向箭头
    return model
  }

  [loadPlanner] (name, logsim) {
    const {
      type: runMode,
      catalogs,
      modelsMapPlanner,
      modelsMapPlannerLogsim,
    } = this
    const catalog = catalogs.plannerList.find(p => p.variable === name)
    const { catalogParams, catalogSubCategory } = catalog
    const map = logsim ? modelsMapPlannerLogsim : modelsMapPlanner
    const planner = new Group()
    // 挂车从数组第2开始
    const trailers = catalogSubCategory === 'combination' ? catalogParams.slice(2).map(() => new Group()) : []
    let promise
    if (map[name]) {
      promise = Promise.resolve(map[name])
    } else {
      // 组合型挂车从第1个开始加载，第0个没有模型
      const params = catalogSubCategory === 'combination' ? catalogParams.slice(1) : catalogParams
      promise = Promise.all(params.map(async (v) => {
        const g = (await loadModel(this.getModelFinalPath(catalog, v.model3d))).clone()
        // fixMaterialEncoding(g)
        const {
          boundingBox: {
            center = { x: 0, y: 0, z: 0 },
            trailerOffsetX: offsetX = 0,
            trailerOffsetZ: offsetZ = 0,
          },
        } = v

        if (runMode === 'editor') {
          g.position.set(+center.x + offsetX, +center.y, +center.z + offsetZ)
        } else if (runMode === 'player') {
          // player模式不修正挂车相对于车头的偏移量
          g.position.set(+center.x, +center.y, +center.z)
        }
        return g
      })).then((groups) => {
        const groupsTransparent = groups.map((g) => {
          const mesh = g.clone()
          setTransparent(mesh)
          return mesh
        })
        modelsMapPlanner[name] = groups
        modelsMapPlannerLogsim[name] = groupsTransparent
        return logsim ? groupsTransparent : groups
      })
    }
    return {
      planner,
      trailers,
      promise,
    }
  }

  async loadPlannerModel (name, logsim = false) {
    const { type: runMode } = this
    const { planner, trailers, promise } = this[loadPlanner](name, logsim)
    const groups = await promise
    groups.forEach((g, i) => {
      if (i === 0) {
        planner.add(g.clone())
      } else {
        trailers[i - 1].add(g.clone())
      }
    })
    if (runMode === 'editor') {
      const group = new Group()
      group.add(planner.clone())
      trailers.forEach((t) => {
        group.add(t.clone())
      })
      return group
    }
    return { planner, trailers }
  }

  /**
   * 同步加载planner模型
   * @param name
   * @param logsim
   * @return {{planner: *, trailers: *}|Group}
   */
  loadPlannerModelSync (name, logsim = false) {
    const { type: runMode } = this
    const { planner, trailers, promise } = this[loadPlanner](name, logsim)
    promise.then((groups, i) => {
      groups.forEach((g, i) => {
        if (i === 0) {
          planner.add(g.clone())
        } else {
          trailers[i - 1].add(g.clone())
        }
      })
    })
    if (runMode === 'editor') {
      const group = new Group()
      group.add(planner.clone())
      trailers.forEach((t) => {
        group.add(t.clone())
      })
      return group
    }
    return { planner, trailers }
  }

  [loadSignlight] () {
    const {
      prefix,
      modelsMapSignlight,
      [loadingSignlightPromise]: promise,
    } = this
    if (!promise) {
      this[loadingSignlightPromise] = Promise.all(
        Object.entries(signlightURLs).map(async ([name, url]) => {
          modelsMapSignlight[name] = (await loadModel(`${prefix}/${url}`)).clone().children[0]
          modelsMapSignlight[name].rotation.x = 0
          return modelsMapSignlight[name]
        }),
      ).then(() => modelsMapSignlight)
      return this[loadingSignlightPromise]
    } else {
      if (isEmpty(modelsMapSignlight)) {
        return promise
      }
      return Promise.resolve(modelsMapSignlight)
    }
  }

  /**
   * 加载signlight模型，返回一个Map，key为signlight的名称，value为signlight的Group
   * @return {Promise<Map<string, Group>>}
   */
  async loadSignlightModels () {
    await this[loadSignlight]()
    return new Map(Object.entries(this.modelsMapSignlight).map((m) => {
      const mesh = new Group()
      mesh.add(m[1].clone())
      return [m[0], mesh]
    }))
  }

  /**
   * 加载signlight模型，返回一个Map，key为signlight的名称，value为signlight的Group
   * @return {Map<string, Group>}
   */
  loadSignlightModelsSync () {
    const map = new Map(Object.entries(signlightURLs).map(m => [m[0], new Group()]))
    this[loadSignlight]().then(() => {
      const { modelsMapSignlight } = this
      Object.entries(modelsMapSignlight).forEach(([name, mesh]) => {
        const group = map.get(name)
        group.add(mesh.clone())
      })
    })
    return map
  }

  /**
   * 加载signlight箭头以及所有贴图，因为交付物不是Group，所以无法提供Sync版
   * @returns {Promise<any>}
   */
  async loadSignArrowModels () {
    if (this.signArrowObjects) {
      return this.signArrowObjects
    }
    await this.loadSignArrowImgsPromise
    return this.signArrowObjects
  }

  /**
   * 同步加载所有模型
   * @return {Promise<Awaited<Awaited<unknown>[]>[]>}
   */
  loadAllModels () {
    const {
      catalogs: {
        plannerList,
        vehicleList,
        obstacleList,
        pedestrianList,
      },
    } = this
    const plannerPromise = Promise.all(plannerList.map(planner => this.loadPlannerModel(planner.variable)))
    const vehiclePromise = Promise.all(vehicleList.map(car => this.loadCarModel(car)))
    const obstaclePromise = Promise.all(obstacleList.map(ob => this.loadObstacleModel(ob)))
    const pedestrianPromise = Promise.all(pedestrianList.map(pe => this.loadPedestrianModel(pe)))
    return Promise.all([
      plannerPromise,
      vehiclePromise,
      obstaclePromise,
      pedestrianPromise,
    ])
  }

  loadModel (url) {
    return loadModel(url)
  }

  /**
   * 根据type从Catalogs里查找对应的模型定义
   * @param type
   * @param modelId
   * @return {undefined|*}
   */
  findCatalogByModelId (type, modelId) {
    const { catalogs } = this
    switch (type) {
      case 'car':
        return catalogs.vehicleList.find(v => v.catalogParams[0].properties.modelId === modelId)
      case 'pedestrian':
        return catalogs.pedestrianList.find(v => v.catalogParams.properties.modelId === modelId)
      case 'obstacle':
        return catalogs.obstacleList.find(v => v.catalogParams.properties.modelId === modelId)
    }
    return undefined
  }

  /**
   * 清除模型缓存
   * @param type
   * @param name
   */
  clearModelCache (type, name) {
    if (type) {
      switch (type) {
        case 'planner':
          if (name) {
            delete this.modelsMapPlanner[name]
            delete this.modelsMapPlannerLogsim[name]
          } else {
            this.modelsMapPlanner = {}
            this.modelsMapPlannerLogsim = {}
          }
          break
        case 'car':
          if (name) {
            delete this.modelsMapCar[name]
            delete this.modelsMapCarLogsim[name]
          } else {
            this.modelsMapCar = {}
            this.modelsMapCarLogsim = {}
          }
          break
        case 'pedestrian':
          if (name) {
            delete this.modelsMapPedestrian[name]
            delete this.modelsMapPedestrianLogsim[name]
          } else {
            this.modelsMapPedestrian = {}
            this.modelsMapPedestrianLogsim = {}
          }
          break
        case 'obstacle':
          if (name) {
            delete this.modelsMapObstacle[name]
            delete this.modelsMapObstacleLogsim[name]
          } else {
            this.modelsMapObstacle = {}
            this.modelsMapObstacleLogsim = {}
          }
          break
        case 'signlight':
          this.modelsMapSignlight = {}
          break
      }
    } else {
      this.modelsMapObstacle = {}
      this.modelsMapObstacleLogsim = {}
      this.modelsMapPedestrian = {}
      this.modelsMapPedestrianLogsim = {}
      this.modelsMapCar = {}
      this.modelsMapCarLogsim = {}
      this.modelsMapPlanner = {}
      this.modelsMapPlannerLogsim = {}
      this.modelsMapSignlight = {}
    }
  }
}

export default ScenarioModelsManager
