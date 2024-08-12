// 路口编辑模式下辅助元素的渲染
import type {
  Object3D,
} from 'three'
import {
  BufferAttribute,
  BufferGeometry,
  Color,
  DynamicDrawUsage,
  Group,
  Mesh,
  MeshBasicMaterial,
  PlaneGeometry,
  Vector2,
} from 'three'
import { getRoadEndPoints } from '@/stores/road/road'
import { RenderOrder } from '@/utils/business'
import {
  disposeMaterial,
  getCenterByTwoVec3,
  halfPI,
  sortByAbsId,
  transformVec3,
} from '@/utils/common3d'

interface IRenderEndEdgeParams {
  parent: Object3D
  road: biz.IRoad
  preDispose: boolean
}

interface IDisposeEndEdgeParams {
  parent: Object3D
  roadId: string
}

export const RoadEndEdgeColor = {
  normal: 0x80497F,
  hovered: 0xFF9800,
  selected: 0xB42A1F,
}

const junctionMaskColor = {
  normal: 0xFFFFFF,
  hovered: 0xFFFFFF,
  selected: 0xFFFF00,
}
const junctionMaskMat = new MeshBasicMaterial({
  color: junctionMaskColor.selected,
  depthTest: false,
  transparent: true,
  opacity: 0.3,
})

const edgeGeo = new PlaneGeometry(1, 1)
const edgeMat = new MeshBasicMaterial({
  color: RoadEndEdgeColor.normal,
  depthTest: false,
})

/**
 * 渲染路口的高亮蒙层
 * @param params
 */
export function createHighlightJunctionMask (params: {
  junction: biz.IJunction
  parent: Object3D
}) {
  const { junction, parent } = params
  const { id: junctionId, geoAttr = null } = junction
  if (!geoAttr) return
  const { vertices, indices } = geoAttr
  const geo = new BufferGeometry()
  const _vertices =
    vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
  const _indices =
    indices instanceof Float32Array ?
      Array.prototype.slice.call(indices) :
      indices

  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(_indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  const mesh = new Mesh(geo, junctionMaskMat.clone())
  mesh.name = 'junctionMask'
  mesh.junctionId = junctionId
  mesh.renderOrder = RenderOrder.junctionMask
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  parent.add(mesh)
}

/**
 * 销毁路口的高亮蒙层
 * @param parent
 */
export function disposeHighlightJunctionMask (parent: Object3D) {
  const matchChildren: Array<Mesh> = []
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'junctionMask') {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child)
    }
  })
  parent.remove(...matchChildren)
}

/**
 * 高亮选中的道路端面辅助元素颜色
 * @param params
 * @returns
 */
export function updateRoadEndEdgeColor (params: {
  parent: Object3D
  linkRoad: string | Array<string> // 对应端面网格的 uuid
  status: common.objectStatus
}) {
  const { parent, status } = params
  let { linkRoad } = params
  if (!Array.isArray(linkRoad)) {
    linkRoad = [linkRoad]
  }
  linkRoad.forEach((_linkRoad) => {
    // 正反向车道需要判断 direction
    const [roadId, percent, direction] = _linkRoad.split('_')
    parent.traverse((child) => {
      if (
        child instanceof Mesh &&
        child.name === 'roadEndEdge' &&
        child.roadId === roadId &&
        child.percent === percent &&
        child.direction === direction
      ) {
        child.material.color.set(RoadEndEdgeColor[status])
        child.material.needsUpdate = true
      }
    })
  })
}

/**
 * 重置所有道路端面辅助元素的颜色
 * @param parent
 */
export function resetAllRoadEndEdgeColor (parent: Object3D) {
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'roadEndEdge') {
      const { material } = child
      if (!new Color(RoadEndEdgeColor.normal).equals(material.color)) {
        material.color.set(RoadEndEdgeColor.normal)
        material.needsUpdate = true
      }
    }
  })
}

