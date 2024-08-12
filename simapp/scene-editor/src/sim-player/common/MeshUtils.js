import { Matrix4, Quaternion, Vector2, Vector3 } from 'three'

/**
 *
 * @param {*} center
 * @param {*} length 应该是道路方向长度； s 方向长度
 * @param {*} width  t 方向长度
 * a, b 是道路的两个端点， 在s方向上， a,c是t方向上的两个端点
 *
 * a     c
 *
 * b     d
 *
 */
export function genAttributesByCenter (center, length, width) {
  const positions = []
  const offsets = []
  const indices = []

  const addVertex = (vertex, up, linesofar) => {
    positions.push(...vertex, up)
    offsets.push(linesofar)
  }

  const xOffset = width / 2 // u
  const yOffset = length / 2 // v

  const [x, y, z] = center

  const a = [x - xOffset, y + yOffset, z]
  const b = [x - xOffset, y - yOffset, z]
  const c = [x + xOffset, y + yOffset, z]
  const d = [x + xOffset, y - yOffset, z]

  addVertex(a, 1, length)
  addVertex(b, 1, 0)
  addVertex(c, -1, length)
  addVertex(d, -1, 0)

  indices.push(0, 1, 2, 2, 1, 3)

  return {
    positions,
    offsets,
    indices,
  }
}

const lengthMax = 1.2

let index1 = -1
let index2 = -1
let index3 = -1

let WIDTH = 10
let OFFSET_T = 0

/**
 *
 * @param {} vertex
 * @param {*} up
 * @param {*} normalX
 * @param {*} normalY
 * @param {*} positions
 * @param {*} offsets
 * @param {*} indices
 * @param {*} distance
 * @param {*} offset 用来校正linesofar
 */
function addVertex (vertex, up, normalX, normalY, positions, offsets, indices, distance, offset = 0) {
  const vertexSize = 4

  const dir = new Vector2(normalX, normalY).multiplyScalar(OFFSET_T * up) // handle the offset

  const { x, y, z } = vertex
  const posX = x + normalX * WIDTH / 2 + dir.x
  const posY = y + normalY * WIDTH / 2 + dir.y
  const posZ = z
  // positions.push(x + normalX * WIDTH / 2 + dir.x, y + normalY * WIDTH / 2 + dir.y, z, distance);
  // console.log(posX, posY, posZ, distance);
  positions.push(posX, posY, posZ, distance)
  offsets.push(normalX, normalY, up, offset)

  index1 = index2
  index2 = index3
  index3 = positions.length / vertexSize - 1

  if (index1 >= 0) {
    indices.push(index1, index2, index3)
  }
}

// todo
/*
[
    {
      "x": -14.07145488011874,
      "y": -22.665883355834044,
      "z": 0
  },
  {
      "x": -14.095755345961631,
      "y": -19.300294143205875,
      "z": 0
  },
  {
      "x": -14.09874679272308,
      "y": -18.88598188205111,
      "z": 0
  }
]
*/
function post (originPoints, minLen) {
  const ret = []
  if (originPoints.length > 2) {
    let pre = null
    for (let i = 0; i < originPoints.length; i++) {
      if (!pre || originPoints[i].distanceTo(pre) > minLen) {
        ret.push(originPoints[i])
        pre = originPoints[i]
      }
    }
  } else {
    return originPoints
  }
  return ret
}

/**
 *
 * @param {*} context
 * @param {*} points
 * @param {*} width
 * @param {*} shouldPost 是否过滤掉间隔较小的点
 * @returns
 */
