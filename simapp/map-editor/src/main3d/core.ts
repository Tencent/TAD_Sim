import { throttle } from 'lodash'
import {
  OrthographicCamera,
  PerspectiveCamera,
  Scene,
  WebGLRenderer,
} from 'three'
import { globalConfig } from '@/utils/preset'

interface IInitParams {
  dom: HTMLElement
}

export type CameraType = 'P' | 'O'

// 三维场景最基本核心的元素
let animateId: number = 0
class Base3D {
  scene: Scene
  mainCamera: PerspectiveCamera | OrthographicCamera
  renderer: WebGLRenderer
  pCamera: PerspectiveCamera
  oCamera: OrthographicCamera
  dom: HTMLElement
  areaWidth: number
  areaHeight: number
  startAnimationFrame: Function
  endAnimationFrame: Function
  resizeObserver: ResizeObserver
  timer: number | null
  constructor () {
    this.timer = null
  }

  init (params: IInitParams) {
    const { dom } = params
    this.dom = dom
    const { clientWidth, clientHeight } = this.dom
    this.areaWidth = clientWidth
    this.areaHeight = clientHeight

    this.initScene()
    this.initCamera()
    this.initRenderer()

    this.addResizeListener()

    // 开启高帧率的视图刷新
    this.startAnimationFrame = () => {
      const scope = this
      // 在开启高刷之前，强制关掉定时器的高刷
      this.forceBlockTimer()

      function animate () {
        animateId = requestAnimationFrame(animate)
        scope.render()
      }
      animate()
    }
    this.endAnimationFrame = () => {
      if (animateId) {
        cancelAnimationFrame(animateId)
        animateId = 0
      }
    }
  }

  initScene () {
    this.scene = new Scene()
    this.scene.matrixAutoUpdate = false
    if (globalConfig.isDev) {
      window.scene = this.scene
    }
  }

  initCamera () {
    this.pCamera = new PerspectiveCamera(
      60,
      this.areaWidth / this.areaHeight,
      1,
      10000,
    )
    this.scene.add(this.pCamera)

    this.oCamera = new OrthographicCamera(
      -this.areaWidth / 2,
      this.areaWidth / 2,
      this.areaHeight / 2,
      -this.areaHeight / 2,
      -1000,
      1000,
    )
    this.scene.add(this.oCamera)
  }

  initRenderer () {
    this.renderer = new WebGLRenderer({
      alpha: true,
      antialias: true,
      logarithmicDepthBuffer: true,
    })
    // 开发环境开启，生产环境关闭
    this.renderer.debug.checkShaderErrors = import.meta.env.DEV
    this.renderer.setPixelRatio(window.devicePixelRatio)
    this.renderer.setClearColor(0x1F1F27, 0)
    this.renderer.setSize(this.areaWidth, this.areaHeight)
    this.dom.appendChild(this.renderer.domElement)
    // TODO remove
    window.gri = () => {
      return this.renderer.info
    }
  }

  // 通过节流来避免高频触发，每 16ms 触发一次，也能够保证 1s 60帧的渲染效果
  render = throttle(() => {
    if (this.renderer && this.scene && this.mainCamera) {
      this.renderer.render(this.scene, this.mainCamera)
    }
  }, 16)

  // 创建一个定时器，在指定的时间内调用高刷的渲染函数
  // 【主要用于创建三维模型自动加载纹理贴图，如果不主动触发渲染函数，纹理的效果无法更新的情况】
  renderByTimer (time: number = 1000) {
    this.forceBlockTimer()

    // 在同步方法中开始高刷
    this.startAnimationFrame()

    // @ts-expect-error
    this.timer = setTimeout(() => {
      // 通过定时器结束高刷
      this.forceBlockTimer()
    }, time)
  }

  // 强行中断定时器中的内容
  forceBlockTimer () {
    if (this.timer !== null) {
      // 如果已经存在之前定义的计时器，则先清空，再重新定义新的
      clearTimeout(this.timer)
      this.timer = null
      this.endAnimationFrame()
    }
  }

  onResize = () => {
    const { clientWidth, clientHeight } = this.dom

    this.areaWidth = clientWidth
    this.areaHeight = clientHeight
    // 更新渲染器的尺寸
    this.renderer.setSize(this.areaWidth, this.areaHeight)

    // 更新透视相机的宽高比例
    this.pCamera.aspect = this.areaWidth / this.areaHeight
    this.pCamera.updateProjectionMatrix()

    // 更新正交相机的边界范围
    this.oCamera.left = -this.areaWidth / 2
    this.oCamera.right = this.areaWidth / 2
    this.oCamera.top = this.areaHeight / 2
    this.oCamera.bottom = -this.areaHeight / 2
    this.oCamera.updateProjectionMatrix()

    this.render()
  }

  addResizeListener () {
    this.resizeObserver = new ResizeObserver(this.onResize)
    this.resizeObserver.observe(this.dom)
  }

  removeResizeListener () {
    this.resizeObserver && this.resizeObserver.unobserve(this.dom)
  }

  dispose () {
    this.scene?.clear()
    this.removeResizeListener()
    this.scene = null
    this.renderer = null
    this.mainCamera = null
    this.pCamera = null
    this.oCamera = null
    this.dom = null
    if (this.timer !== null) {
      clearTimeout(this.timer)
      this.timer = null
    }
  }
}

export default Base3D
