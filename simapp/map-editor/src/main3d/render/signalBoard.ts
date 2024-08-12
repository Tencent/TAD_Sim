import {
  BackSide,
  Box3,
  Color,
  FrontSide,
  MathUtils,
  Mesh,
  MeshBasicMaterial,
} from 'three'
import type {
  Group,
  Object3D,
} from 'three'
import { load3DModel, loadTexture } from '../loaders'
import { getSignalBoardConfig } from '@/config/signalBoard'
import { RenderOrder } from '@/utils/business'
import { axisY, fixedPrecision } from '@/utils/common3d'

interface IRenderSignalBoard {
  data: biz.ISignalBoard
  parent: Group
}

// 标志牌的颜色
const signalBoardColor = {
  normal: 0xE8E8E8,
  hovered: 0xFFFFFF,
  selected: 0xE9E9E9,
}

/**
 * 更新标志牌的颜色
 */
export function updateSignalBoardColor (params: {
  object: Object3D
  status: common.objectStatus
  isForced?: boolean
}) {
  const { object, status, isForced = false } = params
  object.traverse((child) => {
    if (child instanceof Mesh) {
      if (
        isForced ||
        !new Color(signalBoardColor.selected).equals(child.material.color)
      ) {
        child.material.color.set(signalBoardColor[status])
        child.material.needsUpdate = true
      }
    }
  })
}

// 克隆一份标志牌模型的备份，作为标志牌的银色背面网格
function cloneSignalBoardModelBackPart (modelGroup: Group) {
  const mesh = modelGroup.children[0]
  const backMesh = mesh.clone() as Mesh
  const backMat = new MeshBasicMaterial({
    color: signalBoardColor.normal,
    side: BackSide,
  })
  // 更新背面网格的材质
  backMesh.material = backMat
  backMesh.material.needsUpdate = true
  backMesh.name = 'signalBoard'
  backMesh.renderOrder = RenderOrder.signalBoard
  backMesh.objectId = mesh.objectId
  backMesh.poleId = mesh.poleId

  // 调整背面网格的距离
  backMesh.translateX(-0.001)
  modelGroup.add(backMesh)
}

// 渲染信号标志牌
export async function createSignalBoard (params: IRenderSignalBoard) {
  const { data: signalBoardData, parent } = params
  const {
    id: objectId,
    poleId,
    position,
    angle,
    type,
    subtype,
  } = signalBoardData
  const configOption = getSignalBoardConfig(type, subtype)
  if (!configOption) return
  // 从外层容器中，找对应的 pole
  const poleContainer = parent.getObjectByProperty('objectId', poleId)
  if (!poleContainer) return

  const { modelUrl, textureUrl } = configOption
  if (!modelUrl) return
  // 加载当前标志牌对应的模型和纹理文件
  const model = await load3DModel(modelUrl)
  const texture = await loadTexture(textureUrl)
  const mat = new MeshBasicMaterial({
    color: signalBoardColor.normal,
    map: texture,
    side: FrontSide,
  })

  // 对容器和内部的网格赋值
  model.traverse((child) => {
    if (child instanceof Mesh) {
      // 更新对应网格的属性
      child.name = 'signalBoard'
      child.renderOrder = RenderOrder.signalBoard
      child.objectId = objectId
      child.poleId = poleId

      // 用纹理更新对应的标志牌网格材质
      child.material = mat
      child.material.needsUpdate = true
    }
  })
  model.name = 'signalBoardGroup'
  model.objectId = objectId
  model.poleId = poleId

  // 在添加到场景中调整角度之前，通过包围盒计算尺寸
  const box = new Box3()
  box.expandByObject(model)
  const width = fixedPrecision(box.max.z - box.min.z)
  const height = fixedPrecision(box.max.y - box.min.y)
  const length = fixedPrecision(box.max.x - box.min.x)

  // 在标志牌背面clone一份纯色的网格，作为标志牌背面（不能看到纹理）
  cloneSignalBoardModelBackPart(model)

  // 先调整位置
  model.position.set(position.x, position.y, position.z)

  // 将角度制调整成弧度制
  const rad = MathUtils.degToRad(angle)
  // 将标志牌的角度，调整成绕着竖直 Y 轴旋转一定的角度
  model.setRotationFromAxisAngle(axisY, rad)

  poleContainer.add(model)

  // 由于数据层不知道具体的模型尺寸，需要在创建后回传给数据层
  return {
    width,
    height,
    length,
  }
}

// 移动杆上的信号标志牌
export function moveSignalBoard (params: IRenderSignalBoard) {
  const { data: signalBoard, parent } = params
  const { id, position } = signalBoard
  // 找到第一个属性符合的元素，也就是标志牌外层的容器
  const signalBoardGroup = parent.getObjectByProperty('objectId', id)
  if (!signalBoardGroup) return

  // 只需要调整位置
  signalBoardGroup.position.set(position.x, position.y, position.z)
}

// 旋转杆上的信号标志牌，是绕着竖杆调整角度
export function rotateSignalBoard (params: IRenderSignalBoard) {
  const { data: signalBoard, parent } = params
  const { id, position, angle } = signalBoard
  // 找到第一个属性符合的元素，也就是标志牌外层的容器
  const signalBoardGroup = parent.getObjectByProperty('objectId', id)
  if (!signalBoardGroup) return

  // 先调整位置
  signalBoardGroup.position.set(position.x, position.y, position.z)

  // 将角度制调整成弧度制
  const rad = MathUtils.degToRad(angle)
  // 将标志牌的角度，调整成绕着竖直 Y 轴旋转一定的角度
  signalBoardGroup.setRotationFromAxisAngle(axisY, rad)
}
