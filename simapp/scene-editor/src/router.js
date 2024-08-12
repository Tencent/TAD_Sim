import { createRouter, createWebHashHistory } from 'vue-router'
import Home from '@/components/home.vue'
import KpiReportGeneric from '@/components/popups/kpi-report-generic.vue'

// const KpiReportGeneric = () => import('@/components/popups/kpi-report-generic.vue')

/**
 * 创建并配置 Vue Router 实例，定义应用的路由规则
 * @returns {object} routerInstance - 返回一个配置好的 Vue Router 实例
 */
export default createRouter({
  history: createWebHashHistory(), // 使用哈希模式创建路由历史
  routes: [
    {
      // 首页组件
      path: '/',
      name: 'home',
      component: Home,
    },
    {
      // KPI 报告通用组件
      path: '/kpi-report-generic',
      name: 'kpi-report-generic',
      component: KpiReportGeneric,
    },
  ],
})
