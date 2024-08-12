import {
  BufferAttribute,
  BufferGeometry,
  CatmullRomCurve3,
  DynamicDrawUsage,
  Line,
  LineBasicMaterial,
  LineDashedMaterial,
  Mesh,
  MeshBasicMaterial,
  type Object3D,
} from 'three'
import { getBoundaryById } from '@/stores/road/boundary'
import { getLaneById } from '@/stores/road/lane'
import {
  Constant,
  RenderOrder,
  getParallelCurveSamplePoints,
} from '@/utils/business'
import {
  CatmullromTension,
  CurveType,
  disposeMaterial,
  sortByAbsId,
  transformVec3ByObject,
} from '@/utils/common3d'

export const BoundaryHelperLineColor = {
  normal: 0xAAA6EB,
  hovered: 0x00FFFF,
  selected: 0xAC342A,
}

/**
 * 渲染预览的虚拟车道边界辅助线
 * @param params
 */
export function renderVirtualBoundaryLine (params: {
  lanes: Array<biz.ILane>
  boundarys: Array<biz.ILaneBoundary>
  parent: Object3D
  targetLaneId: string
  laneWidthOffset: number // 新增的车道宽度偏移量
}) {
  const { parent, lanes, boundarys, targetLaneId, laneWidthOffset } = params

  // 基于最内侧车道的左边界，作为虚拟边界计算的参考线
  let refKeyPath
  let refBoundary
  // 迭代车道数组，计算从参考线到目标车道右边界已经叠加的偏移量
  let offset = 0
  // 是否遍历到目标的车道
  let findTargetLane = false

  const boundaryMat = new LineDashedMaterial({
    color: BoundaryHelperLineColor.selected,
    depthTest: false,
    dashSize: 2,
    gapSize: 2,
  })
  // 在支持双向道路的前提下，区分正向车道和反向车道
  const isForward = Number(targetLaneId) < 0
  const forwardLanes = lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)
  const targetLanes = isForward ? forwardLanes : reverseLanes
  for (const lane of targetLanes) {
    const { normalWidth, id, lbid, rbid } = lane

    if (isForward) {
      // 基于最内侧车道的左边界采样点，创建一条参考线
      if (id === '-1' && !refKeyPath) {
        refBoundary = getBoundaryById(boundarys, lbid)
      }
    } else {
      if (id === '1' && !refKeyPath) {
        refBoundary = getBoundaryById(boundarys, rbid)
      }
    }
    // 如果没有找到基准的边界线，则直接返回
    if (!refBoundary) return

    refKeyPath = new CatmullRomCurve3(
      transformVec3ByObject(refBoundary.samplePoints),
    )
    refKeyPath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      refKeyPath.tension = CatmullromTension
    }

    if (id === targetLaneId) {
      findTargetLane = true
      offset += normalWidth + laneWidthOffset
    } else {
      offset += normalWidth
    }

    // 从目标车道才开始渲染虚拟的车道边界辅助元素用于预览新的位置
    if (findTargetLane) {
      const { offsetPoints } = getParallelCurveSamplePoints({
        keyPath: refKeyPath,
        offset,
        segment: Constant.helperCurveSamplePointsNumber,
        side: isForward ? 0 : 1,
      })

      const boundaryGeo = new BufferGeometry().setFromPoints(offsetPoints)
      const boundaryLine = new Line(boundaryGeo, boundaryMat)
      boundaryLine.computeLineDistances()
      boundaryLine.name = 'virtualElement'
      boundaryLine.renderOrder = RenderOrder.laneBoundaryHelperLine
      parent.add(boundaryLine)
    }
  }
}

/**
 * 销毁所有的预览虚拟元素
 * @param parent
 */
