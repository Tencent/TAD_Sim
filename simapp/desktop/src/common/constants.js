const path = require('path')
const { app } = require('electron')
const { getFilePath, getProcessType } = require('./util')

function getAppUserDataPath () {
  const processType = getProcessType()
  let AppUserDataPath = ''

  if (processType === 'main') {
    AppUserDataPath = app.getPath('userData')
  } else if (processType === 'renderer') {
    const userDataDir = process.argv[2]
    // 目前为手动将最后一个 cache 删除，需要思考更合理的方法设置渲染进程的 userData
    AppUserDataPath = userDataDir
      .replace('--user-data-dir=', '')
      .replace(/cache$/, '')
  }

  return AppUserDataPath
}

// 当前项目根目录，对应 desktop
const AppPath = getFilePath('../..')
// 用户数据目录
const AppUserDataPath = getAppUserDataPath()

// 当前项目下的路径
const servicePath = getFilePath('../../service')
const serviceDepsPath = path.join(servicePath, 'simdeps')
const ProjectPath = {
  appPackageJson: getFilePath('../../package.json'),
  service: servicePath,
  serviceDeps: serviceDepsPath,
  serviceNodeDeps: path.join(serviceDepsPath, 'nodedeps'),
  serviceUpgradeTools: path.join(servicePath, 'upgrade_tools'),
  serviceScenario: path.join(servicePath, 'scenario_service'),
  serviceKpi: path.join(servicePath, 'default_grading_kpi.json'),
}

// 用户数据目录下的路径
const serviceDataPath = path.join(AppUserDataPath, './data/service_data') // 使用升级后的目录
const UserDataPath = {
  // 使用到升级后到目录路径
  batchGenerateScenario: path.join(AppUserDataPath, './data/scenario/scene/batch_scene'), // TODO 目前 tadsim 文件夹里没有，应该放到哪儿呢？暂定 data 目录
  serviceData: serviceDataPath,
  kpiReport: path.join(serviceDataPath, 'sim_data'),
  warningConfig: path.join(AppUserDataPath, './sys/warning_config.json'),
  sensorConfig: path.join(
    AppUserDataPath,
    './data/scenario/sensors/config.json',
  ),
  systemSensorConfigBackup: path.join(
    AppUserDataPath,
    './sys/scenario/sensors/config.json',
  ),
  serviceConfig: path.join(AppUserDataPath, './sys/local_service.config'),
  defaultKpi: path.join(AppUserDataPath, './sys/default_grading_kpi.json'),
  userLog: path.join(AppUserDataPath, './log/user_log'), // 确保替换 local_service.config 文件的字段，是升级后的目录 ./log/user_log
  recentOpened: path.join(AppUserDataPath, './cache/recent_opened.json'),
  scenario: path.join(AppUserDataPath, './sys/scenario'), // TODO 不确定改成 ./sys/scenario 是否有影响
  upgradeLog: path.join(AppUserDataPath, './cache/txsim_upgrade.log'),
  debugLog: path.join(AppUserDataPath, './cache/debug_log'),
  debugLogScenarioService: path.join(
    AppUserDataPath,
    './cache/debug_log/txsim_scenario_service.log',
  ),
}

// cache 目录下的内容，每次更新都直接清空
const CacheConfig = [
  {
    target: path.resolve(AppUserDataPath, './cache/recent_opened.json'),
    sourceContent: 'recentOpened',
    isFile: true,
  },
  {
    // 使用 ./cache/module_log 用于区分 user_log
    target: path.resolve(AppUserDataPath, './cache/module_log'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './cache/debug_log'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './cache/scenario/log'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './cache/scenario/scene_log'),
    isFile: false,
  },
]

