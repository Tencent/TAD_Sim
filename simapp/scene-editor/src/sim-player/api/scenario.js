import axios from 'axios'
import GlobalConfig from '../common/Config'

/**
 * 获取配置信息
 * @return {Promise<AxiosResponse<any>> | *}
 */
export function getConfig () {
  return axios({
    method: 'get',
    url: 'http://localhost:9000/configurationinfo',
    withCredentials: true,
  })
}

/**
 * 打开场景
 * @param sceneID
 * @return {*}
 */
export function getSceneDataV2 (sceneID) {
  let p

  if (GlobalConfig.isCloud) {
    p = axios({
      method: 'get',
      url: loadSceneURLCloud,
      withCredentials: true,
      params: {
        id: sceneID,
      },
    })
  } else {
    p = axios({
      method: 'post',
      url: 'http://localhost:9000/openscene/v2',
      withCredentials: true,
      timeout: 120000,
      data: {
        id: sceneID,
      },
    })
  }

  return p
}

const sceneURLCloud = new URL(location.href)
const sceneURLCloudSearchParamKeys = Array.from(sceneURLCloud.searchParams.keys())
sceneURLCloudSearchParamKeys.forEach((key) => {
  sceneURLCloud.searchParams.delete(key)
})

sceneURLCloud.pathname = '/simService/scenes/cloudEditor/save'

export const saveSceneURLCloud = sceneURLCloud.toString()

sceneURLCloud.pathname = '/simService/scenes/cloudEditor/open'

export const loadSceneURLCloud = sceneURLCloud.toString()