export function getAttributes (points, width, context = { t: 0, shouldPost: false }) {
  WIDTH = width
  OFFSET_T = context.t || 0

  const { positions = [], offsets = [], indices = [] } = context
  positions.splice(0)
  offsets.splice(0)
  indices.splice(0)
  context.positions = positions
  context.offsets = offsets
  context.indices = indices

  if (points.length < 2) {
    return { positions, offsets, indices }
  }

  const minLen = context.minStep || 1

  let postPoints = points

  if (context.shouldPost) {
    postPoints = post(points, minLen)
  }

  index1 = index2 = index3 = -1

  const perp = (vector, out) => {
    const { x, y } = vector
    out.set(-y, x)
    return out
  }

  let preVertex = null
  let curVertex = null
  let nextVertex = null

  let preNormal = new Vector2()
  let nextNormal = new Vector2()

  let preDir = new Vector2()
  let nextDir = new Vector2()

  const joinNormal = new Vector2()

  let linesofar = 0

  let offset = 0
  let offsetB = 0

  let normalX = 0
  let normalY = 0

  let extNormal = null

  const reverse = 1

  for (let i = 0; i < postPoints.length; i++) {
    // reverse *= -context.reverseDash;

    preVertex = curVertex
    curVertex = postPoints[i]
    nextVertex = postPoints[i + 1]

    const distance = preVertex ? curVertex.distanceTo(preVertex) : 0
    linesofar += distance * reverse

    preDir = preVertex ? preDir.copy(nextDir) : preDir.subVectors(nextVertex, curVertex).normalize()
    nextDir = nextVertex ? nextDir.subVectors(nextVertex, curVertex).normalize() : nextDir.copy(preDir)

    preNormal = perp(preDir, preNormal).normalize()
    nextNormal = perp(nextDir, nextNormal).normalize()

    joinNormal.addVectors(preNormal, nextNormal).normalize()

    const isMiddle = preVertex && nextVertex

    if (joinNormal.x === 0 && joinNormal.y === 0) {
      ({ x: normalX, y: normalY } = preNormal)
      addVertex(curVertex, 1, normalX, normalY, positions, offsets, indices, linesofar)
      addVertex(curVertex, -1, -normalX, -normalY, positions, offsets, indices, linesofar)
      index1 = index2 = index3 = -1;
      ({ x: normalX, y: normalY } = nextNormal)
      addVertex(curVertex, 1, normalX, normalY, positions, offsets, indices, linesofar)
      addVertex(curVertex, -1, -normalX, -normalY, positions, offsets, indices, linesofar)
    } else {
      if (isMiddle) {
        const joitDotNextDir = joinNormal.x * nextDir.x + joinNormal.y * nextDir.y
        const lengthScale = 1 / (joinNormal.x * preNormal.x + joinNormal.y * preNormal.y)
        joinNormal.multiplyScalar(lengthScale)

        const isBevel = lengthScale > lengthMax

        if (joitDotNextDir >= 0) {
          // 默认延伸方向指向内侧
          offset = joinNormal.dot(preDir)
          offset *= reverse;
          ({ x: normalX, y: normalY } = joinNormal)
          addVertex(curVertex, 1, normalX, normalY, positions, offsets, indices, linesofar, offset)

          if (!isBevel) {
            offsetB = -joinNormal.dot(preDir)
          } else {
            ({ x: normalX, y: normalY } = preNormal)
            offsetB = 0
          }
          offsetB *= reverse
          addVertex(curVertex, -1, -normalX, -normalY, positions, offsets, indices, linesofar, offsetB)

          if (isBevel) {
            index1 = index2 = -1
          } else {
            index1 = index2 = index3 = -1
          }

          extNormal = joinNormal;
          // offset = extNormal.dot(nextDir);
          ({ x: normalX, y: normalY } = extNormal)
          addVertex(curVertex, 1, normalX, normalY, positions, offsets, indices, linesofar, offset)
          extNormal = isBevel ? nextNormal : joinNormal;
          // offset = -extNormal.dot(nextDir);
          ({ x: normalX, y: normalY } = extNormal)
          addVertex(curVertex, -1, -normalX, -normalY, positions, offsets, indices, linesofar, offsetB)
        } else {
          if (!isBevel) {
            extNormal = joinNormal
          } else {
            extNormal = preNormal
          }
          offset = extNormal.dot(preDir)
          offset *= reverse;
          ({ x: normalX, y: normalY } = extNormal)
          addVertex(curVertex, 1, normalX, normalY, positions, offsets, indices, linesofar, offset)

          offsetB = -joinNormal.dot(preDir)
          offsetB *= reverse;
          ({ x: normalX, y: normalY } = joinNormal)
          addVertex(curVertex, -1, -normalX, -normalY, positions, offsets, indices, linesofar, offsetB)

          if (isBevel) {
            extNormal = nextNormal
          } else {
            extNormal = joinNormal
            index1 = index2 = index3 = -1
          }
          // offset = extNormal.dot(nextDir);
          ({ x: normalX, y: normalY } = extNormal)
          addVertex(curVertex, 1, normalX, normalY, positions, offsets, indices, linesofar, offset)

          if (isBevel) {
            index1 = index2 = -1
          }
          extNormal = joinNormal;
          // offset = -extNormal.dot(nextDir);
          ({ x: normalX, y: normalY } = extNormal)
          addVertex(curVertex, -1, -normalX, -normalY, positions, offsets, indices, linesofar, offsetB)
        }
      } else {
        ({ x: normalX, y: normalY } = nextNormal)
        addVertex(curVertex, 1, normalX, normalY, positions, offsets, indices, linesofar)
        addVertex(curVertex, -1, -normalX, -normalY, positions, offsets, indices, linesofar)
      }
    }
  }

  context.totalLength = linesofar
  return {
    positions,
    offsets,
    indices,
  }
}

