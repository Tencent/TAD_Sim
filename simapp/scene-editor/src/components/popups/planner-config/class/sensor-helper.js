/* eslint-disable */
import {
  Group, Vector2, Raycaster, Mesh, SphereGeometry, MeshBasicMaterial, PointLight,
} from 'three'
import LidarMesh from './lidar-mesh'
import CameraMesh from './camera-mesh'
import FisheyeMesh from './fisheye-mesh'
import EmptyMesh from './empty-mesh'

const _mouse = new Vector2()

class SensorHelper extends Group {
  constructor (options) {
    super()
    this.mesh = new EmptyMesh()
    this.add(this.mesh)
    const { scene, camera, canvas } = options
    this.scene = scene
    this.camera = camera
    this.canvas = canvas
    this.raycaster = new Raycaster()
    this.intersection = new PointLight(0xffffff, 1, 0.4)
    this.intersection.add(new Mesh(new SphereGeometry(0.1, 16, 16), new MeshBasicMaterial({ color: 0xffffff } )))
    this.scene.add(this.intersection)
    this.canvas.addEventListener('pointermove', this.calculateIntersect)
  }

  calculateIntersect = evt => {
    const {
      left, top,
      width, height,
    } = this.canvas.getBoundingClientRect()
    _mouse.x = ((evt.clientX - left) / width) * 2 - 1
    _mouse.y = -((evt.clientY - top) / height) * 2 + 1
    this.raycaster.setFromCamera(_mouse, this.camera)
    const intersects = this.raycaster.intersectObject(this.mesh, true)
    if(this.visible) {
      if(intersects.length) {
        this.intersection.visible = true
        this.intersection.position.copy(intersects[0].point)
        this.dispatchEvent({
          type: 'intersect',
          position: intersects[0].point.clone(),
        })
      } else {
        this.intersection.visible = false
        this.dispatchEvent({
          type: 'not-intersect',
        })
      }
    } else {
      this.dispatchEvent({
        type: 'not-intersect',
      })
      this.intersection.visible = false
    }
  }

  update (sensor) {
    const {
      type,
    } = sensor
    switch (type) {
    case 'Truth':
    case 'TraditionalLidar':
      if (!(this.mesh instanceof LidarMesh)) {
        this.disposeMesh()
        this.mesh = new LidarMesh(sensor)
        this.add(this.mesh)
      }
      break
    case 'Camera':
    case 'Semantic':
    case 'Depth':
    case 'Radar':
      if (!(this.mesh instanceof CameraMesh)) {
        this.disposeMesh()
        this.mesh = new CameraMesh(sensor)
        this.add(this.mesh)
      }
      break
    case 'Fisheye':
    case 'Ultrasonic':
      if (!(this.mesh instanceof FisheyeMesh)) {
        this.disposeMesh()
        this.mesh = new FisheyeMesh(sensor)
        this.add(this.mesh)
      }
      break
    default:
      this.disposeMesh()
      this.mesh = new EmptyMesh()
      this.add(this.mesh)
    }
    if (this.mesh) {
      this.mesh.update(sensor)
    }
  }

  disposeMesh () {
    this.remove(this.mesh)
    if (this.mesh && this.mesh.dispose) {
      this.mesh.dispose()
    }
  }

  dispose () {
    this.canvas.removeEventListener('pointermove', this.calculateIntersect)
    this.mesh.dispose()
  }
}

export default SensorHelper
