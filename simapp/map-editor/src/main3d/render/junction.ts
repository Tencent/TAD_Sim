// 交叉路口的渲染函数
import type {
  Vector3,
} from 'three'
import {
  BufferAttribute,
  BufferGeometry,
  DoubleSide,
  DynamicDrawUsage,
  Group,
  Line,
  Mesh,
  MeshBasicMaterial,
} from 'three'
import { getLaneLinkUniqueFlag } from '@/stores/junction/link'
import { RenderOrder } from '@/utils/business'
import { disposeMaterial } from '@/utils/common3d'

export const linkColor = {
  normal: 0x00FFFF,
  hovered: 0xFFFF00,
  linked: 0xFF3399, // 连接线两端的点被选中，此时该 link 的颜色状态
  selected: 0xFF0001, // 待删除的 link 颜色
}

interface IBasicInfo {
  junctionId: string
  parent: Group
}

interface IRenderLinksParams extends IBasicInfo {
  laneLinks: Array<biz.ILaneLink>
}

interface IRenderLinkParam extends IBasicInfo {
  laneLink: biz.ILaneLink
}

interface IDisposeLinkParams {
  laneLinkId: string
  parent: Group
}

// 可共用的材质
const junctionMat = new MeshBasicMaterial({
  color: 0x777777,
  depthTest: false,
  side: DoubleSide,
})
export const linkNormalMat = new MeshBasicMaterial({
  color: linkColor.normal,
  depthTest: false,
})

/**
 * 单独销毁某一条交叉路口，包括交叉路口自身实例和对应 link 线的实例
 */
export function disposeOneJunction (params: {
  junctionId: string
  parent: Group
}) {
  const { junctionId, parent } = params
  disposeJunction({
    junctionId,
    parent,
  })
  disposeJunctionLinks({
    junctionId,
    parent,
  })
}

/**
 * 单独渲染某一个交叉路口
 * @returns
 */
export function renderJunction (params: {
  junction: biz.IJunction
  parent: Group
  preDispose?: boolean
}) {
  const { junction, parent, preDispose = true } = params
  return new Promise((resolve) => {
    const { id, laneLinks, geoAttr, linkRoads } = junction
    // 如果交叉路口只包含了一条道路的边界，那么就不绘制交叉路口的几何形状
    if (linkRoads.length < 1) return
    if (!geoAttr) return
    const { vertices, indices } = geoAttr
    if (vertices.length < 1 || indices.length < 1) return

    if (preDispose) {
      // 在重新渲染路口前，销毁旧的路口
      disposeOneJunction({
        junctionId: id,
        parent,
      })
    }

    // 绘制路口的网格
    renderJunctionMesh({
      vertices,
      indices,
      junctionId: id,
      parent,
    })

    // 绘制交叉路口对应 link 连接线的结构
    renderJunctionLinks({
      junctionId: id,
      parent,
      laneLinks,
    })

    resolve(id)
  })
}

// 生成交叉路口网格
function renderJunctionMesh (params: {
  vertices: Array<number>
  indices: Array<number>
  junctionId: string
  parent: Group
}) {
  const { vertices, indices, junctionId, parent } = params
  const junctionGeo = new BufferGeometry()

  const _vertices =
    vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
  const _indices =
    indices instanceof Float32Array ?
      Array.prototype.slice.call(indices) :
      indices

  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  junctionGeo.setIndex(_indices)
  junctionGeo.setAttribute('position', positionAttr)
  junctionGeo.computeVertexNormals()
  const junctionMesh = new Mesh(junctionGeo, junctionMat)
  junctionMesh.name = 'junction'
  junctionMesh.junctionId = junctionId
  junctionMesh.renderOrder = RenderOrder.junction
  junctionMesh.matrixAutoUpdate = false
  junctionMesh.updateMatrix()
  parent.add(junctionMesh)
}

/**
 * 销毁交叉路口网格
 * @param params
 */
