import { Line, Mesh } from 'three'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeMaterial } from '@/utils/common3d'
import { disposeHighlightRoadMask } from '@/main3d/render/road'

class EditElevationHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editElevationContainer')
  }

  onActivate () {}
  onDeactivate () {
    this.clear()
  }

  clear () {
    // 销毁道路的高亮蒙层
    disposeHighlightRoadMask(this.container)
    this.container.clear()
  }

  dispose () {
    this.container.traverse((child) => {
      if (child instanceof Mesh || child instanceof Line) {
        child.geometry.dispose()
        disposeMaterial(child)
      }
    })
  }
}

export default EditElevationHelper
