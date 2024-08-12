import type { Object3D } from 'three'
import { GridHelper, Line } from 'three'

interface IGridParams {
  size?: number
  divisions?: number
  color?: number
}

class Grid {
  object: GridHelper
  constructor (params: IGridParams = {}) {
    const { size = 1000, divisions = 100, color = 0x999999 } = params
    this.object = new GridHelper(size, divisions, color, color)
  }

  setVisible (isVisible: boolean) {
    if (this.object.visible === isVisible) return
    this.object.visible = isVisible
  }

  dispose () {
    this.object.traverse((child: Object3D) => {
      if (child instanceof Line) {
        child.geometry.dispose()
        if (Array.isArray(child.material)) {
          child.material.forEach(mat => mat.dispose())
        } else {
          child.material.dispose()
        }
      }
    })
  }
}

export default Grid
