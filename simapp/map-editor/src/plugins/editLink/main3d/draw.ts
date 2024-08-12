import type {
  Object3D,
  Vector3,
} from 'three'
import {
  CircleGeometry,
  Group,
  Line,
  Mesh,
  MeshBasicMaterial,
} from 'three'
import root3d from '@/main3d/index'
import { getLaneLinkUniqueFlag } from '@/stores/junction/link'
import { disposeMaterial, halfPI } from '@/utils/common3d'
import { RenderOrder } from '@/utils/business'
import { linkColor } from '@/main3d/render/junction'
import { loadTexture } from '@/main3d/loaders'
import { ImgUrls } from '@/utils/urls'

export const junctionLinkHTPointColor = {
  normal: 0xC1B22C,
  hovered: 0xFF9800,
  selected: 0xFA8601,
}

const junctionLinkHTPointGeo = new CircleGeometry(0.6, 16)

/**
 * 控制某一交叉路口所有 link 线的显示和隐藏
 * @param junctionId
 * @param visible
 */
export function setJunctionLinkVisible (junctionId: string, visible: boolean) {
  const parent = root3d.mapElementsContainer
  parent.traverse((child) => {
    if (
      child instanceof Group &&
      child.name === 'junctionLinkContainer' &&
      child.junctionId === junctionId
    ) {
      child.visible = visible
    }
  })
}

/**
 * 控制所有交叉路口的 link 线的显示和隐藏
 * @param visible
 */
export function setAllJunctionLinksVisible (visible: boolean) {
  // 获取渲染交叉路口和道路的容器
  const parent = root3d.mapElementsContainer

  parent.traverse((child) => {
    if (child instanceof Group && child.name === 'junctionLinkContainer') {
      child.visible = visible
    }
  })
}

/**
 * 控制所有道路中的车道 link 线的显示和隐藏
 * @param visible
 */
export function setAllRoadLinksVisible (visible: boolean) {
  // 获取渲染交叉路口和道路的容器
  const parent = root3d.mapElementsContainer

  parent.traverse((child) => {
    if (child instanceof Group && child.name === 'roadLinkContainer') {
      child.visible = visible
    }
  })
}

/**
 * 渲染道路首/末端 link 线的首/尾点( Head / Tail )
 * @param params:{
 *   road: biz.IRoad
 *   percent: '1' | '0'
 *   parent: Group
 * }
 */
export async function renderHTPointOnRoad (params: {
  road: biz.IRoad
  percent: '1' | '0'
  parent: Group
  direction: biz.ILaneDirection // 正反向车道的方向
}) {
  const { road, percent, parent, direction } = params
  const hTPointMap = new Map()
  const sections = road.sections
  const isTail = percent === '1'
  const section = sections[isTail ? sections.length - 1 : 0]
  const { id: sectionId, lanes } = section
  for (const lane of lanes) {
    const { id: laneId, samplePoints: laneSamplePoints } = lane
    // 只渲染匹配的车道顶点
    if (Number(laneId) < 0 && direction === 'reverse') continue
    if (Number(laneId) > 0 && direction === 'forward') continue
    const hTPointFlag = getLaneLinkUniqueFlag(
      road.id,
      sectionId,
      laneId,
      isTail ? 'end' : 'start',
      direction,
    )
    // 定位点
    const pointPosition = isTail ?
      laneSamplePoints[laneSamplePoints.length - 1] :
      laneSamplePoints[0]
    // 计算点的朝向
    const pointDirection = road.keyPath.getTangentAt(isTail ? 1 : 0)
    if (isTail && direction === 'reverse') {
      pointDirection.x = -pointDirection.x
      pointDirection.z = -pointDirection.z
    } else if (!isTail && direction === 'reverse') {
      pointDirection.x = -pointDirection.x
      pointDirection.z = -pointDirection.z
    }
    pointDirection.y = 0
    pointDirection.normalize()

    if (!hTPointMap.get(hTPointFlag)) {
      const hTPoint = await genHTPoint(
        hTPointFlag,
        pointPosition,
        pointDirection,
      )
      hTPointMap.set(hTPointFlag, hTPoint)
      parent.add(hTPoint)
    }
  }
  hTPointMap.clear()
}
/**
 * renderHTPointOnRoad 辅助函数，生成单个点
 * @param  hTPointFlag: string
 * @param  position: Vector3
 * @param  direction: Vector3
 * @return htPoint: Mesh
 */
