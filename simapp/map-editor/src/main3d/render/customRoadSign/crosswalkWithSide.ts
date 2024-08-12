import type {
  Object3D,
} from 'three'
import {
  Box3,
  BufferAttribute,
  BufferGeometry,
  DynamicDrawUsage,
  Group,
  Mesh,
  Vector3,
} from 'three'
import {
  type IRenderRoadSign,
  customRoadSignMaterial,
  transparentMaterial,
} from '../roadSign'
import { RenderOrder } from '@/utils/business'
import { PI, axisY } from '@/utils/common3d'

/**
 * 创建分道箭头
 */
function createArrowGeo () {
  /**
   *     /
   *    / \
   *   /   \
   *  /     \
   * /_______\
   *    | |
   *    |_|
   */
  const rectA = { z: 0.15, y: 0, x: 0 }
  const rectB = { z: 0.15, y: 0, x: -0.4 }
  const rectC = { z: -0.15, y: 0, x: -0.4 }
  const rectD = { z: -0.15, y: 0, x: 0 }
  const triA = { z: 0.3, y: 0, x: 0 }
  const triB = { z: -0.3, y: 0, x: 0 }
  const triC = { z: 0, y: 0, x: 0.6 }
  const vertices = []
  const indices = []
  vertices.push(
    rectA.x,
    rectA.y,
    rectA.z,
    rectB.x,
    rectB.y,
    rectB.z,
    rectC.x,
    rectC.y,
    rectC.z,
    rectD.x,
    rectD.y,
    rectD.z,
  )
  indices.push(0, 2, 1)
  indices.push(2, 0, 3)
  vertices.push(
    triA.x,
    triA.y,
    triA.z,
    triB.x,
    triB.y,
    triB.z,
    triC.x,
    triC.y,
    triC.z,
  )
  indices.push(4, 6, 5)

  const _vertices = new Float32Array(vertices)
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  return geo
}

/**
 * 获取人行横道的几何体
 */
function createCrosswalkGeo (params: { length: number, width: number }) {
  const { length, width } = params
  /**
   * 人行横道，每一个实线长度为人行横道的 width 属性
   * 每一个长条白色实线区域，宽度为 40-45cm，间隔为60cm
   */
  const perLength = width // 每一个白线区域的长度
  const perWidth = 0.45 // 每一个白线区域的宽度
  const gap = 0.6 // 间隔

  // 顶点的坐标
  const vertices = []
  // 三角面的索引
  const indices = []
  // 最左和最右边界的角点
  const edgePoints = []
  // 第一个白色实体绘制的偏移距离
  const firstOffset = 0.1 // 10cm
  // 绘制白色实体的偏移量
  let offset = firstOffset
  // 循环的索引
  let basicIndex = 0
  const halfLength = perLength / 2

  // 用 while 代替 for 循环，
  // 如果从偏移处开始，到终点距离有1/3条白线宽度的距离，都可以绘制白色实体区域(只不过最后的白色实体可能会不完整)
  while (offset + perWidth / 3 <= length) {
    // 由于最后的白色区域不一定能够绘制一条完整的，所以最后一条需要动态调整绘制的宽度
    let areaWidth = perWidth
    if (length - offset < perWidth) {
      areaWidth = length - offset
    }
    const tl = {
      x: offset - length / 2,
      y: 0,
      z: halfLength,
    }
    const tr = {
      x: offset + areaWidth - length / 2,
      y: 0,
      z: halfLength,
    }
    const br = {
      x: offset + areaWidth - length / 2,
      y: 0,
      z: -halfLength,
    }
    const bl = {
      x: offset - length / 2,
      y: 0,
      z: -halfLength,
    }
    if (basicIndex === 0) {
      // 第一组实体边界
      edgePoints.push(tl.x, tl.y, tl.z)
      edgePoints.push(bl.x, bl.y, bl.z)
    } else if (offset + (perWidth / 3) * 4 + gap > length) {
      // 最后一组实体边界
      edgePoints.push(br.x, br.y, br.z)
      edgePoints.push(tr.x, tr.y, tr.z)
    }

    // 一组4个角点的坐标
    vertices.push(tl.x, tl.y, tl.z)
    vertices.push(tr.x, tr.y, tr.z)
    vertices.push(br.x, br.y, br.z)
    vertices.push(bl.x, bl.y, bl.z)
    // 索引
    const indexLt = basicIndex * 4 + 0
    const indexLr = basicIndex * 4 + 1
    const indexBr = basicIndex * 4 + 2
    const indexBt = basicIndex * 4 + 3
    // 一组2个三角面的顶点索引顺序
    indices.push(indexLt, indexLr, indexBr)
    indices.push(indexBr, indexBt, indexLt)

    offset += perWidth + gap
    basicIndex++
  }

  // 人行横道实线实体
  // 基于顶点和索引，创建人行横道的geometry
  const _vertices = new Float32Array(vertices)
  const geo = new BufferGeometry()
  const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
    DynamicDrawUsage,
  )
  geo.setIndex(indices)
  geo.setAttribute('position', positionAttr)
  geo.computeVertexNormals()

  return geo
}

/**
 * 创建行人左右分道的人行横道整体元素（未赋值数据状态）
 * @param params
 */
