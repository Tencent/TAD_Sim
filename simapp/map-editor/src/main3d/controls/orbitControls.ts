import type {
  Matrix4,
  OrthographicCamera,
  PerspectiveCamera,
} from 'three'
import {
  EventDispatcher,
  MOUSE,
  Quaternion,
  Spherical,
  TOUCH,
  Vector2,
  Vector3,
} from 'three'

const _changeEvent = { type: 'change' }
const _startEvent = { type: 'start' }
const _endEvent = { type: 'end' }

interface IPointerPosition {
  [key: number]: Vector2
}
interface IParams {
  object: PerspectiveCamera | OrthographicCamera
  domElement: HTMLElement
  render: Function
  startAnimationFrame: Function
  endAnimationFrame: Function
}

class OrbitControls extends EventDispatcher {
  object: PerspectiveCamera | OrthographicCamera
  domElement: HTMLElement
  render: Function
  startAnimationFrame: Function
  endAnimationFrame: Function
  enabled: boolean
  target: Vector3
  minDistance: number
  maxDistance: number
  minZoom: number
  maxZoom: number
  minPolarAngle: number
  maxPolarAngle: number
  minAzimuthAngle: number
  maxAzimuthAngle: number
  enableDamping: boolean
  dampingFactor: number
  enableZoom: boolean
  zoomSpeed: number
  enableRotate: boolean
  rotateSpeed: number
  enablePan: boolean
  panSpeed: number
  screenSpacePanning: boolean
  keyPanSpeed: number
  autoRotate: boolean
  autoRotateSpeed: number
  target0: Vector3
  position0: Vector3
  zoom0: number
  _domElementKeyEvents: HTMLElement | null

  getPolarAngle: Function
  getAzimuthalAngle: Function
  getDistance: Function
  listenToKeyEvents: Function
  saveState: Function
  reset: Function
  update: Function
  dispose: Function

