import { Mesh, Vector3 } from 'three'
import { alignObjectByDir } from '../common/MeshUtils'
import Utility from './Utility'

const halfPI = Math.PI / 2

/**
 * @class Signlight
 * @description 信号灯类
 * @export
 */
class Signlight {
  constructor () {
    this.id = ''
    this.routeId = ''
    this.laneId = 0
    this.startShift = 0.0
    this.startTime = 0.0
    this.startOffset = 0.0
    this.direction = 0
    this.timeGreen = 0
    this.timeYellow = 0
    this.timeRed = 0
    this.enabled = true
    this.phase = 'A'
    this.lane = 'ALL'
    this.plan = '0'
    this.junction = undefined
    this.phaseNumber = undefined
    this.signalHead = []
    this.color = 'green'
    this.bordered = false
    this.disabled = false
    this.model = null
    this.materials = null
    this.dir = new Vector3()
    this.tolanelinkids = ''
  }

  /**
   * 复制一个信号灯对象，不包括model
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.routeId = other.routeId
    this.laneId = other.laneId
    this.startShift = other.startShift
    this.startTime = other.startTime
    this.startOffset = other.startOffset
    this.direction = other.direction
    this.timeGreen = other.timeGreen
    this.timeYellow = other.timeYellow
    this.timeRed = other.timeRed
    this.enabled = other.enabled
    this.lane = other.lane
    this.phase = other.phase
    this.startAlt = other.startAlt
    this.eventId = other.eventId
    this.plan = other.plan
    this.junction = other.junction
    this.phaseNumber = other.phaseNumber
    this.signalHead = other.signalHead
    this.tolanelinkids = other.tolanelinkids
  }

  /**
   * 创建三维模型
   * @param objects
   */
  createModel (objects) {
    const {
      geometry,
      ...materials
    } = objects
    this.materials = materials
    Object.values(materials).forEach((value) => {
      const { material, transparentMaterial } = value
      material.depthTest = false
      // material.depthWrite = false
      transparentMaterial.depthTest = false
      // transparentMaterial.depthWrite = false
    })
    const material = this._pickMaterial(this.color, this.bordered, this.disabled)
    this.model = new Mesh(geometry, material)
  }

  setVisible (show) {
    if (this.model) {
      this.model.visible = !!show
    }
  }

  /**
   * 根据phase设置方向
   * @param phase
   */
  setDirection (phase) {
    let d = 0
    switch (phase) {
      case 'L0':
      case 'R0':
        d = halfPI
        break
      case 'L':
        d = 0
        break
      case 'R':
        d = Math.PI
        break
      default:
        d = -halfPI
    }
    alignObjectByDir(this, this.dir, d)
  }

  addToScene (scene) {
    scene.add(this.model)
  }

  removeFromScene (scene) {
    scene.remove(this.model)
  }

  /**
   * 设置状态
   * @param color
   * @param bordered
   * @param disabled
   */
  setStatus ({ color = this.color, bordered = this.bordered, disabled = this.disabled }) {
    this.color = color
    this.bordered = bordered
    this.disabled = disabled
    const material = this._pickMaterial(color, bordered, disabled)
    if (this.model && material) {
      this.model.material = material
    }
  }

  /**
   * 根据颜色、是否有边框、是否禁用，选择材质
   * @param color
   * @param bordered
   * @param disabled
   * @return {*|null}
   * @private
   */
  _pickMaterial (color, bordered, disabled) {
    if (bordered) {
      color += 'Bordered'
    }
    if (this.materials) {
      const material = disabled ? this.materials[color]?.transparentMaterial : this.materials[color]?.material
      if (!material) {
        console.warn(`没有找到此种颜色的交通灯：color: ${color}, bordered: ${bordered}, disabled: ${disabled}`)
      }
      return material
    }
    console.warn('此交通灯还没有model')
    return null
  }