// 渲染辅助的道路端点边缘辅助元素
// 使用一层 group 包裹实体的 mesh，当要跟随视角调整缩放，调整 group 沿道路方向的维度
export function renderRoadEndEdge (params: IRenderEndEdgeParams) {
  const { parent, road, preDispose } = params
  const { id: roadId, sections } = road
  if (preDispose) {
    disposeRoadEndEdge({
      parent,
      roadId,
    })
  }
  // 先判断当前道路是否有正反向车道
  const forwardLanes = sections[0].lanes
    .filter(lane => Number(lane.id) < 0)
    .sort(sortByAbsId)
  const reverseLanes = sections[0].lanes
    .filter(lane => Number(lane.id) > 0)
    .sort(sortByAbsId)

  // 正向车道
  if (forwardLanes.length > 0) {
    // 0 表示车道起始位置，1 表示车道结束位置
    for (let i = 0; i <= 1; i++) {
      const isTail = i !== 0
      const { left: leftPoint, right: rightPoint } = getRoadEndPoints({
        road,
        isTail,
        direction: 'forward', // 正向车道
      })

      if (!leftPoint || !rightPoint) continue

      // 将采样点转换成三维顶点
      const leftPointVec3 = transformVec3(leftPoint)
      const rightPointVec3 = transformVec3(rightPoint)
      // 两个点的距离表示当前道路在端点处的宽度
      const roadWidth = leftPointVec3.distanceTo(rightPointVec3)
      // 道路在端点处的中点
      const centerPoint = getCenterByTwoVec3({
        v1: leftPointVec3,
        v2: rightPointVec3,
        fixed: true,
      })

      // 获取当前 section 最内侧和最外侧车道边界线，基于对应的首/尾顶点计算辅助元素的尺寸和位置
      // 由于存在偏某个维度的缩放，需要用一个 group 包裹实体辅助元素
      const edgeGroup = new Group()
      const scaleY = 1
      const edge = new Mesh(edgeGeo.clone(), edgeMat.clone())
      edge.scale.set(roadWidth, 1, 1)
      edge.translateZ(i === 0 ? -scaleY / 2 : scaleY / 2)
      edge.rotateX(-halfPI)
      edge.name = 'roadEndEdge'
      edge.roadId = roadId
      // 当前辅助元素归属于道路的比例，0 为起始点，1 为结束点
      edge.percent = String(i)
      // 正向车道
      edge.direction = 'forward'
      edge.renderOrder = RenderOrder.roadEndEdge
      edgeGroup.add(edge)

      // 外层 group 不参与实际的射线交互，可以不挂载标识的属性
      edgeGroup.name = 'endEdgeGroup'
      // 外层 group 调整角度跟车道切线方向平行
      const vec2 = new Vector2(
        rightPoint.x - leftPoint.x,
        rightPoint.z - leftPoint.z,
      )
      edgeGroup.rotateY(-vec2.angle())
      edgeGroup.position.set(centerPoint.x, centerPoint.y, centerPoint.z)
      // 将辅助元素上移一小段距离，避免跟车道元素顶点完全重合
      edgeGroup.translateY(0.02)
      parent.add(edgeGroup)
    }
  }

  // 反向车道
  if (reverseLanes.length > 0) {
    // 正向车道，0 表示车道起始位置，1 表示车道结束位置
    // 由于反向车道的边界线采样顺序跟正向车道的一致，所以对于反向车道
    // 当遇到 0 时，视为车道结束的方向，1 是车道起始的方向
    for (let i = 0; i <= 1; i++) {
      const isTail = i === 0
      const { left: leftPoint, right: rightPoint } = getRoadEndPoints({
        road,
        isTail: !isTail,
        direction: 'reverse', // 反向车道
      })

      if (!leftPoint || !rightPoint) continue

      // 将采样点转换成三维顶点
      const leftPointVec3 = transformVec3(leftPoint)
      const rightPointVec3 = transformVec3(rightPoint)
      // 两个点的距离表示当前道路在端点处的宽度
      const roadWidth = leftPointVec3.distanceTo(rightPointVec3)
      // 道路在端点处的中点
      const centerPoint = getCenterByTwoVec3({
        v1: leftPointVec3,
        v2: rightPointVec3,
        fixed: true,
      })

      // 获取当前 section 最内侧和最外侧车道边界线，基于对应的首/尾顶点计算辅助元素的尺寸和位置
      // 由于存在偏某个维度的缩放，需要用一个 group 包裹实体辅助元素
      const edgeGroup = new Group()
      const scaleY = 1
      const edge = new Mesh(edgeGeo.clone(), edgeMat.clone())
      edge.scale.set(roadWidth, 1, 1)
      edge.translateZ(i === 0 ? -scaleY / 2 : scaleY / 2)
      edge.rotateX(-halfPI)
      edge.name = 'roadEndEdge'
      edge.roadId = roadId
      // 当前辅助元素归属于道路的比例，0 为起始点，1 为结束点
      edge.percent = String(i)
      // 反向车道
      edge.direction = 'reverse'
      edge.renderOrder = RenderOrder.roadEndEdge
      edgeGroup.add(edge)

      // 外层 group 不参与实际的射线交互，可以不挂载标识的属性
      edgeGroup.name = 'endEdgeGroup'
      // 外层 group 调整角度跟车道切线方向平行
      const vec2 = new Vector2(
        rightPoint.x - leftPoint.x,
        rightPoint.z - leftPoint.z,
      )
      edgeGroup.rotateY(-vec2.angle())
      edgeGroup.position.set(centerPoint.x, centerPoint.y, centerPoint.z)
      // 将辅助元素上移一小段距离，避免跟车道元素顶点完全重合
      edgeGroup.translateY(0.02)
      parent.add(edgeGroup)
    }
  }
}

// 销毁道路端点边缘的辅助元素
export function disposeRoadEndEdge (params: IDisposeEndEdgeParams) {
  const { parent, roadId } = params
  const matchChildren: Array<Group> = []
  parent.traverse((child) => {
    if (
      child instanceof Mesh &&
      child.roadId === roadId &&
      child.name === 'roadEndEdge'
    ) {
      child.geometry.dispose()
      disposeMaterial(child)
      matchChildren.push(child.parent as Group)
    }
  })
  parent.remove(...matchChildren)
  matchChildren.forEach((group) => {
    group.clear()
  })
}

// 销毁所有的道路端点边缘的辅助元素
export function disposeAllRoadEndEdge (parent: Group) {
  const containers: Array<Group> = []
  parent.traverse((child) => {
    if (child instanceof Group && child.name === 'endEdgeGroup') {
      containers.push(child)
    }
  })

  containers.forEach((container) => {
    const matchChildren: Array<Mesh> = []
    container.traverse((child) => {
      if (child instanceof Mesh && child.name === 'roadEndEdge') {
        child.geometry.dispose()
        disposeMaterial(child)
        matchChildren.push(child)
      }
    })
    container.remove(...matchChildren)
    container.clear()
  })

  parent.remove(...containers)
}
