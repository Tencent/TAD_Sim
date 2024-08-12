import { ElMessage } from 'element-plus'
import { errorHandler } from '@/common/errorHandler'
import {
  delMaps as delMapsService,
  exportMaps as exportMapsService,
  getMapList as getMapListService,
  importMaps as importMapsService,
  importMapsSub as importMapsServiceSub,
} from '@/api/map'
import { editor, player } from '@/api/interface'
import i18n from '@/locales'

/**
 * 获取地图列表
 *
 * @param commit
 * @return {Promise<void>}
 */
export async function getMapList ({ commit }) {
  try {
    const { data } = await getMapListService()
    commit('updateState', { mapList: data || [] })
  } catch (e) {
    await errorHandler(e)
  }
}

/**
 * 删除地图s
 * @param dispatch
 * @param names
 * @return {Promise<void>}
 */
export async function delMaps ({ dispatch }, names) {
  try {
    await delMapsService(names)
    // 更新一下hadmap信息列表
    await editor.scenario.updateHadmapInfo()
    await player.scenario.updateHadmapInfo()
    await dispatch('getMapList')
  } catch (e) {
    await errorHandler(e)
  }
}

/**
 * 导入地图
 * @param dispatch
 * @param payload
 * @return {Promise<void>}
 */
export async function importMapsSub ({ dispatch }, payload) {
  try {
    const { code, message } = await importMapsServiceSub(payload)
    await editor.scenario.updateHadmapInfo()
    await player.scenario.updateHadmapInfo()
    await dispatch('getMapList')
    if (code === 0) {
      ElMessage.success(i18n.t('tips.importMapSuccess'))
      return true
    } else if (code === -1) {
      const result = message.split(';') || []
      const task = setInterval(() => {
        const msg = result.shift()
        if (msg) {
          ElMessage.error(msg)
        }
        if (result.length === 0) {
          clearInterval(task)
        }
      }, 1000)
      return false
    }
  } catch (e) {
    return false
  }
}

/**
 * 导出地图
 * @param context
 * @param payload
 * @return {Promise<void>}
 */
export async function exportMaps (context, payload) {
  try {
    const { code, message } = await exportMapsService(payload)
    if (code === 0) {
      ElMessage.success(i18n.t('tips.exportMapSuccess'))
    } else {
      ElMessage.error({
        message,
      })
    }
  } catch (e) {
    await errorHandler(e)
  }
}
