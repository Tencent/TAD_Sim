import { throttle } from 'lodash'
import {
  CatmullRomCurve3,
  Color,
  type Intersection,
  Line,
  MOUSE,
  Mesh,
  Vector3,
} from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditRoadHelper from './main3d/'
import {
  ControlLineColor,
  ControlPointColor,
  activeSelectedControlLineColor,
  activeSelectedControlPointColor,
  disposeAllControlPointsAndCurve,
  disposeAllVirtualElement,
  disposeOneRoadControlPointAndCurve,
  renderControlCurve,
  renderControlPoint,
  renderOneRoadControlPointAndCurve,
  renderVirtualControlPoint,
  renderVirtualRoad,
  resetAllControlPointColor,
  resetControlLineColor,
} from './main3d/draw'
import { type IAddControlPointRes, useEditRoadStore } from './store'
import { useRoadInteractionStore } from './store/interaction'
import root3d from '@/main3d'
import {
  getSelected,
  registryHoverElement,
  registryIgnoreDragOffsetElements,
  setInteractionPlane,
  setSelected,
} from '@/main3d/controls/dragControls'
import {
  CatmullromTension,
  CurveType,
  axisY,
  fixedPrecision,
  transformVec3ByObject,
  worldCenter,
} from '@/utils/common3d'
import { useRoadStore } from '@/stores/road'
import { useHistoryStore } from '@/stores/history'
import i18n from '@/locales'
import { useJunctionStore } from '@/stores/junction'
import { getJunction, getRoad } from '@/utils/mapCache'
import {
  removeObjectInRoad,
  updateObjectInRoad,
} from '@/main3d/dispatcher/common'
import { useConfig3dStore } from '@/stores/config3d'
import selectionBox from '@/main3d/selection/index'
import ee, { CustomEventType } from '@/utils/event'
import {
  createHighlightRoadMask,
  disposeHighlightRoadMask,
} from '@/main3d/render/road'
import { useObjectStore } from '@/stores/object'
import { handleIntersectionGuideLineByRoadId } from '@/stores/object/customRoadSign/intersectionGuideLine'

