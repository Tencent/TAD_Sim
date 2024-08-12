import {
  Application,
  Container,
  type FederatedPointerEvent,
  Graphics,
  Text,
  utils,
} from 'pixi.js'
import { CatmullRomCurve3 } from 'three'
import { cloneDeep } from 'lodash'
import {
  CurveType,
  fixedPrecision,
  transformVec3ByObject,
} from '@/utils/common3d'
import { Constant } from '@/utils/business'

const gridLabelFontStyle = {
  fill: 0xB9B9B9,
  fontSize: 12,
}

let isInit = false
let dragTarget: null | Container | Graphics = null
const xGridOffset = 20
const maxZoom = 20
const minZoom = 0
const zoomSpeed = 0.95
// 点击新增控制点，高度的误差范围内，可以视为新增控制点成功
const deviation = 5

class Root2D extends utils.EventEmitter {
  app: Application
  parentDom: HTMLElement
  canvas: HTMLCanvasElement
  width: number
  height: number
  grid: Container
  container: Container
  _zoom: number
  controlPoints: Array<common.vec3>
  roadId: string // 当前选中的道路 id
  index: number // 当前选中的控制点在数组中的索引
  curvePath: biz.ICurve3 | null
  constructor () {
    super()
  }

  init (params: { container: HTMLElement, width: number, height: number }) {
    if (isInit) return
    const { container: parentDom, width, height } = params

    const canvas = document.createElement('canvas')
    canvas.width = width
    canvas.height = height
    parentDom.appendChild(canvas)

    this.app = new Application({
      width,
      height,
      backgroundColor: 0x111111,
      view: canvas,
      antialias: true,
    })
    this.app.resizeTo = parentDom
    this.app.stage.eventMode = 'dynamic'
    this.app.stage.hitArea = this.app.screen
    this.parentDom = parentDom
    this.canvas = canvas
    this.width = width
    this.height = height
    this._zoom = 1
    this.roadId = ''
    this.curvePath = null
    this.index = -1

    isInit = true

    // 当前的控制点
    this.controlPoints = []

    // 初始化辅助元素
    this.initElement()

    // 更新元素
    this.update(true)

    // 初始化监听器
    this.initListeners()

    this.app.render()
  }

  resize (width: number, height: number) {
    if (!this.app) return
    if (!this.container) return

    this.width = width
    this.height = height
    setTimeout(() => {
      this.container.y = this.height / 2
      this.app.resize()
      this.update(true)
    }, 0)
  }

  set zoom (value: number) {
    const { container } = this
    const _zoom = Math.min(Math.max(value, minZoom), maxZoom)
    if (_zoom !== this._zoom) {
      this._zoom = _zoom
      container.scale.x = _zoom
      container.scale.y = _zoom
      this.update(true)
    }
  }

  get zoom () {
    return this._zoom
  }

  updateCurve (points: Array<common.vec3>) {
    if (points.length < 2) return

    // 先清空内部的子元素
    this.container.children.forEach((child) => {
      // 取消监听
      if (child.name === 'curve') {
        child.destroy({
          children: true,
          texture: true,
          baseTexture: true,
        })
      }
    })

    // 基于所有的高程控制点，创建一条曲线，然后绘制采样点形成高程曲线
    const curve = new CatmullRomCurve3(transformVec3ByObject(points))
    curve.curveType = CurveType
    const deltaPercent = 0.01
    const line = new Graphics()
    line.name = 'curve'
    line.lineStyle(1 / this.zoom, 0xF31616)
    for (let i = 0; i <= 1; i += deltaPercent) {
      const p = curve.getPointAt(i)
      const _x = p.x
      const _y = -p.y
      if (i === 0) {
        line.moveTo(_x, _y)
      } else {
        line.lineTo(_x, _y)
      }
    }

    this.curvePath = curve

    this.container.addChild(line)
  }

  // 删除选中的高程控制点
  removeControlPoint () {
    if (!this.roadId) return
    if (this.index === -1 || this.controlPoints.length < 2) return
    // 如果选中的是首尾的控制点，则不允许删除
    if (this.index === 0 || this.index === this.controlPoints.length - 1) return

    this.emit('remove', this.index)
    this.index = -1
  }

