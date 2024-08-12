import {
  BufferGeometry,
  CatmullRomCurve3,
  CircleGeometry,
  Color,
  Line,
  LineBasicMaterial,
  LineDashedMaterial,
  Mesh,
  MeshBasicMaterial,
} from 'three'
import type {
  Group,
  Object3D,
} from 'three'
import { useRoadInteractionStore } from '../store/interaction'
import {
  Constant,
  RenderOrder,
  getParallelCurveSamplePoints,
} from '@/utils/business'
import {
  CatmullromTension,
  CurveType,
  disposeMaterial,
  halfPI,
  transformVec3ByObject,
} from '@/utils/common3d'

// cpId: 某一组控制点的 id 标识
interface IBasicParams {
  cpId?: string
  parent: Object3D
}

interface IRenderPointParams extends IBasicParams {
  cpId: string
  points?: Array<common.vec3WithId>
  preDispose?: boolean
}

interface IRenderCurveParams extends IBasicParams {
  cpId: string
  keyPath?: biz.ICurve3
  preDispose?: boolean
}

export const ControlPointColor = {
  normal: 0xAF7DFF,
  hovered: 0xFF9800,
  selected: 0xB42A1F,
}

export const ControlLineColor = {
  normal: 0x00FFFF,
  hovered: 0xFF9800,
  selected: 0xFF0000,
}

const controlPointGeo = new CircleGeometry(1, 16)

/**
 * 创建虚拟的道路结构
 * @param params
 */
export function renderVirtualRoad (params: {
  keyPath: biz.ICurve3
  road: biz.IRoad
  parent: Object3D
}) {
  const { keyPath, road, parent } = params
  // 获取整条道路最大的宽度
  // let maxWidth = 0
  // 由于支持了双向道路，需要判断正向车道和反向车道各自的最大宽度
  let forwardMaxWidth = 0
  let reverseMaxWidth = 0
  for (const section of road.sections) {
    // 每个 section 的最大宽度
    let forwardWidthInSection = 0
    let reverseWidthInSection = 0
    for (const lane of section.lanes) {
      if (Number(lane.id) > 0) {
        reverseWidthInSection += lane.normalWidth
      } else {
        forwardWidthInSection += lane.normalWidth
      }
    }

    if (forwardWidthInSection > forwardMaxWidth) {
      forwardMaxWidth = forwardWidthInSection
    }
    if (reverseWidthInSection > reverseMaxWidth) {
      reverseMaxWidth = reverseWidthInSection
    }
  }
  // 参考线上的采样点
  const { refPoints, offsetPoints: forwardPoints } =
    getParallelCurveSamplePoints({
      keyPath,
      elevationPath: road.elevationPath,
      offset: forwardMaxWidth,
      segment: Constant.helperCurveSamplePointsNumber,
    })
  const { offsetPoints: reversePoints } = getParallelCurveSamplePoints({
    keyPath,
    elevationPath: road.elevationPath,
    offset: reverseMaxWidth,
    segment: Constant.helperCurveSamplePointsNumber,
    side: 1, // 反向车道
  })

  const previewPoints = [...reversePoints, ...forwardPoints.reverse()]
  // 插入数组的第一个采样点，实现首尾闭合效果
  previewPoints.push(previewPoints[0])
  previewPoints.push(refPoints[0])
  previewPoints.push(...refPoints)

  const boundaryGeo = new BufferGeometry().setFromPoints(previewPoints)
  const boundaryMat = new LineDashedMaterial({
    color: 0xFFFFFF,
    depthTest: false,
    dashSize: 2,
    gapSize: 2,
  })
  const boundaryCurve = new Line(boundaryGeo, boundaryMat)
  boundaryCurve.computeLineDistances()
  boundaryCurve.name = 'virtualElement'
  boundaryCurve.renderOrder = RenderOrder.controlLine
  parent.add(boundaryCurve)
}

/**
 * 渲染虚拟辅助的控制点和控制线
 * @param params
 */
