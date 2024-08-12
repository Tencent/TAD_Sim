import { LineBasicMaterial } from 'three'
import { MapElementType } from '../common/Constant'
import { MapElement } from './Common.js'

/**
 * 车道数据类
 */
class RoadData extends MapElement {
  constructor () {
    super()
    this.elemType = MapElementType.ROAD

    this.id = 0
    this.type = 0
    this.len = 0
    this.dir = 0
    this.mat = 0

    this.sections = []
  }

  parse (data) {
    this.id = data.roadid
    this.type = data.type
    this.len = data.len
    this.dir = data.dir
    this.mat = data.mat
  }
}

// 车道默认材质
RoadData.mat = new LineBasicMaterial({ color: 0x0E084C })

export default RoadData