  /**
   * 基于控制点更新绘制的高程曲线和控制点
   * @param points
   * @param isSync 是否需要同步更新缓存的控制点数据
   */
  updateControlPoints (params: {
    roadId: string
    points: Array<common.vec3>
    isSync: boolean
  }) {
    const { roadId, points, isSync = false } = params
    if (!points || points.length < 2) return
    if (isSync && this.roadId !== roadId) {
      // 如果是需要从点击的道路高程数据中同步控制点，则需要更新视角缩放
      this.zoom = 1
      // 重置选中的控制点索引
      this.index = -1

      // 基于当前控制点的范围，调整初始的缩放
      const { width: screenWidth } = this.app.screen
      const roadLength = points[points.length - 1].x

      const ratio = screenWidth / roadLength

      if (ratio > 3) {
        this.zoom = 3
      } else {
        this.zoom = ratio - 0.2
      }
    }

    this.disposeContainer()

    this.updateCurve(points)

    // 绘制控制点
    points.forEach((p, index) => {
      const circlePoint = createCirclePoint()
      const _x = p.x
      const _y = -p.y
      circlePoint.position.set(_x, _y)
      circlePoint.scale.set(1 / this.zoom, 1 / this.zoom)
      // 如果控制点的索引存在，则高亮当前的控制点
      if (this.index === index) {
        circlePoint.setToActive()
      }
      this.container.addChild(circlePoint)

      circlePoint.on('pointerdown', this.onDragStart, circlePoint)
      circlePoint.on('pointerover', this.onHoverOn, circlePoint)
      circlePoint.on('pointerout', this.onHoverOff, circlePoint)
    })

    this.roadId = roadId
    if (isSync) {
      this.controlPoints = points
    }
  }

  onHoverOn () {
    // @ts-igonre
    const point = this as Graphics
    point.setToActive()
  }

  onHoverOff () {
    // @ts-igonre
    const point = this as Graphics
    point.setToNormal()
    // 如果是选中拖拽过程中
    if (dragTarget) {
      if (dragTarget === point) {
        point.setToActive()
      } else {
        point.setToNormal()
      }
    } else {
      // 非拖拽过程
      if (root2dInstance.index > -1) {
        // 有选中的控制点索引
        const targetPoint = root2dInstance.controlPoints[root2dInstance.index]

        // 通过 x 坐标（即s轴的坐标）判定选中的目标控制点
        if (
          targetPoint &&
          fixedPrecision(targetPoint.x, 3) === fixedPrecision(point.x, 3)
        ) {
          point.setToActive()
        } else {
          point.setToNormal()
        }
      } else {
        // 没有选中的控制点索引
        point.setToNormal()
      }
    }
  }

  onDragStart () {
    // @ts-expect-error
    dragTarget = this as Graphics
    root2dInstance.app.stage.on('pointermove', root2dInstance.onDragMove)

    // 先重绘所有控制点的颜色
    root2dInstance.container.children.forEach((child) => {
      if (child.name === 'point') {
        child.setToNormal()
      }
    })
    // 再高亮选中的控制点颜色
    dragTarget.setToActive()

    // 更新当前选中控制点
    const _x = dragTarget.x
    const _y = -dragTarget.y
    root2dInstance.emit('select', {
      x: _x,
      y: _y,
    })

    // 通过 x 坐标来判断移动的是哪一个控制点
    let index = -1
    const points = root2dInstance.controlPoints
    for (let i = 0; i < points.length; i++) {
      if (index > -1) continue
      const p = points[i]
      if (Number(_x).toFixed(3) === Number(p.x).toFixed(3)) {
        // 如果 x 坐标一致
        index = i
      }
    }
    if (index > -1) {
      root2dInstance.index = index
    }
  }

  // 拖拽高程控制点
  onDragMove = (event: FederatedPointerEvent) => {
    event.preventDefault()
    if (!dragTarget) return
    if (root2dInstance.index < 0) return

    const newPosition = event.getLocalPosition(dragTarget.parent)
    // 只允许修改纵向高度
    dragTarget.position.set(dragTarget.position.x, newPosition.y)

    const newPoints = cloneDeep(root2dInstance.controlPoints)
    newPoints[root2dInstance.index].y = -newPosition.y

    this.updateCurve(newPoints)
  }

  onDragEnd () {
    const { app } = root2dInstance
    app.stage.off('pointermove', this.onDragMove)
    if (!dragTarget) return

    // 触发数据层道路高程控制点的更新
    const { x, y } = dragTarget.position
    // 将二维交互的高程控制点在有效范围内
    const newHeight = Math.max(
      Constant.minElevation,
      Math.min(-y, Constant.maxElevation),
    )
    root2dInstance.emit('update', {
      x,
      y: newHeight,
    })

    dragTarget = null
  }

