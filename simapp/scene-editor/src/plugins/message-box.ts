import { markRaw } from 'vue'
import type { App } from 'vue'
import { ElMessageBox } from 'element-plus'
import type { MessageBoxData } from 'element-plus'
import type { ElMessageBoxOptions } from 'element-plus/es/components/message-box/src/message-box.type'
import { CircleCheckFilled, CircleCloseFilled, InfoFilled, QuestionFilled } from '@element-plus/icons-vue'
import i18n from '@/locales'

function messageBoxError (message: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxError (message: string, title?: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxError (
  message: string,
  title?: string | ElMessageBoxOptions,
  options: ElMessageBoxOptions = {},
): Promise<MessageBoxData> {
  let t: string = i18n.t('tips.error')
  let opt: ElMessageBoxOptions
  if (typeof title === 'string') {
    t = title
    opt = options
  } else if (typeof title === 'object') {
    opt = title || {}
  } else {
    opt = options
  }
  return ElMessageBox.alert(
    message,
    t,
    {
      type: 'error',
      icon: markRaw(CircleCloseFilled),
      center: true,
      customClass: 'sim-message-box',
      ...opt,
    },
  )
}

function messageBoxSuccess (message: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxSuccess (message: string, title?: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxSuccess (
  message: string,
  title?: string | ElMessageBoxOptions,
  options: ElMessageBoxOptions = {},
): Promise<MessageBoxData> {
  let t: string = i18n.t('tips.success')
  let opt: ElMessageBoxOptions
  if (typeof title === 'string') {
    t = title
    opt = options
  } else if (typeof title === 'object') {
    opt = title || {}
  } else {
    opt = options
  }
  return ElMessageBox.alert(
    message,
    t,
    {
      type: 'success',
      icon: markRaw(CircleCheckFilled),
      center: true,
      customClass: 'sim-message-box',
      ...opt,
    },
  )
}

function messageBoxInfo (message: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxInfo (message: string, title?: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxInfo (
  message: string,
  title?: string | ElMessageBoxOptions,
  options: ElMessageBoxOptions = {},
): Promise<MessageBoxData> {
  let t: string = i18n.t('tips.tips')
  let opt: ElMessageBoxOptions
  if (typeof title === 'string') {
    t = title
    opt = options
  } else if (typeof title === 'object') {
    opt = title || {}
  } else {
    opt = options
  }
  return ElMessageBox.alert(
    message,
    t,
    {
      type: 'info',
      icon: markRaw(InfoFilled),
      center: true,
      customClass: 'sim-message-box',
      ...opt,
    },
  )
}

function messageBoxConfirm (message: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxConfirm (message: string, title?: string, options?: ElMessageBoxOptions): Promise<MessageBoxData>
function messageBoxConfirm (
  message: string,
  title?: string | ElMessageBoxOptions,
  options: ElMessageBoxOptions = {},
): Promise<MessageBoxData> {
  let t: string = i18n.t('tips.tips')
  let opt: ElMessageBoxOptions
  if (typeof title === 'string') {
    t = title
    opt = options
  } else if (typeof title === 'object') {
    opt = title || {}
  } else {
    opt = options
  }
  return ElMessageBox.confirm(
    message,
    t,
    {
      type: 'warning',
      icon: markRaw(QuestionFilled),
      center: true,
      customClass: 'sim-message-box',
      ...opt,
    },
  )
}

export {
  messageBoxError,
  messageBoxSuccess,
  messageBoxInfo,
  messageBoxConfirm,
}

export default {
  install (app: App) {
    app.config.globalProperties.$errorBox = messageBoxError
    app.config.globalProperties.$successBox = messageBoxSuccess
    app.config.globalProperties.$infoBox = messageBoxInfo
    app.config.globalProperties.$confirmBox = messageBoxConfirm
  },
}
