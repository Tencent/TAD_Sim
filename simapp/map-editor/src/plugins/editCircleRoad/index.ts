import { throttle } from 'lodash'
import {
  Color,
  MOUSE,
  Mesh,
  Vector2,
  Vector3,
} from 'three'
import type {
  Intersection,
  MeshBasicMaterial,
} from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import { useEditRoadStore } from '../editRoad/store'
import config from './config.json'
import EditCircleRoadHelper from './main3d'
import { useCircleRoadInteraction } from './store/interaction'
import {
  HelperAreaColor,
  createCircleEndArea,
  disposeAllVirtualElement,
  disposeHelperElement,
  renderVirtualArcRoad,
  renderVirtualCircleRoad,
} from './main3d/draw'
import { useRoadStore } from '@/stores/road'
import {
  getSelected,
  registryHoverElement,
  registryIgnoreDragOffsetElements,
  setInteractionPlane,
  setSelected,
} from '@/main3d/controls/dragControls'
import root3d from '@/main3d/index'
import { PI, axisY, worldCenter } from '@/utils/common3d'
import { Constant } from '@/utils/business'
import { useConfig3dStore } from '@/stores/config3d'
import { getRoad } from '@/utils/mapCache'

// 点击开始时环形道路的中心基准点
const centerPoint = new Vector3()
// 点击开始时，相机的缩放参数
let zoom = 0
// 环形道路配置的缓存
const circleRoadOptions: Array<biz.ICircleOption> = []
class EditLaneWidthPlugin extends TemplatePlugin {
  editHelper: EditCircleRoadHelper
  unsubscribeInteractionStore: Function
  unsubscribeRoadStore: Function
  constructor () {
    super()
    this.config = config

    // 覆写模板中定义的交互逻辑
    this.dragDispatchers = {
      dragStart: (options: common.IDragOptions) => {
        if (!this.enabled) return

        // 判断是否有符合当前模式的交互物体
        const { intersections, pointerdownButton } = options.object
        if (pointerdownButton === MOUSE.LEFT) {
          // 如果是左键点击，支持：选中水平面拖拽创建环形道路；拖拽圆弧道路的边缘调整范围
          intersections.forEach((intersection: Intersection) => {
            if (getSelected()) return

            const { object, point } = intersection
            switch (object.name) {
              case 'helperArea': {
                // 环形道路首尾的辅助交互区域
                setSelected(intersection)

                // 当前设置成高度为零的水平面
                setInteractionPlane({
                  normal: axisY,
                  point: worldCenter,
                })

                // 禁用视角调整
                root3d.setViewControlsEnabled(false)
                // 清空环形道路配置的缓存
                circleRoadOptions.length = 0
                break
              }
              case 'lane': {
                // 鼠标左键选中车道
                setSelected(intersection)
                break
              }
              case 'ground': {
                setSelected(intersection)

                // 圆环中心基准点
                centerPoint.x = point.x
                centerPoint.y = 0
                centerPoint.z = point.z

                // 当前设置成高度为零的水平面
                setInteractionPlane({
                  normal: axisY,
                  point: worldCenter,
                })

                // 禁用视角调整
                root3d.setViewControlsEnabled(false)
                // 获取此时相机的缩放程度
                zoom = root3d.getCameraZoom()
                // 清空环形道路配置的缓存
                circleRoadOptions.length = 0
                break
              }
              default:
                break
            }
          })
        }
      },
      // 节流
      drag: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const selected = getSelected()
        if (!selected) return
        if (selected.object.name === 'ground') {
          // 基于初始点的坐标，创建一个环形道路
          this.dragPreviewCircleRoad(options)
        } else if (selected.object.name === 'helperArea') {
          // 拖拽道路首、尾的交互区域，调整环形道路的弧度
          this.dragPreviewHelperArea(options)
        }
      },
      // up 事件有可能存在误触连续调用，通过节流来避免
      dragEnd: throttle((options: common.IDragOptions) => {
        if (!this.enabled) return
        const selected = getSelected()
        if (!selected) return

        // 不管选中说明物体，拖拽结束后都恢复视角控制
        root3d.setViewControlsEnabled(true)

        if (selected.object.name === 'ground') {
          // 调用 store 中实际创建环形道路的方法
          this.dragEndCircleRoad(options)
        } else if (selected.object.name === 'helperArea') {
          this.dragEndHelperArea(options)
        }
      }, 100),
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { pointerdownButton } = options.object
        const selected = getSelected()
        if (!selected) return

        // 不管选中什么物体，click结束后恢复视角控制器
        root3d.setViewControlsEnabled(true)

        const interactionStore = useCircleRoadInteraction()

        if (pointerdownButton === MOUSE.LEFT) {
          switch (selected.object.name) {
            case 'lane': {
              // @ts-expect-error
              const { roadId } = selected.object
              if (!roadId) return

              if (interactionStore.roadId) {
                if (interactionStore.roadId === roadId) {
                  // 如果选中的时同一条道路

                } else {
                  // 如果有上一次选中的道路，需要提前先取消选中
                  interactionStore.unselectRoad()
                  interactionStore.selectRoad(roadId)
                }
              } else {
                interactionStore.selectRoad(roadId)
              }

              break
            }
            case 'ground': {
              // 取消当前道路的选中效果
              interactionStore.unselectRoad()
              break
            }
            default:
              break
          }
        }
      },
      hoverOn: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { elementName, elementUuid } = options.object
        if (!elementName || !elementUuid) return
        if (elementName === 'helperArea') {
          // 展示道路首尾端面的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (
              !new Color(HelperAreaColor.selected).equals(material.color)
            ) {
              material.color.set(HelperAreaColor.hovered)
              material.needsUpdate = true
              this.render()
            }
          }
        }
      },
      hoverOff: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { elementName, elementUuid } = options.object
        if (!elementName || !elementUuid) return
        if (elementName === 'helperArea') {
          // 取消道路首尾端面的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (
              !new Color(HelperAreaColor.selected).equals(material.color)
            ) {
              material.color.set(HelperAreaColor.normal)
              material.needsUpdate = true
              this.render()
            }
          }
        }
      },
    }
  }

  dragPreviewHelperArea (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return

    // 高亮当前辅助交互区域
    const helperAreaMesh = selected.object as Mesh
    const mat = helperAreaMesh.material as MeshBasicMaterial
    const { color: currentColor } = mat
    if (!new Color(HelperAreaColor.selected).equals(currentColor)) {
      mat.color.set(HelperAreaColor.selected)
      mat.needsUpdate = true
    }

    const parent = this.editHelper.container

    // 从辅助交互的三维元素上获取属性
    const {
      isHead,
      startAngle,
      endAngle,
      radius,
      center,
      forwardRoadWidth,
      reverseRoadWidth,
      roadId,
    } = helperAreaMesh
    // 辅助区域绑定的是正向和反向的道路集合
    const [, reverseRoad] = roadId
    let isClockwise = false
    // 获取反向道路，判断是否是顺时针创建的
    const road = getRoad(reverseRoad)
    if (
      road &&
      road.isCircleRoad &&
      road.circleOption &&
      road.circleOption.isClockwise
    ) {
      isClockwise = true
    }

    // 鼠标拖拽到的点
    const _point = new Vector3(newPosition.x, newPosition.y, newPosition.z)
    // 从圆心到鼠标的方向
    const direction = _point.clone().sub(center).normalize()
    const directionVec2 = new Vector2(direction.z, direction.x)
    const newAngle = directionVec2.angle()

    let _startAngle = startAngle
    let _endAngle = endAngle
    if (isHead) {
      _startAngle = newAngle
    } else {
      _endAngle = newAngle
    }

    // 先将角度都调整成正数，再取余
    _startAngle = (_startAngle + PI * 2) % (PI * 2)
    _endAngle = (_endAngle + PI * 2) % (PI * 2)
    if (isClockwise) {
      // 如果是顺时针绘制，则起始角度应该大于截止角度
      if (_startAngle < _endAngle) {
        _startAngle += PI * 2
      }
    } else {
      // 如果是常规逆时针绘制，起始角度应该小于截止角度
      if (_endAngle < _startAngle) {
        _endAngle += PI * 2
      }
    }

    // 形成的夹角最小是 4°
    const minAngle = PI / 45
    const maxAngle = PI * 2 - minAngle
    if (isClockwise) {
      // 起始角度大于截止角度
      if (_startAngle - _endAngle < minAngle) return
      if (_startAngle - _endAngle > maxAngle) return
    } else {
      // 截止角度大于起始角度
      if (_endAngle - _startAngle < minAngle) return
      if (_endAngle - _startAngle > maxAngle) return
    }

    // 每一次渲染预览虚拟的元素之前，都需要销毁上一次的虚拟预览元素
    disposeHelperElement({
      parent,
      options: {
        keepArea: true, // 保留道路首和尾的辅助交互区域
      },
    })
    disposeAllVirtualElement(parent)

    // 渲染虚拟的圆弧道路
    const circleOptions = renderVirtualArcRoad({
      parent,
      center,
      radius,
      startAngle: _startAngle,
      endAngle: _endAngle,
      forwardRoadWidth,
      reverseRoadWidth,
      isClockwise,
    })

    // 先清空
    circleRoadOptions.length = 0
    // 再缓存配置
    circleRoadOptions.push(...circleOptions)

    this.render()
  }

  dragEndHelperArea (options: common.IDragOptions) {
    const selected = getSelected()
    if (!selected) return
    if (circleRoadOptions.length < 1) return

    const parent = this.editHelper.container
    // @ts-expect-error
    const { roadId } = selected.object

    // 取消所有辅助元素的渲染
    disposeAllVirtualElement(parent)
    disposeHelperElement({
      parent,
    })

    const roadStore = useRoadStore()
    const _points = circleRoadOptions[0].points as Array<common.vec3>
    // 更新圆弧道路
    roadStore.updateArcRoad({
      ...circleRoadOptions[0],
      points: _points,
      roadId,
    })

    this.render()
  }

  dragPreviewCircleRoad (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    const parent = this.editHelper.container

    // 圆环半径上的点
    let _point = new Vector3(newPosition.x, newPosition.y, newPosition.z)
    // 此时鼠标交互点的位置，跟圆环基准点，计算对应圆环半径、角度
    // 半径【TODO 考虑半径不能过于小的异常情况】
    let radius = _point.distanceTo(centerPoint)

    // 由于是双向双车道，限制最小的圆环半径
    const direction = _point.clone().sub(centerPoint).normalize()
    // 角度
    const directionVec2 = new Vector2(direction.z, direction.x)
    // 目前暂时设置最小的半径为两条车道宽度 + 1m，差不多 8m
    const minRadius = Constant.laneWidth * 2 + 1
    const maxRadius = 1000
    const angle = directionVec2.angle() % (PI * 2)

    if (radius < minRadius) {
      radius = minRadius
      // 如果限制了最小半径，需要重新定义半径上的点
      _point = centerPoint.clone().addScaledVector(direction, radius)
    } else if (radius > maxRadius) {
      radius = maxRadius
      // 如果限制了最大半径，也需要重新定义半径上的点
      _point = centerPoint.clone().addScaledVector(direction, radius)
    }

    // 每一次渲染预览虚拟的元素之前，都需要销毁上一次的虚拟预览元素
    disposeAllVirtualElement(parent)
    // 包含辅助交付的元素也需要销毁
    disposeHelperElement({ parent: this.editHelper.container })

    const config3dStore = useConfig3dStore()

    const circleOptions = renderVirtualCircleRoad({
      parent,
      radiusPoint: _point,
      center: centerPoint.clone(),
      radius,
      angle,
      // 从配置中读取当前支持的预留路口数
      junctionNumber: config3dStore.reserveJunction,
      zoom,
    })

    // 先清空
    circleRoadOptions.length = 0
    // 再缓存配置
    circleRoadOptions.push(...circleOptions)

    this.render()
  }

  dragEndCircleRoad (options: common.IDragOptions) {
    const selected = getSelected()
    if (!selected) return

    const parent = this.editHelper.container

    const interactionStore = useCircleRoadInteraction()
    // 如果此时有选中的道路，则需要取消选中
    interactionStore.unselectRoad()

    // 基于当前缓存的配置，绘制对应的环形道路
    const roadStore = useRoadStore()
    roadStore.createCircleRoad(circleRoadOptions)

    disposeAllVirtualElement(parent)
    this.render()
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditCircleRoadHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()

    this.addDragListener()

    // 注册当前模式下，可支持
    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['helperArea'], // 响应环形道路首尾的辅助交互区域
    })

    // 忽略基于 ground 平面点击进行拖拽时 offset 偏移计算
    registryIgnoreDragOffsetElements('ground')
  }

  initDispatchers () {
    const roadStore = useRoadStore()
    const interactionStore = useCircleRoadInteraction()
    interactionStore.$reset()

    const parent = this.editHelper.container

    this.unsubscribeRoadStore = roadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          switch (name) {
            case 'createCircleRoad':
            case 'updateArcRoad': {
              interactionStore.updateTimestamp()

              // 销毁渲染的辅助元素
              disposeHelperElement({ parent })
              disposeAllVirtualElement(parent)

              // 在完成环形道路角度的调整后，判断如果之前有选中的元素，需要保持选中的状态
              if (interactionStore.roadId) {
                interactionStore.selectRoad(interactionStore.roadId)
              }

              this.render()
              break
            }
          }
        })

        onError((err) => {
          console.log(err)
        })
      },
    )

    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          switch (name) {
            case 'applyState': {
              store.updateTimestamp()
              // 销毁渲染的辅助元素
              disposeHelperElement({ parent })
              disposeAllVirtualElement(parent)

              // 如果有选中的元素，则渲染选中的效果
              if (store.roadId) {
                store.selectRoad(store.roadId)
              }
              this.render()
              break
            }
            case 'selectRoad': {
              store.updateTimestamp()

              const { roadId } = store
              // 选中车道，则展示当前道路的属性面板
              const editRoadStore = useEditRoadStore()
              // 当前选中车道对应的参考线控制点集 id
              const currentCp = editRoadStore.getControlPointByRoadId(roadId)
              // 获取控制点控制的所有正向和反向道路的 roadId
              if (!currentCp) return
              const [forwardRoadId] = currentCp.roadId

              const forwardRoad = getRoad(forwardRoadId)
              const parent = this.editHelper.container

              if (forwardRoad && forwardRoad.isCircleRoad) {
                // 如果只有一条单向环形道路
                // 打开地图文件，所有的道路都是正向道路，如果圆弧道路在之前创建的时候就是反向道路，需要调整辅助元素创建的位置
                if (
                  forwardRoad.isCircleRoad &&
                  forwardRoad.circleOption &&
                  forwardRoad.circleOption.isClockwise
                ) {
                  // 顺时针的道路，视为是反向的
                  createCircleEndArea({
                    road: [null, forwardRoad],
                    parent,
                    isClockwise: true,
                  })
                } else {
                  createCircleEndArea({ road: [forwardRoad, null], parent })
                }
              }

              this.render()
              break
            }
            case 'unselectRoad': {
              store.updateTimestamp()

              // 销毁渲染的辅助元素
              disposeHelperElement({ parent })
              disposeAllVirtualElement(parent)
              this.render()
              break
            }
            default:
              break
          }
        })

        onError((err) => {
          console.log(err)
        })
      },
    )
  }

  onActivate () {}
  onDeactivate () {
    // 在插件取消激活时，重置插件内所有的交互状态
    const interactionStore = useCircleRoadInteraction()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.editHelper.dispose()
    this.unsubscribeRoadStore && this.unsubscribeRoadStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
  }
}

export default new EditLaneWidthPlugin()
