import type {
  Vector3,
} from 'three'
import {
  BufferGeometry,
  Group,
  Line,
  LineBasicMaterial,
  Mesh,
  MeshBasicMaterial,
  Points,
  PointsMaterial,
  SphereGeometry,
} from 'three'
import { disposeMaterial, transformVec3ByObject } from '@/utils/common3d'

interface IDisposeBoundary {
  roadId: string
  sectionId: string
  laneId: string
  parent: Group
}
interface IRenderBoundary {
  parent: Group
  road: biz.IRoad
  sectionId: string
  laneId: string
  preDispose: boolean
  leftPoints: Array<Vector3>
  rightPoints: Array<Vector3>
}

interface IRenderTweenCurve {
  roadId: string
  sectionId: string
  laneId: string
  controlPoints: Array<common.vec3WithId>
  parent: Group
  preDispose: boolean
  tweenSamplePoints: Array<Vector3>
}

/**
 * 通过控制点渲染补间的曲线，包括曲线和对应的控制点
 * @param params
 */
export function renderTweenCurve (params: IRenderTweenCurve) {
  const {
    roadId,
    sectionId,
    laneId,
    controlPoints,
    parent,
    tweenSamplePoints,
  } = params
  // 先找当前车道辅助元素外层 group
  let helperGroup
  parent.traverse((child) => {
    if (
      child instanceof Group &&
      child.name === 'boundaryHelperContainer' &&
      child.roadId === roadId &&
      child.sectionId === sectionId &&
      child.laneId === laneId
    ) {
      helperGroup = child
    }
  })
  if (!helperGroup) return

  const cpMat = new MeshBasicMaterial({ color: 0xFF0000 })
  for (const cp of controlPoints) {
    const { x, y, z, id } = cp
    const cpGeo = new SphereGeometry(0.05, 8, 8)
    const cpMesh = new Mesh(cpGeo, cpMat.clone())
    cpMesh.position.set(x, y, z)
    cpMesh.name = 'tweenControlPoint'
    cpMesh.pointId = id
    cpMesh.roadId = roadId
    cpMesh.sectionId = sectionId
    cpMesh.laneId = laneId
    parent.add(cpMesh)
  }

  // 如果没有平滑过渡边界的补间采样点，说明当前没有平滑过渡的效果，只是单纯的截断，使用左右控制点连接直线划分
  let curvePoints = []
  if (tweenSamplePoints.length > 0) {
    curvePoints = tweenSamplePoints
  } else {
    curvePoints = transformVec3ByObject(controlPoints)
  }
  const curveGeo = new BufferGeometry()
  curveGeo.setFromPoints(curvePoints)
  const curveMat = new MeshBasicMaterial({
    color: 0x00FF00,
  })
  const curve = new Line(curveGeo, curveMat)
  curve.name = 'tweenCurve'
  curve.roadId = roadId
  curve.sectionId = sectionId
  curve.laneId = laneId
  parent.add(curve)
}

/**
 * 渲染某一条道路对应的最外侧车道边界线辅助元素
 * @param params
 * @returns
 */
