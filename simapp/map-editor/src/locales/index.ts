import { createI18n } from 'vue-i18n'
import { get } from 'lodash'
import zhCn from './dictionary/zh-cn'
import en from './dictionary/en'

// 单机版是通过 electron.env 配置的语言参数来管理的
// const cacheLocale = window.localStorage.getItem('locale')
type Lang = 'en' | 'zh-CN' | ''
const lang: Lang = get(window, 'electron.env.TADSIM_LANGUAGE')
// 项目已有的语言配置字段在前端侧的映射
const langMap = {
  'en': 'en',
  'zh-CN': 'zhCn',
}
const locale = lang ? langMap[lang] : langMap['zh-CN']

const i18n = createI18n({
  locale,
  messages: {
    zhCn,
    en,
  },
})

export default i18n
