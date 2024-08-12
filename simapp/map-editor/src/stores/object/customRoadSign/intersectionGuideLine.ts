import { CatmullRomCurve3, type Intersection, Vector2, Vector3 } from 'three'
import { createDefaultObjectData } from '../common'
import { useObjectStore } from '../index'
import type { IRoadSignConfig } from '@/config/roadSign'
import { genObjectId } from '@/utils/guid'
import { getJunction, getObject, setObject } from '@/utils/mapCache'
import { Constant, sortJunctionRefBoundary } from '@/utils/business'
import {
  CatmullromTension,
  CurveType,
  PI,
  fixedPrecision,
  getBezier3WithDirection,
  getClosestPointFromCurvePath,
  getTValue,
  getValidRadInRange,
  getVerticalVector,
  transformVec3ByObject,
} from '@/utils/common3d'

// 贝塞尔曲线控制点的距离（默认0.5有点大）
const curveRatio = 0.4
const sampleSegment = 4

/**
 * 基于路径获取路口导向线的几何体属性
 * @param curve
 */
function getGeoAttrsByCurve (curve: biz.ICurve3) {
  // 顶点和三角面索引数组
  const vertices = []
  const indices = []
  // 顶点的索引
  let index = 0
  // 路径s轴方向的偏移量
  let sOffset = 0
  const roadLength = curve.getLength()

  const step = 2 // 步长为 200cm
  const width = Constant.intersectionGuideLine.width // 线宽为 15cm

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

  return {
    vertices,
    indices,
  }
}

/**
 * 获取一个坐标点，在路口中匹配的最佳驶入驶出道路组合
 * @param params
 */
export function getMatchRoadInJunction (params: {
  point: common.vec3
  junctionId: string
}) {
  const { point, junctionId } = params
  const junction = getJunction(junctionId)

  // 判断当前交互点离哪一条导向线距离更近
  if (!junction) return null
  const _point = new Vector3(point.x, point.y, point.z)

  const { refRoads } = junction
  // 路口连通的道路按顺时针排列
  const sortedRefRoads = sortJunctionRefBoundary(refRoads)

  // 当前交互点距离最近的驶入驶出道路组合
  let targetTailRefRoad: biz.IRefRoad | null = null
  let targetHeadRefRoad: biz.IRefRoad | null = null
  // 目标点到驶入驶出道路连接线的距离总和（取最小的）
  let totalDist = 0

  // 从驶入路口的道路（道路尾）到驶出路口的道路的连接线（道路头）
  for (let i = 0; i < sortedRefRoads.length; i++) {
    const { isTail, direction } = sortedRefRoads[i]
    // 在双向车道存在的情况下，from 道路有 2 种，其余情况 pass
    if (
      (isTail && direction === 'reverse') ||
      (!isTail && direction === 'forward')
    ) {
      continue
    }

    // 驶入路口的道路参考线坐标点
    const {
      leftPoint: tailLeftPoint,
      rightPoint: tailRightPoint,
      alongVec: tailAlongVec,
    } = sortedRefRoads[i]
    const fromPoint = direction === 'forward' ? tailLeftPoint : tailRightPoint
    const tailAlongVec2 = new Vector2(tailAlongVec.x, tailAlongVec.z)
    const tailAngle = tailAlongVec2.angle()

    // 获取当前驶入路口的道路，顺时针最近的驶出路口道路，进行组合，计算对应的距离
    let headRefRoad: biz.IRefRoad | null = null
    let index = i
    // 计数器
    let count = 0

    while (!headRefRoad && count < sortedRefRoads.length) {
      // 迭代至下一个 refRoad 的索引（如果超过范围就从头算）
      index = (index + 1) % sortedRefRoads.length
      count++
      // 如果是驶出路口的道路，也有 2 种情况
      const { isTail: _isTail, direction: _direction } = sortedRefRoads[index]
      if (
        (_isTail && _direction === 'forward') ||
        (!_isTail && _direction === 'reverse')
      ) {
        // 计算驶入和驶出道路的夹角，不能接近 0 或 180°
        const { alongVec: headAlongVec } = sortedRefRoads[index]
        const headAlongVec2 = new Vector2(headAlongVec.x, headAlongVec.z)
        const headAngle = headAlongVec2.angle()

        // 计算驶入路口道路跟驶出路口道路的alongVec2的朝向角度偏差绝对值
        const deltaAngle = getValidRadInRange(tailAngle - headAngle)
        // 如果偏差的角度在 [-135°, -45°] 区间内，都视为有效的组合
        if (deltaAngle < -PI / 4 && deltaAngle > -(PI / 4) * 3) {
          headRefRoad = sortedRefRoads[index]
        }
      }
    }

    if (!headRefRoad) continue
    const { leftPoint: headLeftPoint, rightPoint: headRightPoint } = headRefRoad
    const toPoint =
      headRefRoad.direction === 'forward' ? headLeftPoint : headRightPoint
    // 驶入路口和驶出路口的道路都存在以后，计算跟交互点的距离
    const p1 = new Vector3(fromPoint.x, fromPoint.y, fromPoint.z)
    const p2 = new Vector3(toPoint.x, toPoint.y, toPoint.z)
    const dist = _point.distanceTo(p1) + _point.distanceTo(p2)

    // 取距离最小的一组道路
    if (!targetHeadRefRoad || !targetTailRefRoad || dist < totalDist) {
      totalDist = dist
      targetHeadRefRoad = headRefRoad
      targetTailRefRoad = sortedRefRoads[i]
    }
  }

  if (!targetHeadRefRoad || !targetTailRefRoad) return null

  // 基于有效的驶入路口道路和驶出路口道路，计算导向线的路径
  // 通过两条道路参考线的端点，以及车道方向，计算对应的贝塞尔曲线
  const targetTailRefPoint =
    targetTailRefRoad.direction === 'forward' ?
      targetTailRefRoad.leftPoint :
      targetTailRefRoad.rightPoint
  const targetHeadRefPoint =
    targetHeadRefRoad.direction === 'forward' ?
      targetHeadRefRoad.leftPoint :
      targetHeadRefRoad.rightPoint
  const { curve, cp1, cp2 } = getBezier3WithDirection({
    point1: targetTailRefPoint,
    direction1: targetTailRefRoad.alongVec,
    direction2: targetHeadRefRoad.alongVec,
    point2: targetHeadRefPoint,
    ratio: curveRatio,
  })

  // 贝塞尔曲线的2个基准点和2个控制点
  const controlPoints: Array<common.vec3> = []
  controlPoints.push(
    // 第一个基准点
    {
      x: targetTailRefPoint.x,
      y: targetTailRefPoint.y,
      z: targetTailRefPoint.z,
    },
    // 第一个控制点
    {
      x: cp1.x,
      y: cp1.y,
      z: cp1.z,
    },
    // 第二个控制点
    {
      x: cp2.x,
      y: cp2.y,
      z: cp2.z,
    },
    // 第二个基准点
    {
      x: targetHeadRefPoint.x,
      y: targetHeadRefPoint.y,
      z: targetHeadRefPoint.z,
    },
  )

  // 返回关联的两条道路，以及对应路径
  return {
    curve,
    tailRoadId: targetTailRefRoad.roadId,
    headRoadId: targetHeadRefRoad.roadId,
    controlPoints,
  }
}

