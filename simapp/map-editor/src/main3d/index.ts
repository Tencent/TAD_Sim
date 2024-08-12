import type {
  Object3D,
} from 'three'
import {
  AmbientLight,
  DirectionalLight,
  Group,
  Light,
  Mesh,
  OrthographicCamera,
  Vector2,
  Vector3,
} from 'three'
import { get } from 'lodash'
import { OBJExporter } from 'three/addons/exporters/OBJExporter.js'
import Ground from './helpers/ground'
import { disposeAllRoadAndLinks } from './render/road'
import { disposeAllJunctionAndLinks } from './render/junction'
import {
  cacheMainState,
  updateEditObjectCamera,
} from './controls/editObjectHandler'
import Base3D from '@/main3d/core'
import OrbitControls from '@/main3d/controls/orbitControls'
import DragControls, { updateRaycasterLineThreshold } from '@/main3d/controls/dragControls'
import Axes from '@/main3d/helpers/axes'
import Grid from '@/main3d/helpers/gird'
import { OperationModel } from '@/utils/business'
import { useConfig3dStore } from '@/stores/config3d'
import {
  disposeMaterial,
  fixedPrecision,
  getValidRadInRange,
  halfPI,
  oCameraOffset,
  pCameraOffset,
  worldCenter,
} from '@/utils/common3d'
import ee, { CustomEventType, getPointerMoveEvent } from '@/utils/event'

interface IInitRootParams {
  dom: HTMLElement
}

// 对核心元素做了一层封装，将偏业务功能的其他类型三维元素进行初始化
class Root3D {
  core: Base3D
  viewControls: OrbitControls
  dragControls: DragControls
  dom: HTMLElement
  axes: Axes
  grid: Grid
  ground: Ground
  mapElementsContainer: Group
  proxyScene: Group
  constructor () {}
  init (params: IInitRootParams) {
    const { dom } = params
    this.dom = dom
    this.core = new Base3D()
    this.core.init({
      dom,
    })

    this.initDefaultCamera()
    this.initProxyScene()
    this.initLight()
    this.initAxes()
    this.initGrid()
    this.initGround()
    // TODO 是否有必要在此之前添加优先级更高的监听器？？
    this.initDragControls()
    this.initViewControls()
    this.initMapElementsContainer()
    // this.applyOperationConfig()

    this.core.render()
  }

  testExporter () {
    // 导出前对场景内容预处理
    const presetHandler = () => {
      this.core.scene.remove(this.grid.object)
      this.core.scene.remove(this.axes.object)

      this.mapElementsContainer.traverse((child) => {
        if (
          child instanceof Mesh &&
          (child.name === 'roadPlacementArea' ||
          child.name === 'junctionPlacementArea' ||
          child.name === 'verticalPolePlacementArea' ||
          child.name === 'horizontalPolePlacementArea')
        ) {
          child.geometry.dispose()
          disposeMaterial(child)
          if (child.parent) {
            child.parent.remove(child)
          }
        }
      })

      const removeChildren1 = []
      for (const child of this.mapElementsContainer.children) {
        if (
          child.name === 'roadLinkContainer' ||
          child.name === 'junctionLinkContainer'
        ) {
          removeChildren1.push(child)
        }
      }
      this.mapElementsContainer.remove(...removeChildren1)

      const removeChildren = []
      for (const child of this.proxyScene.children) {
        if (child.name !== 'mapElementsContainer') {
          removeChildren.push(child)
        }
      }
      this.proxyScene.remove(...removeChildren)
    }
    presetHandler()

    const exporter = new OBJExporter()
    const data = exporter.parse(this.core.scene)

    const link = document.createElement('a')
    link.style.display = 'none'
    document.body.appendChild(link)

    const filename = 'test_map.obj'
    link.href = URL.createObjectURL(new Blob([data], { type: 'text/plain' }))

    link.download = filename
    link.click()

    // 主动释放内容
    URL.revokeObjectURL(link.href)
  }

  initEditObjectHandler () {
    // 初始化添加物体到三维场景中的处理器
    cacheMainState({
      camera: this.core.mainCamera,
      domElement: this.dom,
      objects: this.proxyScene.children,
    })
  }

