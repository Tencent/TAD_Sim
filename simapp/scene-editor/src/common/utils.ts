import { cloneDeep, max } from 'lodash-es'
import type { Object3D } from 'three'
import { Box3, CanvasTexture, Mesh, Vector3 } from 'three'
import md5 from 'crypto-js/md5'
import type {
  CarModel,
  CatalogModel,
  CatalogParams,
  ObstacleModel,
  PedestrianModel,
} from 'models-manager/src/catalogs/class.ts'
import {
  CatalogCategory,
  InterfaceCategory,
} from 'models-manager/src/catalogs/class.ts'
import { join } from 'path-browserify'
import i18n from '@/locales'
import store from '@/store'

interface StringDictionary {
  [propName: string]: string
}

const chnNumChar = ['零', '一', '二', '三', '四', '五', '六', '七', '八', '九']
const chnUnitSection = ['', '万', '亿', '万亿', '亿亿']
const chnUnitChar = ['', '十', '百', '千']

/**
 * 将数字转换成中文数字
 * @param section
 */
function sectionToChinese (section: number) {
  let strIns = ''
  let chnStr = ''
  let unitPos = 0
  let zero = true
  while (section > 0) {
    const v = section % 10
    if (v === 0) {
      if (!zero) {
        zero = true
        chnStr = chnNumChar[v] + chnStr
      }
    } else {
      zero = false
      strIns = chnNumChar[v]
      strIns += chnUnitChar[unitPos]
      chnStr = strIns + chnStr
    }
    unitPos += 1
    section = Math.floor(section / 10)
  }
  return chnStr
}

/**
 * 将数字转换成中文数字
 * @param num
 */
function numberToChinese (num: number) {
  let unitPos = 0
  let strIns = ''
  let chnStr = ''
  let needZero = false

  if (num === 0) {
    return chnNumChar[0]
  }

  while (num > 0) {
    const section = num % 10000
    if (needZero) {
      chnStr = chnNumChar[0] + chnStr
    }
    strIns = sectionToChinese(section)
    strIns += (section !== 0) ? chnUnitSection[unitPos] : chnUnitSection[0]
    chnStr = strIns + chnStr
    needZero = (section < 1000) && (section > 0)
    num = Math.floor(num / 10000)
    unitPos += 1
  }

  return chnStr
}

/**
 * promise包装的延时函数
 * @param ms
 */
function delay (ms: number) {
  return new Promise((resolve) => {
    setTimeout(resolve, ms)
  })
}

/**
 * 菜单对象
 */
interface MenuItem {
  key: string
  name: string
  icon: string
  children?: MenuItem[]
}

/**
 * 根据key获取菜单对象
 * @param key
 * @param menus
 */
function getMenuObj (key: string, menus: Array<MenuItem>) {
  if (!key) return null
  let temp = menus
  let menu
  do {
    menu = temp.find(m => key.startsWith(m.key))
    if (!menu) break
    temp = menu.children || []
  } while (menu.key !== key)
  return menu
}

function toPlainObject<T> (object: T): T {
  return JSON.parse(JSON.stringify(object))
}

interface MyObject {
  [key: string]: string | number | boolean | MyObject | Array<string | number | boolean | MyObject>
}

/**
 * 深度冻结对象
 * @param object
 */
function deepFreezeObject (object: MyObject): MyObject {
  const set = new Set()
  const freezeObject = (obj: MyObject) => {
    const propNames = Object.getOwnPropertyNames(obj)

    set.add(obj)

    propNames.forEach((propName) => {
      const val = obj[propName]

      if (typeof val === 'object' && val !== null && !set.has(val)) {
        freezeObject(val as MyObject)
      }
    })

    return Object.freeze(obj)
  }

  return freezeObject(object)
}

/**
 * 获取文件夹名称
 * @param path
 * @param sep
 */
function dirname (path: string, sep: string) {
  const paths = path.split(sep)
  paths.pop()
  return paths.join(sep)
}

/**
 * 获取文件名称
 * @param path
 * @param sep
 */
function filename (path: string, sep: string) {
  const paths = path.split(sep)
  return paths.pop()
}

function validateScenarioName (name: string) {
  return /^[a-z0-9][\w+\-.()]*$/i.test(name)
}

