import { editor } from '@/api/interface'

export default {
  strict: import.meta.env.MODE === 'development', // 开发模式下启用严格模式
  namespaced: true, // 模块命名空间隔离
  state: {}, // 模块状态
  getters: {
    /**
     * 获取选中对象的最大偏移量
     * @param {object} state - 模块状态
     * @param {object} getter - Vuex getters
     * @param {object} rootState - 根模块状态
     * @param {object} rootGetters - 根模块getters
     * @returns {number} - 选中对象的最大偏移量
     */
    maxOffset (state, getter, rootState, rootGetters) {
      const selectedObject = rootGetters['scenario/selectedObject']
      if (!selectedObject) return 0
      // return editor.scenario.getLaneWidth(selectedObject) / 2
      return editor.scenario.getMaxOffset(selectedObject)
    },
    /**
     * 获取选中对象的最大移动距离
     * @param {object} state - 模块状态
     * @param {object} getter - Vuex getters
     * @param {object} rootState - 根模块状态
     * @param {object} rootGetters - 根模块getters
     * @returns {number} - 选中对象的最大移动距离
     */
    maxShift (state, getter, rootState, rootGetters) {
      const selectedObject = rootGetters['scenario/selectedObject']
      if (!selectedObject) return 0
      return editor.scenario.getLaneLength(selectedObject)
    },
    /**
     * 获取选中对象的可选车道ID列表
     * @param {object} state - 模块状态
     * @param {object} getter - Vuex getters
     * @param {object} rootState - 根模块状态
     * @param {object} rootGetters - 根模块getters
     * @returns {Array} - 可选车道ID列表
     */
    optionLaneIDList (state, getter, rootState, rootGetters) {
      const selectedObject = rootGetters['scenario/selectedObject']
      if (!selectedObject) return []
      return editor.scenario.getLaneList(selectedObject)
    },
  },
  mutations: {
    /**
     * 更新模块状态
     * @param {object} state - 模块状态
     * @param {object} payload - 需要更新的状态数据
     */
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
}
