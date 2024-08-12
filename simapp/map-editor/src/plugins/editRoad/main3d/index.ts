import { Line, Mesh } from 'three'
import { useEditRoadStore } from '../store'
import {
  disposeAllControlPointsAndCurve,
  disposeAllVirtualElement,
  renderAllControlPointAndCurve,
} from './draw'
import { disposeMaterial } from '@/utils/common3d'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'
import { disposeHighlightRoadMask } from '@/main3d/render/road'

class EditRoadHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editRoadContainer')
  }

  onActivate () {
    // 在道路插件激活时，展示所有孤立的控制点
    const editRoadStore = useEditRoadStore()
    renderAllControlPointAndCurve({
      parent: this.container,
      controlPoints: editRoadStore.controlPoints,
      onlyIsolated: true, // 只渲染孤立的控制点
    })
  }

  onDeactivate () {
    this.clear()
  }

  clear () {
    // 将内部的控制点、控制线辅助元素清空
    disposeAllControlPointsAndCurve(this.container)
    disposeAllVirtualElement(this.container)
    // 将道路高亮辅助蒙层清空
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

export default EditRoadHelper
