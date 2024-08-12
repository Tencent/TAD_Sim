/* eslint-disable */
import {
  BufferGeometry,
  Line,
  Vector3,
  LineBasicMaterial,
} from 'three'

class EmptyMesh extends Line {
  constructor () {
    const geometry = new BufferGeometry()
    super(geometry, new LineBasicMaterial({
      color: 0xffffff,
      linewidth: 1,
    }))
  }

  update (sensor) {
    this.geometry.dispose()
    this.geometry = new BufferGeometry().setFromPoints([
      new Vector3(0, 0, 0), new Vector3(0, sensor.far, 0),
    ])
  }

  dispose () {
    this.geometry.dispose()
    this.material.dispose()
  }
}


export default EmptyMesh
