// 封装过的纹理加载器
import { memoize } from 'lodash'
import type { Texture } from 'three'
import { TextureLoader } from 'three'

// 全局唯一的纹理加载器
const textureLoader = new TextureLoader()

// 纹理的本地静态缓存
const textureMap: Map<string, Texture> = new Map()

// 清空本地纹理缓存
export function clearTextureCache () {
  textureMap.clear()
}

// 通过 memoize 优化图片纹理加载的网络请求
const requestTexture = memoize((url: string) => {
  return new Promise((resolve, reject) => {
    textureLoader.load(url, resolve, undefined, reject)
  })
})

// 对外提供的获取图片纹理资源的方法
export async function loadTexture (url: string): Promise<Texture> {
  if (textureMap.has(url)) {
    // 如果有缓存，则从缓存中获取
    const textureCache = textureMap.get(url) as Texture
    return textureCache.clone()
  } else {
    // 如果没有缓存，则通过请求加载
    const texture = (await requestTexture(url)) as Texture
    textureMap.set(url, texture)
    return texture.clone()
  }
}

// /**
//  * 通过 promise 封装纹理加载器，变成同步的调用逻辑
//  * @param url
//  * @returns
//  */
// export function loadTexture(url: string): Promise<Texture> {
//   return new Promise((resolve, reject) => {
//     if (!url) {
//       reject(new Error('texture url is empty'))
//       return
//     }

//     // 优先从缓存中获取纹理的备份
//     if (textureMap.has(url)) {
//       resolve(textureMap.get(url).clone())
//       return
//     }

//     // 首次加载，需要缓存原始数据
//     textureLoader.load(
//       url,
//       (texture: Texture) => {
//         textureMap.set(url, texture)

//         // 返回纹理的备份
//         resolve(texture.clone())
//       },
//       undefined,
//       (err) => {
//         reject(new Error(`load texture '${url}' failed: ${err}`))
//         return
//       }
//     )
//   })
// }
