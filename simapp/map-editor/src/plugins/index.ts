import type TemplatePlugin from './template/plugin'
import editRoadPlugin from './editRoad'
import editJunctionPlugin from './editJunction'
import editLinkPlugin from './editLink'
import editLaneNumberPlugin from './editLaneNumber'
import editLaneWidthPlugin from './editLaneWidth'
import editLaneAttrPlugin from './editLaneAttr'
import editBoundaryPlugin from './editBoundary'
import editSectionPlugin from './editSection'
import editObjectPlugin from './editObject'
import editElevationPlugin from './editElevation'
import editSignalControlPlugin from './editSignalControl'
import editCircleRoadPlugin from './editCircleRoad'
import editCrgPlugin from './editCrg'
import root3d from '@/main3d/index'
import { usePluginStore } from '@/stores/plugin'
import { useHistoryStore } from '@/stores/history'
import i18n from '@/locales'
import { cachePluginConfig } from '@/utils/tools'
import { globalConfig } from '@/utils/preset'

const plugins = {
  editRoad: editRoadPlugin,
  editJunction: editJunctionPlugin,
  editObject: editObjectPlugin,
  editLaneAttr: editLaneAttrPlugin,
  editLaneWidth: editLaneWidthPlugin,
  editLaneNumber: editLaneNumberPlugin,
  editBoundary: editBoundaryPlugin,
  editSection: editSectionPlugin,
  editLink: editLinkPlugin,
  editElevation: editElevationPlugin,
  editSignalControl: editSignalControlPlugin,
  editCircleRoad: editCircleRoadPlugin,
  editCrg: editCrgPlugin,
}

class PluginManager {
  registryMap: Map<string, TemplatePlugin>
  unsubscribePluginStore: Function
  plugins: Array<biz.IPluginName>
  constructor () {
    this.registryMap = new Map()
    this.plugins = []
  }

  // 在模块加载时就预先缓存使用的插件配置
  preCachePluginConfig () {
    this.plugins = []
    const { plugin: exposePlugins } = globalConfig.exposeModule
    for (const pluginName in exposePlugins) {
      if (exposePlugins[pluginName]) {
        this.plugins.push(pluginName)
      }
    }
    this.plugins.forEach((pluginName) => {
      const plugin = plugins[pluginName]
      const { config } = plugin
      cachePluginConfig(config)
    })
  }

  init () {
    this.initDispatch()

    // 同步注册插件
    this.plugins.map((pluginName: string) =>
      this.registryPlugin(pluginName as biz.IPluginName),
    )

    // 完整插件的初始化，需要触发一次重绘
    root3d.core.render()
  }

  initDispatch () {
    const pluginStore = usePluginStore()
    const historyStore = useHistoryStore()
    pluginStore.$reset()

    this.unsubscribePluginStore = pluginStore.$onAction(
      ({ name, store, args, after, onError }) => {
        // before store change
        let isPluginChanged = false
        switch (name) {
          case 'activate': {
            const [pluginName] = args
            if (!pluginName) return
            if (pluginName !== store.currentPlugin) {
              // 状态切换的回调函数触发，需要在 before 执行
              isPluginChanged = true
              this.deactivate(store.currentPlugin)
              this.activate(pluginName)
            }
            break
          }
        }

        after((res) => {
          if (name === 'applyState') {
            // 还原插件模式时，增量重绘前后两种插件模式下的元素
            const [lastState, diffState] = args
            // 取消上一个状态已激活的插件，即 diffState.currentPlugin 取消激活
            // 激活上一次状态记录的插件状态，即 lastState.currentPlugin 激活
            this.deactivate(diffState.currentPlugin)
            this.activate(lastState.currentPlugin)

            root3d.core.render()
            return
          }

          switch (name) {
            case 'activate': {
              // 插件模式的切换日志记录，需要在 after store changed 后调用
              // 因为 history.save 操作需要获取当前所有 store 的实时数据
              // 只有模式切换后，才会写入操作记录
              if (!isPluginChanged) return
              // 每一次切换编辑模式，都会重置交互的状态

              const abbrPluginName = `plugins.${store.currentPlugin}`
              // 切换插件模式，不计入 diff 的操作记录
              historyStore.save({
                title: `${i18n.global.t(
                  'actions.toggleModeTo',
                )} : ${i18n.global.t(abbrPluginName)}`,
                diff: false,
              })
              break
            }
          }
          root3d.core.render()
        })
        onError((err) => {
          console.log(err)
        })
      },
    )
  }

  getPlugin (name: biz.IPluginName) {
    if (!name) return null
    if (!this.registryMap.has(name)) return null
    return this.registryMap.get(name)
  }

  // 获取插件的配置信息
  getPluginConfig (name?: biz.IPluginName) {
    if (name) {
      if (!this.registryMap.has(name)) return
      const plugin = this.registryMap.get(name)
      return plugin?.config
    }

    const config = []
    for (const plugin of this.registryMap.values()) {
      config.push({
        ...plugin.config,
      })
    }
    return config
  }

  // 动态引入是否会引入其他的问题，最好做自测和分析
  registryPlugin (pluginName: biz.IPluginName) {
    try {
      if (this.registryMap.has(pluginName)) return
      const pluginInstance = plugins[pluginName]
      pluginInstance.init({
        scene: root3d.proxyScene,
        render: root3d.core.render,
      })
      this.registryMap.set(pluginName, pluginInstance)
    } catch (err) {
      console.log(`registry ${pluginName} plugin error: `, err)
    }
  }

  activate (name: biz.IPluginName | '') {
    if (!name) return
    if (!this.registryMap.has(name)) return
    const plugin = this.registryMap.get(name)
    if (!plugin) return
    plugin.activate()
  }

  deactivate (name: biz.IPluginName | '') {
    if (!name) return
    if (!this.registryMap.has(name)) return
    const plugin = this.registryMap.get(name)
    if (!plugin) return
    plugin.deactivate()
  }

  clear () {
    for (const plugin of this.registryMap.values()) {
      // 将所有插件取消激活状态
      plugin.deactivate()
    }
  }

  dispose () {
    for (const plugin of this.registryMap.values()) {
      plugin.dispose()
    }
    this.registryMap.clear()

    this.unsubscribePluginStore && this.unsubscribePluginStore()
  }
}

const pluginManager = new PluginManager()
pluginManager.preCachePluginConfig()

export default pluginManager
