// 封装过的 fbx 模型加载器
import { memoize } from 'lodash'
import type { Group } from 'three'
import { FBXLoader } from 'three/examples/jsm/loaders/FBXLoader'

// 全局唯一的 fbx 模型加载器实例
const fbxLoader = new FBXLoader()

// 本地的 fbx 静态资源缓存
const fbxModelMap: Map<string, Group> = new Map()

// 清空本地模型缓存
export function clearFbxModelCache () {
  fbxModelMap.clear()
}

// 通过 memoize 优化模型加载的网络请求
const requestFBXModel = memoize((url: string) => {
  return new Promise((resolve, reject) => {
    fbxLoader.load(url, resolve, undefined, reject)
  })
})

// 对外提供的获取 fbx 模型资源的方法
export async function loadFBXModel (url: string): Promise<Group> {
  if (fbxModelMap.has(url)) {
    // 如果有缓存，则从缓存中获取
    const modelCache = fbxModelMap.get(url) as Group
    return modelCache.clone()
  } else {
    // 如果没有缓存，则通过请求加载
    const model = (await requestFBXModel(url)) as Group
    fbxModelMap.set(url, model)
    return model.clone()
  }
}

// /**
//  * 通过 promise 封装模型加载器，变成同步的调用逻辑
//  * @param url
//  * @returns
//  */
// export function loadFBXModel(url: string): Promise<Group> {
//   return new Promise((resolve, reject) => {
//     if (!url) {
//       reject(new Error('fbx model url is empty'))
//       return
//     }

//     // 优先从缓存中获取模型的备份
//     if (fbxModelMap.has(url)) {
//       resolve(fbxModelMap.get(url).clone())
//       return
//     }

//     // 首次加载，需要缓存原始数据
//     fbxLoader.load(
//       url,
//       (modelObj: Group) => {
//         fbxModelMap.set(url, modelObj)
//         resolve(modelObj.clone())
//       },
//       undefined,
//       (err) => {
//         reject(new Error(`load fbx model '${url}' failed: ${err}`))
//         return
//       }
//     )
//   })
// }
