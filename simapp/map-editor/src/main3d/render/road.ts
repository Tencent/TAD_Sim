// 道路级别元素的渲染函数

import {
  BufferAttribute,
  BufferGeometry,
  CatmullRomCurve3,
  Color,
  DoubleSide,
  DynamicDrawUsage,
  Group,
  Line,
  Mesh,
  MeshBasicMaterial,
} from 'three'
import {
  LaneTypeEnum,
  RenderOrder,
  getGeometryAttrBySamplePoints,
  getParallelCurveSamplePoints,
  parseLaneBoundaryMark,
} from '@/utils/business'
import {
  CurveType,
  DefaultColor,
  disposeMaterial,
  sortByAbsId,
  transformVec3ByObject,
} from '@/utils/common3d'
import { getLaneBoundaryOnBothSides } from '@/stores/road/lane'

interface IRenderRoadParams {
  road: biz.IRoad
  parent: Group
  preDispose?: boolean
}

interface IDisposeRoadParams {
  roadId: string
  parent: Group
}
interface IRenderLaneMesh {
  vertices: Array<number>
  indices: Array<number>
  uvs: Array<number>
  laneId: string
  sectionId: string
  roadId: string
  parent: Group
  linkContainer: Group
  lanePoints: Array<common.vec3>
  type: number
  roadType?: 'default' | 'tunnel'
}

// 不同车道类型，渲染对应的车道颜色
const laneColor = {
  Sidewalk: 0xD9DED4, // 人行道
  Biking: 0xC27572, // 自行车道
  Median: 0x717C36, // 中央隔离带
  Shoulder: 0xFAFAFA, // 路肩
  Border: 0xFAFAFA, // 边界
  Restricted: 0xFAFAFA, // 禁止驶入车道
}

// 可共用的材质
const laneMat = new MeshBasicMaterial({
  color: 0x777777,
  depthTest: false,
  side: DoubleSide,
})
const linkMat = new MeshBasicMaterial({
  color: 0x00FFFF,
  depthTest: false,
})
// 隧道蒙层的材质
const tunnelMaskMat = new MeshBasicMaterial({
  color: 0x566CA4,
  side: DoubleSide,
  depthTest: false,
})
const boundaryMats = {
  White: new MeshBasicMaterial({
    color: DefaultColor.White.value,
    depthTest: false,
    side: DoubleSide,
  }),
  Yellow: new MeshBasicMaterial({
    color: DefaultColor.Yellow.value,
    depthTest: false,
    side: DoubleSide,
  }),
  Red: new MeshBasicMaterial({
    color: DefaultColor.Red.value,
    depthTest: false,
    side: DoubleSide,
  }),
  Green: new MeshBasicMaterial({
    color: DefaultColor.Green.value,
    depthTest: false,
    side: DoubleSide,
  }),
  Blue: new MeshBasicMaterial({
    color: DefaultColor.Blue.value,
    depthTest: false,
    side: DoubleSide,
  }),
}

const roadMaskColor = {
  normal: 0xFFFFFF,
  hovered: 0xFFFFFF,
  selected: 0xFFFF00,
}
const roadMaskMat = new MeshBasicMaterial({
  color: roadMaskColor.selected,
  depthTest: false,
  transparent: true,
  opacity: 0.2,
})

/**
 * 更新隧道的蒙层
 * @param params
 */
