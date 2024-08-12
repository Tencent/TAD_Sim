<script setup lang="ts">
import {
  AmbientLight,
  Box3,
  DirectionalLight,
  Group,
  LoadingManager,
  Mesh,
  PerspectiveCamera,
  Scene,
  Sprite,
  Vector3,
  WebGLRenderer,
} from 'three'
import { h, inject, nextTick, onMounted, onUnmounted, ref, watch } from 'vue'
import { Camera } from '@element-plus/icons-vue'
import { throttle } from 'lodash'
import { FBXLoader } from 'three/examples/jsm/loaders/FBXLoader'
import { ElButton, ElMessage } from 'element-plus'
import { disposeMaterial, fixedPrecision, halfPI } from '@/utils/common3d'
import InputNumber from '@/components/common/inputNumber.vue'
import OrbitControls from '@/main3d/controls/orbitControls'
import Grid from '@/main3d/helpers/gird'
import Axes from '@/main3d/helpers/axes'
import { createTextSprite } from '@/main3d/render/text'
import { useModelCacheStore } from '@/stores/object/modelCache'

const props = defineProps({
  // 模型路径
  modelPath: {
    type: String,
    default: '',
  },
  editModelName: {
    type: String,
    default: '',
  },
})

const emit = defineEmits(['sync-params', 'generate-snapshot', 'loaded'])

const isModify = inject('isModify')

const modelCacheStore = useModelCacheStore()
const precision = 2
// 向父组件提供三维场景截图的方法
defineExpose({
  applyDefaultSnapshot,
  reset,
})

const webglPreviewRef = ref()
// 模型参数
const width = ref(0)
const height = ref(0)
const length = ref(0)
const posX = ref(0)
const posY = ref(0)
const posZ = ref(0)
const defaultSnapshot = ref(null)
const snapshotDisabled = ref(true)

// 部分参数的极值
const maxSize = 10000

let _dom: HTMLElement | null = null
let _areaWidth = 0
let _areaHeight = 0
let _scene: Scene | null = null
let _camera: PerspectiveCamera | null = null
let _renderer: WebGLRenderer | null = null
let _group: Group | null = null
let _resizeObserver: ResizeObserver | null = null
let _controls: OrbitControls | null = null
let _timer = -1
let _animateId = 0
let _grid: Grid | null = null
let _axes: Axes | null = null
let _labelContainer: Group | null = null
let _xSprite: Sprite | null = null
let _ySprite: Sprite | null = null
let _zSprite: Sprite | null = null

