/* eslint-disable */
import {
  BufferGeometry,
  FrontSide,
  BackSide,
  MathUtils,
  Mesh,
  Group,
  MeshLambertMaterial,
  Vector3,
  Euler,
} from 'three'
import {
  ConvexGeometry,
} from 'three/examples/jsm/geometries/ConvexGeometry'

class CameraMesh extends Group {
  constructor () {
    super()
    this.matF = new MeshLambertMaterial({
      color: 0x16D1F3,
      side: FrontSide,
      opacity: .5,
      transparent: true,
    })
    this.matB = new MeshLambertMaterial({
      color: 0x16D1F3,
      side: BackSide,
      opacity: .5,
      transparent: true,
    })
    this.add(new Mesh(new BufferGeometry(), this.matF))
    this.add(new Mesh(new BufferGeometry(), this.matB))

    this.children[0].renderOrder = 1
    this.children[1].renderOrder = 0
    this.rotation.set(0, -Math.PI / 2, Math.PI / 2, 'YZX')
  }

  update (sensor) {
    const {
      upperFov,
      lowerFov,
      leftFov,
      rightFov,
      far,
    } = sensor

    const upperFovRad = Math.min(upperFov, 85) * MathUtils.DEG2RAD
    const lowerFovRad = Math.max(lowerFov, -85) * MathUtils.DEG2RAD
    const leftFovRad = Math.min(leftFov, 85) * MathUtils.DEG2RAD
    const rightFovRad = Math.max(rightFov, -85) * MathUtils.DEG2RAD

    const origin = new Vector3(0, 0, 0)
    const topLeft = new Vector3(0, 0, -far)
    const topRight = new Vector3(0, 0, -far)
    const bottomLeft = new Vector3(0, 0, -far)
    const bottomRight = new Vector3(0, 0, -far)

    const euler = new Euler()
    euler.set(upperFovRad, leftFovRad, 0)
    topLeft.applyEuler(euler)

    euler.set(upperFovRad, rightFovRad, 0)
    topRight.applyEuler(euler)

    euler.set(lowerFovRad, leftFovRad, 0)
    bottomLeft.applyEuler(euler)

    euler.set(lowerFovRad, rightFovRad, 0)
    bottomRight.applyEuler(euler)

    this.children[0].geometry.dispose()
    this.children[1].geometry.dispose()

    this.children[0].geometry = new ConvexGeometry([origin, topLeft, topRight, bottomLeft, bottomRight])
    this.children[1].geometry = this.children[0].geometry.clone()
  }

  dispose () {
    this.children[0].geometry.dispose()
    this.children[1].geometry.dispose()
    this.matF.dispose()
    this.matB.dispose()
  }
}


export default CameraMesh
