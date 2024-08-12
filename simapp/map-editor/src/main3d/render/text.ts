// 文字精灵提示
import {
  CanvasTexture,
  Color,
  Mesh,
  MeshBasicMaterial,
  PlaneGeometry,
  Sprite,
  SpriteMaterial,
} from 'three'
import { RenderOrder } from '@/utils/business'

// 创建文字的纹理
function createTextTexture (params: {
  content: string
  color: string | number
  isSingleWord?: boolean
}) {
  const { content, color, isSingleWord = false } = params

  const canvas = document.createElement('canvas')
  const ctx = canvas.getContext('2d')
  if (!ctx) return null

  const _color = new Color(color)

  // 获取字符串预测量的像素宽度
  const { width: textWidth } = ctx.measureText(content)

  const defaultWidth = isSingleWord ? textWidth + 8 : textWidth + 36
  const defaultHeight = 16
  canvas.style.width = `${defaultWidth}px`
  canvas.style.height = `${defaultHeight}px`
  canvas.width = defaultWidth
  canvas.height = defaultHeight
  // 长宽比
  const ratio = defaultWidth / defaultHeight
  ctx.fillStyle = _color.getStyle()
  ctx.font = '14px sans-serif'
  ctx.fillText(content, 0, defaultHeight / 2 + 4)
  const texture = new CanvasTexture(canvas)

  return {
    texture,
    ratio,
  }
}

// 创建文字的精灵
export function createTextSprite (params: {
  content: string
  color?: string | number
  scale?: number
  isSingleWord?: boolean
}) {
  const { content, color = 0xFFFFFF, scale = 1, isSingleWord = false } = params
  const textureRes = createTextTexture({
    content,
    color,
    isSingleWord,
  })
  if (!textureRes) return null
  const { texture, ratio } = textureRes

  const mat = new SpriteMaterial({
    map: texture,
    transparent: true,
    opacity: 0.9,
  })
  mat.needsUpdate = true
  const sprite = new Sprite(mat)
  sprite.name = 'textSprite'
  sprite.scale.set(scale * ratio, scale, 1)

  return sprite
}

// 创建文字的平面网格
export function createTextMesh (params: {
  content: string
  color?: string | number
  scale?: number
}) {
  const { content, color = 0xFFFFFF, scale = 1 } = params
  const textureRes = createTextTexture({
    content,
    color,
  })
  if (!textureRes) return null
  const { texture, ratio } = textureRes

  const mat = new MeshBasicMaterial({
    map: texture,
    transparent: true,
    opacity: 0.9,
    depthTest: false,
  })
  mat.needsUpdate = true
  const geo = new PlaneGeometry(1, 1)
  const mesh = new Mesh(geo, mat)
  mesh.name = 'textMesh'
  mesh.scale.set(scale * ratio, scale, 1)
  // 给平面文字网格设置渲染层级
  mesh.renderOrder = RenderOrder.textMesh

  return mesh
}
