class Pedestrian {
  constructor () {
    this.id = ''
    this.routeId = ''
    this.laneId = ''
    this.startShift = 0.0
    this.startOffset = 0.0
    this.startTime = 0.0
    this.endTime = 0.0
    this.direction1 = 0
    this.direction1_t = 0
    this.direction1_v = 0
    this.direction2 = -1
    this.direction2_t = -1
    this.direction2_v = -1
    this.subType = '0'
    this.endPosArr = []
  }

  /**
   * 复制对象，不包括模型
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.routeId = other.routeId
    this.laneId = other.laneId
    this.startShift = other.startShift
    this.startOffset = other.startOffset
    this.startTime = other.startTime
    this.endTime = other.endTime
    this.direction1 = other.direction1
    this.direction1_t = other.direction1_t
    this.direction1_v = other.direction1_v
    this.direction2 = other.direction2
    this.direction2_t = other.direction2_t
    this.direction2_v = other.direction2_v
    this.subType = other.subType
    this.type = other.type
    this.conditionType = other.conditionType
    if (other.conditions) {
      this.conditions = []
      for (const con of other.conditions) {
        const conC = Object.assign({}, con)
        this.conditions.push(conC)
      }
    }
  }

  setVisible (show) {
    if (this.model) {
      this.model.visible = !!show
    }
  }

  /**
   * 设置行人的路径（非轨迹线）
   * @param route
   */
  setRoute (route) {
    this.endPosArr.splice(0)
    let point = {}
    if (route.realStartLat) {
      point.lon = (+route.realStartLon).toFixed(8)
      point.lat = (+route.realStartLat).toFixed(8)
    } else {
      point.lon = (+route.startLon).toFixed(8)
      point.lat = (+route.startLat).toFixed(8)
    }

    point.alt = 0
    this.endPosArr.push(point)

    if (route.endLon > -181) {
      point = {}
      point.lon = (+route.endLon).toFixed(8)
      point.lat = (+route.endLat).toFixed(8)
      point.alt = 0
      this.endPosArr.push(point)
    }
  }

  pedestrianFormData () {
    const data = {}

    this.copyToPedestrianFormData(data)

    return data
  }

  /**
   * 复制对象到表单数据
   * @param objectInfo
   */
  copyToPedestrianFormData (objectInfo) {
    objectInfo.id = this.id
    objectInfo.routeId = this.routeId
    objectInfo.laneId = this.laneId
    objectInfo.startShift = this.startShift
    objectInfo.offset = this.startOffset
    objectInfo.startTime = this.startTime
    objectInfo.endTime = this.endTime
    objectInfo.directionOne = this.direction1
    objectInfo.durationOne = this.direction1_t
    objectInfo.velocityOne = this.direction1_v
    objectInfo.directionTwo = this.direction2
    objectInfo.durationTwo = this.direction2_t
    objectInfo.velocityTwo = this.direction2_v
    objectInfo.index = this.index
    objectInfo.subType = this.subType
    objectInfo.trigger = {}
    objectInfo.trigger.conditionType = this.conditionType
    objectInfo.trigger.conditionList = []
    if (this.conditions) {
      this.conditions.forEach((value) => {
        objectInfo.trigger.conditionList.push(Object.assign({}, value))
      })
    }

    objectInfo.endPosArr = []
    this.endPosArr.forEach((value, idx, arr) => {
      const point = {}
      point.lon = value.lon
      point.lat = value.lat
      point.alt = value.alt
      objectInfo.endPosArr.push(point)
    })
  }
}

/**
 * 默认行人数据
 * @type {{
 * offset: number, directionTwo: number, maxOffset: number,
 * trigger: {conditionList: *[], conditionType: string},
 * startShift: number, endPosArr: *[], directionOne: number,
 * laneId: string, routeId: string, durationOne: number,
 * durationTwo: number, velocityTwo: number, startTime: number,
 * maxShift: number, subType: string, endTime: number, velocityOne: number,
 * }}
 */
Pedestrian.defaultPedestrianData = {
  routeId: '',
  laneId: '',
  startShift: 0,
  offset: 0,
  startTime: 0,
  endTime: 13,
  directionOne: 0,
  durationOne: 3,
  velocityOne: 0.8,
  directionTwo: 90,
  durationTwo: 10,
  velocityTwo: 1.5,
  maxShift: 10,
  maxOffset: 2,
  subType: 'human',
  endPosArr: [],
  trigger: {
    conditionType: '',
    conditionList: [],
  },
}

export default Pedestrian