function validateScenarioSetName (name: string) {
  return /^[a-z0-9][\w+\-.()]*$/i.test(name)
}

function validateModuleSetName (name: string) {
  return /^[a-z0-9][\w+\-.()]*$/i.test(name)
}

function validateKpiGroupName (name: string) {
  return /^[a-z0-9][\w+\-.()]*$/i.test(name)
}

function getFileSize (value: number) {
  if (!value) return '0KB'
  if (value < 1024) {
    return `${value}B`
  } else if (value < 1024 * 1024) {
    return `${(value / 1024).toFixed(1)}KB`
  } else if (value < 1024 * 1024 * 1024) {
    return `${(value / 1024 / 1024).toFixed(1)}MB`
  } else if (value < 1024 * 1024 * 1024 * 1024) {
    return `${(value / 1024 / 1024 / 1024).toFixed(1)}GB`
  } else {
    return `${(value / 1024 / 1024 / 1024 / 1024).toFixed(1)}TB`
  }
}

function getTimeName (value: number) {
  if (!value) return '0秒'
  value = value / 1000
  if (value < 60) {
    return `${value}秒`
  } else if (value < 60 * 60) {
    return `${(value / 60).toFixed(1)}分钟`
  } else if (value < 60 * 60 * 24) {
    return `${(value / 60 / 60).toFixed(1)}小时`
  } else if (value < 60 * 60 * 24 * 365) {
    return `${(value / 60 / 60 / 24).toFixed(1)}天`
  } else {
    return `${(value / 60 / 60 / 24 / 365).toFixed(1)}年`
  }
}

function timeAxisLabelFormatter (value: string | number) {
  let [integerPlace, decimalPlace = ''] = `${value}`.split('.')
  if (decimalPlace.length > 3) {
    decimalPlace = decimalPlace.substring(0, 3)
  } else if (decimalPlace.length === 0) {
    return integerPlace
  }
  return `${integerPlace}.${decimalPlace}`
}

