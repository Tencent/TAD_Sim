<template>
  <div ref="wrapper" class="sensor-view">
    <div class="intersection-position">
      <span v-if="intersection">X: {{ intersection.x }}, Y: {{ intersection.y }}, Z: {{ intersection.z }}</span>
    </div>
    <canvas ref="container" />
  </div>
</template>

<script>
import { mapGetters, mapState } from 'vuex'
import { throttle } from 'lodash-es'
import {
  AmbientLight,
  AxesHelper,
  GridHelper,
  Group,
  MathUtils,
  Mesh,
  MeshBasicMaterial,
  PerspectiveCamera,
  PointLight,
  Scene,
  SphereGeometry,
  Sprite,
  SpriteMaterial,
  Vector3,
  WebGLRenderer,
} from 'three'
import { ScenarioModelsManager } from 'models-manager/index'
import OrbitControls from './class/orbit-controls'
import SensorHelper from './class/sensor-helper'
import { formatSensorParams, getMaxDistance } from './formatSensorParams'
import { createTextTexture, filePathToURL } from '@/common/utils'

const halfPI = Math.PI / 2

function prepareMesh (mesh) {
  mesh.rotation.set(0, 0, 0)
  makeTransparent(mesh, 0.8)
  return mesh
}

function makeTransparent (mesh, opacity = 0.6) {
  const transparent = opacity !== 1
  if (mesh.material) {
    if (Array.isArray(mesh.material)) {
      mesh.material = mesh.material.map((m) => {
        const mm = m.clone()
        mm.transparent = transparent
        mm.opacity = opacity
        return mm
      })
    } else if (mesh.material.clone) {
      mesh.material = mesh.material.clone()
      mesh.material.transparent = transparent
      mesh.material.opacity = opacity
    }
  }
  if (mesh.children && mesh.children.length) {
    mesh.children.forEach(c => makeTransparent(c))
  }
}

const sphere = new SphereGeometry(0.1, 16, 16)
const center = new Mesh(sphere, new MeshBasicMaterial({ color: 0xFF4444 }))
center.material.transparent = true
center.material.opacity = 0.8

function setNaNToZero (array) {
  return array.map((item) => {
    if (Number.isNaN(item)) {
      return 0
    }
    return Number.parseFloat(item)
  })
}

/**
 * 主车真实长度 4.68618m
 * 场景里为1:1真实大小
 */