  initElement () {
    // 纵横的网格参考线相关
    this.grid = new Container()
    this.grid.name = 'grid'

    const axisX = new Graphics()
    axisX.name = 'axisX'

    const axisY = new Graphics()
    axisY.name = 'axisY'

    const gridX = new Graphics()
    gridX.name = 'gridX'

    const gridY = new Graphics()
    gridY.name = 'gridY'

    const labelX = new Container()
    labelX.name = 'labelX'
    const labelY = new Container()
    labelY.name = 'labelY'

    this.grid.addChild(axisX, axisY, gridX, gridY, labelX, labelY)

    // 控制点的容器
    this.container = new Container()
    this.container.y = this.height / 2
    this.container.x = xGridOffset
    this.container.scale.set(1, 1)
    this.container.sortableChildren = true

    this.app.stage.addChild(this.grid, this.container)
  }

  // 更新辅助网格线
  updateGrid () {
    const { zoom, grid, width, height } = this
    let step = 40
    let stickStep = step * zoom

    while (stickStep < 40 * 0.6667) {
      step *= 2
      stickStep = step * zoom
    }

    while (stickStep > 40 * 1.5) {
      step /= 2
      stickStep = step * zoom
    }
    const axisX = grid.getChildByName('axisX') as Graphics
    axisX.clear()
    axisX.lineStyle(1, 0x777777)
    axisX.moveTo(0, height / 2)
    axisX.lineTo(width, height / 2)

    const axisY = grid.getChildByName('axisY') as Graphics
    axisY.clear()
    axisY.lineStyle(1, 0x777777)
    axisY.moveTo(xGridOffset, 0)
    axisY.lineTo(xGridOffset, height)

    const gridX = grid.getChildByName('gridX') as Graphics
    gridX.clear()
    gridX.lineStyle(1, 0x383838)
    for (
      let i = xGridOffset + stickStep;
      i <= xGridOffset + width;
      i += stickStep
    ) {
      gridX.moveTo(i, 0)
      gridX.lineTo(i, height)
    }

    const gridY = grid.getChildByName('gridY') as Graphics
    gridY.clear()
    gridY.lineStyle(1, 0x383838)
    for (let i = height / 2 + stickStep; i <= height; i += stickStep) {
      gridY.moveTo(0, i)
      gridY.lineTo(width, i)
    }
    for (let i = height / 2 - stickStep; i >= 0; i -= stickStep) {
      gridY.moveTo(0, i)
      gridY.lineTo(width, i)
    }

    const labelX = grid.getChildByName('labelX') as Container
    labelX.children.forEach((c) => {
      c.destroy({ children: true, texture: true, baseTexture: true })
    })
    labelX.removeChildren()
    for (let i = 1; i < (xGridOffset + width) / stickStep; i++) {
      const text = new Text(`${i * step}`, gridLabelFontStyle)
      text.anchor.set(0.5)
      text.x = i * stickStep + xGridOffset
      text.y = height / 2 + 20
      labelX.addChild(text)
    }

    const yLabels = grid.getChildByName('labelY') as Container
    yLabels.children.forEach((c) => {
      c.destroy({ children: true, texture: true, baseTexture: true })
    })
    yLabels.removeChildren()
    for (let i = 1; i < height / 2 / stickStep; i++) {
      const textPositive = new Text(`${i * step}`, gridLabelFontStyle)
      textPositive.anchor.set(0.5)
      textPositive.x = xGridOffset / 2
      textPositive.y = height / 2 - i * stickStep
      yLabels.addChild(textPositive)
      const textNegative = new Text(`${-i * step}`, gridLabelFontStyle)
      textNegative.anchor.set(0.5)
      textNegative.x = xGridOffset / 2
      textNegative.y = height / 2 + i * stickStep
      yLabels.addChild(textNegative)
    }
  }

  /**
   * 更新元素
   * @param flag 是否更新网格的效果
   */
  update (flag: boolean = false) {
    // 使用已经同步过的控制点，来重新渲染
    this.updateControlPoints({
      roadId: this.roadId,
      points: this.controlPoints,
      isSync: false,
    })
    if (flag) {
      this.updateGrid()
    }
  }

