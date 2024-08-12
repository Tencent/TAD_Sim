/**
 * 交通流交通车类
 */
class TrafficV {
  constructor () {
    this.id = -1
    this.type = ''
    this.time = -1
    this.x = 0
    this.y = 0
    this.heading = 0
    this.v = 0
    this.v1 = 0
    this.theta = 0
    this.length = 0
    this.width = 0
    this.height = 0
    this.lon = 0
    this.lat = 0
    this.visible = true
    this.showAbsVelocity = 0
    this.showAbsAcc = 0
    this.showRelativeAcc = 0
    this.showRelativeVelocity = 0
    this.showRelativeVelocityHorizontal = 0
    this.showRelativeDistVertical = 0
    this.showRelativeDistHorizontal = 0
    this.angle = 0
    this.isTransparent = false
  }

  /**
   * 复制对象
   * @param o
   */
  copy (o) {
    this.id = o.id
    this.type = o.type
    this.time = o.time
    this.x = o.x
    this.y = o.y
    this.heading = o.heading
    this.v = o.v
    this.v1 = o.v1
    this.theta = o.theta
    this.length = o.length
    this.width = o.width
    this.height = o.height
    this.lon = o.lon
    this.lat = o.lat
    this.visible = o.visible
    this.showAbsVelocity = o.showAbsVelocity
    this.showAbsAcc = o.showAbsAcc
    this.showRelativeAcc = o.showRelativeAcc
    this.showRelativeVelocity = o.showRelativeVelocity
    this.showRelativeVelocityHorizontal = o.showRelativeVelocityHorizontal
    this.showRelativeDistVertical = o.showRelativeDistVertical
    this.showRelativeDistHorizontal = o.showRelativeDistHorizontal
  }

  /**
   * 从网络数据复制对象
   * @param o
   */
  copyFromNet (o) {
    this.id = o.id
    this.type = o.type
    this.time = o.time
    this.heading = o.heading
    this.v = o.v
    this.v1 = o.v1
    this.theta = o.theta
    this.length = o.length
    this.width = o.width
    this.height = o.height
    this.lon = o.x
    this.lat = o.y
    this.trajectory = o.trajectory
    this.showAbsVelocity = o.showAbsVelocity
    this.showAbsAcc = o.showAbsAcc
    this.showRelativeAcc = o.showRelativeAcc
    this.showRelativeVelocity = o.showRelativeVelocity
    this.showRelativeVelocityHorizontal = o.showRelativeVelocityHorizontal
    this.showRelativeDistVertical = o.showRelativeDistVertical
    this.showRelativeDistHorizontal = o.showRelativeDistHorizontal
  }
}

export default TrafficV