function normalizeHeader (str: string): string {
  return str.replace(/[()[\].+\-=\\/<>?!~@#$%^&*'"{}| ]/g, '_')
}

interface TableOption {
  data: Array<StringDictionary>
  columns: Array<{ prop: string, label: string }>
}

function convertKpiSheetData (sheetData: Array<{ header: string, data: string[] }>) {
  if (sheetData && sheetData.length) {
    const tableOption: TableOption = {
      data: [],
      columns: [],
    }
    const length = max(sheetData.map((sheet) => {
      tableOption.columns.push({
        prop: normalizeHeader(sheet.header),
        label: sheet.header,
      })
      return sheet.data.length
    }))
    if (length) {
      for (let i = 0; i < length; i += 1) {
        // 将纵向表格转换成横向表格
        const row: StringDictionary = {}
        for (const column of sheetData) {
          const key = normalizeHeader(column.header)
          const value = column.data[i] || ''
          row[key] = value
        }
        tableOption.data.push(row)
      }
      return tableOption
    }
  }
  return {
    columns: [],
    data: [],
  }
}

function isZeroAndOne (array: number[]) {
  return array.every(item => item === 0 || item === 1)
}

const reportFileNameReg = /\d{4}(_\d{1,2}){5}(_[a-z]*)?\.dict$/

function getReportFileSuffix (path: string): string {
  const filename = path.split(/\\|\//).pop()
  if (filename) {
    const arr = reportFileNameReg.exec(filename)
    if (arr && arr[2]) {
      return arr[2]
    }
  }
  return ''
}

function getNow () {
  return new Date().toISOString().replace('T', ' ').replace('Z', '')
}

// 统计字符串长度，中文算2个字符
function calcStrLength (str: string | number): number {
  if (typeof str !== 'string') {
    str = str.toString()
  }
  const chats = str.split('')
  let len = 0
  chats.forEach((chat) => {
    if (/[\u4E00-\u9FA5]/.test(chat)) {
      len += 2
    } else {
      len += 1
    }
  })
  return len
}

const multiEgosModuleNameReg = /^(Ego_\d{3})-(.*)$/

const base64Reg = /:(.*?);/

function base64ToFile (base64: string, fileName: string): File {
  const arr: string[] = base64.split(',')
  if (arr.length > 1) {
    const matchResult = arr[0].match(base64Reg)
    if (matchResult && matchResult.length > 1) {
      const mime: string = matchResult[1]
      const bstr: string = atob(arr[1])
      let n = bstr.length
      const u8arr = new Uint8Array(n)
      while (n--) {
        u8arr[n] = bstr.charCodeAt(n)
      }
      return new File([u8arr], fileName, { type: mime })
    }
  }
  throw new Error('传入的base64不合法')
}

function createTextTexture (text: string, color: string) {
  const canvas = document.createElement('canvas')
  const context = canvas.getContext('2d')
  canvas.width = 20
  canvas.height = 20
  if (context) {
    context.fillStyle = color
    context.font = '32px sans-serif'
    context.fillText(text, 0, 16)
    return new CanvasTexture(canvas)
  }
  throw new Error('创建坐标系标签材质时出现了错误')
}

// 根据用户输入的模型名称生成hash值和modelId
function genHashAndModelIdFromName (name: string) {
  const hashValue = md5(name).toString()
  // 预留0-100000id给预设模型使用
  const modelId = +`0x${hashValue.substring(0, 4)}` + 100000
  return {
    hash: hashValue,
    modelId,
  }
}

// 获取模型显示分类
function getModelInterfaceCatalog (model: CatalogModel): InterfaceCategory {
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
    case CatalogCategory.CAR:
      return model.catalogParams[0].properties.interfaceCategory
    case CatalogCategory.PEDESTRIAN:
    case CatalogCategory.OBSTACLE:
      return model.catalogParams.properties.interfaceCategory
  }
}

// 获取模型显示名称
function getModelDisplayName (model: CatalogModel): string {
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
      return model.catalogParams[0].properties.alias!
    case CatalogCategory.CAR:
      if (i18n.locale === 'zh-CN') {
        return model.catalogParams[0].properties.zhName!
      } else {
        return model.catalogParams[0].properties.enName!
      }
    case CatalogCategory.PEDESTRIAN:
    case CatalogCategory.OBSTACLE:
      if (i18n.locale === 'zh-CN') {
        return model.catalogParams.properties.zhName!
      } else {
        return model.catalogParams.properties.enName!
      }
  }
}

// 显示模型显示名称
function setModelDisplayName (model: CatalogModel, name: string) {
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
      model.catalogParams[0].properties.alias = name
      break
    case CatalogCategory.CAR:
      model.catalogParams[0].properties.zhName = name
      model.catalogParams[0].properties.enName = name
      break
    case CatalogCategory.PEDESTRIAN:
    case CatalogCategory.OBSTACLE:
      model.catalogParams.properties.zhName = name
      model.catalogParams.properties.enName = name
  }
}

type Properties = keyof CatalogParams['properties']
type PropertyValueType = CatalogParams['properties'][Properties]

// 获取和设置模型属性
function getModelProperty (model: CatalogModel, name: Properties): PropertyValueType {
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
    case CatalogCategory.CAR:
      return model.catalogParams[0].properties[name]
    case CatalogCategory.OBSTACLE:
    case CatalogCategory.PEDESTRIAN:
      return model.catalogParams.properties[name]
  }
}

function setModelProperty (model: CatalogModel, name: Properties, value: PropertyValueType) {
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
    case CatalogCategory.CAR:
      (model.catalogParams[0].properties[name] as PropertyValueType) = value
      break
    case CatalogCategory.OBSTACLE:
    case CatalogCategory.PEDESTRIAN:
      (model.catalogParams.properties[name] as PropertyValueType) = value
  }
}

// 文件URL转文件路径
function fileURLToPath (fileURL: string): string {
  let url: URL

  try {
    url = new URL(fileURL)
  } catch (error) {
    throw new Error('Invalid URL: The provided string is not a valid file URL.')
  }

  if (url.protocol !== 'file:') {
    throw new Error('Invalid URL: Only file URLs are supported.')
  }

  let path = decodeURIComponent(url.pathname)

  // Windows compatibility: Convert '/' prefix to the correct drive letter
  if (/^\/[a-z]:/i.test(path)) {
    path = path.slice(1)
  }

  return path
}

