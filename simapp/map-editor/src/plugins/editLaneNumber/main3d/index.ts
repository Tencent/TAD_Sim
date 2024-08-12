import { Line, Mesh } from 'three'
import { disposeBoundaryHelperLine, disposeLaneMask } from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeMaterial } from '@/utils/common3d'

class EditLaneNumberHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editLaneNumberContainer')
  }

  onActivate (): void {}
  onDeactivate (): void {
    this.clear()
  }

  clear (): void {
    // 清空所有的辅助元素
    disposeLaneMask({
      parent: this.container,
    })
    disposeBoundaryHelperLine({
      parent: this.container,
    })
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

export default EditLaneNumberHelper
