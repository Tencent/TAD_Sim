/**
 * 交通流障碍物类
 * @author <marsyu>
 * @date 2019-04-25
 */
class TrafficO {
  constructor () {
    this.id = -1
    this.type = ''
    this.x = 0
    this.y = 0
    this.heading = 0
    this.length = 0
    this.width = 0
    this.height = 0
    this.lon = 0
    this.lat = 0
  }

  copy (o) {
    this.id = o.id
    this.x = o.x
    this.y = o.y
    this.heading = o.heading
    this.length = o.length
    this.width = o.width
    this.height = o.height
    this.type = o.type
    this.lon = o.lon
    this.lat = o.lat
  }

  copyFromNet (o) {
    this.id = o.id
    this.heading = o.heading
    this.length = o.length
    this.width = o.width
    this.height = o.height
    this.type = o.type
    this.lon = o.x
    this.lat = o.y
  }
}

export default TrafficO
