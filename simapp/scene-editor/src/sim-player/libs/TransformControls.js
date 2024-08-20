/**
 * @author arodic / https://github.com/arodic
 */
import {
  BoxGeometry,
  BufferGeometry,
  Color,
  CylinderGeometry,
  DoubleSide,
  Euler,
  Float32BufferAttribute,
  Line,
  LineBasicMaterial,
  Matrix4,
  Mesh,
  MeshBasicMaterial,
  Object3D,
  OctahedronGeometry,
  OrthographicCamera,
  PerspectiveCamera,
  PlaneGeometry,
  Quaternion,
  Raycaster,
  SphereGeometry,
  TorusGeometry,
  Vector3,
} from 'three'

/**
 * 场景元素操作类，拖动效果等
 * @param camera
 * @param domElement
 * @constructor
 */
class TransformControls extends Object3D {
  constructor(camera, domElement) {
    super()

    domElement = (domElement !== undefined) ? domElement.parentNode : document

    this.visible = false

    const _gizmo = new TransformControlsGizmo()
    this.add(_gizmo)

    // 拖动时检测相交的地平面
    const _plane = new TransformControlsPlane()
    this.add(_plane)

    const scope = this

    // Define properties with getters/setter
    // Setting the defined property will automatically trigger change event
    // Defined properties are passed down to gizmo and plane
    defineProperty('camera', camera)
    defineProperty('object', undefined)
    defineProperty('enabled', true)
    defineProperty('axis', null)
    defineProperty('mode', 'translate')
    defineProperty('translationSnap', null)
    defineProperty('rotationSnap', null)
    defineProperty('space', 'local')
    defineProperty('size', 1)
    defineProperty('dragging', false)
    defineProperty('showX', true)
    defineProperty('showY', true)
    defineProperty('showZ', true)

    // 复用的事件对象
    const changeEvent = { type: 'change' }
    const mouseDownEvent = { type: 'mouseDown' }
    const mouseUpEvent = { type: 'mouseUp', mode: scope.mode }
    const objectChangeEvent = { type: 'objectChange' }

    // Reusable utility variables
    const ray = new Raycaster()

    let _tempVector = new Vector3()
    let _tempVector2 = new Vector3()
    const _tempQuaternion = new Quaternion()
    const _unit = {
      X: new Vector3(1, 0, 0),
      Y: new Vector3(0, 1, 0),
      Z: new Vector3(0, 0, 1),
    }
    const _identityQuaternion = new Quaternion()
    const _alignVector = new Vector3()

    const pointStart = new Vector3()
    const pointEnd = new Vector3()
    const rotationAxis = new Vector3()
    let rotationAngle = 0

    const cameraPosition = new Vector3()
    const cameraQuaternion = new Quaternion()
    const cameraScale = new Vector3()

    const parentPosition = new Vector3()
    const parentQuaternion = new Quaternion()
    const parentScale = new Vector3()

    const worldPositionStart = new Vector3()
    const worldQuaternionStart = new Quaternion()
    const worldScaleStart = new Vector3()

    const worldPosition = new Vector3()
    const worldQuaternion = new Quaternion()
    const worldScale = new Vector3()

    const eye = new Vector3()

    const _positionStart = new Vector3()
    const _quaternionStart = new Quaternion()
    const _scaleStart = new Vector3()

    // TODO: remove properties unused in plane and gizmo

    defineProperty('parentQuaternion', parentQuaternion)
    defineProperty('worldPosition', worldPosition)
    defineProperty('worldPositionStart', worldPositionStart)
    defineProperty('worldQuaternion', worldQuaternion)
    defineProperty('worldQuaternionStart', worldQuaternionStart)
    defineProperty('cameraPosition', cameraPosition)
    defineProperty('cameraQuaternion', cameraQuaternion)
    defineProperty('pointStart', pointStart)
    defineProperty('pointEnd', pointEnd)
    defineProperty('rotationAxis', rotationAxis)
    defineProperty('rotationAngle', rotationAngle)
    defineProperty('eye', eye)

    domElement.addEventListener('mousedown', onPointerDown, false)
    domElement.addEventListener('touchstart', onPointerDown, false)
    domElement.addEventListener('mousemove', onPointerHover, false)
    domElement.addEventListener('touchmove', onPointerHover, false)
    if (domElement.parentNode != null) {
      domElement.parentNode.addEventListener('mousemove', onPointerMove, false)
      domElement.parentNode.addEventListener('mouseup', onPointerUp, false)
    }
    domElement.addEventListener('touchmove', onPointerMove, false)
    domElement.addEventListener('touchend', onPointerUp, false)
    domElement.addEventListener('touchcancel', onPointerUp, false)
    domElement.addEventListener('touchleave', onPointerUp, false)
    domElement.addEventListener('contextmenu', onContext, false)

    this.dispose = function () {
      domElement.removeEventListener('mousedown', onPointerDown)
      domElement.removeEventListener('touchstart', onPointerDown)
      domElement.removeEventListener('mousemove', onPointerHover)
      domElement.removeEventListener('touchmove', onPointerHover)
      if (domElement.parentNode != null) {
        domElement.parentNode.removeEventListener('mousemove', onPointerMove, false)
        domElement.parentNode.removeEventListener('mouseup', onPointerUp, false)
      }
      domElement.removeEventListener('touchmove', onPointerMove)
      domElement.removeEventListener('touchend', onPointerUp)
      domElement.removeEventListener('touchcancel', onPointerUp)
      domElement.removeEventListener('touchleave', onPointerUp)
      domElement.removeEventListener('contextmenu', onContext)
    }

    /**
     * Set current object
     * @param object
     */
    this.attach = function (object) {
      this.object = object
      this.visible = true
    }

    /**
     * Detatch from object
     */
    this.detach = function () {
      this.object = undefined
      this.visible = false
      this.axis = null
    }

    /**
     * Defined getter, setter and store for a property
     * @param propName
     * @param defaultValue
     * @return {*}
     */
    function defineProperty(propName, defaultValue) {
      let propValue = defaultValue

      Object.defineProperty(scope, propName, {
        get() {
          return propValue !== undefined ? propValue : defaultValue
        },
        set(value) {
          if (propValue !== value) {
            propValue = value
            _plane[propName] = value
            _gizmo[propName] = value

            scope.dispatchEvent({ type: `${propName}-changed`, value })
            scope.dispatchEvent(changeEvent)
          }
        },
      })

      scope[propName] = defaultValue
      _plane[propName] = defaultValue
      _gizmo[propName] = defaultValue
    }

    /**
     * updateMatrixWorld  updates key transformation variables
     */
    this.updateMatrixWorld = function () {
      if (this.object !== undefined) {
        this.object.updateMatrixWorld()
        this.object.parent && this.object.parent.matrixWorld.decompose(parentPosition, parentQuaternion, parentScale)
        this.object.matrixWorld.decompose(worldPosition, worldQuaternion, worldScale)
      }

      this.camera.updateMatrixWorld()
      this.camera.matrixWorld.decompose(cameraPosition, cameraQuaternion, cameraScale)

      if (this.camera instanceof PerspectiveCamera) {
        eye.copy(cameraPosition).sub(worldPosition).normalize()
      } else if (this.camera instanceof OrthographicCamera) {
        eye.copy(cameraPosition).sub(worldPosition).normalize()
      }

      Object3D.prototype.updateMatrixWorld.call(this)
    }

    // 鼠标hover事件
    this.pointerHover = function (pointer) {
      if (
        this.object === undefined ||
        this.dragging === true ||
        (pointer.button !== undefined && pointer.button !== 0)
      ) return

      ray.setFromCamera(pointer, this.camera)

      const intersect = ray.intersectObjects(_gizmo.picker[this.mode].children, true)[0] || false

      if (intersect) {
        this.axis = intersect.object.name
      } else {
        this.axis = null
      }
    }

    // 鼠标按下事件
    this.pointerDown = function (pointer) {
      if (
        this.object === undefined ||
        this.dragging === true ||
        (pointer.button !== undefined && pointer.button !== 0)
      ) return

      if ((pointer.button === 0 || pointer.button === undefined) && this.axis !== null) {
        ray.setFromCamera(pointer, this.camera)

        // 激光拾取法
        const planeIntersect = ray.intersectObjects([_plane], true)[0] || false

        if (planeIntersect) {
          let space = this.space

          if (this.mode === 'scale') {
            space = 'local'
          } else if (this.axis === 'E' || this.axis === 'XYZE' || this.axis === 'XYZ') {
            space = 'world'
          }

          if (space === 'local' && this.mode === 'rotate') {
            const snap = this.rotationSnap

            if (this.axis === 'X' && snap) this.object.rotation.x = Math.round(this.object.rotation.x / snap) * snap
            if (this.axis === 'Y' && snap) this.object.rotation.y = Math.round(this.object.rotation.y / snap) * snap
            if (this.axis === 'Z' && snap) this.object.rotation.z = Math.round(this.object.rotation.z / snap) * snap
          }

          this.object.updateMatrixWorld()
          this.object.parent.updateMatrixWorld()

          // 记录起始状态
          _positionStart.copy(this.object.position)
          _quaternionStart.copy(this.object.quaternion)
          _scaleStart.copy(this.object.scale)

          this.object.matrixWorld.decompose(worldPositionStart, worldQuaternionStart, worldScaleStart)

          pointStart.copy(planeIntersect.point).sub(worldPositionStart)

          if (space === 'local') pointStart.applyQuaternion(worldQuaternionStart.clone().invert())
        }

        this.dragging = true
        mouseDownEvent.mode = this.mode
        this.dispatchEvent(mouseDownEvent)
      }
    }

    // 鼠标移动事件
    this.pointerMove = function (pointer) {
      const axis = this.axis
      const mode = this.mode
      const object = this.object
      let space = this.space

      if (mode === 'scale') {
        space = 'local'
      } else if (axis === 'E' || axis === 'XYZE' || axis === 'XYZ') {
        space = 'world'
      }
      if (
        object === undefined ||
        axis === null ||
        this.dragging === false ||
        (pointer.button !== undefined && pointer.button !== 0)
      ) return

      ray.setFromCamera(pointer, this.camera)

      const planeIntersect = ray.intersectObjects([_plane], true)[0] || false

      if (planeIntersect === false) return

      pointEnd.copy(planeIntersect.point).sub(worldPositionStart)

      // 本地坐标系移动时，转换一下旋转
      if (space === 'local') pointEnd.applyQuaternion(worldQuaternionStart.clone().invert())

      if (mode === 'translate') {
        if (axis.search('X') === -1) {
          pointEnd.x = pointStart.x
        }
        if (axis.search('Y') === -1) {
          pointEnd.y = pointStart.y
        }
        if (axis.search('Z') === -1) {
          pointEnd.z = pointStart.z
        }

        // Apply translate
        if (space === 'local') {
          object.position.copy(pointEnd).sub(pointStart).applyQuaternion(_quaternionStart)
        } else {
          object.position.copy(pointEnd).sub(pointStart)
        }

        object.position.add(_positionStart)

        // Apply translation snap
        if (this.translationSnap) {
          if (space === 'local') {
            object.position.applyQuaternion(_tempQuaternion.copy(_quaternionStart).invert())

            if (axis.search('X') !== -1) {
              object.position.x = Math.round(object.position.x / this.translationSnap) * this.translationSnap
            }

            if (axis.search('Y') !== -1) {
              object.position.y = Math.round(object.position.y / this.translationSnap) * this.translationSnap
            }

            if (axis.search('Z') !== -1) {
              object.position.z = Math.round(object.position.z / this.translationSnap) * this.translationSnap
            }

            object.position.applyQuaternion(_quaternionStart)
          }

          if (space === 'world') {
            if (object.parent) {
              object.position.add(_tempVector.setFromMatrixPosition(object.parent.matrixWorld))
            }

            if (axis.search('X') !== -1) {
              object.position.x = Math.round(object.position.x / this.translationSnap) * this.translationSnap
            }

            if (axis.search('Y') !== -1) {
              object.position.y = Math.round(object.position.y / this.translationSnap) * this.translationSnap
            }

            if (axis.search('Z') !== -1) {
              object.position.z = Math.round(object.position.z / this.translationSnap) * this.translationSnap
            }

            if (object.parent) {
              object.position.sub(_tempVector.setFromMatrixPosition(object.parent.matrixWorld))
            }
          }
        }
      } else if (mode === 'scale') {
        if (axis.search('XYZ') !== -1) {
          let d = pointEnd.length() / pointStart.length()

          if (pointEnd.dot(pointStart) < 0) d *= -1

          _tempVector.set(d, d, d)
        } else {
          _tempVector.copy(pointEnd).divide(pointStart)

          if (axis.search('X') === -1) {
            _tempVector.x = 1
          }
          if (axis.search('Y') === -1) {
            _tempVector.y = 1
          }
          if (axis.search('Z') === -1) {
            _tempVector.z = 1
          }
        }

        // Apply scale
        object.scale.copy(_scaleStart).multiply(_tempVector)
      } else if (mode === 'rotate') {
        // 旋转速度
        const ROTATION_SPEED = 20 / worldPosition.distanceTo(_tempVector.setFromMatrixPosition(this.camera.matrixWorld))

        const quaternion = this.space === 'local' ? worldQuaternion : _identityQuaternion

        const unit = _unit[axis]

        if (axis === 'E') {
          _tempVector.copy(pointEnd).cross(pointStart)
          rotationAxis.copy(eye)
          rotationAngle = pointEnd.angleTo(pointStart) * (_tempVector.dot(eye) < 0 ? 1 : -1)
        } else if (axis === 'XYZE') {
          _tempVector.copy(pointEnd).sub(pointStart).cross(eye).normalize()
          rotationAxis.copy(_tempVector)
          rotationAngle = pointEnd.sub(pointStart).dot(_tempVector.cross(eye)) * ROTATION_SPEED
        } else if (axis === 'X' || axis === 'Y' || axis === 'Z') {
          _alignVector.copy(unit).applyQuaternion(quaternion)

          rotationAxis.copy(unit)

          _tempVector = unit.clone()
          _tempVector2 = pointEnd.clone().sub(pointStart)
          if (space === 'local') {
            _tempVector.applyQuaternion(quaternion)
            _tempVector2.applyQuaternion(worldQuaternionStart)
          }
          rotationAngle = _tempVector2.dot(_tempVector.cross(eye).normalize()) * ROTATION_SPEED
        }

        // Apply rotation snap
        if (this.rotationSnap) rotationAngle = Math.round(rotationAngle / this.rotationSnap) * this.rotationSnap

        this.rotationAngle = rotationAngle

        // Apply rotate
        if (space === 'local') {
          object.quaternion.copy(_quaternionStart)
          object.quaternion.multiply(_tempQuaternion.setFromAxisAngle(rotationAxis, rotationAngle))
        } else {
          object.quaternion.copy(_tempQuaternion.setFromAxisAngle(rotationAxis, rotationAngle))
          object.quaternion.multiply(_quaternionStart)
        }
      }

      this.dispatchEvent(changeEvent)
      this.dispatchEvent(objectChangeEvent)
    }

    // 鼠标抬起事件
    this.pointerUp = function (pointer) {
      if (pointer.button !== undefined && pointer.button !== 0) return

      if (this.dragging && (this.axis !== null)) {
        mouseUpEvent.mode = this.mode
        this.dispatchEvent(mouseUpEvent)
      }

      this.dragging = false

      if (pointer.button === undefined) this.axis = null
    }

    /**
     * normalize mouse / touch pointer and remap {x,y} to view space.
     * @param event
     * @return {{button, x: number, y: number}}
     */
    function getPointer(event) {
      const pointer = event.changedTouches ? event.changedTouches[0] : event

      const rect = domElement.getBoundingClientRect()

      return {
        x: (pointer.clientX - rect.left) / rect.width * 2 - 1,
        y: -(pointer.clientY - rect.top) / rect.height * 2 + 1,
        button: event.button,
      }
    }

    /**
     * mouse / touch event handlers
     * @param event
     */
    function onContext(event) {
      event.preventDefault()
    }

    /**
     * 鼠标hover事件
     * @param event
     */
    function onPointerHover(event) {
      if (!scope.enabled) return
      scope.pointerHover(getPointer(event))
    }

    /**
     * 鼠标按下事件
     * @param event
     */
    function onPointerDown(event) {
      if (!scope.enabled) return
      event.preventDefault()
      scope.pointerHover(getPointer(event))
      scope.pointerDown(getPointer(event))
    }

    /**
     * 鼠标移动事件
     * @param event
     */
    function onPointerMove(event) {
      if (!scope.enabled) return
      event.preventDefault()
      scope.pointerMove(getPointer(event))
    }

    /**
     * 鼠标抬起事件
     * @param event
     */
    function onPointerUp(event) {
      if (!scope.enabled) return
      // Prevent MouseEvent on mobile
      event.preventDefault()
      scope.pointerUp(getPointer(event))
    }

    // TODO: depricate
    this.getMode = function () {
      return scope.mode
    }

    this.setMode = function (mode) {
      scope.mode = mode
    }

    // this.setTranslationSnap = function (translationSnap) {
    //   scope.translationSnap = translationSnap
    // }

    // this.setRotationSnap = function (rotationSnap) {
    //   scope.rotationSnap = rotationSnap
    // }

    this.setSize = function (size) {
      scope.size = size
    }

    this.setSpace = function (space) {
      scope.space = space
    }

    this.update = function () {
      console.warn('TransformControls: update function has been depricated.')
    }
  }
  isTransformControls = true
}

