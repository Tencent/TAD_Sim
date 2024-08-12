import { cloneDeep } from 'lodash-es'
import { AccelerationTerminationType, DistanceMode, TriggerCondition } from '../common/Constant'
import VelocityData from './VelocityData'
import { MessageBox } from './MessageBox'
import store from '@/store'

/**
 * 对比条件函数
 * @param a
 * @param b
 * @return {number}
 */
function compareFunc (a, b) {
  if (a.condition.type < b.condition.type) {
    return -1
  }

  if (a.condition.type > b.condition.type) {
    return 1
  }

  if (a.condition.mode < b.condition.mode) {
    return -1
  }

  if (a.condition.mode > b.condition.mode) {
    return 1
  }

  return a.condition.value - b.condition.value
}

/**
 * 车辆类型
 */
class Vehicle {
  constructor () {
    this.id = ''
    this.routeId = ''
    this.laneId = ''
    this.accId = ''
    this.mergeId = ''
    this.type = 0
    this.startShift = 0.0
    this.startTime = 0.0
    this.startVelocity = 0.0
    this.startOffset = 0.0
    this.maxVelocity = 0.0
    this.length = 4.5
    this.width = 2
    this.height = 1.8
    this._behavior = 'TrafficVehicle'
    this.aggress = 0.5
    this.follow = ''
    this.endPosArr = []
    this.triggers = []
    this.aiModel = 'TrafficVehicleDefault'

    this.route = null // 关联的路径
    this.model = null
    this.boundingBox = null

    this.angle = 0
    this.start_angle = ''
    this.eventId = ''
    this.sensorGroup = ''
    this.obuStatus = '0'
  }

  /**
   * 复制数据
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.routeId = other.routeId
    this.laneId = other.laneId
    this.accId = other.accId
    this.mergeId = other.mergeId
    this.type = other.type
    this.startShift = other.startShift
    this.startTime = other.startTime
    this.startVelocity = other.startVelocity
    this.startOffset = other.startOffset
    this.maxVelocity = other.maxVelocity
    this.length = other.length
    this.width = other.width
    this.height = other.height
    this.behavior = other.behavior
    this.aggress = other.aggress
    this.follow = other.follow
    this.startAlt = other.startAlt
    this.aiModel = other.aiModel
    this.eventId = other.eventId || ''
    this.sensorGroup = other.sensorGroup
    this.obuStatus = other.obuStatus
    this.start_angle = other.start_angle
    this.boundingBox = other.boundingBox
    this.triggers = other.triggers || []
    this.angle = other.angle || 0
    this.endPosArr = cloneDeep(other.endPosArr)
    if (this.route) {
      this.route.updateEndPositions(this.endPosArr)
    }
  }

  /**
   * 设置可见
   * @param show
   */
  setVisible (show) {
    if (this.model) {
      this.model.visible = !!show
    }
  }

  /**
   * 设置路径
   * @param route
   */
  setRoute (route) {
    this.endPosArr = cloneDeep(route.endPosArr)

    if (route) {
      route.host = this
    }
    this.route = route
  }