export function disposeAllVirtualElement (parent: Object3D) {
  const matchChildren: Array<Line> = []
  parent.traverse((child) => {
    if (child instanceof Line && child.name === 'virtualElement') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 更新某一条车道边界辅助线的颜色
 * @param params
 */
export function updateBoundaryHelperLineColor (params: {
  parent: Object3D
  roadId: string
  sectionId: string
  boundaryId: string
  status: common.objectStatus
}) {
  const { parent, roadId, sectionId, boundaryId, status } = params
  parent.traverse((child) => {
    if (
      child instanceof Line &&
      child.name === 'laneBoundaryHelperLine' &&
      child.roadId === roadId &&
      child.sectionId === sectionId &&
      child.boundaryId === boundaryId
    ) {
      const material: LineBasicMaterial = child.material
      if (Array.isArray(material)) {
        material.forEach((m) => {
          m.color.set(BoundaryHelperLineColor[status])
          m.needsUpdate = true
        })
      } else {
        material.color.set(BoundaryHelperLineColor[status])
        material.needsUpdate = true
      }
    }
  })
}

/**
 * 在选中的车道上方渲染一层蒙层
 * @param params
 */
export function renderLaneMask (params: {
  parent: Object3D
  roadId: string
  section: biz.ISection
  laneId: string
}) {
  const { parent, roadId, section, laneId } = params
  const lane = getLaneById(section.lanes, laneId)
  if (!lane || !lane.geoAttr) return
  const { vertices, indices } = lane.geoAttr

  const _vertices =
    vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
  const _indices =
    indices instanceof Float32Array ?
      Array.prototype.slice.call(indices) :
      indices

  // 创建车道蒙层的网格
  const maskGeo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  maskGeo.setIndex(_indices)
  maskGeo.setAttribute('position', positionAttr)
  maskGeo.computeVertexNormals()

  const maskMat = new MeshBasicMaterial({
    color: 0x000000,
    transparent: true,
    opacity: 0.1,
    depthTest: false,
  })
  const maskMesh = new Mesh(maskGeo, maskMat)
  maskMesh.name = 'laneMask'
  maskMesh.roadId = roadId
  maskMesh.sectionId = section.id
  maskMesh.laneId = laneId
  maskMesh.renderOrder = RenderOrder.laneMask
  parent.add(maskMesh)
}

/**
 * 销毁车道蒙层辅助元素
 * @param params
 */
export function disposeLaneMask (params: {
  parent: Object3D
  roadId?: string
  sectionId?: string
  laneId?: string
}) {
  const { parent, roadId = '', sectionId = '', laneId = '' } = params
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'laneMask') {
      // 如果不指定标识的 id，表示删除所有的车道蒙层
      // 如果存在指定的 id，则只删除匹配的车道蒙层
      if (
        !roadId ||
        !sectionId ||
        !laneId ||
        (child.roadId === roadId &&
        child.sectionId === sectionId &&
        child.laneId === laneId)
      ) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 渲染某一个 section 的车道边界辅助线
 * @param params
 */
export function renderOneSectionBoundaryHelperLine (params: {
  parent: Object3D
  roadId: string
  section: biz.ISection
}) {
  const { parent, roadId, section } = params

  const helperMat = new LineBasicMaterial({
    color: BoundaryHelperLineColor.normal,
    depthTest: false,
  })

  for (const lane of section.lanes) {
    const { rbid, lbid, id } = lane
    const isForward = Number(id) < 0
    let boundary
    // 判断是正向车道还是反向车道
    if (isForward) {
      // 正向车道辅助线用右边界
      boundary = getBoundaryById(section.boundarys, rbid)
    } else {
      // 反向车道辅助线用左边界
      boundary = getBoundaryById(section.boundarys, lbid)
    }
    if (!boundary) continue

    const { samplePoints } = boundary
    const points = transformVec3ByObject(samplePoints)
    const helperGeo = new BufferGeometry().setFromPoints(points)
    const helper = new Line(helperGeo, helperMat.clone())
    helper.name = 'laneBoundaryHelperLine'
    helper.roadId = roadId
    helper.sectionId = section.id
    helper.laneId = lane.id
    helper.boundaryId = isForward ? rbid : lbid
    helper.renderOrder = RenderOrder.laneBoundaryHelperLine
    parent.add(helper)
  }
}

/**
 * 销毁匹配的车道边界辅助线
 * @param params
 */
export function disposeBoundaryHelperLine (params: {
  parent: Object3D
  roadId?: string
  sectionId?: string
}) {
  const { parent, roadId = '', sectionId = '' } = params
  const matchChildren: Array<Line> = []
  parent.traverse((child) => {
    if (child instanceof Line && child.name === 'laneBoundaryHelperLine') {
      // roadId 或 sectionId 不存在，表示删除所有的车道边界辅助线
      // roadId 和 sectionId 存在，只删除匹配的车道边界辅助线
      if (
        !roadId ||
        !sectionId ||
        (child.roadId === roadId && child.sectionId === sectionId)
      ) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    }
  })
  parent.remove(...matchChildren)
}
