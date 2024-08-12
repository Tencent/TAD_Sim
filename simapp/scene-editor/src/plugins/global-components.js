import SimLabel from '@/components/sim-label.vue'
import BaseAuth from '@/components/BaseAuth.vue'

/**
 * 全局组件注册
 */
export default {
  install (app) {
    app.component('SimLabel', SimLabel)
    app.component('BaseAuth', BaseAuth)
  },
}