  /**
   * 生成默认form数据
   * @return {{
   * boundingBox: null,
   * fittingStrategy: string,
   * carWidth: number,
   * start_angle: string,
   * startShift: number,
   * acceleration: string,
   * routeId: string,
   * carType: number,
   * startVelocity: number,
   * carLength: number,
   * angle: number,
   * startTime: number,
   * id: string,
   * startAlt: (*|number),
   * sensorGroup: string,
   * behavior: string,
   * obuStatus: string,
   * eventId: string,
   * index: *,
   * follow: string,
   * triggers: [],
   * laneId: string,
   * startOffset: number,
   * aggress: number,
   * carHeight: number,
   * maxVelocity: number,
   * aiModel: string
   * }}
   */
  carFormData () {
    const data = {
      id: this.id,
      routeId: this.routeId,
      laneId: this.laneId,
      acceleration: this.accId,
      fittingStrategy: this.mergeId,
      behavior: this.behavior,
      carType: this.type,
      startTime: this.startTime,
      startVelocity: this.startVelocity,
      startShift: this.startShift,
      startOffset: this.startOffset,
      carLength: this.length,
      carWidth: this.width,
      carHeight: this.height,
      maxVelocity: this.maxVelocity,
      aggress: this.aggress,
      index: this.index,
      follow: this.follow,
      startAlt: this.startAlt,
      aiModel: this.aiModel,
      angle: this.angle || 0,
      start_angle: this.start_angle,
      triggers: this.triggers,
      eventId: this.eventId,
      sensorGroup: this.sensorGroup,
      obuStatus: this.obuStatus,
      boundingBox: this.boundingBox,
    }

    if (!(data.behavior in VehicleBehaviorType)) {
      console.error('vehicle behaviorType error!', this.behavior)
      const msg = 'vehicle behaviorType error!'
      MessageBox.promptEditorUIMessage('error', msg)
      data.behavior = VehicleBehaviorType.TrafficVehicle
    }
    this.convertAccMergeData2Array(data)
    this.convertConditionToMerge(data)
    data.endPosArr = this.endPosArr.map(({ lon, lat, alt, velocity, gear }) => {
      const point = { lon, lat, alt }
      const v = velocity === undefined ? this.startVelocity : velocity
      point.velocity = Number.parseFloat(v)
      point.gear = gear === undefined ? 'drive' : gear
      return point
    })
    return data
  }

