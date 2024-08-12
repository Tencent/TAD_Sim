import {
  BufferGeometry,
  CircleGeometry,
  Color,
  Group,
  Line,
  LineDashedMaterial,
  Mesh,
  MeshBasicMaterial,
  PlaneGeometry,
  Sprite,
  Vector3,
} from 'three'
import type {
  Object3D,
} from 'three'
import { Constant, RenderOrder } from '@/utils/business'
import {
  DefaultColor,
  PI,
  axisY,
  createArcPoints,
  disposeMaterial,
  fixedPrecision,
  halfPI,
  transformVec3,
} from '@/utils/common3d'
import { createTextSprite } from '@/main3d/render/text'
import root3d from '@/main3d/index'

interface IRenderCircleLineParams {
  radius: number
  angle: number // 跟水平向右的横轴坐标的夹角，弧度制
  center: Vector3 // 圆环中心基准点
  junctionNumber: number // 预留的路口数量
}

interface IRenderCircleRoadParams extends IRenderCircleLineParams {
  parent: Object3D
  radiusPoint: Vector3 // 半径上的点（鼠标射线此时位置）
  zoom?: number // 相机的缩放效果，用于控制精灵的尺寸
}

// 高亮颜色
const activeColor = 0x16D1F3
// 2 度对应的弧度
const unit = PI / 90
// 给出口预留的弧度
const angleOffset = unit * 8

export const HelperAreaColor = {
  normal: 0x80497F,
  hovered: 0xFF9800,
  selected: 0xB42A1F,
}

const centerGeo = new CircleGeometry(0.4, 16)
const centerMat = new MeshBasicMaterial({
  color: activeColor,
})
const dashLineMat = new LineDashedMaterial({
  color: activeColor,
  dashSize: 2,
  gapSize: 2,
})
const helperAreaMat = new MeshBasicMaterial({
  color: HelperAreaColor.normal,
  depthTest: false,
})
const helperAreaGeo = new PlaneGeometry(1, 1)

// 创建道路首尾两侧调整道路弧度的辅助交互区域
export function createCircleEndArea (params: {
  road: Array<biz.IRoad | null>
  parent: Object3D
  isClockwise?: boolean
}) {
  const { road, parent, isClockwise = false } = params
  const [forwardRoad] = road
  // 如果2条道路都不存在，则无效
  if (!forwardRoad) return

  // 区分正向和反向车道
  let forwardRoadWidth = 0
  let reverseRoadWidth = 0

  const circleOption: biz.ICircleOption = forwardRoad.circleOption
  const { keyPath } = forwardRoad
  const startPoint: common.vec3 = keyPath.points[0]
  const endPoint: common.vec3 = keyPath.points[keyPath.points.length - 1]

  forwardRoad.sections[0].lanes.forEach((lane) => {
    if (Number(lane.id) < 0) {
      // 正向车道
      forwardRoadWidth += lane.normalWidth
    } else {
      // 反向车道
      reverseRoadWidth += lane.normalWidth
    }
  })

  const roadId = [forwardRoad.id]
  const areaWidth = forwardRoadWidth + reverseRoadWidth

  if (!circleOption || !startPoint || !endPoint) return

  // 环形道路相关的属性
  const { radius, center, startAngle, endAngle } = circleOption
  const _startPoint = transformVec3(startPoint)
  const _endPoint = transformVec3(endPoint)
  const _center = transformVec3(center)

  const basicDirection = new Vector3(0, 0, 1)
  const startDirection = basicDirection
    .clone()
    .applyAxisAngle(axisY, startAngle)
  const endDirection = basicDirection.clone().applyAxisAngle(axisY, endAngle)
  const offset = forwardRoadWidth - reverseRoadWidth
  let startPos = new Vector3()
  let endPos = new Vector3()
  startPos = _startPoint.clone().addScaledVector(startDirection, offset / 2)
  endPos = _endPoint.clone().addScaledVector(endDirection, offset / 2)

  // 当前道路圆弧区间的虚线范围辅助线
  const dashLinePoints = [_startPoint, _center, _endPoint]
  const lineMat = dashLineMat.clone()
  lineMat.color = new Color(activeColor)
  lineMat.needsUpdate = true
  const dashLineGeo = new BufferGeometry().setFromPoints(dashLinePoints)
  const dashLine = new Line(dashLineGeo, lineMat)
  dashLine.computeLineDistances()
  // 半径辅助预览线
  dashLine.name = 'helperLine'
  dashLine.renderOrder = RenderOrder.circleRoadLine
  parent.add(dashLine)

  // 创建圆形的基准点
  const centerMesh = new Mesh(centerGeo, centerMat)
  centerMesh.rotateX(-halfPI)
  centerMesh.name = 'helperPoint'
  centerMesh.renderOrder = RenderOrder.circleRoadLine
  centerMesh.position.set(center.x, center.y, center.z)
  parent.add(centerMesh)

  // 渲染道路首和尾的辅助交互区域【在辅助元素上挂载自定义属性】
  const geo = helperAreaGeo.clone()
  const startGroup = new Group()
  const startArea = new Mesh(geo, helperAreaMat.clone())
  startArea.scale.set(areaWidth, 1, 1)
  startArea.rotateX(-halfPI)
  startArea.rotateZ(halfPI)
  startArea.name = 'helperArea'
  startArea.renderOrder = RenderOrder.circleRoadLine
  // 自定义属性
  startArea.isHead = true
  startArea.startAngle = startAngle
  startArea.endAngle = endAngle
  startArea.radius = radius
  startArea.center = _center
  // 关联的道路 id，[正向道路, 反向道路]
  startArea.roadId = roadId
  startArea.forwardRoadWidth = forwardRoadWidth
  startArea.reverseRoadWidth = reverseRoadWidth
  startGroup.add(startArea)
  startGroup.position.set(startPos.x, startPos.y, startPos.z)
  startGroup.rotateOnAxis(axisY, startAngle)
  parent.add(startGroup)

  // 尾部
  const endGroup = new Group()
  const endArea = new Mesh(geo, helperAreaMat.clone())
  endArea.scale.set(areaWidth, 1, 1)
  endArea.rotateX(-halfPI)
  endArea.rotateZ(halfPI)
  endArea.name = 'helperArea'
  endArea.renderOrder = RenderOrder.circleRoadLine
  // 自定义属性
  endArea.isHead = false
  endArea.startAngle = startAngle
  endArea.endAngle = endAngle
  endArea.radius = radius
  endArea.center = _center
  // 关联的道路 id，[正向道路, 反向道路]
  endArea.roadId = roadId
  endArea.forwardRoadWidth = forwardRoadWidth
  endArea.reverseRoadWidth = reverseRoadWidth
  endGroup.add(endArea)
  endGroup.position.set(endPos.x, endPos.y, endPos.z)
  endGroup.rotateOnAxis(axisY, endAngle)
  parent.add(endGroup)
}