/**
 *
 * @param {*} points
 * @param {*} start 0 - 1
 * @param {*} end  0 - 1
 * @param {*} length
 */
export function getSubPointsByStartAndEnd (points, start, end, length) {
  // const start = offset - length / 2
  // const end = offset + length / 2

  let totalLength = 0

  const currentPoint = new Vector3()
  const beforePoint = new Vector3()
  for (let i = 1; i < points.length; i++) {
    const [x1, y1, z1] = points[i]
    const [x0, y0, z0] = points[i - 1]

    currentPoint.set(x1, y1, z1)
    beforePoint.set(x0, y0, z0)

    totalLength += currentPoint.distanceTo(beforePoint)
  }

  start *= totalLength
  end *= totalLength

  // real length
  length = (end - start) * totalLength

  let linesofar = 0

  const [x, y, z] = points[0]
  const pre = new Vector3(x, y, z)
  const current = pre.clone()

  const dir = new Vector3()

  const result = []

  if (start < 0) {
    const [x0, y0, z0] = points[0]
    const [x1, y1, z1] = points[1]
    dir.subVectors(new Vector3(x0, y0, z0), new Vector3(x1, y1, z1)).normalize().multiplyScalar(-start)

    const { x, y, z } = new Vector3().addVectors(current, dir)
    result.push([x, y, z])

    linesofar = start
    pre.set(x, y, z)
  }

  for (let i = 0; i < points.length; i++) {
    const p = points[i]
    current.set(p[0], p[1], p[2])
    linesofar += current.distanceTo(pre)
    if (linesofar < start) {
      pre.copy(current)
      continue
    }
    if (result.length === 0) {
      const delta = linesofar - start
      dir.subVectors(pre, current).normalize().multiplyScalar(delta)
      const firstPoint = new Vector3().addVectors(current, dir)
      result.push([firstPoint.x, firstPoint.y, firstPoint.z])
    } else {
      if (linesofar < end) {
        result.push([current.x, current.y, current.z])
      }
    }

    if (linesofar >= end) {
      const delta = linesofar - end
      dir.subVectors(pre, current).normalize().multiplyScalar(delta)
      const endPoint = new Vector3().addVectors(current, dir)
      result.push([endPoint.x, endPoint.y, endPoint.z])
      pre.copy(current)
      break
    }

    pre.copy(current)
  }

  if (start > linesofar) {
    const [lastX, lastY, lastZ] = points[points.length - 1]
    const [preX, preY, preZ] = points[points.length - 2]

    dir
      .subVectors(current.set(lastX, lastY, lastZ), pre.set(preX, preY, preZ))
      .normalize()
      .multiplyScalar(start - linesofar)
    let { x, y, z } = new Vector3().addVectors(current, dir)
    result.push([x, y, z])
    pre.set(x, y, z)

    linesofar += dir.length()

    dir.normalize().multiplyScalar(length);

    ({ x, y, z } = new Vector3().addVectors(pre, dir))
    result.push([x, y, z])
    pre.set(x, y, z)

    linesofar += dir.length()
  }

  if (linesofar < end) {
    const [lastX, lastY, lastZ] = points[points.length - 1]
    const [preX, preY, preZ] = points[points.length - 2]

    dir
      .subVectors(current.set(lastX, lastY, lastZ), pre.set(preX, preY, preZ))
      .normalize()
      .multiplyScalar(end - linesofar)
    const { x, y, z } = new Vector3().addVectors(current, dir)
    result.push([x, y, z])
    pre.set(x, y, z)
    linesofar += dir.length()
  }

  return result
}

/**
 *
 * @param {Quaternion} quaternion
 * @param {Vector3} vFrom
 * @param {Vector3} vTo
 * @return {Quaternion}
 */