  convertAccMergeData2Array (carDefData) {
    const tmpConditions = []
    const tmpAccelerations = []
    const tmpDirections = []
    const tmpDurations = []
    const tmpOffsets = []
    const tmpCounts = []
    const tmpTerminations = []

    const acc = window.simuScene.sceneParser.findAccInMap(this.accId)
    const merge = window.simuScene.sceneParser.findMergeInMap(this.mergeId)

    // 加速度排序
    acc.nodes.sort(compareFunc)

    // 并道排序
    merge.merges.sort(compareFunc)

    let aiter = 0
    let miter = 0

    // 加速度和并道归并排序
    while (aiter < acc.nodes.length && miter < merge.merges.length) {
      if (compareFunc(acc.nodes[aiter], merge.merges[miter]) < 0) {
        tmpConditions.push(acc.nodes[aiter].condition)
        tmpAccelerations.push(acc.nodes[aiter].acc)
        tmpCounts.push(acc.nodes[aiter].count)
        tmpTerminations.push(acc.nodes[aiter].termination)
        tmpDirections.push('static')
        tmpDurations.push(0)
        tmpOffsets.push(0)
        aiter++
      } else if (compareFunc(acc.nodes[aiter], merge.merges[miter]) === 0) {
        tmpConditions.push(acc.nodes[aiter].condition)
        tmpAccelerations.push(acc.nodes[aiter].acc)
        tmpTerminations.push(acc.nodes[aiter].termination)
        tmpDirections.push(merge.merges[miter].direction)
        tmpDurations.push(merge.merges[miter].duration)
        tmpOffsets.push(merge.merges[miter].offset)
        tmpCounts.push(merge.merges[miter].count)
        aiter++
        miter++
      } else {
        tmpConditions.push(merge.merges[miter].condition)
        tmpDirections.push(merge.merges[miter].direction)
        tmpDurations.push(merge.merges[miter].duration)
        tmpOffsets.push(merge.merges[miter].offset)
        tmpCounts.push(merge.merges[miter].count)

        if (aiter === 0) {
          tmpAccelerations.push(0)
          tmpTerminations.push({ type: AccelerationTerminationType.None, value: 0 })
        } else {
          tmpAccelerations.push(acc.nodes[aiter - 1].acc)
          tmpTerminations.push(acc.nodes[aiter - 1].termination)
        }
        miter++
      }
    }

    // 加速度排完，并道还没排完
    if (acc.nodes.length === aiter && miter < merge.merges.length) {
      for (let i = miter; i < merge.merges.length; ++i) {
        tmpConditions.push(merge.merges[i].condition)
        tmpDirections.push(merge.merges[i].direction)
        tmpDurations.push(merge.merges[i].duration)
        tmpOffsets.push(merge.merges[i].offset)
        tmpCounts.push(merge.merges[i].count)
        if (acc.nodes.length === 0) {
          tmpAccelerations.push(0)
          tmpTerminations.push({ type: AccelerationTerminationType.None, value: 0 })
        } else {
          tmpAccelerations.push(acc.nodes[aiter - 1].acc)
          tmpTerminations.push(acc.nodes[aiter - 1].termination)
        }
      }

      // 并道排完，加速度还没排完
    } else if (aiter < acc.nodes.length && merge.merges.length === miter) {
      for (let i = aiter; i < acc.nodes.length; ++i) {
        tmpConditions.push(acc.nodes[i].condition)
        tmpAccelerations.push(acc.nodes[i].acc)
        tmpTerminations.push(acc.nodes[i].termination)
        tmpCounts.push(acc.nodes[i].count)
        tmpDirections.push('static')
        tmpDurations.push(0)
        tmpOffsets.push(0)
      }
    }

    // 速度参与归并排序
    carDefData.conditions = []
    carDefData.directions = []
    carDefData.accelerations = []
    carDefData.durations = []
    carDefData.offsets = []
    carDefData.counts = []
    carDefData.velocities = []
    carDefData.terminations = []

    let velocities = window.simuScene.sceneParser.findVelocityInMap(this.id)
    if (!velocities) {
      velocities = new VelocityData()
    }

    const compare2Func = (a, b) => {
      if (a.type < b.type) {
        return -1
      }

      if (a.type > b.type) {
        return 1
      }

      return a.value - b.value
    }

    // 速度排序
    velocities.nodes.sort(compareFunc)
    let amiter = 0
    let viter = 0
    while (amiter < tmpConditions.length && viter < velocities.nodes.length) {
      if (compare2Func(tmpConditions[amiter], velocities.nodes[viter].condition) < 0) {
        carDefData.velocities.push('')
        carDefData.conditions.push(tmpConditions[amiter])
        carDefData.directions.push(tmpDirections[amiter])
        carDefData.accelerations.push(tmpAccelerations[amiter])
        carDefData.terminations.push(tmpTerminations[amiter])
        carDefData.durations.push(tmpDurations[amiter])
        carDefData.offsets.push(tmpOffsets[amiter])
        carDefData.counts.push(tmpCounts[amiter])
        amiter++
      } else if (compare2Func(tmpConditions[amiter], velocities.nodes[viter].condition) === 0) {
        carDefData.velocities.push(velocities.nodes[viter].velocity)
        carDefData.conditions.push(tmpConditions[amiter])
        carDefData.directions.push(tmpDirections[amiter])
        carDefData.accelerations.push(tmpAccelerations[amiter])
        carDefData.terminations.push(tmpTerminations[amiter])
        carDefData.durations.push(tmpDurations[amiter])
        carDefData.offsets.push(tmpOffsets[amiter])
        carDefData.counts.push(tmpCounts[amiter])
        amiter++
        viter++
      } else {
        carDefData.conditions.push(velocities.nodes[viter].condition)
        carDefData.velocities.push(velocities.nodes[viter].velocity)
        carDefData.counts.push(velocities.nodes[viter].count)

        carDefData.accelerations.push(0)
        carDefData.terminations.push({ type: AccelerationTerminationType.None, value: 0 })
        carDefData.directions.push('static')
        carDefData.durations.push(0)
        carDefData.offsets.push(0)

        viter++
      }
    }

    // 加速度和并道已排完序，速度没排完
    if (tmpConditions.length === amiter && viter < velocities.nodes.length) {
      for (let i = viter; i < velocities.nodes.length; ++i) {
        carDefData.conditions.push(velocities.nodes[i].condition)
        carDefData.velocities.push(velocities.nodes[i].velocity)
        carDefData.counts.push(velocities.nodes[i].count)
        carDefData.accelerations.push(0)
        carDefData.terminations.push({ type: AccelerationTerminationType.None, value: 0 })
        carDefData.directions.push('static')
        carDefData.durations.push(0)
        carDefData.offsets.push(0)
      }

      // 速度没排完, 加速度和并道没有排完序
    } else if (amiter < tmpConditions.length && velocities.nodes.length === viter) {
      for (let i = amiter; i < tmpConditions.length; ++i) {
        carDefData.velocities.push('')
        carDefData.conditions.push(tmpConditions[amiter])
        carDefData.directions.push(tmpDirections[amiter])
        carDefData.accelerations.push(tmpAccelerations[amiter])
        carDefData.terminations.push(tmpTerminations[amiter])
        carDefData.durations.push(tmpDurations[amiter])
        carDefData.offsets.push(tmpOffsets[amiter])
        carDefData.counts.push(tmpCounts[amiter])
        amiter++
      }
    }
  }

