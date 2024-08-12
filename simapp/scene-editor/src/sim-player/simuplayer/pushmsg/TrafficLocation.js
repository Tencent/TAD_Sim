/**
 * 交通车规划路径点
 */
class VehicleTrajectoryPoint {
  constructor () {
    this.x = 0
    this.y = 0
    this.t = 0
    this.v = 0
    this.theta = 0
    this.kappa = 0
    this.s = 0
  }

  copy (o) {
    this.x = o.x
    this.y = o.y
    this.t = o.t
    this.v = o.v
    this.theta = o.theta
    this.kappa = o.kappa
    this.s = o.s
  }
}

/**
 * 交通车规划路径
 */
class VehicleTrajectory {
  constructor () {
    this.a = 0
    this.type = 0
    this.flag = 0
    this.points = []
  }

  copy (o) {
    this.a = o.a
    this.type = o.type
    this.flag = o.flag
    this.points.splice(0)

    o.points.forEach((value) => {
      const p = new VehicleTrajectoryPoint()
      p.copy(value)
      this.points.push(p)
    })
  }

  parse (data) {
    this.a = data.a
    this.type = data.type
    this.flag = data.flag
    const len = data.point.length
    for (let i = 0; i < len; ++i) {
      this.points[i] = new VehicleTrajectoryPoint()
      this.points[i].x = data.point[i].x
      this.points[i].y = data.point[i].y
      this.points[i].t = data.point[i].t
      this.points[i].v = data.point[i].v
      this.points[i].theta = data.point[i].theta
      this.points[i].kappa = data.point[i].kappa
    }
  }
}

/**
 * 交通流交通车类
 */
class TrafficCar {
  constructor () {
    this.id = -1
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
    this.type = 0
    this.trajectory = undefined
  }

  copy (o) {
    this.id = o.id
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
    if (o.trajectory) {
      this.trajectory = new VehicleTrajectory()
      this.trajectory.copy(o.trajectory)
    }
  }

  /**
   * 从交通流数据帧中复制数据
   * @param {*} c
   */
  copyFromTrafficFrame (c) {
    const keys = ['id', 'x', 'y', 'heading', 'v', 'type', 'showAbsVelocity', 'showAbsAcc', 'showRelativeAcc', 'showRelativeVelocity', 'showRelativeVelocityHorizontal', 'showRelativeDistVertical', 'showRelativeDistHorizontal', 'v1', 'theta', 'length', 'width', 'height']
    keys.forEach((key) => {
      this[key] = c[key]
    })
  }
}

/**
 * 交通流障碍物类
 */
class TrafficObstacle {
  constructor () {
    this.id = -1
    this.x = 0
    this.y = 0
    this.heading = 0
    this.length = 0
    this.width = 0
    this.height = 0
    this.type = 0
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
  }

  /**
   * 从交通流数据帧中复制数据
   * @param {*} o
   */
  copyFromTrafficFrame (o) {
    [
      'id',
      'type',
      'heading',
      'x',
      'y',
      'length',
      'width',
      'height',
    ].forEach((key) => {
      this[key] = o[key]
    })
  }
}

/**
 * 交通流动态障碍物类
 */
class TrafficDynamicObstacle {
  constructor () {
    this.id = -1
    this.x = 0
    this.y = 0
    this.heading = 0
    this.length = 0
    this.width = 0
    this.height = 0
    this.type = 0
    this.v = 0
    this.v1 = 0
    this.showAbsVelocity = 0
    this.showAbsAcc = 0
    this.showRelativeAcc = 0
    this.showRelativeVelocity = 0
    this.showRelativeVelocityHorizontal = 0
    this.showRelativeDistVertical = 0
    this.showRelativeDistHorizontal = 0
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
    this.v = o.v
    this.v1 = o.v1
  }

  /**
   * 从交通流数据帧中复制数据
   * @param {*} o
   */
  copyFromTrafficFrame (o) {
    [
      'id',
      'type',
      'heading',
      'x',
      'y',
      'v',
      'v1',
      'showAbsVelocity',
      'showAbsAcc',
      'showRelativeAcc',
      'showRelativeVelocity',
      'showRelativeVelocityHorizontal',
      'showRelativeDistVertical',
      'showRelativeDistHorizontal',
      'length',
      'width',
      'height',
    ].forEach((key) => {
      this[key] = o[key]
    })
  }
}

/**
 * 交通流交通灯类
 */
class TrafficLight {
  constructor () {
    this.id = -1
    this.x = 0
    this.y = 0
    this.heading = 0
    this.color = 0
  }

  copy (o) {
    this.id = o.id
    this.x = o.x
    this.y = o.y
    this.heading = o.heading
    this.color = o.color
  }

  /**
   * 从交通流数据帧中复制数据
   * @param {*} o
   */
  copyFromTrafficFrame (o) {
    [
      'id',
      'heading',
      'x',
      'y',
      'color',
      'controlPhases',
    ].forEach((key) => {
      this[key] = o[key]
    })
  }
}

/**
 * 交通流位置类
 */
class TrafficLocation {
  constructor () {
    this.cars = []
    this.obstacles = []
    this.dynamicObstacles = []
    this.lights = []
    this.carMap = new Map()
    this.obstacleMap = new Map()
    this.dynamicObstacleMap = new Map()
    this.lightMap = new Map()
  }

  /**
   * 解析帧数据
   * @param data
   */
  parse (data) {
    if (data.cars?.length) {
      this.cars = data.cars.map((veh) => {
        const car = new TrafficCar()
        car.time = veh.t
        car.copyFromTrafficFrame(veh)
        if (veh.planningLine) {
          car.trajectory = new VehicleTrajectory()
          car.trajectory.parse(veh.planningLine)
        }
        this.carMap.set(car.id, car)
        return car
      })
    }

    if (data.staticObstacles?.length) {
      this.obstacles = data.staticObstacles.map((so) => {
        const ob = new TrafficObstacle()
        ob.copyFromTrafficFrame(so)
        this.obstacleMap.set(ob.id, ob)
        return ob
      })
    }

    if (data.dynamicObstacles?.length) {
      this.dynamicObstacles = data.dynamicObstacles.map((ped) => {
        const dyo = new TrafficDynamicObstacle()
        dyo.copyFromTrafficFrame(ped)
        this.dynamicObstacleMap.set(dyo.id, dyo)
        return dyo
      })
    }

    if (data.trafficLights?.length) {
      this.lights = data.trafficLights.map((tl) => {
        const light = new TrafficLight()
        light.copyFromTrafficFrame(tl)
        this.lightMap.set(light.id, light)
        return light
      })
    }
  }
}

export default TrafficLocation
