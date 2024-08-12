// 基于 threejs dragControls 的代码，自定义拖动控制类
import _ from 'lodash'
import {
  EventDispatcher,
  Plane,
  Raycaster,
  Vector2,
  Vector3,
} from 'three'

/**
 * 计算两点之间的平方距离
 * @param {number} x1 - 第一个点的 x 坐标
 * @param {number} y1 - 第一个点的 y 坐标
 * @param {number} x2 - 第二个点的 x 坐标
 * @param {number} y2 - 第二个点的 y 坐标
 * @return {number} 两点之间的平方距离
 */
function distanceSquared (x1, y1, x2, y2) {
  return (x1 - x2) ** 2 + (y1 - y2) ** 2
}

/**
 * 拖动控制类，用于实现对象的拖动功能
 * @extends {EventDispatcher} 基于 es5 实现的继承，继承代码在最后
 */
const DragControls = function (_objects, _camera, _domElement) {
  const _plane = new Plane((new Vector3(0, 0, 0.1)).normalize(), 0) // 用于计算射线与平面的交点，可以用于计算鼠标在 3D 空间中的位置
  const _raycaster = new Raycaster()
  this.raycaster = _raycaster
  _raycaster.params.Line.threshold = 1 // 设置射线检测的精度
  this._camera = _camera
  this.dragButton = null

  const startOffset = {
    offsetX: 0,
    offsetY: 0,
  }
  let startPosition = new Vector3()
  let dragStatus = 'notDragging' // notDragging | pre | dragging
  let _dragButton = null // 用于记录鼠标按下时的按键

  const _mouse = new Vector2() // 用于存储鼠标的坐标
  const _intersection = new Vector3() // 用于存储射线与 _plane 相交的坐标
  const hoverOnEvent = {
    type: 'hoveron',
  }
  const hoverOffEvent = {
    type: 'hoveroff',
  }
  let hoverTarget = null
  let pointerId = -1

  let _selected = null

  // mousemove 事件
  const onDocumentMouseMove = (event) => {
    event.preventDefault()

    const { offsetX, offsetY } = event

    const { offsetX: originX, offsetY: originY } = startOffset

    const {
      left,
      top,
      width,
      height,
    } = _domElement.getBoundingClientRect()
    _mouse.x = ((event.clientX - left) / width) * 2 - 1
    _mouse.y = -((event.clientY - top) / height) * 2 + 1

    _raycaster.setFromCamera(_mouse, this._camera)
    const intersects = _raycaster.intersectObjects(_objects, true)
    resort(intersects)

    const position = new Vector3()
    if (_raycaster.ray.intersectPlane(_plane, _intersection)) {
      position.copy(_intersection).setZ(0)
    }

    // 当“偏移距离”大于 64 才被认为是在拖拽
    if (dragStatus === 'pre' && distanceSquared(offsetX, offsetY, originX, originY) > 64) {
      dragStatus = 'dragging'
      // 基于 EventDispatcher 的事件派发函数进行包装的方法
      dispatchEvent(intersects, {
        originalEvent: event,
        type: 'dragstart',
        dragButton: _dragButton,
        intersect: _selected,
        target: _.get(intersects, '0.object', this),
        position: startPosition.clone(),
        ray: _raycaster.ray,
      })
    }

    if (dragStatus === 'dragging') {
      dispatchEvent(intersects, {
        originalEvent: event,
        dragButton: _dragButton,
        type: 'drag',
        intersect: _selected,
        target: _.get(intersects, '0.object', this),
        position,
        ray: _raycaster.ray,
      })
    }

    if (_selected && dragStatus === 'pre') {
      event.stopImmediatePropagation()
    }

    if (intersects[0]) {
      // 记录当前 hover 的对象
      const [{ object }] = intersects
      if (hoverTarget !== object) {
        hoverTarget && hoverTarget.dispatchEvent({ ...hoverOffEvent, dragStatus })
        object.dispatchEvent({ ...hoverOnEvent, ...intersects[0], dragStatus })
        hoverTarget = object
      }
    } else if (hoverTarget) {
      // 如果 hover 的对象发生变化，则派发 hoveroff 事件
      hoverTarget.dispatchEvent({ ...hoverOffEvent })
      hoverTarget = null
    }

    dispatchEvent(intersects, {
      originalEvent: event,
      type: 'mousemove',
      intersect: intersects[0],
      target: _.get(intersects, '0.object', this),
      position,
      ray: _raycaster.ray,
    })
  }

  // mouseDown 事件
  const onDocumentMouseDown = (event) => {
    ({ pointerId } = event)
    const { offsetX, offsetY, button } = event
    if (this.dragButton !== null && button !== this.dragButton) return
    event.preventDefault()
    _domElement.setPointerCapture(pointerId)
    _dragButton = button
    const intersects = _raycaster.intersectObjects(_objects, true)
    resort(intersects)
    const position = new Vector3()
    if (_raycaster.ray.intersectPlane(_plane, _intersection)) {
      // 如果射线与平面相交，则将相交点作为鼠标位置。该 position 记录了鼠标在 3D 空间中的位置。
      event.stopImmediatePropagation()
      position.copy(_intersection).setZ(0)
    }

    if (intersects[0]) {
      // 记录鼠标按下时射线检测到的对象
      [_selected] = intersects
    } else {
      _domElement.releasePointerCapture(pointerId)
      return
    }

    // 改变 drag 状态
    dragStatus = 'pre'
    startPosition = position.clone()

    // 记录鼠标按下时候的鼠标位置
    startOffset.offsetX = offsetX
    startOffset.offsetY = offsetY

    // 派发 mousemove 事件
    dispatchEvent(intersects, {
      originalEvent: event,
      type: 'mousedown',
      intersect: intersects[0],
      target: _.get(intersects, '0.object', this),
      position,
      ray: _raycaster.ray,
    })
  }

  const onDocumentMouseUp = (event) => {
    event.preventDefault()
    if (Object.prototype.hasOwnProperty.call(_domElement, pointerId)) {
      // 确保鼠标抬起时，事放与 pointerId 相关的事件处理器
      _domElement.releasePointerCapture(pointerId)
      pointerId = -1
    }
    _dragButton = null
    const position = new Vector3()

    const intersects = _raycaster.intersectObjects(_objects, true)
    resort(intersects)
    if (_raycaster.ray.intersectPlane(_plane, _intersection)) {
      position.copy(_intersection).setZ(0)
    }
    dispatchEvent(intersects, {
      originalEvent: event,
      type: 'mouseup',
      intersect: intersects[0],
      target: _.get(intersects, '0.object', this),
      position,
      ray: _raycaster.ray,
    })

    let evt
    if (dragStatus === 'pre') {
      // 如果 mouseup 前是 mousedown 的状态，则触发 click 事件
      evt = {
        originalEvent: event,
        type: 'click',
        intersect: _selected,
        target: _.get(intersects, '0.object', this),
        position,
        ray: _raycaster.ray,
      }
    } else if (dragStatus === 'dragging') {
      // 如果 mouseup 前是 dragging 的状态，则触发 dragend 事件
      evt = {
        originalEvent: event,
        type: 'dragend',
        intersect: _selected,
        target: _.get(intersects, '0.object', this),
        position,
        ray: _raycaster.ray,
      }
    }
    dispatchEvent(intersects, evt)

    // 重置 _selected 和 dragStatus
    _selected = null
    dragStatus = 'notDragging'
  }

  const dragAndDropHandler = (event, type) => {
    // event.preventDefault()

    const rect = _domElement.getBoundingClientRect()
    _mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1
    _mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1

    _raycaster.setFromCamera(_mouse, this._camera)

    let target = null
    const position = new Vector3()
    if (_raycaster.ray.intersectPlane(_plane, _intersection)) {
      position.copy(_intersection).setZ(0)
    }

    const intersects = _raycaster.intersectObjects(_objects, true)
    if (intersects.length) {
      resort(intersects)
      target = intersects[0].object
    }

    let data = event.dataTransfer.getData('application/json') || null

    if (data) {
      data = JSON.parse(data)
    }

    this.dispatchEvent({
      originalEvent: event,
      type,
      position,
      object: target,
      intersect: intersects[0],
      data,
      ray: _raycaster.ray,
    })
  }

  const onNativeDragOver = (event) => {
    dragAndDropHandler(event, 'nativedragover')
  }

  const onNativeDrop = (event) => {
    dragAndDropHandler(event, 'nativedrop')
  }

  /**
   * 基于 EventDispatcher 的事件派发函数进行包装的方法
   * @param intersects
   * @param {{type: string}} event
   */
  const dispatchEvent = (intersects, event) => {
    let returnValue = true
    let cancelBubble = false
    const evt = {
      ...event,
      stopPropagation () {
        cancelBubble = true
      },
    }

    intersects.every((intersect) => {
      const cancelBubbled = (returnValue === false || cancelBubble === true)
      if (cancelBubbled) {
        returnValue = intersect.object.dispatchEvent({ ...evt, intersect })
      }
      return !cancelBubbled
    })
    if (returnValue !== false && cancelBubble !== true) {
      this.dispatchEvent(evt)
    }
  }

  // 绑定事件
  function activate () {
    _domElement.addEventListener('pointermove', onDocumentMouseMove, false)
    _domElement.addEventListener('pointerdown', onDocumentMouseDown, false)
    _domElement.addEventListener('pointerup', onDocumentMouseUp, false)
    _domElement.addEventListener('dragover', onNativeDragOver, false)
    _domElement.addEventListener('drop', onNativeDrop, false)
  }

  // 解绑事件
  function deactivate () {
    _domElement.removeEventListener('pointermove', onDocumentMouseMove, false)
    _domElement.removeEventListener('pointerdown', onDocumentMouseDown, false)
    _domElement.removeEventListener('pointerup', onDocumentMouseUp, false)
    _domElement.removeEventListener('dragover', onNativeDragOver, false)
    _domElement.removeEventListener('drop', onNativeDrop, false)
  }

  function dispose () {
    deactivate()
  }

  activate()

  // API
  this.enabled = true

  this.activate = activate
  this.deactivate = deactivate
  this.dispose = dispose
  this.setCamera = function (camera) {
    this._camera = camera
  }
}

function resort (intersects) {
  // 射线检测结果排序，按照 renderOrder 降序排列
  intersects.sort((a, b) => {
    return b.object.renderOrder - a.object.renderOrder
  })
}

// 继承 EventDispatcher 事件派发器
DragControls.prototype = Object.create(EventDispatcher.prototype)
DragControls.prototype.constructor = DragControls

export { DragControls }
