import { isNumber } from 'lodash-es'
import {
  BufferGeometry,
  Group,
  Line,
  LineBasicMaterial,
  LineDashedMaterial,
  Vector3,
} from 'three'
import { MapElementType } from '../common/Constant'
import { composeLeftRightPoints, parseLaneMark } from '../common/MeshUtils'
import { MapElement } from './Common'

/**
 * 车道线颜色定义
 * @type {{red: number, green: number, white: number, blue: number, yellow: number}}
 */
const COLORMAP = {
  white: 0xF1F1F1,
  red: 0xFC2D2D,
  green: 0x00D82C,
  blue: 0x0DC2EC,
  yellow: 0xF1EA15,
}

class LaneBoundaryData extends MapElement {
  constructor () {
    super()
    this.elemType = MapElementType.BOUNDARY
    this.id = 0
    this.mark = 0
    this.mesh = null
  }

  parse (data) {
    for (const key in data) {
      if (key === 'data') {
        continue
      }
      this[key] = data[key]
    }
  }

  composeSceneData (material) {
    if (isNumber(this.mark)) {
      this.mesh = this.createLineGroupByMark()
    } else {
      super.composeSceneData(material)
    }
  }

  /**
   * 根据mark生成车道线
   * @return {Group}
   */
  createLineGroupByMark () {
    // 解析mark线形
    const {
      // bold, 暂不支持加粗车道线
      color,
      double,
      leftType,
      rightType,
    } = parseLaneMark(this.mark)
    const group = new Group()
    let leftPoints = []
    let rightPoints = []
    const points = this.getUniqueData().map(v => new Vector3(v.x, v.y, v.z))
    if (double && leftType !== 'none') {
      // double 且左车道线不为none才认为是双线，否则是单线
      ({ leftPoints, rightPoints } = composeLeftRightPoints(points))
    } else if (rightType !== 'none') {
      rightPoints = points
    }
    // 左侧
    if (leftPoints.length) {
      const mat = this.genMaterial(leftType, color)
      const geo = new BufferGeometry()
      geo.setFromPoints(leftPoints)
      const line = new Line(geo, mat)
      line.computeLineDistances()
      group.add(line)
    }
    // 右侧
    if (rightPoints.length) {
      const mat = this.genMaterial(rightType, color)
      const geo = new BufferGeometry()
      geo.setFromPoints(rightPoints)
      const line = new Line(geo, mat)
      line.computeLineDistances()
      group.add(line)
    }

    return group
  }

  /**
   * 根据type和color生成车道线材质
   * @param type
   * @param color
   * @return {LineBasicMaterial}
   */
  genMaterial (type, color) {
    let mat
    if (!(color in COLORMAP)) {
      color = 'white'
    }
    color = COLORMAP[color]
    if (type === 'dashed') {
      mat = new LineDashedMaterial({
        color,
        dashSize: 6,
        gapSize: 9,
      })
    } else {
      mat = new LineBasicMaterial({
        color,
        transparent: false,
        depthTest: true,
      })
    }
    return mat
  }

  addToScene (scene) {
    if (this.isAddedToScene === false && this.mesh) {
      scene.add(this.mesh)
      this.isAddedToScene = true
    }
  }

  removeFromScene (scene) {
    if (this.isAddedToScene === true) {
      scene.remove(this.mesh)
      this.isAddedToScene = false
    }
  }
}

export default LaneBoundaryData