class EditRoadPlugin extends TemplatePlugin {
  editHelper: EditRoadHelper
  unsubscribeEditRoadStore: Function
  unsubscribeInteractionStore: Function
  unsubscribeConfigStore: Function
  unsubscribeRoadStore: Function
  constructor () {
    super()
    this.config = config

    // 当前是否能拖拽道路
    let canDragRoad = false
    // 当前是否能选取多个参考线控制点
    let canSelectMultiplePoints = false

    // 覆写模板中定义的空的交互事件
    this.dragDispatchers = {
      dragStart: (options: common.IDragOptions) => {
        if (!this.enabled) return
        // 判断是否有符合当前模式的交互物体
        const { intersections, pointerdownButton } = options.object
        if (pointerdownButton === MOUSE.LEFT) {
          // 如果是左键点击，则选中对应的元素包括：整条道路、控制点、参考线
          const interactionStore = useRoadInteractionStore()
          intersections.forEach((intersection: Intersection) => {
            if (getSelected()) return

            const { object } = intersection

            switch (object.name) {
              case 'controlPoint': {
                // 如果是鼠标左键 down 了控制点，有可能后续是选中，有可能是移动
                setSelected(intersection)

                // 由于在开始拖拽的阶段，不确定是否会移动控制点，需要提前设置移动的基准平面
                // 当前设置成高度为零的水平面
                setInteractionPlane({
                  normal: axisY,
                  point: worldCenter,
                })

                // @ts-expect-error
                const { pointId: refPointId, controlPointId } = object
                const parent = this.editHelper.container
                // 如果当前 down 选中的点，不是上一次交互选中的点，则需要重置所有控制点的颜色
                if (interactionStore?.refPoint?.id !== refPointId) {
                  resetAllControlPointColor(parent)
                }

                // 判断当前交互的控制点，是否是已经框选的多个控制点之一
                let isInclude = false

                if (refPointId && controlPointId) {
                  const editRoadStore = useEditRoadStore()
                  const refPoint = editRoadStore.getRefPointById(
                    controlPointId,
                    refPointId,
                  )
                  if (refPoint) {
                    interactionStore.selectRefPoint(refPoint)
                  }
                  for (const _refPoint of interactionStore.moreRefPoints) {
                    if (isInclude) continue
                    if (_refPoint.id === refPointId) {
                      isInclude = true
                    }
                  }
                }

                if (isInclude) {
                  // 如果是选中了多个控制点的前提下，且拖拽了其中一个控制点，则应该一起联动更新位置
                  if (interactionStore.moreRefPoints.length > 0) {
                    for (const _refPoint of interactionStore.moreRefPoints) {
                      const targetObject = parent.getObjectByProperty(
                        'pointId',
                        _refPoint.id,
                      )
                      if (targetObject instanceof Mesh) {
                        const { material } = targetObject
                        material.color.set(ControlPointColor.selected)
                        material.needsUpdate = true
                      }
                    }
                  }
                } else {
                  // 取消框选的多个控制点的选中效果
                  if (interactionStore.moreRefPoints.length > 0) {
                    for (const _refPoint of interactionStore.moreRefPoints) {
                      const targetObject = parent.getObjectByProperty(
                        'pointId',
                        _refPoint.id,
                      )
                      if (targetObject instanceof Mesh) {
                        const { material } = targetObject
                        material.color.set(ControlPointColor.normal)
                        material.needsUpdate = true
                      }
                    }
                  }
                  interactionStore.unselectMoreRefPoints()
                }

                // 如果选中的是控制点，则有可能拖动对应的控制点，需要提前禁用视角控制器
                root3d.setViewControlsEnabled(false)
                root3d.core.render()
                break
              }
              case 'controlLine': {
                setSelected(intersection)
                break
              }
              case 'lane': {
                // 如果是鼠标左键 down 了车道，有可能后续是选中，有可能是移动
                setSelected(intersection)

                // 由于在开始拖拽的阶段，不确定是否会移动整条道路，需要提前设置移动的基准平面
                // 当前设置成高度为零的水平面
                setInteractionPlane({
                  normal: axisY,
                  point: worldCenter,
                })
                // @ts-expect-error
                // 交互状态模块选中车道对应的道路
                const { roadId } = object
                const editRoadStore = useEditRoadStore()
                // 当前选中的车道，对应的控制点集 id
                const currentCp = editRoadStore.getControlPointByRoadId(roadId)

                let lastCp
                if (interactionStore.roadId) {
                  lastCp = editRoadStore.getControlPointByRoadId(
                    interactionStore.roadId,
                  )
                } else if (interactionStore.refLineCpId) {
                  lastCp = editRoadStore.getControlPointById(
                    interactionStore.refLineCpId,
                  )
                } else if (interactionStore.refPoint) {
                  lastCp = editRoadStore.getControlPointById(
                    interactionStore.refPoint.parentId,
                  )
                }

                // 只有当前选中的车道对应的控制点集 id，跟上一次交互的控制点集 id 一致，才能响应道路的整体拖拽
                if (currentCp && currentCp === lastCp) {
                  root3d.setViewControlsEnabled(false)
                  canDragRoad = true
                }
                break
              }
              case 'ground': {
                setSelected(intersection)

                // 触发开始拖拽选取的逻辑
                if (!canSelectMultiplePoints) {
                  canSelectMultiplePoints = true
                  setInteractionPlane({
                    normal: axisY,
                    point: worldCenter,
                  })
                  const { originEvent } = options.object
                  // 判断当前是否有选中的道路，展示了对应的道路参考线控制点
                  const { roadId, refPoint, refLineCpId } = interactionStore
                  const points: Array<biz.IRefPoint> = []
                  let cp: biz.IControlPoint | null | undefined
                  const editRoadStore = useEditRoadStore()
                  if (roadId) {
                    // 如果当前选中的是道路，则基于道路查询对应的控制点集
                    cp = editRoadStore.getControlPointByRoadId(roadId)
                  } else if (refPoint) {
                    // 选中某一个具体的控制点
                    cp = editRoadStore.getControlPointById(refPoint.parentId)
                  } else if (refLineCpId) {
                    // 选中的道路参考线的控制点集id
                    cp = editRoadStore.getControlPointById(refLineCpId)
                  }
                  if (cp) {
                    // 将选中道路的参考线控制点添加到多选交互的备选点集合中
                    points.push(...cp.points)
                  }

                  selectionBox.onSelectStart({
                    coordinate: {
                      x: originEvent.offsetX,
                      y: originEvent.offsetY,
                    },
                    points,
                  })
                }
                break
              }
              default:
                break
            }
          })
        } else if (pointerdownButton === MOUSE.RIGHT) {
          // 如果是右键点击，判断是否有可新增控制点的平面
          intersections.forEach((intersection: Intersection) => {
            // 如果已经有符合交互的选中元素了，则返回
            if (getSelected()) return

            const { object } = intersection
            switch (object.name) {
              case 'ground':
              case 'lane':
              case 'controlLine':
                setSelected(intersection)
                break
              default:
                break
            }
          })
        }
      },
      // move 事件触发的频率过高，通过节流提升拖拽预览效果的性能
      drag: throttle((options: common.IDragOptions) => {
        if (!this.enabled) return

        const selected = getSelected()
        if (!selected) return

        if (selected.object.name === 'controlPoint') {
          // 拖拽控制点，展示拖拽的预览效果
          this.dragPreviewControlPoint(options)
        } else if (selected.object.name === 'lane') {
          if (!canDragRoad) return

          // 拖拽整条道路，展示拖拽的预览效果
          this.dragPreviewRoad(options)
        } else if (selected.object.name === 'ground') {
          // TODO 点击地平面开始拖拽，支持框选对应的控制点
          if (canSelectMultiplePoints) {
            const { originEvent } = options.object
            selectionBox.onSelectMove({
              x: originEvent.offsetX,
              y: originEvent.offsetY,
            })
          }
        }
      }, 100),
      // up 事件有可能存在误触连续调用，通过节流来避免
      dragEnd: throttle((options) => {
        if (!this.enabled) return
        const selected = getSelected()
        if (!selected) return

        // 不管选中什么物体，在拖拽结束后，恢复视角控制器
        root3d.setViewControlsEnabled(true)

        if (selected.object.name === 'controlPoint') {
          // 销毁所有辅助拖拽的预览元素
          disposeAllVirtualElement(this.editHelper.container)

          // 调用 store 中实际修改参考线控制点的方法
          const { object: controlPointMesh } = selected
          const { newPosition } = options.object
          // @ts-expect-error
          const { pointId, controlPointId } = controlPointMesh
          const editRoadStore = useEditRoadStore()
          const interactionStore = useRoadInteractionStore()
          const cpData = editRoadStore.getControlPointById(controlPointId)
          if (!cpData) return

          // 获取移动的偏移量
          let offset: common.vec3 | null = null
          for (const _point of cpData.points) {
            if (offset) continue
            if (_point.id === pointId) {
              offset = {
                x: fixedPrecision(newPosition.x - _point.x),
                y: fixedPrecision(newPosition.y - _point.y),
                z: fixedPrecision(newPosition.z - _point.z),
              }
            }
          }

          // 判断当前移动的是一个控制点，还是框选的多个控制点
          let isInclude = false
          // 需要预览移动的控制点
          const selectedPoints: Array<biz.IRefPoint> = []

          const moreRefPointIds: Array<string> = []
          for (const p of interactionStore.moreRefPoints) {
            moreRefPointIds.push(p.id)
            if (!isInclude && p.id === pointId) {
              isInclude = true
            }
          }
          if (moreRefPointIds.length > 1 && isInclude && offset) {
            // 移动多个框选的控制点
            cpData.points.forEach((p) => {
              if (moreRefPointIds.includes(p.id)) {
                // 如果属于框选的控制点，则需要加上偏移量
                selectedPoints.push({
                  ...p,
                  x: p.x + offset.x,
                  y: p.y + offset.y,
                  z: p.z + offset.z,
                })
              }
            })

            editRoadStore.moveMoreControlPoints({
              cpId: cpData.id,
              points: selectedPoints,
            })
          } else {
            // 移动单个控制点
            editRoadStore.moveControlPoint(pointId, controlPointId, newPosition)
          }
        } else if (selected.object.name === 'lane') {
          if (!canDragRoad) return

          // 针对道路最终放置的位置，调用 store 中的更新方法
          // 销毁所有辅助拖拽的预览元素
          disposeAllVirtualElement(this.editHelper.container)

          // 拖拽道路移动位置结束后，取消缓存的多个选中控制点
          const interactionStore = useRoadInteractionStore()
          interactionStore.unselectMoreRefPoints()

          // @ts-expect-error
          const { roadId } = selected.object
          if (!roadId) return

          // 通过 roadId 找到对应的参考线控制点集
          const editRoadStore = useEditRoadStore()
          const currentCp = editRoadStore.getControlPointByRoadId(roadId)
          if (!currentCp) return

          // 移动整条道路，对应的参考线控制点也得一并移动相同的位移
          // 因此调用 editRoadStore 当前编辑模式下的方法更正确更优
          const { newPosition } = options.object
          if (!newPosition) return

          const offset = newPosition.clone().sub(selected.point)
          // 只保留 zx 平面的分量
          offset.y = 0

          editRoadStore.moveAllControlPoint({
            controlPointId: currentCp.id,
            offset,
          })

          canDragRoad = false
        } else if (selected.object.name === 'ground') {
          // 框选多个控制点
          if (canSelectMultiplePoints) {
            // 重置状态
            canSelectMultiplePoints = false
            selectionBox.onSelectEnd()
          }
        }
      }, 100),
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { pointerdownButton } = options.object
        const selected = getSelected()
        if (!selected) return

        // 不管选中什么物体，click结束后恢复视角控制器
        root3d.setViewControlsEnabled(true)

        if (pointerdownButton === MOUSE.LEFT) {
          const interactionStore = useRoadInteractionStore()
          switch (selected.object.name) {
            case 'controlPoint':
              {
                // 销毁所有辅助拖拽的预览元素
                disposeAllVirtualElement(this.editHelper.container)

                // 取消缓存的多个选中控制点
                interactionStore.unselectMoreRefPoints()

                // 选中一个道路参考线控制点【虽然 down 事件已经选中，属于同样的逻辑，但 click 事件含义不一样】
                // @ts-expect-error
                const { pointId: refPointId, controlPointId } = selected.object
                if (refPointId && controlPointId) {
                  const editRoadStore = useEditRoadStore()
                  const refPoint = editRoadStore.getRefPointById(
                    controlPointId,
                    refPointId,
                  )
                  if (!refPoint) return

                  interactionStore.selectRefPoint(refPoint)
                }
              }
              break
            case 'controlLine': {
              // @ts-expect-error
              const { controlPointId } = selected.object
              if (controlPointId) {
                // 取消缓存的多个选中控制点
                interactionStore.unselectMoreRefPoints()

                interactionStore.selectRefLine(controlPointId)
              }
              break
            }
            case 'lane': {
              // 销毁所有辅助拖拽的预览元素
              disposeAllVirtualElement(this.editHelper.container)

              // 取消缓存的多个选中控制点
              interactionStore.unselectMoreRefPoints()

              // 选中一条车道对应的道路【虽然 down 事件已经选中，属于同样的逻辑，但 click 事件含义不一样】
              // @ts-expect-error
              const { roadId, sectionId, laneId } = selected.object
              if (roadId && sectionId && laneId) {
                interactionStore.selectLane(roadId, sectionId, laneId)
              }
              break
            }
            case 'ground': {
              // 鼠标左键点击 ground，则取消当前道路、控制点的选中状态
              interactionStore.unselectExceptSomeone()

              // 取消缓存的多个选中控制点
              interactionStore.unselectMoreRefPoints()

              if (canSelectMultiplePoints) {
                canSelectMultiplePoints = false
                selectionBox.onSelectEnd()
              }
              break
            }
            default:
              break
          }
        } else if (pointerdownButton === MOUSE.RIGHT) {
          const selected = getSelected()
          if (!selected) return
          const { object } = selected
          switch (object.name) {
            case 'ground':
              // 只有地面元素才能响应鼠标右键的点击效果
              this.clickAddControlPoint(selected)
              break
            case 'controlLine': {
              const interactionStore = useRoadInteractionStore()
              if (!interactionStore.refLineCpId) return

              // 只有在道路参考线选中的前提下，才响应在参考线上右键 click 新增补间控制点
              this.clickAddTweenControlPoint(selected)
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
        if (elementName === 'controlPoint') {
          // 展示参考线控制点的 hovered 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (
              !new Color(ControlPointColor.selected).equals(material.color)
            ) {
              material.color.set(ControlPointColor.hovered)
              material.needsUpdate = true
              this.render()
            }
          }
        } else if (elementName === 'controlLine') {
          // 展示参考线控制线的 hovered 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Line) {
            const { material } = targetObject
            if (
              !new Color(ControlLineColor.selected).equals(material.color)
            ) {
              material.color.set(ControlLineColor.hovered)
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
        if (elementName === 'controlPoint') {
          // 取消参考线控制点的 hovered 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (
              !new Color(ControlPointColor.selected).equals(material.color)
            ) {
              material.color.set(ControlPointColor.normal)
              material.needsUpdate = true
              this.render()
            }
          }
        } else if (elementName === 'controlLine') {
          // 取消参考线控制线的 hovered 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Line) {
            const { material } = targetObject
            if (
              !new Color(ControlLineColor.selected).equals(material.color)
            ) {
              material.color.set(ControlLineColor.normal)
              material.needsUpdate = true
              this.render()
            }
          }
        }
      },
    }
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditRoadHelper({
      render: this.render,
    })
    this.editHelper.init()

