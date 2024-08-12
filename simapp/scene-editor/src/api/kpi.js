import dict from '@/common/dict'
import mockElectron from '@/api/mock-electron'
import ajax from '@/api/axios'
import i18n from '@/locales'

const { electron: { system, kpi } = mockElectron } = window
function request (target, method, ...args) {
  return new Promise((resolve, reject) => {
    target[method](...args, (status) => {
      const { retCode, data } = status
      if (retCode === 1) {
        resolve(data)
      } else {
        reject(new Error(i18n.t(dict.retCode[retCode]) || retCode))
      }
    })
  })
}

/**
 * 获取kpi列表
 * @return {Promise<unknown>}
 */
export function getKpiList () {
  return request(kpi, 'findAll')
}

/**
 * 创建kpi
 * @param payload
 * @return {Promise<unknown>}
 */
export function updateKpi (payload) {
  return request(kpi, 'update', payload)
}

/**
 * 删除kpi
 * @param name
 * @return {Promise<unknown>}
 */
export function destroyKpi (name) {
  return request(kpi, 'destroy', name)
}

/**
 * 获取kpi组列表
 * @return {Promise<unknown>}
 */
export function getKpiGroupList () {
  return request(kpi.group, 'findAll')
}

/**
 * 创建kpi组
 * @param payload
 * @return {Promise<unknown>}
 */
export function createKpiGroup (payload) {
  return request(kpi.group, 'create', payload)
}

/**
 * 更新kpi组
 * @param payload
 * @return {Promise<unknown>}
 */
export function updateKpiGroup (payload) {
  return request(kpi.group, 'update', payload)
}

/**
 * 删除kpi组
 * @param id
 * @return {Promise<unknown>}
 */
export function destroyKpiGroup (id) {
  return request(kpi.group, 'destroy', id)
}

/**
 * 激活kpi组
 * @param id
 * @return {Promise<unknown>}
 */
export function activeKpiGroup (id) {
  return request(kpi.group, 'active', id)
}

/**
 * 复制一个kpi组
 * @param payload
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function createKpisByKpiGroup (payload) {
  return ajax.post('http://127.0.0.1:8302/api/config/add_grading_kpis', payload)
}

/**
 * 获取kpi组下的kpi列表
 * @param id
 * @return {Promise<unknown>}
 */
export function getKpisByKpiGroup (id) {
  return request(kpi.group, 'findKpis', id)
}

/**
 * 获取kpi报告列表
 * @return {Promise<unknown>}
 */
export function getKpiReportList () {
  return request(kpi.report, 'findAll')
}

/**
 * 删除kpi报告
 * @param ids
 * @return {Promise<unknown>}
 */
export function destroyKpiReport (ids) {
  return request(kpi.report, 'destroy', ids)
}

/**
 * 获取kpi报告
 * @param id
 * @return {Promise<unknown>}
 */
export function getKpiReport (id) {
  return request(kpi.report, 'find', id)
}

/**
 * 上传报告截图
 * @param params
 * @return {Promise<unknown>}
 */
export function uploadReportImage (params) {
  return request(kpi.report, 'uploadReportImage', params)
}

/**
 * 删除报告截图
 * @param params
 * @return {Promise<unknown>}
 */
export function deleteReportImage (params) {
  return request(kpi.report, 'deleteReportImage', params)
}

/**
 * 获取报告截图
 * @param params
 * @return {Promise<unknown>}
 */
export function getReportImage (params) {
  return request(kpi.report, 'getReportImage', params)
}

/**
 * 获取报告截图base64
 * @param filePath
 * @return {Promise<unknown>}
 */
export function getImageBase64 (filePath) {
  return request(system, 'getImageBase64', filePath)
}

/**
 * 设置kpi标签
 * @param params
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function setGradingLabel (params) {
  return ajax.post('http://127.0.0.1:8302/api/config/set_grading_kpi_label', params)
}

/**
 * 获取kpi标签描述
 * @return {Promise<axios.AxiosResponse<any>>}
 */
export function getGradingLabelDescriptions () {
  return request(kpi.labels, 'getGradingLabelDescriptions')
}

export function setGradingLabelDescription (params) {
  return request(kpi.labels, 'setGradingLabelDescription', params)
}

export function delGradingLabelDescription (params) {
  return request(kpi.labels, 'delGradingLabelDescription', params)
}
