import { getMenuObj } from '@/common/utils'
import i18n from '@/locales'
import { messageBoxConfirm } from '@/plugins/message-box'
import { getPermission } from '@/utils/permission'

const { electron: { focusedBrowserWindow, crossBrowserWindowMessage, editor, mapEditor } } = window

/**
 * 目录数据定义
 * tree of {hotKey: string, disabled: boolean, title: string, key: string, children: []}
 * key: 唯一标识
 * title: 菜单名称
 * disabled: 是否禁用
 * hotKey: 快捷键
 * split: 是否在下方显示分割线
 * children: 子菜单
 */
const menus = [
  {
    key: '1',
    title: 'menu.file',
    disabled: false,
    children: [
      {
        key: '1.1',
        title: 'menu.new',
        hotKey: 'Ctrl+N',
        disabled: false,
      },
      {
        key: '1.2',
        title: 'menu.open',
        hotKey: 'Ctrl+O',
        disabled: false,
      },
      {
        key: '1.3',
        title: 'menu.recent',
        children: [],
        disabled: false,
      },
      {
        key: '1.5',
        title: 'menu.save',
        hotKey: 'Ctrl+S',
        disabled: false,
      },
      {
        key: '1.6',
        title: 'menu.saveAs',
        hotKey: 'Ctrl+Shift+S',
        disabled: false,
      },
      {
        key: '1.7',
        title: 'menu.importFromTAPD',
        split: true,
        disabled: false,
      },
      {
        key: '1.4',
        title: 'menu.exit',
        hotKey: 'Ctrl+Q',
        disabled: false,
      },
    ],
  },
  {
    key: '2',
    title: 'menu.edit',
    disabled: false,
    children: [
      /*
      {
        key: '2.1',
        title: 'menu.withdraw',
        hotKey: 'Ctrl+Z',
        split: true,
        disabled: false,
      },
      */
      {
        key: '2.2',
        title: 'menu.delete',
        hotKey: 'Delete',
        disabled: false,
      },
    ],
  },
  {
    key: '3',
    title: 'menu.data',
    disabled: false,
    children: [
      {
        key: '3.1',
        title: 'menu.scenarioManagement',
        disabled: false,
      },
      {
        key: '3.2',
        title: 'menu.mapManagement',
        split: true,
        disabled: false,
      },
      {
        key: '3.3',
        title: 'menu.moduleManagement',
        disabled: false,
      },
      {
        key: '3.7',
        title: 'menu.moduleGlobalManagement',
        disabled: false,
      },
      {
        key: '3.4',
        title: 'menu.traffic',
        split: true,
        disabled: false,
      },
      {
        key: '3.5',
        title: 'menu.indicator',
        disabled: false,
      },
      {
        key: '3.6',
        title: 'menu.evaluation',
        disabled: false,
      },
    ],
  },
  {
    key: '4',
    title: 'menu.system',
    disabled: false,
    children: [
      {
        key: '4.1',
        title: 'menu.simSettings',
        disabled: false,
        split: false,
      },
      /*
      {
        key: '4.2',
        title: 'menu.renderQuality',
        disabled: true,
      },
      */
    ],
  },
  {
    key: '6',
    title: 'menu.help',
    disabled: false,
    children: [
      /*
      {
        key: '6.1',
        title: 'menu.helpDoc',
        disabled: true,
      },
      {
        key: '6.2',
        title: 'menu.whatsNew',
        split: true,
        disabled: true,
      },
      */
      {
        key: '6.3',
        title: 'menu.about',
        disabled: false,
      },
    ],
  },
]