  /**
   * 计算当前需要展示的颜色
   * @return {string}
   */
  calculateColor () {
    const greenDuration = +this.timeGreen
    const yellowDuration = +this.timeYellow
    const redDuration = +this.timeRed
    let startTime = +this.startTime

    const sum = greenDuration + yellowDuration + redDuration

    if (startTime > sum) {
      startTime = startTime % sum
    }

    let color = 'gray'
    if (this.enabled == false) {
      color = 'gray'
    } else if (Utility.equalToZero(sum)) {
      // disable light gray
      color = 'gray'
    } else if (greenDuration > startTime) {
      color = 'green'
    } else if (greenDuration + yellowDuration > startTime) {
      color = 'yellow'
    } else {
      color = 'red'
    }
    return color
  }

  /**
   * 更新显示
   */
  updateDisplay () {
    const color = this.calculateColor()
    this.setStatus({ color })
    this.setDirection(this.phase)
  }

  /**
   * 转换为表单数据
   * @return {{}}
   */
  signlightFormData () {
    const data = {}
    this.copyToSignlightFormData(data)
    return data
  }

  copyToSignlightFormData (objectInfo) {
    objectInfo.id = this.id
    objectInfo.routeId = this.routeId
    objectInfo.laneId = this.laneId
    objectInfo.startShift = this.startShift
    objectInfo.offset = this.startOffset
    objectInfo.startTime = this.startTime
    objectInfo.direction = this.direction
    objectInfo.greenDuration = this.timeGreen
    objectInfo.yellowDuration = this.timeYellow
    objectInfo.redDuration = this.timeRed
    objectInfo.index = this.index
    objectInfo.enabled = this.enabled
    objectInfo.phase = this.phase
    objectInfo.startAlt = this.startAlt
    objectInfo.eventId = this.eventId
    objectInfo.plan = this.plan
    objectInfo.junction = this.junction
    objectInfo.phaseNumber = this.phaseNumber
    objectInfo.signalHead = this.signalHead
  }

  // 将合并的语义灯数据，拆为交互更方便的单个箭头
  getPhases () {
    const phasesStr = this.phase
    if (!phasesStr) return []
    if (phasesStr === 'A') return ['L', 'T', 'R', 'L0', 'R0']
    const phases = phasesStr.split(';')
    return phases
  }

  // 将合并的语义灯数据，拆为交互更方便的单个箭头语义灯
  divideByPhases (relativeLaneMap, baseId) {
    const sl = this
    const phases = sl.getPhases()
    const relativeLane = relativeLaneMap[sl.routeId]
    if (phases.length && relativeLane) {
      const newSignlights = []
      let count = 0
      const okRelativeLane = phases.map(phase => relativeLane.find(e => e.phase === phase)).filter(e => !!e)
      okRelativeLane.forEach((item, index) => {
        const id = index === 0 ? sl.id : `${baseId + count++}`
        const newSl = new Signlight()
        newSl.copyNoModel(sl)
        newSl.id = id
        newSl.phase = item.phase
        newSl.lane = item.lane ? item.lane.replace(/,/g, ';') : 'ALL'
        newSl.tolanelinkids = item.tolanelinkids
        newSignlights.push(newSl)
      })
      if (!newSignlights.length) {
        console.error('this signlight got wrong lane data: ', sl, relativeLaneMap)
      }
      return newSignlights
    }
    return [sl]
  }
}

/**
 * 默认信号灯数据
 * @type {{
 * phase: string, junction: undefined,
 * eventId: string, offset: number,
 * phaseNumber: undefined, startShift: number,
 * redDuration: number, enabled: boolean,
 * yellowDuration: number, laneId: number,
 * routeId: string, signalHead: *[],
 * startTime: number, greenDuration: number,
 * plan: string, direction: number
 * }}
 */
Signlight.defaultSignlightData = {
  routeId: '',
  laneId: 0,
  direction: 0,
  startShift: 0,
  offset: 0,
  startTime: 0,
  greenDuration: 30,
  yellowDuration: 3,
  redDuration: 30,
  phase: 'A',
  enabled: true,
  eventId: '',
  plan: '0',
  junction: undefined,
  phaseNumber: undefined,
  signalHead: [],
}

export default Signlight
