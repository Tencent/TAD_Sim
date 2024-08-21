import { isArray, isEmpty, isFunction, memoize } from 'lodash-es'
import { signlightArrowImgs, signlightURLs, simpleCarWhiteUrl } from './scene/constants'
import { fileURLToPath } from './utils/loader'
import { findCarInCatalog, findPedestrianInCatalog } from './catalogs/utils'
import {
  BoxGeometry,
  DoubleSide,
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
// 蓝盒子模型
const boxGeo = new BoxGeometry(1, 1, 1)
const boxMat = new MeshLambertMaterial({
  color: 0x16D1F3,
  side: DoubleSide,
})
const boxMesh = new Mesh(boxGeo, boxMat)

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
    const { type, catalogs, prefix } = options
    this.type = type
    this._catalogs = catalogs
    this.prefix = prefix
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
      prefix,
      modelsMapCar,
      modelsMapCarLogsim,
    } = this
    let catalog
    if (runMode === 'player') {
      // 播放1000的车固定用白盒子
      if (code === 1000) {
        return {
          model: new Group(),
          promise: loadModel(`${prefix}/${simpleCarWhiteUrl}`).then(mesh => mesh.clone()),
        }
      }
      // player模式把 traffic code转成 type
      catalog = findCarInCatalog(catalogs, code)
    } else {
      catalog = catalogs.vehicleList.find(v => v.variable === code)
    }

    const type = catalog?.variable || code
    const model = new Group()
    const map = logsim ? modelsMapCarLogsim : modelsMapCar
    let promise
    if (map[type]) {
      const mesh = map[type]
      promise = Promise.resolve(mesh)
    } else {
      if (!catalog) {
        console.warn(`未找到 vehicle 定义：${code}。使用默认蓝盒子模型。`)
        const {
          center,
          dimensions: {
            height,
            length,
            width,
          },
        } = boundingBox
        const mesh = boxMesh.clone()
        mesh.scale.set(length, width, height)
        modelsMapCar[type] = mesh
        const meshTransparent = mesh.clone()
        modelsMapCarLogsim[type] = meshTransparent
        setTransparent(meshTransparent)
        mesh.position.set(+center.x, +center.y, +center.z)
        meshTransparent.position.set(+center.x, +center.y, +center.z)
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
    return { model, promise }
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
    const { model, promise } = this[loadCar](`${code}`, boundingBox, logsim)
    promise.then((mesh) => {
      model.add(mesh.clone())
    })
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
    if (map[type]) {
      const mesh = map[type]
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

      if (!catalog) {
        console.warn(`未找到 pedestrian: ${code}`)
        const {
          center,
          dimensions: {
            height,
            length,
            width,
          },
        } = boundingBox
        const mesh = boxMesh.clone()
        mesh.scale.set(length, width, height)
        modelsMapPedestrian[type] = mesh
        const meshTransparent = mesh.clone()
        modelsMapPedestrianLogsim[type] = meshTransparent
        setTransparent(meshTransparent)
        mesh.position.set(+center.x, +center.y, +center.z)
        meshTransparent.position.set(+center.x, +center.y, +center.z)
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
    return { model, promise }
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
    const { model, promise } = this[loadPedestrian](`${code}`, boundingBox, logsim)
    promise.then((mesh) => {
      model.add(mesh.clone())
    })
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
    if (map[type]) {
      const mesh = map[type]
      promise = Promise.resolve(mesh)
    } else {
      if (!catalog) {
        console.warn(`未找到 obstacle 定义：${code}`)
        const {
          center,
          dimensions: {
            height,
            length,
            width,
          },
        } = boundingBox
        const mesh = boxMesh.clone()
        mesh.scale.set(length, width, height)
        modelsMapObstacle[type] = mesh
        const meshTransparent = mesh.clone()
        modelsMapObstacleLogsim[type] = meshTransparent
        setTransparent(meshTransparent)
        mesh.position.set(+center.x, +center.y, +center.z)
        meshTransparent.position.set(+center.x, +center.y, +center.z)
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
    return { model, promise }
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
    const { model, promise } = this[loadObstacle](`${code}`, boundingBox, logsim)
    promise.then((mesh) => {
      model.add(mesh.clone())
    })
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
        return catalogs.pedestrianList.find(v => v.catalogParams.properties.modelId === modelId)
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