  initDefaultCamera () {
    // 默认使用正交相机
    this.updateCamera({
      type: 'O',
      position: new Vector3(oCameraOffset.x, oCameraOffset.y, oCameraOffset.z),
      target: worldCenter,
    })
    this.updateOrthographicCameraZoom(1, true)
  }

  // 跟地图相关的所有元素最外侧容器
  initProxyScene () {
    const container = new Group()
    container.name = 'proxyScene'
    this.proxyScene = container
    /**
     * 其他的地图编辑软件中，以 xy 平面作为水平面，x 轴正方向指向东，y 轴正方向为指向北，z 轴正方向为高程正方向
     * threejs 中常规的坐标系，以 xz 平面作为水平面，x 轴正方向为屏幕右方向，y 轴正方向为屏幕上方向，z 轴正方向为指向屏幕前方（即指向屏幕外的用户）
     * 为了保证编辑器生成的地图数据跟有效性和合理性，需要对坐标系在使用上做转换，
     * 即以 zx 平面作为水平面，z 轴正方向指向东，x 轴正方向指向北，y 轴正方向为高程正方向，在保证三维场景视角控制器能够正常使用的前提下，向标准的地图编辑软件对齐
     * 在保存和加载地图数据时，需要对三维采样顶点坐标做响应的坐标转换
     */
    this.core.scene.add(container)
  }

  // 初始化承载地图实际元素的容器
  initMapElementsContainer () {
    const container = new Group()
    container.name = 'mapElementsContainer'
    container.matrixAutoUpdate = false
    this.mapElementsContainer = container
    this.proxyScene.add(container)
  }

  clearMapElementContainer () {
    // 销毁所有的道路级别元素
    disposeAllRoadAndLinks(this.mapElementsContainer)
    // 销毁所有的路口级别元素
    disposeAllJunctionAndLinks(this.mapElementsContainer)
    // TODO 销毁所有的物体级别元素

    this.mapElementsContainer.clear()
  }

  initDragControls () {
    const { mainCamera, renderer, startAnimationFrame, endAnimationFrame } =
      this.core
    this.dragControls = new DragControls({
      objects: this.proxyScene.children,
      camera: mainCamera,
      domElement: renderer.domElement,
      startAnimationFrame,
      endAnimationFrame,
    })
    // 设置 Line 类型元素的设想检测阈值
    updateRaycasterLineThreshold(0.5)
  }

  setViewControlsEnabled (enabled: boolean) {
    this.viewControls.enabled = enabled
  }

  initViewControls () {
    const {
      mainCamera,
      renderer,
      render,
      startAnimationFrame,
      endAnimationFrame,
    } = this.core
    this.viewControls = new OrbitControls({
      object: mainCamera,
      domElement: renderer.domElement,
      render,
      startAnimationFrame,
      endAnimationFrame,
    })
    const config3dStore = useConfig3dStore()
    this.viewControls.addEventListener('zoom', (option) => {
      const { value } = option
      config3dStore.updateCameraZoom(value)
      // 相机zoom的缩放，也会触发鼠标射线跟坐标系的交互坐标
      const pointerEvent = getPointerMoveEvent()
      if (pointerEvent) {
        ee.emit(CustomEventType.webglArea.pointermove, pointerEvent)
      }
    })
    this.viewControls.addEventListener('perspective-zoom', () => {
      // 透视视角下鼠标滚轮滚动，也会触发鼠标射线投影的坐标值更新
      const pointerEvent = getPointerMoveEvent()
      if (pointerEvent) {
        ee.emit(CustomEventType.webglArea.pointermove, pointerEvent)
      }
    })
    // 设置相机的视角只能在北半球的极角（即不能看从水平面下方往上看）
    // this.viewControls.minPolarAngle = Math.PI / 36
    this.viewControls.maxPolarAngle = Math.PI / 2
    // 相机默认只有在透视相机才能渲染，正交相机不支持旋转
    this.viewControls.enableRotate = !!get(mainCamera, 'isPerspectiveCamera')

    this.viewControls.addEventListener('change', () => {
      // 如果是正交相机，则忽略相机的调整
      if (this.core.mainCamera === this.core.oCamera) return

      // 计算当前相机视角形成的角度
      const posVec3 = this.core.mainCamera.position
      const targetVec3 = this.viewControls.target

      // 忽略高度，只考虑二维的坐标
      const posVec2 = new Vector2(posVec3.z, posVec3.x)
      const targetVec2 = new Vector2(targetVec3.z, targetVec3.x)

      const direction = posVec2.clone().sub(targetVec2).normalize()

      // 当前角度是二维 xy 坐标系下，跟x轴形成的夹角
      const angle = getValidRadInRange(direction.angle())

      // 触发数据层的相机角度更新
      config3dStore.updateCameraAngle(angle)
    })
  }

