import { MathUtils } from 'three'
import { cloneDeep, isNil } from 'lodash-es'

class Planner {
  constructor (simuScene) {
    this.simuScene = simuScene
    this.type = 'car'
    this.name = ''
    this.id = 0
    this.routeId = 0
    this.startVelocity = 0.0
    this.maxVelocity = 0.0
    this.start_angle = ''
    this.unrealLevelIndex = 0

    this.laneId = 0
    this.startShift = 0
    this.startOffset = 0
    this.endLon = -999
    this.endLat = -999
    this.startAlt = -9999
    this.endAlt = -9999

    // 规划线采样间隔
    this.sampleInterval = 0.1
    this.controlTrack = []
    this.endPosArr = []
    this.sceneevents = []
    this.control_longitudinal = true
    this.control_lateral = true
    this.trajectory_enabled = false
    this.acceleration_max = 12
    this.deceleration_max = 10

    this.model = null
    this.route = null
    this.boundingBox = null
  }

  /**
   * 复制另一个planner数据，不复制模型
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.type = other.type
    this.name = other.name
    this.routeId = other.routeId
    this.startVelocity = other.startVelocity
    this.maxVelocity = other.maxVelocity
    this.start_angle = other.start_angle
    this.unrealLevelIndex = other.unrealLevelIndex
    this.laneId = other.laneId
    this.startShift = other.startShift
    this.startOffset = other.startOffset
    this.endLon = other.endLon
    this.endLat = other.endLat
    this.startAlt = other.startAlt
    this.endAlt = other.endAlt
    this.sampleInterval = other.sampleInterval
    this.trajectory_enabled = other.trajectory_enabled
    this.sceneevents = other.sceneevents || []
    this.control_longitudinal = other.control_longitudinal
    this.control_lateral = other.control_lateral
    this.acceleration_max = other.acceleration_max
    this.deceleration_max = other.deceleration_max
    this.boundingBox = other.boundingBox

    // 单独处理轨迹点
    this.endPosArr = cloneDeep(other.endPosArr)
    if (this.route) {
      this.route.updateEndPositions(this.endPosArr)
      this.route.curve?.setTrackEnabled(!!this.trajectory_enabled)
    }
  }

  setVisible (show) {
    if (this.model) {
      this.model.visible = !!show
    }
  }

  /**
   * 生成一份planner数据
   * @return {{
   * sampleInterval: number, boundingBox: null,
   * sceneevents: [], endLon: number, start_angle: string,
   * control_longitudinal: boolean, deceleration_max: number,
   * type: string, startShift: number, endPosArr: [],
   * laneId: number, endLat: number, control_lateral: boolean,
   * routeId: number, startOffset: number, startVelocity: number,
   * trajectory_enabled: boolean, name: string, endAlt: number,
   * maxVelocity: number, acceleration_max: number, subType: string,
   * id: number, startAlt: number,
   * }}
   */
  plannerFormData () {
    return {
      id: this.id,
      name: this.name,
      maxVelocity: this.maxVelocity,
      routeId: this.routeId,
      start_angle: this.start_angle,
      startVelocity: this.startVelocity,
      startShift: this.startShift,
      startOffset: this.startOffset,
      laneId: this.laneId,
      sampleInterval: this.sampleInterval,
      endLon: this.endLon,
      endLat: this.endLat,
      endAlt: this.endAlt,
      startAlt: this.startAlt,
      sceneevents: this.sceneevents,
      control_longitudinal: this.control_longitudinal,
      control_lateral: this.control_lateral,
      acceleration_max: this.acceleration_max,
      deceleration_max: this.deceleration_max,
      trajectory_enabled: this.trajectory_enabled,
      subType: this.type,
      type: 'planner',
      endPosArr: cloneDeep(this.endPosArr),
      boundingBox: this.boundingBox,
    }
  }

  /**
   * 从后端数据生成planner数据
   * @param data
   * @return {Planner}
   */
  setFromBackEndData (data) {
    // 处理多主车前缀
    const egoMatchResult = /Ego_(\d{3})/.exec(data.group)
    if (egoMatchResult?.length) {
      this.id = Number.parseInt(egoMatchResult[1])
    } else {
      this.id = 1
      console.warn(`Wrong planner group: ${data.group}`)
    }
    this.type = data.type
    this.name = data.name
    this.routeId = data.routeID
    this.start_angle = data.theta * MathUtils.RAD2DEG
    this.startVelocity = data.start_v
    this.maxVelocity = data.Velocity_Max
    this.sceneevents = data.sceneevents
    this.control_longitudinal = data.control_longitudinal
    this.control_lateral = data.control_lateral
    this.acceleration_max = data.acceleration_max
    this.deceleration_max = data.deceleration_max
    this.trajectory_enabled = data.trajectory_enabled
    this.boundingBox = data.boundingBox

    if (!isNil(data.alt_start)) {
      this.startAlt = +data.alt_start
    }
    if (!isNil(data.alt_end)) {
      this.endAlt = +data.alt_end
    }
    if (!isNil(data.pathSampleInterval)) {
      this.sampleInterval = +data.pathSampleInterval
    } else {
      this.sampleInterval = 10
    }
    return this
  }

