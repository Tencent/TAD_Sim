import { createApp } from 'vue'
import { createPinia } from 'pinia'
import ElementPlus from 'element-plus'
import App from './App.vue'
import { initPreset } from './utils/preset'
// 尝试加载 webWorker
import '@/services/worker'
import i18n from './locales/index'
import 'element-plus/dist/index.css'
import './assets/base.less'
import './assets/theme.less'
import { myDragConfig } from './components/plugins/my-drag'
import { MyDropConfig } from './components/plugins/my-drop'
import BaseAuth from './components/common/BaseAuth.vue'

// 初始化一些预置的属性或配置
initPreset()

const app = createApp(App)

// 全局组件
app.component('BaseAuth', BaseAuth)

// 自定义的指令
app.directive('my-drag', myDragConfig)
app.directive('my-drop', MyDropConfig)

app.use(i18n)
app.use(createPinia())
app.use(ElementPlus)
app.mount('#app')