export default {
  name: 'SensorView',
  data () {
    return {
      intersection: null,
      installSlot: 0,
      resizeObserver: null,
    }
  },
  computed: {
    ...mapGetters('planners', [
      'currentSensor',
    ]),
    ...mapState('planners', [
      'currentPlanner',
    ]),
  },
  watch: {
    'currentSensor': {
      deep: true,
      handler (val) {
        this.draw(val)
      },
    },
    'currentPlanner.catalogParams': {
      deep: true,
      handler (val) {
        this.loadPlannerModel(val)
      },
    },
  },
  mounted () {
    const {
      $refs: {
        wrapper,
        container,
      },
    } = this
    const { clientWidth: WIDTH, clientHeight: HEIGHT } = wrapper

    this.modelsManager = new ScenarioModelsManager({
      type: 'editor',
      catalogs: {},
      prefix: 'assets/models',
    })

    this.resizeObserver = new ResizeObserver((entries) => {
      const entry = entries[0]
      const { contentRect: { width, height } } = entry
      this.setSize(width, height)
    })

    this.resizeObserver.observe(wrapper)

    container.setAttribute('width', WIDTH)
    container.setAttribute('height', HEIGHT)

    const aspect = WIDTH / HEIGHT
    this.renderer = new WebGLRenderer({
      antialias: true,
      canvas: container,
      alpha: true,
    })
    this.renderer.setPixelRatio(window.devicePixelRatio)
    this.renderer.setSize(WIDTH, HEIGHT)
    this.renderer.autoClear = false

    this.scene = new Scene()

    this.camera = new PerspectiveCamera(50, aspect, 1, 2000)
    this.camera.position.set(0, -20, 10)
    this.camera.up.set(0, 0, 1)
    this.scene.add(this.camera)
    this.sensorHelper = new SensorHelper({
      scene: this.scene,
      camera: this.camera,
      canvas: container,
    })
    this.center = center.clone()
    this.sensorHelper.add(this.center)
    this.sensorHelper.addEventListener('intersect', ({ position }) => {
      this.intersection = new Vector3()
      this.intersection.x = (+position.x).toFixed(5)
      this.intersection.y = (+position.y).toFixed(5)
      this.intersection.z = (+position.z).toFixed(5)
    })
    this.sensorHelper.addEventListener('not-intersect', () => {
      if (this.intersection !== null) this.intersection = null
    })
    this.scene.add(this.sensorHelper)

    this.light = new AmbientLight(0xFFFFFF, 3)
    this.scene.add(this.light)

    this.pointLight = new PointLight(0xFFFFFF)
    this.camera.add(this.pointLight)

    const gridSize = 1000
    const gridDivisions = 100
    const gridHelper = new GridHelper(gridSize, gridDivisions)
    gridHelper.rotation.x = halfPI
    this.scene.add(gridHelper)

    this.controls = new OrbitControls(this.camera, container)
    this.controls.enablePan = true
    this.controls.enableKeys = false
    this.controls.minDistance = 5
    this.controls.maxDistance = 500
    this.controls.maxPolarAngle = halfPI
    this.axes = new Group()
    const axesHelper = new AxesHelper(5)
    // axesHelper.rotation.set(-halfPI, 0, halfPI)
    this.axes.add(axesHelper)
    const axesX = new Sprite(new SpriteMaterial({ map: createTextTexture('x', '#ff3333') }))
    const axesY = new Sprite(new SpriteMaterial({ map: createTextTexture('y', '#33ff33') }))
    const axesZ = new Sprite(new SpriteMaterial({ map: createTextTexture('z', '#4466ff') }))
    axesX.position.set(5.5, 0, 0)
    axesY.position.set(0, 5.5, 0)
    axesZ.position.set(0, 0, 5.5)

    this.axes.add(axesX)
    this.axes.add(axesY)
    this.axes.add(axesZ)

    this.scene.add(this.axes)
    if (this.currentPlanner) {
      this.loadPlannerModel(this.currentPlanner.catalogParams)
    }

    this.autoRender()
    if (this.currentSensor) {
      this.draw(this.currentSensor)
    }
  },
  beforeUnmount () {
    this.sensorHelper.dispose()
    this.center.geometry.dispose()
    if (this.planner) {
      this.planner.children[0].geometry.dispose()
    }
    this.axes.children.forEach((a, index) => {
      if (index === 0) {
        a.geometry.dispose()
      } else {
        a.material.map.dispose()
        a.material.dispose()
      }
    })
    this.controls.dispose()
    this.renderer.dispose()
    this.renderer.forceContextLoss()
    cancelAnimationFrame(this.renderId)
    this.resizeObserver.disconnect()
  },
  methods: {
    setSize: throttle(function (width, height) {
      this.camera.aspect = width / height
      this.camera.updateProjectionMatrix()
      this.renderer.setSize(width, height)
    }, 100, {
      leading: false,
      trailing: true,
    }),
    draw (sensor) {
      if (!sensor) {
        this.sensorHelper.visible = false
        this.installSlot = 0
        this.fixSlotPosition()
        return
      }

      const params = formatSensorParams(sensor)
      this.installSlot = sensor.InstallSlot.substring(1)

      let {
        x,
        y,
        z,
        rotateX,
        rotateY,
        rotateZ,
        upperFov,
        lowerFov,
        leftFov,
        rightFov,
      } = params

      const far = getMaxDistance(sensor);

      ([
        x,
        y,
        z,
        rotateX,
        rotateY,
        rotateZ,
        upperFov,
        lowerFov,
        leftFov,
        rightFov,
      ] = setNaNToZero([
        x,
        y,
        z,
        rotateX,
        rotateY,
        rotateZ,
        upperFov,
        lowerFov,
        leftFov,
        rightFov,
      ]))
      this.sensorHelper.update({
        type: sensor.type,
        x,
        y,
        z,
        rotateX,
        rotateY,
        rotateZ,
        upperFov,
        lowerFov,
        leftFov,
        rightFov,
        far,
      })
      this.sensorHelper.position.set(x / 100, y / 100, z / 100)
      this.sensorHelper.rotation.set(rotateX * MathUtils.DEG2RAD, rotateY * MathUtils.DEG2RAD, rotateZ * MathUtils.DEG2RAD, 'ZYX')
      this.sensorHelper.visible = true
      this.fixSlotPosition()
    },
    autoRender () {
      this.renderer.render(this.scene, this.camera)
      this.renderId = requestAnimationFrame(this.autoRender.bind(this))
    },
    async loadPlannerModel (vehicleParam) {
      const group = new Group()
      for (const v of vehicleParam) {
        if (!v.model3d) continue
        const url = filePathToURL(v.model3d)
        const g = (await this.modelsManager.loadModel(url)).clone()
        const mesh = g.children[0]
        prepareMesh(mesh)
        const {
          center = { x: 0, y: 0, z: 0 },
          trailerOffsetX: offsetX = 0,
          trailerOffsetZ: offsetZ = 0,
        } = v.boundingBox
        mesh.position.set(+center.x + offsetX, center.y, +center.z + offsetZ)
        group.add(mesh)
      }
      if (this.planner) {
        this.scene.remove(this.planner)
      }
      this.planner = group
      this.scene.add(group)
      this.fixSlotPosition()
    },
    fixSlotPosition () {
      if (this.planner?.children && this.currentPlanner?.catalogSubCategory === 'combination') {
        const {
          trailerOffsetX: offsetX = 0,
          trailerOffsetZ: offsetZ = 0,
        } = this.currentPlanner.catalogParams[+this.installSlot + 1].boundingBox
        this.planner.position.set(-offsetX, 0, -offsetZ)
      }
    },
  },
}
</script>

<style scoped lang="less">
.sensor-view {
  background-color: #161616;
  position: relative;
  z-index: 1;
  width: 100%;
  height: 100%;
  overflow: hidden;
  user-select: none;

  .intersection-position {
    position: absolute;
    z-index: 2;
    top: 0;
    left: 0;
    box-sizing: border-box;
    height: 20px;
    width: 100%;
    font-size: 12px;
    line-height: 20px;
    background-color: #111;
    color: #dcdcdc;
    opacity: .8;
    text-align: right;
    padding: 0 20px;
  }
}
</style>