    const parent = this.editHelper.container

    this.scene.add(parent)

    // 注册框选多个道路参考线控制点的监听
    selectionBox.init({
      camera: root3d.core.oCamera,
      renderer: root3d.core.renderer,
    })

    const interactionStore = useRoadInteractionStore()
    // 监听框选多个控制点的事件
    ee.on(CustomEventType.controlPoint.hover, (data: Array<biz.IRefPoint>) => {
      // 先重置所有控制点和线的颜色
      resetAllControlPointColor(parent)
      resetControlLineColor(parent)

      if (data.length < 1) return

      // 将框选的元素，展示高亮的状态
      for (const _refPoint of data) {
        const targetObject = parent.getObjectByProperty('pointId', _refPoint.id)
        if (targetObject instanceof Mesh) {
          const { material } = targetObject
          material.color.set(ControlPointColor.selected)
          material.needsUpdate = true
        }
      }

      root3d.core.render()
    })

    ee.on(CustomEventType.controlPoint.select, (data: Array<biz.IRefPoint>) => {
      // 先重置所有控制点和线的颜色
      resetAllControlPointColor(parent)
      resetControlLineColor(parent)

      if (data.length < 1) return

      // 将框选的元素，展示高亮的状态
      for (const _refPoint of data) {
        const targetObject = parent.getObjectByProperty('pointId', _refPoint.id)
        if (targetObject instanceof Mesh) {
          const { material } = targetObject
          material.color.set(ControlPointColor.selected)
          material.needsUpdate = true
        }
      }
      // 从数据层面选中多个控制点
      if (data.length === 1) {
        // 只有1个控制点，则单独选中
        interactionStore.selectRefPoint(data[0])
      } else {
        // 2个以上控制点
        interactionStore.selectMoreRefPoints(data)
      }

      root3d.core.render()
    })

    this.initDispatchers()

