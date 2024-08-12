// import devtools from '@vue/devtools'
import { createApp } from 'vue'
import { createPinia } from 'pinia'
import ElementPlus from 'element-plus'
import * as ElementPlusIconsVue from '@element-plus/icons-vue'
import epZhCn from 'element-plus/dist/locale/zh-cn.mjs'
import epEn from 'element-plus/dist/locale/en.mjs'
import router from './router'
import store from './store'
import { i18nPlugin, locale } from './locales'
import filters from './filters'
import App from './App.vue'
import icons from '@/plugins/svg-icons'
import resizer from '@/plugins/resizer'
import modeLibrary from '@/plugins/mode-library'
import myDrag from '@/plugins/my-drag'
import progressMessage from '@/plugins/progress-message'
import globalComponents from '@/plugins/global-components'
import SimMessageBox from '@/plugins/message-box'
import 'element-plus/dist/index.css'

import '@/assets/less/element-plus.reset.less'
import '@/assets/less/common.less' // 放在最后，需要覆盖element-plus的样式

const app = createApp(App) // 创建 Vue 应用实例
const pinia = createPinia() // 创建 Pinia 实例

// 将全局过滤器添加到 Vue 应用实例的全局属性中
Object.keys(filters).forEach((key) => {
  app.config.globalProperties[key] = filters[key]
})

// Element Plus 图标组件注册为全局组件
Object.entries(ElementPlusIconsVue).forEach(([key, component]) => {
  app.component(key, component)
})

app.use(store)
app.use(pinia)
app.use(router)

// 配置 Element Plus 的本地化
const epLocale = {
  'zh-CN': epZhCn,
  'en': epEn,
}
app.use(ElementPlus, { locale: epLocale[locale] || epEn })

// 使用 i18n 插件、全局组件、resizer 等其他插件
app.use(i18nPlugin)
app.use(globalComponents)
app.use(resizer)
app.use(modeLibrary)
app.use(icons)
app.use(myDrag)
app.use(progressMessage)
app.use(SimMessageBox)

// 添加一个全局的 $ts 方法，用于处理翻译
app.$ts = function (key, fallback) {
  if (this.$te(key)) {
    return this.$t(key)
  }
  return fallback
}

// 关闭生产提示
app.config.productionTip = false
app.mount('#app')