export function updateTunnelMask (params: { road: biz.ITunnel, parent: Group }) {
  const { road, parent } = params
  if (!road) return
  const { roadType, tunnelS, tunnelLength, length: roadLength } = road
  if (roadType !== 'tunnel') return

  // 销毁旧的隧道蒙层
  disposeTunnelMask({
    roadId: road.id,
    parent,
  })

  // 当前道路最左和最右边界线所有采样点集合
  const allLeftPoints = []
  const allRightPoints = []

  // 遍历每一个section最外的左右边界线采样点
  for (const section of road.sections) {
    const { left: leftBoundary, right: rightBoundary } =
      getLaneBoundaryOnBothSides(section)
    if (!leftBoundary || !rightBoundary) continue

    for (let i = 0; i < leftBoundary.samplePoints.length; i++) {
      allLeftPoints.push(leftBoundary.samplePoints[i])
      allRightPoints.push(rightBoundary.samplePoints[i])
    }
  }

  // 将边界线采样点拟合成一条曲线，再根据隧道的起始点和长度来重新采样
  const startPercent = Math.max(0, tunnelS / roadLength) // 最小为 0
  const endPercent = Math.min(1, (tunnelS + tunnelLength) / roadLength) // 最大为 1

  const leftPath = new CatmullRomCurve3(transformVec3ByObject(allLeftPoints))
  leftPath.curveType = CurveType
  const rightPath = new CatmullRomCurve3(transformVec3ByObject(allRightPoints))
  rightPath.curveType = CurveType

  const { refPoints: leftPoints } = getParallelCurveSamplePoints({
    keyPath: leftPath,
    offset: 1,
    segment: allLeftPoints.length - 1,
    pStart: startPercent,
    pEnd: endPercent,
  })
  const { refPoints: rightPoints } = getParallelCurveSamplePoints({
    keyPath: rightPath,
    offset: 1,
    segment: allLeftPoints.length - 1,
    pStart: startPercent,
    pEnd: endPercent,
  })

  const { vertices, indices } = getGeometryAttrBySamplePoints(
    leftPoints,
    rightPoints,
  )
  const _vertices =
    vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
  const _indices =
    indices instanceof Float32Array ?
      Array.prototype.slice.call(indices) :
      indices

  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )

  geo.setIndex(_indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  const mesh = new Mesh(geo, tunnelMaskMat.clone())
  mesh.name = 'tunnelMask'
  mesh.roadId = road.id
  mesh.renderOrder = RenderOrder.tunnelMask
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

/**
 * 创建道路的高亮辅助蒙层
 * @param params
 */
export function createHighlightRoadMask (params: {
  road: biz.IRoad
  parent: Group
}) {
  const { road, parent } = params

  // 当前道路最左和最右边界线采样点集合
  const leftPoints = []
  const rightPoints = []

  // 遍历每一个section最外的左右边界线采样点
  for (const section of road.sections) {
    const { left: leftBoundary, right: rightBoundary } =
      getLaneBoundaryOnBothSides(section)
    if (!leftBoundary || !rightBoundary) continue

    for (let i = 0; i < leftBoundary.samplePoints.length; i++) {
      leftPoints.push(leftBoundary.samplePoints[i])
      rightPoints.push(rightBoundary.samplePoints[i])
    }
  }

  const { vertices, indices } = getGeometryAttrBySamplePoints(
    leftPoints,
    rightPoints,
  )
  const _vertices =
    vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
  const _indices =
    indices instanceof Float32Array ?
      Array.prototype.slice.call(indices) :
      indices

  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )

  geo.setIndex(_indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  const mesh = new Mesh(geo, roadMaskMat.clone())
  mesh.name = 'roadMask'
  mesh.roadId = road.id
  mesh.renderOrder = RenderOrder.roadMask
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

/**
 * 销毁道路的高亮蒙层
 */
export function disposeHighlightRoadMask (parent: Group) {
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'roadMask') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 渲染完成的整条道路
 * @param params
 */
export async function renderRoad (params: IRenderRoadParams) {
  const { parent, road, preDispose = true } = params
  return new Promise((resolve) => {
    // 渲染道路主体，主要是遍历渲染车道
    const { id: roadId, roadType = 'default' } = road
    if (preDispose) {
      disposeRoad({
        roadId: road.id,
        parent,
      })
    }

    // 创建一个用于承载所有车道 link 线的容器
    const linkContainer = new Group()
    linkContainer.name = 'roadLinkContainer'
    linkContainer.roadId = roadId
    // 默认不显示
    linkContainer.visible = false
    linkContainer.matrixAutoUpdate = false
    linkContainer.updateMatrix()
    parent.add(linkContainer)

    const renderAllChild = []
    for (const section of road.sections) {
      // 渲染 section 中的车道
      const { lanes, boundarys, id: sectionId } = section
      lanes.sort(sortByAbsId)
      // 获取当前车道的边界线
      for (const lane of lanes) {
        const {
          samplePoints,
          lbid,
          rbid,
          id: laneId,
          geoAttr,
          type: laneType,
        } = lane
        if (!geoAttr) continue
        const { vertices, indices, uvs = [] } = geoAttr
        // 通过车道边界线 id 找出对应车道边界线的数据
        const leftBoundary = boundarys.find(b => b.id === lbid)
        const rightBoundary = boundarys.find(b => b.id === rbid)
        if (!leftBoundary || !rightBoundary) continue
        if (vertices.length < 1 || indices.length < 1 || uvs.length < 1) {
          continue
        }
        // 将渲染每个车道网格的方法转换成异步函数
        renderAllChild.push(
          renderLaneMesh({
            vertices,
            indices,
            uvs,
            laneId,
            sectionId,
            roadId,
            parent,
            linkContainer,
            lanePoints: samplePoints, // 车道中心线的采样点
            type: laneType,
            roadType,
          }),
        )
      }

      // 渲染 section 中的车道边界线
      for (const boundary of section.boundarys) {
        // 将渲染每个车道边界线网格的方法转换成异步函数
        renderAllChild.push(
          renderLaneBoundary({
            roadId,
            boundary,
            parent,
          }),
        )
      }
    }

    if (road.roadType === 'tunnel') {
      updateTunnelMask({
        road: road as biz.ITunnel,
        parent,
      })
    }

    Promise.all(renderAllChild).then(() => {
      resolve(true)
    })
  })
}

/**
 * 渲染某一个 section 中的所有车道边界线
 * @param params
 */
function renderLaneBoundary (params: {
  roadId: string
  boundary: biz.ILaneBoundary
  parent: Group
}) {
  const { roadId, boundary, parent } = params
  return new Promise((resolve) => {
    const { id: boundaryId, mark, firstLineAttr, secondLineAttr } = boundary

    // 如果 mark 为 0x0000000，即为无边界，则不渲染当前边界
    if (!mark) return

    // 如果连第一条边界属性也没有，则返回
    if (!firstLineAttr) return

    // 获取车道渲染的颜色
    const { color: colorData } = parseLaneBoundaryMark(mark)

    const _boundaryMat = boundaryMats[colorData.name]

    // 第一条边界线
    if (firstLineAttr) {
      const { vertices, indices } = firstLineAttr
      const _vertices =
        vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
      const _indices =
        indices instanceof Float32Array ?
          Array.prototype.slice.call(indices) :
          indices
      const geo = new BufferGeometry()
      const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
        DynamicDrawUsage,
      )
      geo.setIndex(_indices)
      geo.setAttribute('position', positionAttr)
      geo.computeVertexNormals()

      // 基于车道边界线的几何体创建对应的网格
      const boundaryMesh = new Mesh(geo, _boundaryMat)
      boundaryMesh.name = 'laneBoundary'
      boundaryMesh.roadId = roadId
      boundaryMesh.boundaryId = boundaryId
      boundaryMesh.renderOrder = RenderOrder.laneBoundary
      boundaryMesh.matrixAutoUpdate = false
      boundaryMesh.updateMatrix()
      parent.add(boundaryMesh)
    }

    // 第二条边界线
    if (secondLineAttr) {
      const { vertices, indices } = secondLineAttr
      if (vertices && indices) {
        const _vertices = new Float32Array(vertices)
        const _indices =
          indices instanceof Float32Array ?
            Array.prototype.slice.call(indices) :
            indices
        const geo = new BufferGeometry()
        const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
          DynamicDrawUsage,
        )
        geo.setIndex(_indices)
        geo.setAttribute('position', positionAttr)
        geo.computeVertexNormals()

        // 基于车道边界线的几何体创建对应的网格
        const boundaryMesh = new Mesh(geo, _boundaryMat)
        boundaryMesh.name = 'laneBoundary'
        boundaryMesh.roadId = roadId
        boundaryMesh.boundaryId = boundaryId
        boundaryMesh.renderOrder = RenderOrder.laneBoundary
        boundaryMesh.matrixAutoUpdate = false
        boundaryMesh.updateMatrix()
        parent.add(boundaryMesh)
      }
    }

    resolve(true)
  })
}

