import { Line, Mesh } from 'three'
import { disposeAllVirtualElement, disposeHelperElement } from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeMaterial } from '@/utils/common3d'

class EditCircleRoadHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editCircleRoadContainer')
  }

  onActivate (): void {}
  onDeactivate (): void {
    this.clear()
  }

  clear (): void {
    disposeHelperElement({ parent: this.container })
    disposeAllVirtualElement(this.container)
    this.container.clear()
  }

  dispose (): void {
    this.container.traverse((child) => {
      if (child instanceof Mesh || child instanceof Line) {
        child.geometry.dispose()
        disposeMaterial(child)
      }
    })
  }
}

export default EditCircleRoadHelper