    this.addDragListener()

    // 注册当前模式下，可支持 hover 交互的元素名称
    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['controlPoint', 'controlLine', 'lane'],
    })

    // 忽略拖拽车道过程中的 offset 计算
    registryIgnoreDragOffsetElements('lane')
  }

  /**
   * 拖拽移动虚拟的道路
   * @param options
   * @returns
   */
  dragPreviewRoad (options: common.IDragOptions) {
    const { newPosition } = options.object

    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { roadId } = selected.object
    if (!roadId || !newPosition) return

    // 通过 roadId 找到对应的参考线控制点集
    const editRoadStore = useEditRoadStore()
    const currentCp = editRoadStore.getControlPointByRoadId(roadId)
    if (!currentCp) return

    // newPosition 为鼠标射线跟地平面的相交点，需要跟 down 时的相交点计算偏移量
    const offset = newPosition.clone().sub(selected.point)
    // 只保留 zx 平面的分量
    offset.y = 0

    // 通过控制点来计算新的参考线控制点预览位置
    const newPoints = currentCp.points.map((p) => {
      const newPoint = new Vector3(p.x + offset.x, p.y, p.z + offset.z)
      return newPoint
    })
    const newKeyPath = new CatmullRomCurve3(newPoints)
    newKeyPath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      newKeyPath.tension = CatmullromTension
    }

    // 在创建新的虚拟预览元素之前，先销毁历史存在的虚拟预览元素
    disposeAllVirtualElement(this.editHelper.container)

    // 区分正向和反向的道路，将控制点形成的关键曲线传给对应的道路状态
    const [forwardRoadId] = currentCp.roadId
    const roadStore = useRoadStore()
    // 正向车道
    const forwardRoad = roadStore.getRoadById(forwardRoadId)
    if (forwardRoad) {
      renderVirtualRoad({
        keyPath: newKeyPath,
        road: forwardRoad,
        parent: this.editHelper.container,
      })
    }

    this.render()
  }

  /**
   * 拖拽控制点展示预览放置位置的效果
   * @param options
   * @returns
   */
  dragPreviewControlPoint (options: common.IDragOptions) {
    // 新的控制点位置
    const { newPosition } = options.object

    // 基于新的预览控制点位置，重新计算一条虚拟的道路
    const editRoadStore = useEditRoadStore()
    const interactionStore = useRoadInteractionStore()
    const roadStore = useRoadStore()

    const selected = getSelected()
    if (!selected) return
    const { refPoint } = interactionStore
    if (!refPoint) return
    const { id: pointId, parentId: controlPointId } = refPoint
    const controlPointData = editRoadStore.getControlPointById(controlPointId)
    if (!controlPointData) return
    const { points: originPoints, roadId } = controlPointData
    const keyPathPoints: Array<Vector3> = []
    // 先获取鼠标拖拽控制点的偏移量
    let offset: common.vec3 | null = null
    for (const _point of originPoints) {
      if (offset) continue
      if (_point.id === pointId) {
        offset = {
          x: fixedPrecision(newPosition.x - _point.x),
          y: fixedPrecision(newPosition.y - _point.y),
          z: fixedPrecision(newPosition.z - _point.z),
        }
      }
    }

    // 判断当前交互的控制点，是否是已经框选的多个控制点之一
    let isInclude = false
    // 需要预览移动的控制点
    const selectedPoints = []

    const moreRefPointIds: Array<string> = []
    for (const p of interactionStore.moreRefPoints) {
      moreRefPointIds.push(p.id)
      if (!isInclude && p.id === pointId) {
        isInclude = true
      }
    }

    if (moreRefPointIds.length > 1 && isInclude && offset) {
      originPoints.forEach((p) => {
        if (moreRefPointIds.includes(p.id)) {
          // 如果属于框选的控制点，则需要加上偏移量
          const position = new Vector3(
            p.x + offset.x,
            p.y + offset.y,
            p.z + offset.z,
          )
          keyPathPoints.push(position)
          selectedPoints.push(position)
        } else {
          keyPathPoints.push(new Vector3(p.x, p.y, p.z))
        }
      })
    } else {
      // 如果是选中一个控制点
      originPoints.forEach((p) => {
        if (p.id !== pointId) {
          // 如果控制点不是拖拽的控制点，则直接将顶点坐标存入
          keyPathPoints.push(new Vector3(p.x, p.y, p.z))
        } else {
          const position = new Vector3(
            newPosition.x,
            newPosition.y,
            newPosition.z,
          )
          keyPathPoints.push(position)
          selectedPoints.push(position)
        }
      })
    }

    // 将新的控制点和控制的道路信息，用于绘制预览的虚拟放置结果信息
    const keyPath = new CatmullRomCurve3(keyPathPoints)
    keyPath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      keyPath.tension = CatmullromTension
    }

    // 在创建新的虚拟预览元素之前，先销毁历史存在的虚拟预览元素
    disposeAllVirtualElement(this.editHelper.container)

    // 绘制预览的虚拟控制点和控制线
    renderVirtualControlPoint({
      points: selectedPoints,
      parent: this.editHelper.container,
    })

    // 绘制预览的虚拟正向道路
    const [forwardRoadId] = roadId
    const forwardRoad = roadStore.getRoadById(forwardRoadId)
    if (forwardRoad) {
      renderVirtualRoad({
        keyPath,
        road: forwardRoad,
        parent: this.editHelper.container,
      })
    }

    this.render()
  }

  /**
   * 响应点击操作新增道路参考线控制点
   * @param selected
   */
  clickAddControlPoint (selected: Intersection) {
    const intersectionPoint = selected.point

    // 判断是否存在当前选中的道路，如果存在的话，则接着选中道路的控制点继续添加，否则视为新建道路
    const editRoadStore = useEditRoadStore()
    const interactionStore = useRoadInteractionStore()

    if (interactionStore.refPoint) {
      // 有选中的参考点，则将新的点推入到之前的控制点集合中
      const { parentId, id: refPointId } = interactionStore.refPoint
      // 判断上一次交互的参考点，是否是首或尾的端点
      const { isTail, isHead } = editRoadStore.isEndRefPoint(
        parentId,
        refPointId,
      )
      if (isTail) {
        // 将选中的参考点换成新增的参考点
        editRoadStore.addControlPoint({
          vec3: intersectionPoint,
          isTail: true,
          cpId: parentId,
          onSuccess: (res: IAddControlPointRes) => {
            interactionStore.selectRefPoint(res.refPoint)
          },
        })
      } else if (isHead) {
        editRoadStore.addControlPoint({
          vec3: intersectionPoint,
          isTail: false,
          cpId: parentId,
          onSuccess: (res: IAddControlPointRes) => {
            interactionStore.selectRefPoint(res.refPoint)
          },
        })
      }
      // 首尾都不是，则不做任何操作
    } else {
      // 没有选中的参考点，则视为新建一组控制点集合
      editRoadStore.addControlPoint({
        vec3: intersectionPoint,
        onSuccess: (res: IAddControlPointRes) => {
          interactionStore.selectRefPoint(res.refPoint)
        },
      })
    }
  }

  /**
   * 响应点击操作新增参考线的【补间】控制点
   * @param selected
   */
  clickAddTweenControlPoint (selected: Intersection) {
    const { point } = selected
    // @ts-expect-error
    const { controlPointId } = selected.object

    const editRoadStore = useEditRoadStore()
    const interactionStore = useRoadInteractionStore()
    const cpData = editRoadStore.getControlPointById(controlPointId)
    if (!cpData) return

    // 调用编辑模式 store 中插入补间控制点的方法
    editRoadStore.addTweenControlPoint({
      point,
      cpId: controlPointId,
      onSuccess: (res: IAddControlPointRes) => {
        interactionStore.selectRefPoint(res.refPoint)
      },
    })
  }

  initDispatchers () {
    const editRoadStore = useEditRoadStore()
    const roadStore = useRoadStore()
    const historyStore = useHistoryStore()
    const interactionStore = useRoadInteractionStore()
    const config3dStore = useConfig3dStore()
    editRoadStore.$reset()
    roadStore.$reset()
    interactionStore.$reset()

    const parent = this.editHelper.container

    this.unsubscribeConfigStore = config3dStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          if (name === 'updateRoadType') {
            // 更新 roadType，如果有选中的元素，则需要取消选中
            interactionStore.unselectExceptSomeone()
          }
        })
      },
    )
    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        // 缓存上一次点击选中的 roadId
        let lastSelectedRoadId = ''
        if (name === 'selectRoad' || name === 'selectLane') {
          lastSelectedRoadId = store.roadId
        }

        after(async (res) => {
          if (!this.enabled && name !== 'removeCurrentRoad') return
          switch (name) {
            case 'applyState': {
              interactionStore.updateTimestamp()
              // 还原状态时，增量对控制点和控制线做重绘
              const [currentState, diffState] = args
              const {
                roadId: currentRoadId,
                refPoint: currentRefPoint,
                refLineCpId: currentRefLineCpId,
              } = currentState
              resetAllControlPointColor(parent)

              const editRoadStore = useEditRoadStore()

              // 直接销毁所有的辅助元素
              disposeAllControlPointsAndCurve(parent)
              // 如果之前有高亮的道路，先销毁辅助高亮蒙层再渲染新的
              disposeHighlightRoadMask(parent)

              // 渲染所有孤立控制点
              editRoadStore.controlPoints.forEach((cp) => {
                const { id, points } = cp
                if (points.length === 1) {
                  renderControlPoint({
                    cpId: id,
                    points,
                    parent,
                    preDispose: true,
                  })
                }
              })

              // 重新渲染选中元素的控制点和控制线
              let currentCp
              if (currentRefPoint) {
                currentCp = editRoadStore.getControlPointById(
                  currentRefPoint.parentId,
                )
              } else if (currentRefLineCpId) {
                currentCp =
                  editRoadStore.getControlPointById(currentRefLineCpId)
              } else if (currentRoadId) {
                currentCp = editRoadStore.getControlPointByRoadId(currentRoadId)

                // 如果上一个状态有道路的选中
                const currentRoad = getRoad(currentRoadId)
                if (currentRoad) {
                  // 则展示对应道路的高亮
                  createHighlightRoadMask({
                    road: currentRoad,
                    parent,
                  })
                }
              }

              if (currentCp) {
                // 判断当前控制点管理的道路是否是环形道路
                const [forwardRoadId] = currentCp.roadId
                let isCircleRoad = false
                const forwardRoad = getRoad(forwardRoadId)
                if (forwardRoad && forwardRoad.isCircleRoad) {
                  isCircleRoad = true
                }

                if (!isCircleRoad) {
                  // 如果不是环形道路，则撤销和重做时，会渲染对应的道路参考线的控制点和控制线
                  renderOneRoadControlPointAndCurve({
                    parent,
                    cpId: currentCp.id,
                    points: currentCp.points,
                  })

                  // 激活当前状态的辅助元素高亮效果
                  if (currentRefPoint) {
                    activeSelectedControlPointColor({
                      parent,
                      refPointId: currentRefPoint.id,
                    })
                  }
                  if (currentRefLineCpId) {
                    activeSelectedControlLineColor({
                      parent,
                      controlPointId: currentRefLineCpId,
                    })
                  }
                }
              }

              this.render()
              return
            }
            case 'selectLane':
            case 'selectRoad': {
              interactionStore.updateTimestamp()
              const currentCp = editRoadStore.getControlPointByRoadId(
                store.roadId,
              )
              if (!currentCp) return
              const road = getRoad(store.roadId)
              if (!road) return

              const lastCp =
                editRoadStore.getControlPointByRoadId(lastSelectedRoadId)

              if (lastCp) {
                disposeOneRoadControlPointAndCurve({
                  parent,
                  cpId: lastCp.id,
                })
              }

              if (!road.isCircleRoad) {
                // 只有是普通的曲线道路，才会展示道路参考线的控制点和控制线
                // 环形道路不展示
                renderOneRoadControlPointAndCurve({
                  parent,
                  cpId: currentCp.id,
                  points: currentCp.points,
                  preDispose: true,
                })
              }

              // 如果之前有高亮的道路，先销毁辅助高亮蒙层再渲染新的
              disposeHighlightRoadMask(parent)
              // 选中道路，展示道路的高亮蒙层
              createHighlightRoadMask({
                road,
                parent,
              })

              this.render()
              break
            }
            case 'unselectRoad': {
              interactionStore.updateTimestamp()
              const unselectCpId = res as string
              if (unselectCpId) {
                let currentCpId = ''
                if (store.roadId) {
                  const currentCp = editRoadStore.getControlPointByRoadId(
                    store.roadId,
                  )
                  if (currentCp) {
                    currentCpId = currentCp.id
                  }
                } else if (store.refPoint) {
                  currentCpId = store.refPoint.parentId
                } else if (store.refLineCpId) {
                  currentCpId = store.refLineCpId
                }

                // 当前没有选中的道路，或当前选中的道路对应的控制点集 id 跟取消选中的控制点集 id 不同
                if (!currentCpId || currentCpId !== unselectCpId) {
                  // 如果之前有高亮的道路，则销毁对应的辅助高亮蒙层
                  disposeHighlightRoadMask(parent)

                  disposeOneRoadControlPointAndCurve({
                    parent,
                    cpId: unselectCpId,
                  })
                  this.render()
                }
              }
              break
            }
            case 'selectRefLine': {
              interactionStore.updateTimestamp()
              if (!res) return
              const cpId = args[0] as string
              const currentCp = editRoadStore.getControlPointById(cpId)
              // 优先判断是否需要重新绘制一遍辅助元素
              if (currentCp) {
                renderOneRoadControlPointAndCurve({
                  parent,
                  cpId: currentCp.id,
                  points: currentCp.points,
                  preDispose: true,
                })
              }
              // 激活选中的参考线
              activeSelectedControlLineColor({
                controlPointId: cpId,
                parent,
              })
              this.render()
              break
            }
            case 'unselectRefLine': {
              interactionStore.updateTimestamp()
              resetControlLineColor(parent)
              const unselectCpId = res as string
              if (unselectCpId) {
                const currentCpId = store.refLineCpId
                // 当前没有选中的控制点，或当前选中的控制点集跟取消选中的不是同一个控制点集
                if (!currentCpId || currentCpId !== unselectCpId) {
                  disposeOneRoadControlPointAndCurve({
                    parent,
                    cpId: unselectCpId,
                  })
                }
              }

              this.render()
              break
            }
            case 'selectRefPoint': {
              interactionStore.updateTimestamp()
              // 将选中的参考线控制点设置成激活的选中颜色
              if (!res) return
              const refPointData = args[0]
              activeSelectedControlPointColor({
                refPointId: refPointData.id,
                parent,
              })
              const currentCp = editRoadStore.getControlPointById(
                refPointData.parentId,
              )
              if (currentCp) {
                renderOneRoadControlPointAndCurve({
                  parent,
                  cpId: currentCp.id,
                  points: currentCp.points,
                  preDispose: true,
                })
              }
              this.render()
              break
            }
            case 'unselectRefPoint': {
              interactionStore.updateTimestamp()
              // 将所有的参考线控制点都恢复成初始颜色
              const unselectCpId = res as string
              if (unselectCpId) {
                // 判断当前取消选中的控制点是否是孤立控制点，如果是，则不销毁
                const unselectCp =
                  editRoadStore.getControlPointById(unselectCpId)
                if (unselectCp && unselectCp.points.length < 2) {
                  resetAllControlPointColor(parent)
                  this.render()
                  return
                }

                const currentCpId = store.refPoint?.parentId
                // 当前没有选中的控制点，或当前选中的控制点集跟取消选中的不是同一个控制点集
                if (!currentCpId || currentCpId !== unselectCpId) {
                  disposeOneRoadControlPointAndCurve({
                    parent,
                    cpId: unselectCpId,
                  })
                }
              }
              this.render()
              break
            }
            case 'removeCurrentRoad': {
              interactionStore.updateTimestamp()
              if (!res) return
              const {
                linkJunctionId,
                roadId,
                road: removedRoad,
                removeJunctionRes,
              } = res

              // 删除道路，跟当前道路连通的路口 junctionId 也需要记录
              const _roads = []
              removedRoad && _roads.push(removedRoad)
              const _objectId = []
              const _objects = []

              const { objects, objectId } = removeObjectInRoad(roadId)

              // 将道路关联的物体，添加到缓存中
              _objectId.push(...objectId)
              _objects.push(...objects)

              // 需要对之前道路连通的junction结构做更新
              const _junctions: Array<biz.IJunction> = []

              // 跟道路连通的路口中物体
              for (const removeRes of removeJunctionRes) {
                const {
                  junction: removedJunction,
                  objectId: objectIdInJunction = [],
                  objects: objectsInJunction = [],
                } = removeRes
                _junctions.push(removedJunction)
                // 如果删除道路，连带删除了路口，判断删除的路口中是否带有连带删除的路口物体元素
                // 如果有，则需要添加到缓存中一并删除
                _objectId.push(...objectIdInJunction)
                _objects.push(...objectsInJunction)
              }

              // 针对路口导向线（虽然在路口中跟路口关联，但也跟道路关联），如果删除了对应的 frid 或 trid，也需要联动删除
              handleIntersectionGuideLineByRoadId({
                roadIds: roadId,
                objectIds: _objectId,
                objects: _objects,
                handler: (objectId: string) => {
                  // 调用逻辑层的删除，触发对应渲染层的元素销毁
                  const objectStore = useObjectStore()
                  objectStore.removeRoadSign(objectId, false)
                },
              })

              historyStore.save({
                title: i18n.global.t('actions.road.remove'),
                roadId,
                junctionId: linkJunctionId,
                objectId: _objectId,
                roads: _roads,
                junctions: _junctions,
                objects: _objects,
              })

              this.render()
              break
            }
            default:
              break
          }
        })
        onError((err) => {
          console.error(err)
        })
      },
    )
    this.unsubscribeEditRoadStore = editRoadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after(async (res) => {
          // 如果当前插件不可用，则不会执行订阅方法后的渲染逻辑
          if (!this.enabled) return
          let cpId: string
          let currentCp
          let targetPercent: number
          switch (name) {
            case 'applyState': {
              interactionStore.updateTimestamp()

              // 撤销和重做的应用状态，需要清空多选的状态
              interactionStore.unselectMoreRefPoints()

              // 辅助元素的状态，在 roadInteraction 中控制
              // 还原状态时，增量对控制点和控制线做重绘
              const [, diffState] = args
              for (const item in diffState.controlPoints) {
                if (editRoadStore.controlPoints[item]) {
                  currentCp = editRoadStore.controlPoints[item]
                }
              }
              // 直接销毁所有的辅助元素
              disposeAllControlPointsAndCurve(parent)
              // 渲染所有孤立控制点
              editRoadStore.controlPoints.forEach((cp) => {
                const { id, points } = cp
                if (points.length === 1) {
                  renderControlPoint({
                    cpId: id,
                    points,
                    parent,
                    preDispose: true,
                  })
                }
              })

              if (currentCp) {
                // 判断当前控制点管理的道路是否是环形道路
                const [forwardRoadId] = currentCp.roadId
                let isCircleRoad = false
                const forwardRoad = getRoad(forwardRoadId)
                if (forwardRoad && forwardRoad.isCircleRoad) {
                  isCircleRoad = true
                }

                if (!isCircleRoad) {
                  // 重新渲染选中元素的控制点和控制线
                  renderOneRoadControlPointAndCurve({
                    parent,
                    cpId: currentCp.id,
                    points: currentCp.points,
                  })
                }
              }
              this.render()

              return
            }
            case 'applyMapFileState': {
              const [newState] = args
              interactionStore.updateTimestamp()
              return
            }
            case 'addControlPoint':
            case 'addTweenControlPoint': {
              interactionStore.updateTimestamp()
              if (!res) return
              const { percent } = res
              // 如果返回的结果中带有 percent，则需要将 percent 传入给道路中控制 section 的更新
              if (percent !== undefined) {
                targetPercent = percent
              }
              ;({ cpId = '' } = args[0])
              if (cpId) {
                currentCp = store.getControlPointById(cpId)
              } else {
                currentCp = store.controlPoints[store.length - 1]
                cpId = currentCp.id
              }

              break
            }
            case 'removeControlPoint': {
              interactionStore.updateTimestamp()
              if (!res) return

              const { pointNumber } = res
              if (pointNumber === 0 || pointNumber === 1) {
                const { cpId, forwardRoadId } = res
                // 销毁辅助参考线和控制点
                disposeOneRoadControlPointAndCurve({
                  parent,
                  cpId,
                })

                // 如果剩余的控制点数，不能形成一条完成的道路，则需要删除道路
                // 如果只剩一个参考点，则渲染独立的参考点
                if (pointNumber === 1) {
                  const cp = editRoadStore.getControlPointById(cpId)
                  if (cp) {
                    renderControlPoint({
                      parent,
                      cpId,
                      points: cp.points,
                      preDispose: true,
                    })
                  }
                }

                // 需要对之前道路连通的junction结构做更新
                const _roads: Array<biz.IRoad> = []
                const _roadId: Array<string> = []
                const _junctions: Array<biz.IJunction> = []
                const _junctionId: Array<string> = []
                const _objectId: Array<string> = []
                const _objects: Array<biz.IObject> = []

                const removeForwardRoadRes = await roadStore.removeRoad(
                  forwardRoadId,
                )
                if (removeForwardRoadRes) {
                  const { road: removeForwardRoad, removeJunctionRes } =
                    removeForwardRoadRes
                  _roadId.push(forwardRoadId)
                  _roads.push(removeForwardRoad)
                  if (removeJunctionRes) {
                    // 跟道路连通的路口中物体
                    for (const removeRes of removeJunctionRes) {
                      const {
                        junction: removedJunction,
                        objectId: objectIdInJunction = [],
                        objects: objectsInJunction = [],
                      } = removeRes
                      _junctionId.push(removedJunction.id)
                      _junctions.push(removedJunction)
                      // 如果删除道路，连带删除了路口，判断删除的路口中是否带有连带删除的路口物体元素
                      // 如果有，则需要添加到缓存中一并删除
                      _objectId.push(...objectIdInJunction)
                      _objects.push(...objectsInJunction)
                    }
                  }
                }
                // 跟道路关联的物体
                const { objectId: objectIdInRoad, objects: objectsInRoad } =
                  removeObjectInRoad(_roadId)
                _objectId.push(...objectIdInRoad)
                _objects.push(...objectsInRoad)

                // 针对路口导向线（虽然在路口中跟路口关联，但也跟道路关联），如果删除了对应的 frid 或 trid，也需要联动删除
                handleIntersectionGuideLineByRoadId({
                  roadIds: [forwardRoadId],
                  objectIds: _objectId,
                  objects: _objects,
                  handler: (objectId: string) => {
                    // 调用逻辑层的删除，触发对应渲染层的元素销毁
                    const objectStore = useObjectStore()
                    objectStore.removeRoadSign(objectId, false)
                  },
                })

                // 对 junctionId 做去重
                const _uniqueJunctionId = Array.from(new Set(_junctionId))
                const _uniqueObjectId = Array.from(new Set(_objectId))

                historyStore.save({
                  title: i18n.global.t('actions.controlPoint.remove'),
                  roadId: _roadId,
                  junctionId: _uniqueJunctionId,
                  roads: _roads,
                  junctions: _junctions,
                  objectId: _uniqueObjectId,
                  objects: _objects,
                })

                // 只剩下 0 或 1 个控制点，说明道路也要随之删除，不走后续的更新道路结构的方法
                this.render()
                return
              }
              ;[, cpId = ''] = args
              const { percent } = res
              // 如果返回的结果中带有 percent，则需要将 percent 传入给道路中控制 section 的更新
              if (percent !== undefined) {
                targetPercent = percent
              }
              currentCp = store.getControlPointById(cpId)
              break
            }
            case 'moveControlPoint': {
              interactionStore.updateTimestamp()
              ;[, cpId = ''] = args
              const percent = res ? res.percent : undefined
              // 如果返回的结果中带有 percent，则需要将 percent 传入给道路中控制 section 的更新
              if (percent !== undefined) {
                targetPercent = percent
              }
              currentCp = store.getControlPointById(cpId)
              break
            }
            case 'moveMoreControlPoints': {
              interactionStore.updateTimestamp()
              cpId = args[0].cpId
              currentCp = store.getControlPointById(cpId)
              break
            }
            case 'moveAllControlPoint': {
              interactionStore.updateTimestamp()
              if (!res) return
              currentCp = res
              break
            }
            case 'removeAllControlPoint': {
              interactionStore.updateTimestamp()
              // 该方法只在交互模块中一个地方调用，不单独计入操作记录
              if (!res) return
              const cpId = res
              disposeOneRoadControlPointAndCurve({
                cpId,
                parent,
              })
              return
            }
            default:
              break
          }
          if (!currentCp) return

          renderControlPoint({
            parent,
            cpId: currentCp.id,
            points: currentCp.points,
            preDispose: true,
          })

          if (currentCp.points.length > 1) {
            // 如果控制点的数量大于 1， 则需要重新绘制新的道路结构
            const keyPath = new CatmullRomCurve3(
              transformVec3ByObject(currentCp.points),
            )
            keyPath.curveType = CurveType
            if (CurveType === 'catmullrom') {
              keyPath.tension = CatmullromTension
            }

            renderControlCurve({
              parent,
              cpId: currentCp.id,
              keyPath,
              preDispose: true,
            })

            // 区分正向和反向的道路，将控制点形成的关键曲线传给对应的道路状态
            const [forwardRoadId] = currentCp.roadId
            // 在更新路径之前，先获取原有的路径长度
            const originRoad = getRoad(forwardRoadId)
            let originRoadLength = 0
            if (originRoad) {
              originRoadLength = originRoad.length
            }

            // 正向车道
            await roadStore.updateRoadKeyPath({
              roadId: forwardRoadId,
              keyPath,
              targetPercent,
            })

            // 如果是隧道，需要基于更新的路径长度，调整原有的隧道属性
            // 基于正向道路的隧道属性来计算对象道路的
            const tunnelRoad = getRoad(forwardRoadId) as biz.ITunnel
            if (
              tunnelRoad &&
              tunnelRoad.roadType === 'tunnel' &&
              originRoadLength !== 0
            ) {
              const { tunnelS, tunnelLength } = tunnelRoad
              const newRoadLength = keyPath.getLength()
              let _tunnelS = tunnelS
              let _tunnelLength = tunnelLength
              if (tunnelS + tunnelLength >= originRoadLength) {
                // 如果 s 坐标超过道路总长度的数值，则需要将 s 坐标缩小，此时隧道长度为 0
                if (_tunnelS > newRoadLength) {
                  _tunnelS = newRoadLength
                }
                // 如果相等或者超过道路总长度，则视为一直填充满。即 s 坐标不变，修改长度
                _tunnelLength = newRoadLength - _tunnelS
              } else {
                // 如果总和小于道路长度不是特别多，误差 1m 内，可以视为充满
                if (
                  tunnelS + tunnelLength < originRoadLength &&
                  tunnelS + tunnelLength > originRoadLength - 1
                ) {
                  _tunnelLength = newRoadLength - _tunnelS
                } else {
                  // 否则固定隧道长度
                }
              }

              // 数据层更新隧道属性
              roadStore.updateTunnelAttr({
                roadId: forwardRoadId,
                s: _tunnelS,
                length: _tunnelLength,
                needRedraw: true,
              })
            }
          }

          // 获取当前参考线控制点集对应的 road 连通的 junctionId，需要联动更新
          const _roads = []
          const _junctions = []
          const diffRoadId = []
          const diffJunctionId: Array<string> = []

          const [forwardRoadId] = currentCp.roadId
          // 将可能存在更新的道路和路口进行汇总，缓存到操作记录中
          // 正向道路
          const forwardRoad = roadStore.getRoadById(forwardRoadId)
          if (forwardRoad) {
            // 如果正向道路是环形道路，则不渲染对应道路参考线的控制点和控制线
            if (forwardRoad.circleOption) {
              disposeAllControlPointsAndCurve(parent)
            }

            diffRoadId.push(forwardRoadId)
            forwardRoad && _roads.push(forwardRoad)
            for (const _junctionId of forwardRoad.linkJunction) {
              // 判断是否有重复的 junction
              if (!diffJunctionId.includes(_junctionId)) {
                diffJunctionId.push(_junctionId)
              }
            }
          }

          const { objectId, objects } = updateObjectInRoad(diffRoadId)

          // 手动更新关联的路口的几何体
          const updateJunctionPromise: Array<Promise<string>> = []
          const junctionStore = useJunctionStore()
          for (const junctionId of diffJunctionId) {
            const _junction = getJunction(junctionId)
            if (_junction) {
              _junctions.push(_junction)
              updateJunctionPromise.push(
                (() => {
                  return new Promise((resolve) => {
                    if (_junction.linkRoads.length < 2) {
                      // 如果路口的连通道路数少于2条，则删除路口和路口绑定的物体
                      const removeJunctionRes = junctionStore.removeJunction(
                        _junction.id,
                        true,
                      )
                      if (removeJunctionRes) {
                        // 将删除的物体同步到要缓存到操作记录中的数组里
                        objectId.push(...removeJunctionRes.objectId)
                        objects.push(...removeJunctionRes.objects)
                      }
                      resolve(_junction.id)
                    } else {
                      junctionStore
                        .updateJunction({
                          junction: _junction,
                          isPureUpdate: true,
                        })
                        .then(() => {
                          resolve(_junction.id)
                        })
                    }
                  })
                })(),
              )
            }
          }
          // 等待所有的路口数据更新完毕，再触发操作记录保存
          await Promise.all(updateJunctionPromise)

          // 针对道路移动的行为，需要更新路口导向线的渲染效果
          if (
            name === 'addControlPoint' ||
            name === 'moveControlPoint' ||
            name === 'moveMoreControlPoints' ||
            name === 'moveAllControlPoint'
          ) {
            // 针对路口导向线（虽然在路口中跟路口关联，但也跟道路关联），如果删除了对应的 frid 或 trid，也需要联动删除
            handleIntersectionGuideLineByRoadId({
              roadIds: currentCp.roadId,
              objectIds: objectId,
              objects,
              handler: (objectId: string) => {
                // 更新路口导向线的渲染效果
                const objectStore = useObjectStore()
                objectStore.updateRoadSignByJunction(objectId)
              },
            })
          }

          let title = ''

          switch (name) {
            case 'addControlPoint':
            case 'addTweenControlPoint': {
              title = i18n.global.t('actions.controlPoint.add')
              break
            }
            case 'moveControlPoint':
            case 'moveMoreControlPoints':
              title = i18n.global.t('actions.controlPoint.move')
              break
            case 'moveAllControlPoint':
              title = i18n.global.t('actions.road.move')
              break
            case 'removeControlPoint':
              title = i18n.global.t('actions.controlPoint.remove')
              break
            default:
              break
          }
          if (title) {
            // 保存操作记录
            historyStore.save({
              title,
              roadId: diffRoadId,
              junctionId: diffJunctionId,
              roads: _roads,
              junctions: _junctions,
              objectId,
              objects,
            })
          }

          if (name === 'moveAllControlPoint') {
            // 如果在移动整条道路时，有道路的选中效果
            if (interactionStore.roadId) {
              const currentRoad = getRoad(interactionStore.roadId)
              if (currentRoad) {
                // 先销毁旧的高亮道路蒙层
                disposeHighlightRoadMask(parent)
                // 再渲染新的高亮道路蒙层
                createHighlightRoadMask({
                  road: currentRoad,
                  parent,
                })
              }
            }
          }

          this.render()
        })
        onError((err) => {
          console.log(err)
        })
      },
    )

    this.unsubscribeRoadStore = roadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          if (name === 'updateTunnelAttr') {
            interactionStore.updateTimestamp()
          }
        })
      },
    )
  }

  onActivate () {}
  onDeactivate () {
    // 在插件取消激活时，重置插件内所有的交互状态
    const interactionStore = useRoadInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.removeDragListener()
    this.editHelper.dispose()
    this.unsubscribeEditRoadStore && this.unsubscribeEditRoadStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
    this.unsubscribeConfigStore && this.unsubscribeConfigStore()
    this.unsubscribeRoadStore && this.unsubscribeRoadStore()
  }
}

export default new EditRoadPlugin()
