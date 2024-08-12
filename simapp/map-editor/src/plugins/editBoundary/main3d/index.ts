import { Line } from 'three'
import { disposeBoundaryHelperLine } from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeMaterial } from '@/utils/common3d'

class EditBoundaryHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editBoundaryContainer')
  }

  onActivate (): void {}
  onDeactivate (): void {
    // eslint-disable-next-line no-unused-expressions
    this.clear
  }

  clear (): void {
    // 销毁所有车道边界辅助线
    disposeBoundaryHelperLine({
      parent: this.container,
    })
  }

  dispose (): void {
    this.container.traverse((child) => {
      if (child instanceof Line) {
        child.geometry.dispose()
        disposeMaterial(child)
      }
    })
  }
}

export default EditBoundaryHelper
