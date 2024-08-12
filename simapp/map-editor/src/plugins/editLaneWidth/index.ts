import {
  CatmullRomCurve3,
  Color,
  type Intersection,
  Line,
  MOUSE,
  Vector3,
} from 'three'
import { throttle } from 'lodash'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditLaneWidthHelper from './main3d'
import {
  BoundaryHelperLineColor,
  disposeAllVirtualElement,
  disposeBoundaryHelperLine,
  disposeLaneMask,
  renderLaneMask,
  renderOneSectionBoundaryHelperLine,
  renderVirtualBoundaryLine,
  updateBoundaryHelperLineColor,
} from './main3d/draw'
import { useLaneWidthInteractionStore } from './store/interaction'
import { useRoadStore } from '@/stores/road'
import {
  getSelected,
  registryHoverElement,
  registryIgnoreDragOffsetElements,
  setInteractionPlane,
  setSelected,
} from '@/main3d/controls/dragControls'
import root3d from '@/main3d'
import {
  CatmullromTension,
  CurveType,
  axisY,
  fixedPrecision,
  getClosestPointFromCurvePath,
  getVerticalVector,
  transformVec3ByObject,
  worldCenter,
} from '@/utils/common3d'
import { getBoundaryById } from '@/stores/road/boundary'
import { getLaneById } from '@/stores/road/lane'

class EditLaneWidthPlugin extends TemplatePlugin {
  editHelper: EditLaneWidthHelper
  unsubscribeInteractionStore: Function
  unsubscribeRoadStore: Function
  constructor () {
    super()
    this.config = config

    // 覆写模板中定义的交互逻辑
    this.dragDispatchers = {
      dragStart: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const { intersections, pointerdownButton } = options.object
        if (pointerdownButton === MOUSE.LEFT) {
          intersections.forEach((intersection: Intersection) => {
            if (getSelected()) return

            const { object } = intersection
            switch (object.name) {
              case 'ground':
              case 'lane':
                setSelected(intersection)
                break
              case 'laneBoundaryHelperLine': {
                // 如果是鼠标左键 down 了边界辅助线，则可以开始拖拽辅助线更新车道宽度
                setSelected(intersection)

                // 由于在开始拖拽的阶段，不确定是否会移动控制点，需要提前设置移动的基准平面
                // 当前设置成高度为零的水平面
                setInteractionPlane({
                  normal: axisY,
                  point: worldCenter,
                })

                // 如果是拖拽了辅助线，需要提前禁止视角控制器
                root3d.setViewControlsEnabled(false)
                break
              }
              default:
                break
            }
          })
        }
      },
      drag: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const selected = getSelected()
        if (!selected) return

        if (selected.object.name === 'laneBoundaryHelperLine') {
          // 拖拽辅助线，展示拖拽的预览效果
          this.dragPreviewBoundary(options)
        }
      },
      // up 事件有可能存在误触连续调用，通过节流来避免
      dragEnd: throttle((options: common.IDragOptions) => {
        if (!this.enabled) return
        const selected = getSelected()
        if (!selected) return

        // 不管选中什么物体，在拖拽结束后，恢复视角控制器
        root3d.setViewControlsEnabled(true)

        const { object } = selected
        if (object.name === 'laneBoundaryHelperLine') {
          // 销毁所有辅助拖拽的预览元素
          disposeAllVirtualElement(this.editHelper.container)

          const { newPosition } = options.object
          // 基于新位置跟车道边界辅助线交互点的偏移量，计算目标车道右侧的虚拟边界预览采样点
          const interactionStore = useLaneWidthInteractionStore()
          const roadStore = useRoadStore()
          // @ts-expect-error
          const { roadId, sectionId, laneId, boundaryId } = selected.object
          const querySectionRes = roadStore.getSectionById(roadId, sectionId)
          if (!querySectionRes) return
          // 获取目标 section 的对应 boundary 和 lane
          const targetBoundary = getBoundaryById(
            querySectionRes.section.boundarys,
            boundaryId,
          )
          const targetLane = getLaneById(querySectionRes.section.lanes, laneId)
          if (!targetBoundary || !targetLane) return
          const { samplePoints } = targetBoundary

          const isForward = Number(laneId) < 0

          // 计算车道宽度的偏移量
          const widthOffset = this.calcVerticalOffset({
            points: samplePoints,
            pA: newPosition,
            pB: selected.point,
            isForward,
          })

          // 如果预览的边界线跨越到目标车道左侧边界的左侧，则返回
          if (
            widthOffset < 0 &&
            Math.abs(widthOffset) > targetLane.normalWidth
          ) {
            this.render()
            return
          }

          // 在触发数据层面更新之前，选中新车道结构下的车道
          interactionStore.selectLane({
            roadId,
            sectionId,
            laneId,
          })

          // 优先更新当前交互模块的时间戳
          interactionStore.updateTimestamp()

          const newLaneWidth = fixedPrecision(
            targetLane.normalWidth + widthOffset,
          )
          // 从数据层面更新车道宽度
          roadStore.updateLaneWidth({
            roadId,
            sectionId,
            laneId,
            width: newLaneWidth,
          })
        }
      }, 100),
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { pointerdownButton } = options.object
        const selected = getSelected()
        if (!selected) return