// 创建虚拟的道路边缘效果
function createVirtualCircleRoad (params: IRenderCircleRoadParams) {
  const { radius, center, parent, junctionNumber = 1, angle } = params

  // 预留1个出口
  // 预留2个出口，每个出口处的角度差 180 度
  // 预留3个出口，每个出口处的角度差 120 度
  // 预留4个出口，每个出口处的角度差 90 度
  const angleRange = (PI * 2) / junctionNumber
  let basicAngle = angle

  // 半径为 radius 的道路参考线采样点
  const circleOptions: Array<biz.ICircleOption> = []

  for (let i = 0; i < junctionNumber; i++) {
    const startAngle = basicAngle + angleOffset
    const endAngle = basicAngle + angleRange - angleOffset

    // 半径处的采样点
    const radiusPoints = createArcPoints({
      radius,
      center,
      startAngle,
      endAngle,
      unit,
    })
    // 保存半径上的采样点
    circleOptions.push({
      points: radiusPoints,
      center,
      radius,
      startAngle,
      endAngle,
    })

    // 半径内侧道路的边界线采样点
    const innerRadius = radius - Constant.laneWidth * 2
    const innerPoints = createArcPoints({
      radius: innerRadius,
      center,
      startAngle,
      endAngle,
      unit,
    })

    // 半径外侧道路的边界线采样点
    const outerRadius = radius + Constant.laneWidth * 2
    const outerPoints = createArcPoints({
      radius: outerRadius,
      center,
      startAngle,
      endAngle,
      unit,
    })
    // 通过半径采样点，补充内侧和外层采样点的范围
    innerPoints.unshift(radiusPoints[0])
    innerPoints.push(radiusPoints[radiusPoints.length - 1])
    outerPoints.unshift(radiusPoints[0])
    outerPoints.push(radiusPoints[radiusPoints.length - 1])

    // basicAngle 递增，且保证范围在 [0, 2 * PI] 之间
    basicAngle = (basicAngle + angleRange) % (PI * 2)

    // 不同虚拟辅助线对应的颜色
    const radiusMat = dashLineMat.clone()
    radiusMat.color = new Color(DefaultColor.Yellow.value)
    radiusMat.needsUpdate = true
    const boundaryMat = dashLineMat.clone()
    boundaryMat.color = new Color(DefaultColor.White.value)
    boundaryMat.needsUpdate = true

    const radiusGeo = new BufferGeometry().setFromPoints(radiusPoints)
    const innerGeo = new BufferGeometry().setFromPoints(innerPoints)
    const outerGeo = new BufferGeometry().setFromPoints(outerPoints)
    const radiusLine = new Line(radiusGeo, radiusMat)
    const innerLine = new Line(innerGeo, boundaryMat)
    const outerLine = new Line(outerGeo, boundaryMat)
    radiusLine.computeLineDistances()
    innerLine.computeLineDistances()
    outerLine.computeLineDistances()
    radiusLine.name = 'virtualElement'
    innerLine.name = 'virtualElement'
    outerLine.name = 'virtualElement'
    radiusLine.renderOrder = RenderOrder.circleRoadLine
    innerLine.renderOrder = RenderOrder.circleRoadLine
    outerLine.renderOrder = RenderOrder.circleRoadLine
    parent.add(radiusLine, innerLine, outerLine)
  }

  return circleOptions
}

