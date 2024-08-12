import { Group } from 'three'
import { clearFbxModelCache, loadFBXModel } from './fbxLoader'
import { clearTextureCache } from './textureLoader'

export { loadTexture } from './textureLoader'

// 清除所有三维场景静态资源的本地缓存
export function clear3DCache () {
  clearFbxModelCache()
  clearTextureCache()
}

/**
 * 通用的模型加载器
 * @param url
 * @returns
 */
export function load3DModel (url: string): Promise<Group> {
  // 目前只有一种类型的模型加载器，后续可以补充其他类型模型加载器
  if (/(.fbx)$/i.test(url)) {
    return loadFBXModel(url)
  }

  const defaultGroup = new Group()
  defaultGroup.name = 'emptyGroup'

  return Promise.resolve(defaultGroup)
}
