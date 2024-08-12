import {
  BufferAttribute,
  BufferGeometry,
  DynamicDrawUsage,
  Line,
  LineBasicMaterial,
  Mesh,
  MeshBasicMaterial,
  type Object3D,
} from 'three'
import { getBoundaryById } from '@/stores/road/boundary'
import { getLaneById } from '@/stores/road/lane'
import { RenderOrder } from '@/utils/business'
import { disposeMaterial, transformVec3ByObject } from '@/utils/common3d'

export const BoundaryHelperLineColor = {
  normal: 0xAAA6EB,
  hovered: 0x00FFFF,
  selected: 0xAC342A,
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
    helper.boundaryId = rbid
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