// 渲染预览的部分环形（圆弧）道路
export function renderVirtualArcRoad (params: {
  parent: Object3D
  center: common.vec3
  radius: number
  startAngle: number
  endAngle: number
  forwardRoadWidth: number
  reverseRoadWidth: number
  isClockwise?: boolean
}) {
  const {
    parent,
    center,
    radius,
    startAngle,
    endAngle,
    forwardRoadWidth,
    reverseRoadWidth,
    isClockwise = false,
  } = params
  const _center = new Vector3(center.x, center.y, center.z)

  // 不同虚拟辅助线对应的颜色
  const radiusMat = dashLineMat.clone()
  radiusMat.color = new Color(DefaultColor.Yellow.value)
  radiusMat.needsUpdate = true
  const boundaryMat = dashLineMat.clone()
  boundaryMat.color = new Color(DefaultColor.White.value)
  boundaryMat.needsUpdate = true

  // 半径为 radius 的道路参考线采样点
  const circleOptions: Array<biz.ICircleOption> = []

  // 半径处的采样点
  const radiusPoints = createArcPoints({
    radius,
    center: _center,
    startAngle,
    endAngle,
    unit,
    isClockwise,
  })

  // 保存半径上的采样点
  circleOptions.push({
    points: radiusPoints,
    center,
    radius,
    startAngle,
    endAngle,
  })

  const radiusGeo = new BufferGeometry().setFromPoints(radiusPoints)
  const radiusLine = new Line(radiusGeo, radiusMat)
  radiusLine.computeLineDistances()
  radiusLine.name = 'virtualElement'
  radiusLine.renderOrder = RenderOrder.circleRoadLine
  parent.add(radiusLine)

  // 如果内侧道路的宽度大于 0
  if (reverseRoadWidth > 0) {
    // 半径内侧道路的边界线采样点
    const innerRadius = radius - reverseRoadWidth
    const innerPoints = createArcPoints({
      radius: innerRadius,
      center: _center,
      startAngle,
      endAngle,
      unit,
      isClockwise,
    })
    innerPoints.unshift(radiusPoints[0])
    innerPoints.push(radiusPoints[radiusPoints.length - 1])
    const innerGeo = new BufferGeometry().setFromPoints(innerPoints)
    const innerLine = new Line(innerGeo, boundaryMat)
    innerLine.computeLineDistances()
    innerLine.name = 'virtualElement'
    innerLine.renderOrder = RenderOrder.circleRoadLine
    parent.add(innerLine)
  }
  // 如果外侧道路的宽度大于 0
  if (forwardRoadWidth > 0) {
    // 半径外侧道路的边界线采样点
    const outerRadius = radius + forwardRoadWidth
    const outerPoints = createArcPoints({
      radius: outerRadius,
      center: _center,
      startAngle,
      endAngle,
      unit,
      isClockwise,
    })
    outerPoints.unshift(radiusPoints[0])
    outerPoints.push(radiusPoints[radiusPoints.length - 1])
    const outerGeo = new BufferGeometry().setFromPoints(outerPoints)
    const outerLine = new Line(outerGeo, boundaryMat)
    outerLine.computeLineDistances()
    outerLine.name = 'virtualElement'
    outerLine.renderOrder = RenderOrder.circleRoadLine
    parent.add(outerLine)
  }

  // 当前道路圆弧区间的虚线范围辅助线
  const dashLinePoints = [
    radiusPoints[0],
    _center,
    radiusPoints[radiusPoints.length - 1],
  ]
  const lineMat = dashLineMat.clone()
  lineMat.color = new Color(activeColor)
  lineMat.needsUpdate = true
  const dashLineGeo = new BufferGeometry().setFromPoints(dashLinePoints)
  const dashLine = new Line(dashLineGeo, lineMat)
  dashLine.computeLineDistances()
  // 半径辅助预览线
  dashLine.name = 'virtualElement'
  dashLine.renderOrder = RenderOrder.circleRoadLine
  parent.add(dashLine)

  // 创建圆形的基准点
  const centerMesh = new Mesh(centerGeo, centerMat)
  centerMesh.rotateX(-halfPI)
  centerMesh.name = 'virtualElement'
  centerMesh.renderOrder = RenderOrder.circleRoadLine
  centerMesh.position.set(center.x, center.y, center.z)
  parent.add(centerMesh)

  return circleOptions
}

