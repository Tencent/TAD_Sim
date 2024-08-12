<template>
  <div ref="wrapper" class="signlight-map">
    <canvas ref="container" />
  </div>
</template>

<script>
import { throttle } from 'lodash-es'
import {
  AmbientLight,
  AxesHelper,
  Geometry,
  GridHelper,
  Group,
  LineDashedMaterial,
  LineSegments,
  OrthographicCamera,
  Scene,
  Vector3,
  WebGLRenderer,
} from 'three'
import Signlight from 'sim-player/sceneeditor/Signlight'
import { MapElementType } from 'sim-player/common/Constant'
import { yawToVector3 } from 'sim-player/common/MeshUtils'
import OrbitControls from '@/components/popups/planner-config/class/orbit-controls'
import { editor } from '@/api/interface'
import { DragControls } from '@/components/popups/planner-config/class/drag-controls'
import eventBus from '@/event-bus'

const HALF_PI = Math.PI / 2

export default {
  name: 'SignlightMap',
  emits: ['click-light'],
  data () {
    return {}
  },
  mounted () {
    const wrapper = this.$refs.wrapper
    const container = this.$refs.container
    const { clientWidth: WIDTH, clientHeight: HEIGHT } = wrapper

    this.resizeObserver = new ResizeObserver((entries) => {
      const entry = entries[0]
      const { contentRect: { width, height } } = entry
      this.setSize(width, height)
    })
    this.resizeObserver.observe(wrapper)

    container.setAttribute('width', WIDTH)
    container.setAttribute('height', HEIGHT)

    this.aspect = WIDTH / HEIGHT
    this.renderer = new WebGLRenderer({
      antialias: true,
      canvas: container,
      alpha: true,
    })
    this.renderer.setPixelRatio(window.devicePixelRatio)
    this.renderer.setSize(WIDTH, HEIGHT)
    this.renderer.autoClear = false
    this.renderer.setClearColor(0x1F1F27, 1.0)

    this.scene = new Scene()

    this.topSceneRange = 80
    this.camera = new OrthographicCamera(
      -1 * this.topSceneRange * this.aspect,
      this.topSceneRange * this.aspect,
      this.topSceneRange,
      -1 * this.topSceneRange,
    )
    this.camera.position.set(0, 0, 200)
    this.camera.up.set(0, 0, 1)
    this.scene.add(this.camera)

    this.light = new AmbientLight(0x666666, 1)
    this.scene.add(this.light)

    this.controls = new OrbitControls(this.camera, container)
    this.controls.enablePan = true
    this.controls.enableKeys = false
    this.controls.enableRotate = false
    this.controls.minDistance = 5
    this.controls.maxDistance = 500
    this.controls.maxPolarAngle = HALF_PI

    this.axes = new AxesHelper(20)
    this.scene.add(this.axes)

    this.grid = new GridHelper(1000, 50, 0x101010, 0x101010)
    this.grid.rotation.x = -HALF_PI
    this.grid.renderOrder = -1
    this.grid.material.depthTest = false
    this.scene.add(this.grid)

    this.simuScene = window.simuScene
    const mapMeshGroup = editor.scenario.getMapGroup()
    if (mapMeshGroup) {
      this.scene.add(mapMeshGroup)
    }
    // 装灯的
    this.signlights = []
    this.signlightGroup = new Group()
    this.signlightGroup.renderOrder = 101
    this.scene.add(this.signlightGroup)

    this.dragControl = new DragControls(this.signlightGroup.children, this.camera, container)
    // 左键点击
    this.dragControl.dragButton = 0
    this.dragControl.addEventListener('click', (evt) => {
      const mesh = evt.intersect?.object
      if (mesh) {
        const signlight = this.signlights.find(s => s.model === mesh)
        if (signlight) {
          this.$emit('click-light', signlight.id)
        }
      }
    })

    this.highlightLanelinkMap = new Map()

    // 选中的灯
    this.chosenSignlights = []
    this.autoRender()

    eventBus.$on('focus-to-junction', this.focusToJunction)
  },
  beforeUnmount () {
    this.controls.dispose()
    this.dragControl.dispose()
    this.renderer.dispose()
    this.renderer.forceContextLoss()
    cancelAnimationFrame(this.renderId)
    this.resizeObserver.disconnect()
    eventBus.$off('focus-to-junction', this.focusToJunction)
  },
  methods: {
    autoRender () {
      this.renderer.render(this.scene, this.camera)
      this.renderId = requestAnimationFrame(this.autoRender.bind(this))
    },
    setSize: throttle(function (width, height) {
      this.renderer.setSize(width, height)
      this.aspect = width / height
      this.camera.aspect = width / height
      this.camera.left = -1 * this.topSceneRange * this.aspect
      this.camera.right = this.topSceneRange * this.aspect
      this.camera.top = this.topSceneRange
      this.camera.bottom = -1 * this.topSceneRange
      this.camera.updateProjectionMatrix()
    }, 100, {
      leading: false,
      trailing: true,
    }),
    async showSignlights ({ signlights, routes }) {
      const points = routes.map((r) => {
        [r.startLon, r.startLat] = r.start.split(',')
        r.endPosArr = [{ lon: r.startLon, lat: r.startLat }]
        return { startLon: r.startLon, startLat: r.startLat }
      })
      const infos = await this.simuScene.hadmap.batchGetNearbyLaneInfo(points)
      routes.forEach((r, i) => {
        const data = infos[i]
        if (data?.err === 0) {
          r.yaw = data.yaw
          if (data.type === 'lane') {
            r.roadId = data.rid
            r.sectionId = data.sid
            r.laneId = data.lid
            r.elemType = MapElementType.LANE
            r.startAlt = data.alt
          } else if (data.type === 'lanelink') {
            r.lanelinkId = data.llid
            r.elemType = MapElementType.LANELINK
            r.startAlt = data.alt
            const lanelink = this.simuScene.hadmap.getLanelink(r.lanelinkId)
            if (lanelink) {
              ({ fromRoadId: r.roadId, fromSectionId: r.sectionId, fromLaneId: r.laneId } = lanelink)
            }
          }
        } else {
          const msg = `route: ${r.id} (lon: ${
            r.startLon} lat: ${r.startLat}) no lane`
          console.error('queryinfobypt error:', msg)
        }
      })

      this.signlights.length = 0
      await Promise.all(signlights.map(async (s) => {
        const signlight = new Signlight()
        signlight.copyNoModel(s)
        const arrowObjects = await this.simuScene.modelsManager.loadSignArrowModels()
        signlight.createModel(arrowObjects)
        signlight.addToScene(this.signlightGroup)
        let startOffset = ({ L: -1.75, R: 1.75, T: 0, L0: -3.5, R0: 3.5 })[signlight.phase]
        const route = routes.find(r => `${r.id}` === `${signlight.routeId}`)
        if (route) {
          this.signlights.push(signlight)
          const { laneId, roadId, sectionId } = route
          signlight.roadId = roadId
          signlight.sectionId = sectionId
          signlight.laneId = laneId
          // 支持双向道路之后，对向车道（id>0的）要取shift=0
          const shift = laneId > 0 ? 0 : Number.MAX_SAFE_INTEGER
          if (laneId > 0) {
            startOffset *= -1
          }
          const pos = this.simuScene.getLaneShiftPos(
            route.roadId,
            route.sectionId,
            route.laneId,
            shift,
            startOffset,
          )
          signlight.model.position.set(pos[0], pos[1], pos[2])
          signlight.startShift = pos[4] || 0
          if (signlight.startShift < 0) {
            signlight.startShift = 0
          }
          // 换成后台返回的yaw
          // const dir = this.simuScene.getLaneShiftDir(roadId, sectionId, laneId, shift)
          const dir = yawToVector3(route.yaw)
          signlight.dir.copy(dir)
          signlight.updateDisplay()
        } else {
          console.error(`未找到 signlight：${signlight.id} 的route`)
        }
      }))
    },
    hideAllSignlights () {
      this.signlights.forEach((signlight) => {
        signlight.removeFromScene(this.signlightGroup)
      })
      this.signlights.length = 0
    },
    /**
     * 更改灯颜色
     * @param {{id:string, color: 'red'|'yellow'|'green'|'gray', disabled: false}[]} options
     */
    changeSignlightColors (options) {
      options.forEach((opt) => {
        const signlight = this.signlights.find(s => `${s.id}` === `${opt.id}`)
        if (signlight) {
          signlight.setStatus({ color: opt.color, disabled: opt.disabled })
        } else {
          console.warn(`未找到 signlight，id: ${opt.id}`)
        }
      })
    },
    /**
     * 高亮灯
     * @param {string[]} ids
     */
    highlightSignlights (ids) {
      ids.forEach((id) => {
        const signlight = this.signlights.find(s => `${s.id}` === `${id}`)
        if (!signlight) {
          console.warn(`未找到 signlight，id: ${id}`)
          return
        }

        this.chosenSignlights.push(id)
        signlight.setStatus({ bordered: true })

        // 高亮对应的 lanelink
        if (signlight.tolanelinkids) {
          const linkIds = signlight.tolanelinkids.split(',')
          linkIds.forEach((lanelink) => {
            this.highlightLanelink(lanelink, '#ffb324')
          })
        }
      })
    },
    lowlightAllSignlights () {
      this.chosenSignlights.forEach((id) => {
        const signlight = this.signlights.find(s => `${s.id}` === `${id}`)
        if (!signlight) {
          console.warn(`未找到 signlight，id: ${id}`)
          return
        }
        signlight.setStatus({ bordered: false })
      })
      this.chosenSignlights.length = 0
      this.cancelHighlightLanelink()
    },
    highlightLanelink (linkId, color = 0xFF0000) {
      const curLanelink = this.simuScene.hadmap.getLanelink(linkId)
      if (curLanelink?.data) {
        const geom = new Geometry()
        geom.vertices.push(...curLanelink.data)
        const material = new LineDashedMaterial({
          color,
          dashSize: 2,
          gapSize: 2,
        })
        material.depthWrite = false
        const line = new LineSegments(
          geom,
          material,
        )
        line.renderOrder = 100
        this.highlightLanelinkMap.set(linkId, line)
        this.scene.add(line)
      }
    },
    cancelHighlightLanelink () {
      this.highlightLanelinkMap.forEach((line) => {
        this.scene.remove(line)
      })
    },
    focusToJunction (event) {
      const id = event
      const linkVecs = []
      this.simuScene.hadmap.lanelinksMap.forEach((link) => {
        if (`${link.junctionId}` === `${id}` && link.data?.length >= 2) {
          linkVecs.push(link.data[0], link.data[link.data.length - 1])
        }
      })
      if (linkVecs.length) {
        const sumVec = linkVecs.reduce((vec, data) => {
          vec.x += data.x
          vec.y += data.y
          return vec
        }, new Vector3())
        sumVec.multiplyScalar(1 / linkVecs.length)
        this.camera.position.set(sumVec.x, sumVec.y, 200)
        this.camera.lookAt(sumVec)
        this.controls.target.copy(sumVec)
        this.controls.update()
      } else {
        // todo: 所选的junction 的所有 link都没有点
      }
    },
  },
}
</script>

<style scoped lang="less">
</style>