export function renderVirtualControlPoint (params: {
  points: Array<common.vec3>
  parent: Object3D
}) {
  const { points, parent } = params

  const pointMat = new MeshBasicMaterial({
    color: ControlPointColor.hovered,
    depthTest: false,
  })
  for (const p of points) {
    const point = new Mesh(controlPointGeo.clone(), pointMat)
    point.name = 'virtualElement'
    point.position.set(p.x, p.y, p.z)
    point.rotateX(-halfPI)
    point.renderOrder = RenderOrder.controlPoint
    parent.add(point)
  }
}

/**
 * 将虚拟辅助的元素从容器中销毁
 * @param params
 */
export function disposeAllVirtualElement (parent: Object3D) {
  const matchChildren: Array<Mesh | Line> = []
  parent.traverse((child) => {
    if (child instanceof Mesh || child instanceof Line) {
      if (child.name === 'virtualElement') {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 高亮选中的参考线颜色
 * @param params
 */
export function activeSelectedControlLineColor (params: {
  controlPointId: string
  parent: Object3D
}) {
  const { controlPointId, parent } = params
  parent.traverse((child) => {
    if (
      child instanceof Line &&
      child.name === 'controlLine' &&
      child.controlPointId === controlPointId
    ) {
      const material: LineBasicMaterial = child.material
      if (Array.isArray(material)) {
        material.forEach((m) => {
          m.color.set(ControlLineColor.selected)
          m.needsUpdate = true
        })
      } else {
        material.color.set(ControlLineColor.selected)
        material.needsUpdate = true
      }
    }
  })
}

/**
 * 重置所有参考线的颜色
 * @param parent
 */
export function resetControlLineColor (parent: Object3D) {
  parent.traverse((child) => {
    if (child instanceof Line && child.name === 'controlLine') {
      const { material } = child
      if (!new Color(ControlLineColor.normal).equals(material.color)) {
        material.color.set(ControlLineColor.normal)
        material.needsUpdate = true
      }
    }
  })
}

/**
 * 高亮选中的参考线控制点颜色
 * @param params
 */
export function activeSelectedControlPointColor (params: {
  refPointId: string
  parent: Object3D
}) {
  const { refPointId, parent } = params
  const pointMesh = parent.getObjectByProperty('pointId', refPointId)
  if (!pointMesh || !(pointMesh instanceof Mesh)) return
  if (Array.isArray(pointMesh.material)) {
    pointMesh.material.forEach((m) => {
      m.color.set(ControlPointColor.selected)
      m.needsUpdate = true
    })
  } else {
    pointMesh.material.color.set(ControlPointColor.selected)
    pointMesh.material.needsUpdate = true
  }
}

/**
 * 重置所有参考线控制点的颜色
 * @param parent
 */
export function resetAllControlPointColor (parent: Object3D) {
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'controlPoint') {
      const { material } = child
      if (!new Color(ControlPointColor.normal).equals(material.color)) {
        material.color.set(ControlPointColor.normal)
        material.needsUpdate = true
      }
    }
  })
}

/**
 * 渲染所有的控制点和形成的辅助曲线
 * @param controlPoints
 * @param parent
 */
export function renderAllControlPointAndCurve (params: {
  controlPoints: Array<biz.IControlPoint>
  parent: Group
  onlyIsolated?: boolean
}) {
  const { controlPoints, parent, onlyIsolated = false } = params
  for (const cps of controlPoints) {
    const { id, points } = cps
    // 如果是只渲染孤立控制点的模式，则跳过控制点数量超过 1 的点集
    if (onlyIsolated && points.length > 1) continue
    renderOneRoadControlPointAndCurve({
      cpId: id,
      parent,
      points,
      preDispose: true,
    })
  }
}

/**
 * 销毁所有的控制点和控制线的实例
 * @param controlPoints
 * @param parent
 */
export function disposeAllControlPointsAndCurve (parent: Group) {
  disposeControlPoint({
    parent,
  })
  disposeControlCurve({
    parent,
  })
}

/**
 * 单独渲染某一条道路的控制点和控制线
 * @param params
 * @returns
 */
export function renderOneRoadControlPointAndCurve (params: IRenderPointParams) {
  const { cpId, parent, points, preDispose = true } = params
  if (!points) return
  // 由于控制线的绘制有控制点的数量限制，需要手动提前销毁之前的控制线实例
  disposeControlCurve({
    cpId,
    parent,
  })

  if (points.length >= 2) {
    // 先渲染控制曲线
    const keyPath = new CatmullRomCurve3(transformVec3ByObject(points))
    keyPath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      keyPath.tension = CatmullromTension
    }
    renderControlCurve({
      parent,
      cpId,
      keyPath,
      preDispose: false,
    })
  }
  // 后渲染控制点
  renderControlPoint({
    parent,
    cpId,
    points,
    preDispose,
  })
}

/**
 * 单独销毁某一条道路的控制点和控制线
 * @param params
 */
export function disposeOneRoadControlPointAndCurve (params: IBasicParams) {
  const { cpId, parent } = params
  disposeControlPoint({
    parent,
    cpId,
  })
  disposeControlCurve({
    parent,
    cpId,
  })
}

export function renderControlPoint (params: IRenderPointParams) {
  const { parent, cpId, points, preDispose = false } = params
  if (preDispose) {
    disposeControlPoint(params)
  }
  if (!points || points.length < 1) return

  const roadInteractionStore = useRoadInteractionStore()
  // 如果存在多选的控制点
  const moreRefPointIds = roadInteractionStore.moreRefPoints.map(p => p.id)

  points.forEach((p) => {
    let color = ControlPointColor.normal
    // 在每次重绘时，判断当前的控制点是否是已选中的控制点，如果是，则更换选中的颜色
    if (
      roadInteractionStore?.refPoint?.id === p.id ||
      moreRefPointIds.includes(p.id)
    ) {
      color = ControlPointColor.selected
    }
    const pointMat = new MeshBasicMaterial({
      color,
      depthTest: false,
    })
    const point = new Mesh(controlPointGeo.clone(), pointMat)
    point.position.set(p.x, p.y, p.z)
    point.name = 'controlPoint'
    // TODO 是否需要给封装一层，保证类型注释能够正常解析???
    point.pointId = p.id
    point.controlPointId = cpId
    point.rotateX(-halfPI)
    point.renderOrder = RenderOrder.controlPoint
    parent.add(point)
  })
}
/**
 * 销毁 roadId 匹配上的控制点，或删除全部控制点
 * @param params
 */
export function disposeControlPoint (params: IBasicParams) {
  const { parent, cpId = '' } = params
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'controlPoint') {
      // 如果 roadId 不存在，则删除所有的 controlPoint
      // 如果元素 roadId 属性匹配，则删除匹配上的 controlPoint
      if (!cpId || cpId === child.controlPointId) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })

  parent.remove(...matchChildren)
}