/**
 * 鼠标操作手柄
 * @constructor
 */
class TransformControlsGizmo extends Object3D {
  constructor() {
    'use strict'
    super()
    this.type = 'TransformControlsGizmo'
    // shared materials
    const gizmoMaterial = new MeshBasicMaterial({
      depthTest: false,
      depthWrite: false,
      transparent: true,
      side: DoubleSide,
      fog: false,
    })
    const gizmoLineMaterial = new LineBasicMaterial({
      depthTest: false,
      depthWrite: false,
      transparent: true,
      linewidth: 1,
      fog: false,
    })

    // Make unique material for each axis/color
    const matInvisible = gizmoMaterial.clone()
    matInvisible.opacity = 0.15

    const matHelper = gizmoMaterial.clone()
    matHelper.opacity = 0.33

    const matRed = gizmoMaterial.clone()
    matRed.color.set(0xFF0000)

    const matGreen = gizmoMaterial.clone()
    matGreen.color.set(0x00FF00)

    const matBlue = gizmoMaterial.clone()
    matBlue.color.set(0x0000FF)

    const matWhiteTransperent = gizmoMaterial.clone()
    matWhiteTransperent.opacity = 0.25

    const matYellowTransparent = matWhiteTransperent.clone()
    matYellowTransparent.color.set(0xFFFF00)

    const matCyanTransparent = matWhiteTransperent.clone()
    matCyanTransparent.color.set(0x00FFFF)

    const matMagentaTransparent = matWhiteTransperent.clone()
    matMagentaTransparent.color.set(0xFF00FF)

    const matYellow = gizmoMaterial.clone()
    matYellow.color.set(0xFFFF00)

    const matLineRed = gizmoLineMaterial.clone()
    matLineRed.color.set(0xFF0000)

    const matLineGreen = gizmoLineMaterial.clone()
    matLineGreen.color.set(0x00FF00)

    const matLineBlue = gizmoLineMaterial.clone()
    matLineBlue.color.set(0x0000FF)

    const matLineCyan = gizmoLineMaterial.clone()
    matLineCyan.color.set(0x00FFFF)

    const matLineMagenta = gizmoLineMaterial.clone()
    matLineMagenta.color.set(0xFF00FF)

    const matLineYellow = gizmoLineMaterial.clone()
    matLineYellow.color.set(0xFFFF00)

    const matLineGray = gizmoLineMaterial.clone()
    matLineGray.color.set(0x787878)

    const matLineYellowTransparent = matLineYellow.clone()
    matLineYellowTransparent.opacity = 0.25

    // reusable geometry
    const arrowGeometry = new CylinderGeometry(0, 0.05, 0.2, 12, 1, false)

    const scaleHandleGeometry = new BoxGeometry(0.125, 0.125, 0.125)

    const lineGeometry = new BufferGeometry()
    lineGeometry.setAttribute('position', new Float32BufferAttribute([0, 0, 0, 1, 0, 0], 3))

    /**
     * 圈圈几何体
     * @param radius
     * @param arc
     * @return {BufferGeometry}
     * @constructor
     */
    const CircleGeometry = function (radius, arc) {
      const geometry = new BufferGeometry()
      const vertices = []

      for (let i = 0; i <= 64 * arc; ++i) {
        vertices.push(0, Math.cos(i / 32 * Math.PI) * radius, Math.sin(i / 32 * Math.PI) * radius)
      }

      geometry.setAttribute('position', new Float32BufferAttribute(vertices, 3))

      return geometry
    }

    /**
     * Special geometry for transform helper. If scaled with position vector it spans from [0,0,0] to position
     * @param radius
     * @param arc
     * @return {BufferGeometry}
     * @constructor
     */
    const TranslateHelperGeometry = function (radius, arc) {
      const geometry = new BufferGeometry()
      geometry.setAttribute('position', new Float32BufferAttribute([0, 0, 0, 1, 1, 1], 3))
      return geometry
    }

    /**
     * Gizmo definitions - custom hierarchy definitions for setupGizmo() function
     * @type {{
     * XY: ((Mesh|number[])[]|(Line|number[])[])[],
     * YZ: ((Mesh|number[])[]|(Line|number[])[])[],
     * XZ: ((Mesh|number[])[]|(Line|number[])[])[],
     * X: ((Mesh|number[]|string)[]|Line[])[],
     * XYZ: (Mesh|number[])[][],
     * Y: ((Mesh|number[]|string)[]|(Mesh|number[]|(number|number)[]|string)[]|(Line|number[])[])[],
     * Z: ((Mesh|number[]|string)[]|(Line|number[])[])[]
     * }}
     */
    const gizmoTranslate = {
      X: [
        [new Mesh(arrowGeometry, matRed), [1, 0, 0], [0, 0, -Math.PI / 2], null, 'fwd'],
        [new Mesh(arrowGeometry, matRed), [1, 0, 0], [0, 0, Math.PI / 2], null, 'bwd'],
        [new Line(lineGeometry, matLineRed)],
      ],
      Y: [
        [new Mesh(arrowGeometry, matGreen), [0, 1, 0], null, null, 'fwd'],
        [new Mesh(arrowGeometry, matGreen), [0, 1, 0], [Math.PI, 0, 0], null, 'bwd'],
        [new Line(lineGeometry, matLineGreen), null, [0, 0, Math.PI / 2]],
      ],
      Z: [
        [new Mesh(arrowGeometry, matBlue), [0, 0, 1], [Math.PI / 2, 0, 0], null, 'fwd'],
        [new Mesh(arrowGeometry, matBlue), [0, 0, 1], [-Math.PI / 2, 0, 0], null, 'bwd'],
        [new Line(lineGeometry, matLineBlue), null, [0, -Math.PI / 2, 0]],
      ],
      XYZ: [
        [new Mesh(new OctahedronGeometry(0.1, 0), matWhiteTransperent), [0, 0, 0], [0, 0, 0]],
      ],
      XY: [
        [new Mesh(new PlaneGeometry(0.295, 0.295), matYellowTransparent), [0.15, 0.15, 0]],
        [new Line(lineGeometry, matLineYellow), [0.18, 0.3, 0], null, [0.125, 1, 1]],
        [new Line(lineGeometry, matLineYellow), [0.3, 0.18, 0], [0, 0, Math.PI / 2], [0.125, 1, 1]],
      ],
      YZ: [
        [new Mesh(new PlaneGeometry(0.295, 0.295), matCyanTransparent), [0, 0.15, 0.15], [0, Math.PI / 2, 0]],
        [new Line(lineGeometry, matLineCyan), [0, 0.18, 0.3], [0, 0, Math.PI / 2], [0.125, 1, 1]],
        [new Line(lineGeometry, matLineCyan), [0, 0.3, 0.18], [0, -Math.PI / 2, 0], [0.125, 1, 1]],
      ],
      XZ: [
        [new Mesh(new PlaneGeometry(0.295, 0.295), matMagentaTransparent), [0.15, 0, 0.15], [-Math.PI / 2, 0, 0]],
        [new Line(lineGeometry, matLineMagenta), [0.18, 0, 0.3], null, [0.125, 1, 1]],
        [new Line(lineGeometry, matLineMagenta), [0.3, 0, 0.18], [0, -Math.PI / 2, 0], [0.125, 1, 1]],
      ],
    }

    /**
     * Picker definitions - custom hierarchy definitions for setupPicker() function
     * @type {{
     * XY: (Mesh|number[])[][],
     * YZ: (Mesh|number[])[][],
     * XZ: (Mesh|number[])[][],
     * X: (Mesh|number[])[][],
     * XYZ: Mesh[][],
     * Y: (Mesh|number[])[][],
     * Z: (Mesh|number[])[][]
     * }}
     */
    const pickerTranslate = {
      X: [
        [new Mesh(new CylinderGeometry(0.2, 0, 1, 4, 1, false), matInvisible), [0.6, 0, 0], [0, 0, -Math.PI / 2]],
      ],
      Y: [
        [new Mesh(new CylinderGeometry(0.2, 0, 1, 4, 1, false), matInvisible), [0, 0.6, 0]],
      ],
      Z: [
        [new Mesh(new CylinderGeometry(0.2, 0, 1, 4, 1, false), matInvisible), [0, 0, 0.6], [Math.PI / 2, 0, 0]],
      ],
      XYZ: [
        [new Mesh(new OctahedronGeometry(0.2, 0), matInvisible)],
      ],
      XY: [
        [new Mesh(new PlaneGeometry(0.4, 0.4), matInvisible), [0.2, 0.2, 0]],
      ],
      YZ: [
        [new Mesh(new PlaneGeometry(0.4, 0.4), matInvisible), [0, 0.2, 0.2], [0, Math.PI / 2, 0]],
      ],
      XZ: [
        [new Mesh(new PlaneGeometry(0.4, 0.4), matInvisible), [0.2, 0, 0.2], [-Math.PI / 2, 0, 0]],
      ],
    }

    /**
     * Helper definitions - custom hierarchy definitions for setupHelper() function
     * @type {{
     * DELTA: (Line|string)[][],
     * X: (Line|number[]|string)[][],
     * Y: (Line|number[]|string)[][],
     * START: (Mesh|string)[][],
     * END: (Mesh|string)[][],
     * Z: (Line|number[]|string)[][]
     * }}
     */
    const helperTranslate = {
      START: [
        [new Mesh(new OctahedronGeometry(0.01, 2), matHelper), null, null, null, 'helper'],
      ],
      END: [
        [new Mesh(new OctahedronGeometry(0.01, 2), matHelper), null, null, null, 'helper'],
      ],
      DELTA: [
        [new Line(TranslateHelperGeometry(), matHelper), null, null, null, 'helper'],
      ],
      X: [
        [new Line(lineGeometry, matHelper.clone()), [-1e3, 0, 0], null, [1e6, 1, 1], 'helper'],
      ],
      Y: [
        [new Line(lineGeometry, matHelper.clone()), [0, -1e3, 0], [0, 0, Math.PI / 2], [1e6, 1, 1], 'helper'],
      ],
      Z: [
        [new Line(lineGeometry, matHelper.clone()), [0, 0, -1e3], [0, -Math.PI / 2, 0], [1e6, 1, 1], 'helper'],
      ],
    }

    /**
     * Gizmo definitions - custom hierarchy definitions for setupGizmo() function
     * @type {{
     * E: ((Line|number[])[]|(Mesh|number[])[]|(Mesh|number[]|(number|number)[])[])[],
     * XYZE: (Line|number[])[][],
     * X: (Line[]|(Mesh|number[])[])[],
     * Y: ((Line|number[])[]|(Mesh|number[])[])[],
     * Z: ((Line|number[])[]|(Mesh|number[])[])[]
     * }}
     */
    const gizmoRotate = {
      X: [
        [new Line(CircleGeometry(1, 0.5), matLineRed)],
        [new Mesh(new OctahedronGeometry(0.04, 0), matRed), [0, 0, 0.99], null, [1, 3, 1]],
      ],
      Y: [
        [new Line(CircleGeometry(1, 0.5), matLineGreen), null, [0, 0, -Math.PI / 2]],
        [new Mesh(new OctahedronGeometry(0.04, 0), matGreen), [0, 0, 0.99], null, [3, 1, 1]],
      ],
      Z: [
        [new Line(CircleGeometry(1, 0.5), matLineBlue), null, [0, Math.PI / 2, 0]],
        [new Mesh(new OctahedronGeometry(0.04, 0), matBlue), [0.99, 0, 0], null, [1, 3, 1]],
      ],
      E: [
        [new Line(CircleGeometry(1.25, 1), matLineYellowTransparent), null, [0, Math.PI / 2, 0]],
        [
          new Mesh(
            new CylinderGeometry(0.03, 0, 0.15, 4, 1, false),
            matLineYellowTransparent,
          ),
          [1.17, 0, 0],
          [0, 0, -Math.PI / 2],
          [1, 1, 0.001],
        ],
        [
          new Mesh(
            new CylinderGeometry(0.03, 0, 0.15, 4, 1, false),
            matLineYellowTransparent,
          ),
          [-1.17, 0, 0],
          [0, 0, Math.PI / 2],
          [1, 1, 0.001],
        ],
        [
          new Mesh(
            new CylinderGeometry(0.03, 0, 0.15, 4, 1, false),
            matLineYellowTransparent,
          ),
          [0, -1.17, 0],
          [Math.PI, 0, 0],
          [1, 1, 0.001],
        ],
        [
          new Mesh(
            new CylinderGeometry(0.03, 0, 0.15, 4, 1, false),
            matLineYellowTransparent,
          ),
          [0, 1.17, 0],
          [0, 0, 0],
          [1, 1, 0.001],
        ],
      ],
      XYZE: [
        [new Line(CircleGeometry(1, 1), matLineGray), null, [0, Math.PI / 2, 0]],
      ],
    }

    /**
     * 旋转助手
     * @type {{AXIS: (Line|number[]|string)[][]}}
     */
    const helperRotate = {
      AXIS: [
        [new Line(lineGeometry, matHelper.clone()), [-1e3, 0, 0], null, [1e6, 1, 1], 'helper'],
      ],
    }

    /**
     * @type {{
     * E: Mesh[][],
     * XYZE: Mesh[][],
     * X: (Mesh|number[])[][],
     * Y: (Mesh|number[])[][],
     * Z: (Mesh|number[])[][]
     * }}
     */
    const pickerRotate = {
      X: [
        [new Mesh(new TorusGeometry(1, 0.1, 4, 24), matInvisible), [0, 0, 0], [0, -Math.PI / 2, -Math.PI / 2]],
      ],
      Y: [
        [new Mesh(new TorusGeometry(1, 0.1, 4, 24), matInvisible), [0, 0, 0], [Math.PI / 2, 0, 0]],
      ],
      Z: [
        [new Mesh(new TorusGeometry(1, 0.1, 4, 24), matInvisible), [0, 0, 0], [0, 0, -Math.PI / 2]],
      ],
      E: [
        [new Mesh(new TorusGeometry(1.25, 0.1, 2, 24), matInvisible)],
      ],
      XYZE: [
        [new Mesh(new SphereGeometry(0.7, 10, 8), matInvisible)],
      ],
    }

    /**
     * 缩放配件
     * @type {{
     * XY: ((Mesh|number[])[]|(Line|number[])[])[],
     * YZ: ((Mesh|number[])[]|(Line|number[])[])[],
     * XZ: ((Mesh|number[])[]|(Line|number[])[])[],
     * XYZX: (Mesh|number[])[][],
     * X: ((Mesh|number[])[]|(Line|number[])[])[],
     * XYZZ: (Mesh|number[])[][],
     * Y: ((Mesh|number[])[]|(Line|number[])[])[],
     * XYZY: (Mesh|number[])[][],
     * Z: ((Mesh|number[])[]|(Line|number[])[])[]
     * }}
     */
    const gizmoScale = {
      X: [
        [new Mesh(scaleHandleGeometry, matRed), [0.8, 0, 0], [0, 0, -Math.PI / 2]],
        [new Line(lineGeometry, matLineRed), null, null, [0.8, 1, 1]],
      ],
      Y: [
        [new Mesh(scaleHandleGeometry, matGreen), [0, 0.8, 0]],
        [new Line(lineGeometry, matLineGreen), null, [0, 0, Math.PI / 2], [0.8, 1, 1]],
      ],
      Z: [
        [new Mesh(scaleHandleGeometry, matBlue), [0, 0, 0.8], [Math.PI / 2, 0, 0]],
        [new Line(lineGeometry, matLineBlue), null, [0, -Math.PI / 2, 0], [0.8, 1, 1]],
      ],
      XY: [
        [new Mesh(scaleHandleGeometry, matYellowTransparent), [0.85, 0.85, 0], null, [2, 2, 0.2]],
        [new Line(lineGeometry, matLineYellow), [0.855, 0.98, 0], null, [0.125, 1, 1]],
        [new Line(lineGeometry, matLineYellow), [0.98, 0.855, 0], [0, 0, Math.PI / 2], [0.125, 1, 1]],
      ],
      YZ: [
        [new Mesh(scaleHandleGeometry, matCyanTransparent), [0, 0.85, 0.85], null, [0.2, 2, 2]],
        [new Line(lineGeometry, matLineCyan), [0, 0.855, 0.98], [0, 0, Math.PI / 2], [0.125, 1, 1]],
        [new Line(lineGeometry, matLineCyan), [0, 0.98, 0.855], [0, -Math.PI / 2, 0], [0.125, 1, 1]],
      ],
      XZ: [
        [new Mesh(scaleHandleGeometry, matMagentaTransparent), [0.85, 0, 0.85], null, [2, 0.2, 2]],
        [new Line(lineGeometry, matLineMagenta), [0.855, 0, 0.98], null, [0.125, 1, 1]],
        [new Line(lineGeometry, matLineMagenta), [0.98, 0, 0.855], [0, -Math.PI / 2, 0], [0.125, 1, 1]],
      ],
      XYZX: [
        [new Mesh(new BoxGeometry(0.125, 0.125, 0.125), matWhiteTransperent), [1.1, 0, 0]],
      ],
      XYZY: [
        [new Mesh(new BoxGeometry(0.125, 0.125, 0.125), matWhiteTransperent), [0, 1.1, 0]],
      ],
      XYZZ: [
        [new Mesh(new BoxGeometry(0.125, 0.125, 0.125), matWhiteTransperent), [0, 0, 1.1]],
      ],
    }

    /**
     * 缩放拾取
     * @type {{
     * XY: (Mesh|number[])[][],
     * YZ: (Mesh|number[])[][],
     * XZ: (Mesh|number[])[][],
     * XYZX: (Mesh|number[])[][],
     * X: (Mesh|number[])[][],
     * XYZZ: (Mesh|number[])[][],
     * Y: (Mesh|number[])[][],
     * XYZY: (Mesh|number[])[][],
     * Z: (Mesh|number[])[][]
     * }}
     */
    const pickerScale = {
      X: [
        [new Mesh(new CylinderGeometry(0.2, 0, 0.8, 4, 1, false), matInvisible), [0.5, 0, 0], [0, 0, -Math.PI / 2]],
      ],
      Y: [
        [new Mesh(new CylinderGeometry(0.2, 0, 0.8, 4, 1, false), matInvisible), [0, 0.5, 0]],
      ],
      Z: [
        [new Mesh(new CylinderGeometry(0.2, 0, 0.8, 4, 1, false), matInvisible), [0, 0, 0.5], [Math.PI / 2, 0, 0]],
      ],
      XY: [
        [new Mesh(scaleHandleGeometry, matInvisible), [0.85, 0.85, 0], null, [3, 3, 0.2]],
      ],
      YZ: [
        [new Mesh(scaleHandleGeometry, matInvisible), [0, 0.85, 0.85], null, [0.2, 3, 3]],
      ],
      XZ: [
        [new Mesh(scaleHandleGeometry, matInvisible), [0.85, 0, 0.85], null, [3, 0.2, 3]],
      ],
      XYZX: [
        [new Mesh(new BoxGeometry(0.2, 0.2, 0.2), matInvisible), [1.1, 0, 0]],
      ],
      XYZY: [
        [new Mesh(new BoxGeometry(0.2, 0.2, 0.2), matInvisible), [0, 1.1, 0]],
      ],
      XYZZ: [
        [new Mesh(new BoxGeometry(0.2, 0.2, 0.2), matInvisible), [0, 0, 1.1]],
      ],
    }

    /**
     * 缩放助手
     * @type {{
     * X: (Line|number[]|string)[][],
     * Y: (Line|number[]|string)[][],
     * Z: (Line|number[]|string)[][]
     * }}
     */
    const helperScale = {
      X: [
        [new Line(lineGeometry, matHelper.clone()), [-1e3, 0, 0], null, [1e6, 1, 1], 'helper'],
      ],
      Y: [
        [new Line(lineGeometry, matHelper.clone()), [0, -1e3, 0], [0, 0, Math.PI / 2], [1e6, 1, 1], 'helper'],
      ],
      Z: [
        [new Line(lineGeometry, matHelper.clone()), [0, 0, -1e3], [0, -Math.PI / 2, 0], [1e6, 1, 1], 'helper'],
      ],
    }

    // Creates an Object3D with gizmos described in custom hierarchy definition.
    const setupGizmo = function (gizmoMap) {
      const gizmo = new Object3D()

      for (const name in gizmoMap) {
        for (let i = gizmoMap[name].length; i--;) {
          const object = gizmoMap[name][i][0].clone()
          const position = gizmoMap[name][i][1]
          const rotation = gizmoMap[name][i][2]
          const scale = gizmoMap[name][i][3]
          const tag = gizmoMap[name][i][4]

          // name and tag properties are essential for picking and updating logic.
          object.name = name
          object.tag = tag

          if (position) {
            object.position.set(position[0], position[1], position[2])
          }
          if (rotation) {
            object.rotation.set(rotation[0], rotation[1], rotation[2])
          }
          if (scale) {
            object.scale.set(scale[0], scale[1], scale[2])
          }

          object.updateMatrix()

          const tempGeometry = object.geometry.clone()
          tempGeometry.applyMatrix4(object.matrix)
          object.geometry = tempGeometry

          object.position.set(0, 0, 0)
          object.rotation.set(0, 0, 0)
          object.scale.set(1, 1, 1)

          gizmo.add(object)
        }
      }

      return gizmo
    }

    // Reusable utility variables
    const tempVector = new Vector3(0, 0, 0)
    const tempEuler = new Euler()
    const alignVector = new Vector3(0, 1, 0)
    const zeroVector = new Vector3(0, 0, 0)
    const lookAtMatrix = new Matrix4()
    const tempQuaternion = new Quaternion()
    const tempQuaternion2 = new Quaternion()
    const identityQuaternion = new Quaternion()

    const unitX = new Vector3(1, 0, 0)
    const unitY = new Vector3(0, 1, 0)
    const unitZ = new Vector3(0, 0, 1)

    // Gizmo creation
    this.gizmo = {}
    this.picker = {}
    this.helper = {}

    this.add(this.gizmo.translate = setupGizmo(gizmoTranslate))
    this.add(this.gizmo.rotate = setupGizmo(gizmoRotate))
    this.add(this.gizmo.scale = setupGizmo(gizmoScale))
    this.add(this.picker.translate = setupGizmo(pickerTranslate))
    this.add(this.picker.rotate = setupGizmo(pickerRotate))
    this.add(this.picker.scale = setupGizmo(pickerScale))
    this.add(this.helper.translate = setupGizmo(helperTranslate))
    this.add(this.helper.rotate = setupGizmo(helperRotate))
    this.add(this.helper.scale = setupGizmo(helperScale))

    // Pickers should be hidden always

    this.picker.translate.visible = false
    this.picker.rotate.visible = false
    this.picker.scale.visible = false

    // updateMatrixWorld will update transformations and appearance of individual handles
    this.updateMatrixWorld = function () {
      let space = this.space

      if (this.mode === 'scale') space = 'local' // scale always oriented to local rotation

      const quaternion = space === 'local' ? this.worldQuaternion : identityQuaternion

      // Show only gizmos for current transform mode
      this.gizmo.translate.visible = this.mode === 'translate'
      this.gizmo.rotate.visible = this.mode === 'rotate'
      this.gizmo.scale.visible = this.mode === 'scale'

      this.helper.translate.visible = this.mode === 'translate'
      this.helper.rotate.visible = this.mode === 'rotate'
      this.helper.scale.visible = this.mode === 'scale'

      let handles = []
      handles = handles.concat(this.picker[this.mode].children)
      handles = handles.concat(this.gizmo[this.mode].children)
      handles = handles.concat(this.helper[this.mode].children)

      let eyeDistance = this.worldPosition.distanceTo(this.cameraPosition)
      if (this.camera instanceof OrthographicCamera) {
        const range = this.camera.right - this.camera.left
        eyeDistance = range
      }

      for (let i = 0; i < handles.length; i++) {
        const handle = handles[i]

        // hide aligned to camera
        handle.visible = true
        handle.rotation.set(0, 0, 0)
        handle.position.copy(this.worldPosition)

        // var eyeDistance = this.worldPosition.distanceTo(this.cameraPosition)
        handle.scale.set(1, 1, 1).multiplyScalar(eyeDistance * this.size / 7)

        // TODO: simplify helpers and consider decoupling from gizmo

        if (handle.tag === 'helper') {
          handle.visible = false

          if (handle.name === 'AXIS') {
            handle.position.copy(this.worldPositionStart)
            handle.visible = !!this.axis

            if (this.axis === 'X') {
              tempQuaternion.setFromEuler(tempEuler.set(0, 0, 0))
              handle.quaternion.copy(quaternion).multiply(tempQuaternion)

              if (Math.abs(alignVector.copy(unitX).applyQuaternion(quaternion).dot(this.eye)) > 0.9) {
                handle.visible = false
              }
            }

            if (this.axis === 'Y') {
              tempQuaternion.setFromEuler(tempEuler.set(0, 0, Math.PI / 2))
              handle.quaternion.copy(quaternion).multiply(tempQuaternion)

              if (Math.abs(alignVector.copy(unitY).applyQuaternion(quaternion).dot(this.eye)) > 0.9) {
                handle.visible = false
              }
            }

            if (this.axis === 'Z') {
              tempQuaternion.setFromEuler(tempEuler.set(0, Math.PI / 2, 0))
              handle.quaternion.copy(quaternion).multiply(tempQuaternion)

              if (Math.abs(alignVector.copy(unitZ).applyQuaternion(quaternion).dot(this.eye)) > 0.9) {
                handle.visible = false
              }
            }

            if (this.axis === 'XYZE') {
              tempQuaternion.setFromEuler(tempEuler.set(0, Math.PI / 2, 0))
              alignVector.copy(this.rotationAxis)
              handle.quaternion.setFromRotationMatrix(lookAtMatrix.lookAt(zeroVector, alignVector, unitY))
              handle.quaternion.multiply(tempQuaternion)
              handle.visible = this.dragging
            }

            if (this.axis === 'E') {
              handle.visible = false
            }
          } else if (handle.name === 'START') {
            handle.position.copy(this.worldPositionStart)
            handle.visible = this.dragging
          } else if (handle.name === 'END') {
            handle.position.copy(this.worldPosition)
            handle.visible = this.dragging
          } else if (handle.name === 'DELTA') {
            handle.position.copy(this.worldPositionStart)
            handle.quaternion.copy(this.worldQuaternionStart)
            tempVector.set(1e-10, 1e-10, 1e-10).add(this.worldPositionStart).sub(this.worldPosition).multiplyScalar(-1)
            tempVector.applyQuaternion(this.worldQuaternionStart.clone().invert())
            handle.scale.copy(tempVector)
            handle.visible = this.dragging
          } else {
            handle.quaternion.copy(quaternion)

            if (this.dragging) {
              handle.position.copy(this.worldPositionStart)
            } else {
              handle.position.copy(this.worldPosition)
            }

            if (this.axis) {
              handle.visible = this.axis.search(handle.name) !== -1
            }
          }

          // If updating helper, skip rest of the loop
          continue
        }

        // Align handles to current local or world rotation
        handle.quaternion.copy(quaternion)

        const AXIS_HIDE_TRESHOLD = 0.99
        const PLANE_HIDE_TRESHOLD = 0.2
        const AXIS_FLIP_TRESHOLD = -0.4

        if (this.mode === 'translate' || this.mode === 'scale') {
          // Hide translate and scale axis facing the camera
          if (!(this.camera instanceof OrthographicCamera)) {
            if (handle.name === 'X' || handle.name === 'XYZX') {
              if (Math.abs(alignVector.copy(unitX).applyQuaternion(quaternion).dot(this.eye)) > AXIS_HIDE_TRESHOLD) {
                handle.scale.set(1e-10, 1e-10, 1e-10)
                handle.visible = false
              }
            }
            if (handle.name === 'Y' || handle.name === 'XYZY') {
              if (Math.abs(alignVector.copy(unitY).applyQuaternion(quaternion).dot(this.eye)) > AXIS_HIDE_TRESHOLD) {
                handle.scale.set(1e-10, 1e-10, 1e-10)
                handle.visible = false
              }
            }
            if (handle.name === 'Z' || handle.name === 'XYZZ') {
              if (Math.abs(alignVector.copy(unitZ).applyQuaternion(quaternion).dot(this.eye)) > AXIS_HIDE_TRESHOLD) {
                handle.scale.set(1e-10, 1e-10, 1e-10)
                handle.visible = false
              }
            }
            if (handle.name === 'XY') {
              if (Math.abs(alignVector.copy(unitZ).applyQuaternion(quaternion).dot(this.eye)) < PLANE_HIDE_TRESHOLD) {
                handle.scale.set(1e-10, 1e-10, 1e-10)
                handle.visible = false
              }
            }
            if (handle.name === 'YZ') {
              if (Math.abs(alignVector.copy(unitX).applyQuaternion(quaternion).dot(this.eye)) < PLANE_HIDE_TRESHOLD) {
                handle.scale.set(1e-10, 1e-10, 1e-10)
                handle.visible = false
              }
            }

            if (handle.name === 'XZ') {
              if (Math.abs(alignVector.copy(unitY).applyQuaternion(quaternion).dot(this.eye)) < PLANE_HIDE_TRESHOLD) {
                handle.scale.set(1e-10, 1e-10, 1e-10)
                handle.visible = false
                // console.log("xz set visible false")
              }
            }
          }

          // Flip translate and scale axis ocluded behind another axis

          if (handle.name.search('X') !== -1) {
            if (alignVector.copy(unitX).applyQuaternion(quaternion).dot(this.eye) < AXIS_FLIP_TRESHOLD) {
              if (handle.tag === 'fwd') {
                handle.visible = false
              } else {
                handle.scale.x *= -1
              }
            } else if (handle.tag === 'bwd') {
              handle.visible = false
            }
          }

          if (handle.name.search('Y') !== -1) {
            if (alignVector.copy(unitY).applyQuaternion(quaternion).dot(this.eye) < AXIS_FLIP_TRESHOLD) {
              if (handle.tag === 'fwd') {
                handle.visible = false
              } else {
                handle.scale.y *= -1
              }
            } else if (handle.tag === 'bwd') {
              handle.visible = false
            }
          }

          if (handle.name.search('Z') !== -1) {
            if (alignVector.copy(unitZ).applyQuaternion(quaternion).dot(this.eye) < AXIS_FLIP_TRESHOLD) {
              if (handle.tag === 'fwd') {
                handle.visible = false
              } else {
                handle.scale.z *= -1
              }
            } else if (handle.tag === 'bwd') {
              handle.visible = false
            }
          }
        } else if (this.mode === 'rotate') {
          // Align handles to current local or world rotation

          tempQuaternion2.copy(quaternion)
          alignVector.copy(this.eye).applyQuaternion(tempQuaternion.copy(quaternion).invert())

          if (handle.name.search('E') !== -1) {
            handle.quaternion.setFromRotationMatrix(lookAtMatrix.lookAt(this.eye, zeroVector, unitY))
          }

          if (handle.name === 'X') {
            tempQuaternion.setFromAxisAngle(unitX, Math.atan2(-alignVector.y, alignVector.z))
            tempQuaternion.multiplyQuaternions(tempQuaternion2, tempQuaternion)
            handle.quaternion.copy(tempQuaternion)
          }

          if (handle.name === 'Y') {
            tempQuaternion.setFromAxisAngle(unitY, Math.atan2(alignVector.x, alignVector.z))
            tempQuaternion.multiplyQuaternions(tempQuaternion2, tempQuaternion)
            handle.quaternion.copy(tempQuaternion)
          }

          if (handle.name === 'Z') {
            tempQuaternion.setFromAxisAngle(unitZ, Math.atan2(alignVector.y, alignVector.x))
            tempQuaternion.multiplyQuaternions(tempQuaternion2, tempQuaternion)
            handle.quaternion.copy(tempQuaternion)
          }
        }

        // Hide disabled axes
        handle.visible = handle.visible && (!handle.name.includes('X') || this.showX)
        handle.visible = handle.visible && (!handle.name.includes('Y') || this.showY)
        handle.visible = handle.visible && (!handle.name.includes('Z') || this.showZ)
        handle.visible = handle.visible && (!handle.name.includes('E') || (this.showX && this.showY && this.showZ))

        // highlight selected axis

        handle.material._opacity = handle.material._opacity || handle.material.opacity
        handle.material._color = handle.material._color || handle.material.color.clone()

        handle.material.color.copy(handle.material._color)
        handle.material.opacity = handle.material._opacity

        if (!this.enabled) {
          handle.material.opacity *= 0.5
          handle.material.color.lerp(new Color(1, 1, 1), 0.5)
        } else if (this.axis) {
          if (handle.name === this.axis) {
            handle.material.opacity = 1.0
            handle.material.color.lerp(new Color(1, 1, 1), 0.5)
          } else if (this.axis.split('').includes(handle.name)) {
            handle.material.opacity = 1.0
            handle.material.color.lerp(new Color(1, 1, 1), 0.5)
          } else {
            handle.material.opacity *= 0.25
            handle.material.color.lerp(new Color(1, 1, 1), 0.5)
          }
        }
      }

      Object3D.prototype.updateMatrixWorld.call(this)
    }
  }
  isTransformControlsGizmo = true
}