function quaternionSetFromUnitVectors (quaternion, vFrom, vTo) {
  // assumes direction vectors vFrom and vTo are normalized
  let r = vFrom.dot(vTo) + 1

  if (r < Number.EPSILON) {
    // vFrom and vTo point in opposite directions
    r = 0

    if (Math.abs(vFrom.x) > Math.abs(vFrom.z)) {
      quaternion._x = -vFrom.y
      quaternion._y = vFrom.x
      quaternion._z = 0
      quaternion._w = r
    } else {
      quaternion._x = 0
      quaternion._y = -vFrom.z
      quaternion._z = vFrom.y
      quaternion._w = r
    }
  } else {
    // crossVectors( vFrom, vTo ); // inlined to avoid cyclic dependency on Vector3
    quaternion._x = vFrom.y * vTo.z - vFrom.z * vTo.y
    quaternion._y = vFrom.z * vTo.x - vFrom.x * vTo.z
    quaternion._z = vFrom.x * vTo.y - vFrom.y * vTo.x
    quaternion._w = r
  }

  return quaternion.normalize()
}

const east = new Vector3(1, 0, 0)
const axisZ = new Vector3(0, 0, 1)

export function setRotationByDir (mesh, dir, angle = 0) {
  const quaternion = new Quaternion()
  quaternionSetFromUnitVectors(quaternion, east, dir)
  const quaternionZ = new Quaternion()
  quaternionZ.setFromAxisAngle(axisZ, angle)
  quaternion.multiply(quaternionZ)
  mesh.rotation.setFromQuaternion(quaternion)
}

export function alignObjectByDir (object, dir, angle = 0) {
  if (object && object.model) {
    setRotationByDir(object.model, dir, angle)
  }
}

/**
 * 根据地平面方向和车道面的法向量旋转模型
 * @param {Object3D} mesh
 * @param {number} theta
 * @param {Vector3} normal
 */
export function setRotateByDirNormal (mesh, theta, normal) {
  if (normal.z < 0) {
    normal.negate()
  }
  const matrix = new Matrix4()
  matrix.makeRotationAxis(axisZ, theta)
  const quaternionZ = new Quaternion()
  quaternionSetFromUnitVectors(quaternionZ, axisZ, normal)
  const matrixZ = new Matrix4()
  matrixZ.makeRotationFromQuaternion(quaternionZ)
  matrixZ.multiply(matrix)
  mesh.rotation.setFromRotationMatrix(matrixZ)
}

const DOUBLE_LINES_SPACING = 0.15

/**
 * 根据采样点points生成一定宽度的左右两条采样点
 * @param {Vector3[]} points
 * @returns {{rightPoints: Vector3[], leftPoints: Vector3[]}}
 */
export function composeLeftRightPoints (points) {
  const dirZ = new Vector3(0, 0, 1)
  const leftPoints = []
  const rightPoints = []

  for (let i = 1; i < points.length; i++) {
    const { x: prevX, y: prevY, z: prevZ } = points[i - 1]
    const { x: currX, y: currY, z: currZ } = points[i]
    const dir = new Vector3(currX - prevX, currY - prevY, currZ - prevZ)

    dir
      .cross(dirZ)
      .normalize()
      .multiplyScalar(DOUBLE_LINES_SPACING / 2)

    const { x, y, z } = dir

    if (i === 1) {
      leftPoints.push({ x: prevX - x, y: prevY - y, z: prevZ - z })
      rightPoints.push({ x: prevX + x, y: prevY + y, z: prevZ + z })
    }

    leftPoints.push({ x: currX - x, y: currY - y, z: currZ - z })
    rightPoints.push({ x: currX + x, y: currY + y, z: currZ + z })
  }

  return { leftPoints, rightPoints }
}

/**
 * 解析车道线模型
 * @param {number} mark
 * @return {{color, double: boolean, bold, leftType: string, type, rightType}}
 */
export function parseLaneMark (mark) {
  const markStr = mark.toString(16).padStart(7, '0')
  let double
  let leftType
  let rightType
  let color

  switch (markStr[2]) {
    case '1':
    case '2':
    case '3':
      color = 'yellow'
      break
    case '5':
      color = 'red'
      break
    case '6':
      color = 'green'
      break
    case '7':
      color = 'blue'
      break
    default:
      color = 'white'
  }

  const bold = markStr[3] === '1'

  switch (markStr[5]) {
    case '1':
      double = true
      leftType = 'solid'
      break
    case '2':
      double = true
      leftType = 'dashed'
      break
    default:
      double = false
      leftType = 'none'
  }

  switch (markStr[6]) {
    case '1':
      rightType = 'solid'
      break
    case '2':
      rightType = 'dashed'
      break
    default:
      rightType = 'none'
  }

  return {
    double,
    leftType,
    rightType,
    bold,
    color,
  }
}

export function yawToVector3 (yaw) {
  const result = new Vector3(1, 0, 0)
  const axis = new Vector3(0, 0, 1)
  return result.applyAxisAngle(axis, yaw)
}
