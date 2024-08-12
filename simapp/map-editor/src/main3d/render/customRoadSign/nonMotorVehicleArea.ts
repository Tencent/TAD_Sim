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
  yellowRoadSignColor,
} from '../roadSign'
import { PI, axisY } from '@/utils/common3d'
import { RenderOrder } from '@/utils/business'

/**
 * 创建非机动车禁驶区标线
 * @param params
 */
export function createNonMotorVehicleArea (params: IRenderRoadSign) {
  const { data: roadSignData, parent } = params
  const {
    id: objectId,
    length,
    width,
    junctionId,
    position,
    lookAtPoint,
  } = roadSignData

  // 向外延伸的距离 3 米
  const extendLength = 3
  // 矩形区域的范围
  const rectLength = length - extendLength * 2
  const rectWidth = width - extendLength * 2
  const halfRectLength = rectLength / 2
  const halfRectWidth = rectWidth / 2
  // 每一段黄色实体的长度 1 米
  const perLength = 1
  // 每一段黄色实体的宽度 0.2 米
  const perWidth = 0.2
  const halfPerWidth = perWidth / 2

  // 长度和宽度方向开始循环计算黄色实体角点的偏移量
  let lOffset = halfRectLength + extendLength
  let wOffset = -halfRectWidth - extendLength
  // 长度和宽度方向，顶点和索引的数组
  const lVertices = []
  const lIndices = []
  const wVertices = []
  const wIndices = []

  // 黄色实体的材质
  const mat = customRoadSignMaterial.clone()
  mat.color.set(yellowRoadSignColor.normal)
  mat.needsUpdate = true

  // 底部占位区域的顶点和索引数组
  const pVertices = []
  const pIndices = []
  const pMat = transparentMaterial.clone()
  pMat.color.set(yellowRoadSignColor.normal)
  // 为了便于选取，透明区域宽度的增量比例
  const ratio = 4

  // 只需要计算一次长度方向和宽度方向的定位点数据，通过clone和rotate的形式创建其他的
  // 长度方向的计算
  let lBasicIndex = 0
  let wBasicIndex = 0
  while (lOffset > -halfRectLength) {
    let step = perLength
    // 如果索引是 0，则步长是延长的长度
    if (lBasicIndex === 0) {
      step = extendLength
    } else {
      if (lOffset + halfRectLength < perLength) {
        step = lOffset + halfRectLength
      }
    }
    const p1 = {
      x: -halfRectWidth + halfPerWidth,
      y: 0,
      z: lOffset,
    }
    const p2 = {
      x: -halfRectWidth + halfPerWidth,
      y: 0,
      z: lOffset - step,
    }
    const p3 = {
      x: -halfRectWidth - halfPerWidth,
      y: 0,
      z: lOffset - step,
    }
    const p4 = {
      x: -halfRectWidth - halfPerWidth,
      y: 0,
      z: lOffset,
    }

    lVertices.push(p1.x, p1.y, p1.z)
    lVertices.push(p2.x, p2.y, p2.z)
    lVertices.push(p3.x, p3.y, p3.z)
    lVertices.push(p4.x, p4.y, p4.z)

    const index1 = lBasicIndex * 4 + 0
    const index2 = lBasicIndex * 4 + 1
    const index3 = lBasicIndex * 4 + 2
    const index4 = lBasicIndex * 4 + 3

    lIndices.push(index1, index2, index3)
    lIndices.push(index3, index4, index1)

    if (lBasicIndex === 0) {
      lOffset -= extendLength
      // 第一组顶点
      pVertices.push(
        p1.x + perWidth * ratio,
        p1.y,
        p1.z,
        p4.x - perWidth * ratio,
        p4.y,
        p4.z,
      )
    } else {
      lOffset -= perLength * 2
    }
    lBasicIndex++
  }
  // 手动添加另一侧的顶点
  pVertices.push(
    -halfRectWidth + halfPerWidth + perWidth * ratio,
    0,
    -halfRectLength,
    -halfRectWidth - halfPerWidth - perWidth * ratio,
    0,
    -halfRectLength,
  )
  // 长度方向的占位网格顶点索引
  pIndices.push(0, 2, 3)
  pIndices.push(3, 1, 0)

  // 宽度方向的循环计算
  while (wOffset < halfRectWidth) {
    let step = perLength
    if (wBasicIndex === 0) {
      step = extendLength
    } else {
      if (halfRectWidth - wOffset < perLength) {
        step = halfRectWidth - wOffset
      }
    }
    const p1 = {
      x: wOffset,
      y: 0,
      z: -halfRectLength + halfPerWidth,
    }
    const p2 = {
      x: wOffset + step,
      y: 0,
      z: -halfRectLength + halfPerWidth,
    }
    const p3 = {
      x: wOffset + step,
      y: 0,
      z: -halfRectLength - halfPerWidth,
    }
    const p4 = {
      x: wOffset,
      y: 0,
      z: -halfRectLength - halfPerWidth,
    }

    wVertices.push(p1.x, p1.y, p1.z)
    wVertices.push(p2.x, p2.y, p2.z)
    wVertices.push(p3.x, p3.y, p3.z)
    wVertices.push(p4.x, p4.y, p4.z)

    const index1 = wBasicIndex * 4 + 0
    const index2 = wBasicIndex * 4 + 1
    const index3 = wBasicIndex * 4 + 2
    const index4 = wBasicIndex * 4 + 3

    wIndices.push(index1, index2, index3)
    wIndices.push(index3, index4, index1)

    if (wBasicIndex === 0) {
      wOffset += extendLength
      // 第一组顶点
      pVertices.push(
        p1.x,
        p1.y,
        p1.z + perWidth * ratio,
        p4.x,
        p4.y,
        p4.z - perWidth * ratio,
      )
    } else {
      wOffset += perLength * 2
    }
    wBasicIndex++
  }
  // 手动添加另一侧的顶点
  pVertices.push(
    halfRectWidth,
    0,
    -halfRectLength + halfPerWidth + perWidth * ratio,
    halfRectWidth,
    0,
    -halfRectLength - halfPerWidth - perWidth * ratio,
  )
  // 宽度方向的占位网格顶点索引
  pIndices.push(4, 6, 7)
  pIndices.push(7, 5, 4)

  // 外层的容器
  const group = new Group()
  group.name = 'roadSignGroup'
  group.objectId = objectId
  group.junctionId = junctionId
  // 新增一个自定义属性，用于标识当前容器中元素的自定义颜色
  group.customColor = 'yellow'

  // 透明的占位区域
  const _pVertices = new Float32Array(pVertices)
  const pGeo = new BufferGeometry()
  const pPositionAttr = new BufferAttribute(_pVertices, 3).setUsage(
    DynamicDrawUsage,
  )
  pGeo.setIndex(pIndices)
  pGeo.setAttribute('position', pPositionAttr)
  pGeo.computeVertexNormals()
  const pMesh = new Mesh(pGeo, pMat)
  pMesh.isPlaceHolder = true
  pMesh.renderOrder = RenderOrder.roadSignPlaceholder
  pMesh.matrixAutoUpdate = false
  pMesh.updateMatrix()
  group.add(pMesh)

  const pMeshClone = pMesh.clone()
  pMeshClone.rotateOnAxis(axisY, PI)
  pMeshClone.matrixAutoUpdate = false
  pMeshClone.updateMatrix()
  group.add(pMeshClone)

  // 长度方向的网格
  const _lVertices = new Float32Array(lVertices)
  const lGeo = new BufferGeometry()
  const lPositionAttr = new BufferAttribute(_lVertices, 3).setUsage(
    DynamicDrawUsage,
  )
  lGeo.setIndex(lIndices)
  lGeo.setAttribute('position', lPositionAttr)
  lGeo.computeVertexNormals()

  const lMesh = new Mesh(lGeo, mat)
  lMesh.renderOrder = RenderOrder.roadSign
  lMesh.matrixAutoUpdate = false
  lMesh.updateMatrix()
  group.add(lMesh)

  const lMeshClone = lMesh.clone()
  lMeshClone.rotateOnAxis(axisY, PI)
  lMeshClone.matrixAutoUpdate = false
  lMeshClone.updateMatrix()
  group.add(lMeshClone)

  // 宽度方向的网格
  const _wVertices = new Float32Array(wVertices)
  const wGeo = new BufferGeometry()
  const wPositionAttr = new BufferAttribute(_wVertices, 3).setUsage(
    DynamicDrawUsage,
  )
  wGeo.setIndex(wIndices)
  wGeo.setAttribute('position', wPositionAttr)
  wGeo.computeVertexNormals()

  const wMesh = new Mesh(wGeo, mat)
  wMesh.renderOrder = RenderOrder.roadSign
  wMesh.matrixAutoUpdate = false
  wMesh.updateMatrix()
  group.add(wMesh)

  const wMeshClone = wMesh.clone()
  wMeshClone.rotateOnAxis(axisY, PI)
  wMeshClone.matrixAutoUpdate = false
  wMeshClone.updateMatrix()
  group.add(wMeshClone)

  group.traverse((child) => {
    if (child instanceof Mesh) {
      child.name = 'roadSign'
      child.objectId = objectId
      child.junctionId = junctionId
    }
  })

  group.position.set(position.x, position.y, position.z)
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