  // 透视视角下，点击指北针将相机视角调整至看向正北方向
  resetPerspectiveCameraAngle () {
    // 基于当前相机的位置和目标点，重新计算新的相机位置，保证目标点在相机位置的正北方向
    // 即 z 坐标相同
    const originPos = this.core.mainCamera.position
    const originTarget = this.viewControls.target

    // 先判断大概的方位是否是正北。如果基本是正北方向，则可以忽略此次角度的重置
    const deltaZ = originPos.z - originTarget.z
    // 误差 [-0.2, 0.2] 之间，且相机位置的 x 坐标小于目标点的 x 坐标
    if (
      Math.abs(fixedPrecision(deltaZ)) < 0.2 &&
      originPos.x < originTarget.x
    ) {
      return
    }

    // 两种调试视角方案：
    // 1、相机位置不动，目标点绕相机位置所在的y轴旋转
    // 2、目标点不动，相机位置绕目标点所在的y轴旋转【采用】
    const newPos = originPos.clone()
    // 计算相机位置到目标点的半径（忽略高度）
    // 将高度先调整为跟目标点同样的高度，方便计算距离
    newPos.y = originTarget.y

    const radius = newPos.distanceTo(originTarget)

    // 相机新的位置，应该位于目标点的正南方向，即z值相同，x值小于目标点
    newPos.z = originTarget.z
    newPos.x = originTarget.x - radius
    // 恢复相机原来的高度
    newPos.y = originPos.y

    // 更新相机
    this.updateCamera({
      type: 'P',
      position: newPos,
      target: originTarget,
    })
  }

  /**
   * 更新相机的属性，包括相机类型，位置，看向的目标
   * position 和 target 最好是成对出现，保证相机视角稳定正确
   * @param params
   * @returns
   */
  updateCamera (params: {
    type?: biz.ICameraType | ''
    position?: common.vec3 | null
    target?: common.vec3 | null
  }) {
    const { core } = root3dInstance
    const { type = '', position = null, target = null } = params
    let newTarget, newPos
    if (!type && !position && !target) return

    if (!type) {
      // 如果不更新相机类型，则只更新相机的位置和目标
      if (position && target) {
        newTarget = new Vector3(target.x, target.y, target.z)
        newPos = new Vector3(position.x, position.y, position.z)
      }
    } else {
      if (position && target) {
        // 即指定了更换的相机类型，又指定了相机的位置和目标
        newTarget = new Vector3(target.x, target.y, target.z)
        newPos = new Vector3(position.x, position.y, position.z)
        core.mainCamera = type === 'P' ? core.pCamera : core.oCamera
      } else {
        // 单纯地更新相机类型，自动基于当前相机的上一个状态来定义类型更新后的位置和目标
        // 获取当前相机上一个状态的 pos 和 target
        const lastPos = core.mainCamera.position
        const lastTarget = this.viewControls.target
        if (type === 'P') {
          // 相机类型没有变化，则直接返回
          if (core.mainCamera === core.pCamera) return
          // 从正交相机切换到透视相机，保持还是看向 target，相机位置基于原来垂直位置做偏移
          newTarget = new Vector3(lastTarget.x, 0, lastTarget.z)
          newPos = new Vector3(
            lastTarget.x + pCameraOffset.x,
            lastPos.y,
            lastTarget.z,
          )
          core.mainCamera = core.pCamera
        } else {
          // 相机类型没有变化，则直接返回
          if (core.mainCamera === core.oCamera) return
          // 从透视相机切换到正交相机，保持还是看向 target，只不过 target 需要转换成在 zx 水平面的
          newTarget = new Vector3(lastTarget.x, 0, lastTarget.z)
          // 相机高度使用上一个状态相机位置高度，正交相机需要使用默认的水平偏移量
          newPos = new Vector3(
            lastTarget.x + oCameraOffset.x,
            lastPos.y,
            lastTarget.z,
          )
          core.mainCamera = core.oCamera
        }
      }
    }

    if (!newPos || !newTarget) return

    // 更新相机属性
    core.mainCamera.position.set(newPos.x, newPos.y, newPos.z)
    core.mainCamera.lookAt(newTarget)
    core.mainCamera.updateProjectionMatrix()

    if (this.viewControls) {
      // 更新视角控制器的属性
      this.viewControls.object = core.mainCamera
      this.viewControls.target = newTarget
      this.viewControls.saveState()
      // 如果是正交相机，不支持视角旋转；透视相机，支持视角旋转
      this.viewControls.enableRotate = !!get(
        core.mainCamera,
        'isPerspectiveCamera',
      )
      this.viewControls.update()
    }

    if (this.dragControls) {
      // 更新拖拽控制点的属性
      this.dragControls.updateCamera(core.mainCamera)
    }

    updateEditObjectCamera(core.mainCamera)

    core.render()
  }

