<script setup lang="ts">
import { inject, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import {
  AxesHelper,
  Box3,
  DirectionalLight,
  GridHelper,
  Group,
  HemisphereLight,
  LinearEncoding,
  LoadingManager,
  Material,
  Mesh,
  MeshPhongMaterial,
  PerspectiveCamera,
  PlaneGeometry,
  PointLight,
  Scene,
  Sprite,
  SpriteMaterial,
  Vector3,
  WebGLRenderer,
  sRGBEncoding,
} from 'three'
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js'
import { FBXLoader } from 'three/examples/jsm/loaders/FBXLoader.js'
import type {
  BoundingBox,
  CatalogModel,
  Dimensions,
} from 'models-manager/src/catalogs/class.ts'
import { CatalogCategory,
} from 'models-manager/src/catalogs/class.ts'
import { base64ToFile, computeBoundingBox, createTextTexture, fileURLToPath, getModel3dURL } from '@/common/utils'
import { uploadThumbnail } from '@/api/models.ts'

const props = defineProps<{
  data?: CatalogModel
}>()

const emits = defineEmits<{
  (e: 'snap-shot', path: string): void
  (e: 'loaded', result: LoadingResult): void
}>()

interface LoadingResult {
  loadedFiles: string[]
  errorFiles: string[]
  dimensions: Dimensions
}

defineExpose({
  autoSnapShot,
})

const viewWidth = 340
const viewHeight = 340

// 外层容器
const wrap = ref<HTMLDivElement>()
const halfPI = Math.PI / 2

let renderer: WebGLRenderer,
  scene: Scene,
  camera: PerspectiveCamera,
  dLight: DirectionalLight,
  hLight: HemisphereLight,
  pLight: PointLight,
  grid: GridHelper,
  renderId: number,
  controls: OrbitControls,
  axis: AxesHelper,
  planner: Group | undefined

const dLightIntensity = ref<number>(0.6)
const hLightIntensity = ref<number>(0.6)
const pLightIntensity = ref<number>(0.4)

onMounted(() => {
  // 创建three.js场景
  if (!wrap.value) return
  const sightTarget = new Vector3(0.6, 1, 0.75)
  renderer = new WebGLRenderer({
    antialias: true,
    alpha: true,
  })
  renderer.setSize(viewWidth, viewHeight)
  wrap.value.appendChild(renderer.domElement)
  scene = new Scene()
  camera = new PerspectiveCamera(45, viewWidth / viewHeight, 0.1, 1000)
  camera.position.set(7, -8, 5)
  camera.up.set(0, 0, 1)
  camera.lookAt(sightTarget)
  controls = new OrbitControls(camera, renderer.domElement)
  controls.target.copy(sightTarget)
  controls.update()
  dLight = new DirectionalLight(0xFFFFFF, dLightIntensity.value)
  dLight.position.set(10, -10, 10)
  dLight.castShadow = true
  dLight.shadow.camera.top = 180
  dLight.shadow.camera.bottom = -100
  dLight.shadow.camera.left = -120
  dLight.shadow.camera.right = 120
  hLight = new HemisphereLight(0xFFFFFF, 0xFFFFFF, hLightIntensity.value)
  hLight.position.set(0, 0, 200)
  pLight = new PointLight(0xFFFFFF, pLightIntensity.value, 30, 4)
  pLight.position.set(12, 10, 3)
  grid = new GridHelper(200, 200, 0x999999, 0x999999)
  grid.rotation.x = halfPI
  if (grid.material instanceof Material) {
    grid.material.opacity = 0.1
    grid.material.transparent = true
  }

  axis = new AxesHelper(10)
  const ground = new Mesh(new PlaneGeometry(2000, 2000), new MeshPhongMaterial({
    color: 0x777777,
    depthWrite: false,
  }))
  ground.receiveShadow = true
  const axesX = new Sprite(new SpriteMaterial({ map: createTextTexture('x', '#ff3333') }))
  const axesY = new Sprite(new SpriteMaterial({ map: createTextTexture('y', '#33ff33') }))
  const axesZ = new Sprite(new SpriteMaterial({ map: createTextTexture('z', '#4466ff') }))
  axesX.position.set(5.5, 0, 0)
  axesY.position.set(0, 5.5, 0)
  axesZ.position.set(0, 0, 5.5)

  axis.add(axesX)
  axis.add(axesY)
  axis.add(axesZ)

  scene.add(pLight)
  scene.add(dLight)
  scene.add(hLight)
  scene.add(grid)
  scene.add(axis)

  function render () {
    renderer.render(scene, camera)
    renderId = requestAnimationFrame(render)
  }

  render()
})

onBeforeUnmount(() => {
  // 销毁three.js场景
  wrap.value!.removeChild(renderer.domElement)
  renderer.dispose()
  renderer.forceContextLoss()
  cancelAnimationFrame(renderId)
})

const loadedFiles: string[] = []
const errorFiles: string[] = []

const mode = inject('mode')

// 是否在模型加载完成后需要通知父组件自动填入，一般在编辑模式下的第一次加载不自动填入
let firstEditLoading = mode === 'edit' || mode === 'copy'

const loadingManager = new LoadingManager()
loadingManager.onProgress = function (fileUrl, current) {
  if (current === 1) {
    loadedFiles.length = 0
  }
  loadedFiles.push(fileUrl)
}
loadingManager.onStart = function (...args) {
  console.log('loading start', ...args)
}
loadingManager.onLoad = function () {
  if (!firstEditLoading) {
    const boundingBox = computeBoundingBox(planner!)
    const size = {
      width: boundingBox.y,
      length: boundingBox.x,
      height: boundingBox.z,
    }
    resetCamera()
    emits('loaded', { loadedFiles: loadedFiles.slice(), errorFiles: errorFiles.slice(), dimensions: size })
  } else {
    // 编辑模式第一次加载不通知，后面的都需要通知
    firstEditLoading = false
  }
  loadedFiles.length = 0
  errorFiles.length = 0
}
loadingManager.onError = function (fileUrl) {
  errorFiles.push(fileUrl)
}

const fbxLoader = new FBXLoader(loadingManager)

function prepareMesh (mesh: Mesh) {
  mesh.rotation.set(0, 0, 0)
  fixMaterialEncoding(mesh)
  return mesh
}

function fixMaterialEncoding (object: Mesh) {
  object.traverse((o) => {
    if (o instanceof Mesh) {
      if (Array.isArray(o.material)) {
        o.material.forEach((m) => {
          const { map } = m as MeshPhongMaterial
          if (map?.encoding === sRGBEncoding) {
            map.encoding = LinearEncoding
          }
        })
      } else {
        const { map } = o.material as MeshPhongMaterial
        if (map?.encoding === sRGBEncoding) {
          map.encoding = LinearEncoding
        }
      }
    }
  })
}

type OnCleanup = (cleanupFn: () => void) => void

type WatchedParams = [string | undefined, BoundingBox | undefined]

// 渲染模型，根据boundingBox调整模型位置
async function renderModel (value: WatchedParams, oldValue: WatchedParams, onCleanUp: OnCleanup) {
  const [model3d, boundingBox] = value
  let canceled = false
  onCleanUp(() => {
    canceled = true
  })
  if (model3d && model3d !== oldValue[0] && boundingBox) {
    const g = await fbxLoader.loadAsync(model3d)
    if (!canceled) {
      if (planner) {
        scene.remove(planner)
        planner = undefined
      }
      const group = new Group()
      const mesh = g.children[0] as Mesh
      prepareMesh(mesh)
      const {
        center = { x: 0, y: 0, z: 0 },
        trailerOffsetX: offsetX = 0,
        trailerOffsetZ: offsetZ = 0,
      } = boundingBox
      mesh.position.set(center.x + offsetX, center.y, center.z + offsetZ)
      group.add(mesh)
      planner = group
      scene.add(group)
      // await postLoadModel()
    }
  } else if (boundingBox && planner?.children[0]) {
    const mesh = planner.children[0]
    const {
      center = { x: 0, y: 0, z: 0 },
      trailerOffsetX: offsetX = 0,
      trailerOffsetZ: offsetZ = 0,
    } = boundingBox
    mesh.position.set(center.x + offsetX, center.y, center.z + offsetZ)
  } else if (!model3d && planner) {
    scene.remove(planner)
    planner = undefined
  }
}

function resetCamera () {
  if (camera && planner) {
    const { x: posX, y: posY, z: posZ } = planner.position
    const box = new Box3()
    const size = new Vector3()
    box.setFromObject(planner)
    box.getSize(size)
    const { x: length, y: width, z: height } = size
    let sizeRatio = width === 0 ? 0 : length / width
    let heightRatio = length === 0 ? 0 : Math.sqrt(height / length)
    const ratio = 1.6
    // 调整相机视角到最佳的侧面视角
    let offsetY = length * ratio * heightRatio
    const offsetZ = height * ratio
    let offsetX = width * ratio * heightRatio * sizeRatio
    if (width > length) {
      sizeRatio = length === 0 ? 0 : width / length
      heightRatio = width === 0 ? 0 : Math.sqrt(height / width)
      offsetY = length * ratio * heightRatio * sizeRatio
      offsetX = width * ratio * heightRatio
    }
    const cameraPosX = posX + offsetX
    const cameraPosY = posY - offsetY
    const cameraPosZ = posZ + offsetZ
    // 模型相对 xyz 坐标跟三维场景坐标的转换
    camera.position.set(cameraPosX, cameraPosY, cameraPosZ)
    const target = new Vector3(posX, posY, posZ + height / 2)
    camera.lookAt(target)
    camera.updateProjectionMatrix()
    controls.target = target
    controls.update()
  }
}

// 观察模型path和boundingBox数据，有变动就更新模型
watch(
  [
    (): string | undefined => props.data ? getModel3dURL(props.data) : undefined,
    (): BoundingBox | undefined => {
      if (props.data) {
        const {
          catalogCategory,
          catalogParams,
        } = props.data
        if (catalogCategory === CatalogCategory.EGO || catalogCategory === CatalogCategory.CAR) {
          return catalogParams[0].boundingBox
        }
        return catalogParams.boundingBox
      }
      return undefined
    },
  ],
  renderModel,
  { deep: true, immediate: true },
)

// 截图的base64
let iconSnapshot = ''

// 截图事件响应
async function snapShot (): Promise<void> {
  axis.visible = false
  grid.visible = false
  renderer.render(scene, camera)
  iconSnapshot = renderer.domElement.toDataURL('image/png')
  axis.visible = true
  grid.visible = true

  await saveIcon()
}
async function saveIcon () {
  if (!props.data) return
  const pathArr = getModel3dURL(props.data).split('/')
  const fbxFileName = pathArr.pop()
  if (fbxFileName && fbxFileName.endsWith('.fbx')) {
    const iconFileName = fbxFileName.replace(/.fbx$/, '.png')
    const file = base64ToFile(iconSnapshot, iconFileName)
    const { dstpath } = await uploadThumbnail(file, fileURLToPath(pathArr.join('/')))
    iconSnapshot = ''
    emits('snap-shot', dstpath)
  } else {
    // todo: error, fbx文件名不合法
  }
}

// 自动截图，用于保存模型时用户没有截图
async function autoSnapShot (): Promise<void> {
  resetCamera()
  await snapShot()
}

type lightType = 'd' | 'h' | 'p'

function setLightIntensity (type: lightType, value: number) {
  switch (type) {
    case 'p':
      pLightIntensity.value = value
      pLight.intensity = value
      break
    case 'h':
      hLightIntensity.value = value
      hLight.intensity = value
      break
    case 'd':
      dLightIntensity.value = value
      dLight.intensity = value
      break
    default:
  }
}

function resetLightIntensity () {
  setLightIntensity('d', 0.6)
  setLightIntensity('p', 0.6)
  setLightIntensity('h', 0.4)
}
</script>

<template>
  <div
    ref="wrap"
    class="model-view"
    :style="{ width: `${viewWidth}px`, height: `${viewHeight}px` }"
  >
    <el-tooltip
      content="点击生成缩略图"
      placement="top-end"
    >
      <el-icon class="snap-shot-button" @click="snapShot">
        <Camera />
      </el-icon>
    </el-tooltip>
  </div>
  <div v-if="false" class="scene-lights-config">
    <div class="sl-config-box">
      <span class="sl-config-title">平行光强度</span>
      <el-slider
        :min="0"
        :max="5"
        :step="0.02"
        :model-value="dLightIntensity"
        @input="setLightIntensity('d', $event as number)"
      />
    </div>
    <div class="sl-config-box">
      <span class="sl-config-title">环境光强度</span>
      <el-slider
        :min="0"
        :max="5"
        :step="0.02"
        :model-value="hLightIntensity"
        @input="setLightIntensity('h', $event as number)"
      />
    </div>
    <div class="sl-config-box">
      <span class="sl-config-title">点光源强度</span>
      <el-slider
        :min="0"
        :max="5"
        :step="0.02"
        :model-value="pLightIntensity"
        @input="setLightIntensity('p', $event as number)"
      />
    </div>
    <el-button type="primary" plain @click="resetLightIntensity">
      还原光照
    </el-button>
  </div>
</template>

<style scoped lang="less">
@import "@/assets/less/mixins";

.model-view {
  position: relative;
  z-index: 1;
  margin-bottom: 24px;

  .snap-shot-button {
    position: absolute;
    z-index: 2;
    right: 12px;
    bottom: 31px;
    cursor: pointer;
    color: @global-font-color;
  }

  :deep(canvas) {
    &:focus-visible {
      outline: none;
      //border: none;
    }
  }
}

.scene-lights-config {
  .sl-config-box {
    display: flex;
    line-height: 32px;

    .sl-config-title {
      flex: 0 0 120px;
    }
  }
}
</style>
