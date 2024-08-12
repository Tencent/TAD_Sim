import axios from 'axios'
import { MapVersion } from '@/utils/business'
import { globalConfig } from '@/utils/preset'
import { getServiceSessionId } from '@/utils/tools'

// 接口请求超时暂定 25s【备注：之前由于 ubuntu 版本存在接口请求 19s 才成功情况，于是将超时时间从 15s 调整到 25s】
const timeout = 25000

// 单机版请求实例
const axiosInstance = axios.create({
  baseURL: 'http://127.0.0.1:9000/',
  timeout,
})

const { isDev = false } = globalConfig

// 云端版请求实例
const cloudAxiosInstance = axios.create({
  // 如果是本地测试环境，需要对云端接口做一些调整
  baseURL: isDev ? '/api' : '',
  timeout,
})

// 单机版请求的地址
const defaultRequestUrls = {
  createSession: '/mapedit/create_session', // 创建会话
  openHadmap: '/mapedit/open_hadmap', // 打开地图（只加载，不获取数据）
  getHadmap: '/mapedit/get_hadmap', // 获取地图数据
  createHadmap: '/mapedit/create_hadmap', // 新建地图
  modifyHadmap: '/mapedit/modify_hadmap', // 修改地图
  saveHadmap: '/mapedit/save_hadmap', // 保存地图
  getHadMapList: '/hadmaplist', // 获取地图列表
  importHadmap: '/hadmaps/upload', // 导入地图
  exportHadmap: '/hadmaps/download', // 导出地图
  deleteHadmap: '/hadmaps/delete', // 删除地图
  getProgress: '/hadmaps/getprogressval', // 获取进度条状态
  stopProgress: '/hadmaps/stopprogress', // 终止进度条操作
  getModelList: '/mapmodel/getmodellist', // 获取本地自定义上传模型列表
  uploadModel: '/mapmodel/upload', // 上传模型文件
  uploadThumb: '/mapmodel/upload_thumbnail', // 上传模型对应的缩略提
  saveModel: '/mapmodel/save', // 保存(包括新增和更新)自定义上传的模型
  deleteModel: '/mapmodel/delete', // 删除自定义模型
}

// 云端版请求的地址
const cloudRequestUrls = {
  // 云端版更新的接口
  getHadmap: '/simService/maps/cloudEditor/open', // 获取地图文件内容
  saveHadmap: '/simService/maps/cloudEditor/save', // 保存地图文件
  getMapCatalogue: '/simService/dataPlatform/forward/v1/map/catalogue', // 获取地图目录
  getV2XDeviceList:
    '/simService/scenes/cloudEditor/forward/getcatalog?type=V2X', // 获取V2X设备列表
  getMapCrgGroupList: '/simService/mapCrgGroup/list', // 获取地图 crg 组的列表
  getMapCrgList: '/simService/mapCrg/list', // 获取某一个组下的所有 crg 文件列表
  getCrgDetail: '/simService/mapCrg/detail', // 获取一个 crg 文件的详情
  getRoadCrgRela: '/simService/maps/cloudEditor/roadCrgRela', // 获取一个地图中，所有道路在云平台 crg 绑定关系
}

/**
 * 创建会话
 */
export async function createSession () {
  const res = await axiosInstance.get(defaultRequestUrls.createSession)
  return res
}
/**
 * 打开地图（不获取数据）
 * @param mapName
 * @returns
 */
export async function openHadmap (mapName: string) {
  const sessionId = getServiceSessionId()
  const res = await axiosInstance.post(defaultRequestUrls.openHadmap, {
    sessionid: sessionId,
    map: mapName,
  })
  return res
}
/**
 * 读取地图信息
 * @param mapName
 * @returns
 */
export async function getHadmap (mapName: string) {
  const sessionId = getServiceSessionId()
  const res = await axiosInstance.post(defaultRequestUrls.getHadmap, {
    sessionid: sessionId,
    map: mapName,
  })
  return res
}

/**
 * 获取地图文件列表
 * @returns
 */
export async function getMapList () {
  const res = await axiosInstance.get(defaultRequestUrls.getHadMapList)
  return res
}

interface IMapSource {
  name: string
}
/**
 * 导出地图
 * @param params
 */
export async function exportHadmap (params: {
  dest: string
  sources: Array<IMapSource>
}) {
  const { dest, sources } = params
  const res = await axiosInstance.post(defaultRequestUrls.exportHadmap, {
    dest,
    sources,
  })
  return res
}

interface IImportMapSource extends IMapSource {
  // true：新增和替换原有的；false：保留原有的，同时将新增的换一个名称保留
  replaceFlag: boolean
}
/**
 * 导入地图
 * @param params
 */
export async function importHadmap (params: {
  dir: string
  sources: Array<IImportMapSource>
}) {
  const { dir, sources } = params
  const res = await axiosInstance.post(defaultRequestUrls.importHadmap, {
    dir,
    sources,
    type: 'files',
  })
  return res
}

/**
 * 删除地图
 * @param sources
 */
export async function deleteHadmap (sources: Array<IMapSource>) {
  const res = await axiosInstance.post(defaultRequestUrls.deleteHadmap, sources)
  return res
}

/**
 * 新建地图
 * create_hadmap 接口调用完已经保存新地图了，不需要调用 save_hadmap
 * @param params
 * @returns
 */
export async function createHadmap (params: {
  name: string
  mapType: biz.IMapType
  data: biz.ICommonMapElements
}) {
  const { name, mapType, data } = params
  const sessionId = getServiceSessionId()
  const res = await axiosInstance.post(defaultRequestUrls.createHadmap, {
    sessionid: sessionId,
    map: name,
    data: {
      ...data,
      // header 在单机版和云平台，都放在 data 中，跟 roads、lanelinks 等同层级
      header: {
        // 透传地图版本和类型
        version: MapVersion.v2,
        map_type: mapType,
      },
    },
  })
  return res
}

