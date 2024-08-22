/* eslint-disable */
import {
  BufferGeometry,
  ConeGeometry,
  CircleGeometry,
  FrontSide,
  BackSide,
  Group,
  MathUtils,
  Mesh,
  SphereGeometry,
  MeshLambertMaterial,
} from 'three'

const { PI } = Math
const halfPi = PI / 2
const { DEG2RAD } = MathUtils
const sphereWidthSegments = 128
const sphereHeightSegments = 64

class LidarMesh extends Group {
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
    this.sphereMeshF = new Mesh(new BufferGeometry, this.matF)
    this.upperConeMeshF = new Mesh(new BufferGeometry, this.matF)
    this.lowerConeMeshF = new Mesh(new BufferGeometry, this.matF)
    this.sphereMeshB = new Mesh(new BufferGeometry, this.matB)
    this.upperConeMeshB = new Mesh(new BufferGeometry, this.matB)
    this.lowerConeMeshB = new Mesh(new BufferGeometry, this.matB)

    this.leftCircleMeshF = new Mesh(new BufferGeometry, this.matF)
    this.leftCircleMeshB = new Mesh(new BufferGeometry, this.matB)
    this.rightCircleMeshF = new Mesh(new BufferGeometry, this.matF)
    this.rightCircleMeshB = new Mesh(new BufferGeometry, this.matB)

    this.sphereMeshF.renderOrder = 1
    this.upperConeMeshF.renderOrder = 1
    this.lowerConeMeshF.renderOrder = 1
    this.sphereMeshB.renderOrder = 0
    this.upperConeMeshB.renderOrder = 0
    this.lowerConeMeshB.renderOrder = 0

    this.leftCircleMeshF.renderOrder = 1
    this.leftCircleMeshB.renderOrder = 0
    this.rightCircleMeshF.renderOrder = 1
    this.rightCircleMeshB.renderOrder = 0

    this.add(this.sphereMeshF)
    this.add(this.sphereMeshB)

    this.add(this.upperConeMeshF)
    this.add(this.lowerConeMeshF)
    this.add(this.upperConeMeshB)
    this.add(this.lowerConeMeshB)

    this.add(this.leftCircleMeshF)
    this.add(this.leftCircleMeshB)
    this.add(this.rightCircleMeshF)
    this.add(this.rightCircleMeshB)
    this.rotation.set(0, -Math.PI / 2, -Math.PI / 2, 'YZX')
  }

  update (sensor) {
    const {
      upperFov,
      lowerFov,
      rightFov,
      leftFov,
      far,
    } = sensor

    const upperFovRad = upperFov * DEG2RAD
    const lowerFovRad = lowerFov * DEG2RAD
    const rightFovRad = rightFov * DEG2RAD
    const leftFovRad = leftFov * DEG2RAD

    const sphereGeometry = new SphereGeometry(
      far,
      sphereWidthSegments,
      sphereHeightSegments,
      -halfPi + rightFovRad,
      leftFovRad - rightFovRad,
      halfPi - upperFovRad,
      upperFovRad - lowerFovRad,
    )
    this.sphereMeshF.geometry.dispose()
    this.sphereMeshB.geometry.dispose()
    this.sphereMeshF.geometry = sphereGeometry
    this.sphereMeshB.geometry = sphereGeometry.clone()

    this.upperConeMeshF.geometry.dispose()
    this.upperConeMeshB.geometry.dispose()
    const upperHeight = Math.sin(upperFovRad) * far
    this.upperConeMeshF.geometry = new ConeGeometry(
      Math.cos(upperFovRad) * far,
      upperHeight,
      128,
      16,
      true,
      -leftFovRad, leftFovRad - rightFovRad,
    )
    this.upperConeMeshF.position.y = upperHeight / 2
    this.upperConeMeshF.rotation.x = PI
    this.upperConeMeshB.geometry = this.upperConeMeshF.geometry.clone()
    this.upperConeMeshB.position.y = upperHeight / 2
    this.upperConeMeshB.rotation.x = PI

    this.lowerConeMeshF.geometry.dispose()
    this.lowerConeMeshB.geometry.dispose()
    const lowerHeight = Math.sin(lowerFovRad) * far
    this.lowerConeMeshF.geometry = new ConeGeometry(
      Math.cos(lowerFovRad) * far,
      lowerHeight,
      128,
      16,
      true,
      -leftFovRad, leftFovRad - rightFovRad,
    )
    this.lowerConeMeshF.position.y = lowerHeight / 2
    this.lowerConeMeshF.rotation.x = PI
    this.lowerConeMeshB.geometry = this.lowerConeMeshF.geometry.clone()
    this.lowerConeMeshB.position.y = lowerHeight / 2
    this.lowerConeMeshB.rotation.x = PI

    if (leftFov - rightFov >= 360) {
      // 则是个整圆，隐藏左右两边
      this.leftCircleMeshF.visible = false
      this.leftCircleMeshB.visible = false
      this.rightCircleMeshF.visible = false
      this.rightCircleMeshB.visible = false
    } else {
      this.leftCircleMeshF.visible = true
      this.leftCircleMeshB.visible = true
      this.rightCircleMeshF.visible = true
      this.rightCircleMeshB.visible = true

      this.leftCircleMeshF.geometry.dispose()
      this.leftCircleMeshB.geometry.dispose()
      this.leftCircleMeshF.geometry = new CircleGeometry(
        far,
        32,
        halfPi - upperFovRad,
        upperFovRad - lowerFovRad,
      )
      this.leftCircleMeshF.rotation.set(0, leftFovRad - halfPi, halfPi)
      this.leftCircleMeshB.geometry = this.leftCircleMeshF.geometry.clone()
      this.leftCircleMeshB.rotation.set(0, leftFovRad - halfPi, halfPi)

      this.rightCircleMeshF.geometry.dispose()
      this.rightCircleMeshB.geometry.dispose()
      this.rightCircleMeshF.geometry = new CircleGeometry(
        far,
        32,
        halfPi + lowerFovRad,
        upperFovRad - lowerFovRad,
      )
      this.rightCircleMeshF.rotation.set(0, halfPi + rightFovRad, -halfPi)
      this.rightCircleMeshB.geometry = this.rightCircleMeshF.geometry.clone()
      this.rightCircleMeshB.rotation.set(0, halfPi + rightFovRad, -halfPi)
    }
  }

  dispose () {
    this.sphereMeshF.geometry.dispose()
    this.sphereMeshB.geometry.dispose()
    this.upperConeMeshF.geometry.dispose()
    this.lowerConeMeshF.geometry.dispose()
    this.upperConeMeshB.geometry.dispose()
    this.lowerConeMeshB.geometry.dispose()
    this.leftCircleMeshF.geometry.dispose()
    this.leftCircleMeshB.geometry.dispose()
    this.rightCircleMeshF.geometry.dispose()
    this.rightCircleMeshB.geometry.dispose()
    this.matF.dispose()
    this.matB.dispose()
  }
}


export default LidarMesh