  updateOrthographicCameraZoom (zoom: number, noRender: boolean = false) {
    if (this.core.mainCamera !== this.core.oCamera) return

    this.core.mainCamera.zoom = zoom
    this.core.mainCamera.updateProjectionMatrix()
    if (this.viewControls) {
      this.viewControls.saveState()
      this.viewControls.update()
    }
    if (!noRender) {
      this.core.render()
    }
  }

  // 获取相机的缩放效果
  getCameraZoom () {
    if (this.core.mainCamera === this.core.oCamera) {
      // 正交相机，看 zoom 参数
      return this.core.mainCamera.zoom
    } else {
      // 透视相机，看相机位置距离 target 的距离
      const cameraPos = this.core.mainCamera.position.clone()
      const target = this.viewControls.target.clone()
      const dist = cameraPos.distanceTo(target)
      return dist
    }
  }

  initLight () {
    const { scene } = this.core
    const ambientLight = new AmbientLight(0xB0B0B0)
    scene.add(ambientLight)

    const dLightForward = new DirectionalLight(0xFFFFFF, 0.6)
    dLightForward.position.set(1, 2, 1)
    scene.add(dLightForward)

    const dLightBackward = new DirectionalLight(0xFFFFFF, 0.2)
    dLightBackward.position.set(-1, -1, -1)
    scene.add(dLightBackward)
  }

  initAxes () {
    this.axes = new Axes()
    // 旋转坐标轴参考线，从视觉效果上认为 x 方向指向东，y 正方向指向北，z 正方向指向高程正方向
    this.axes.object.rotateX(-halfPI)
    this.axes.object.rotateZ(-halfPI)
    this.core.scene.add(this.axes.object)
  }

  initGrid () {
    this.grid = new Grid({
      size: 9000,
      divisions: 300,
      color: 0x303030,
    })
    this.grid.object.translateY(-0.1)
    this.core.scene.add(this.grid.object)
  }

  initGround () {
    this.ground = new Ground()
    this.proxyScene.add(this.ground.object)
  }

  getObjectByUuid (uuid: string) {
    return this.core.scene.getObjectByProperty('uuid', uuid)
  }

  getObjectById (id: string) {
    const object = this.mapElementsContainer.getObjectByProperty('objectId', id)
    if (!object) return null
    return object
  }

  // 按照配置做相关的参数和属性调整
  applyOperationConfig () {
    const { mainCamera } = this.core
    const main3dStore = useConfig3dStore()
    if (mainCamera instanceof OrthographicCamera) {
      if (main3dStore.operationModel === OperationModel.select) {
        this.viewControls.enableRotate = false
      }
    }
  }

  dispose () {
    this.core.scene.traverse((child: Object3D) => {
      if (child instanceof Light) {
        child.dispose()
      }
    })
    this.axes.dispose()
    this.grid.dispose()
    this.ground.dispose()
    this.core.dispose()
    this.viewControls.dispose()
    this.dragControls.dispose()

    this.viewControls = null
    this.dragControls = null
    this.core = null
    this.axes = null
    this.grid = null
    this.ground = null
    this.dom = null
  }
}

const root3dInstance = new Root3D()

export default root3dInstance