// 路口导向线
export function createIntersectionGuideLine (params: {
  option: IRoadSignConfig
  intersection: Intersection
}) {
  const { option, intersection } = params
  const { name, showName, type, subtype } = option
  const roadSignData: biz.IIntersectionGuideLine = {
    ...createDefaultObjectData(),
    geoAttrs: [],
    curvePoints: [],
    frid: '',
    trid: '',
  }
  roadSignData.id = genObjectId()
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype

  // 基于当前交互点在路口中的位置，计算可能关联的道路参考线
  const { object, point: intersectionPoint } = intersection
  // @ts-expect-error
  const { junctionId } = object

  const curveRes = getMatchRoadInJunction({
    point: intersectionPoint,
    junctionId,
  })
  if (!curveRes) return null

  const { curve, tailRoadId, headRoadId, controlPoints } = curveRes

  const { vertices, indices } = getGeoAttrsByCurve(curve)
  const _vertices = new Float32Array(vertices)

  const geoAttrs: Array<biz.IGeoAttr> = [
    {
      vertices: _vertices,
      indices,
    },
  ]

  roadSignData.geoAttrs = geoAttrs
  roadSignData.junctionId = junctionId
  roadSignData.width = Constant.intersectionGuideLine.width
  roadSignData.length = fixedPrecision(curve.getLength())
  // 取路径的五分点作为坐标
  const halfPoint = curve.getPointAt(0.5)
  roadSignData.position = {
    x: halfPoint.x,
    y: halfPoint.y,
    z: halfPoint.z,
  }

  // 保存路口中形成导向线的关联道路
  roadSignData.curvePoints = controlPoints
  roadSignData.frid = tailRoadId
  roadSignData.trid = headRoadId

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 更新路口中已有的路口导向线属性（主要是更新 geoAttr）
export function updateIntersectionGuideLine (objectId: string) {
  const roadSignData = getObject(objectId) as biz.IIntersectionGuideLine
  if (!roadSignData) return null

  const { junctionId, frid, trid } = roadSignData
  const junction = getJunction(junctionId)
  if (!junction) return null

  // 由于已经有关联的驶入和驶出道路了，直接计算对应的路径
  // 基于路口的道路，重新创建对应的路径和几何体属性
  let tailRefRoad: biz.IRefRoad | null = null
  let headRefRoad: biz.IRefRoad | null = null
  for (const refRoad of junction.refRoads) {
    if (tailRefRoad && headRefRoad) continue
    if (refRoad.roadId === frid) {
      tailRefRoad = refRoad
    }
    if (refRoad.roadId === trid) {
      headRefRoad = refRoad
    }
  }
  if (!tailRefRoad || !headRefRoad) return null

  const { curve, cp1, cp2 } = getBezier3WithDirection({
    point1: tailRefRoad.leftPoint,
    direction1: tailRefRoad.alongVec,
    direction2: headRefRoad.alongVec,
    point2: headRefRoad.leftPoint,
    ratio: curveRatio,
  })
  // 贝塞尔曲线的2个基准点和2个控制点
  const controlPoints: Array<common.vec3> = []
  controlPoints.push(
    // 第一个基准点
    {
      x: tailRefRoad.leftPoint.x,
      y: tailRefRoad.leftPoint.y,
      z: tailRefRoad.leftPoint.z,
    },
    // 第一个控制点
    {
      x: cp1.x,
      y: cp1.y,
      z: cp1.z,
    },
    // 第二个控制点
    {
      x: cp2.x,
      y: cp2.y,
      z: cp2.z,
    },
    // 第二个基准点
    {
      x: headRefRoad.leftPoint.x,
      y: headRefRoad.leftPoint.y,
      z: headRefRoad.leftPoint.z,
    },
  )
  const { vertices, indices } = getGeoAttrsByCurve(curve)
  const _vertices = new Float32Array(vertices)
  const geoAttrs: Array<biz.IGeoAttr> = [
    {
      vertices: _vertices,
      indices,
    },
  ]

  roadSignData.geoAttrs = geoAttrs
  roadSignData.length = fixedPrecision(curve.getLength())
  // 取路径的五分点作为坐标
  const halfPoint = curve.getPointAt(0.5)
  roadSignData.position = {
    x: halfPoint.x,
    y: halfPoint.y,
    z: halfPoint.z,
  }
  roadSignData.curvePoints = controlPoints

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 保存地图，解析路口导向线
export function saveParseIntersectionGuideLine (params: {
  originData: biz.IIntersectionGuideLine
  targetData: biz.ICommonObject
}) {
  const { originData, targetData } = params

  const {
    name,
    junctionId = '',
    length,
    width,
    frid: fromRoadId,
    trid: toRoadId,
    curvePoints,
  } = originData

  // 基于当前路口导向线关联的道路，获取一条符合 frid 和 trid 的 lanelink
  const junction = getJunction(junctionId)
  if (!junction) return null

  let targetLink: biz.ILaneLink | null = null
  for (const link of junction.laneLinks) {
    if (targetLink) continue

    const { frid, trid } = link
    if (frid === fromRoadId && trid === toRoadId) {
      targetLink = link
    }
  }
  if (!targetLink) return null

  targetData.name = name
  targetData.length = String(fixedPrecision(length))
  targetData.validLength = targetData.length
  targetData.width = String(fixedPrecision(width))
  targetData.lanelinkid = targetLink.id
  targetData.roadid = targetLink.roadid
  targetData.t = '0'
  targetData.s = '0'
  targetData.hdg = '0'

  // 在 userdata 中缓存对应路径的关键点（主要用于场景编辑器还原路径）
  // 需要转换成相对于 laneLink 路径的相对 st 坐标
  targetData.userdata = []
  const linkCurve = new CatmullRomCurve3(
    transformVec3ByObject(targetLink.samplePoints),
  )
  linkCurve.curveType = CurveType
  if (CurveType === 'catmullrom') {
    linkCurve.tension = CatmullromTension
  }
  const linkLength = linkCurve.getLength()
  const stPoints = []
  // 将三维的路径点，转换成相对于 link 路径的 st 坐标
  for (const point of curvePoints) {
    const {
      point: closestPoint,
      tangent,
      percent,
    } = getClosestPointFromCurvePath({
      curvePath: linkCurve,
      point,
    })
    const _percent = Math.max(0, Math.min(1, percent))
    const s = fixedPrecision(linkLength * _percent)
    const t = getTValue({
      tangent,
      refLinePoint: closestPoint,
      targetPoint: point,
    })
    stPoints.push({ s, t })
  }
  // 将对象转换成字符串，同时将所有双引号转换成 # 号，方便后端解析 xml 使用
  const value = JSON.stringify(stPoints).replaceAll('"', '#')

  targetData.userdata.push({
    code: 'sample_point',
    value,
  })

  return targetData
}

// 加载地图，解析路口导向线
export function loadParseIntersectionGuideLine (params: {
  config: IRoadSignConfig
  data: biz.ICommonObject
  junction: biz.IJunction
  laneLink: biz.ILaneLink
}) {
  const { config, data: commonData, junction, laneLink } = params
  const { id: objectId } = commonData
  const { name, type, subtype, showName } = config
  const roadSignData: biz.IIntersectionGuideLine = {
    ...createDefaultObjectData(),
    geoAttrs: [],
    curvePoints: [],
    frid: '',
    trid: '',
  }

  roadSignData.id = objectId
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype

  // 从 lanelink 中获取对应的 frid 和 trid
  const { frid, trid } = laneLink
  // 基于路口的道路，重新创建对应的路径和几何体属性
  let tailRefRoad: biz.IRefRoad | null = null
  let headRefRoad: biz.IRefRoad | null = null
  for (const refRoad of junction.refRoads) {
    if (tailRefRoad && headRefRoad) continue
    if (refRoad.roadId === frid) {
      tailRefRoad = refRoad
    }
    if (refRoad.roadId === trid) {
      headRefRoad = refRoad
    }
  }
  if (!tailRefRoad || !headRefRoad) return null

  const { curve, cp1, cp2 } = getBezier3WithDirection({
    point1: tailRefRoad.leftPoint,
    direction1: tailRefRoad.alongVec,
    direction2: headRefRoad.alongVec,
    point2: headRefRoad.leftPoint,
    ratio: curveRatio,
  })
  // 贝塞尔曲线的2个基准点和2个控制点
  const controlPoints: Array<common.vec3> = []
  controlPoints.push(
    // 第一个基准点
    {
      x: tailRefRoad.leftPoint.x,
      y: tailRefRoad.leftPoint.y,
      z: tailRefRoad.leftPoint.z,
    },
    // 第一个控制点
    {
      x: cp1.x,
      y: cp1.y,
      z: cp1.z,
    },
    // 第二个控制点
    {
      x: cp2.x,
      y: cp2.y,
      z: cp2.z,
    },
    // 第二个基准点
    {
      x: headRefRoad.leftPoint.x,
      y: headRefRoad.leftPoint.y,
      z: headRefRoad.leftPoint.z,
    },
  )
  const { vertices, indices } = getGeoAttrsByCurve(curve)
  const _vertices = new Float32Array(vertices)
  const geoAttrs: Array<biz.IGeoAttr> = [
    {
      vertices: _vertices,
      indices,
    },
  ]

  roadSignData.geoAttrs = geoAttrs
  roadSignData.junctionId = junction.id
  roadSignData.width = Constant.intersectionGuideLine.width
  roadSignData.length = fixedPrecision(curve.getLength())
  // 取路径的五分点作为坐标
  const halfPoint = curve.getPointAt(0.5)
  roadSignData.position = {
    x: halfPoint.x,
    y: halfPoint.y,
    z: halfPoint.z,
  }

  // 路口中形成导向线的关联道路
  roadSignData.curvePoints = controlPoints
  roadSignData.frid = frid
  roadSignData.trid = trid

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

/**
 * 找到跟道路关联的路口导向线，并执行对应的回调
 * @param params
 */
export function handleIntersectionGuideLineByRoadId (params: {
  roadIds: Array<string>
  objectIds?: Array<string>
  objects?: Array<biz.IObject>
  handler: Function
}) {
  const { roadIds, objectIds, objects, handler } = params
  const objectStore = useObjectStore()
  for (const _objId of objectStore.ids) {
    const objData = getObject(_objId)
    if (!objData) continue
    const { name } = objData
    if (name === 'Intersection_Guide_Line') {
      const guideLineData = objData as biz.IIntersectionGuideLine
      const { frid, trid } = guideLineData
      // 如果关联的道路匹配上
      if (roadIds.includes(frid) || roadIds.includes(trid)) {
        // 避免重复添加
        if (objectIds && objects && !objectIds.includes(_objId)) {
          objectIds.push(_objId)
          objects.push(objData)
          if (typeof handler === 'function') {
            // 找到对应的物体，执行回调
            handler(_objId)
          }
        }
      }
    }
  }
}
