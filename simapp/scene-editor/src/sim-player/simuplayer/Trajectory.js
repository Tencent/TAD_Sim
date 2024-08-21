import {
  BufferAttribute,
  BufferGeometry,
  Line,
  LineBasicMaterial,
  Mesh,
  MeshBasicMaterial,
  Uint16BufferAttribute,
  Vector3,
} from 'three'

// 最大轨迹点数
const maxTrajectoryPointCount = 400
const curvePointCount = maxTrajectoryPointCount
const lineZ = 0.8
const planeZ = 0.5
const planeWidth = 0.4

// 默认材质
const lineMaterial = new LineBasicMaterial({
  color: 0x00FF00,
})
const planeMaterial = new MeshBasicMaterial({
  color: 0x00C1D0,
  opacity: 0.9,
})

planeMaterial.transparent = true

const planeMaterialTransparent = new MeshBasicMaterial({
  color: 0x00C1D0,
  opacity: 0.6,
})

planeMaterialTransparent.transparent = true

/**
 * 生成规划线mesh
 * @return {{plane: Mesh, line: Line}}
 */
function getMesh () {
  const planeFloat32Array = new Float32Array(curvePointCount * 3 * 2)
  const planeBufferAttribute = new BufferAttribute(planeFloat32Array, 3)
  const planeGeometry = new BufferGeometry()
  const planeIndex = getPlaneGeometryIndex(curvePointCount * 2)

  const lineFloat32Array = new Float32Array(curvePointCount * 3)
  const lineBufferAttribute = new BufferAttribute(lineFloat32Array, 3)
  const lineGeometry = new BufferGeometry()

  planeGeometry.setIndex(planeIndex)
  planeGeometry.setAttribute('position', planeBufferAttribute)
  lineGeometry.setAttribute('position', lineBufferAttribute)

  const plane = new Mesh(planeGeometry, planeMaterial)
  const line = new Line(lineGeometry, lineMaterial)

  return {
    plane,
    line,
  }
}

/**
 * 生成平面索引
 * @param count
 * @return {Uint16BufferAttribute}
 */
function getPlaneGeometryIndex (count) {
  const index = []

  for (let i = 4; i <= count; i = i + 2) {
    index.push(i - 4, i - 3, i - 2)
    index.push(i - 2, i - 3, i - 1)
  }

  return new Uint16BufferAttribute(index, 1)
}

/**
 * 生成平面顶点
 * @param vertices
 * @return {Float32Array}
 */
function getPlaneGeometryPosition (vertices) {
  const { length } = vertices
  const dirZ = new Vector3(0, 0, 1)
  const position = []

  for (let i = 3; i < length; i = i + 3) {
    const prevX = vertices[i - 3]
    const prevY = vertices[i - 2]
    const prevZ = vertices[i - 1]
    const currX = vertices[i]
    const currY = vertices[i + 1]
    const currZ = vertices[i + 2]
    const dir = new Vector3(currX - prevX, currY - prevY, currZ - prevZ)

    dir.cross(dirZ).normalize().multiplyScalar(planeWidth / 2)

    const { x, y, z } = dir

    if (i === 3) {
      position.push(prevX - x, prevY - y, prevZ - z)
      position.push(prevX + x, prevY + y, prevZ + z)
    }

    position.push(currX - x, currY - y, currZ - z)
    position.push(currX + x, currY + y, currZ + z)
  }

  return position
}

/**
 * 更新几何体
 * @param mesh
 * @param vertices
 * @param drawCount
 */
function updateGeometry (mesh, vertices, drawCount) {
  if (mesh) {
    const { geometry } = mesh
    const position = geometry.getAttribute('position')
    const { array } = position

    geometry.setDrawRange(0, drawCount)

    vertices.forEach((vertex, index) => {
      array[index] = vertex
    })

    position.needsUpdate = true
    // geometry.computeBoundingBox();
    geometry.computeBoundingSphere()
  }
}

/**
 * 更新平面几何体
 * @param mesh
 * @param vertices
 */
function updatePlaneGeometry (mesh, vertices) {
  const extendedVertices = getPlaneGeometryPosition(vertices)
  const extendedDrawCount = (extendedVertices.length / 3 - 2) * 3

  updateGeometry(mesh, extendedVertices, extendedDrawCount)
}

/**
 * 轨迹线类
 */
class Trajectory {
  constructor (scene, coordinateConvert) {
    const { plane, line } = getMesh()

    this.scene = scene
    this.coordinateConvert = coordinateConvert
    this.plane = plane
    this.line = line
    this.transparent = false

    this.reset()
    this.scene.add(this.plane)
    this.scene.add(this.line)
  }

  /**
   * 渲染轨迹，采样并更新Geometry
   * @param data
   */
  render (data = []) {
    if (data.length < 2) {
      console.warn('trajectory.render data.length < 2')
      return
    }

    const points = data.slice(0, maxTrajectoryPointCount).map((point) => {
      return new Vector3(point.x, point.y, point.z)
    })
    const planeVertices = []
    const lineVertices = []

    points.forEach((point) => {
      const [x, y] = this.coordinateConvert.lonlat2xyz(point.x, point.y, 0)

      planeVertices.push(x, y, planeZ)
      lineVertices.push(x, y, lineZ)
    })

    updatePlaneGeometry(this.plane, planeVertices)
    updateGeometry(this.line, lineVertices, lineVertices.length / 3)
  }

  setVisible (visible) {
    this.plane.visible = visible
    this.line.visible = visible
  }

  reset () {
    this.line.geometry.setDrawRange(0, 0)
    this.plane.geometry.setDrawRange(0, 0)
  }

  dispose () {
    this.plane.geometry.dispose()
    this.line.geometry.dispose()
    this.scene.remove(this.plane)
    this.scene.remove(this.line)
  }
}

export default Trajectory