/**
 * 生成车道网格（包括常规和平滑过渡车道）
 * @param params
 * @returns
 */
// TODO 采样点的频率（间隔）以及车道材质纹理的应用，可以优化
function renderLaneMesh (params: IRenderLaneMesh) {
  const {
    vertices,
    uvs,
    indices,
    laneId,
    sectionId,
    roadId,
    parent,
    linkContainer,
    lanePoints, // 车道中心线的采样点
    type: laneType,
    roadType = 'default',
  } = params
  return new Promise((resolve) => {
    // 创建车道的网格，并挂载自定义的属性
    const _vertices =
      vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
    const _uvs = uvs instanceof Float32Array ? uvs : new Float32Array(uvs)
    const _indices =
      indices instanceof Float32Array ?
        Array.prototype.slice.call(indices) :
        indices
    const laneGeo = new BufferGeometry()
    const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
      DynamicDrawUsage,
    )
    const uvAttr = new BufferAttribute(_uvs, 2).setUsage(DynamicDrawUsage)
    laneGeo.setIndex(_indices)
    laneGeo.setAttribute('position', positionAttr)
    laneGeo.setAttribute('uv', uvAttr)
    laneGeo.computeVertexNormals()

    // 测试车道应用重复纹理的效果
    // const texture = LocalMaps['testUV']
    // texture.wrapS = RepeatWrapping
    // texture.wrapT = RepeatWrapping
    // texture.repeat.set(3, 1)
    const currentLaneMat = laneMat.clone()
    // 基于实际的车道类型，更新对应的车道颜色
    const laneTypeName = LaneTypeEnum[laneType]
    if (roadType === 'default') {
      if (laneTypeName && laneColor[laneTypeName]) {
        currentLaneMat.color = new Color(laneColor[laneTypeName])
        currentLaneMat.needsUpdate = true
      }
    } else if (roadType === 'tunnel') {
      // 如果是隧道，需要使用不一样的车道颜色
      // currentLaneMat.color = new Color(0x566ca4)
      // currentLaneMat.needsUpdate = true
    }

    const laneMesh = new Mesh(laneGeo, currentLaneMat)
    laneMesh.name = 'lane'
    laneMesh.laneId = laneId
    laneMesh.sectionId = sectionId
    laneMesh.roadId = roadId
    laneMesh.renderOrder = RenderOrder.lane
    laneMesh.matrixAutoUpdate = false
    laneMesh.updateMatrix()
    parent.add(laneMesh)

    // 渲染常规车道的 link 连接线
    const linkGeo = new BufferGeometry()
    linkGeo.setFromPoints(transformVec3ByObject(lanePoints))
    const linkMesh = new Line(linkGeo, linkMat)
    linkMesh.name = 'laneLink'
    linkMesh.laneId = laneId
    linkMesh.sectionId = sectionId
    linkMesh.roadId = roadId
    linkMesh.renderOrder = RenderOrder.laneLink
    linkMesh.matrixAutoUpdate = false
    linkMesh.updateMatrix()
    linkContainer.add(linkMesh)

    resolve(laneId)
  })
}

