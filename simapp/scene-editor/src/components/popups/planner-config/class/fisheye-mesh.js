/* eslint-disable */
import {
  BufferGeometry,
  ConeBufferGeometry,
  FrontSide,
  BackSide,
  MathUtils,
  Mesh,
  MeshLambertMaterial,
  SphereBufferGeometry,
} from 'three'

const sphereWidthSegments = 128
const sphereHeightSegments = 64
const { DEG2RAD } = MathUtils


class FisheyeMesh extends Mesh {
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
    this.capMeshF = new Mesh(new BufferGeometry, this.matF)
    this.sphereMeshB = new Mesh(new BufferGeometry, this.matB)
    this.capMeshB = new Mesh(new BufferGeometry, this.matB)

    this.sphereMeshF.renderOrder = 1
    this.capMeshF.renderOrder = 1
    this.sphereMeshB.renderOrder = 0
    this.capMeshB.renderOrder = 0

    this.add(this.sphereMeshF)
    this.add(this.capMeshF)
    this.add(this.sphereMeshB)
    this.add(this.capMeshB)
    this.rotation.set(0, -Math.PI / 2, -Math.PI / 2, 'YZX')
  }

  update (sensor) {
    const {
      upperFov,
      // lowerFov,
      leftFov,
      // rightFov,
      far,
    } = sensor

    // 暂时只支持对称图形，所以只取上和左
    const upperFovRad = Math.min(upperFov, 80) * DEG2RAD
    // const lowerFovRad = Math.max(lowerFov, -80) * DEG2RAD
    const leftFovRad = Math.min(leftFov, 80) * DEG2RAD
    // const rightFovRad = Math.max(rightFov, -80) * DEG2RAD

    // 0 ~ 80
    let maxFov = upperFovRad
    let minFov = leftFovRad
    if (minFov > maxFov) {
      [maxFov, minFov] = [minFov, maxFov]
    }
    // 计算较短一边的缩放
    const fullHeight = far * Math.sin(maxFov)
    const lowerHeight = far * Math.sin(minFov)
    // scale <= 1
    let scale = lowerHeight / fullHeight

    let vScale = 1
    let hScale = 1
    if (leftFovRad > upperFovRad) {
      vScale = scale
    } else if (upperFovRad > leftFovRad) {
      hScale = scale
    }

    // console.log(maxFov, vScale, hScale, 333333333333)

    const sphereGeometry = new SphereBufferGeometry(
      far,
      sphereWidthSegments,
      sphereHeightSegments,
      0,
      Math.PI * 2,
      0,
      maxFov,
    )
    this.sphereMeshF.geometry.dispose()
    this.sphereMeshF.geometry = sphereGeometry
    this.sphereMeshF.rotation.x = -Math.PI / 2

    this.sphereMeshB.geometry.dispose()
    this.sphereMeshB.geometry = sphereGeometry.clone()
    this.sphereMeshB.rotation.x = -Math.PI / 2

    const height = Math.cos(maxFov) * far
    this.capMeshF.geometry.dispose()
    this.capMeshF.geometry = new ConeBufferGeometry(
      Math.sin(maxFov) * far,
      height,
      128,
      16,
      true,
    )
    this.capMeshF.rotation.x = Math.PI / 2
    this.capMeshF.position.z = -height / 2

    this.capMeshB.geometry.dispose()
    this.capMeshB.geometry = this.capMeshF.geometry.clone()
    this.capMeshB.rotation.x = Math.PI / 2
    this.capMeshB.position.z = -height / 2
    this.scale.set(hScale, vScale, 1)
  }

  dispose () {
    this.sphereMeshF.geometry.dispose()
    this.capMeshF.geometry.dispose()
    this.matF.dispose()

    this.sphereMeshB.geometry.dispose()
    this.capMeshB.geometry.dispose()
    this.matB.dispose()
  }
}


export default FisheyeMesh
