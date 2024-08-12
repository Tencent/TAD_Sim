import {
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

/**
 * 获取人行横道的几何体
 */
export function getCrosswalkGeo (params: { length: number, width: number }) {
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

  // 基于第一组边界角点和最后一组边界角点，创建透明的占位区域，用于辅助选中
  const pVertices = new Float32Array(edgePoints)
  const pIndices = [0, 2, 1, 2, 0, 3]
  const pGeo = new BufferGeometry()
  const pPositionAttr = new BufferAttribute(pVertices, 3).setUsage(
    DynamicDrawUsage,
  )
  pGeo.setIndex(pIndices)
  pGeo.setAttribute('position', pPositionAttr)
  pGeo.computeVertexNormals()

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

  return {
    placeholder: pGeo,
    main: geo,
  }
}

/**
 * 创建人行横道
 */
export function createCrosswalkLine (params: IRenderRoadSign) {
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

  // 获取人行横道实体和辅助放置元素的几何体
  const geometries = getCrosswalkGeo({
    length,
    width,
  })
  const { main: mainGeo, placeholder: pGeo } = geometries

  // 外层的容器
  const group = new Group()
  group.name = 'roadSignGroup'
  group.objectId = objectId
  group.roadId = roadId

  // 透明的辅助交互的占位网格
  const pMesh = new Mesh(pGeo, transparentMaterial.clone())
  pMesh.name = 'roadSign'
  pMesh.isPlaceHolder = true
  pMesh.objectId = objectId
  pMesh.roadId = roadId
  pMesh.renderOrder = RenderOrder.roadSignPlaceholder
  pMesh.matrixAutoUpdate = false
  pMesh.updateMatrix()
  group.add(pMesh)

  // 人行横道白线的实体网格
  const mesh = new Mesh(mainGeo, customRoadSignMaterial.clone())
  mesh.name = 'roadSign'
  mesh.objectId = objectId
  mesh.roadId = roadId
  mesh.renderOrder = RenderOrder.roadSign
  mesh.matrixAutoUpdate = false
  mesh.updateMatrix()
  group.add(mesh)

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