export function renderBoundaryHelper (params: IRenderBoundary) {
  const {
    road,
    sectionId,
    laneId,
    parent,
    preDispose,
    leftPoints,
    rightPoints,
  } = params
  if (!road) return
  const { id: roadId } = road
  if (preDispose) {
    disposeBoundaryHelper({
      roadId,
      sectionId,
      laneId,
      parent,
    })
  }

  // 每一条车道对应一个 group 容器
  const boundaryHelperGroup = new Group()
  boundaryHelperGroup.name = 'boundaryHelperContainer'
  boundaryHelperGroup.roadId = roadId
  boundaryHelperGroup.sectionId = sectionId
  boundaryHelperGroup.laneId = laneId
  boundaryHelperGroup.visible = true
  parent.add(boundaryHelperGroup)

  // 车道左右的边界线
  const lbGeo = new BufferGeometry()
  lbGeo.setFromPoints(leftPoints)
  const rbGeo = new BufferGeometry()
  rbGeo.setFromPoints(rightPoints)

  const boundaryMat = new LineBasicMaterial({
    color: 0xFFFFFF,
  })
  // 左侧（内侧）边界线
  const lbLine = new Line(lbGeo, boundaryMat.clone())
  lbLine.name = 'boundaryHelper'
  lbLine.roadId = roadId
  lbLine.sectionId = sectionId
  lbLine.laneId = laneId
  lbLine.side = 'left'
  boundaryHelperGroup.add(lbLine)

  // 右侧（外侧）边界线
  const rbLine = new Line(rbGeo, boundaryMat.clone())
  rbLine.name = 'boundaryHelper'
  rbLine.roadId = roadId
  rbLine.sectionId = sectionId
  rbLine.laneId = laneId
  rbLine.side = 'right'
  boundaryHelperGroup.add(rbLine)

  // 左侧边界上的预设点
  const presetPointMat = new PointsMaterial({ size: 0.2 })
  const lPointGeo = new BufferGeometry()
  lPointGeo.setFromPoints(leftPoints)
  const rPointGeo = new BufferGeometry()
  rPointGeo.setFromPoints(rightPoints)

  // 左侧
  const lPoint = new Points(lPointGeo, presetPointMat)
  // lPoint.name === 'presetPointHelper'
  lPoint.roadId = roadId
  lPoint.sectionId = sectionId
  lPoint.laneId = laneId
  lPoint.side = 'left'
  boundaryHelperGroup.add(lPoint)
  // 右侧
  const rPoint = new Points(rPointGeo, presetPointMat)
  // rPoint.name === 'presetPointHelper'
  rPoint.roadId = roadId
  rPoint.sectionId = sectionId
  rPoint.laneId = laneId
  rPoint.side = 'right'
  boundaryHelperGroup.add(rPoint)
}

/**
 * 销毁某一条 road 的车道边界线辅助元素
 * @param params
 * @returns
 */
export function disposeBoundaryHelper (params: IDisposeBoundary) {
  const { roadId, sectionId, laneId, parent } = params
  const matchChildren: Array<Line | Points> = []
  let boundaryHelperContainer: Group | undefined
  // 先找外层的 group 元素
  parent.traverse((child) => {
    if (
      child instanceof Group &&
      child.name === 'boundaryHelperContainer' &&
      child.roadId === roadId &&
      child.sectionId === sectionId &&
      child.laneId === laneId
    ) {
      boundaryHelperContainer = child
    }
  })

  if (!boundaryHelperContainer) return
  // group 内部的所有辅助元素都需要销毁
  boundaryHelperContainer.traverse((child) => {
    if (
      (child instanceof Line && child.name === 'boundaryHelper') ||
      (child instanceof Points && child.name === 'presetPointHelper')
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  boundaryHelperContainer.remove(...matchChildren)
  boundaryHelperContainer.clear()

  parent.remove(boundaryHelperContainer)
}

/**
 * 销毁所有的车道边界线辅助元素
 * @param parent
 */
export function disposeAllBoundaryHelper (parent: Group) {
  // 获取所有道路的边界线 helper 容器
  const boundaryHelperContainers: Array<Group> = []
  parent.traverse((child) => {
    if (child instanceof Group && child.name === 'boundaryHelperContainer') {
      boundaryHelperContainers.push(child)
    }
  })

  // 销毁所有容器内的辅助元素
  boundaryHelperContainers.forEach((container) => {
    const matchChildren: Array<Line | Points> = []
    container.traverse((child) => {
      if (
        (child instanceof Line && child.name === 'boundaryHelper') ||
        (child instanceof Points && child.name === 'presetPointHelper')
      ) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    })
    container.remove(...matchChildren)
    container.clear()
  })

  parent.remove(...boundaryHelperContainers)
}