// 渲染预览的虚拟整体环形道路
export function renderVirtualCircleRoad (params: IRenderCircleRoadParams) {
  const { parent, radiusPoint, center, radius, zoom = 1 } = params

  // 渲染从圆心指向圆环半径上的辅助线
  const points = [radiusPoint, center]
  const geo = new BufferGeometry().setFromPoints(points)
  const line = new Line(geo, dashLineMat)
  line.computeLineDistances()
  line.name = 'virtualElement'

  parent.add(line)

  // 创建半径长度的文字描述精灵
  const radiusDesc = `${fixedPrecision(radius)}m`
  // 判断当前相机的类型，以及不同类型相机的距离，调整文字提示的尺寸

  const sprite = createTextSprite({
    content: radiusDesc,
    color: activeColor,
  })
  if (sprite) {
    const spritePos = new Vector3(
      (center.x + radiusPoint.x) / 2,
      (center.y + radiusPoint.y) / 2,
      (center.z + radiusPoint.z) / 2,
    )
    // 半径中点的位置再向上偏移一些
    spritePos.x += 3

    sprite.position.set(spritePos.x, spritePos.y, spritePos.z)
    const ratio = sprite.scale.x / sprite.scale.y
    let _scale = 1
    const maxScale = 5
    const minScale = 4.5
    if (root3d.core.mainCamera === root3d.core.oCamera) {
      // 正交视角
      _scale = (sprite.scale.y / zoom) * 20
    } else {
      // 透视视角
      _scale = (sprite.scale.y / Math.sqrt(zoom)) * 50
      // 限制在最大和最小区间中
      _scale = Math.max(minScale, Math.min(maxScale, _scale))
    }
    sprite.scale.set(_scale * ratio, _scale, 1)
    parent.add(sprite)
  }

  // 创建预览的虚拟道路效果
  const circleOptions = createVirtualCircleRoad(params)

  // 创建圆形的基准点
  const centerMesh = new Mesh(centerGeo, centerMat)
  centerMesh.rotateX(-halfPI)
  centerMesh.name = 'virtualElement'
  centerMesh.renderOrder = RenderOrder.circleRoadLine
  centerMesh.position.set(center.x, center.y, center.z)
  parent.add(centerMesh)

  return circleOptions
}

// 销毁辅助交互的元素
export function disposeHelperElement (params: {
  parent: Object3D
  options?: { keepArea?: boolean }
}) {
  const { parent, options } = params
  const { keepArea = false } = options || {}
  const matchLine: Array<Line | Mesh> = []
  const matchGroup: Array<Group> = []
  parent.traverse((child) => {
    if (
      (child instanceof Line && child.name === 'helperLine') ||
      (child instanceof Mesh && child.name === 'helperPoint')
    ) {
      // 销毁所有的辅助线
      child.geometry.dispose()
      disposeMaterial(child)
      matchLine.push(child)
    } else if (child instanceof Mesh && child.name === 'helperArea') {
      // 如果要保留交互区域，只删除辅助线
      if (keepArea) return

      // 辅助交互区域，是嵌套在 Group 中
      child.geometry.dispose()
      disposeMaterial(child)
      matchGroup.push(child.parent as Group)
    }
  })
  parent.remove(...matchLine)
  matchGroup.forEach((group) => {
    group.clear()
  })
  parent.remove(...matchGroup)
}

// 销毁所有的预览虚拟元素
export function disposeAllVirtualElement (parent: Object3D) {
  const matchChildren: Array<Line | Sprite | Mesh> = []
  parent.traverse((child) => {
    if (
      (child instanceof Line || child instanceof Mesh) &&
      child.name === 'virtualElement'
    ) {
      // 销毁所有的辅助线
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    } else if (child instanceof Sprite && child.name === 'textSprite') {
      // 销毁所有的文字提示
      child.geometry.dispose()
      child.material.dispose()
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}