export function renderControlCurve (params: IRenderCurveParams) {
  const { parent, cpId, keyPath, preDispose = false } = params
  if (preDispose) {
    disposeControlCurve(params)
  }
  if (!keyPath) return
  const points = keyPath.getSpacedPoints(Constant.helperCurveSamplePointsNumber)
  // 每次重绘判断当前参考线是否有选中，有选中替换颜色
  let color = ControlLineColor.normal
  const interactionStore = useRoadInteractionStore()
  if (interactionStore.refLineCpId && interactionStore.refLineCpId === cpId) {
    color = ControlLineColor.selected
  }
  const curveGeo = new BufferGeometry().setFromPoints(points)
  const curveMat = new LineBasicMaterial({
    color,
    depthTest: false,
  })
  const curve = new Line(curveGeo, curveMat)
  curve.name = 'controlLine'
  curve.controlPointId = cpId
  curve.renderOrder = RenderOrder.controlLine
  parent.add(curve)
}

/**
 * 销毁 roadId 匹配上的控制曲线，或删除全部控制曲线
 * @param params
 */
export function disposeControlCurve (params: IBasicParams) {
  const { parent, cpId = '' } = params
  const matchChildren: Array<Line> = []
  parent.traverse((child) => {
    if (child instanceof Line && child.name === 'controlLine') {
      if (!cpId || cpId === child.controlPointId) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}
