import renderDispatcher from '@/main3d/dispatcher'
import pluginManager from '@/plugins'
import { useFileStore } from '@/stores/file'
import root3d from '@/main3d'
import { globalConfig } from '@/utils/preset'
import { useModelCacheStore } from '@/stores/object/modelCache'

// 初始化主流程
export function initMainProcess () {
  try {
    const { isCloud, projectName, exposeModule } = globalConfig

    // 初始化渲染逻辑派发模块
    renderDispatcher.init()

    // 初始化插件管理模块
    pluginManager.init()

    const fileStore = useFileStore()
    // 在插件模块初始化完毕后，创建一个新场景
    fileStore.createNew()

    // 初始化将物体从列表栏拖拽到三维场景中的处理逻辑（依赖 dom 元素）
    root3d.initEditObjectHandler()

    // 如果是云端环境，解析 url 中的 queryString，加载对应的地图文件
    if (isCloud) {
      const url = new URL(window.location.href)
      const { searchParams } = url
      // 获取默认加载的地图文件名称
      const mapName = searchParams.get('name')
      const version = searchParams.get('version')
      const mapId = searchParams.get('id')
      const mapGroupId = searchParams.get('mapGroupId') || ''
      if (mapName && version && mapId) {
        // 如果文件名存在，则加载对应的地图内容跟你
        fileStore.loadMapInCloud({
          mapName,
          version,
          mapId,
          mapGroupId,
        })
      }

      // 云端环境，同步 V2X 设备列表【主线能力】
      fileStore.syncV2XList()

      // 云端环境，如果是广汽项目，则同步 crg 列表
      if (projectName === 'gac_develop' || projectName === 'gac_release') {
        // 初始的根节点id为0
        const parentId = 0
        fileStore.syncCrgList(parentId)
      }
    } else {
      // 单机版同步本地自定义模型列表
      if (exposeModule.function.supportCustomModel) {
        const modelCacheStore = useModelCacheStore()
        modelCacheStore.syncLocalCustomModel()
      }
    }
  } catch (err) {
    console.log('main process error: ', err)
  }
}

// 销毁主流程
export function disposeMainProcess () {
  root3d.dispose()
  pluginManager.dispose()
}
