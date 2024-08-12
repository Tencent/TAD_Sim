import { ElLoading, ElMessage, ElMessageBox } from 'element-plus'
import { markRaw } from 'vue'
import type { VNode } from 'vue'
import { useHelperStore } from '@/stores/helper'
import excSvg from '@/components/dialog/icon/excmark.svg'
import queSvg from '@/components/dialog/icon/quemark.svg'
import i18n from '@/locales'

export function fullScreenLoading (content: string) {
  const instance = ElLoading.service({
    lock: true,
    fullscreen: true,
    text: `${content}...`,
    customClass: 'fullscreen-loading',
    background: 'rgba(0, 0, 0, 0.5)',
  })
  return instance
}

export function successMessage (params: {
  content: string
  duration?: number
  grouping?: boolean
}) {
  const { content, duration = 2000, grouping = true } = params
  ElMessage({
    message: content,
    customClass: 'sim-message',
    duration,
    grouping,
    type: 'success',
  })
}

export function errorMessage (params: {
  content: string
  duration?: number
  grouping?: boolean
}) {
  const { content, duration = 2000, grouping = true } = params
  ElMessage({
    message: content,
    customClass: 'sim-message',
    duration,
    grouping,
    type: 'error',
  })
}

export function warningMessage (params: {
  content: string
  duration?: number
  grouping?: boolean
}) {
  const { content, duration = 2000, grouping = true } = params
  ElMessage({
    message: content,
    customClass: 'sim-message',
    duration,
    grouping,
    type: 'warning',
  })
}

export const LayoutConstant = {
  // 最大尺寸
  bottomMaxHeight: 500,
  rightMaxWidth: 400,
  leftMaxWidth: 370, // 370px 刚好能一行四列
  // 最小尺寸
  bottomMinHeight: 30,
  rightMinWidth: 200,
  leftMinWidth: 120,
  // 初始尺寸
  initBottomHeight: 180,
  initRightWidth: 260,
  initLeftWidth: 220,
  // 缓存的 key 标识
  bottomKey: 'bottom-area-height',
  rightKey: 'right-area-width',
  leftKey: 'left-area-width',
}

interface TArgs {
  type: 'exc' | 'que' // title 前 icon 的类型
  msg: string | VNode | (() => VNode)
  title: string
  confirmButtonText?: string
  cancelButtonText?: string
}

export function confirm (args: TArgs) {
  const helperStore = useHelperStore()
  helperStore.closeShortcutKeys()
  const { type, msg, title, confirmButtonText, cancelButtonText } = args

  let titleMarker: object
  let classType: string = 'custom-confirm-box'
  switch (type) {
    case 'exc':
      titleMarker = excSvg as any
      break
    case 'que':
      titleMarker = queSvg as any
      classType = 'custom-confirm-box-que'
      break
  }

  return new Promise<boolean>((resolve) => {
    return ElMessageBox.confirm(msg, title, {
      autofocus: false,
      closeOnPressEscape: false,
      closeOnClickModal: false,
      showClose: false,
      confirmButtonText:
        confirmButtonText || i18n.global.t('desc.tips.confirm'),
      cancelButtonText: cancelButtonText || i18n.global.t('desc.tips.cancel'),
      center: true,
      icon: markRaw(titleMarker),
      customClass: classType, // 选择适当地方引入 './index.less'
      beforeClose: (action, instance, done) => {
        helperStore.openShortcutKeys()
        done()
      },
    })
      .then((res) => {
        // res 为 'confirm'
        resolve(true)
      })
      .catch((res) => {
        // 点击取消后 res 为 'cancel'
        resolve(false)
      })
  })
}