const SystemConfig = [
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/gis_images'),
    origin: path.resolve(AppUserDataPath, './scenario/gis_images'),
    source: '',
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/gis_models'),
    origin: path.resolve(AppUserDataPath, './scenario/gis_models'),
    source: '',
    isFile: false,
  },
  {
    // 保存系统预设的地图
    target: path.resolve(AppUserDataPath, './sys/scenario/hadmap'),
    source: path.resolve(AppPath, './scenario/hadmap'),
    isFile: false,
  },
  {
    // 系统预设的模型
    target: path.resolve(AppUserDataPath, './sys/scenario/models'),
    source: path.resolve(AppPath, './scenario/models'),
    isFile: false,
  },
  {
    // 保存系统预设的场景
    target: path.resolve(AppUserDataPath, './sys/scenario/scene'),
    source: path.resolve(AppPath, './scenario/scene'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/OpenX'),
    source: path.resolve(AppPath, './scenario/OpenX'),
    isFile: false,
  },
  {
    target: path.resolve(
      AppUserDataPath,
      './sys/scenario/sensors/sensor_global.xml',
    ),
    source: path.resolve(AppPath, './scenario/sensors/sensor_global.xml'),
    isFile: true,
  },
  // 全局环境配置从安装目录拷贝到 sys 目录下
  // {
  //   target: path.resolve(
  //     AppUserDataPath,
  //     './sys/scenario/sensors/environment_global.xml',
  //   ),
  //   source: path.resolve(AppPath, './scenario/sensors/environment_global.xml'),
  //   isFile: true,
  // },
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/template'),
    source: path.resolve(AppPath, './scenario/template'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/config.xml'),
    source: path.resolve(AppPath, './scenario/config.xml'),
    isFile: true,
  },
  {
    target: path.resolve(
      AppUserDataPath,
      './sys/service_data/model_in_the_loop',
    ),
    source: path.resolve(AppPath, './service/data/model_in_the_loop'),
    isFile: false,
  },
  {
    target: path.resolve(
      AppUserDataPath,
      './sys/service_data/model_in_the_loop.tar',
    ),
    source: path.resolve(AppPath, './service/data/model_in_the_loop.tar'),
    isFile: true,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/service_data/script'),
    source: path.resolve(AppPath, './service/data/script'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/default_grading_kpi.json'),
    origin: path.resolve(AppUserDataPath, './default_grading_kpi.json'),
    source: path.resolve(AppPath, './service/default_grading_kpi.json'),
    isFile: true,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/warning_config.json'),
    origin: path.resolve(AppUserDataPath, './warning_config.json'),
    sourceContent: 'warningConfig',
    isFile: true,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/service_data/carsim_para'),
    source: path.resolve(AppPath, './service/data/carsim_para'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/hadmap_config.xml'),
    source: path.resolve(AppPath, './scenario/hadmap_config.xml'),
    isFile: true,
  },
  {
    // 版本更新判断新目录下 sys/scenario/simu_scene.sqlite 文件是否存在，如果存在则删除
    // 服务端会判断该文件是否存在，如果没有会自动生成
    // 删除之前，先备份到 data/old_scenario 目录中
    target: path.resolve(AppUserDataPath, './data/old_scenario/simu_scene.sqlite'),
    source: path.resolve(AppUserDataPath, './sys/scenario/simu_scene.sqlite'),
    shouldDelete: true,
    isFile: true,
  },
  {
    // 在 sys 目录下也保存一份 sensorConfig 的备份
    target: path.resolve(AppUserDataPath, './sys/scenario/sensors/config.json'),
    origin: path.resolve(AppUserDataPath, './scenario/sensors/config.json'),
    sourceContent: 'sensorConfig',
    isFile: true,
  },
  // 在版本升级时，如果这个文件存在，需要拷贝到用户数据目录 sys/scenario 中
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/preset_scenesets.sqlite'),
    source: path.resolve(AppPath, './scenario/preset_scenesets.sqlite'),
    isFile: true,
  },
  {
    // 保存系统预设的Catalogs
    target: path.resolve(AppUserDataPath, './sys/scenario/Catalogs'),
    source: path.resolve(AppPath, './service/Catalogs'),
    isFile: false,
  },
  // 播放页实时数据面板，预设的消息配置。封装 pb 路径，方便定制实时数据图表的显示
  {
    target: path.resolve(AppUserDataPath, './sys/pb_info.json'),
    source: path.resolve(AppPath, './service/pb_info.json'),
    isFile: true,
  },
  {
    target: path.resolve(AppUserDataPath, './sys/scenario/场景语义生成模板.xlsx'),
    source: path.resolve(AppPath, './scenario/场景语义生成模板.xlsx'),
    isFile: true,
  },
]

const UserDataConfig = [
  // 拷贝旧版本整个 scenario 目录到 data/old_scenario 中作为备份
  {
    target: path.resolve(AppUserDataPath, './data/old_scenario'),
    origin: path.resolve(AppUserDataPath, './scenario'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/hadmap'),
    origin: path.resolve(AppUserDataPath, './scenario/hadmap'),
    source: path.resolve(AppPath, './scenario/hadmap'),
    onlyUserData: true,
    isHadMap: true,
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/scene'),
    origin: path.resolve(AppUserDataPath, './scenario/scene'),
    source: path.resolve(AppPath, './scenario/scene'),
    onlyUserData: true,
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/Dynamics'),
    source: path.resolve(AppPath, './service/Catalogs/Dynamics'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/Environments'),
    source: path.resolve(AppPath, './service/Catalogs/Environments'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/MiscObjects'),
    source: path.resolve(AppPath, './service/Catalogs/MiscObjects'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/Pedestrians'),
    source: path.resolve(AppPath, './service/Catalogs/Pedestrians'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/Sensors'),
    source: path.resolve(AppPath, './service/Catalogs/Sensors'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/Vehicles'),
    source: path.resolve(AppPath, './service/Catalogs/Vehicles'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/tadsim_setting.conf'),
    source: path.resolve(AppPath, './service/Catalogs/tadsim_setting.conf'),
    isFile: true,
  },
  {
    target: path.resolve(
      AppUserDataPath,
      './data/scenario/sensors/sensor_global.xml',
    ),
    origin: path.resolve(
      AppUserDataPath,
      './scenario/sensors/sensor_global.xml',
    ),
    source: path.resolve(AppPath, './scenario/sensors/sensor_global.xml'),
    isFile: true,
  },
  // 用户目录环境配置文件也需要在版本升级时拷贝到新目录中，跟 sensor_global 文件同目录
  // 环境配置已挪到 Catalogs 目录
  // {
  //   target: path.resolve(
  //     AppUserDataPath,
  //     './data/scenario/sensors/environment_global.xml',
  //   ),
  //   origin: path.resolve(
  //     AppUserDataPath,
  //     './scenario/sensors/environment_global.xml',
  //   ),
  //   isFile: true,
  // },
  // 新增 service_data/display 目录的拷贝
  {
    target: path.resolve(AppUserDataPath, './data/service_data/display'),
    origin: path.resolve(AppUserDataPath, './service_data/display'),
    // ubuntu 中历史版本的 display 目录跟 service_data 同级，需要单独兼容
    // 并且 ubuntu 下拷贝目录名存在大写的情况，目标目录的名称也需要大写保持一致
    targetBackup: path.resolve(AppUserDataPath, './data/service_data/Display'),
    originBackup: path.resolve(AppUserDataPath, './Display'),
    isFile: false,
  },
  {
    target: path.resolve(AppUserDataPath, './data/service_data/grading.xml'),
    origin: path.resolve(AppUserDataPath, './scenario/grading.xml'),
    source: path.resolve(AppPath, './scenario/grading.xml'),
    isFile: true,
  },
  {
    target: path.resolve(AppUserDataPath, './data/service_data/sim_data'),
    origin: path.resolve(AppUserDataPath, './service_data/sim_data'),
    source: path.resolve(AppPath, './service/data/sim_data'),
    isFile: false,
  },
  // 并不是在启动项目第一时间创建的文件，而是通过后端的脚本生成
  {
    target: path.resolve(AppUserDataPath, './data/service.sqlite'),
    origin: path.resolve(AppUserDataPath, './service.sqlite'),
    isFile: true,
  },
  {
    target: path.resolve(
      AppUserDataPath,
      './data/scenario/sensors/config.json',
    ),
    origin: path.resolve(AppUserDataPath, './scenario/sensors/config.json'),
    sourceContent: 'sensorConfig',
    isFile: true,
  },
]

const UpdateUserDataConfig = [
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/Vehicles/VehicleCatalog.xosc'),
    source: path.resolve(AppPath, './service/Catalogs/Vehicles/VehicleCatalog.xosc'),
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/Pedestrians/PedestrianCatalog.xosc'),
    source: path.resolve(AppPath, './service/Catalogs/Pedestrians/PedestrianCatalog.xosc'),
  },
  {
    target: path.resolve(AppUserDataPath, './data/scenario/Catalogs/MiscObjects/MiscObjectCatalog.xosc'),
    source: path.resolve(AppPath, './service/Catalogs/MiscObjects/MiscObjectCatalog.xosc'),
  },
]

const UserCatalogsPath = path.resolve(AppUserDataPath, './data/scenario/Catalogs')
const SysCatalogsPath = path.resolve(AppUserDataPath, './sys/scenario/Catalogs')
const AppCatalogsPath = path.resolve(AppPath, './service/Catalogs')

const LogConfig = [
  {
    target: path.resolve(AppUserDataPath, './log/user_log'),
    origin: path.resolve(AppUserDataPath, './user_log'),
    isFile: false,
  },
]

const CrossBrowserWindowMessageChannel = 'cross-browser-window-message-channel'

module.exports = {
  AppPath,
  AppUserDataPath,
  ProjectPath,
  UserDataPath,
  CrossBrowserWindowMessageChannel,
  CacheConfig,
  UserDataConfig,
  UpdateUserDataConfig,
  SystemConfig,
  LogConfig,
  UserCatalogsPath,
  SysCatalogsPath,
  AppCatalogsPath,
}
