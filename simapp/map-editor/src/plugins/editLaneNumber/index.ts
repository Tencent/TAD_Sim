import { Color, type Intersection, Line, MOUSE } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditLaneNumberHelper from './main3d'
import { useLaneNumberInteractionStore } from './store/interaction'
import {
  BoundaryHelperLineColor,
  disposeBoundaryHelperLine,
  disposeLaneMask,
  renderLaneMask,
  renderOneSectionBoundaryHelperLine,
} from './main3d/draw'
import root3d from '@/main3d'
import { useRoadStore } from '@/stores/road'
import {
  getSelected,
  registryHoverElement,
  setSelected,
} from '@/main3d/controls/dragControls'

class EditLaneNumberPlugin extends TemplatePlugin {
  editHelper: EditLaneNumberHelper
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
                setSelected(intersection)

                break
              }
              default:
                break
            }
          })
        }
      },
      drag: (options: common.IDragOptions) => {
        if (!this.enabled) return undefined
      },
      dragEnd: (options: common.IDragOptions) => {
        if (!this.enabled) return undefined
      },
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { pointerdownButton } = options.object
        if (pointerdownButton === MOUSE.LEFT) {
          const selected = getSelected()
          if (!selected) return
          const interactionStore = useLaneNumberInteractionStore()
          switch (selected.object.name) {
            case 'laneBoundaryHelperLine': {
              // @ts-expect-error
              const { roadId, sectionId, laneId } = selected.object
              if (!roadId || !sectionId || !laneId) return

              const roadStore = useRoadStore()
              roadStore.addLane({
                roadId,
                sectionId,
                laneId,
              })
              return
            }
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

    this.editHelper = new EditLaneNumberHelper({
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
  }

  initDispatchers () {
    const roadStore = useRoadStore()
    const interactionStore = useLaneNumberInteractionStore()
    interactionStore.$reset()

    const parent = this.editHelper.container
    // 监听 roadStore 中更新车道方法的调用，做后处理
    this.unsubscribeRoadStore = roadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        if (!this.enabled) return
        after((res) => {
          switch (name) {
            case 'applyState': {
              // 更新时间戳，保证撤销、重做能够稳定有效 diff
              interactionStore.updateTimestamp()
              break
            }
            case 'addLane': {
              const { roadId, sectionId, laneId } = interactionStore
              // 更新时间戳，保证撤销、重做能够稳定有效 diff
              interactionStore.updateTimestamp()
              // 销毁原先的车道边界辅助线
              disposeBoundaryHelperLine({
                parent,
              })
              // 销毁原先的车道蒙层
              disposeLaneMask({
                parent,
              })

              if (roadId && sectionId) {
                const querySectionRes = roadStore.getSectionById(
                  roadId,
                  sectionId,
                )
                if (querySectionRes) {
                  // 基于新的车道结构，渲染新的车道边界辅助线
                  renderOneSectionBoundaryHelperLine({
                    parent,
                    roadId,
                    section: querySectionRes.section,
                  })

                  if (laneId) {
                    // 选中新车道结构的车道
                    renderLaneMask({
                      parent,
                      roadId,
                      section: querySectionRes.section,
                      laneId,
                    })
                  }
                }
              }
              this.render()
              break
            }
            case 'removeLane': {
              // 更新时间戳，保证撤销、重做能够稳定有效 diff
              interactionStore.updateTimestamp()
              interactionStore.unselectLane()
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

              const { roadId, sectionId } = store
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
              if (!res) return
              const { roadId, sectionId } = store
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

              const { roadId: lastRoadId, sectionId: lastSectionId } = res
              if (lastRoadId && lastSectionId) {
                // 如果在选中新的 section 时，之前有另一个 section 是被选中的状态，则销毁其车道边界辅助线
                disposeBoundaryHelperLine({
                  parent,
                  roadId: lastRoadId,
                  sectionId: lastSectionId,
                })
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
    const interactionStore = useLaneNumberInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.editHelper.dispose()
    this.unsubscribeRoadStore && this.unsubscribeRoadStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
  }
}

export default new EditLaneNumberPlugin()
