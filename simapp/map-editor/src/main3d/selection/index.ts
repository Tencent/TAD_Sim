// 曲线道路编辑模式下，框选多个控制点的交互
import type {
  OrthographicCamera,
  WebGLRenderer,
} from 'three'
import {
  Frustum,
  Vector2,
  Vector3,
} from 'three'
import { cloneDeep } from 'lodash'
import SelectionHelper from './helper'
import ee, { CustomEventType } from '@/utils/event'

const frustum = new Frustum()
const vecNear = new Vector3()
const vecTopLeft = new Vector3()
const vecTopRight = new Vector3()
const vecDownRight = new Vector3()
const vecDownLeft = new Vector3()

const vecFarTopLeft = new Vector3()
const vecFarTopRight = new Vector3()
const vecFarDownRight = new Vector3()
const vecFarDownLeft = new Vector3()

const startPoint = new Vector3()
const endPoint = new Vector3()

const selected: Array<biz.IRefPoint> = []
let candidates: Array<biz.IRefPoint> = []
let isSelecting = false

class SelectionBox {
  helper: SelectionHelper
  renderer: WebGLRenderer
  camera: OrthographicCamera
  constructor () {}
  init (params: { camera: OrthographicCamera, renderer: WebGLRenderer }) {
    this.renderer = params.renderer
    this.camera = params.camera

    // 初始化html页面中的辅助元素
    this.helper = new SelectionHelper({
      renderer: params.renderer,
    })
  }

  onSelectStart (params: {
    coordinate: common.vec2
    points: Array<biz.IRefPoint>
  }) {
    const { coordinate, points = [] } = params
    // 开始选中状态
    isSelecting = true

    // 备选的道路参考线控制点集合
    candidates = points

    const { x, y } = this.renderer.getSize(new Vector2())
    const v = new Vector3(
      (coordinate.x / x) * 2 - 1,
      (-coordinate.y / y) * 2 + 1,
      0,
    )
    startPoint.copy(v)
    this.helper.onSelectStart(coordinate)
  }

  onSelectMove (coordinate: common.vec2) {
    if (!isSelecting) return

    const { x, y } = this.renderer.getSize(new Vector2())
    const v = new Vector3(
      (coordinate.x / x) * 2 - 1,
      (-coordinate.y / y) * 2 + 1,
      0,
    )
    endPoint.copy(v)
    this.helper.onSelectMove(coordinate)

    this.updateFrustum(startPoint, endPoint)
    selected.length = 0
    candidates.forEach((p) => {
      const point = new Vector3(p.x, p.y, p.z)
      if (frustum.containsPoint(point)) {
        selected.push(p)
      }
    })

    // 触发选中的控制点的 hover 更新
    ee.emit(CustomEventType.controlPoint.hover, selected)
  }

  onSelectEnd () {
    this.helper.onSelectEnd()
    if (isSelecting) {
      // 重置选中的状态
      const _selected = cloneDeep(selected)
      ee.emit(CustomEventType.controlPoint.select, _selected)
      isSelecting = false
    }

    // 清空状态
    selected.length = 0
    candidates.length = 0
  }

  // 更新 frustum 的区域
  updateFrustum (startPoint: Vector3, endPoint: Vector3) {
    const { planes } = frustum
    if (startPoint.x === endPoint.x) {
      endPoint.x += Number.EPSILON
    }

    if (startPoint.y === endPoint.y) {
      endPoint.y += Number.EPSILON
    }

    const left = Math.min(startPoint.x, endPoint.x)
    const top = Math.max(startPoint.y, endPoint.y)
    const right = Math.max(startPoint.x, endPoint.x)
    const down = Math.min(startPoint.y, endPoint.y)

    vecTopLeft.set(left, top, -1)
    vecTopRight.set(right, top, -1)
    vecDownRight.set(right, down, -1)
    vecDownLeft.set(left, down, -1)

    vecFarTopLeft.set(left, top, 1)
    vecFarTopRight.set(right, top, 1)
    vecFarDownRight.set(right, down, 1)
    vecFarDownLeft.set(left, down, 1)

    vecTopLeft.unproject(this.camera)
    vecTopRight.unproject(this.camera)
    vecDownRight.unproject(this.camera)
    vecDownLeft.unproject(this.camera)

    vecFarTopLeft.unproject(this.camera)
    vecFarTopRight.unproject(this.camera)
    vecFarDownRight.unproject(this.camera)
    vecFarDownLeft.unproject(this.camera)

    planes[0].setFromCoplanarPoints(vecTopLeft, vecFarTopLeft, vecFarTopRight)
    planes[1].setFromCoplanarPoints(
      vecTopRight,
      vecFarTopRight,
      vecFarDownRight,
    )
    planes[2].setFromCoplanarPoints(
      vecFarDownRight,
      vecFarDownLeft,
      vecDownLeft,
    )
    planes[3].setFromCoplanarPoints(vecFarDownLeft, vecFarTopLeft, vecTopLeft)
    planes[4].setFromCoplanarPoints(vecTopRight, vecDownRight, vecDownLeft)
    planes[5].setFromCoplanarPoints(
      vecFarDownRight,
      vecFarTopRight,
      vecFarTopLeft,
    )
    planes[5].normal.multiplyScalar(-1)
  }

  dispose () {}
}

export default new SelectionBox()