        // 不管选中什么物体，在点击后恢复视角控制器
        root3d.setViewControlsEnabled(true)

        if (pointerdownButton === MOUSE.LEFT) {
          const interactionStore = useLaneWidthInteractionStore()
          switch (selected.object.name) {
            case 'lane': {
              // 鼠标左键点击车道网格，选中当前车道所在的 section
              // @ts-expect-error
              const { roadId, sectionId, laneId } = selected.object
              if (!roadId || !sectionId || !laneId) return
              const { roadId: currentRoadId, sectionId: currentSectionId } =
                interactionStore
              if (
                !currentRoadId ||
                !currentSectionId ||
                roadId !== currentRoadId ||
                sectionId !== currentSectionId
              ) {
                if (interactionStore.laneId) {
                  // 如果存在选中的车道，则先取消车道的选中
                  interactionStore.unselectLane()
                }
                // 如果当前不存在已选中的道路和 section
                // 或者之前选中的道路和 section 跟当前选中的不是同一个
                // 则直接选中对应的 section
                interactionStore.selectSection({
                  roadId,
                  sectionId,
                })
              } else if (
                roadId === currentRoadId &&
                sectionId === currentSectionId
              ) {
                // 如果之前选中的道路和 section 跟当前选中的一致
                // 则选中对应的车道 lane
                interactionStore.selectLane({
                  roadId,
                  sectionId,
                  laneId,
                })
              }
              break
            }
            case 'ground': {
              // 鼠标左键点击 ground，取消当前的选中状态
              interactionStore.unselectAll()
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
        if (elementName === 'laneBoundaryHelperLine') {
          // 展示车道边界辅助线的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Line) {
            const { material } = targetObject
            if (
              !new Color(BoundaryHelperLineColor.selected).equals(
                material.color,
              )
            ) {
              material.color.set(BoundaryHelperLineColor.hovered)
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
        if (elementName === 'laneBoundaryHelperLine') {
          // 取消车道边界辅助线的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Line) {
            const { material } = targetObject
            if (
              !new Color(BoundaryHelperLineColor.selected).equals(
                material.color,
              )
            ) {
              material.color.set(BoundaryHelperLineColor.normal)
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

    this.editHelper = new EditLaneWidthHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()

    this.addDragListener()

    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['laneBoundaryHelperLine'], // 响应车道边界线的辅助曲线
    })

    // 忽略拖拽车道边界辅助线过程中的 offset 偏移计算
    registryIgnoreDragOffsetElements('laneBoundaryHelperLine')
  }

  /**
   * 拖拽车道边界辅助线，生成预览的虚拟边界位置
   * @param options
   * @returns
   */
  dragPreviewBoundary (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { roadId, sectionId } = selected.object
    // 基于新位置跟车道边界辅助线交互点的偏移量，计算目标车道右侧的虚拟边界预览采样点
    const roadStore = useRoadStore()
    const querySectionRes = roadStore.getSectionById(roadId, sectionId)
    if (!querySectionRes) return
    const { section } = querySectionRes
    // @ts-expect-error
    const { laneId, boundaryId } = selected.object
    // 获取目标 section 的对应 boundary 和 lane
    const targetBoundary = getBoundaryById(section.boundarys, boundaryId)
    const targetLane = getLaneById(section.lanes, laneId)
    if (!targetBoundary || !targetLane) return
    const { samplePoints } = targetBoundary

    const isForward = Number(laneId) < 0

    // 计算车道宽度的偏移量
    const widthOffset = this.calcVerticalOffset({
      points: samplePoints,
      pA: newPosition,
      pB: selected.point,
      isForward,
    })

    // 如果预览的边界线跨越到目标车道左侧边界的左侧，则返回
    if (widthOffset < 0 && Math.abs(widthOffset) > targetLane.normalWidth) {
      return
    }

    // 在创建新的预览车道边界辅助线之前，先销毁历史存在的虚拟预览元素
    disposeAllVirtualElement(this.editHelper.container)

    // 高亮拖拽的那一条车道边界辅助线
    updateBoundaryHelperLineColor({
      parent: this.editHelper.container,
      roadId,
      sectionId,
      boundaryId,
      status: 'hovered',
    })

    // 将车道宽度偏移量应用到虚拟的车道边界辅助线的渲染中
    renderVirtualBoundaryLine({
      parent: this.editHelper.container,
      lanes: section.lanes,
      boundarys: section.boundarys,
      targetLaneId: laneId,
      laneWidthOffset: widthOffset,
    })

    this.render()
  }

  /**
   * 计算某点 A 到曲线上的某点 B 的切线垂直分量（切线右侧）偏移量
   * @param params
   */
  calcVerticalOffset (params: {
    points: Array<common.vec3>
    pA: common.vec3
    pB: common.vec3
    isForward?: boolean // 当前车道的朝向
  }) {
    const { points, pA, pB, isForward = true } = params
    const keyPath = new CatmullRomCurve3(transformVec3ByObject(points))
    keyPath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      keyPath.tension = CatmullromTension
    }

    // 获取 pB 在曲线上的索引以及对应的切线方向
    const { point: closestPoint, tangent } = getClosestPointFromCurvePath({
      curvePath: keyPath,
      point: pB,
    })
    // 计算当前切线方向偏右的垂直分量
    const vertical = getVerticalVector(tangent)
    if (!isForward) {
      // 如果是反向车道，则将垂直向量也反向
      vertical.negate()
    }

    // 计算水平面上的相对偏移量
    const originVec3 = new Vector3(closestPoint.x, 0, closestPoint.z)
    const newVec3 = new Vector3(pA.x, 0, pA.z)
    const offsetVec3 = newVec3.sub(originVec3)

    // 获取 pApB 相对偏移量，在 pB 点垂直方向上的分量对应的距离
    let offset = offsetVec3.clone().projectOnVector(vertical).length()
    if (offsetVec3.dot(vertical) < 0) {
      // 如果偏移方向跟切线垂直分量是反向，则偏移量取反
      offset = -offset
    }
    fixedPrecision(offset)

    return offset
  }

  initDispatchers () {
    const roadStore = useRoadStore()
    const interactionStore = useLaneWidthInteractionStore()
    interactionStore.$reset()

    const parent = this.editHelper.container
    // 监听 roadStore 中更新车道方法的调用，做后处理
    this.unsubscribeRoadStore = roadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        if (!this.enabled) return
        after((res) => {
          switch (name) {
            case 'applyState': {
              interactionStore.updateTimestamp()
              break
            }
            case 'updateLaneWidth': {
              const { roadId, sectionId, laneId } = interactionStore
              if (!roadId || !sectionId || !laneId) return
              // 手动更新时间戳
              interactionStore.updateTimestamp()

              const querySectionRes = roadStore.getSectionById(
                roadId,
                sectionId,
              )
              // 销毁原先的车道边界辅助线
              disposeBoundaryHelperLine({
                parent: this.editHelper.container,
                roadId,
                sectionId,
              })
              // 销毁原先的车道蒙层
              disposeLaneMask({
                parent: this.editHelper.container,
              })

              if (querySectionRes) {
                // 基于新的车道结构，渲染新的车道边界辅助线
                renderOneSectionBoundaryHelperLine({
                  parent: this.editHelper.container,
                  roadId,
                  section: querySectionRes.section,
                })
                // 选中新车道结构的车道
                renderLaneMask({
                  parent: this.editHelper.container,
                  roadId,
                  section: querySectionRes.section,
                  laneId,
                })
              }
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

    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          switch (name) {
            case 'applyState': {
              // 销毁上一个状态所有的车道边界辅助线
              disposeBoundaryHelperLine({
                parent,
              })

              // 销毁上一个状态所有的车道透明蒙层
              disposeLaneMask({
                parent,
              })

              // 更新时间戳，保证撤销、重做能够稳定有效 diff
              store.updateTimestamp()

              const { roadId, sectionId, laneId } = store
              if (sectionId) {
                // 重新获取 section 的数据
                const querySectionRes = roadStore.getSectionById(
                  roadId,
                  sectionId,
                )
                if (querySectionRes) {
                  const { section } = querySectionRes
                  if (roadId && section) {
                    // 应用完操作记录数据后的，展示对应状态下的车道边界辅助线
                    renderOneSectionBoundaryHelperLine({
                      parent,
                      roadId,
                      section,
                    })
                    if (laneId) {
                      // 如果有选中的车道，则渲染选中车道的透明蒙层
                      renderLaneMask({
                        parent,
                        roadId,
                        section,
                        laneId,
                      })
                    }
                  }
                }
              }
              this.render()
              break
            }
            case 'selectLane': {
              if (!res) return
              const { roadId, sectionId, laneId } = store
              if (!roadId || !sectionId || !laneId) return
              const {
                roadId: lastRoadId,
                sectionId: lastSectionId,
                laneId: lastLaneId,
              } = res
              if (lastRoadId && lastSectionId && lastLaneId) {
                disposeLaneMask({
                  parent,
                  roadId: lastRoadId,
                  sectionId: lastSectionId,
                  laneId: lastLaneId,
                })
              }
              const querySectionRes = roadStore.getSectionById(
                roadId,
                sectionId,
              )
              if (querySectionRes) {
                renderLaneMask({
                  parent,
                  roadId,
                  section: querySectionRes.section,
                  laneId,
                })
              }
              this.render()
              break
            }
            case 'unselectLane': {
              if (!res) return
              const { roadId, sectionId, laneId } = res
              if (!roadId || !sectionId || !laneId) return
              // 销毁车道蒙层
              disposeLaneMask({
                parent,
                roadId,
                sectionId,
                laneId,
              })
              // 销毁选中section生成的车道边界辅助线
              disposeBoundaryHelperLine({
                parent,
                roadId,
                sectionId,
              })
              this.render()
              break
            }
            case 'selectSection': {
              const { roadId, sectionId } = store
              // TODO 渲染当前选中 section 中所有车道的右侧边界辅助曲线
              if (!roadId || !sectionId) return
              const querySectionRes = roadStore.getSectionById(
                roadId,
                sectionId,
              )
              if (querySectionRes) {
                renderOneSectionBoundaryHelperLine({
                  parent,
                  roadId,
                  section: querySectionRes.section,
                })
              }

              if (res) {
                const { roadId: lastRoadId, sectionId: lastSectionId } = res
                if (lastRoadId && lastSectionId) {
                  // 如果在选中新的 section 时，之前有另一个 section 是被选中的状态，则销毁其车道边界辅助线
                  disposeBoundaryHelperLine({
                    parent,
                    roadId: lastRoadId,
                    sectionId: lastSectionId,
                  })
                }
              }
              this.render()

              break
            }
            case 'unselectSection': {
              if (!res) return
              const { roadId, sectionId } = res
              if (!roadId || !sectionId) return

              // 销毁取消选中前的上一个选中section的边界辅助线
              disposeBoundaryHelperLine({
                parent,
                roadId,
                sectionId,
              })
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
    const interactionStore = useLaneWidthInteractionStore()
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
