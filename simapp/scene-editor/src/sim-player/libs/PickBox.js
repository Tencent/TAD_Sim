import { Box3, Box3Helper, Vector3 } from 'three'

/**
 * 场景中的黄色方框，用于选中场景中的元素
 */
class PickBox {
  constructor (scene) {
    this.target = null
    this.scene = scene
    this.box = new Box3()
    this.boxHelper = new Box3Helper(this.box, 0xFFFF00)
    this.boxHelper.visible = false
    this.scene.add(this.boxHelper)
  }

  /**
   * 用选框框住一个元素
   * @param obj
   */
  pick (obj) {
    if (obj?.isObject3D) {
      this.box.makeEmpty()
      this.target = obj.clone()
      this.target.rotation.set(0, 0, 0)
      const realBox = new Box3()
      realBox.setFromObject(obj)
      const realCenter = new Vector3()
      realBox.getCenter(realCenter)
      this.box.setFromObject(this.target)
      const size = new Vector3()
      this.box.getSize(size)
      this.box.setFromCenterAndSize(realCenter, size)
      this.boxHelper.rotation.copy(obj.rotation)
      this.boxHelper.updateMatrixWorld()
      this.boxHelper.visible = true
    } else {
      console.error('Object is not a instance of Object3D', obj)
    }
  }

  isPicked () {
    return this.target !== null
  }

  hide () {
    this.boxHelper.visible = false
  }

  dispose () {
    this.hide()
    this.boxHelper.geometry.dispose()
    this.boxHelper.material.dispose()
    this.boxHelper = null
  }
}

export default PickBox
