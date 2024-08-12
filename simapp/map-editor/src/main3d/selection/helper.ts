import type { WebGLRenderer } from 'three'
import { Vector2 } from 'three'

class SelectionHelper {
  startPoint: Vector2
  pointTopLeft: Vector2
  pointBottomRight: Vector2
  element: HTMLElement
  renderer: WebGLRenderer
  constructor (params: { renderer: WebGLRenderer }) {
    this.element = document.createElement('div')
    this.element.style.cssText = `
  border: 1px solid #55aaff;
  background-color: rgba(75, 160, 255, 0.3);
  position: absolute;
  `
    this.element.style.pointerEvents = 'none'

    this.renderer = params.renderer

    this.startPoint = new Vector2()
    this.pointTopLeft = new Vector2()
    this.pointBottomRight = new Vector2()
  }

  onSelectStart = (coordinate: common.vec2) => {
    if (!this.renderer.domElement.parentElement) return
    this.renderer.domElement.parentElement.appendChild(this.element)

    this.element.style.left = `${coordinate.x}px`
    this.element.style.top = `${coordinate.y}px`
    this.element.style.width = '0px'
    this.element.style.height = '0px'

    this.startPoint.x = coordinate.x
    this.startPoint.y = coordinate.y
  }

  onSelectMove = (coordinate: common.vec2) => {
    this.pointBottomRight.x = Math.max(this.startPoint.x, coordinate.x)
    this.pointBottomRight.y = Math.max(this.startPoint.y, coordinate.y)
    this.pointTopLeft.x = Math.min(this.startPoint.x, coordinate.x)
    this.pointTopLeft.y = Math.min(this.startPoint.y, coordinate.y)

    this.element.style.left = `${this.pointTopLeft.x}px`
    this.element.style.top = `${this.pointTopLeft.y}px`
    this.element.style.width =
      `${this.pointBottomRight.x - this.pointTopLeft.x}px`
    this.element.style.height =
      `${this.pointBottomRight.y - this.pointTopLeft.y}px`
  }

  onSelectEnd = () => {
    if (!this.element.parentElement) return
    this.element.parentElement.removeChild(this.element)
  }

  dispose () {}
}

export default SelectionHelper