  constructor (params: IParams) {
    super()

    const {
      object,
      domElement,
      render,
      startAnimationFrame,
      endAnimationFrame,
    } = params

    this.object = object
    this.domElement = domElement
    this.domElement.style.touchAction = 'none'
    this.render = render
    this.startAnimationFrame = startAnimationFrame
    this.endAnimationFrame = endAnimationFrame
    this.enabled = true
    this.target = new Vector3()
    this.minDistance = 0
    this.maxDistance = Number.POSITIVE_INFINITY
    this.minZoom = 0
    this.maxZoom = Number.POSITIVE_INFINITY
    this.minPolarAngle = 0
    this.maxPolarAngle = Math.PI
    this.minAzimuthAngle = Number.NEGATIVE_INFINITY
    this.maxAzimuthAngle = Number.POSITIVE_INFINITY
    this.enableDamping = false
    this.dampingFactor = 0.05
    this.enableZoom = true
    this.zoomSpeed = 1.0
    this.enableRotate = true
    this.rotateSpeed = 1.0
    this.enablePan = true
    this.panSpeed = 1.0
    this.screenSpacePanning = true
    this.keyPanSpeed = 7.0
    this.autoRotate = false
    this.autoRotateSpeed = 2.0
    this.target0 = this.target.clone()
    this.position0 = this.object.position.clone()
    this.zoom0 = this.object.zoom
    this._domElementKeyEvents = null

    // 内部变量
    const keys = {
      LEFT: 'ArrowLeft',
      UP: 'ArrowUp',
      RIGHT: 'ArrowRight',
      BOTTOM: 'ArrowDown',
    }
    const mouseButtons = {
      LEFT: MOUSE.ROTATE,
      MIDDLE: MOUSE.DOLLY,
      RIGHT: MOUSE.PAN,
    }
    const touches = { ONE: TOUCH.ROTATE, TWO: TOUCH.DOLLY_PAN }

    const STATE = {
      NONE: -1,
      ROTATE: 0,
      DOLLY: 1,
      PAN: 2,
      TOUCH_ROTATE: 3,
      TOUCH_PAN: 4,
      TOUCH_DOLLY_PAN: 5,
      TOUCH_DOLLY_ROTATE: 6,
    }

    let state = STATE.NONE

    const scope = this

    const EPS = 0.000001

    // current position in spherical coordinates
    const spherical = new Spherical()
    const sphericalDelta = new Spherical()

    let scale = 1
    const panOffset = new Vector3()
    let zoomChanged = false

    const rotateStart = new Vector2()
    const rotateEnd = new Vector2()
    const rotateDelta = new Vector2()

    const panStart = new Vector2()
    const panEnd = new Vector2()
    const panDelta = new Vector2()

    const dollyStart = new Vector2()
    const dollyEnd = new Vector2()
    const dollyDelta = new Vector2()

    const pointers: Array<PointerEvent> = []

    const pointerPositions: IPointerPosition = {}

    // 对外暴露的方法
    this.getPolarAngle = () => {
      return spherical.phi
    }

    this.getAzimuthalAngle = () => {
      return spherical.theta
    }

    this.getDistance = () => {
      return this.object.position.distanceTo(this.target)
    }

    this.listenToKeyEvents = (domElement: HTMLElement) => {
      domElement.addEventListener('keydown', onKeyDown)
      this._domElementKeyEvents = domElement
    }

    this.saveState = () => {
      this.target0.copy(this.target)
      this.position0.copy(this.object.position)
      this.zoom0 = this.object.zoom
    }

    this.reset = () => {
      this.target.copy(this.target0)
      this.object.position.copy(this.position0)
      this.object.zoom = this.zoom0

      this.object.updateProjectionMatrix()
      this.dispatchEvent(_changeEvent)

      this.update()

      state = STATE.NONE
    }

    this.update = (() => {
      const offset = new Vector3()

      // so camera.up is the orbit axis
      const quat = new Quaternion().setFromUnitVectors(
        object.up,
        new Vector3(0, 1, 0),
      )
      const quatInverse = quat.clone().invert()

      const lastPosition = new Vector3()
      const lastQuaternion = new Quaternion()

      const twoPI = 2 * Math.PI

      return function update (isMoveEvent: boolean) {
        const position = scope.object.position

        offset.copy(position).sub(scope.target)

        // rotate offset to "y-axis-is-up" space
        offset.applyQuaternion(quat)

        // angle from z-axis around y-axis
        spherical.setFromVector3(offset)

        if (scope.autoRotate && state === STATE.NONE) {
          rotateLeft(getAutoRotationAngle())
        }

        if (scope.enableDamping) {
          spherical.theta += sphericalDelta.theta * scope.dampingFactor
          spherical.phi += sphericalDelta.phi * scope.dampingFactor
        } else {
          spherical.theta += sphericalDelta.theta
          spherical.phi += sphericalDelta.phi
        }

        // restrict theta to be between desired limits

        let min = scope.minAzimuthAngle
        let max = scope.maxAzimuthAngle

        if (Number.isFinite(min) && Number.isFinite(max)) {
          if (min < -Math.PI) min += twoPI
          else if (min > Math.PI) min -= twoPI

          if (max < -Math.PI) max += twoPI
          else if (max > Math.PI) max -= twoPI

          if (min <= max) {
            spherical.theta = Math.max(min, Math.min(max, spherical.theta))
          } else {
            spherical.theta =
              spherical.theta > (min + max) / 2 ?
                Math.max(min, spherical.theta) :
                Math.min(max, spherical.theta)
          }
        }

        // restrict phi to be between desired limits
        spherical.phi = Math.max(
          scope.minPolarAngle,
          Math.min(scope.maxPolarAngle, spherical.phi),
        )

        spherical.makeSafe()

        spherical.radius *= scale

        // restrict radius to be between desired limits
        spherical.radius = Math.max(
          scope.minDistance,
          Math.min(scope.maxDistance, spherical.radius),
        )

        // move target to panned location

        if (scope.enableDamping === true) {
          scope.target.addScaledVector(panOffset, scope.dampingFactor)
        } else {
          scope.target.add(panOffset)
        }

        offset.setFromSpherical(spherical)

        // rotate offset back to "camera-up-vector-is-up" space
        offset.applyQuaternion(quatInverse)

        position.copy(scope.target).add(offset)

        scope.object.lookAt(scope.target)

        if (scope.enableDamping === true) {
          sphericalDelta.theta *= 1 - scope.dampingFactor
          sphericalDelta.phi *= 1 - scope.dampingFactor

          panOffset.multiplyScalar(1 - scope.dampingFactor)
        } else {
          sphericalDelta.set(0, 0, 0)

          panOffset.set(0, 0, 0)
        }

        scale = 1

        if (
          zoomChanged ||
          lastPosition.distanceToSquared(scope.object.position) > EPS ||
          8 * (1 - lastQuaternion.dot(scope.object.quaternion)) > EPS
        ) {
          scope.dispatchEvent(_changeEvent)

          lastPosition.copy(scope.object.position)
          lastQuaternion.copy(scope.object.quaternion)
          zoomChanged = false

          // 如果是 move 事件触发的相机控制器 update 方法，则使用性能更好的页面刷新机制
          // 避免高频触发的 move 事件引起的画面刷新的卡顿
          if (!isMoveEvent) {
            scope.render()
          }
          return true
        }
        if (!isMoveEvent) {
          scope.render()
        }
        return false
      }
    })()

    this.dispose = () => {
      this.domElement.removeEventListener('contextmenu', onContextMenu)

      this.domElement.removeEventListener('pointerdown', onPointerDown)
      this.domElement.removeEventListener('pointercancel', onPointerCancel)
      this.domElement.removeEventListener('wheel', onMouseWheel)

      this.domElement.removeEventListener('pointermove', onPointerMove)
      this.domElement.removeEventListener('pointerup', onPointerUp)

      if (this._domElementKeyEvents !== null) {
        this._domElementKeyEvents.removeEventListener('keydown', onKeyDown)
      }
    }

    // 内部方法
    function getAutoRotationAngle () {
      return ((2 * Math.PI) / 60 / 60) * scope.autoRotateSpeed
    }

    function getZoomScale () {
      return 0.95 ** scope.zoomSpeed
    }

    function rotateLeft (angle: number) {
      sphericalDelta.theta -= angle
    }

    function rotateUp (angle: number) {
      sphericalDelta.phi -= angle
    }

    const panLeft = (() => {
      const v = new Vector3()

      return function panLeft (distance: number, objectMatrix: Matrix4) {
        v.setFromMatrixColumn(objectMatrix, 0) // get X column of objectMatrix
        v.multiplyScalar(-distance)

        panOffset.add(v)
      }
    })()

    const panUp = (() => {
      const v = new Vector3()

      return function panUp (distance: number, objectMatrix: Matrix4) {
        if (scope.screenSpacePanning === true) {
          v.setFromMatrixColumn(objectMatrix, 1)
        } else {
          v.setFromMatrixColumn(objectMatrix, 0)
          v.crossVectors(scope.object.up, v)
        }

        v.multiplyScalar(distance)

        panOffset.add(v)
      }
    })()

    // deltaX and deltaY are in pixels; right and down are positive
    const pan = (() => {
      const offset = new Vector3()

      return function pan (deltaX: number, deltaY: number) {
        const element = scope.domElement

        if ((scope.object as PerspectiveCamera).isPerspectiveCamera) {
          // perspective
          const position = scope.object.position
          offset.copy(position).sub(scope.target)
          let targetDistance = offset.length()

          // half of the fov is center to top of screen
          targetDistance *= Math.tan(
            (((scope.object as PerspectiveCamera).fov / 2) * Math.PI) / 180.0,
          )

          // we use only clientHeight here so aspect ratio does not distort speed
          panLeft(
            (2 * deltaX * targetDistance) / element.clientHeight,
            scope.object.matrix,
          )
          panUp(
            (2 * deltaY * targetDistance) / element.clientHeight,
            scope.object.matrix,
          )
        } else if ((scope.object as OrthographicCamera).isOrthographicCamera) {
          // orthographic
          panLeft(
            (deltaX *
            ((scope.object as OrthographicCamera).right -
            (scope.object as OrthographicCamera).left)) /
            scope.object.zoom /
            element.clientWidth,
            scope.object.matrix,
          )
          panUp(
            (deltaY *
            ((scope.object as OrthographicCamera).top -
            (scope.object as OrthographicCamera).bottom)) /
            scope.object.zoom /
            element.clientHeight,
            scope.object.matrix,
          )
        } else {
          // camera neither orthographic nor perspective
          console.warn(
            'WARNING: OrbitControls.js encountered an unknown camera type - pan disabled.',
          )
          scope.enablePan = false
        }
      }
    })()

    function dollyOut (dollyScale: number) {
      if ((scope.object as PerspectiveCamera).isPerspectiveCamera) {
        scale /= dollyScale
      } else if ((scope.object as OrthographicCamera).isOrthographicCamera) {
        scope.object.zoom = Math.max(
          scope.minZoom,
          Math.min(scope.maxZoom, scope.object.zoom * dollyScale),
        )
        scope.object.updateProjectionMatrix()
        zoomChanged = true
      } else {
        console.warn(
          'WARNING: OrbitControls.js encountered an unknown camera type - dolly/zoom disabled.',
        )
        scope.enableZoom = false
      }
    }

    function dollyIn (dollyScale: number) {
      if ((scope.object as PerspectiveCamera).isPerspectiveCamera) {
        scale *= dollyScale
      } else if ((scope.object as OrthographicCamera).isOrthographicCamera) {
        scope.object.zoom = Math.max(
          scope.minZoom,
          Math.min(scope.maxZoom, scope.object.zoom / dollyScale),
        )
        scope.object.updateProjectionMatrix()
        zoomChanged = true
      } else {
        console.warn(
          'WARNING: OrbitControls.js encountered an unknown camera type - dolly/zoom disabled.',
        )
        scope.enableZoom = false
      }
    }

    //
    // event callbacks - update the object state
    //

    function handleMouseDownRotate (event: MouseEvent) {
      rotateStart.set(event.clientX, event.clientY)
    }

    function handleMouseDownDolly (event: MouseEvent) {
      dollyStart.set(event.clientX, event.clientY)
    }

    function handleMouseDownPan (event: MouseEvent) {
      panStart.set(event.clientX, event.clientY)
    }

    function handleMouseMoveRotate (event: MouseEvent) {
      rotateEnd.set(event.clientX, event.clientY)

      rotateDelta
        .subVectors(rotateEnd, rotateStart)
        .multiplyScalar(scope.rotateSpeed)

      const element = scope.domElement

      rotateLeft((2 * Math.PI * rotateDelta.x) / element.clientHeight) // yes, height

      rotateUp((2 * Math.PI * rotateDelta.y) / element.clientHeight)

      rotateStart.copy(rotateEnd)

      scope.update(true)
    }

    function handleMouseMoveDolly (event: MouseEvent) {
      dollyEnd.set(event.clientX, event.clientY)

      dollyDelta.subVectors(dollyEnd, dollyStart)

      if (dollyDelta.y > 0) {
        dollyOut(getZoomScale())
      } else if (dollyDelta.y < 0) {
        dollyIn(getZoomScale())
      }

      dollyStart.copy(dollyEnd)

      scope.update(true)
    }

    function handleMouseMovePan (event: MouseEvent) {
      panEnd.set(event.clientX, event.clientY)

      panDelta.subVectors(panEnd, panStart).multiplyScalar(scope.panSpeed)

      pan(panDelta.x, panDelta.y)

      panStart.copy(panEnd)

      scope.update(true)
    }

    function handleMouseWheel (event: WheelEvent) {
      let originZoom
      if (scope.object.isOrthographicCamera) {
        originZoom = scope.object.zoom
      }
      if (event.deltaY < 0) {
        dollyIn(getZoomScale())
      } else if (event.deltaY > 0) {
        dollyOut(getZoomScale())
      }
      if (scope.object.isOrthographicCamera) {
        const currentZoom = scope.object.zoom
        if (currentZoom !== originZoom) {
          scope.dispatchEvent({
            type: 'zoom',
            value: currentZoom,
          })
        }
      } else {
        // 由于透视视角不更新 zoom 值，所以无法触发 zoom 事件，需要自定义另一个事件来监听透视视角下的滚轮缩放
        scope.dispatchEvent({
          type: 'perspective-zoom',
        })
      }

      scope.update()
    }

    function handleKeyDown (event: KeyboardEvent) {
      let needsUpdate = false

      switch (event.code) {
        case keys.UP:
          pan(0, scope.keyPanSpeed)
          needsUpdate = true
          break

        case keys.BOTTOM:
          pan(0, -scope.keyPanSpeed)
          needsUpdate = true
          break

        case keys.LEFT:
          pan(scope.keyPanSpeed, 0)
          needsUpdate = true
          break

        case keys.RIGHT:
          pan(-scope.keyPanSpeed, 0)
          needsUpdate = true
          break
      }

      if (needsUpdate) {
        event.preventDefault()

        scope.update()
      }
    }

    function handleTouchStartRotate () {
      if (pointers.length === 1) {
        rotateStart.set(pointers[0].pageX, pointers[0].pageY)
      } else {
        const x = 0.5 * (pointers[0].pageX + pointers[1].pageX)
        const y = 0.5 * (pointers[0].pageY + pointers[1].pageY)

        rotateStart.set(x, y)
      }
    }

    function handleTouchStartPan () {
      if (pointers.length === 1) {
        panStart.set(pointers[0].pageX, pointers[0].pageY)
      } else {
        const x = 0.5 * (pointers[0].pageX + pointers[1].pageX)
        const y = 0.5 * (pointers[0].pageY + pointers[1].pageY)

        panStart.set(x, y)
      }
    }

    function handleTouchStartDolly () {
      const dx = pointers[0].pageX - pointers[1].pageX
      const dy = pointers[0].pageY - pointers[1].pageY

      const distance = Math.sqrt(dx * dx + dy * dy)

      dollyStart.set(0, distance)
    }

    function handleTouchStartDollyPan () {
      if (scope.enableZoom) handleTouchStartDolly()

      if (scope.enablePan) handleTouchStartPan()
    }

    function handleTouchStartDollyRotate () {
      if (scope.enableZoom) handleTouchStartDolly()

      if (scope.enableRotate) handleTouchStartRotate()
    }

    function handleTouchMoveRotate (event: PointerEvent) {
      if (pointers.length == 1) {
        rotateEnd.set(event.pageX, event.pageY)
      } else {
        const position = getSecondPointerPosition(event)

        const x = 0.5 * (event.pageX + position.x)
        const y = 0.5 * (event.pageY + position.y)

        rotateEnd.set(x, y)
      }

      rotateDelta
        .subVectors(rotateEnd, rotateStart)
        .multiplyScalar(scope.rotateSpeed)

      const element = scope.domElement

      rotateLeft((2 * Math.PI * rotateDelta.x) / element.clientHeight) // yes, height

      rotateUp((2 * Math.PI * rotateDelta.y) / element.clientHeight)

      rotateStart.copy(rotateEnd)
    }

    function handleTouchMovePan (event: PointerEvent) {
      if (pointers.length === 1) {
        panEnd.set(event.pageX, event.pageY)
      } else {
        const position = getSecondPointerPosition(event)

        const x = 0.5 * (event.pageX + position.x)
        const y = 0.5 * (event.pageY + position.y)

        panEnd.set(x, y)
      }

      panDelta.subVectors(panEnd, panStart).multiplyScalar(scope.panSpeed)

      pan(panDelta.x, panDelta.y)

      panStart.copy(panEnd)
    }

    function handleTouchMoveDolly (event: PointerEvent) {
      const position = getSecondPointerPosition(event)

      const dx = event.pageX - position.x
      const dy = event.pageY - position.y

      const distance = Math.sqrt(dx * dx + dy * dy)

      dollyEnd.set(0, distance)

      dollyDelta.set(0, (dollyEnd.y / dollyStart.y) ** scope.zoomSpeed)

      dollyOut(dollyDelta.y)

      dollyStart.copy(dollyEnd)
    }

    function handleTouchMoveDollyPan (event: PointerEvent) {
      if (scope.enableZoom) handleTouchMoveDolly(event)

      if (scope.enablePan) handleTouchMovePan(event)
    }

    function handleTouchMoveDollyRotate (event: PointerEvent) {
      if (scope.enableZoom) handleTouchMoveDolly(event)

      if (scope.enableRotate) handleTouchMoveRotate(event)
    }

    //
    // event handlers - FSM: listen for events and reset state
    //

    function onPointerDown (event: PointerEvent) {
      if (scope.enabled === false) return

      if (pointers.length === 0) {
        scope.domElement.setPointerCapture(event.pointerId)
        scope.domElement.addEventListener('pointermove', onPointerMove)
        scope.domElement.addEventListener('pointerup', onPointerUp)
      }

      addPointer(event)

      if (event.pointerType === 'touch') {
        onTouchStart(event)
      } else {
        onMouseDown(event)
      }

      // 在 pointerdown 时，开启高帧率的视图刷新
      scope.startAnimationFrame()
    }

    function onPointerMove (event: PointerEvent) {
      if (scope.enabled === false) return

      if (event.pointerType === 'touch') {
        onTouchMove(event)
      } else {
        onMouseMove(event)
      }
    }

    function onPointerUp (event: PointerEvent) {
      removePointer(event)

      if (pointers.length === 0) {
        scope.domElement.releasePointerCapture(event.pointerId)

        scope.domElement.removeEventListener('pointermove', onPointerMove)
        scope.domElement.removeEventListener('pointerup', onPointerUp)
      }

      scope.dispatchEvent(_endEvent)

      state = STATE.NONE

      scope.endAnimationFrame()
    }

    function onPointerCancel (event: PointerEvent) {
      removePointer(event)
      scope.endAnimationFrame()
    }

    function onMouseDown (event: MouseEvent) {
      let mouseAction

      switch (event.button) {
        case 0:
          mouseAction = mouseButtons.LEFT
          break

        case 1:
          mouseAction = mouseButtons.MIDDLE
          break

        case 2:
          mouseAction = mouseButtons.RIGHT
          break

        default:
          mouseAction = -1
      }

      switch (mouseAction) {
        case MOUSE.DOLLY:
          if (scope.enableZoom === false) return

          handleMouseDownDolly(event)

          state = STATE.DOLLY

          break

        case MOUSE.ROTATE:
          if (event.ctrlKey || event.metaKey || event.shiftKey) {
            if (scope.enablePan === false) return

            handleMouseDownPan(event)

            state = STATE.PAN
          } else {
            if (scope.enableRotate === false) return

            handleMouseDownRotate(event)

            state = STATE.ROTATE
          }

          break

        case MOUSE.PAN:
          if (event.ctrlKey || event.metaKey || event.shiftKey) {
            if (scope.enableRotate === false) return

            handleMouseDownRotate(event)

            state = STATE.ROTATE
          } else {
            if (scope.enablePan === false) return

            handleMouseDownPan(event)

            state = STATE.PAN
          }

          break

        default:
          state = STATE.NONE
      }

      if (state !== STATE.NONE) {
        scope.dispatchEvent(_startEvent)
      }
    }

    function onMouseMove (event: MouseEvent) {
      if (scope.enabled === false) return

      switch (state) {
        case STATE.ROTATE:
          if (scope.enableRotate === false) return

          handleMouseMoveRotate(event)

          break

        case STATE.DOLLY:
          if (scope.enableZoom === false) return

          handleMouseMoveDolly(event)

          break

        case STATE.PAN:
          if (scope.enablePan === false) return

          handleMouseMovePan(event)

          break
      }
    }

    function onMouseWheel (event: WheelEvent) {
      if (
        scope.enabled === false ||
        scope.enableZoom === false ||
        state !== STATE.NONE
      ) {
        return
      }

      event.preventDefault()

      scope.dispatchEvent(_startEvent)

      handleMouseWheel(event)

      scope.dispatchEvent(_endEvent)
    }

    function onKeyDown (event: KeyboardEvent) {
      if (scope.enabled === false || scope.enablePan === false) return

      handleKeyDown(event)
    }

    function onTouchStart (event: PointerEvent) {
      trackPointer(event)

      switch (pointers.length) {
        case 1:
          switch (touches.ONE) {
            case TOUCH.ROTATE:
              if (scope.enableRotate === false) return

              handleTouchStartRotate()

              state = STATE.TOUCH_ROTATE

              break

            case TOUCH.PAN:
              if (scope.enablePan === false) return

              handleTouchStartPan()

              state = STATE.TOUCH_PAN

              break

            default:
              state = STATE.NONE
          }

          break

        case 2:
          switch (touches.TWO) {
            case TOUCH.DOLLY_PAN:
              if (scope.enableZoom === false && scope.enablePan === false) {
                return
              }

              handleTouchStartDollyPan()

              state = STATE.TOUCH_DOLLY_PAN

              break

            case TOUCH.DOLLY_ROTATE:
              if (scope.enableZoom === false && scope.enableRotate === false) {
                return
              }

              handleTouchStartDollyRotate()

              state = STATE.TOUCH_DOLLY_ROTATE

              break

            default:
              state = STATE.NONE
          }

          break

        default:
          state = STATE.NONE
      }

      if (state !== STATE.NONE) {
        scope.dispatchEvent(_startEvent)
      }
    }

    function onTouchMove (event: PointerEvent) {
      trackPointer(event)

      switch (state) {
        case STATE.TOUCH_ROTATE:
          if (scope.enableRotate === false) return

          handleTouchMoveRotate(event)

          scope.update(true)

          break

        case STATE.TOUCH_PAN:
          if (scope.enablePan === false) return

          handleTouchMovePan(event)

          scope.update(true)

          break

        case STATE.TOUCH_DOLLY_PAN:
          if (scope.enableZoom === false && scope.enablePan === false) return

          handleTouchMoveDollyPan(event)

          scope.update(true)

          break

        case STATE.TOUCH_DOLLY_ROTATE:
          if (scope.enableZoom === false && scope.enableRotate === false) return

          handleTouchMoveDollyRotate(event)

          scope.update(true)

          break

        default:
          state = STATE.NONE
      }
    }

    function onContextMenu (event: MouseEvent) {
      if (scope.enabled === false) return

      event.preventDefault()
    }

    function addPointer (event: PointerEvent) {
      pointers.push(event)
    }

    function removePointer (event: PointerEvent) {
      delete pointerPositions[event.pointerId]

      for (let i = 0; i < pointers.length; i++) {
        if (pointers[i].pointerId == event.pointerId) {
          pointers.splice(i, 1)
          return
        }
      }
    }

    function trackPointer (event: PointerEvent) {
      let position = pointerPositions[event.pointerId]

      if (position === undefined) {
        position = new Vector2()
        pointerPositions[event.pointerId] = position
      }

      position.set(event.pageX, event.pageY)
    }

    function getSecondPointerPosition (event: PointerEvent) {
      const pointer =
        event.pointerId === pointers[0].pointerId ? pointers[1] : pointers[0]

      return pointerPositions[pointer.pointerId]
    }

    this.domElement.addEventListener('contextmenu', onContextMenu)

    this.domElement.addEventListener('pointerdown', onPointerDown)
    this.domElement.addEventListener('pointercancel', onPointerCancel)
    this.domElement.addEventListener('wheel', onMouseWheel, {
      passive: false,
    })

    // force an update at start
    this.update()
  }
}

export default OrbitControls
