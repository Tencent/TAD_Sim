import { CubicBezierCurve3, Group, Vector3 } from 'three'
import { transformVec3ByObject } from '../../utils/common'
import { getVerticalVector, renderCustomRoadSignByGeoAttrs } from './index'

// 创建路口导向线
export function createIntersectionGuideLine (params) {
  const { data, parent, ignoreAlt = false } = params
  const { id, geomdata } = data

  if ((geomdata?.points?.length || 0) < 1) return
  const bezierPoints = geomdata.points[0].point

  // 忽略高度
  if (ignoreAlt) {
    bezierPoints.forEach((p) => {
      p.z = 0
    })
  }

  const objectId = String(id)

  // 基于 mapsdk 计算的贝塞尔曲线控制点，计算对应的路径
  const [p1, cp1, cp2, p2] = transformVec3ByObject(bezierPoints)
  const curve = new CubicBezierCurve3(p1, cp1, cp2, p2)

  const geoAttrs = getGeoAttrs(curve)

  const group = new Group()
  group.name = 'customRoadSignGroup'
  group.objectId = objectId

  renderCustomRoadSignByGeoAttrs({
    geoAttrs,
    group,
    color: 'yellow',
    objectId,
  })

  parent.add(group)
}

function getGeoAttrs (curve) {
  // 顶点和三角面索引数组
  const vertices = []
  const indices = []
  // 顶点的索引
  let index = 0
  // 路径s轴方向的偏移量
  let sOffset = 0
  const roadLength = curve.getLength()

  const step = 2 // 步长为 200cm
  const width = 0.15 // 线宽为 15cm

  const relativeOffsets = [
    { s: 0, t: -width / 2 },
    { s: step, t: -width / 2 },
    { s: step, t: width / 2 },
    { s: 0, t: width / 2 },
  ]

  let targetPoint = new Vector3()
  while (sOffset < roadLength) {
    for (const _offset of relativeOffsets) {
      const { s, t } = _offset
      // 百分比控制在 [0,1] 区间
      const percent = Math.max(0, Math.min(1, (s + sOffset) / roadLength))
      const refPoint = curve.getPointAt(percent)
      const tangent = curve.getTangentAt(percent)
      const vertical = getVerticalVector(tangent)
      targetPoint = refPoint.clone()
      targetPoint.addScaledVector(vertical, t)

      vertices.push(targetPoint.x, targetPoint.y, targetPoint.z)
    }

    // 4个顶点的索引
    const a = index
    const b = index + 1
    const c = index + 2
    const d = index + 3

    indices.push(a, c, b)
    indices.push(c, a, d)

    sOffset += step * 2
    index += 4
  }

  const _vertices = new Float32Array(vertices)
  const geoAttrs = [
    {
      vertices: _vertices,
      indices,
    },
  ]

  return geoAttrs
}