/**
 * 销毁隧道的蒙层效果
 * @param params
 */
export function disposeTunnelMask (params: IDisposeRoadParams) {
  const { roadId = '', parent } = params

  // 删除当前 road 匹配的隧道蒙层
  const matchTunnelMask: Array<Mesh> = []
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'tunnelMask' &&
      child.roadId === roadId
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchTunnelMask.push(child)
    }
  })
  parent.remove(...matchTunnelMask)
}

/**
 * 销毁某一条道路中所有创建的实体元素
 * @param params
 * @returns
 */
export function disposeRoad (params: IDisposeRoadParams) {
  const { roadId = '', parent } = params
  // 删除当前 road 匹配的车道网格
  const matchLanes: Array<Mesh> = []
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'lane' &&
      child.roadId === roadId
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchLanes.push(child)
    }
  })
  parent.remove(...matchLanes)

  // 销毁隧道的蒙层
  disposeTunnelMask(params)

  // 删除当前 road 匹配的车道边界线
  const matchBoundary: Array<Mesh> = []
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.name === 'laneBoundary' &&
      child.roadId === roadId
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchBoundary.push(child)
    }
  })
  parent.remove(...matchBoundary)

  // 删除当前 road 匹配的 link 连接线
  let linkContainer: Group | undefined
  const matchLaneLinks: Array<Line> = []
  parent.traverse((child) => {
    if (
      child instanceof Group &&
      child.name === 'roadLinkContainer' &&
      child.roadId === roadId
    ) {
      linkContainer = child
    }
  })

  if (!linkContainer) return
  linkContainer.traverse((child) => {
    if (
      child instanceof Line &&
      child.name === 'laneLink' &&
      child.roadId === roadId
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchLaneLinks.push(child)
    }
  })
  linkContainer.remove(...matchLaneLinks)
  linkContainer.clear()

  parent.remove(linkContainer)
}

/**
 * 销毁所有的道路相关实体元素
 * @param parent
 */
export function disposeAllRoadAndLinks (parent: Group) {
  // 销毁所有的车道实体
  const matchLanes: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'lane') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchLanes.push(child)
    }
  })
  parent.remove(...matchLanes)

  // 删除当前 road 匹配的车道边界线
  const matchBoundary: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'laneBoundary') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchBoundary.push(child)
    }
  })
  parent.remove(...matchBoundary)

  // 获取所有的 linkContainer
  const linkContainers: Array<Group> = []
  parent.traverse((child) => {
    if (child instanceof Group && child.name === 'roadLinkContainer') {
      linkContainers.push(child)
    }
  })

  // 销毁所有的 lane link 线
  linkContainers.forEach((container) => {
    const matchLaneLinks: Array<Line> = []
    container.traverse((child) => {
      if (child instanceof Line && child.name === 'laneLink') {
        child.geometry.dispose()
        disposeMaterial(child)
        matchLaneLinks.push(child)
      }
    })
    container.remove(...matchLaneLinks)
    container.clear()
  })

  parent.remove(...linkContainers)
}
