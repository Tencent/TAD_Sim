import { cloneDeep, get, pick } from 'lodash'
import { defineStore } from 'pinia'
import { useHistoryStore } from './history'
import { useJunctionStore } from './junction'
import { usePluginStore } from './plugin'
import { useRoadStore } from './road'
import { useHelperStore } from './helper'
import { useObjectStore } from './object'
import { useConfig3dStore } from './config3d'
import i18n from '@/locales'
import { useEditRoadStore } from '@/plugins/editRoad/store'
import {
  createHadmap,
  createSession,
  getHadmap,
  getMapCrgGroupListInCloud,
  getMapList,
  getRoadCrgRela,
  getV2XDeviceList,
  modifyHadmap,
  openHadmap,
  openHadmapInCloud,
  saveHadmap,
  saveHadmapInCloud,
} from '@/services'
import { parseMapElements } from '@/services/loadParser'
import { pickUpMapElementData } from '@/services/saveParser'
import { clearIdCache } from '@/utils/guid'
import {
  getAllPluginConfig,
  saveServiceSessionId,
  setOriginMapCache,
} from '@/utils/tools'
import pluginManager from '@/plugins/index'
import root3d from '@/main3d'
import { oCameraOffset } from '@/utils/common3d'
import {
  getDiffElementData,
  updateOriginMapCacheByDiff,
} from '@/services/saveParser/diff'
import {
  errorMessage,
  fullScreenLoading,
  successMessage,
  warningMessage,
} from '@/utils/common'
import { clearAllCache, getRoad } from '@/utils/mapCache'
import pool from '@/services/worker'
import testFileJson from '@/test/testFile.json'
import { globalConfig } from '@/utils/preset'
import { MapVersion } from '@/utils/business'
import { useModelCacheStore } from '@/stores/object/modelCache'

interface IState {
  mapList: Array<biz.IFile>
  openedFile: string // 当前加载的地图文件，带文件名后缀
  isPreset: boolean // 如果存在打开的地图文件，是否是预设地图
  mapVersion: string // 打开地图的版本
  mapType: string // 打开地图类型（opendrive 规范）
  isLoading: boolean
  version: string // 云端版才涉及 version
  mapId: string // 云端版才涉及 mapId
  coskey: string // 云端存储的地图文件 cos 地址
  mapGroupId: string // 云端地图目录分组
  v2xDeviceList: {
    // 通过接口请求返回的云端版 v2x 设备列表
    rsu: Array<string> // 通信单元
    camera: Array<string> // 摄像头
    lidar: Array<string> // 激光雷达
    radar: Array<string> // 毫米波雷达
  }
  crgList: Array<any> // 通过云端版接口返回的 crg 的结构列表，可能存在多层级的嵌套
  autoSaveTimerId: number // 自动保存的计时器
}

