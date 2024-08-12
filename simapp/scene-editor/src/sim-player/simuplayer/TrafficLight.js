import { Color, Group, IcosahedronBufferGeometry, Mesh, MeshPhongMaterial, Vector3 } from 'three'

const halfPI = Math.PI / 2

/**
 * @class TrafficLight
 * @description 交通灯类
 */
class TrafficLight {
  constructor () {
    this.id = -1
    this.x = 0
    this.y = 0
    this.heading = 0
    this.color = -1
    this.lon = 0
    this.lat = 0
    this.visible = true
    this.bordered = false
    this.disabled = false
    this.materials = null
    this.dir = new Vector3()
    this.phase = []
    this.model = new Group()
  }

  copy (o) {
    this.id = o.id
    this.x = o.x
    this.y = o.y
    this.heading = o.heading
    this.color = o.color
    this.lon = o.lon
    this.lat = o.lat
    this.visible = o.visible
  }

  /**
   * 创建3D模型，多个箭头同时创建
   * @param objects
   */
  createModels (objects) {
    const {
      geometry,
      ...materials
    } = objects
    this.materials = materials
    Object.values(materials).forEach((value) => {
      const { material, transparentMaterial } = value
      material.depthTest = false
      transparentMaterial.depthTest = false
      // material.depthWrite = false
      // transparentMaterial.depthWrite = false
    })
    const material = this._pickMaterial()
    if (this.phase.length) {
      // 多个方向放在一起，根据相位调整相对位置和朝向
      this.phase.forEach((p) => {
        const model = new Mesh(geometry, material)
        switch (p) {
          case TrafficLight.phaseMap.L:
            model.position.set(-2, 0, 0)
            model.rotation.set(0, 0, halfPI)
            break
          case TrafficLight.phaseMap.R:
            model.position.set(2, 0, 0)
            model.rotation.set(0, 0, -halfPI)
            break
          case TrafficLight.phaseMap.L0:
            model.position.set(-4, 0, 0)
            model.rotation.set(0, 0, Math.PI)
            break
          case TrafficLight.phaseMap.R0:
            model.position.set(4, 0, 0)
            model.rotation.set(0, 0, -Math.PI)
            break
          default:
        }
        this.model.add(model)
      })
    } else {
      // 没有箭头的渲染一个球
      const noPhaseGeo = new IcosahedronBufferGeometry(1, 2)
      const noPhaseMesh = new Mesh(noPhaseGeo, material)
      this.model.add(noPhaseMesh)
    }
  }

  /**
   * 设置交通灯颜色
   * @param color
   */
  setColor (color) {
    // 模型没加载出来
    if (!this.materials) return
    color = +color
    this.color = color
    const mat = this._pickMaterial()
    this.model.traverse((m) => {
      m.material = mat
    })
  }

  /**
   * 根据颜色和相位选择材质
   * @return {*|null}
   * @private
   */
  _pickMaterial () {
    const { color: _color, bordered, disabled, phase } = this
    let color = 'green'
    const { colorMap } = TrafficLight
    switch (+_color) {
      case colorMap.green:
        color = 'green'
        break
      case colorMap.yellow:
        color = 'yellow'
        break
      case colorMap.red:
        color = 'red'
        break
      case colorMap.gray:
        color = 'gray'
        break
      default:
      // todo: hide self
        return null
    }

    if (!phase.length) {
      // 无相位返回一个普通的颜色
      return TrafficLight.colorMaterialMap[color]
    }

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
   * 从网络数据中拷贝
   * @param l
   */
  copyFromNet (l) {
    this.id = l.id
    this.lon = l.x
    this.lat = l.y
    this.heading = l.heading
    this.color = l.color || 0
    this.phase = l.controlPhases || []
  }
}

TrafficLight.phaseMap = {
  U: 0,
  L: 1,
  T: 2,
  R: 3,
  L0: 4,
  R0: 5,
}

TrafficLight.colorMap = {
  reserve: -1,
  green: 0,
  yellow: 1,
  red: 2,
  gray: 3,
}

TrafficLight.colorMaterialMap = {
  green: new MeshPhongMaterial({ color: new Color('green') }),
  yellow: new MeshPhongMaterial({ color: new Color('yellow') }),
  red: new MeshPhongMaterial({ color: new Color('red') }),
  gray: new MeshPhongMaterial({ color: new Color('gray') }),
}

export default TrafficLight
