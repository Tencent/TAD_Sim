import { Mesh } from 'three'
import { disposeLaneHelper } from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeMaterial } from '@/utils/common3d'

class EditLaneAttrHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editLaneAttrContainer')
  }

  onActivate (): void {}
  onDeactivate (): void {
    this.clear()
  }

  clear (): void {
    // 清空所有的辅助元素
    disposeLaneHelper({
      parent: this.container,
    })
  }

  dispose (): void {
    this.container.traverse((child) => {
      if (child instanceof Mesh) {
        child.geometry.dispose()
        disposeMaterial(child)
      }
    })
  }
}

export default EditLaneAttrHelper
