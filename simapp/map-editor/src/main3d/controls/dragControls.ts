import {
  EventDispatcher,
  MOUSE,
  Plane,
  Raycaster,
  Vector2,
  Vector3,
} from 'three'
import type {
  Group,
  Intersection,
  Object3D,
  OrthographicCamera,
  PerspectiveCamera,
} from 'three'
import { usePluginStore } from '@/stores/plugin'
import { calcHypotenuse, sortByRenderOrder } from '@/utils/common3d'

// 交互的全局临时变量
let _selected: Intersection | null = null
// 用户预览效果的辅助拖拽临时容器
const _plane: Plane = new Plane()
const _raycaster = new Raycaster()

// 自定义派发的交互事件类型（事件用小写，不用驼峰）
export type IntersectionType =
  | 'dragstart'
  | 'dragend'
  | 'drag'
  | 'click'
  | 'hoveron'
  | 'hoveroff'

// selected 元素
export function setSelected (i: Intersection | null) {
  _selected = i
}
export function getSelected () {
  return _selected
}
// plane 元素
export function setRefPlaneByNormalAndPoint (normal: Vector3, point: Vector3) {
  _plane.setFromNormalAndCoplanarPoint(normal, point)
}

export function getRefPlane () {
  return _plane
}

// raycaster 元素
export function getRaycaster () {
  return _raycaster
}

/**
 * 调整拖拽交互控制器中射线检查对 Line 元素的阈值
 * @param value
 */
export function updateRaycasterLineThreshold (value: number) {
  // 设置射线检测对 Line 元素的检查范围阈值
  if (_raycaster.params.Line?.threshold) {
    _raycaster.params.Line.threshold = value
  }
}

// 缓存可 hover 元素的名称
// TODO 减少使用 any 类型
const hoverElements: any = {}
export function registryHoverElement (params: {
  pluginName: string
  objectName: string | Array<string>
}) {
  const { pluginName, objectName } = params
  hoverElements[pluginName] = []
  if (typeof objectName === 'string') {
    hoverElements[pluginName].push(objectName)
    return
  }

  hoverElements[pluginName].push(...objectName)
}

// 缓存拖拽位置忽略偏移量计算的元素（适用于自定义 geometry 顶点的元素）
const ignoreDragOffsetElements: Array<string> = []
export function registryIgnoreDragOffsetElements (
  elementName: string | Array<string>,
) {
  if (typeof elementName === 'string') {
    ignoreDragOffsetElements.push(elementName)
  } else {
    ignoreDragOffsetElements.push(...elementName)
  }
}

// 有外层 Group 容器的网格 name 集合
const hasPoleGroupElements: Array<string> = []
export function registryHasPoleGroupElements (
  elementName: string | Array<string>,
) {
  if (typeof elementName === 'string') {
    hasPoleGroupElements.push(elementName)
  } else {
    hasPoleGroupElements.push(...elementName)
  }
}

interface IEventDispatchers {
  pointermove: (this: HTMLElement, ev: PointerEvent) => void
  pointerdown: (this: HTMLElement, ev: PointerEvent) => void
  pointerup: (this: HTMLElement, ev: PointerEvent) => void
}

interface IParams {
  objects: Array<Object3D>
  camera: PerspectiveCamera | OrthographicCamera
  domElement: HTMLElement
  startAnimationFrame: Function
  endAnimationFrame: Function
}

const _pointer = new Vector2()
const _offset = new Vector3()
const _intersection = new Vector3()
const _worldPosition = new Vector3()

/**
 * 通过一个法向量和一个三维坐标，确定移动物体过程中的基准平面
 * @param normal
 * @param point
 */
export function setInteractionPlane (params: {
  normal: Vector3
  point?: Vector3
}) {
  if (!_selected) return null
  const defaultPoint = _worldPosition.setFromMatrixPosition(
    _selected.object.matrixWorld,
  )
  const { normal, point = defaultPoint } = params
  _plane.setFromNormalAndCoplanarPoint(normal, point)
}

/**
 * 拖拽控制器自身不调用 render 函数，通过订阅函数，谁订阅谁触发 render
 */
