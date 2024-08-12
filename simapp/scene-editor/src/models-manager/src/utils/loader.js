import { memoize } from 'lodash-es'
import { TextureLoader } from 'three'
import { FBXLoader } from 'three/examples/jsm/loaders/FBXLoader'

const textureLoader = new TextureLoader()
const fbxLoader = new FBXLoader()

// 静态资源 url 作为作为 key 值
const textureMap = new Map()
const modelMap = new Map()

const loadTexture = memoize((url) => {
  return new Promise((resolve, reject) => {
    textureLoader.load(url, resolve, undefined, reject)
  })
})

const loadModel = memoize((url) => {
  return new Promise((resolve, reject) => {
    fbxLoader.load(url, resolve, undefined, reject)
  })
})

export async function getTexture (url) {
  if (textureMap.has(url)) {
    return textureMap.get(url)
  } else {
    const texture = await loadTexture(url)
    textureMap.set(url, texture)
    // texture 不能返回 clone
    return texture
  }
}

export async function getModel (url) {
  if (modelMap.has(url)) {
    return modelMap.get(url).clone()
  } else {
    const model = await loadModel(url)
    modelMap.set(url, model)
    return model.clone()
  }
}

// 文件URL转文件路径
export function fileURLToPath (fileURL) {
  let url
  if (!/^\/[a-z]:/i.test(fileURL)) {
    // linux 系统
    fileURL = `file://${fileURL}`
  }
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
export function filePathToURL (path) {
  const isWindows = /^([a-z]:|\\\\)/i.test(path)
  // 对于 Windows 系统，将反斜杠替换为正斜杠，并添加前导斜杠
  if (isWindows) {
    path = `/${path.replace(/\\/g, '/')}`
  }
  // 使用 encodeURIComponent 转义特殊字符
  const encodedPath = path.split('/').map(part => encodeURIComponent(part)).join('/')
  return `file://${encodedPath}`
}
