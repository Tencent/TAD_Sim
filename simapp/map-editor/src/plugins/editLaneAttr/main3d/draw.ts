import {
  BufferAttribute,
  BufferGeometry,
  Color,
  DynamicDrawUsage,
  Line,
  LineBasicMaterial,
  LineLoop,
  Mesh,
  MeshBasicMaterial,
  type Object3D,
} from 'three'
import { getBoundaryById } from '@/stores/road/boundary'
import { getLaneById } from '@/stores/road/lane'
import { RenderOrder } from '@/utils/business'
import { disposeMaterial, transformVec3ByObject } from '@/utils/common3d'

const BoundaryHelperColor = {
  normal: 0xAAA6EB,
  hovered: 0x00FFFF,
  selected: 0xAC342A,
}

/**
 * 更新某一条车道对应的边界线颜色
 * @param params
 */
export function updateLaneBoundaryHelperColor (params: {
  parent: Object3D
  roadId: string
  sectionId: string
  laneId: string
  status: common.objectStatus
  forced?: boolean // 是否强制更新颜色
}) {
  const { parent, roadId, sectionId, laneId, status, forced = false } = params
  parent.traverse((child) => {
    if (
      child instanceof Line &&
      child.name === 'laneBoundaryHelper' &&
      child.roadId === roadId &&
      child.sectionId === sectionId &&
      child.laneId === laneId
    ) {
      const material: LineBasicMaterial = child.material
      if (
        !forced &&
        new Color(BoundaryHelperColor.selected).equals(material.color)
      ) {
        // 如果是非强制更新，且当前线框的颜色已经为选中高亮，则不触发
        return
      }
      material.color.set(BoundaryHelperColor[status])
      material.needsUpdate = true

      let renderOrder = RenderOrder.laneBoundaryHelperLine
      if (status === 'hovered') {
        renderOrder += 1
      } else if (status === 'selected') {
        renderOrder += 2
      }
      // 为了保证 hovered 和 selected 状态的线框不存在遮挡，手动控制 renderOrder
      child.renderOrder = renderOrder
    }
  })
}

/**
 * 更新车道蒙层的透明度
 * @param params
 */
export function updateLaneMaskOpacity (params: {
  parent: Object3D
  roadId: string
  sectionId: string
  laneId: string
  opacity: number
}) {
  const { parent, roadId, sectionId, laneId, opacity } = params
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'laneMask' &&
      child.roadId === roadId &&
      child.sectionId === sectionId &&
      child.laneId === laneId
    ) {
      const material: MeshBasicMaterial = child.material
      material.opacity = opacity
      material.needsUpdate = true
    }
  })
}

/**
 * 渲染某一个 section 中所有车道的辅助元素
 * @param params
 */
export function renderOneSectionLaneHelper (params: {
  parent: Object3D
  roadId: string
  section: biz.ISection
}) {
  const { parent, roadId, section } = params

  for (const lane of section.lanes) {
    const { id: laneId } = lane

    // 渲染一个网格蒙层
    renderLaneMask({
      parent,
      roadId,
      section,
      laneId,
    })

    // 渲染当前蒙层对应的边界区域
    renderLaneBoundaryHelper({
      parent,
      roadId,
      section,
      lane,
    })
  }
}

/**
 * 销毁某一段 section 中的车道辅助蒙层
 * @param params
 */
export function disposeLaneHelper (params: {
  parent: Object3D
  roadId?: string
  sectionId?: string
}) {
  const { parent, roadId = '', sectionId = '' } = params
  const matchChildren: Array<Mesh | Line> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'laneMask') {
      // 如果不指定标识的 id，表示删除所有的车道蒙层
      // 如果存在指定的 id，则只删除匹配的车道蒙层
      if (
        !roadId ||
        !sectionId ||
        (child.roadId === roadId && child.sectionId === sectionId)
      ) {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    } else if (child instanceof Line && child.name === 'laneBoundaryHelper') {
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

/**
 * 渲染某一条车道周围的边界线
 * @param params
 */
export function renderLaneBoundaryHelper (params: {
  parent: Object3D
  roadId: string
  section: biz.ISection
  lane: biz.ILane
}) {
  const { parent, roadId, section, lane } = params
  const { id: sectionId, boundarys } = section
  const { rbid, lbid, id: laneId } = lane
  const leftBoundary = getBoundaryById(boundarys, lbid)
  const rightBoundary = getBoundaryById(boundarys, rbid)

  if (!leftBoundary || !rightBoundary) return

  const mat = new LineBasicMaterial({
    color: BoundaryHelperColor.normal,
    depthTest: false,
  })

  const points = []
  points.push(...transformVec3ByObject(leftBoundary.samplePoints))
  points.push(...transformVec3ByObject(rightBoundary.samplePoints).reverse())

  // 将车道左右两侧的边界线组合成闭合的线框
  const geo = new BufferGeometry().setFromPoints(points)
  const line = new LineLoop(geo, mat)
  line.name = 'laneBoundaryHelper'
  line.roadId = roadId
  line.sectionId = sectionId
  line.laneId = laneId
  line.renderOrder = RenderOrder.laneBoundaryHelperLine
  parent.add(line)
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
    opacity: 0, // 车道蒙层默认透明度为 0，选中后才更新透明度为 0.1
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
