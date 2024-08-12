import {
  CircleGeometry,
  Color,
  Group,
  LineSegments,
  Math as MathUtils,
  Mesh,
  MeshPhongMaterial,
  WireframeGeometry,
} from 'three'

/**
 * 激光雷达
 * @class Lidar
 * @extends Group
 */
class Lidar extends Group {
  /**
   * @param {number} upperFov
   * @param {number} lowerFov
   * @param {number} channels
   * @param {number} range
   * @param {number} [frequency]
   */
  constructor (upperFov, lowerFov, channels, range, frequency = 1) {
    super()

    this.upperFov = upperFov
    this.lowerFov = lowerFov
    this.channels = channels
    this.range = range
    this.frequency = frequency

    const {
      circle: circleGeometry,
      line: lineGeometry,
    } = this.createGeometry()
    const circleMaterial = new MeshPhongMaterial({
      color: 0xFFFFFF,
      emissive: 0xFFFFFF,
      side: 2,
      flatShading: true,
      transparent: true,
      opacity: 0.1,
    })

    const lines = new LineSegments(lineGeometry)

    lines.material.transparent = true
    lines.material.opacity = 0.3
    lines.material.depthTest = true
    lines.material.color = new Color(0xFFFFFF)
    const circle = new Mesh(circleGeometry, circleMaterial)

    this.add(circle)
    this.add(lines)
  }

  createGeometry () {
    const {
      upperFov,
      lowerFov,
      range,
      channels,
    } = this

    const thetaStart = MathUtils.DEG2RAD * lowerFov
    const thetaLength = MathUtils.DEG2RAD * (upperFov - lowerFov)

    const circle = new CircleGeometry(range, channels - 1, thetaStart, thetaLength)
    const line = new WireframeGeometry(circle)

    return {
      circle,
      line,
    }
  }

  /**
   * 更新
   */
  update () {
    const {
      circle: circleGeometry,
      line: lineGeometry,
    } = this.createGeometry()

    this.children[0].geometry.dispose()
    this.children[1].geometry.dispose()

    this.children[0].geometry = circleGeometry
    this.children[1].geometry = lineGeometry
  }

  dispose () {
    this.children[0].geometry.dispose()
    this.children[1].geometry.dispose()
  }
}

export default Lidar