export function createCrosswalkWithSideGroup (params: {
  length: number
  width: number
  group?: Object3D // 是否有透传的容器
  needPlaceholder?: boolean // 是否需要辅助交互的占位网格
}) {
  const {
    length: totalLength,
    width: totalWidth,
    group: originGroup,
    needPlaceholder = false,
  } = params

  // 三角形长度为1米，同时需要保留一定的间隔区域
  const arrowLength = 1 // 三角箭头的长度
  const gap = 0.4 // 两条人行横道线的中间间隔
  const arrowGap = 0.3 // 三角形箭头左右间隔
  const lineLength = totalLength - arrowLength - arrowGap * 2
  const lineWidth = (totalWidth - gap) / 2
  // 创建人行横道的白色实线
  const lineGeo = createCrosswalkGeo({
    length: lineLength,
    width: lineWidth,
  })

  // 创建三角形箭头
  const arrowGeo = createArrowGeo()

  // 白线实体通用的材质
  const mat = customRoadSignMaterial.clone()

  // 外层的容器
  const group = originGroup || new Group()

  // 将模型组装到容器中
  const lineMesh = new Mesh(lineGeo, mat)
  lineMesh.renderOrder = RenderOrder.roadSign
  // 白线区域的偏移量
  const lOffsetX = (totalLength - lineLength) / 2
  const lOffsetZ = (gap + lineWidth) / 2
  lineMesh.position.set(lOffsetX, 0, lOffsetZ)
  lineMesh.matrixAutoUpdate = false
  lineMesh.updateMatrix()
  group.add(lineMesh)

  // 上侧的三角箭头
  const arrowTopMesh = new Mesh(arrowGeo, mat)
  arrowTopMesh.renderOrder = RenderOrder.roadSign
  // 三角箭头的偏移量
  const aOffsetX = -(lineLength - totalLength / 2 + gap + arrowLength / 2)
  const aTopOffsetZ = gap / 2 + (lineWidth / 4) * 3
  arrowTopMesh.position.set(aOffsetX, 0, aTopOffsetZ)
  arrowTopMesh.matrixAutoUpdate = false
  arrowTopMesh.updateMatrix()
  group.add(arrowTopMesh)

  // 下侧的三角箭头
  const arrowBottomMesh = new Mesh(arrowGeo, mat)
  arrowBottomMesh.renderOrder = RenderOrder.roadSign
  const aBottomOffsetZ = gap / 2 + lineWidth / 4
  arrowBottomMesh.position.set(aOffsetX, 0, aBottomOffsetZ)
  arrowBottomMesh.matrixAutoUpdate = false
  arrowBottomMesh.updateMatrix()
  group.add(arrowBottomMesh)

  // 将模型进行克隆，绕中心旋转180°得到另一侧的人行横道
  const _lineMesh = lineMesh.clone()
  _lineMesh.rotateOnAxis(axisY, PI)
  _lineMesh.position.set(-lOffsetX, 0, -lOffsetZ)
  _lineMesh.updateMatrix()
  group.add(_lineMesh)

  const _arrowTopMesh = arrowTopMesh.clone()
  _arrowTopMesh.rotateOnAxis(axisY, PI)
  _arrowTopMesh.position.set(-aOffsetX, 0, -aTopOffsetZ)
  _arrowTopMesh.updateMatrix()
  group.add(_arrowTopMesh)

  const _arrowBottomMesh = arrowBottomMesh.clone()
  _arrowBottomMesh.rotateOnAxis(axisY, PI)
  _arrowBottomMesh.position.set(-aOffsetX, 0, -aBottomOffsetZ)
  _arrowBottomMesh.updateMatrix()
  group.add(_arrowBottomMesh)

  if (needPlaceholder) {
    // 需要创建底层的辅助交互占位区域
    const box3 = new Box3()
    box3.expandByObject(group)
    const { x: maxX, z: maxZ } = box3.max
    const { x: minX, z: minZ } = box3.min
    const edges = []
    edges.push(maxX, 0, maxZ)
    edges.push(maxX, 0, minZ)
    edges.push(minX, 0, minZ)
    edges.push(minX, 0, maxZ)
    const pVertices = new Float32Array(edges)
    const pIndices = [0, 1, 2, 2, 3, 0]
    const pGeo = new BufferGeometry()
    const pPositionAttr = new BufferAttribute(pVertices, 3).setUsage(
      DynamicDrawUsage,
    )
    pGeo.setIndex(pIndices)
    pGeo.setAttribute('position', pPositionAttr)
    pGeo.computeVertexNormals()
    const pMesh = new Mesh(pGeo, transparentMaterial.clone())
    pMesh.isPlaceHolder = true
    pMesh.renderOrder = RenderOrder.roadSignPlaceholder
    pMesh.matrixAutoUpdate = false
    pMesh.updateMatrix()
    group.add(pMesh)
  }

  return group
}

/**
 * 创建行人左右分道的人行横道线
 */
export function createCrosswalkWithSideLine (params: IRenderRoadSign) {
  const { data: roadSignData, parent } = params
  const {
    id: objectId,
    length,
    width,
    roadId,
    position,
    lookAtPoint,
    projectNormal,
  } = roadSignData

  const group = createCrosswalkWithSideGroup({
    length,
    width,
    needPlaceholder: true,
  })
  // 容器赋值数据状态
  group.name = 'roadSignGroup'
  group.objectId = objectId
  group.roadId = roadId

  // 给内部网格赋值数据状态
  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'roadSign'
      child.objectId = objectId
      child.roadId = roadId
    }
  })

  group.position.set(position.x, position.y, position.z)

  if (projectNormal) {
    group.up.set(projectNormal.x, projectNormal.y, projectNormal.z)
  }
  if (lookAtPoint) {
    const _lookAtPoint = new Vector3(
      lookAtPoint.x,
      lookAtPoint.y,
      lookAtPoint.z,
    )
    group.lookAt(_lookAtPoint)
  }

  parent.add(group)

  return {
    length,
    width,
    height: 0,
  }
}
