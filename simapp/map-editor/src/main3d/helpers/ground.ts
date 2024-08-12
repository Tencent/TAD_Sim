import { Mesh, MeshBasicMaterial, PlaneGeometry } from 'three'
import { disposeMaterial, halfPI } from '@/utils/common3d'

class Ground {
  object: Mesh
  constructor () {
    const groundGeo = new PlaneGeometry(1, 1)
    const groundMat = new MeshBasicMaterial({
      // color: 0xff00ff,
      transparent: true,
      opacity: 0,
    })
    const ground = new Mesh(groundGeo, groundMat)
    ground.name = 'ground'
    const ratio = 100000000000000000
    ground.scale.set(ratio, ratio, 1)
    ground.rotateX(-halfPI)
    this.object = ground
  }

  setVisible (isVisible: boolean) {
    if (this.object.visible === isVisible) return
    this.object.visible = isVisible
  }

  dispose () {
    this.object.geometry.dispose()
    disposeMaterial(this.object)
  }
}

export default Ground