// 文件路径转文件URL
function filePathToURL (path: string): string {
  const isWindows = /^([a-z]:|\\\\)/i.test(path)
  // 对于 Windows 系统，将反斜杠替换为正斜杠，并添加前导斜杠
  if (isWindows) {
    path = `/${path.replace(/\\/g, '/')}`
  }
  // 使用 encodeURIComponent 转义特殊字符
  const encodedPath = path.split('/').map(part => encodeURIComponent(part)).join('/')
  return `file://${encodedPath}`
}

const modelBasePath = 'assets/models'

// 获取模型fbx路径
function getModel3dURL (model: CatalogModel): string {
  let model3d: string
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
      if (model.catalogSubCategory === 'combination') {
        // todo: 卡车暂时只返回车头的模型
        model3d = model.catalogParams[1].model3d
      } else {
        model3d = model.catalogParams[0].model3d
      }
      break
    case CatalogCategory.CAR:
      model3d = model.catalogParams[0].model3d
      break
    case CatalogCategory.PEDESTRIAN:
    case CatalogCategory.OBSTACLE:
      model3d = model.catalogParams.model3d
      break
  }
  let result
  // 所有用户模型使用绝对路径，所有主车模型使用绝对路径
  // if (model.variable.startsWith('user_') || model.variable === '' || model.catalogCategory === CatalogCategory.EGO) {
  // 改为判断是否绝对路径。绝对路径：用户后导入的模型，或者所有主车模型。相对路径：预设模型
  if (isAbsoluteFilePath(model3d) || model.variable === '') {
    result = model3d ? filePathToURL(model3d) : ''
  } else {
    result = model3d ? join(modelBasePath, model3d) : ''
  }
  return result
}

function isAbsoluteFilePath (path: string) {
  const isWindowsPath = /^([a-z]:|\\\\)/i.test(path)
  const isUnixPath = path.startsWith('/')
  return isWindowsPath || isUnixPath
}

// 设置模型fbx路径
function setModel3d (model: CatalogModel, model3d: string, index: number = 0) {
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
      if (model.catalogSubCategory === 'combination') {
        model.catalogParams[index + 1].model3d = model3d
      } else {
        model.catalogParams[0].model3d = model3d
      }
      break
    case CatalogCategory.CAR:
      model.catalogParams[0].model3d = model3d
      break
    case CatalogCategory.PEDESTRIAN:
    case CatalogCategory.OBSTACLE:
      model.catalogParams.model3d = model3d
  }
}

// 获取模型图标路径
function getModelIcon (item: CatalogModel, withTimestamp: boolean = false): string {
  const model3d = getModel3dURL(item)
  if (!model3d) {
    console.warn('icon not found:', item)
    return ''
  }
  const pathArr = model3d.split('/')
  const fbx = pathArr.pop()
  if (fbx) {
    // 模型的图标加 thumbnail_
    const thumbnailName = `thumbnail_${fbx.replace(/\.fbx$/, '.png')}`
    pathArr.push(thumbnailName)
    let path = pathArr.join('/')
    if (withTimestamp) {
      path += `?_t=${Date.now()}`
    }
    return path
  }
  return ''
}

// 格式化模型的3D路径，把C:\xxx\yyy.fbx 转换为 xxx/yyy.fbx，useModelVariable为true时，使用模型变量替换模型路径
function formatModel3d (model: CatalogModel, useModelVariable: boolean = true): CatalogModel {
  const data: CatalogModel = cloneDeep(model)
  if ((data.catalogCategory === CatalogCategory.EGO || data.catalogCategory === CatalogCategory.CAR) && data.catalogSubCategory === 'combination') {
    // 卡车组合模型，需要处理
    data.catalogParams.slice(1).forEach((params) => {
      const { model3d } = params
      const [lastPath, fbx] = filePathToURL(model3d).split('/').slice(-2)
      let wholePath = `${lastPath}/${fbx}`
      if (useModelVariable) {
        wholePath = `${data.variable}/${fbx}`
      }
      params.model3d = wholePath
    })
  } else {
    const model3d = getModel3dURL(data)
    const [lastPath, fbx] = model3d.split('/').slice(-2)
    let wholePath = `${lastPath}/${fbx}`
    if (useModelVariable) {
      wholePath = `${data.variable}/${fbx}`
    }
    setModel3d(data, wholePath)
  }
  return data
}

