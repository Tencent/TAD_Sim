import { cloneDeep } from 'lodash-es'
import { DistanceMode, TriggerCondition } from '../common/Constant'
import { VehicleBehaviorType } from './Vehicle'
import { MessageBox } from './MessageBox'
import { getPedestrianSubType } from '@/common/utils'
import i18n from '@/locales'
import store from '@/store'

class PedestrianV2 {
  constructor () {
    this.id = ''
    this.routeId = ''
    this.laneId = ''
    this.startShift = 0.0
    this.startOffset = 0.0
    this.startTime = 0.0

    this._behavior = 'UserDefine'
    this._subType = '' // is a number
    this.startVelocity = 0.0
    this.maxVelocity = 1
    this.conditions = []
    this.directions = []
    this.velocities = []
    this.counts = []
    this.endPosArr = []
    this.type = 'pedestrian'
    this.eventId = ''
    this.triggers = []
    this.angle = 0
    this.start_angle = ''
    this.model = null
    this.boundingBox = null
  }

  /**
   * copy pedestrian data without model
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.routeId = other.routeId
    this.laneId = other.laneId
    this.startShift = other.startShift
    this.startOffset = other.startOffset
    this.startAlt = other.startAlt
    this.startTime = other.startTime
    this.subType = other.subType
    this.type = other.type
    this.behavior = other.behavior
    this.startVelocity = other.startVelocity
    this.maxVelocity = other.maxVelocity
    this.eventId = other.eventId
    this.triggers = other.triggers
    this.angle = other.angle
    this.start_angle = other.start_angle
    this.boundingBox = other.boundingBox

    if (other.conditions) {
      this.conditions = cloneDeep(other.conditions)
    }
    if (other.directions) {
      this.directions = cloneDeep(other.directions)
    }
    if (other.velocities) {
      this.velocities = cloneDeep(other.velocities)
    }
    if (other.counts) {
      this.counts = cloneDeep(other.counts)
    }
    this.endPosArr = cloneDeep(other.endPosArr)
    if (this.route) {
      this.route.updateEndPositions(other.endPosArr)
    }
  }

  setVisible (show) {
    if (this.model) {
      this.model.visible = !!show
    }
  }

  setRoute (route) {
    this.endPosArr = cloneDeep(route.endPosArr)
    if (route) {
      route.host = this
    }
    this.route = route
  }

  /**
   * 复制自己的事件数据
   * @param pedestrianDefData
   */
  convertEventData2Array (pedestrianDefData) {
    if (this.conditions) {
      pedestrianDefData.conditions = cloneDeep(this.conditions)
    }
    if (this.directions) {
      pedestrianDefData.directions = cloneDeep(this.directions)
    }
    if (this.velocities) {
      pedestrianDefData.velocities = cloneDeep(this.velocities)
    }
    if (this.counts) {
      pedestrianDefData.counts = cloneDeep(this.counts)
    }
  }

  /**
   * 将Condition转换为merge数据
   * @param pedestrianDefData
   */
  convertConditionToMerge (pedestrianDefData) {
    const {
      conditions,
      directions,
      velocities,
      counts,
    } = pedestrianDefData

    if (conditions.length !== counts.length || conditions.length !== velocities.length) {
      console.error('condition length error!')
    }

    const timeList = []
    const conditionList = []
    conditions.forEach(({ type, value, mode }, i) => {
      if (type === TriggerCondition.TimeAbsolute) {
        timeList.push({
          time: value,
          velocity: velocities[i],
          direction: directions[i],
        })
      } else {
        conditionList.push({
          type,
          value,
          distancemode: mode,
          velocity: velocities[i],
          direction: directions[i],
          times: counts[i],
        })
      }
    })

    pedestrianDefData.trigger = {
      timeList,
      conditionList,
    }
  }

  /**
   * 内部行人数据结构
   * @return {{}}
   */
  pedestrianFormData () {
    const data = {}
    data.id = this.id
    data.routeId = this.routeId
    data.laneId = this.laneId
    data.startShift = this.startShift
    data.startVelocity = this.startVelocity
    data.maxVelocity = this.maxVelocity
    data.behavior = this.behavior
    data.offset = this.startOffset
    data.index = this.index
    data.subType = this.subType
    data.type = this.type
    data.startAlt = this.startAlt
    data.eventId = this.eventId
    data.triggers = this.triggers
    data.angle = this.angle
    data.start_angle = this.start_angle
    data.boundingBox = this.boundingBox
    this.convertEventData2Array(data)
    this.convertConditionToMerge(data)
    data.endPosArr = cloneDeep(this.endPosArr)
    return data
  }