  // 通过鼠标滚轮调节缩放
  onScroll = (event: WheelEvent) => {
    const { deltaX, deltaY, deltaZ } = event
    event.preventDefault()
    const delta = deltaX + deltaY + deltaZ
    if (delta < 0) {
      this.zoom /= zoomSpeed
    } else if (delta > 0) {
      this.zoom *= zoomSpeed
    }
  }

  onRightClick = (event: FederatedPointerEvent) => {
    event.preventDefault()
    const localPoint = this.container.toLocal(event.global)
    // 转换成高程的局部坐标
    const _x = localPoint.x
    const _y = -localPoint.y

    // 判断当前局部坐标点是否在 s 轴区间内
    const roadLength = this.controlPoints[this.controlPoints.length - 1].x
    if (localPoint.x >= roadLength) return

    // 判断跟道路曲线的映射高度差是否在误差范围内
    if (!this.curvePath) return
    const percent = Math.min(Math.max(0, _x / roadLength), 1)
    const closePoint = this.curvePath.getPointAt(percent)
    // 如果点击的高度跟曲线映射点的高度差在误差范围内，都视为有效
    if (Math.abs(_y - closePoint.y) > deviation) return

    // 判断是否有跟左右的控制点过于近
    let isTooClose = false
    let addIndex = -1
    for (let i = 0; i < this.controlPoints.length; i++) {
      const p = this.controlPoints[i]
      if (addIndex === -1 && _x < p.x) {
        addIndex = i
      }
      if (!isTooClose && Math.abs(p.x - _x) < 1) {
        isTooClose = true
      }
    }
    if (isTooClose) return

    // 触发新增控制点
    this.emit('add', {
      x: _x,
      y: _y,
    })

    // 更新当前选中控制点的索引
    if (addIndex !== -1) {
      this.index = addIndex
    }
  }

  disposeContainer () {
    if (!this.container || this.container.children.length < 1) return
    // 先清空内部的子元素
    this.container.children.forEach((child) => {
      // 取消监听
      if (child.name === 'point') {
        child.off('pointerdown', this.onDragStart)
        child.off('pointerover', this.onHoverOn)
        child.off('pointerout', this.onHoverOff)
      }
      child.destroy({
        children: true,
        texture: true,
        baseTexture: true,
      })
    })
    this.container.removeChildren()
  }

  clear () {
    this.index = -1
    this.roadId = ''
    this.curvePath = null
    if (this.controlPoints) {
      this.controlPoints.length = 0
    }
  }

  initListeners () {
    this.app.stage.addEventListener('pointerupoutside', this.onDragEnd)
    this.app.stage.addEventListener('pointerup', this.onDragEnd)
    this.app.stage.addEventListener('rightclick', this.onRightClick)
    this.canvas.addEventListener('wheel', this.onScroll)
    this.on('clear', this.clear)
  }

  disposeListeners () {
    if (this.app && this.app.stage) {
      this.app.stage.removeEventListener('pointerupoutside', this.onDragEnd)
      this.app.stage.removeEventListener('pointerup', this.onDragEnd)
      this.app.stage.removeEventListener('rightclick', this.onRightClick)
    }
    if (this.canvas) {
      this.canvas.removeEventListener('wheel', this.onScroll)
    }
    this.off('clear', this.clear)
  }

  dispose () {
    this.disposeListeners()
    this.clear()
    isInit = false

    // 由于销毁时 pixi 容器会出现白背景的闪烁，通过定时效果在 tab 切换后再执行销毁
    setTimeout(() => {
      this.disposeContainer()
      try {
        this.parentDom && this.parentDom.removeChild(this.canvas)
        this.app &&
        this.app.destroy(true, {
          children: true,
        })
      } catch (err) {}
    }, 50)
  }
}

function createCirclePoint () {
  const circlePoint = new Graphics()
  circlePoint.name = 'point'
  circlePoint.eventMode = 'static'
  circlePoint.cursor = 'pointer'
  circlePoint.zIndex = 1

  // 新增自定义方法
  circlePoint.setToActive = function () {
    circlePoint.clear()
    circlePoint.lineStyle(0)
    circlePoint.beginFill(0xF31616)
    circlePoint.drawCircle(0, 0, 4)
  }
  circlePoint.setToNormal = function () {
    circlePoint.clear()
    circlePoint.lineStyle(0)
    circlePoint.beginFill(0xAF7DFF)
    circlePoint.drawCircle(0, 0, 4)
  }
  circlePoint.setToNormal()
  return circlePoint
}

const root2dInstance = new Root2D()

export default root2dInstance