function getPedestrianSubType (type: string): InterfaceCategory {
  const { pedestrianList, vehicleList } = store.state.catalogs
  let model: CatalogModel | undefined = (pedestrianList as PedestrianModel[]).find(p => p.variable === type)
  if (!model) {
    model = (vehicleList as CarModel[]).find(v => v.variable === type)
  }
  if (!model) {
    console.warn(`未找到Catalog定义：${type}`)
    return InterfaceCategory.PEDESTRIAN
  }
  if (model.catalogCategory === CatalogCategory.CAR) {
    return model.catalogParams[0].properties.interfaceCategory
  } else {
    return model.catalogParams.properties.interfaceCategory
  }
}

interface ObstacleCfg {
  obstacleType: string
  width: number
  height: number
  length: number
}

function getObstacleCfg (type: string): ObstacleCfg {
  const { obstacleList } = store.state.catalogs
  const catalog = (obstacleList as ObstacleModel[]).find(o => o.variable === type)
  if (!catalog) {
    console.warn('未注册的障碍物：', type)
    return {
      obstacleType: type,
      width: 1,
      height: 1,
      length: 1,
    }
  }
  const { width, height, length } = catalog.catalogParams.boundingBox.dimensions
  return {
    obstacleType: type,
    width,
    height,
    length,
  }
}

interface VehicleCfg {
  carType: string
  carWidth: number
  carLength: number
  carHeight: number
}

function getVehicleTypeDefinition (type: string): VehicleCfg {
  const { vehicleList } = store.state.catalogs
  const catalog = (vehicleList as CarModel[]).find(v => v.variable === type)
  if (!catalog) {
    console.warn(`未注册的汽车：${type}`)
    return {
      carType: type,
      carWidth: 1,
      carLength: 1,
      carHeight: 1,
    }
  }
  const {
    boundingBox: {
      dimensions: {
        width,
        height,
        length,
      },
    },
  } = catalog.catalogParams[0]

  return {
    carType: type,
    carWidth: width,
    carLength: length,
    carHeight: height,
  }
}

// 计算模型的size
function computeBoundingBox (object3D: Object3D): Vector3 {
  // 创建一个 Box3 对象，用于计算总的 boundingBox
  const boundingBox = new Box3()
  const size = new Vector3()

  // 遍历 Object3D 的子对象
  object3D.traverse((child) => {
    // 如果子对象是一个 Mesh
    if (child instanceof Mesh) {
      // 创建一个新的 Box3 对象，用于存储子对象的 boundingBox
      const childBoundingBox = new Box3()

      // 计算子对象的 boundingBox
      childBoundingBox.setFromObject(child)

      // 将子对象的 boundingBox 合并到总的 boundingBox 中
      boundingBox.union(childBoundingBox)
    }
  })
  boundingBox.getSize(size)
  return size
}

const defaultEgoId = 'Ego_001'

export {
  numberToChinese,
  delay,
  getMenuObj,
  toPlainObject,
  deepFreezeObject,
  dirname,
  filename,
  validateModuleSetName,
  validateKpiGroupName,
  validateScenarioName,
  validateScenarioSetName,
  getFileSize,
  getTimeName,
  timeAxisLabelFormatter,
  convertKpiSheetData,
  isZeroAndOne,
  getReportFileSuffix,
  getNow,
  calcStrLength,
  multiEgosModuleNameReg,
  base64ToFile,
  createTextTexture,
  genHashAndModelIdFromName,
  getModelInterfaceCatalog,
  getModelDisplayName,
  setModelDisplayName,
  getModel3dURL,
  setModel3d,
  getModelIcon,
  fileURLToPath,
  filePathToURL,
  getPedestrianSubType,
  getObstacleCfg,
  getVehicleTypeDefinition,
  computeBoundingBox,
  getModelProperty,
  setModelProperty,
  formatModel3d,
  defaultEgoId,
}

export type {
  StringDictionary,
}
