import { disposeAllBoundaryHelper } from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'

class EditSectionHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editSectionContainer')
  }

  onActivate (): void {}
  onDeactivate (): void {
    this.clear()
  }

  clear () {
    disposeAllBoundaryHelper(this.container)
  }

  dispose (): void {}
}

export default EditSectionHelper
