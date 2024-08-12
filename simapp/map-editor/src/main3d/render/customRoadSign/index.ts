import type {
  Object3D,
} from 'three'
import {
  BufferAttribute,
  BufferGeometry,
  DynamicDrawUsage,
  Group,
  Mesh,
} from 'three'
import { customRoadSignMaterial, yellowRoadSignColor } from '../roadSign'
import { RenderOrder } from '@/utils/business'

/**
 * 创建由 geoAttrs 绘制的自定义特殊路面标线
 * @param params
 */
export function createCustomRoadSignByGeoAttrs (params: {
  data: unknown
  parent: Object3D
}) {
  const { parent } = params
  const roadSignData = params.data as biz.ICustomRoadSign

  const {
    id: objectId,
    length,
    width,
    roadId,
    sectionId,
    laneId,
    junctionId = '',
    geoAttrs,
    name,
  } = roadSignData

  const mat = customRoadSignMaterial.clone()

  const group = new Group()
  group.name = 'customRoadSignGroup'
  group.objectId = objectId
  group.roadId = roadId
  group.sectionId = sectionId
  group.laneId = laneId
  group.junctionId = junctionId

  if (name === 'Intersection_Guide_Line') {
    // 路口导向线为黄色材质
    mat.color.set(yellowRoadSignColor.normal)
    mat.needsUpdate = true
    // 新增一个自定义属性，用于标识当前容器中元素的自定义颜色
    group.customColor = 'yellow'
  }

  parent.add(group)
  geoAttrs.forEach((geoAttr) => {
    const { vertices, indices } = geoAttr
    const _vertices =
      vertices instanceof Float32Array ? vertices : new Float32Array(vertices)
    const geo = new BufferGeometry()
    const positionAttr = new BufferAttribute(_vertices, 3).setUsage(
      DynamicDrawUsage,
    )
    geo.setIndex(indices)
    geo.setAttribute('position', positionAttr)
    geo.computeVertexNormals()

    const mesh = new Mesh(geo, mat)
    mesh.name = 'customRoadSign'
    mesh.objectId = objectId
    mesh.roadId = roadId
    mesh.sectionId = sectionId
    mesh.laneId = laneId
    mesh.junctionId = junctionId
    mesh.renderOrder = RenderOrder.roadSign
    mesh.matrixAutoUpdate = false
    mesh.updateMatrix()
    group.add(mesh)
  })

  return {
    length,
    width,
    height: 0,
  }
}