  convertConditionToMerge (carDefData) {
    const {
      conditions,
      directions,
      accelerations,
      velocities,
      durations,
      offsets,
      counts,
      terminations,
    } = carDefData

    if (conditions.length !== directions.length ||
      conditions.length !== accelerations.length ||
      conditions.length !== offsets.length ||
      conditions.length !== counts.length ||
      conditions.length !== durations.length ||
      conditions.length !== velocities.length ||
      conditions.length !== terminations.length
    ) {
      console.error('condition length error!')
    }

    const timeList = []
    const conditionList = []

    conditions.forEach((condition, i) => {
      const direction = directions[i]
      const acceleration = accelerations[i]
      const velocity = velocities[i]
      const duration = durations[i]
      const offset = offsets[i]
      const count = counts[i]
      const termination = terminations[i]
      if (condition.type === TriggerCondition.TimeAbsolute) {
        timeList.push({
          time: condition.value,
          direction,
          acceleration,
          velocity,
          durations: duration,
          offsets: offset,
          accelerationTerminationType: termination.type,
          accelerationTerminationValue: termination.value,
        })
      } else {
        conditionList.push({
          type: condition.type,
          value: condition.value,
          distancemode: condition.mode,
          direction,
          acceleration,
          velocity,
          durations: duration,
          offsets: offset,
          times: count,
          accelerationTerminationType: termination.type,
          accelerationTerminationValue: termination.value,
        })
      }
    })

    carDefData.trigger = {
      timeList,
      conditionList,
    }
  }

  get behavior () {
    return this._behavior
  }

  set behavior (value) {
    this._behavior = value
    if (this.route?.curve) {
      this.route.curve.setTrackEnabled(value === VehicleBehaviorType.TrajectoryFollow)
    }
  }

  get trajectoryEnabled () {
    return this.behavior === VehicleBehaviorType.TrajectoryFollow
  }

  get customBehavior () {
    return this.behavior === VehicleBehaviorType.UserDefine || this.behavior === VehicleBehaviorType.TrajectoryFollow
  }

  setFromBackEndData (data, store) {
    this.id = data.id
    this.type = data.vehicleType
    this.routeId = data.routeID
    this.laneId = data.laneID
    this.accId = data.accID
    this.mergeId = data.mergeID
    this.behavior = data.behavior
    this.startShift = data.start_s
    this.startTime = data.start_t
    this.startVelocity = data.start_v
    this.startOffset = data.l_offset
    this.startAlt = data.start_alt || 0
    this.angle = data.angle || 0
    this.eventId = data.eventId
    this.follow = data.follow
    this.length = data.length
    this.width = data.width
    this.height = data.height
    this.aggress = data.aggress
    this.sensorGroup = data.sensorGroup
    this.obuStatus = data.obuStatus
    this.start_angle = data.start_angle
    this.boundingBox = data.boundingBox
    // behavior 请求返回的应该是字符串
    if (this.behavior in VehicleBehaviorType) {
      if (this.behavior === VehicleBehaviorType.TrafficVehicleArterial) {
        this.behavior = VehicleBehaviorType.TrafficVehicle
        this.aiModel = VehicleBehaviorType.TrafficVehicleArterial
      }
    } else {
      const msg = 'vehicle behavior error!'
      console.error(msg)
      MessageBox.promptEditorUIMessage('error', msg)
      this.behavior = VehicleBehaviorType.TrafficVehicle
    }
    if (data.max_v.length < 1) {
      const msg = `traffic vehicle ${this.id} lack of max velocity, use 12 m/s.`
      MessageBox.promptEditorUIMessage('warn', msg)
      this.maxVelocity = '12'
    } else {
      this.maxVelocity = data.max_v
    }

    this.triggers = store.sceneevents.filter(trigger => this.eventId.split(',').includes(trigger.id))

    if (this.accId.length === 0) {
      const msg = `vehicle ${this.id} acceleration is empty, use default acceleration`
      console.warn(msg)
      MessageBox.promptEditorUIMessage('error', msg)
      this.accId = store.getDefaultAccID()
    }

    if (this.mergeId.length === 0) {
      const msg = `vehicle ${this.id} merge is empty, use default merge`
      console.warn(msg)
      MessageBox.promptEditorUIMessage('error', msg)
      this.mergeId = store.getDefaultMergeID()
    }
    return this
  }