  get subType () {
    return this._subType
  }

  set subType (subType) {
    this._subType = subType
    this.type = getPedestrianSubType(subType)
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

  /**
   * 从后端数据中设置行人数据
   * @param data
   * @param store
   */
  setFromBackEndData (data, store) {
    this.id = data.id
    this.routeId = data.routeID
    this.laneId = data.laneID
    this.startShift = data.start_s
    this.startTime = data.start_t
    this.startVelocity = data.start_v
    this.maxVelocity = data.max_v
    this.behavior = data.behavior
    this.eventId = data.eventId || ''
    this.startOffset = data.l_offset
    this.startAlt = data.start_alt || 0
    this.angle = data.angle
    this.start_angle = data.start_angle
    this.subType = data.subType
    this.boundingBox = data.boundingBox

    if (!this.subType) {
      data.subType = 'human'
    }

    if (data.conditionsV2?.length) {
      this.conditions = cloneDeep(data.conditionsV2)
    }

    if (data.directions?.length) {
      this.directions = cloneDeep(data.directions)
    }

    if (data.velocities?.length) {
      this.velocities = cloneDeep(data.velocities)
    }

    if (data.counts?.length) {
      this.counts = cloneDeep(data.counts)
    }

    if (!(this.behavior in VehicleBehaviorType)) {
      let msg = 'pedestrian behavior error!'
      console.error(msg)
      msg = i18n.t('tips.pedestrianFormatChanged')
      MessageBox.promptEditorUIMessage('error', msg)
      this.behavior = VehicleBehaviorType.TrafficVehicle
    }

    this.triggers = store.sceneevents.filter(trigger => this.eventId.split(',').includes(trigger.id))
  }

  /**
   * 创建一个默认行人数据
   * @param type
   * @return {{
   * velocities: *[], eventId: string, boundingBox: BoundingBox,
   * offset: number, counts: *[], start_angle: string, maxOffset: number,
   * trigger: {timeList: *[], conditionList: *[]}, triggers: *[],
   * startShift: number, endPosArr: *[], laneId: string, routeId: string,
   * startVelocity: number, angle: number, maxVelocity: number,
   * startTime: number, maxShift: number, subType: string, behavior: string,
   * conditions: *[],
   * }}
   */
  static createPedestrianData (type) {
    const { vehicleList, pedestrianList } = store.state.catalogs
    let catalog = vehicleList.find(item => item.variable === type)
    let boundingBox
    if (catalog) {
      boundingBox = catalog.catalogParams[0].boundingBox
    } else {
      catalog = pedestrianList.find(item => item.variable === type)
      boundingBox = catalog.catalogParams.boundingBox
    }
    return {
      routeId: '',
      laneId: '',
      startShift: 0,
      offset: 0,
      angle: 0,
      startTime: 0,
      maxShift: 10,
      maxOffset: 2,
      startVelocity: 0.0,
      maxVelocity: 1,
      behavior: VehicleBehaviorType.TrajectoryFollow,
      subType: 'human',
      conditions: [],
      velocities: [],
      counts: [],
      endPosArr: [],
      triggers: [],
      eventId: '',
      start_angle: '',
      boundingBox: cloneDeep(boundingBox),
      trigger: {
        timeList: [],
        conditionList: [],
      },
    }
  }

  /**
   * 将trigger转换为condition
   * @param pedestrianDefData
   */
  static mergeTriggerToCondition (pedestrianDefData) {
    const conditions = []
    const directions = []
    const velocities = []
    const counts = []

    if (pedestrianDefData.trigger) {
      const timeList = pedestrianDefData.trigger.timeList
      // time
      if (timeList?.length) {
        timeList.forEach((value) => {
          const condition = {}
          condition.type = TriggerCondition.TimeAbsolute
          condition.value = value.time
          condition.mode = DistanceMode.None
          conditions.push(condition)
          directions.push(value.direction)
          velocities.push(value.velocity)
          counts.push(-1)
        })
      }

      const conditionList = pedestrianDefData.trigger.conditionList
      // condition
      if (conditionList?.length) {
        conditionList.forEach((value) => {
          const condition = {}
          condition.type = value.type
          condition.value = value.value
          condition.mode = value.distancemode
          conditions.push(condition)
          directions.push(value.direction)
          velocities.push(value.velocity)
          counts.push(value.times)
        })
      }
    }

    pedestrianDefData.conditions = conditions
    pedestrianDefData.directions = directions
    pedestrianDefData.velocities = velocities
    pedestrianDefData.counts = counts
  }
}

export default PedestrianV2