export function disposeJunction (params: IBasicInfo) {
  const { junctionId, parent } = params
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'junction' &&
      child.junctionId === junctionId
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 绘制交叉路口中的 link 连接线
 * @param params
 */
export function renderJunctionLinks (params: IRenderLinksParams) {
  disposeJunctionLinks(params)

  const { junctionId, parent, laneLinks } = params

  // 当前交叉路口 link 的容器
  const linkGroup = new Group()
  linkGroup.name = 'junctionLinkContainer'
  linkGroup.junctionId = junctionId
  // 在渲染模式中，link 默认不显示
  linkGroup.visible = false
  linkGroup.matrixAutoUpdate = false
  linkGroup.updateMatrix()

  // 遍历 link 连接关系，基于对应 from 和 to 数据绘制 link 连接线
  for (const laneLink of laneLinks) {
    const {
      id,
      fid,
      frid,
      fsid,
      ftype,
      tid,
      trid,
      tsid,
      ttype,
      samplePoints,
      enabled,
    } = laneLink
    // 如果 laneLink 无效，先不渲染
    if (!enabled) continue

    const fDirection = Number(fid) < 0 ? 'forward' : 'reverse'
    const tDirection = Number(tid) < 0 ? 'forward' : 'reverse'

    const fromFlag = getLaneLinkUniqueFlag(frid, fsid, fid, ftype, fDirection)
    const toFlag = getLaneLinkUniqueFlag(trid, tsid, tid, ttype, tDirection)
    const laneLinkGeo = new BufferGeometry()
    laneLinkGeo.setFromPoints(samplePoints as Array<Vector3>)
    const linkLine = new Line(laneLinkGeo, linkNormalMat.clone())
    linkLine.name = 'junctionLink'
    linkLine.laneLinkId = id
    linkLine.junctionId = junctionId
    linkLine.fromInfo = fromFlag
    linkLine.toInfo = toFlag
    linkLine.renderOrder = RenderOrder.junctionLink
    linkLine.matrixAutoUpdate = false
    linkLine.updateMatrix()
    linkGroup.add(linkLine)
  }
  parent.add(linkGroup)
}

/**
 * 销毁交叉路口 link 线
 * @param params
 * @returns
 */
export function disposeJunctionLinks (params: IBasicInfo) {
  const { junctionId, parent } = params
  const matchChildren: Array<Line> = []
  let linkContainer: Group | undefined
  // 先获取 link 的容器
  parent.traverse((child) => {
    if (
      child instanceof Group &&
      child.name === 'junctionLinkContainer' &&
      child.junctionId === junctionId
    ) {
      linkContainer = child
    }
  })
  if (!linkContainer) return
  ;(linkContainer as Group).traverse((child) => {
    if (
      child instanceof Line &&
      child.name === 'junctionLink' &&
      child.junctionId === junctionId
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  linkContainer.remove(...matchChildren)
  linkContainer.clear()

  parent.remove(linkContainer)
}

/**
 * 销毁交叉路口的其中一条 link 线
 * @param params
 * @returns
 */
export function disposeJunctionLink (params: IDisposeLinkParams) {
  const { laneLinkId, parent } = params
  const targetLaneLink = parent.getObjectByProperty(
    'laneLinkId',
    laneLinkId,
  ) as Line | undefined
  if (!targetLaneLink) return
  const linkContainer = targetLaneLink.parent
  if (!linkContainer) return
  targetLaneLink.geometry.dispose()
  disposeMaterial(targetLaneLink)
  linkContainer.remove(targetLaneLink)
}

/**
 * 在指定交叉路口的添加一条 link 线
 * @param params:IRenderLinkParam
 * @returns
 */
export function renderJunctionLink (params: IRenderLinkParam) {
  const { junctionId, laneLink, parent } = params
  const {
    id,
    fid,
    frid,
    fsid,
    ftype,
    tid,
    trid,
    tsid,
    ttype,
    samplePoints,
    enabled,
  } = laneLink

  // 如果 laneLink 无效，先不渲染
  if (!enabled) return

  const fDirection = Number(fid) < 0 ? 'forward' : 'reverse'
  const tDirection = Number(tid) < 0 ? 'forward' : 'reverse'

  const fromFlag = getLaneLinkUniqueFlag(frid, fsid, fid, ftype, fDirection)
  const toFlag = getLaneLinkUniqueFlag(trid, tsid, tid, ttype, tDirection)
  const laneLinkGeo = new BufferGeometry()
  laneLinkGeo.setFromPoints(samplePoints as Array<Vector3>)
  const linkLine = new Line(laneLinkGeo, linkNormalMat.clone())
  linkLine.name = 'junctionLink'
  linkLine.laneLinkId = id
  linkLine.junctionId = junctionId
  linkLine.fromInfo = fromFlag
  linkLine.toInfo = toFlag
  linkLine.renderOrder = RenderOrder.junctionLink
  linkLine.matrixAutoUpdate = false
  linkLine.updateMatrix()

  parent.traverse((child) => {
    if (
      child.name === 'junctionLinkContainer' &&
      child.junctionId === junctionId
    ) {
      child.add(linkLine)
    }
  })
}

/**
 * 销毁所有的车道线
 * @param parent
 */
export function disposeAllJunctionAndLinks (parent: Group) {
  const matchJunction: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'junction') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchJunction.push(child)
    }
  })
  parent.remove(...matchJunction)

  // 获取所有的 linkContainer
  const linkContainers: Array<Group> = []
  parent.traverse((child) => {
    if (child instanceof Group && child.name === 'junctionLinkContainer') {
      linkContainers.push(child)
    }
  })

  // 销毁所有的 lane link 线
  linkContainers.forEach((container) => {
    const matchJunctionLinks: Array<Line> = []
    container.traverse((child) => {
      if (child instanceof Line && child.name === 'junctionLink') {
        child.geometry.dispose()
        disposeMaterial(child)
        matchJunctionLinks.push(child)
      }
    })
    container.remove(...matchJunctionLinks)
    container.clear()
  })

  parent.remove(...linkContainers)
}
