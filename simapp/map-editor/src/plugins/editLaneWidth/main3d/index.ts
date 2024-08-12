import { Line, Mesh } from 'three'
import {
  disposeAllVirtualElement,
  disposeBoundaryHelperLine,
  disposeLaneMask,
} from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeMaterial } from '@/utils/common3d'

class EditLaneWidthHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editLaneWidthContainer')
  }

  onActivate (): void {}
  onDeactivate (): void {
    this.clear()
  }

  clear (): void {
    // 销毁所有辅助元素
    disposeLaneMask({
      parent: this.container,
    })
    disposeBoundaryHelperLine({
      parent: this.container,
    })
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

export default EditLaneWidthHelper
