import {
  BufferGeometry,
  Float32BufferAttribute,
  LineSegments,
  Vector3,
} from 'three'
import GlobalConfig from '../common/Config'
import { IgnoreHeight, MapElementType } from '../common/Constant'

/**
 * 内部使用的三维向量，免去了THREE.Vector3的一些其他属性
 */
class TxVec3 {
  constructor (x, y, z) {
    this.x = x
    this.y = y
    this.z = z
  }

  copy (o) {
    this.x = o.x
    this.y = o.y
    this.z = o.z
  }
}

/**
 * 场景元素底层类
 */
class MapElement {
  constructor () {
    this.data = []
    this.isAddedToScene = false
    this.elemType = MapElementType.INVALID
    this._uniqueData = null
  }

  dispose () {
    if (this.line) {
      this.line = null
    }

    if (this.geom) {
      this.geom.dispose()
      this.geom = null
    }
  }

  /**
   * 压入一个点
   * @param pointData
   * @param ct
   */
  pushData (pointData, ct) {
    if (pointData.array != undefined) {
      const nLen = pointData.array.length

      for (let i = 0; i < nLen; ++i) {
        const z = GlobalConfig.ignoreHeight === IgnoreHeight.Yes ? 0 : pointData.array[i].z
        this.data[i] = new TxVec3((pointData.array[i].x), (pointData.array[i].y), z)
      }
    }
  }

  /**
   * 合并场景geo数据
   * @param material
   */
  composeSceneData (material) {
    if (this.data.length > 0) {
      this.geom = new BufferGeometry()
      const positions = []
      const nLen = this.data.length
      for (let i = 0; i < nLen; ++i) {
        positions.push(this.data[i].x, this.data[i].y, this.data[i].z)
      }
      this.geom.setAttribute('position', new Float32BufferAttribute(positions, 3))
      if (!GlobalConfig.MergeGeom()) {
        this.line = new LineSegments(this.geom, material)
      }
    }
  }

  /**
   * 将模型加入场景
   * @param scene
   */
  addToScene (scene) {
    if (!GlobalConfig.MergeGeom()) {
      if (this.isAddedToScene == false) {
        scene.add(this.line)
        this.isAddedToScene = true
        this.line.matrixAutoUpdate = false
        this.line.updateMatrix()
      }
    }
  }

  /**
   * 将模型从场景移除
   * @param scene
   */
  removeFromScene (scene) {
    if (!GlobalConfig.MergeGeom()) {
      if (this.isAddedToScene == true) {
        scene.remove(this.line)
        this.isAddedToScene = false
      }
    }
  }

  /**
   * 设置场景可见性
   * @param show
   */
  setVisible (show) {
    if (this.line != undefined) {
      this.line.visible = show
    }
  }

  /**
   * 获取队列数据
   * @return {null}
   */
  getUniqueData () {
    if (!this._uniqueData) {
      const length = this.data.length

      this._uniqueData = []

      this._uniqueData.push(this.data[0])

      for (let i = 1; i < length - 1; i += 2) {
        this._uniqueData.push(this.data[i])
      }

      this._uniqueData.push(this.data[length - 1])
    }
    return this._uniqueData
  }
}

MapElement.defaultDir = new Vector3(1, 0, 0)
MapElement.UpVector = new Vector3(0, 0, 1)
MapElement.DefaultVector = new Vector3(1, 0, 0)
export { TxVec3, MapElement }