async function genHTPoint (
  hTPointFlag: string,
  position: Vector3,
  direction: Vector3,
) {
  const texture = await loadTexture(ImgUrls.other.arrow)
  const hTPointMat = new MeshBasicMaterial({
    color: junctionLinkHTPointColor.normal,
    depthTest: false,
    map: texture,
  })
  const group = new Group()
  const hTPoint = new Mesh(junctionLinkHTPointGeo.clone(), hTPointMat)
  hTPoint.name = 'junctionLinkHTPoint'
  hTPoint.flag = hTPointFlag
  hTPoint.rotateX(-halfPI)
  hTPoint.rotateZ(-halfPI)

  hTPoint.renderOrder = RenderOrder.junctionLinkHTPoint
  group.add(hTPoint)
  group.position.set(position.x, position.y, position.z)

  // 计算 htPoint 当前位置沿切向方向稍微向前一点的位置 ，并让包裹 point 的 group 看向它
  const frontPosition = group.position.clone().addScaledVector(direction, 1)
  group.lookAt(frontPosition)
  return group
}
/**
 * 移除所有交叉路口 link 线的首/尾点( Head / Tail )
 * @param parent: Group
 */
export function removeAllHTPoints (parent: Group) {
  const matchedChildren = []
  parent.traverse((child) => {
    if (
      child &&
      child.name === 'junctionLinkHTPoint' &&
      child instanceof Mesh
    ) {
      child.material.dispose()
      disposeMaterial(child)
      matchedChildren.push(child)
    }
  })
  matchedChildren.forEach((child) => {
    // junctionLinkHTPoint 为了适配 lookAt, 有一层 Group 包围 , 因此这里 remove Group
    parent.remove(child.parent)
  })
}
/**
 * 设置交叉路口 link 线的首/尾点( Head / Tail )的颜色
 * @param params:{
 * hTPointFlag: string
 * parent: Object3D
 * color: number
 * }
 */
export function setJunctionLinkHTPointColor (params: {
  hTPointFlag: string
  parent: Object3D
  color: number
}) {
  const { hTPointFlag, parent, color } = params
  const hTPointMesh = parent.getObjectByProperty('flag', hTPointFlag)
  if (!hTPointMesh || !(hTPointMesh instanceof Mesh)) return
  hTPointMesh.material.color.set(color)
  hTPointMesh.material.needsUpdate = true
}
/**
 * 获取交叉路口 link 线的首/尾点( Head / Tail )的颜色
 * @param params: {
 *   hTPointFlag: string
 *   parent: Object3D
 * }
 * @return color: number
 */
export function getJunctionLinkHTPointColor (params: {
  hTPointFlag: string
  parent: Object3D
}) {
  const { hTPointFlag, parent } = params
  const hTPointMesh = parent.getObjectByProperty('flag', hTPointFlag)
  if (!hTPointMesh || !(hTPointMesh instanceof Mesh)) return
  return hTPointMesh.material.color
}
/**
 * 设置交叉路口 link 线的颜色
 * @param params: {
 *   junctionId: string
 *   laneLinkId: string
 *   color: number
 * }
 */
export function setJunctionLinkColor (params: {
  laneLinkId: string
  color: number
}) {
  const parent = root3d.mapElementsContainer
  const { laneLinkId, color } = params
  const laneLink = parent.getObjectByProperty('laneLinkId', laneLinkId)
  if (!laneLink || !(laneLink instanceof Line)) return
  laneLink.material.color.set(color)
  if (color === linkColor.normal) {
    laneLink.renderOrder = RenderOrder.laneLink
  } else {
    laneLink.renderOrder = RenderOrder.highLightLaneLink
  }
}
/**
 * 将交叉路口 link 线的首/尾点( Head / Tail )的颜色设置为 junctionLinkHTPointColor['selected']
 * @param params: {
 *   hTPointFlag: string
 *   parent: Object3D
 * }
 */
export function activeJunctionLinkHTPoint (params: {
  hTPointFlag: string
  parent: Object3D
}) {
  setJunctionLinkHTPointColor({
    ...params,
    color: junctionLinkHTPointColor.selected,
  })
}
/**
 * 重置所有交叉路口 link 线的首/尾点( Head / Tail )的颜色
 * @param parent: Object3D
 */
export function resetAllHTPointColor (parent: Object3D) {
  parent.traverse((child) => {
    if (child instanceof Mesh && child.name === 'junctionLinkHTPoint') {
      child.material.color.set(junctionLinkHTPointColor.normal)
      child.material.needsUpdate = true
    }
  })
}
/**
 * 设置指定交叉路口 link 线的颜色
 * @param laneLinkId: string
 * }
 */
export function setJunctionLinkLinkedColor (laneLinkId: string) {
  setJunctionLinkColor({
    laneLinkId,
    color: linkColor.linked,
  })
}
/**
 * 将指定交叉路口 link 线的颜色设置为 linkColor['selected']
 * @param laneLinkId: string
 */
export function setJunctionLinkSelectedColor (laneLinkId: string) {
  setJunctionLinkColor({
    laneLinkId,
    color: linkColor.selected,
  })
}
/**
 * 重置交叉路口 link 线的颜色
 * @param laneLinkId: string
 */
export function resetJunctionLinkColor (laneLinkId: string) {
  setJunctionLinkColor({ laneLinkId, color: linkColor.normal })
}
