/**
 * 轨迹跟踪点类
 * @constructor
 */
class TrajectoryFollowPoint {
  constructor () {
    this.x = 0
    this.y = 0
    this.t = 0
    this.v = 0
    this.theta = 0
    this.kappa = 0
    this.s = 0
    this.a = 0
    this.gear = 0
  }

  parse (data) {
    this.x = data.x
    this.y = data.y
    this.t = data.t
    this.v = data.v
    this.theta = data.theta
    this.kappa = data.kappa
    this.s = data.s
    this.a = data.a
    this.gear = data.gear
  }
}

/**
 * 轨迹跟踪类
 * @constructor
 */
class SelfTrajectoryFollow {
  constructor () {
    this.points = []
    this.a = 0
    this.type = 0
    this.flag = 0
  }

  /**
   * 解析数据
   * @param data
   */
  parse (data) {
    if (!data) {
      console.log('SelfTrajectoryFollwo data is null')
      return
    }

    if (data.point != undefined) {
      for (const p of data.point) {
        const pp = new TrajectoryFollowPoint()
        pp.parse(p)
        this.points.push(pp)
      }
    }

    if (data.a != undefined) {
      this.a = data.a
    }

    if (data.type != undefined) {
      this.type = data.type
    }

    if (data.flag != undefined) {
      this.flag = data.flag
    }
  }
}

export default SelfTrajectoryFollow