  static mergeTriggerToCondition (carDefData) {
    const conditions = []
    const directions = []
    const accelerations = []
    const velocities = []
    const durations = []
    const offsets = []
    const counts = []
    const terminations = []

    if (carDefData.trigger) {
      const timeList = carDefData.trigger.timeList
      // time
      if (timeList?.length) {
        timeList.forEach((value) => {
          conditions.push({
            type: TriggerCondition.TimeAbsolute,
            value: value.time,
            mode: DistanceMode.None,
          })
          directions.push(value.direction)
          accelerations.push(value.acceleration)
          velocities.push(value.velocity)
          durations.push(value.durations)
          offsets.push(value.offsets)
          counts.push(-1)
          terminations.push({
            type: value.accelerationTerminationType,
            value: value.accelerationTerminationValue,
          })
        })
      }

      const conditionList = carDefData.trigger.conditionList
      // conditon
      if (conditionList?.length) {
        conditionList.forEach((value) => {
          conditions.push({
            type: value.type,
            value: value.value,
            mode: value.distancemode,
          })
          directions.push(value.direction)
          accelerations.push(value.acceleration)
          velocities.push(value.velocity)
          durations.push(value.durations)
          offsets.push(value.offsets)
          counts.push(value.times)
          terminations.push({
            type: value.accelerationTerminationType,
            value: value.accelerationTerminationValue,
          })
        })
      }
    }
    carDefData.conditions = conditions
    carDefData.directions = directions
    carDefData.accelerations = accelerations
    carDefData.velocities = velocities
    carDefData.durations = durations
    carDefData.offsets = offsets
    carDefData.counts = counts
    carDefData.terminations = terminations
  }

  static createVehicleData (type) {
    const { vehicleList } = store.state.catalogs
    const vehicle = vehicleList.find(item => item.variable === type)
    return {
      routeId: '',
      laneId: '',
      acceleration: '',
      fittingStrategy: '',
      behavior: VehicleBehaviorType.TrafficVehicle,
      carType: 1,
      startShift: 0,
      startTime: 0,
      startOffset: 0,
      startVelocity: Vehicle.StartSpeed,
      carLength: 4.5,
      carHeight: 1.5,
      carWidth: 1.8,
      maxVelocity: 12,
      aggress: 0.5,
      maxShift: 10,
      maxOffset: 2,
      follow: '',
      conditions: [],
      directions: [],
      accelerations: [],
      velocities: [],
      endPosArr: [],
      trigger: {
        timeList: [],
        conditionList: [],
      },
      triggers: [],
      aiModel: 'TrafficVehicleDefault',
      angle: 0,
      start_angle: '',
      eventId: '',
      sensorGroup: '',
      obuStatus: '0',
      boundingBox: cloneDeep(vehicle.catalogParams[0].boundingBox),
    }
  }
}

export const VehicleBehaviorType = {
  UserDefine: 'UserDefine',
  TrajectoryFollow: 'TrajectoryFollow',
  TrafficVehicle: 'TrafficVehicle',
  TrafficVehicleArterial: 'TrafficVehicleArterial',
}

Vehicle.StartSpeed = 5

export default Vehicle
