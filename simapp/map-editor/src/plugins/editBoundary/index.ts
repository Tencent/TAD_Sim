import { Color, type Intersection, Line, MOUSE } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditBoundaryHelper from './main3d'
import {
  BoundaryHelperLineColor,
  disposeBoundaryHelperLine,
  renderOneSectionBoundaryHelperLine,
  updateBoundaryHelperLineColor,
} from './main3d/draw'
import { useBoundaryInteractionStore } from './store/interaction'
import {
  getSelected,
  registryHoverElement,
  setSelected,
} from '@/main3d/controls/dragControls'
import root3d from '@/main3d'
import { useRoadStore } from '@/stores/road'

class EditBoundaryPlugin extends TemplatePlugin {
  editHelper: EditBoundaryHelper
  unsubscribeInteractionStore: Function
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
              case 'laneBoundaryHelperLine':
                setSelected(intersection)
                break

              default:
                break
            }
          })
        }
      },
      drag: (options: common.IDragOptions) => {
        if (!this.enabled) return undefined
      },
      // up 事件有可能存在误触连续调用，通过节流来避免
      dragEnd: (options: common.IDragOptions) => {
        if (!this.enabled) return undefined
      },
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { pointerdownButton } = options.object
        if (pointerdownButton === MOUSE.LEFT) {
          const selected = getSelected()
          if (!selected) return
          const interactionStore = useBoundaryInteractionStore()
          switch (selected.object.name) {
            case 'laneBoundaryHelperLine': {
              // @ts-expect-error
              const { roadId, sectionId, boundaryId } = selected.object
              if (
                !roadId ||
                !sectionId ||
                !boundaryId ||
                !interactionStore.roadId ||
                !interactionStore.sectionId
              ) {
                return
              }
              if (boundaryId !== interactionStore.boundaryId) {
                // 如果当前选中的车道跟上一次选中的不一样，则取消上一次的选中
                interactionStore.unselectBoundary()
              }
              interactionStore.selectBoundary(boundaryId)

              break
            }
            case 'lane': {
              // 鼠标左键点击车道网格，选中当前车道所在的 section
              // @ts-expect-error
              const { roadId, sectionId } = selected.object
              if (!roadId || !sectionId) return

              const { roadId: currentRoadId, sectionId: currentSectionId } =
                interactionStore
              if (currentRoadId && currentSectionId) {
                if (
                  currentRoadId === roadId &&
                  currentSectionId === sectionId
                ) {
                  // 选中同一个section，不响应
                  return
                } else {
                  if (interactionStore.boundaryId) {
                    // 提前取消上一次选中的边界辅助线
                    interactionStore.unselectBoundary()
                  }
                  // 不是同一个，则取消上一次选中的 section
                  interactionStore.unselectSection()
                }
              }
              interactionStore.selectSection({
                roadId,
                sectionId,
              })
              break
            }
            case 'ground': {
              // 鼠标左键点击 ground，取消当前 section 的选中状态（为了销毁辅助线元素）
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

    this.editHelper = new EditBoundaryHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()

    this.addDragListener()

    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['laneBoundaryHelperLine'],
    })
  }

  initDispatchers () {
    const interactionStore = useBoundaryInteractionStore()
    const roadStore = useRoadStore()
    interactionStore.$reset()

    const parent = this.editHelper.container
    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          switch (name) {
            case 'applyState': {
              // 销毁上一个状态的所有辅助边界线
              disposeBoundaryHelperLine({
                parent,
              })

              // 更新时间戳，保证撤销、重做能够稳定有效 diff
              store.updateTimestamp()

              const { roadId, sectionId, boundaryId } = store
              if (sectionId) {
                // 重新获取 section 的数据
                const querySectionRes = roadStore.getSectionById(
                  roadId,
                  sectionId,
                )
                if (querySectionRes) {
                  const { section } = querySectionRes
                  if (roadId && section) {
                    renderOneSectionBoundaryHelperLine({
                      parent,
                      roadId,
                      section,
                    })

                    if (boundaryId) {
                      // 高亮展示当前选中的边界辅助线
                      updateBoundaryHelperLineColor({
                        parent,
                        roadId,
                        sectionId: section.id,
                        boundaryId,
                        status: 'selected',
                      })
                    }
                  }
                }
              }
              this.render()
              break
            }
            case 'selectBoundary': {
              if (!res) return
              const { roadId, sectionId, boundaryId } = store
              if (!roadId || !sectionId || !boundaryId) return
              // 高亮展示当前选中的边界辅助线
              updateBoundaryHelperLineColor({
                parent,
                roadId,
                sectionId,
                boundaryId,
                status: 'selected',
              })
              this.render()
              break
            }
            case 'unselectBoundary': {
              if (!res) return
              const { roadId, sectionId } = interactionStore
              if (roadId && sectionId) {
                updateBoundaryHelperLineColor({
                  parent,
                  roadId,
                  sectionId,
                  boundaryId: res,
                  status: 'normal',
                })
                this.render()
              }
              break
            }
            case 'selectSection': {
              const { roadId, sectionId } = store
              // 渲染当前选中 section 中所有车道的右侧边界辅助曲线
              if (roadId && sectionId) {
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
              }
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
    const interactionStore = useBoundaryInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.editHelper.dispose()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
  }
}

export default new EditBoundaryPlugin()
