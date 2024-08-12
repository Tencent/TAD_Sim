import { TxVec3 } from '../../sceneeditor/Common'

/**
 * 主车位置类
 */
class SelfLocation {
  constructor () {
    this.id = -1
    this.time = -1
    this.pos = new TxVec3(0, 0, 0)
    this.vel = new TxVec3(0, 0, 0)
    this.angular = new TxVec3(0, 0, 0)
    this.rpy = new TxVec3(0, 0, 0)
  }

  /**
   * 解析数据
   * @param data
   */
  parse (data) {
    this.id = data.id
    if (data.t != undefined) {
      this.time = data.t
    }

    if (data.position.x != undefined) {
      this.pos.x = data.position.x
    }
    if (data.position.y != undefined) {
      this.pos.y = data.position.y
    }
    if (data.position.z != undefined) {
      this.pos.z = data.position.z
    }

    if (data.velocity != undefined) {
      if (data.velocity.x != undefined) {
        this.vel.x = data.velocity.x
      }
      if (data.velocity.y != undefined) {
        this.vel.y = data.velocity.y
      }
      if (data.velocity.z != undefined) {
        this.vel.z = data.velocity.z
      }
    }

    if (data.angular != undefined) {
      if (data.angular.x != undefined) {
        this.angular.x = data.angular.x
      }
      if (data.angular.y != undefined) {
        this.angular.y = data.angular.y
      }
      if (data.angular.z != undefined) {
        this.angular.z = data.angular.z
      }
    }

    if (data.rpy != undefined) {
      if (data.rpy.z != undefined) {
        this.rpy.z = data.rpy.z
      }
    }
  }
}

export default SelfLocation
