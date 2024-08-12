import { BufferGeometry, Line, LineBasicMaterial, type Object3D } from 'three'
import { getBoundaryById } from '@/stores/road/boundary'
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
    const { rbid, lbid, id: laneId } = lane
    if (laneId === '-1') {
      // 如果是最内侧车道，则需要展示最内侧车道左边界辅助线
      const leftBoundary = getBoundaryById(section.boundarys, lbid)
      if (leftBoundary) {
        const leftSamplePoints = transformVec3ByObject(
          leftBoundary.samplePoints,
        )
        const leftHelperGeo = new BufferGeometry().setFromPoints(
          leftSamplePoints,
        )
        const leftHelper = new Line(leftHelperGeo, helperMat.clone())
        leftHelper.name = 'laneBoundaryHelperLine'
        leftHelper.roadId = roadId
        leftHelper.sectionId = section.id
        // 绑定最内侧车道左边界 id
        leftHelper.boundaryId = lbid
        leftHelper.renderOrder = RenderOrder.laneBoundaryHelperLine
        parent.add(leftHelper)
      }
    }

    // 获取当前车道的右侧车道边界线
    const rightBoundary = getBoundaryById(section.boundarys, rbid)
    if (!rightBoundary) continue

    const { samplePoints } = rightBoundary
    const points = transformVec3ByObject(samplePoints)
    const helperGeo = new BufferGeometry().setFromPoints(points)
    const helper = new Line(helperGeo, helperMat.clone())
    helper.name = 'laneBoundaryHelperLine'
    helper.roadId = roadId
    helper.sectionId = section.id
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