watch(
  () => props.modelPath,
  async (val) => {
    // 如果没有路径，则不用渲染
    if (!val) return

    if (!_renderer || !_scene || !_camera) {
      // 如果没有完成初始化，则先初始化再加载模型
      nextTick(() => {
        loadModel(val)
      })
    } else {
      // 如果已经初始化，则直接加载模型
      loadModel(val)
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

onMounted(() => {
  // 初始化webgl三维场景
  init({
    dom: webglPreviewRef.value,
  })
})

onUnmounted(() => {
  dispose()
})

// 加载模型，缺失贴图文件需要处理报错
const errorFiles: string[] = []
const loadingManager = new LoadingManager()
loadingManager.onStart = function (...args) {
  console.log('loading start', ...args)
}
loadingManager.onProgress = function (fileUrl, current) {
  if (current === 1) {
    console.log('loaded')
  }
}
loadingManager.onError = function (fileUrl) {
  errorFiles.push(fileUrl)
}
loadingManager.onLoad = function () {
  emit('loaded', {
    errorFiles: errorFiles.slice(),
  })
  errorFiles.length = 0
}

const fbxLoader = new FBXLoader(loadingManager)
let loadCount = 0

// 三维场景加载模型
async function loadModel (modelPath: string) {
  if (!_scene || !_camera || !_controls) return

  // 获取对应模型的配置
  const config = modelCacheStore.getCustomModelConfigByName(props.editModelName)

  const name = 'customImportModelGroup'

  // 从场景中寻找目标容器
  const originGroup = _scene.getObjectByName(name) as Group
  // 将原有的三维模型销毁
  if (originGroup) {
    originGroup.traverse((child) => {
      if (child instanceof Mesh) {
        child.geometry.dispose()
        disposeMaterial(child)
        if (child.parent) {
          child.parent.remove(child)
        }
      }
    })
    originGroup.clear()
  }

  _group = originGroup || new Group()
  _group.name = name

  // 加载新上传的模型
  const model = await fbxLoader.loadAsync(modelPath)

  // 获取模型的尺寸和初始位置
  const box = new Box3()
  box.expandByObject(model)
  const _width = fixedPrecision(box.max.x - box.min.x, precision)
  const _height = fixedPrecision(box.max.y - box.min.y, precision)
  const _length = fixedPrecision(box.max.z - box.min.z, precision)
  let _posX = fixedPrecision(model.position.x, precision)
  let _posY = fixedPrecision(model.position.y, precision)
  let _posZ = fixedPrecision(model.position.z, precision)
  if (config) {
    _posX = config.posX
    _posY = config.posY
    _posZ = config.posZ
  }

  // 同步模型的参数
  // width.value = _width
  // height.value = _height
  // length.value = _length
  width.value = 0
  height.value = 0
  length.value = 0
  posX.value = _posX
  posY.value = _posY
  posZ.value = _posZ
  // 将模型的尺寸和位置信息透传给外层组件
  emit('sync-params', {
    // width: _width,
    // height: _height,
    // length: _length,
    width: 0,
    height: 0,
    length: 0,
    posX: _posX,
    posY: _posY,
    posZ: _posZ,
  })

  if (isModify) {
    if (loadCount) {
      // 编辑模式下，如果不是第一次加载次模型，则弹窗
      callAutoFillSizePopup({ _width, _height, _length })

      // posX、posY、posZ 重新置零
      posX.value = 0
      posY.value = 0
      posZ.value = 0

      _posX = 0
      _posY = 0
      _posZ = 0

      emit('sync-params', {
        posX: 0,
        posY: 0,
        posZ: 0,
      })
    } else {
      // 如果是编辑模式下初次加载模型，则自动填入 modelList 中的模型尺寸
      const { width: _width, height: _height, length: _length } = config!
      autoFillSize({ _width, _height, _length })
    }
  } else {
    // 非编辑器模式下每次加载模型都会弹窗
    callAutoFillSizePopup({ _width, _height, _length })
  }

  loadCount++

  _group.add(model)
  // 模型相对 xyz 跟三维场景坐标的转换
  _group.position.set(_posY, _posZ, _posX)

  // 如果不存在旧的容器，则将新增的容器添加到场景中
  if (!originGroup) {
    _scene.add(_group)
  }

  let sizeRatio = _width / _length
  let heightRatio = Math.sqrt(_height / _width)
  const ratio = 1.2
  // 调整相机视角到最佳的侧面视角
  let offsetX = _width * ratio * heightRatio
  const offsetY = _height * ratio
  let offsetZ = _length * ratio * heightRatio * sizeRatio
  if (_length > _width) {
    sizeRatio = _length / _width
    heightRatio = Math.sqrt(_height / _length)
    offsetX = _width * ratio * heightRatio * sizeRatio
    offsetZ = _length * ratio * heightRatio
  }
  const cameraPosX = _posX + offsetX
  const cameraPosY = _posY + offsetY
  const cameraPosZ = _posZ + offsetZ

  // 模型相对 xyz 坐标跟三维场景坐标的转换
  _camera.position.set(cameraPosY, cameraPosZ, cameraPosX)
  const target = new Vector3(_posY, _posZ + _height / 2, _posX)
  _camera.lookAt(target)
  _camera.updateProjectionMatrix()
  _controls.target = target
  _controls.update()

  // 根据模型尺寸调整文字标签位置
  if (_xSprite) {
    _xSprite.position.set(0, 0, _length + 1)
  }
  if (_ySprite) {
    _ySprite.position.set(_width + 1, 0, 0)
  }
  if (_zSprite) {
    _zSprite.position.set(0, _height + 1, 0)
  }

  renderByTimer()

  // 由于加载模型需要预留贴图加载的时间，定时后手动触发一次渲染截取默认贴图
  if (_renderer) {
    setTimeout(() => {
      snapshotDisabled.value = false
      const defaultSnapshotFile = getSnapshotFile()
      if (defaultSnapshotFile) {
        defaultSnapshot.value = defaultSnapshotFile
      }
    }, 1200)
  }
}

function init (params: { dom: HTMLElement }) {
  const { dom } = params
  const { clientWidth, clientHeight } = dom
  _dom = dom
  _areaWidth = clientWidth
  _areaHeight = clientHeight

  initScene()
  initLight()
  initCamera()
  initRenderer()
  initControls()
  initGrid()
  initAxes()
  initLabelSprite()
  addListener()
}

function initLabelSprite () {
  if (!_scene) return
  _labelContainer = new Group()

  _xSprite = createTextSprite({
    content: 'X',
    color: 0xFF0000,
    isSingleWord: true,
    scale: 0.5,
  })
  _ySprite = createTextSprite({
    content: 'Y',
    color: 0x00FF00,
    isSingleWord: true,
    scale: 0.5,
  })
  _zSprite = createTextSprite({
    content: 'Z',
    color: 0x0000FF,
    isSingleWord: true,
    scale: 0.5,
  })
  if (_xSprite) {
    _labelContainer.add(_xSprite)
  }
  if (_ySprite) {
    _labelContainer.add(_ySprite)
  }
  if (_zSprite) {
    _labelContainer.add(_zSprite)
  }
  _scene.add(_labelContainer)
}

function initGrid () {
  if (!_scene) return
  _grid = new Grid({
    size: 50,
    divisions: 50,
    color: 0x898989,
  })
  _grid.object.translateY(-0.001)
  _scene.add(_grid.object)
}

function initAxes () {
  if (!_scene) return
  _axes = new Axes()
  // 旋转坐标轴参考线，从视觉效果上认为 x 方向指向东，y 正方向指向北，z 正方向指向高程正方向
  _axes.object.rotateX(-halfPI)
  _axes.object.rotateZ(-halfPI)
  _scene.add(_axes.object)
}

function initScene () {
  _scene = new Scene()
}

function initCamera () {
  if (!_scene) return
  _camera = new PerspectiveCamera(60, _areaWidth / _areaHeight, 0.1, 1000)
  _camera.position.set(20, 20, 20)
  _scene.add(_camera)
}

function initLight () {
  if (!_scene) return
  const ambientLight = new AmbientLight(0xB0B0B0)
  _scene.add(ambientLight)

  const dLightForward = new DirectionalLight(0xFFFFFF, 0.6)
  dLightForward.position.set(1, 1, -1)
  _scene.add(dLightForward)

  const dLightBackward = new DirectionalLight(0xFFFFFF, 0.2)
  dLightBackward.position.set(-1, -1, 1)
  _scene.add(dLightBackward)
}

function initRenderer () {
  if (!_dom) return
  _renderer = new WebGLRenderer({
    alpha: true,
    antialias: true,
    preserveDrawingBuffer: true, // 保留图形缓冲区
  })
  // 开发环境开启，生产环境关闭
  _renderer.debug.checkShaderErrors = import.meta.env.DEV
  _renderer.setPixelRatio(window.devicePixelRatio)
  _renderer.setClearColor(0xCCCCCC, 0)
  _renderer.setSize(_areaWidth, _areaHeight)
  _dom.appendChild(_renderer.domElement)
}

function initControls () {
  if (!_camera || !_renderer) return
  _controls = new OrbitControls({
    object: _camera,
    domElement: _renderer.domElement,
    render,
    startAnimationFrame,
    endAnimationFrame,
  })
  // 设置相机的视角只能在北半球的极角（即不能看从水平面下方往上看）
  _controls.minPolarAngle = Math.PI / 36
  _controls.maxPolarAngle = Math.PI / 2
  _controls.update()
}

function onResize () {
  if (!_dom || !_renderer || !_camera) return

  const { clientWidth, clientHeight } = _dom
  _areaWidth = clientWidth
  _areaHeight = clientHeight
  // 更新渲染器的尺寸
  _renderer.setSize(_areaWidth, _areaHeight)

  // 更新透视相机的宽高比例
  _camera.aspect = _areaWidth / _areaHeight
  _camera.updateProjectionMatrix()

  render()
}

function addListener () {
  if (!_dom) return
  _resizeObserver = new ResizeObserver(onResize)
  _resizeObserver.observe(_dom)
}

function removeListener () {
  if (!_dom) return
  _resizeObserver && _resizeObserver.unobserve(_dom)
}

const render = throttle(() => {
  if (_renderer && _scene && _camera) {
    _renderer.render(_scene, _camera)
  }
}, 16)

// 强行中断定时器中的内容
function forceBlockTimer () {
  if (_timer !== -1) {
    // 如果已经存在之前定义的计时器，则先清空，再重新定义新的
    clearTimeout(_timer)
    _timer = -1
    endAnimationFrame()
  }
}

// 开启高帧率的视图刷新
function startAnimationFrame () {
  // 在开启高刷之前，强制关掉定时器的高刷
  forceBlockTimer()

  function animate () {
    _animateId = requestAnimationFrame(animate)
    render()
  }

  animate()
}

function endAnimationFrame () {
  if (_animateId) {
    cancelAnimationFrame(_animateId)
    _animateId = 0
  }
}

function renderByTimer (time: number = 1000) {
  forceBlockTimer()

  // 在同步方法中开始高刷
  startAnimationFrame()

  // @ts-expect-error
  _timer = setTimeout(() => {
    // 通过定时器结束高刷
    forceBlockTimer()
  }, time)
}

function dispose () {
  removeListener()
  _scene?.traverse((child) => {
    if (child instanceof Mesh) {
      child.geometry.dispose()
      if (Array.isArray(child.material)) {
        child.material.forEach(mat => mat.dispose())
      } else {
        child.material.dispose()
      }
    }
  })
  _group?.clear()
  _scene?.clear()
  _scene = null
  _camera = null
  _renderer?.dispose()
  _renderer = null
  _controls?.dispose()
  _animateId = 0
  _timer = -1
  if (_grid) {
    _grid.dispose()
    _grid = null
  }
  if (_axes) {
    _axes.dispose()
    _axes = null
  }
  _resizeObserver?.disconnect()
  _labelContainer?.traverse((child) => {
    if (child instanceof Sprite) {
      child.geometry.dispose()
      child.material.dispose()
    }
  })
  _labelContainer?.clear()
  _labelContainer = null
  _xSprite = null
  _ySprite = null
  _zSprite = null
}

// 将 base64 的缩略图先转换成 blob 形式，再转换成 File 形式
function dataUrlToFile (dataUrl: string, fileName: string) {
  const arr = dataUrl.split(',')
  const mime = (arr[0].match(/:(.*?);/) as Array<string>)[1]
  const bstr = atob(arr[1])
  let n = bstr.length
  const u8arr = new Uint8Array(n)
  while (n--) {
    u8arr[n] = bstr.charCodeAt(n)
  }
  const blob = new Blob([u8arr], { type: mime })

  return new File([blob], fileName)
}

// 截图
function getSnapshotFile () {
  if (!_renderer) return
  // 将辅助元素隐藏，先渲染一个icon画面，再显示对应的辅助元素
  _axes?.setVisible(false)
  _grid?.setVisible(false)
  if (_labelContainer) {
    _labelContainer.visible = false
  }
  render()

  // 将从三维场景中截取的 base64 缩略图，转换成 File 类型
  const dataUrl = _renderer.domElement.toDataURL('image/png')
  const snapshotFile = dataUrlToFile(dataUrl, 'snapshot.png')

  nextTick(() => {
    _axes?.setVisible(true)
    _grid?.setVisible(true)
    if (_labelContainer) {
      _labelContainer.visible = true
    }

    render()
  })

  return snapshotFile
}

// 点击截图按钮，截取 webgl 场景当前画面作为自定义模型缩略图
function handleSnapshot () {
  if (snapshotDisabled.value) return
  const snapshotFile = getSnapshotFile()
  if (!snapshotFile) return
  emit('generate-snapshot', snapshotFile)
}

// 返回默认的截图
function applyDefaultSnapshot () {
  return defaultSnapshot.value
}

function callAutoFillSizePopup (_size: {
  _width: number
  _height: number
  _length: number
}) {
  const popup = ElMessage({
    type: 'warning',
    customClass: 'auto-fill-size-popup',
    duration: 5000,
    message: h('div', [
      h('p', { class: 'content' }, '检测到模型尺寸，是否自动填入？'),
      // 确认填入
      h('div', { class: 'btn-container' }, [
        h(
          ElButton,
          {
            size: 'small',
            onClick () {
              autoFillSize(_size)
              popup.close()
            },
            type: 'primary',
          },
          () => '确认',
        ),
        // 不填入
        h(
          ElButton,
          {
            size: 'small',
            onClick () {
              popup.close()
            },
          },
          () => '取消',
        ),
      ]),
    ]),
  })
}

function autoFillSize (_size: {
  _width: number
  _height: number
  _length: number
}) {
  const { _width, _height, _length } = _size

  width.value = _width
  length.value = _length
  height.value = _height
  posZ.value = _height / 2

  emit('sync-params', {
    width: _width,
    height: _height,
    length: _length,
    posZ: _height / 2,
  })
}

function reset () {
  // 删除模型、重置相机和控制器、参数重置
  if (!props.modelPath) return
  if (!_group) return

  // 删除模型
  _group?.traverse((child) => {
    if (child instanceof Mesh) {
      child.geometry.dispose()
      if (Array.isArray(child.material)) {
        child.material.forEach(mat => mat.dispose())
      } else {
        child.material.dispose()
      }
    }
  })

  _group.clear()
  _group = null

  // 重置相机和控制器
  const _target = new Vector3(0, 0, 0)

  if (_camera) {
    _camera?.lookAt(_target)
    _camera.updateProjectionMatrix()
  }

  if (_controls) {
    _controls?.target.copy(_target)
    _controls.update()
  }

  // 参数重置
  posX.value = 0
  posY.value = 0
  posZ.value = 0
  width.value = 0
  height.value = 0
  length.value = 0

  emit('sync-params', {
    posX: 0,
    posY: 0,
    posZ: 0,
    width: 0,
    height: 0,
    length: 0,
  })

  render()
}

// 调整原点坐标
watch(
  posX,
  (val) => {
    if (!props.modelPath) return
    if (!_group) return
    const { x, y, z } = _group.position
    const _x = Number(val)
    // 模型相对 x 对应三维场景 z
    _group.position.set(x, y, _x)
    emit('sync-params', {
      posX: _x,
    })
    render()
  },
  {
    immediate: true,
    deep: true,
  },
)
watch(
  posY,
  (val) => {
    if (!props.modelPath) return
    if (!_group) return
    const { x, y, z } = _group.position
    const _y = Number(val)
    // 模型相对 y 对应三维场景 x
    _group.position.set(_y, y, z)
    emit('sync-params', {
      posY: _y,
    })
    render()
  },
  {
    immediate: true,
    deep: true,
  },
)
watch(
  posZ,
  (val) => {
    if (!props.modelPath) return
    if (!_group) return
    const { x, y, z } = _group.position
    const _z = Number(val)
    // 模型相对 z 对应三维场景 y
    _group.position.set(x, _z, z)
    emit('sync-params', {
      posZ: _z,
    })
    render()
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  width,
  (val) => {
    if (!props.modelPath) return
    emit('sync-params', {
      width: Number(val),
    })
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  length,
  (val) => {
    if (!props.modelPath) return
    emit('sync-params', {
      length: Number(val),
    })
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(
  height,
  (val) => {
    if (!props.modelPath) return
    emit('sync-params', {
      height: Number(val),
    })
  },
  {
    immediate: true,
    deep: true,
  },
)
</script>

<template>
  <div ref="webglPreviewRef" class="webgl-preview-container">
    <div
      class="snapshot-icon"
      :class="{ disabled: snapshotDisabled }"
      @click="handleSnapshot"
    >
      <Camera />
    </div>
  </div>

  <div class="model-params">
    <div class="title">
      {{ $t('desc.uploadModel.modelParameters') }}
    </div>
    <div class="row">
      <div class="subtitle">
        * {{ $t('desc.uploadModel.originCoordinates') }}
      </div>
      <div class="item-area">
        <span class="item">
          <InputNumber
            v-model="posX"
            :disabled="false"
            :precision="precision"
            unit="m"
          />
          <span class="label">X</span>
        </span>
        <span class="item">
          <InputNumber
            v-model="posY"
            :disabled="false"
            :precision="precision"
            unit="m"
          />
          <span class="label">Y</span>
        </span>
        <span class="item">
          <InputNumber
            v-model="posZ"
            :disabled="false"
            :precision="precision"
            unit="m"
          />
          <span class="label">Z</span>
        </span>
      </div>
    </div>
    <div class="row">
      <div class="subtitle">
        * {{ $t('desc.uploadModel.sizeParameters') }}
      </div>
      <div class="item-area">
        <span class="item">
          <InputNumber
            v-model="length"
            :disabled="false"
            :precision="precision"
            :max="maxSize"
            :min="0"
            unit="m"
          />
          <span class="label">{{ $t('desc.uploadModel.length') }}</span>
        </span>
        <span class="item">
          <InputNumber
            v-model="width"
            :disabled="false"
            :precision="precision"
            :max="maxSize"
            :min="0"
            unit="m"
          />
          <span class="label">{{ $t('desc.uploadModel.width') }}</span>
        </span>
        <span class="item">
          <InputNumber
            v-model="height"
            :disabled="false"
            :precision="precision"
            :max="maxSize"
            :min="0"
            unit="m"
          />
          <span class="label">{{ $t('desc.uploadModel.height') }}</span>
        </span>
      </div>
    </div>
  </div>
</template>

<style scoped lang="less">
.webgl-preview-container {
  width: 100%;
  height: 240px;
  position: relative;
  background-color: #2b2f35;
  margin-bottom: 10px;

  .snapshot-icon {
    position: absolute;
    z-index: 1;
    right: 10px;
    bottom: 10px;
    width: 26px;
    height: 26px;
    background-color: rgba(0, 0, 0, 0.2);
    border-radius: 2px;
    display: flex;
    justify-content: center;
    align-items: center;
    cursor: pointer;
    color: var(--text-color);

    svg {
      width: 16px;
      height: 16px;
    }

    &:hover {
      background-color: rgba(0, 0, 0, 0.3);
    }

    &.disabled {
      cursor: not-allowed;
    }
  }
}

.model-params {
  font-size: 12px;

  .title {
    margin-bottom: 6px;
    color: var(--property-title-color);
  }

  .row {
    margin-bottom: 6px;

    .subtitle {
      margin-bottom: 4px;
    }

    .item-area {
      display: flex;

      .item {
        display: flex;
        flex-direction: column;
        align-items: center;

        .input-number {
          width: 85%;

          :deep(.el-input__wrapper) {
            border-radius: 2px;
            height: 26px;

            .el-input__inner {
              text-align: center;
            }
          }
        }

        .label {
          color: var(--property-title-color);
        }
      }
    }
  }
}
</style>
