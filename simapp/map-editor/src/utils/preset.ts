import NP from 'number-precision'
import { merge } from 'lodash'
import { ifCloud, ifDev, ifElectron } from './tools'
import moduleConfig from '@/config/projectConfig.json'
import { getPermission } from '@/utils/permission'

function initPreset () {
  if (window) {
    window.oncontextmenu = function (e) {
      e.preventDefault()
    }
  }

  // number-precision 精度配置
  NP.enableBoundaryChecking(false)
}

// 获取运行时的一些配置
const globalConfig = getGlobalConfig()
function getGlobalConfig () {
  // 生产环境 or 开发环境
  const isDev = ifDev()
  const isElectron = ifElectron()
  const isCloud = ifCloud()

  // 读取环境变量中当前发版的项目配置
  const { VITE_APP_PROJECT_VERSION = '' } = import.meta.env
  const currentProject = VITE_APP_PROJECT_VERSION || 'default'
  // 根据当前项目的名称，提供定制化的插件或功能配置能力
  const oldPermissionConfig = (moduleConfig as any)[currentProject]

  // 根据 web-config.json 进行权限处理
  const permissionConfig = {
    file: {
      open: getPermission('action.mapEditor.open.enable'),
      create: getPermission('action.mapEditor.create.enable'),
      save: getPermission('action.mapEditor.save.enable'),
      saveAs: getPermission('action.mapEditor.saveAs.enable'),
      import: getPermission('action.mapEditor.import.enable'),
      export: getPermission('action.mapEditor.export.enable'),
      delete: getPermission('action.mapEditor.delete.enable'),
      saveSqlite: getPermission('action.mapEditor.fileType.sqlite.show'),
    },
    plugin: {
      editRoad: getPermission('router.mapEditor.road.show'),
      editJunction: getPermission('router.mapEditor.junction.show'),
      editLaneAttr: getPermission('router.mapEditor.lane.show'),
      editLaneWidth: getPermission('router.mapEditor.laneWidth.show'),
      editLaneNumber: getPermission('router.mapEditor.laneNumber.show'),
      editBoundary: getPermission('router.mapEditor.boundary.show'),
      editLink: getPermission('router.mapEditor.laneLink.show'),
      editObject: getPermission('router.mapEditor.models.show'),
      // editSection: false,
      editElevation: getPermission('router.mapEditor.elevation.show'),
      editSignalControl: getPermission('router.mapEditor.signalControl.show'),
      editCircleRoad: getPermission('router.mapEditor.circleRoad.show'),
      editCrg: getPermission('router.mapEditor.openCRG.show'),
    },
    function: {
      supportCustomModel: getPermission('router.mapEditor.importCustomModels.show'),
    },
  }

  const exposeModule = merge({}, oldPermissionConfig, permissionConfig)

  return {
    isDev,
    isElectron,
    projectName: currentProject,
    exposeModule,
    isCloud,
  }
}

export { initPreset, globalConfig }
