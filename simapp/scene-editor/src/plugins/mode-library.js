import { getModelDisplayName, getModelIcon } from '@/common/utils'

/**
 * 获取模型图标路径
 * 获取模型显示名称
 */
export default {
  install (app) {
    app.config.globalProperties.$getModelIcon = getModelIcon
    app.config.globalProperties.$getModelName = getModelDisplayName
  },
}