/**
 * 修改已打开的地图数据
 * @param params
 */
export async function modifyHadmap (params: {
  name: string
  datas: Array<biz.ICmdOption>
  roads: Array<biz.ICommonRoad>
  lanelinks: Array<biz.ICommonLaneLink>
  objects: Array<biz.ICommonObject>
}) {
  const { name, datas, roads, lanelinks, objects } = params
  const sessionId = getServiceSessionId()
  const res = await axiosInstance.post(defaultRequestUrls.modifyHadmap, {
    sessionid: sessionId,
    map: name,
    datas,
    roads,
    lanelinks,
    objects,
  })
  return res
}

/**
 * 保存地图
 */
export async function saveHadmap (params: { srcmap: string, dstmap: string }) {
  const { srcmap = '', dstmap = '' } = params
  const sessionId = getServiceSessionId()
  const data = {
    sessionid: sessionId,
    dstmap,
    srcmap,
  }
  const res = await axiosInstance.post(defaultRequestUrls.saveHadmap, data)
  return res
}

/**
 * 保存地图 - 云端版
 * @param params
 */
export async function saveHadmapInCloud (payload: any) {
  const res = await cloudAxiosInstance.post(
    cloudRequestUrls.saveHadmap,
    payload,
  )
  return res
}

/**
 * 打开地图 - 云端版
 * @param params
 */
export async function openHadmapInCloud (params: {
  mapName: string
  version: string
}) {
  const { mapName, version } = params
  const res = await cloudAxiosInstance.get(
    `${cloudRequestUrls.getHadmap}/${mapName}?version=${version}&editorVersion=2.0`,
  )
  return res
}

/**
 * 获取地图目录 - 云端版
 */
export async function getMapCatalogueInCloud () {
  const res = await cloudAxiosInstance.get(
    `${cloudRequestUrls.getMapCatalogue}`,
  )
  return res
}

/**
 * 获取V2X仿真设备列表 - 云端版
 */
export async function getV2XDeviceList () {
  const res = await cloudAxiosInstance.get(
    `${cloudRequestUrls.getV2XDeviceList}`,
  )
  return res
}

/**
 * 获取地图管理平台中 crg group 的列表
 * @param parentId
 */
export async function getMapCrgGroupListInCloud (parentId: number) {
  const res = await cloudAxiosInstance.get(
    `${cloudRequestUrls.getMapCrgGroupList}?parentId=${parentId}`,
  )
  return res
}

/**
 * 获取地图管理平台中，某一个组下的所有 crg 文件列表
 * @param groupId
 */
export async function getMapCrgListInCloud (groupId: number) {
  const res = await cloudAxiosInstance.post(cloudRequestUrls.getMapCrgList, {
    groupId,
    pageNo: 1,
    pageSize: 9999,
  })
  return res
}

/**
 * 获取一个 crg 文件的详情（包含版本信息）
 * @param id
 */
export async function getCrgDetailInCloud (id: number) {
  const res = await cloudAxiosInstance.get(
    `${cloudRequestUrls.getCrgDetail}?rootId=${id}`,
  )
  return res
}

/**
 * 获取一个地图中所有道路跟crg的绑定关系
 * @param mapName
 * @param version
 */
export async function getRoadCrgRela (mapName: string, version: string) {
  const res = await cloudAxiosInstance.get(
    `${cloudRequestUrls.getRoadCrgRela}?mapName=${mapName}&version=${version}`,
  )
  return res
}

// ---------- 自定义三维模型导入 start ----------
/**
 * 获取本地自定义模型列表
 */
export async function getCustomModelList () {
  const res = await axiosInstance.get(`${defaultRequestUrls.getModelList}`)
  return res
}

/**
 * 上传自定义模型文件
 * @param modelPath
 */
export async function uploadCustomModel (modelPath: string) {
  const res = await axiosInstance.post(`${defaultRequestUrls.uploadModel}`, {
    model_path: modelPath,
  })
  return res
}

/**
 * 上传模型对应的缩略图
 * @param params
 */
export async function uploadModelThumb (params: {
  file: File
  modelDir: string
}) {
  const { file, modelDir } = params
  const form = new FormData()
  form.append('file', file)
  form.append('model_dir', modelDir)
  const res = await axiosInstance.post(
    `${defaultRequestUrls.uploadThumb}`,
    form,
  )
  return res
}

/**
 * 保存自定义上传的模型
 * @param params
 */
export async function saveCustomModel (params: {
  id: string
  name: string
  variable: string
  modelPath: string
  length: number
  width: number
  height: number
  posX: number
  posY: number
  posZ: number
  rawFileName: string
}) {
  const {
    id,
    variable,
    name,
    modelPath,
    length,
    width,
    height,
    posX,
    posY,
    posZ,
    rawFileName,
  } = params
  const res = await axiosInstance.post(`${defaultRequestUrls.saveModel}`, {
    id,
    data: {
      variable,
      name,
      model3d: modelPath,
      boundingBox: {
        center: {
          x: posX,
          y: posY,
          z: posZ,
        },
        dimensions: {
          length,
          width,
          height,
        },
      },
      rawFileName,
    },
  })
  return res
}

/**
 * 删除自定义模型
 */
export async function deleteCustomModel (variable: string) {
  const res = await axiosInstance.post(`${defaultRequestUrls.deleteModel}`, {
    variable,
  })
  return res
}
// ---------- 自定义三维模型导入 end ----------
