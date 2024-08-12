/**
 * vuex状态汇总
 * @module store
 */

import { createStore } from 'vuex'
import sensor from './modules/sensor'
import chartBar from './modules/chart-bar'
import debugBar from './modules/debug-bar'
import elementProps from './modules/element-props'
import scenario from './modules/scenario'
import scenarioSet from './modules/scenario-set'
import playlist from './modules/playlist'
import kpi from './modules/kpi'
import kpiGroup from './modules/kpi-group'
import kpiReport from './modules/kpi-report'
import titleBar from './modules/title-bar'
import stage from './modules/stage'
import map from './modules/map'
import simModules from './modules/sim-modules'
import moduleSet from './modules/module-set'
import system from './modules/system'
import mission from './modules/mission'
import environment from './modules/environment'
import tapd from './modules/tapd'
import progressInfo from './modules/progress-top'
import planners from './modules/planners'
import catalogs from './modules/catalogs'
import log2world from './modules/log2world'
import signalControl from './modules/signal-control'
import pbConfig from './modules/pb-config'
import playerViewConfig from './modules/player-view-config'

/**
 * 创建并导出Vuex store实例
 * @type {Store}
 */
const store = createStore({
  state: {},
  getters: {
    /**
     * 获取全局加载状态
     * @param {object} state - 当前模块状态
     * @param {object} getters - Vuex getters
     * @param {object} rootState - 根模块状态
     * @returns {boolean} - 全局加载状态
     */
    loading (state, getters, rootState) {
      // todo: 由多个子模块的loading聚合成一个globalLoading loading1 || loading2
      return rootState.scenario.loading
    },
  },
  modules: {
    sensor,
    'chart-bar': chartBar,
    'debug-bar': debugBar,
    'element-props': elementProps,
    scenario,
    'scenario-set': scenarioSet,
    playlist,
    kpi,
    'kpi-group': kpiGroup,
    'kpi-report': kpiReport,
    'title-bar': titleBar,
    stage,
    map,
    'sim-modules': simModules,
    'module-set': moduleSet,
    system,
    mission,
    environment,
    tapd,
    progressInfo,
    planners,
    catalogs,
    log2world,
    signalControl,
    'pb-config': pbConfig,
    playerViewConfig,
  }, // Vuex模块集合
})

export default store
