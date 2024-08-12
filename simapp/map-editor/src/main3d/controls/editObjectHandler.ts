import {
  Raycaster,
  Vector2,
} from 'three'
import type {
  Intersection,
  Object3D,
  OrthographicCamera,
  PerspectiveCamera,
} from 'three'
import { throttle } from 'lodash'
import { sortByRenderOrder } from '@/utils/common3d'
import { useConfig3dStore } from '@/stores/config3d'
import { useObjectStore } from '@/stores/object'
import root3d from '@/main3d/index'
import type { IPoleConfig } from '@/config/pole'
import type { IRoadSignConfig } from '@/config/roadSign'
import type { ISignalBoardConfig } from '@/config/signalBoard'
import type { ITrafficLightConfig } from '@/config/trafficLight'
import type { IOtherConfig } from '@/config/other'
import {
  type IParkingSpaceConfig,
  getParkingSpaceConfig,
} from '@/config/parkingSpace'
import type { ISensorConfig } from '@/config/sensor'
import type { ICustomModelConfig } from '@/stores/object/modelCache'

type IOption =
  | IPoleConfig
  | IRoadSignConfig
  | ISignalBoardConfig
  | ITrafficLightConfig
  | IOtherConfig
  | ISensorConfig

// 将物体列表中的平面元素，拖拽到三维场景中放置，实现新增物体的逻辑
let _camera: PerspectiveCamera | OrthographicCamera
let _domElement: HTMLElement
let _objects: Array<Object3D> = []
const _intersections: Array<Intersection> = []
const _raycaster: Raycaster = new Raycaster()
const _pointer: Vector2 = new Vector2()
let _currentOption: IOption | ICustomModelConfig | null = null
let _imageRatio = 1 // width / height

function updatePointer (event: MouseEvent) {
  const rect = _domElement.getBoundingClientRect()
  _pointer.x = ((event.clientX - rect.left) / rect.width) * 2 - 1
  _pointer.y = (-(event.clientY - rect.top) / rect.height) * 2 + 1

  // 放置位置超出三维场景区域，视为无效
  if (Math.abs(_pointer.x) > 1 || Math.abs(_pointer.y) > 1) return false

  return true
}

// 初始化拖拽事件处理器的局部变量
export function cacheMainState (params: {
  camera: PerspectiveCamera | OrthographicCamera
  domElement: HTMLElement
  objects: Array<Object3D>
}) {
  const { camera, domElement, objects } = params
  _camera = camera
  _domElement = domElement
  _objects = objects
}

// 切换透视和正交相机时，也需要同步更新一个 camera 对象
export function updateEditObjectCamera (
  camera: PerspectiveCamera | OrthographicCamera,
) {
  _camera = camera
}

// 开始拖拽
export function handleDragStart (params: {
  width: number
  height: number
  option: IOption
}) {
  const { width, height, option } = params
  // 根据当前添加的物体类型，初始化一些拖拽前的设置
  _currentOption = option
  _imageRatio = width / height

  root3d.setViewControlsEnabled(false)
}

// 拖拽住图片并移动过程
function _handleDrag (event: PointerEvent) {
  event.preventDefault()
  if (!_currentOption) return

  const pointerRes = updatePointer(event)
  if (!pointerRes) return

  _intersections.length = 0

  _raycaster.setFromCamera(_pointer, _camera)
  _raycaster.intersectObjects(_objects, true, _intersections)
  if (_intersections.length < 1) return

  _intersections.sort(sortByRenderOrder)

  let targetIntersection: Intersection | undefined
  const { placementAreaName } = _currentOption
  _intersections.forEach((i) => {
    if (targetIntersection) return

    if (placementAreaName.includes(i.object.name)) {
      targetIntersection = i
    }
  })

  const config3d = useConfig3dStore()
  if (!targetIntersection) {
    // 如果不可放置，则恢复默认的拖拽鼠标样式
    config3d.updateDragCursor('none')
  } else {
    // 如果可放置，则调整鼠标的样式
    config3d.updateDragCursor('copy')
    return true
  }

  return false
}

// 通过节流函数避免频繁触发
export const handleDrag = throttle(_handleDrag, 10)

// 结束拖拽，即放置
export function handleDragEnd (event: PointerEvent) {
  event.preventDefault()
  if (!_currentOption) {
    resetDragElement()
    return
  }

  const pointerRes = updatePointer(event)
  if (!pointerRes) {
    resetDragElement()
    return
  }

  _intersections.length = 0

  _raycaster.setFromCamera(_pointer, _camera)
  _raycaster.intersectObjects(_objects, true, _intersections)

  if (_intersections.length < 1) {
    resetDragElement()
    return
  }

  // 按照 renderOrder 的层级，对检测到的数组进行排序
  _intersections.sort(sortByRenderOrder)

  let targetIntersection: Intersection | undefined
  const { placementAreaName } = _currentOption
  _intersections.forEach((i) => {
    if (targetIntersection) return

    if (placementAreaName.includes(i.object.name)) {
      targetIntersection = i
    }
  })

  if (targetIntersection) {
    const objectStore = useObjectStore()
    // 如果是自定义导入的模型
    if ((_currentOption as ICustomModelConfig).isCustomModel) {
      objectStore.addCustomModel({
        option: _currentOption as ICustomModelConfig,
        intersection: targetIntersection,
      })
    } else {
      // 根据主要物体类型，从数据层中新增一个物体
      const { mainType } = _currentOption as IOption
      switch (mainType) {
        case 'pole':
          objectStore.addPole({
            option: _currentOption as IPoleConfig,
            intersection: targetIntersection,
          })
          break
        case 'roadSign':
          objectStore.addRoadSign({
            option: {
              ..._currentOption,
              imageRatio: _imageRatio,
            } as IRoadSignConfig,
            intersection: targetIntersection,
          })
          break
        case 'signalBoard':
          objectStore.addSignalBoard({
            option: _currentOption as ISignalBoardConfig,
            intersection: targetIntersection,
          })
          break
        case 'trafficLight':
          objectStore.addTrafficLight({
            option: _currentOption as ITrafficLightConfig,
            intersection: targetIntersection,
          })
          break
        case 'sensor':
          objectStore.addSensor({
            option: _currentOption as ISensorConfig,
            intersection: targetIntersection,
          })
          break
        case 'other':
          objectStore.addOther({
            option: _currentOption as IOtherConfig,
            intersection: targetIntersection,
          })
          break
        case 'parkingSpace': {
          // 具体的停车位配置
          const detailConfig = getParkingSpaceConfig(_currentOption.name)
          objectStore.addParkingSpace({
            option: detailConfig as IParkingSpaceConfig,
            intersection: targetIntersection,
          })
          break
        }
        default:
          break
      }
    }
  }

  resetDragElement()
}

export function resetDragElement () {
  // 重置状态
  const config3d = useConfig3dStore()
  config3d.updateDragCursor('none')
  root3d.setViewControlsEnabled(true)
  _imageRatio = 1
  _currentOption = null
}
