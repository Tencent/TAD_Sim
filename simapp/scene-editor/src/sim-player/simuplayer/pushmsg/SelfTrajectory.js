/**
 * 规划轨迹点类
 */
class TrajectoryPoint {
  constructor () {
    this.id = -1
    this.x = 0
    this.y = 0
    this.t = 0
    this.v = 0
    this.theta = 0
    this.kappa = 0
    this.s = 0
  }
}

/**
 * 规划轨迹线类
 */
class SelfTrajectory {
  constructor () {
    this.a = 0
    this.type = 0
    this.flag = 0
    this.points = []
  }

  /**
   * 解析数据
   * @param data
   */
  parse (data) {
    this.id = data.id
    this.a = data.a
    this.type = data.type
    this.flag = data.flag
    const len = data.point.length
    for (let i = 0; i < len; ++i) {
      this.points[i] = new TrajectoryPoint()
      this.points[i].x = data.point[i].x
      this.points[i].y = data.point[i].y
      this.points[i].t = data.point[i].t
      this.points[i].v = data.point[i].v
      this.points[i].theta = data.point[i].theta
      this.points[i].kappa = data.point[i].kappa
    }
  }
}

export default SelfTrajectory
