import {
  removeAllHTPoints,
  resetJunctionLinkColor,
  setAllJunctionLinksVisible,
  setAllRoadLinksVisible,
} from './draw'
import TemplateHelper, { type IHelperParams } from '@/plugins/template/helper'

import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import { getJunction } from '@/utils/mapCache'

class EditLinkHelper extends TemplateHelper {
  constructor (params: IHelperParams) {
    super(params)
  }

  init () {
    this.initContainer('editLinkContainer')
  }

  onActivate () {
    // 在编辑连接线模式激活时，渲染所有交叉路口、车道的 link 线
    // setAllJunctionLinksVisible(true)
    // setAllRoadLinksVisible(true)
  }

  onDeactivate () {
    // 清空所有车道线的首尾辅助点
    removeAllHTPoints(this.container)

    // 恢复该 junction 所有 laneLink 默认颜色
    const junctionId = useLinkInteractionStore().junctionId
    if (junctionId) {
      const junction = getJunction(junctionId)
      if (junction) {
        junction.laneLinks.forEach((laneLink: biz.ILaneLink) => {
          resetJunctionLinkColor(laneLink.id)
        })
      }
    }

    // 隐藏 laneLink
    setAllJunctionLinksVisible(false)
    setAllRoadLinksVisible(false)
  }

  dispose () {}
}

export default EditLinkHelper
