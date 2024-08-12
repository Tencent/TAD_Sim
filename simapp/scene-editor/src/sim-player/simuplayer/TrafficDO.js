/**
 * 交通流动态障碍物类
 */
class TrafficDO {
  constructor () {
    this.id = -1
    this.x = 0
    this.y = 0
    this.heading = 0
    this.length = 0
    this.width = 0
    this.height = 0
    this.type = ''
    this.v = 0
    this.v1 = 0
    this.lon = 0
    this.lat = 0
    this.showAbsVelocity = 0
    this.showAbsAcc = 0
    this.showRelativeAcc = 0
    this.showRelativeVelocity = 0
    this.showRelativeVelocityHorizontal = 0
    this.showRelativeDistVertical = 0
    this.showRelativeDistHorizontal = 0
  }

  /**
   * 复制一份数据
   * @param o
   */
  copy (o) {
    this.id = o.id
    this.x = o.x
    this.y = o.y
    this.heading = o.heading
    this.length = o.length
    this.width = o.width
    this.height = o.height
    this.type = o.type
    this.v = o.v
    this.v1 = o.v1
    this.lon = o.lon
    this.lat = o.lat
    this.showAbsVelocity = o.showAbsVelocity
    this.showAbsAcc = o.showAbsAcc
    this.showRelativeAcc = o.showRelativeAcc
    this.showRelativeVelocity = o.showRelativeVelocity
    this.showRelativeVelocityHorizontal = o.showRelativeVelocityHorizontal
    this.showRelativeDistVertical = o.showRelativeDistVertical
    this.showRelativeDistHorizontal = o.showRelativeDistHorizontal
  }

  /**
   * 从网络数据复制
   * @param o
   */
  copyFromNet (o) {
    this.id = o.id
    this.heading = o.heading
    this.length = o.length
    this.width = o.width
    this.height = o.height
    this.type = o.type
    this.lon = o.x
    this.lat = o.y
    this.v = o.v
    this.v1 = o.v1
    this.showAbsVelocity = o.showAbsVelocity
    this.showAbsAcc = o.showAbsAcc
    this.showRelativeAcc = o.showRelativeAcc
    this.showRelativeVelocity = o.showRelativeVelocity
    this.showRelativeVelocityHorizontal = o.showRelativeVelocityHorizontal
    this.showRelativeDistVertical = o.showRelativeDistVertical
    this.showRelativeDistHorizontal = o.showRelativeDistHorizontal
  }

  get subType () {
    return this.type
  }
}

export default TrafficDO