  /**
   * 将内部数据转换为后端数据
   * @return {{
   * boundingBox: *, controlPath: *, sceneevents: *,
   * control_longitudinal: *, deceleration_max: string,
   * type: *, theta: number, start_v: string,
   * control_lateral: *, routeID: *, inputPath: *,
   * pathSampleInterval: string, trajectory_enabled: *,
   * name: *, Velocity_Max: string, acceleration_max: string,
   * group: string,
   * }}
   */
  toBackEndData () {
    const {
      id,
      type,
      name,
      routeId,
      startVelocity,
      start_angle,
      maxVelocity,
      sampleInterval,
      sceneevents,
      control_longitudinal,
      control_lateral,
      acceleration_max,
      deceleration_max,
      trajectory_enabled,
      endPosArr,
      startAlt,
      endAlt,
      route,
      simuScene,
      boundingBox,
    } = this
    const data = {
      group: `Ego_${`${id}`.padStart(3, '0')}`,
      type,
      name,
      routeID: routeId,
      start_v: `${startVelocity}`,
      theta: start_angle * MathUtils.DEG2RAD,
      Velocity_Max: `${maxVelocity}`,
      pathSampleInterval: `${sampleInterval}`,
      sceneevents,
      control_longitudinal,
      control_lateral,
      acceleration_max: `${acceleration_max}`,
      deceleration_max: `${deceleration_max}`,
      trajectory_enabled,
      boundingBox,
      // data path
      inputPath: endPosArr.map(({ lon, lat, alt, velocity, gear }) => ({
        lon: `${lon}`,
        lat: `${lat}`,
        alt: `${alt}`,
        velocity: `${velocity}`,
        gear,
      })),
      // todo: control path 对一下接口
      controlPath: endPosArr.map(({ lon, lat, alt, velocity, gear, accs, frontwheel, heading }) => ({
        lon: `${lon}`,
        lat: `${lat}`,
        alt: `${alt}`,
        velocity: `${velocity}`,
        gear,
        accs: isNil(accs) ? null : `${accs}`,
        heading: isNil(heading) ? null : `${heading}`,
        frontwheel: isNil(frontwheel) ? null : `${frontwheel}`,
      })),
    }

    if (startAlt > -9999) {
      data.alt_start = `${startAlt}`
    }

    if (endAlt > -9999) {
      data.alt_end = `${endAlt}`
    }

    if (data.inputPath.length) {
      data.alt_end = data.inputPath[data.inputPath.length - 1].alt
    }

    // 没有设置轨迹点
    if (data.inputPath.length === 0) {
      const {
        startLon,
        startLat,
      } = route
      data.inputPath.push({
        lon: +startLon,
        lat: +startLat,
        alt: +startAlt,
      })
    }

    // 轨迹跟踪情况下单独计算一下采样点
    let track = []
    if (trajectory_enabled) {
      track = route.curve.track?.generateTrackPointByInterval(sampleInterval, simuScene.ct) || []
    }
    data.controlTrack = track
    return data
  }

  setRoute (route) {
    this.endPosArr = cloneDeep(route.endPosArr)
    if (route) {
      route.host = this
    }
    this.route = route
  }

  /**
   * 创建planner数据
   * @return {{
   * sampleInterval: number,
   * boundingBox: {
   * center: {x: number, y: number, z: number},
   * dimensions: {length: number, width: number, height: number}},
   * sceneevents: *[], endLon: number, start_angle: string,
   * control_longitudinal: boolean, deceleration_max: number,
   * type: string, startShift: number, endPosArr: *[],
   * laneId: string, endLat: number, control_lateral: boolean,
   * routeId: string, startOffset: number, startVelocity: number,
   * trajectory_enabled: boolean, endAlt: number, name: string,
   * maxVelocity: number, acceleration_max: number,
   * }}
   */
  static createPlannerData () {
    return {
      routeId: '',
      startVelocity: 0,
      start_angle: '',
      maxVelocity: 20,
      startShift: 0,
      startOffset: 0,
      laneId: '',
      endLon: -999,
      endLat: -999,
      endAlt: -999,
      sampleInterval: 0.1,
      type: 'car',
      name: '',
      endPosArr: [],
      sceneevents: [],
      control_longitudinal: true,
      control_lateral: true,
      acceleration_max: 12,
      deceleration_max: 10,
      trajectory_enabled: false,
      boundingBox: {
        center: {
          x: 1,
          y: 1,
          z: 1,
        },
        dimensions: {
          height: 1,
          length: 1,
          width: 1,
        },
      },
    }
  }

  get trajectoryEnabled () {
    return this.trajectory_enabled
  }
}

export default Planner
