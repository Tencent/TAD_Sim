// i18n
import { createI18n } from 'vue-i18n'
// import enLocale from 'element-plus/dist/locale/en.mjs'
// import zhLocale from 'element-plus/dist/locale/zh-cn.mjs'
// import ElementLocale from 'element-plus/dist/locale/index.mjs'
import _ from 'lodash'

import enMessages from './lang/en.json'
import zhMessages from './lang/zh.json'

const messages = {}
export const supportLanguages = ['zh-CN', 'en']
const cachedLocale = _.get(window, 'electron.env.TADSIM_LANGUAGE', 'zh-CN')
const fallbackLocale = supportLanguages[0]
export const locale = cachedLocale

messages.en = enMessages
messages['zh-CN'] = zhMessages

const i18n = createI18n({
  locale,
  messages,
  fallbackLocale,
  silentFallbackWarn: true,
  numberFormats: {
    'zh-CN': {
      number: {
        numberingSystem: 'hanidec',
      },
    },
    'en': {
      number: {
        numberingSystem: 'native',
      },
    },
  },
  silentTranslationWarn: true,
})

export default i18n.global
export {
  i18n as i18nPlugin,
}
