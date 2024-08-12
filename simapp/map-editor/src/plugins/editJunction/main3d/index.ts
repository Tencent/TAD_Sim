import { Line, Mesh } from 'three'
import {
  disposeAllRoadEndEdge,
  disposeHighlightJunctionMask,
  renderRoadEndEdge,
} from './draw'
import TemplateHelper from '@/plugins/template/helper'
import { useRoadStore } from '@/stores/road'
import { disposeMaterial } from '@/utils/common3d'
import { getRoad } from '@/utils/mapCache'

interface IParams {
  render: Function
}

class EditJunctionHelper extends TemplateHelper {
  constructor (params: IParams) {
    super(params)
  }

  init () {
    this.initContainer('editJunctionContainer')
  }

  onActivate () {
    const roadStore = useRoadStore()
    for (const roadId of roadStore.ids) {
      const road = getRoad(roadId)
      if (!road) continue
      // 渲染车道首尾两侧成对端点的坐标
      renderRoadEndEdge({
        parent: this.container,
        road,
        preDispose: true,
      })
    }
  }

  onDeactivate () {
    this.clear()
  }

  clear () {
    // 将内部所有道路首尾端面辅助元素清空
    disposeAllRoadEndEdge(this.container)
    // 将路口的高亮辅助蒙层销毁
    disposeHighlightJunctionMask(this.container)

    // eslint-disable-next-line no-unused-expressions
    this.container.clear
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

export default EditJunctionHelper