class DragControls extends EventDispatcher {
  enabled: boolean
  dispatchers: IEventDispatchers
  camera: PerspectiveCamera | OrthographicCamera
  private _domElement: HTMLElement
  private _intersections: Array<Intersection>
  constructor (params: IParams) {
    super()
    const { objects: _objects, domElement: _domElement, camera } = params

    let pointerId = -1
    // 鼠标按下时对应的按键
    let pointerdownButton = -1
    let dragStatus = ''
    let _hovered: Intersection | null = null
    let newPosition: Vector3 | null = null

    this.camera = camera
    this._domElement = _domElement
    this._intersections = []
    this.enabled = true

    let mouseDownX = 0
    let mouseDownY = 0
    let mouseMoveX = 0
    let mouseMoveY = 0

    this.dispatchers = {
      pointermove: (event: PointerEvent) => {
        if (this.enabled === false) return

        this.updatePointer(event)

        mouseMoveX = event.clientX
        mouseMoveY = event.clientY

        // 如果存在选中的物体，但按下时的鼠标按键不是左键，则视为无效
        if (_selected) {
          // 只要有选中的物体，在拖拽时都需要更新状态
          if (dragStatus === 'dragStart') {
            dragStatus = 'dragging'
          }

          // 只响应鼠标左键的拖拽
          if (pointerdownButton === MOUSE.LEFT) {
            _raycaster.setFromCamera(_pointer, this.camera)
            if (_raycaster.ray.intersectPlane(_plane, _intersection)) {
              if (ignoreDragOffsetElements.includes(_selected.object.name)) {
                // 车道网格自身不设置 position 数据，而是通过几何体顶点坐标调整结构
                // 在选中车道拖拽整条道路的过程中，只需要鼠标的射线跟地平面交互的点位置即可
                newPosition = _intersection
              } else {
                newPosition = _intersection.sub(_offset)
              }
              this.dispatchEvent({
                type: 'drag',
                object: {
                  newPosition,
                  originEvent: event,
                },
              })
            }
          }

          return
        }

        // 每次检测之前都需要重置交互到的数组
        this._intersections.length = 0

        // 基于鼠标投影的位置射线做射线检测
        _raycaster.setFromCamera(_pointer, this.camera)
        _raycaster.intersectObjects(_objects, true, this._intersections)

        if (this._intersections.length < 1) return

        // 对检测到的数据按照 renderOrder 做排序
        this._intersections.sort(sortByRenderOrder)

        const pluginStore = usePluginStore()
        if (pluginStore && pluginStore.currentPlugin) {
          // 判断当前编辑模式下，是否有可 hover 的元素
          let lastName
          let lastUuid
          if (_hovered) {
            // 如果之前 hover 的元素还存在
            ;({ name: lastName, uuid: lastUuid } = _hovered.object)
          }

          let currentHovered: Intersection | null = null
          this._intersections.forEach((intersection: Intersection) => {
            if (currentHovered) return
            // 如果新检测到的交互数据中，存在当前模式下可以 hover 的元素，则记录下来
            if (
              hoverElements[pluginStore.currentPlugin] &&
              hoverElements[pluginStore.currentPlugin].includes(
                intersection.object.name,
              )
            ) {
              currentHovered = intersection
            }
          })

          if (!lastName && !lastUuid) {
            if (currentHovered) {
              // 如果没有上一次 hover 的元素，但有了新的 hover 元素
              this.dispatchEvent({
                type: 'hoveron',
                object: {
                  elementName: (currentHovered as Intersection).object.name,
                  elementUuid: (currentHovered as Intersection).object.uuid,
                  originEvent: event,
                },
              })
              _hovered = currentHovered
            }
          } else {
            if (currentHovered) {
              if (
                lastName !== (currentHovered as Intersection).object.name ||
                lastUuid !== (currentHovered as Intersection).object.uuid
              ) {
                // 上一次 hover 元素，跟当前新的 hover 元素不是同一个
                this.dispatchEvent({
                  type: 'hoveroff',
                  object: {
                    elementName: lastName,
                    elementUuid: lastUuid,
                    originEvent: event,
                  },
                })

                _hovered = currentHovered
                this.dispatchEvent({
                  type: 'hoveron',
                  object: {
                    elementName: (currentHovered as Intersection).object.name,
                    elementUuid: (currentHovered as Intersection).object.uuid,
                    originEvent: event,
                  },
                })
              }
            } else {
              // 如果有上一次 hover 的元素，但没有新的 hover 元素
              this.dispatchEvent({
                type: 'hoveroff',
                object: {
                  elementName: lastName,
                  elementUuid: lastUuid,
                  originEvent: event,
                },
              })
              _hovered = null
            }
          }
        }
      },
      pointerdown: (event: PointerEvent) => {
        event.preventDefault()
        if (this.enabled === false) return
        // 目前只接受鼠标左键和右键的响应
        if (event.button !== MOUSE.LEFT && event.button !== MOUSE.RIGHT) {
          return
        }
        mouseDownX = event.clientX
        mouseDownY = event.clientY

        // 将当前交互的 dom 元素指定为未来指针事件的捕获目标
        pointerId = event.pointerId
        this._domElement.setPointerCapture(pointerId)

        // 记录当前鼠标按下的具体按键类型
        pointerdownButton = event.button

        // 记录当前事件的状态
        dragStatus = 'dragStart'

        // 将屏幕坐标转换成相机的投影平面坐标
        this.updatePointer(event)

        // 清空射线交互到的元素数组
        this._intersections.length = 0

        // 射线检测
        _raycaster.setFromCamera(_pointer, this.camera)
        _raycaster.intersectObjects(_objects, true, this._intersections)

        // 如果没有检测到任何元素，则返回
        if (this._intersections.length < 1) return

        // 由于在渲染顺序上使用了 renderOrder，射线检测到的结果不一定是真是的深度顺序
        // 需要对检测到的数组基于 renderOrder 做有大到小的排序
        this._intersections.sort(sortByRenderOrder)

        // 在检测到场景元素的前提下，将 dragstart 事件进行派发，让定制化插件中订阅的逻辑来做后续处理
        this.dispatchEvent({
          type: 'dragstart',
          object: {
            originEvent: event,
            intersections: this._intersections,
            pointerdownButton,
          },
        })

        // 在确保有选中的物体，并且是鼠标左键操作时，记录下当前交互元素在基准平面上的相交位置
        if (_selected && pointerdownButton === MOUSE.LEFT) {
          if (_raycaster.ray.intersectPlane(_plane, _intersection)) {
            // 计算新的拖拽后位置前先重置
            newPosition = null

            let _objectLocation: Vector3
            // 需要判断当前拖拽的网格，是否有外层杆容器
            if (hasPoleGroupElements.includes(_selected.object.name)) {
              // 如果有外层容器，需要基于外层容器的位置来计算偏移量
              _objectLocation = _worldPosition.setFromMatrixPosition(
                (_selected.object.parent as Group).matrixWorld,
              )
            } else {
              if (_selected.object.isBridge) {
                // 如果是天桥模型，需要使用外层的容器来计算位置偏移
                _objectLocation = _worldPosition.setFromMatrixPosition(
                  _selected.object.parent?.matrixWorld,
                )
              } else {
                // 如果没有外层容器，则使用自身的位置计算偏移量
                _objectLocation = _worldPosition.setFromMatrixPosition(
                  _selected.object.matrixWorld,
                )
              }
            }

            _offset.copy(_intersection).sub(_objectLocation)
          }
        }
      },
      pointerup: (event: PointerEvent) => {
        event.preventDefault()
        if (this.enabled === false) return
        if (event.button !== MOUSE.LEFT && event.button !== MOUSE.RIGHT) {
          return
        }
        if (pointerdownButton !== -1 && event.button !== pointerdownButton) {
          return
        }
        try {
          this._domElement.releasePointerCapture(pointerId)
        } catch (err) {}

        if (dragStatus === 'dragStart') {
          // 如果只是经过了 down 事件，而没有 move，则视为点击 click 事件

          this.dispatchEvent({
            type: 'click',
            object: {
              originEvent: event,
              pointerdownButton,
            },
          })
        } else if (dragStatus === 'dragging') {
          const deltaX = mouseMoveX - mouseDownX
          const deltaY = mouseMoveY - mouseDownY
          const offset = calcHypotenuse(deltaX, deltaY)
          if (offset <= 1) {
            // 如果 down 以后，由于鼠标设备原因存在微小的误差引入的判定错误，做兼容处理
            // 在一定范围内的偏移，也可视为 click
            this.dispatchEvent({
              type: 'click',
              object: {
                originEvent: event,
                pointerdownButton,
              },
            })
          } else {
            // 在 down + move 事件后，则视为拖拽结束 dragend 事件
            this.dispatchEvent({
              type: 'dragend',
              object: {
                originEvent: event,
                newPosition,
              },
            })
          }
        }

        // 重置状态
        pointerdownButton = -1
        dragStatus = ''
        newPosition = null
        mouseDownX = 0
        mouseDownY = 0
        mouseMoveX = 0
        mouseMoveY = 0
        // 如果在鼠标操作"down+move+up"的周期中，存在选中的元素，则在 up 的时候清空
        if (_selected) {
          setSelected(null)
        }
      },
    }

    this.activate()
  }

  activate () {
    this._domElement.addEventListener(
      'pointermove',
      this.dispatchers.pointermove,
    )
    this._domElement.addEventListener(
      'pointerdown',
      this.dispatchers.pointerdown,
    )
    this._domElement.addEventListener(
      'pointerup',
      this.dispatchers.pointerup,
    )
  }

  deactivate () {
    this._domElement.removeEventListener(
      'pointermove',
      this.dispatchers.pointermove,
    )
    this._domElement.removeEventListener(
      'pointerdown',
      this.dispatchers.pointerdown,
    )
    this._domElement.removeEventListener(
      'pointerup',
      this.dispatchers.pointerup,
    )
  }

  dispose () {
    this.deactivate()
  }

  updateCamera (camera: PerspectiveCamera | OrthographicCamera) {
    this.camera = camera
  }

  /**
   * 更新文档流中鼠标坐标在三维投影平面上的坐标
   * @param event
   */
  private updatePointer (event: PointerEvent) {
    const rect = this._domElement.getBoundingClientRect()
    _pointer.x = ((event.clientX - rect.left) / rect.width) * 2 - 1
    _pointer.y = (-(event.clientY - rect.top) / rect.height) * 2 + 1
  }
}

export default DragControls