/**
 * 拖动时做激光拾取定位的平面
 * @constructor
 */
class TransformControlsPlane extends Mesh {
  constructor() {
    'use strict'
    super(
      new PlaneGeometry(10000, 10000, 2, 2),
      new MeshBasicMaterial({ visible: false, wireframe: true, side: DoubleSide, transparent: true, opacity: 0.1 }),
    )
    this.type = 'TransformControlsPlane'

    const unitX = new Vector3(1, 0, 0)
    const unitY = new Vector3(0, 1, 0)
    const unitZ = new Vector3(0, 0, 1)

    const tempVector = new Vector3()
    const dirVector = new Vector3()
    const alignVector = new Vector3()
    const tempMatrix = new Matrix4()
    const identityQuaternion = new Quaternion()

    this.updateMatrixWorld = function () {
      let space = this.space

      this.position.copy(this.worldPosition)

      if (this.mode === 'scale') space = 'local' // scale always oriented to local rotation

      unitX.set(1, 0, 0).applyQuaternion(space === 'local' ? this.worldQuaternion : identityQuaternion)
      unitY.set(0, 1, 0).applyQuaternion(space === 'local' ? this.worldQuaternion : identityQuaternion)
      unitZ.set(0, 0, 1).applyQuaternion(space === 'local' ? this.worldQuaternion : identityQuaternion)

      // Align the plane for current transform mode, axis and space.

      alignVector.copy(unitY)

      switch (this.mode) {
        case 'translate':
        case 'scale':
          switch (this.axis) {
            case 'X':
              alignVector.copy(this.eye).cross(unitX)
              dirVector.copy(unitX).cross(alignVector)
              break
            case 'Y':
              alignVector.copy(this.eye).cross(unitY)
              dirVector.copy(unitY).cross(alignVector)
              break
            case 'Z':
              alignVector.copy(this.eye).cross(unitZ)
              dirVector.copy(unitZ).cross(alignVector)
              break
            case 'XY':
              dirVector.copy(unitZ)
              break
            case 'YZ':
              dirVector.copy(unitX)
              break
            case 'XZ':
              alignVector.copy(unitZ)
              dirVector.copy(unitY)
              break
            case 'XYZ':
            case 'E':
              dirVector.set(0, 0, 0)
              break
          }
          break
        case 'rotate':
        default:
          // special case for rotate
          dirVector.set(0, 0, 0)
      }
      if (dirVector.length() === 0) {
        // If in rotate mode, make the plane parallel to camera
        this.quaternion.copy(this.cameraQuaternion)
      } else {
        tempMatrix.lookAt(tempVector.set(0, 0, 0), dirVector, alignVector)
        this.quaternion.setFromRotationMatrix(tempMatrix)
      }
      Object3D.prototype.updateMatrixWorld.call(this)
    }
  }
  isTransformControlsPlane = true
}


export { TransformControls }
