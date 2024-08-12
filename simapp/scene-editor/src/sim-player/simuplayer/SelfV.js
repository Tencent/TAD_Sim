import { Vector3 } from 'three'
import { TxVec3 } from '../sceneeditor/Common.js'

/**
 * 主车播放类
 */
class SelfV {
  constructor () {
    this.id = -1
    this.time = -1
    this.pos = new TxVec3(0, 0, 0)
    this.vel = new TxVec3(0, 0, 0)
    this.angular = new TxVec3(0, 0, 0)
    this.rpy = new TxVec3(0, 0, 0)

    this.lastPos = new TxVec3(0, 0, 0)
    this.offset = new TxVec3(0, 0, 0)
    this.posT = new Vector3(0, 0, 0)
    this.model = null
    this.replay = false
  }

  copy (o) {
    this.time = o.time
    this.pos.copy(o.pos)
    this.vel.copy(o.vel)
    this.angular.copy(o.angular)
    this.rpy.copy(o.rpy)
    this.lastPos.copy(o.lastPos)
    this.posT.set(o.posT)
  }

  copyFromNet (o) {
    this.time = o.time
    this.pos.copy(o.pos)
    this.vel.copy(o.vel)
    this.angular.copy(o.angular)
    this.rpy.copy(o.rpy)
    this.posT.set(this.pos.x, this.pos.y, this.pos.z)
  }

  /**
   * 获取偏移量
   * @param ct
   * @return {TxVec3}
   */
  getOffset (ct) {
    const tmp = ct.lonlat2xyz(this.pos.x, this.pos.y, this.pos.z)
    const tmp2 = ct.lonlat2xyz(this.lastPos.x, this.lastPos.y, this.lastPos.z)
    this.offset.x = tmp[0] - tmp2[0]
    this.offset.y = tmp[1] - tmp2[1]
    this.offset.z = tmp[2] - tmp2[2]

    return this.offset
  }
}

export default SelfV
