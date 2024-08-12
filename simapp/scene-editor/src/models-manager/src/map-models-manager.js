import { createElements } from './map/render'
import {
  registryCustomModelList,
  registryGetPositionNormal,
} from './map/utils/common'
import { initAssetsPath } from './map/utils/urls'

const defaultMapElementTypes = [
  'pole',
  'roadSign',
  'signalBoard',
  'trafficLight',
  'sensor',
  'other',
  'parkingSpace',
  // 默认支持自定义模型
  'customModel',
]

/**
 * 调用示例
 * const mapModelsManager = new MapModelsManager({
 *   modelPath: 'data/models',
 *   imgPath: 'data/imgs',
 * })
 * const container = mapModelsManager.loadAllMapElements(
 *   mapData.array
 * )
 * scene.add(container)
 * render()
 */

// 地图相关物体加载器
class MapModelsManager {
  elementTypes = []
  modelPath = ''
  imgPath = ''
  constructor (options) {
    const {
      elements = defaultMapElementTypes,
      modelPath = '',
      imgPath = '',
    } = options

    // 设置默认支持渲染的元素类型
    this.elementTypes = elements
    this.modelPath = modelPath
    this.imgPath = imgPath

    // 初始化静态资源访问的根路径
    initAssetsPath({
      model: modelPath,
      img: imgPath,
    })
  }

  // 将所有的地图元素数据，解析并渲染到目标容器中
  loadAllMapElements (mapData, options = {}) {
    const {
      ignoreAlt = false,
      mapVersion = '',
      ignoreSize = false,
      getPositionNormal,
      // 从业务逻辑中获取本地自定义导入模型列表透传
      customModelList = [],
    } = options || {}
    // 注册获取道路上某一个位置的法向量的方法
    if (getPositionNormal) {
      registryGetPositionNormal(getPositionNormal)
    }

    // 注册需要缓存的自定义模型列表
    registryCustomModelList(customModelList)

    const container = createElements({
      data: mapData,
      types: this.elementTypes,
      ignoreAlt,
      mapVersion,
      ignoreSize,
    })
    return container
  }
}

export default MapModelsManager