export const useFileStore = defineStore('file', {
  state: (): IState => ({
    mapList: [],
    openedFile: '',
    isPreset: false,
    mapVersion: '',
    mapType: '',
    isLoading: false,
    version: '',
    mapId: '',
    coskey: '',
    mapGroupId: '',
    v2xDeviceList: {
      rsu: [],
      camera: [],
      lidar: [],
      radar: [],
    },
    crgList: [],
    autoSaveTimerId: 0,
  }),
  getters: {},
  actions: {
    // 清空地图列表
    clearMapList () {
      this.mapList = []
    },
    // 每一次启动向后端请求一次 sessionid，后续接口调用需要依赖 sessionid
    async initSession () {
      try {
        const res = await createSession()
        if (!res || get(res, 'data.message') !== 'ok') {
          throw new Error(`api initSession error.`)
        }
        const { sessionId } = res.data

        saveServiceSessionId(sessionId)
      } catch (err) {
        console.log('init session error: ', err)
      }
    },
    // 启动自动保存的计时器
    initAutoSaveTimer () {
      // 在初始化之前，先取消原有的计时器
      this.clearAutoSaveTimer()
      // 触发的间隔 10 分钟
      const time = 10 * 60 * 1000
      this.autoSaveTimerId = setInterval(() => {
        // 判断当前是否有道路元素
        const roadStore = useRoadStore()
        let hasMapElement = false
        if (roadStore.ids.length > 0) {
          hasMapElement = true
        }

        // 如果没有有效的道路元素，则不能自动保存
        if (!hasMapElement) {
          warningMessage({
            content: i18n.global.t(
              'desc.tips.noRoadElementsInTheMapCannotBeSaved',
            ),
          })
          return
        }

        const helperStore = useHelperStore()

        // 判断当前是否有打开的地图
        if (this.openedFile) {
          // 打开地图情况下，调用增量保存
          this.saveDiffMapInCloud(this.openedFile, true)
        } else {
          // 新建地图，弹出保存弹窗
          helperStore.toggleDialogStatus('saveMap')
        }
      }, time)
    },
    clearAutoSaveTimer () {
      if (this.autoSaveTimerId === 0) return
      clearInterval(this.autoSaveTimerId)
      // 重置为 0
      this.autoSaveTimerId = 0
    },
    // 重置渲染层面的关键状态
    resetStore () {
      // 打开地图文件名清空
      this.openedFile = ''

      this.isPreset = false
      this.mapVersion = ''
      this.mapType = ''

      // 清空缓存的地图数据缓存
      setOriginMapCache(null)

      // 清空本地缓存的地图元素
      clearAllCache()

      // history 操作记录状态的重置
      const historyStore = useHistoryStore()
      historyStore.$reset()

      // 道路状态重置
      const roadStore = useRoadStore()
      roadStore.$reset()
      // 路口状态重置
      const junctionStore = useJunctionStore()
      junctionStore.$reset()
      // 物体状态重置
      const objectStore = useObjectStore()
      objectStore.$reset()

      // 当前插件的状态重置
      const pluginStore = usePluginStore()
      // 【注意】使用的是自定义 reset 方法，而不是 $reset
      pluginStore.reset()

      // editRoad 编辑道路状态的重置
      if (useEditRoadStore) {
        const editRoadStore = useEditRoadStore()
        if (editRoadStore) {
          editRoadStore.$reset()
        }
      }

      // 所有的插件执行一次 deactivate 逻辑
      pluginManager.clear()

      // 元素 id 缓存重置
      clearIdCache()

      // 相机视角重置
      root3d.initDefaultCamera()
      const config3dStore = useConfig3dStore()
      config3dStore.resetCamera()

      // 清空地图元素的容器
      root3d.clearMapElementContainer()

      if (globalConfig.isCloud) {
        // 如果是云端环境，触发自动保存的计时器
        this.initAutoSaveTimer()
      }
    },
    // 创建一个新的空场景，统一入口避免多次重复调用
    createNew () {
      // 先判断当前场景是否有已经编辑但未保存的 diff 数据
      const historyStore = useHistoryStore()
      const helperStore = useHelperStore()
      const diffRecord = historyStore.getDiffRecord()
      // TODO 后续有时间可以优化成 Promise 形式
      // 确认清空场景后的回调
      const confirmCallback = () => {
        this.resetStore()

        // 新建地图，默认切换到道路编辑模式
        const pluginStore = usePluginStore()
        const allPlugin = getAllPluginConfig()
        const hasEditRoad = allPlugin.find(e => e.name === 'editRoad')
        if (hasEditRoad) pluginStore.activate('editRoad')
      }

      if (diffRecord.length > 0) {
        // 弹窗二次确认
        helperStore.setMapNotSaveConfirmCallback(confirmCallback)
        helperStore.toggleDialogStatus('mapNotSave', true)
        return
      }

      // 如果没有弹窗，则直接执行回调
      confirmCallback()
    },
    // 保存后重置一部分状态
    resetStoreAfterSave (params: { mapName: string }) {
      const { mapName } = params
      this.openedFile = mapName

      const historyStore = useHistoryStore()
      historyStore.$reset()

      // modelCacheStore 重置删除记录
      const modelCacheStore = useModelCacheStore()
      modelCacheStore.deletedObjId.splice(0)

      pluginManager.clear()

      const pluginStore = usePluginStore()
      const lastPlugin = pluginStore.currentPlugin
      pluginStore.reset()
      if (lastPlugin) {
        pluginStore.activate(lastPlugin)
      }
    },
    // 打开地图列表弹窗，统一入口，避免多次重复调用
    openMapList () {
      // 先判断当前场景是否有已经编辑但未保存的 diff 数据
      const historyStore = useHistoryStore()
      const helperStore = useHelperStore()
      const diffRecord = historyStore.getDiffRecord()
      // TODO 后续有时间可以优化成 Promise 形式
      // 确认清空场景后的回调
      const confirmCallback = async () => {
        // 打开地图管理的弹窗
        helperStore.toggleDialogStatus('mapList', true)
        // 如果地图列表为空，则需要调一次接口更新地图列表
        if (this.mapList.length < 1) {
          await this.loadMapList()
        }
      }

      if (diffRecord.length > 0) {
        // 弹窗二次确认
        helperStore.setMapNotSaveConfirmCallback(confirmCallback)
        helperStore.toggleDialogStatus('mapNotSave', true)
        return
      }

      // 如果没有弹窗，则直接执行回调
      confirmCallback()
    },
    // 退出应用
    quit () {
      // 获取 electron 环境对象
      const focusedBrowserWindow = get(window, 'electron.focusedBrowserWindow')
      if (!focusedBrowserWindow) return
      // 判断当前是否存在未保存的数据
      const historyStore = useHistoryStore()
      const helperStore = useHelperStore()
      const diffRecord = historyStore.getDiffRecord()

      // 关闭窗口
      const closeWindow = async () => {
        // 在关闭界面前，提前关闭 worker 线程
        pool && pool.terminate()

        const { webContents } = focusedBrowserWindow
        const isDevToolsOpened = await webContents.isDevToolsOpened()
        if (isDevToolsOpened) {
          webContents.closeDevTools()
        }
        focusedBrowserWindow.close()
      }

      if (diffRecord.length > 0) {
        // 弹窗二次确认
        helperStore.setMapNotSaveConfirmCallback(closeWindow)
        helperStore.toggleDialogStatus('mapNotSave', true)
        return
      }

      // 如果没有弹窗，则直接执行关闭窗口的回调函数
      closeWindow()
    },
    // 加载地图文件列表
    async loadMapList () {
      this.isLoading = true

      try {
        const res = await getMapList()
        if (!res || get(res, 'data.message') !== 'ok') {
          throw new Error(
            `api get hadmap list error. error code: ${get(res, 'data.code')}`,
          )
        }

        this.mapList = get(res, 'data.data')
        this.isLoading = false
        return true
      } catch (err) {
        console.log('get map list error: ', err)
      }

      this.isLoading = false
      return false
    },
    // 通过地图文件名加载对应地图的数据
    async loadMap (mapName: string) {
      if (!mapName) return false
      this.isLoading = true

      try {
        // 加载地图数据之前，先清空当前场景
        this.resetStore()

        let originData
        if (mapName === 'test') {
          originData = testFileJson
        } else {
          // 先加载地图（但没有读取数据）
          console.time('open map')
          const openRes = await openHadmap(mapName)
          if (!openRes || get(openRes, 'data.message') !== 'ok') {
            throw new Error(
              `api open hadmap error. error info:  ${get(openRes, 'data.info')}`,
            )
          }
          console.timeEnd('open map')
          // 获取地图文件数据
          console.time('get map')
          const getRes = await getHadmap(mapName)
          if (!getRes || !getRes.data) {
            throw new Error(`api get hadmap error.`)
          }
          console.timeEnd('get map')

          // 深拷贝一份地图源文件数据
          originData = getRes.data as biz.ICommonMapElements
        }

        // // 先加载地图（但没有读取数据）
        // console.time('open map')
        // const openRes = await openHadmap(mapName)
        // if (!openRes || get(openRes, 'data.message') !== 'ok') {
        //   throw new Error(
        //     `api open hadmap error. error info:  ${get(openRes, 'data.info')}`
        //   )
        // }
        // console.timeEnd('open map')
        // // 获取地图文件数据
        // console.time('get map')
        // const getRes = await getHadmap(mapName)
        // if (!getRes || !getRes.data) {
        //   throw new Error(`api get hadmap error.`)
        // }
        // console.timeEnd('get map')

        // // 深拷贝一份地图源文件数据
        // const originData = getRes.data as biz.ICommonMapElements
        setOriginMapCache(cloneDeep(originData))

        // 更新加载的文件名称
        this.openedFile = mapName
        // 判断当前打开的地图，是否是预设地图
        let isPreset = false
        this.mapList.forEach((mapOption) => {
          if (mapOption.name === mapName) {
            // '0' 表示非预设
            if (mapOption.preset !== '0') {
              isPreset = true
            }
          }
        })
        this.isPreset = isPreset
        // 判断打开的地图，是否是第三方地图
        this.mapVersion = get(originData, 'header.version') || MapVersion.v2
        // 打开的地图类型
        this.mapType = get(originData, 'header.map_type') || ''

        // 将地图文件中的数据，同步到前端状态
        console.time('parse done')
        await parseMapElements(originData)
        console.timeEnd('parse done')

        // 地图数据加载完成后的处理
        this.onMapLoaded(mapName)

        this.isLoading = false
        return true
      } catch (err) {
        console.log(`load map ${name} content error: `, err)
      }
      // 如果加载地图失败则重置状态
      this.openedFile = ''
      this.isPreset = false
      this.mapVersion = ''
      this.mapType = ''

      setOriginMapCache(null)
      this.isLoading = false
      return false
    },
    // 保存地图的前置逻辑
    saveMap (isSaveAs: boolean = false) {
      const roadStore = useRoadStore()
      const junctionStore = useJunctionStore()
      const modelCacheStore = useModelCacheStore()

      // 判断是否有实际的地图元素
      let hasMapElement = false
      if (roadStore.ids.length > 0) {
        if (!hasMapElement) {
          hasMapElement = true
        }
      }
      if (junctionStore.ids.length > 0) {
        if (!hasMapElement) {
          hasMapElement = true
        }
      }

      if (!hasMapElement) {
        // 如果没有任何实际的道路和路口元素，则提示不能保存
        warningMessage({
          content: i18n.global.t('desc.tips.noMapElementsToSave'),
        })
        return
      }

      const helperStore = useHelperStore()

      if (this.openedFile && isSaveAs) {
        // 打开地图，另存为
        helperStore.toggleDialogStatus('saveAsMap')
      } else if (this.openedFile && !isSaveAs) {
        // 如果是二次编辑的直接保存，属于增量保存
        const historyStore = useHistoryStore()
        const diffRecord = historyStore.getDiffRecord()
        // 二次编辑有 diff 的操作记录，能保存
        // 没有 diff 的操作记录，但是在左侧面板中删除自定义模型时，场景内的物体被删除了，也能保存
        if (
          diffRecord.length > 0 ||
          modelCacheStore.hasDeletedObjWhenDeletingCustomModel
        ) {
          // 走增量保存逻辑
          const { isCloud } = globalConfig
          if (isCloud) {
            // 云端版的增量保存
            this.saveDiffMapInCloud(this.openedFile)
          } else {
            // 单机版的增量保存
            this.modifyAndSaveMap(this.openedFile)
          }
        } else {
          // 否则提示地图没有更新，无须保存
          warningMessage({
            content: i18n.global.t('desc.tips.theMapIsNotUpdatedNoNeedToSave'),
          })
        }
      } else if (!this.openedFile && isSaveAs) {
        // 新建地图，另存为
        helperStore.toggleDialogStatus('saveAsMap')
      } else if (!this.openedFile && !isSaveAs) {
        // 新建地图，保存
        helperStore.toggleDialogStatus('saveMap')
      }
    },
    // 新建+保存，新建+另存为，打开+另存为，这3种情况需要先调用 create_hadmap 接口，在调用 save_hadmap 接口
    async createAndSaveMap (params: {
      mapName: string
      mapType: biz.IMapType
      isSaveAs: boolean
    }) {
      const { mapName, mapType, isSaveAs } = params
      // 保存成新地图使用全量的数据
      this.isLoading = true
      try {
        // 判断如果是另存为，且为同类型，走 save+modify 的逻辑
        const isSaveAndModify =
          isSaveAs === true && mapType === this.mapType && this.openedFile

        let _content
        let diffRes

        if (!isSaveAndModify) {
          // 获取要保存的全量地图元素数据
          _content = await pickUpMapElementData()
          // 先调用新建接口
          const createRes = await createHadmap({
            name: mapName,
            mapType,
            data: _content,
          })
          if (!createRes || get(createRes, 'data.message') !== 'ok') {
            throw new Error(
              `api create hadmap error. error info: ${get(
                createRes,
                'data.info',
              )}`,
            )
          }

          // 再调用保存地图数据接口
          const saveRes = await saveHadmap({
            // 新建地图调用该保存接口，src 和 dst 参数地图名称一致
            srcmap: mapName,
            dstmap: mapName,
          })
          if (!saveRes || get(saveRes, 'data.message') !== 'ok') {
            throw new Error(
              `api save hadmap error. error info: ${get(saveRes, 'data.info')}`,
            )
          }
        } else {
          const saveRes = await saveHadmap({
            srcmap: this.openedFile,
            dstmap: mapName,
          })
          if (!saveRes || get(saveRes, 'data.message') !== 'ok') {
            throw new Error(
              `api save hadmap error. error info: ${get(saveRes, 'data.info')}`,
            )
          }
          diffRes = await getDiffElementData()
          const {
            cmdOptions = [],
            roads = [],
            lanelinks = [],
            objects = [],
          } = diffRes || {}
          const modifyRes = await modifyHadmap({
            name: mapName,
            datas: cmdOptions,
            roads,
            lanelinks,
            objects,
          })
          if (!modifyRes || get(modifyRes, 'data.message') !== 'ok') {
            throw new Error(
              `api modify hadmap error. error info: ${get(
                modifyRes,
                'data.info',
              )}`,
            )
          }
        }

        this.isLoading = false

        // 保存成功提示
        successMessage({
          content: i18n.global.t('desc.tips.saveMapSuccessfully'),
        })

        // 判断有无已打开的地图，如果是已打开地图，另存为的情况，展示的地图名称应该还是原来的地图
        if (this.openedFile) {
          this.resetStoreAfterSave({
            mapName: this.openedFile,
          })
        } else {
          // 更新地图的状态
          this.resetStoreAfterSave({
            mapName,
          })
          this.mapType = mapType
        }

        // 将保存的地图数据，缓存为已打开地图的数据
        if (isSaveAndModify) {
          diffRes && updateOriginMapCacheByDiff(diffRes)
        } else {
          _content && setOriginMapCache(_content)
        }

        // 在完成地图保存的逻辑后，触发跨窗口的通信事件
        const crossBrowserWindowMessage = get(
          window,
          'electron.crossBrowserWindowMessage',
        )
        if (crossBrowserWindowMessage) {
          // @ts-expect-error
          crossBrowserWindowMessage.emit('map-editor-saved', mapName)
        }

        return true
      } catch (err) {
        console.log(`create and save map error: `, err)
      }
      // 保存失败提示
      errorMessage({
        content: i18n.global.t('desc.tips.failedToSaveMap'),
      })
      this.isLoading = false
      return false
    },
    // 打开+保存，需要先调用 modify_hadmap 接口，再调用 save_hadmap 接口
    async modifyAndSaveMap (mapName: string) {
      // 更新已有地图数据，使用增量数据
      this.isLoading = true
      let loadingInstance
      try {
        const diffRes = await getDiffElementData()
        if (!diffRes) {
          this.isLoading = false
          // 提示地图没有更新，无须保存
          warningMessage({
            content: i18n.global.t('desc.tips.theMapIsNotUpdatedNoNeedToSave'),
          })
          return false
        }

        loadingInstance = fullScreenLoading(i18n.global.t('desc.tips.saving'))

        const { cmdOptions, roads, lanelinks, objects } = diffRes
        const modifyRes = await modifyHadmap({
          name: mapName,
          datas: cmdOptions,
          roads,
          lanelinks,
          objects,
        })
        if (!modifyRes || get(modifyRes, 'data.message') !== 'ok') {
          throw new Error(
            `api modify hadmap error. error info: ${get(
              modifyRes,
              'data.info',
            )}`,
          )
        }

        const saveRes = await saveHadmap({
          srcmap: mapName,
          dstmap: mapName,
        })
        if (!saveRes || get(saveRes, 'data.message') !== 'ok') {
          throw new Error(
            `api save hadmap error. error info: ${get(saveRes, 'data.info')}`,
          )
        }

        // 保存成功提示
        successMessage({
          content: i18n.global.t('desc.tips.saveMapSuccessfully'),
        })

        // 重置一部分操作状态
        this.resetStoreAfterSave({
          mapName,
        })
        // // 由于保存时是增量保存，只能通过再请求依次接口拿到保存后的地图源数据作为备份
        // // 获取地图文件数据
        // const getRes = await getHadmap(mapName)
        // if (!getRes || !getRes.data) {
        //   throw new Error(`api get hadmap error.`)
        // }

        // // 深拷贝一份地图源文件数据
        // const originData = getRes.data as biz.ICommonMapElements
        // setOriginMapCache(cloneDeep(originData))

        // 基于当前 diff 的结果，更新本地源数据的缓存
        updateOriginMapCacheByDiff(diffRes)

        this.isLoading = false
        loadingInstance.close()

        // 在完成地图保存的逻辑后，触发跨窗口的通信事件
        const crossBrowserWindowMessage = get(
          window,
          'electron.crossBrowserWindowMessage',
        )
        if (crossBrowserWindowMessage) {
          // @ts-expect-error
          crossBrowserWindowMessage.emit('map-editor-saved', mapName)
        }

        return true
      } catch (err) {
        console.log(`modify and save map error: `, err)
      }

      if (loadingInstance) {
        loadingInstance.close()
      }
      // 保存失败提示
      errorMessage({
        content: i18n.global.t('desc.tips.failedToSaveMap'),
      })
      this.isLoading = false
      return false
    },
    // 当加载地图完成后对应的业务逻辑
    onMapLoaded (mapName: string) {
      // 获取当前地图元素的中心点
      const roadStore = useRoadStore()
      const mapCenter = roadStore.getMapCenter()
      if (mapCenter) {
        // 计算默认正交相机对应的位置偏移量
        const cameraPos = {
          x: mapCenter.x + oCameraOffset.x,
          y: mapCenter.y + oCameraOffset.y,
          z: mapCenter.z + oCameraOffset.z,
        }

        // 切换当前相机的视角看相指定的地图中心点
        root3d.updateCamera({
          position: cameraPos,
          target: mapCenter,
        })
      }

      // 打开地图，默认就渲染对应的地图元素，没有进入任何编辑模式
      // 向操作记录中推入第一条记录，默认第一条记录不能撤销
      const historyStore = useHistoryStore()
      historyStore.save({
        title: `${i18n.global.t('actions.file.load')} : ${mapName}`,
        diff: false,
      })

      // 默认打开道路编辑模式
      const pluginStore = usePluginStore()
      pluginStore.activate('editRoad')
    },

    // ---------- 云端版接口 ----------
    async loadMapInCloud (params: {
      mapName: string
      version: string
      mapId: string
      mapGroupId?: string
    }) {
      const { mapName, version, mapId, mapGroupId = '' } = params
      if (!mapName) return false
      this.isLoading = true

      try {
        // 加载地图数据之前，先清空当前场景
        this.resetStore()
        const openRes = await openHadmapInCloud({ mapName, version })
        if (!openRes || get(openRes, 'data.message') !== 'ok') {
          throw new Error(
            `api open hadmap in cloud environment error. error info`,
          )
        }
        // 云端版返回的是字符串的数据
        const parsedData = await JSON.parse(get(openRes, 'data.data'))

        // 深拷贝一份地图文件源数据
        const originData = parsedData.data as biz.ICommonMapElements
        setOriginMapCache(cloneDeep(originData))

        // 更新当前打开的地图名称
        this.openedFile = mapName

        // 云端版没有预设地图的说法
        this.isPreset = false
        // 判断打开的地图，是否是第三方地图
        this.mapVersion = get(originData, 'header.version') || MapVersion.v2
        // 打开的地图类型
        this.mapType = get(originData, 'header.map_type') || ''

        this.version = version
        this.mapId = mapId
        this.coskey = parsedData.baseVersion || ''
        this.mapGroupId = mapGroupId

        // 解析地图文件中的数据，同步到前端状态
        await parseMapElements(originData)

        this.onMapLoaded(mapName)

        // 如果是广汽云端版，则同步 crg 有效配置
        if (
          globalConfig.projectName === 'gac_develop' ||
          globalConfig.projectName === 'gac_release'
        ) {
          this.syncCrgConfigInCloud()
        }

        this.isLoading = false
        return true
      } catch (err) {
        console.log(`load map ${name} content error: `, err)
      }
      // 如果加载地图失败则重置状态
      this.openedFile = ''
      this.isPreset = false
      this.mapVersion = ''
      this.mapType = ''

      setOriginMapCache(null)
      this.isLoading = false
      return false
    },
    // 【云端版】增量保存地图文件
    async saveDiffMapInCloud (mapName: string, isAutoSave: boolean = false) {
      this.isLoading = true
      let loadingInstance
      try {
        const diffRes = await getDiffElementData()
        if (!diffRes) {
          this.isLoading = false
          // 提示地图没有更新，无须保存
          warningMessage({
            content: i18n.global.t('desc.tips.theMapIsNotUpdatedNoNeedToSave'),
          })
          return false
        }

        // 增量保存由于没有弹出上层对话框，需要设置一个全屏的 loading 阻止用户交互
        loadingInstance = fullScreenLoading(i18n.global.t('desc.tips.saving'))

        const { cmdOptions, roads, lanelinks, objects } = diffRes
        // 组装要透传的数据
        const payload = {
          desc: mapName,
          name: mapName,
          mapData: {
            data: {
              map: mapName,
              datas: cmdOptions,
              roads,
              lanelinks,
              objects,
            },
            geometry: null,
            name: mapName,
            preset: false,
            editorVersion: '2.0',
            // 增量保存时，透传 coskey 给后端
            baseVersion: this.coskey,
          },
          id: this.mapId,
          editorVersion: '2.0',
        }

        // 如果是广汽云端版
        if (
          globalConfig.projectName === 'gac_develop' ||
          globalConfig.projectName === 'gac_release'
        ) {
          payload.labelList = []
          payload.mapGroupId = this.mapGroupId
          payload.mapType = 2
          payload.mapImportFormat = 0
          payload.importFormatVersion = 'opendrive1.4'

          // 补充当前道路关联的 crg 数据
          const crgMap = this.getRoadCrgConfigInCloud()
          if (crgMap) {
            payload.crgMap = crgMap
          }
        }

        const saveDiffRes = await saveHadmapInCloud(payload)
        if (!saveDiffRes || get(saveDiffRes, 'data.message') !== 'ok') {
          throw new Error(
            `api save hadmap in cloud environment error. error info: `,
          )
        }

        // 增量保存完成后，更新当前地图的属性
        // 更新 cos 地址
        const baseVersion = get(
          saveDiffRes,
          'data.data.mapResponse.baseVersion',
        )
        if (baseVersion) {
          this.coskey = baseVersion
        }
        // 更新 version
        const version = get(saveDiffRes, 'data.data.version')
        if (version) {
          this.version = version
        }
        // 更新 mapId
        const mapId = get(saveDiffRes, 'data.data.mapId')
        if (mapId) {
          this.mapId = mapId
        }

        // 更新 url
        this.updateURLParams({
          mapName,
          version,
          mapId,
          mapGroupId: this.mapGroupId,
        })

        if (isAutoSave) {
          // 自动保存完成后提示成功
          successMessage({
            content: i18n.global.t('desc.tips.AutosaveTheMapSuccessfully'),
          })
        } else {
          // 手动保存完成后提示成功
          successMessage({
            content: i18n.global.t('desc.tips.saveMapSuccessfully'),
          })
        }

        // 重置一部分操作状态
        this.resetStoreAfterSave({
          mapName,
        })

        // 基于当前 diff 的结果，更新本地源数据的缓存
        updateOriginMapCacheByDiff(diffRes)

        this.isLoading = false
        loadingInstance.close()

        // 如果是云端环境，则重启自动保存地图的计时器
        if (globalConfig.isCloud) {
          this.initAutoSaveTimer()
        }

        return true
      } catch (err) {
        console.log('save diff map in cloud environment error: ', err)
      }

      if (loadingInstance) {
        loadingInstance.close()
      }

      if (isAutoSave) {
        // 自动保存失败提示
        errorMessage({
          content: i18n.global.t('desc.tips.failedToAutosaveMap'),
        })
      } else {
        // 手动保存失败提示
        errorMessage({
          content: i18n.global.t('desc.tips.failedToSaveMap'),
        })
      }

      // 如果是云端环境，则重启自动保存地图的计时器
      if (globalConfig.isCloud) {
        this.initAutoSaveTimer()
      }

      this.isLoading = false
      return false
    },
    // 【云端版】全量保存地图文件
    async saveAllMapInCloud (params: {
      mapName: string
      isSaveAs: boolean
      mapGroupId?: string
      mapType: biz.IMapType
    }) {
      const { mapName, isSaveAs, mapGroupId = '', mapType } = params
      this.isLoading = true
      try {
        // 获取要保存的全量数据
        const _content = await pickUpMapElementData()
        // 组装要透传的内容
        const payload = {
          desc: mapName,
          name: mapName,
          mapData: {
            data: {
              ..._content,
              // 云平台保存 header，跟 roads、lanelinks 等同层级
              header: {
                version: MapVersion.v2,
                map_type: mapType,
              },
            },
            geometry: null,
            name: mapName,
            preset: false,
            // 临时在mapData中放 editorVersion 用于调试
            editorVersion: '2.0',
          },
          editorVersion: '2.0',
        }
        // 如果是广汽云端版
        if (
          globalConfig.projectName === 'gac_develop' ||
          globalConfig.projectName === 'gac_release'
        ) {
          payload.labelList = []
          payload.mapGroupId = mapGroupId
          payload.mapType = 2
          payload.mapImportFormat = 0
          payload.importFormatVersion = 'opendrive1.4'

          // 补充当前道路关联的 crg 数据
          const crgMap = this.getRoadCrgConfigInCloud()
          if (crgMap) {
            payload.crgMap = crgMap
          }
        }

        const saveAllRes = await saveHadmapInCloud(payload)
        if (!saveAllRes || get(saveAllRes, 'data.message') !== 'ok') {
          throw new Error(
            `api save hadmap in cloud environment error. error info: `,
          )
        }

        this.isLoading = false

        // 保存成功提示
        successMessage({
          content: i18n.global.t('desc.tips.saveMapSuccessfully'),
        })

        // 全量保存，只有【新建+保存】这一种情况，才需要更新地图状体
        // 因为另存为是生成新的文件，跟当前文件无关
        if (!this.openedFile && !isSaveAs) {
          this.resetStoreAfterSave({
            mapName,
          })

          // 更新 cos 地址
          const baseVersion = get(
            saveAllRes,
            'data.data.mapResponse.baseVersion',
          )
          if (baseVersion) {
            this.coskey = baseVersion
          }
          // 更新 version
          const version = get(saveAllRes, 'data.data.version')
          if (version) {
            this.version = version
          }
          // 更新 mapId
          const mapId = get(saveAllRes, 'data.data.mapId')
          if (mapId) {
            this.mapId = mapId
          }
          // 更新当前地图分组
          if (!this.mapGroupId) {
            this.mapGroupId = String(mapGroupId)
          }

          this.updateURLParams({
            mapName,
            version,
            mapId,
            mapGroupId,
          })

          // 将保存的地图数据，缓存为已打开地图的数据
          setOriginMapCache(_content)
        }

        // 如果是云端环境，则重启自动保存地图的计时器
        if (globalConfig.isCloud) {
          this.initAutoSaveTimer()
        }

        return true
      } catch (err) {
        console.log('save all map in cloud environment error. ', err)
      }

      // 保存失败提示
      errorMessage({
        content: i18n.global.t('desc.tips.failedToSaveMap'),
      })

      // 如果是云端环境，则重启自动保存地图的计时器
      if (globalConfig.isCloud) {
        this.initAutoSaveTimer()
      }

      this.isLoading = false
      return false
    },
    // 【云端版】加载完地图后，获取每条道路在云端关联的 crg，同步有效的 crg 配置
    async syncCrgConfigInCloud () {
      if (!this.openedFile || !this.version) return
      try {
        // 获取当前地图所有道路关联的 crg 配置
        const { data } = await getRoadCrgRela(this.openedFile, this.version)
        if (data.message !== 'ok') {
          throw new Error(`api get road crg relationship error.`)
        }
        const crgMap = get(data, 'data.crgMap') || {}

        const roadStore = useRoadStore()
        for (const id of roadStore.ids) {
          const road = getRoad(id)
          if (!road) continue

          const originCrgMap = new Map()
          // 获取某一条road对应的crg配置
          const originRoadCrg = road.crgConfig || []
          for (const option of originRoadCrg) {
            if (!originCrgMap.has(option.file)) {
              originCrgMap.set(option.file, option)
            }
          }

          // 基于接口返回的云端有效的crg配置，更新当前道路的 crg配置
          const newCrgConfig: Array<biz.IOpenCrg> = []
          const validateCrgConfig = crgMap[id] || []
          for (const option of validateCrgConfig) {
            const { fileKey, name, id: crgId, version } = option
            const _version = `v${version}.0`
            if (originCrgMap.has(fileKey)) {
              // 如果该配置在原有的 crg 配置中能匹配上，则直接使用
              const originCrgOption = originCrgMap.get(fileKey) as biz.IOpenCrg
              newCrgConfig.push({
                file: fileKey,
                showName: name,
                id: String(crgId),
                version: _version,
                ...pick(originCrgOption, [
                  'orientation',
                  'mode',
                  'purpose',
                  'sOffset',
                  'tOffset',
                  'zOffset',
                  'zScale',
                  'hOffset',
                ]),
              })
            } else {
              // 否则提供默认的 crg 配置
              newCrgConfig.push({
                file: fileKey,
                showName: name,
                id: String(crgId),
                version: _version,
                orientation: 'same', // 默认为同向
                mode: 'attached', // 默认为相对附着
                purpose: 'elevation', // 默认为高程
                sOffset: '0',
                tOffset: '0',
                zOffset: '0',
                zScale: '1',
                hOffset: '0',
              })
            }
          }

          // 不管新的crg配置是否有数据，始终替换原有的 crg 配置
          road.crgConfig = newCrgConfig

          // 手动清空缓存
          originCrgMap.clear()
        }
      } catch (err) {
        console.log('sync crg config in cloud error.', err)
      }
    },
    // 【云端版】保存地图时，获取当前地图每一条道路关联的 crg 配置
    getRoadCrgConfigInCloud () {
      const roadStore = useRoadStore()
      const crgMap = {}
      for (const id of roadStore.ids) {
        const road = getRoad(id)
        if (!road) continue

        // 读取当前道路的 crg 配置
        if (road.crgConfig && road.crgConfig.length > 0) {
          // 以 roadId 为 key, 当前道路所有 crg 的 id 的数据集合作为 value，进行透传
          crgMap[id] = road.crgConfig.map(option => Number(option.id))
        }
      }
      if (Object.keys(crgMap).length < 1) return null

      return crgMap
    },
    // 保存后，更新当前 url 的querystring 内容
    updateURLParams (params: {
      mapName: string
      mapId: string
      version: string
      mapGroupId: string
    }) {
      const { mapName, mapId, version, mapGroupId } = params
      const url = new URL(window.location.href)
      url.searchParams.set('name', mapName)
      url.searchParams.set('id', mapId)
      url.searchParams.set('version', version)
      url.searchParams.set('mapGroupId', mapGroupId)
      url.searchParams.delete('creatorId')

      window.history.pushState({}, '', url.toString())
    },

    // 【云端版】跳转 v2x 模型管理页面
    openV2XManagerPage () {
      const url = new URL(window.location.href)
      // v2x 模型管理页面的路径
      url.pathname = '/models-manage/v2x'
      window.open(url.toString())
    },
    // 【云端版】通过接口请求，同步云端已经配置好的 V2X 设备的列表
    async syncV2XList () {
      try {
        // 调用接口请求V2X设备列表
        const res = await getV2XDeviceList()
        if (!res || String(get(res, 'data.message')).toLowerCase() !== 'ok') {
          throw new Error(
            `api get v2x device list in cloud environment error. error info: `,
          )
        }
        const { sensors = [] } = get(res, 'data.data') || {}
        const rsu: Array<string> = []
        const camera: Array<string> = []
        const lidar: Array<string> = []
        const radar: Array<string> = []
        sensors.forEach((option) => {
          const idx = String(option.idx)
          if (option.type === 'V2xCamera') {
            camera.push(idx)
          } else if (option.type === 'RSU') {
            rsu.push(idx)
          } else if (option.type === 'V2xRadar') {
            radar.push(idx)
          } else if (option.type === 'V2xTraditionalLidar') {
            lidar.push(idx)
          }
        })
        // 更新v2x设备列表
        this.v2xDeviceList = {
          rsu,
          camera,
          lidar,
          radar,
        }
      } catch (err) {
        console.log('sync v2x device list error:', err)
      }
    },
    // 【云端版】通过接口请求，同步云端已经配置好的 crg 列表
    async syncCrgList (parentId: number) {
      const crgList = []
      try {
        const { data } = await getMapCrgGroupListInCloud(parentId)
        if (data.message !== 'ok') {
          throw new Error(
            `get map crg group list error. parentId is ${parentId}`,
          )
        }
        const list = data.data

        for (let i = 0; i < list.length; i++) {
          const { id, name, hasChildren, leaf } = list[i]
          const option = {
            id: String(id),
            name,
            key: `${name}_${id}`,
            hasChildren,
            leaf,
            children: [],
            isCrgFile: false,
          }

          crgList.push(option)
        }
      } catch (err) {
        console.log(err)
      }

      // 如果是根节点的请求，在完成数据请求后保存对应的数据
      if (parentId === 0) {
        this.crgList = [
          {
            id: '-1',
            name: i18n.global.t('desc.editCrg.allData'),
            key: 'root',
            hasChildren: true,
            leaf: false,
            children: crgList,
            isCrgFile: false,
          },
        ]
      }
      return crgList
    },
  },
})
