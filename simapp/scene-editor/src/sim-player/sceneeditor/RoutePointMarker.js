import {
  BoxGeometry,
  CanvasTexture,
  Mesh,
  MeshBasicMaterial,
  Sprite,
  SpriteMaterial,
  Vector3,
} from 'three'

/**
 * route起始标记点类
 */
class RoutePointMarker {
  constructor (text) {
    this.setText(text)

    this.pos = new Vector3()
  }

  /**
   * 生成文字纹理
   * @param {*} text
   * @param {*} style
   */
  generateTextTexture (text, style) {
    const canvas = document.createElement('canvas')
    canvas.width = 256
    canvas.height = 64

    const context = canvas.getContext('2d')
    context.beginPath()
    context.font = '32px Microsoft YaHei'
    context.fillStyle = style
    context.fillText(text, 0, 32)
    context.fill()
    context.stroke()

    return canvas
  }

  /**
   * 设置文字
   * @param text
   */
  setText (text) {
    if (text !== this.text) {
      this.text = text
      const tex = this.generateTextTexture(text, 'rgba(255, 0, 0, 255)')
      let mat = new SpriteMaterial({
        map: new CanvasTexture(tex),
        transparent: true,
      })

      this.sprite = new Sprite(mat)
      this.sprite.scale.set(2, 2, 1)
      this.sprite.renderOrder = 300

      const boxGeom = new BoxGeometry(0.5, 0.5, 0.5)
      mat = new MeshBasicMaterial({
        // 0x represents #
        color: 0xFF0000,
        wireframe: true,
      })

      this.box = new Mesh(boxGeom, mat)
    }
  }

  /**
   * 设置坐标
   * @param x
   * @param y
   * @param z
   */
  setPos (x, y, z) {
    this.x = x
    this.y = y
    this.z = z
    this.sprite.position.set(x, y, z + 3)
    this.box.position.set(x, y, z)
  }

  addToScene (scene) {
    scene.add(this.sprite)
    scene.add(this.box)
  }

  removeFromScene (scene) {
    scene.remove(this.sprite)
    scene.remove(this.box)
  }

  setVisible (show) {
    if (show) {
      this.sprite.visible = true
      this.box.visible = true
    } else {
      this.sprite.visible = false
      this.box.visible = false
    }
  }

  get position () {
    return this.pos.set(this.x, this.y, this.z)
  }
}

export default RoutePointMarker
