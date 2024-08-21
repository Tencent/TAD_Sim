/**
 * 显示车道号用到的字体
 */
import { Mesh, MeshBasicMaterial } from 'three'
import { FontLoader } from 'three/addons/loaders/FontLoader'
import {TextGeometry} from 'three/addons/geometries/TextGeometry'
import { RenderOrder } from '../common/Constant'

const loader = new FontLoader()

const numberFontPromise = new Promise((resolve, reject) => {
  loader.load('assets/models/font/helvetiker_regular.typeface.json', (font) => {
    resolve(font)
  })
})

const numberMaterial = new MeshBasicMaterial({
  transparent: true,
  color: 0xFFFFFF,
  opacity: 1,
  depthTest: false,
  depthWrite: false,
})

/**
 * 创建数字字体geometry
 * @param id
 * @return {Promise<BufferGeometry>}
 */
async function createNumberGeometry (id) {
  const numberFont = await numberFontPromise
  return new TextGeometry(String(id), {
    font: numberFont,
    size: 0.8,
    depth: 0,
  }).center()
}

/**
 * 创建数字字体mesh
 * @param text
 * @return {Promise<Mesh>}
 */
export async function getTextMesh (text) {
  const geo = await createNumberGeometry(text)
  const mesh = new Mesh(geo, numberMaterial)
  mesh.renderOrder = RenderOrder.OBJECTS
  mesh.matrixAutoUpdate = true
  return mesh
}