export default {
  strict: import.meta.env.MODE === 'development',
  namespaced: true,
  state: {
    isMaximized: false,
    menuVisible: false,
  },
  getters: {
    /**
     * 显示的菜单
     * @param state
     * @param getters
     * @param rootState
     * @param rootGetters
     * @return {*}
     */
    menus (state, getters, rootState, rootGetters) {
      const { openHistory, isDirty, newScenario } = rootState.scenario
      const historyMenuParent = menus[0].children[2]
      if (openHistory.length) {
        historyMenuParent.disabled = false
        historyMenuParent.children = openHistory.map((history, i) => ({ key: `1.3.${i + 1}`, title: history }))
      } else {
        historyMenuParent.disabled = true
      }

      // 批量播放时禁用的菜单
      const batchPlaying = rootGetters['scenario/batchPlaying']
      let batchDisabled = false
      if (batchPlaying) {
        batchDisabled = true
      }
      getMenuObj('1', menus).disabled = batchDisabled
      getMenuObj('2', menus).disabled = batchDisabled
      getMenuObj('3', menus).disabled = batchDisabled
      getMenuObj('4', menus).disabled = batchDisabled
      getMenuObj('6', menus).disabled = batchDisabled

      getMenuObj('2', menus).disabled = rootGetters['scenario/isPlaying']
      const selectedObject = rootGetters['scenario/selectedObject']
      getMenuObj('2.2', menus).disabled = !selectedObject
      // getMenuObj('2.1', menus).disabled = true

      const presentScenario = rootGetters['scenario/presentScenario']
      getMenuObj('3.4', menus).disabled = (!presentScenario || !rootGetters['scenario/isModifying'])

      getMenuObj('1.5', menus).disabled = !isDirty
      getMenuObj('1.6', menus).disabled = !presentScenario || !rootGetters['scenario/isModifying'] || newScenario
      getMenuObj('1.7', menus).disabled = !getPermission('router.tapd.show');

      // 播放时禁用的菜单
      ['3.1', '3.2', '3.4'].forEach((key) => {
        getMenuObj(key, menus).disabled = rootGetters['scenario/isPlaying']
      })
      getMenuObj('3.3', menus).disabled = rootGetters['scenario/isPlaying'] && rootState.scenario.playingStatus === 'playing'

      return menus
    },
  },
  mutations: {
    updateState (state, payload) {
      Object.assign(state, payload)
    },
  },
  actions: {
    /**
     * 退出软件
     * @param state
     * @param dispatch
     * @return {Promise<void>}
     */
    async quit ({ state, dispatch }) {
      const isMapEditorOpened = await mapEditor.isOpened()
      if (isMapEditorOpened) {
        state.mapEditorClosing = true
        mapEditor.focus()
        crossBrowserWindowMessage.emit('map-editor-close')
      } else {
        dispatch('doQuit')
      }
    },

    /**
     * 退出软件
     * @param rootState
     * @param rootGetters
     * @param dispatch
     * @return {Promise<void>}
     */
    async doQuit ({ rootState, rootGetters, dispatch }) {
      const { webContents } = focusedBrowserWindow
      const isPlaying = rootGetters['scenario/isPlaying']
      const { isDirty, playingStatus } = rootState.scenario
      if (isPlaying && playingStatus === 'playing') {
        try {
          await messageBoxConfirm(i18n.t('tips.exitWhilePlaying'))
          await dispatch('scenario/pause', null, { root: true })
        } catch (e) {
          return
        }
      }
      if (isDirty) {
        try {
          await messageBoxConfirm(i18n.t('tips.exitWithoutSaving'))
        } catch (e) {
          return
        }
      }
      await dispatch('scenario/unSetup', null, { root: true })
      const isDevToolsOpened = await webContents.isDevToolsOpened()
      if (isDevToolsOpened) {
        webContents.closeDevTools()
      }
      focusedBrowserWindow.close()
    },
    /**
     * Map Editor 关闭时触发
     * @param state
     * @param dispatch
     * @param confirmed
     */
    mapEditorClosed ({ state, dispatch }, confirmed) {
      if (state.mapEditorClosing && confirmed) {
        dispatch('doQuit')
      }
      state.mapEditorClosing = false
    },
    /**
     * 最大化/还原
     * @param commit
     * @return {Promise<void>}
     */
    async toggleMaximum ({ commit }) {
      const isMaximized = await focusedBrowserWindow.isMaximized()
      if (isMaximized) {
        focusedBrowserWindow.unmaximize()
        commit('updateState', {
          isMaximized: false,
        })
      } else {
        focusedBrowserWindow.maximize()
        commit('updateState', {
          isMaximized: true,
        })
      }
    },
    /**
     * 最小化
     */
    minimum () {
      focusedBrowserWindow.minimize()
    },
    /**
     * 初始化是否最大化
     * @param commit
     * @return {Promise<void>}
     */
    async initIsMaximized ({ commit }) {
      const isMaximized = await focusedBrowserWindow.isMaximized()
      commit('updateState', {
        isMaximized,
      })
    },
    /**
     * 打开历史场景
     * @param dispatch
     * @param rootState
     * @param key
     * @return {Promise<never>}
     */
    async openHistoryScenario ({ dispatch, rootState }, key) {
      const { scenarioList } = rootState.scenario
      const { title } = getMenuObj(key, menus)
      const scenario = scenarioList.find(s => s.filename === title)
      if (!scenario) {
        return Promise.reject(new Error(i18n.t('tips.scenarioDeleted')))
      }
      await dispatch('scenario/openScenario', scenario, { root: true })
    },
    /**
     * 重置所有警告
     * @return {Promise<void>}
     */
    resetAllWarning () {
      return editor.warning.resetAll()
    },
  },
}
