import { type Intersection, MOUSE } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditLaneAttrHelper from './main3d'
import {
  disposeLaneHelper,
  renderLaneMask,
  renderOneSectionLaneHelper,
  updateLaneBoundaryHelperColor,
  updateLaneMaskOpacity,
} from './main3d/draw'
import { useLaneAttrInteractionStore } from './store/interaction'
import { useRoadStore } from '@/stores/road'
import {
  getSelected,
  registryHoverElement,
  setSelected,
} from '@/main3d/controls/dragControls'
import root3d from '@/main3d/index'

class EditLaneAttrPlugin extends TemplatePlugin {
  editHelper: EditLaneAttrHelper
  unsubscribeInteractionStore: Function
  unsubscribeRoadStore: Function
  constructor () {
    super()
    this.config = config

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
              case 'laneMask': // 车道的辅助蒙层
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
      dragEnd: (options: common.IDragOptions) => {
        if (!this.enabled) return undefined
      },
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const { pointerdownButton } = options.object
        if (pointerdownButton === MOUSE.LEFT) {
          const selected = getSelected()
          if (!selected) return
          switch (selected.object.name) {
            case 'laneMask': {
              // 鼠标左键点击车道蒙层，选中对应的车道
              // @ts-expect-error
              const { roadId, sectionId, laneId } = selected.object
              if (!roadId || !sectionId || !laneId) return
              const interactionStore = useLaneAttrInteractionStore()
              const {
                roadId: currentRoadId,
                sectionId: currentSectionId,
                laneId: currentLaneId,
              } = interactionStore

              // 点击车道蒙层，一定存在 roadId 和 sectionId
              if (!currentRoadId || !currentSectionId) return

              if (!currentLaneId) {
                // 如果之前没有选中的车道
                interactionStore.selectLane({
                  roadId,
                  sectionId,
                  laneId,
                })
              } else {
                if (currentLaneId !== laneId) {
                  interactionStore.unselectLane()
                  interactionStore.selectLane({
                    roadId,
                    sectionId,
                    laneId,
                  })
                }
              }

              break
            }
            case 'lane': {
              // 鼠标左键点击车道网格，选中当前 section
              // @ts-expect-error
              const { roadId, sectionId } = selected.object
              if (!roadId || !sectionId) return

              const interactionStore = useLaneAttrInteractionStore()

              // 如果是同一个车道，多次点击则直接返回
              if (
                interactionStore.roadId === roadId &&
                interactionStore.sectionId === sectionId
              ) {
                return
              }

              // 如果是其他车道，需要先取消上一次选中的车道状态
              if (interactionStore.sectionId) {
                interactionStore.unselectSection()
              }

              // 选中 section
              interactionStore.selectSection({
                roadId,
                sectionId,
              })
              break
            }
            case 'ground': {
              // 鼠标左键点击 ground，取消当前 section 的选中状态
              const interactionStore = useLaneAttrInteractionStore()
              if (interactionStore.sectionId) {
                interactionStore.unselectAll()
              }
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
        // hoverOn 到车道蒙层上，则更新对应的车道边界辅助元素的颜色
        if (elementName === 'laneMask') {
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (!targetObject) return

          // @ts-expect-error
          const { roadId, sectionId, laneId } = targetObject
          updateLaneBoundaryHelperColor({
            parent: this.editHelper.container,
            roadId,
            sectionId,
            laneId,
            status: 'hovered',
          })

          this.render()
        }
      },
      hoverOff: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const { elementName, elementUuid } = options.object
        if (!elementName || !elementUuid) return
        // hoverOn 到车道蒙层上，则更新对应的车道边界辅助元素的颜色
        if (elementName === 'laneMask') {
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (!targetObject) return

          // @ts-expect-error
          const { roadId, sectionId, laneId } = targetObject
          updateLaneBoundaryHelperColor({
            parent: this.editHelper.container,
            roadId,
            sectionId,
            laneId,
            status: 'normal',
          })

          this.render()
        }
      },
    }
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditLaneAttrHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()
    this.addDragListener()

    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['laneMask'],
    })
  }

  initDispatchers () {
    const roadStore = useRoadStore()
    const interactionStore = useLaneAttrInteractionStore()
    interactionStore.$reset()

    const parent = this.editHelper.container
    this.unsubscribeRoadStore = roadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        if (!this.enabled) return
        after((res) => {
          switch (name) {
            case 'updateLaneAttr': {
              interactionStore.updateTimestamp()
              break
            }
            case 'removeLane': {
              interactionStore.updateTimestamp()
              interactionStore.unselectAll()
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
              // TODO 补充交互模块在操作记录中的应用效果联动
              disposeLaneHelper({
                parent,
              })

              // 更新时间戳，保证撤销、重做能够稳定有效 diff
              store.updateTimestamp()

              const { roadId, sectionId, laneId } = store
              if (sectionId) {
                // 重新获取 section 的数据
                const roadStore = useRoadStore()
                const querySectionRes = roadStore.getSectionById(
                  roadId,
                  sectionId,
                )
                if (querySectionRes) {
                  const { section } = querySectionRes
                  if (roadId && section && laneId) {
                    renderLaneMask({
                      parent,
                      roadId,
                      section,
                      laneId,
                    })
                  }
                }
              }
              this.render()
              break
            }
            case 'selectSection': {
              // 渲染车道的辅助蒙层
              const { roadId, sectionId } = store
              if (roadId && sectionId) {
                const querySectionRes = roadStore.getSectionById(
                  roadId,
                  sectionId,
                )
                if (querySectionRes) {
                  renderOneSectionLaneHelper({
                    parent,
                    roadId,
                    section: querySectionRes.section,
                  })
                }

                if (res) {
                  const { roadId: lastRoadId, sectionId: lastSectionId } = res
                  if (lastRoadId && lastSectionId) {
                    // 如果在选中新的 section 时，之前有另一个 section 是被选中的状态
                    disposeLaneHelper({
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
              // 删除车道的辅助蒙层
              if (!res) return
              const { roadId, sectionId } = res
              if (!roadId || !sectionId) return

              disposeLaneHelper({
                parent,
                roadId,
                sectionId,
              })
              this.render()
              break
            }
            case 'selectLane': {
              // 更新选中车道边框的颜色
              // 更新选中车道蒙层的透明度
              const { roadId, sectionId, laneId } = store
              if (!roadId || !sectionId || !laneId) return

              updateLaneBoundaryHelperColor({
                parent,
                roadId,
                sectionId,
                laneId,
                status: 'selected',
              })

              updateLaneMaskOpacity({
                parent,
                roadId,
                sectionId,
                laneId,
                opacity: 0.1, // 展示选中车道的蒙层
              })

              this.render()
              break
            }
            case 'unselectLane': {
              // 更新选中车道边框的颜色
              // 更新选中车道蒙层的透明度
              const { roadId, sectionId, laneId } = res
              if (!roadId || !sectionId || !laneId) return

              updateLaneBoundaryHelperColor({
                parent,
                roadId,
                sectionId,
                laneId,
                status: 'normal',
                forced: true, // 强制取消选中的高亮
              })

              updateLaneMaskOpacity({
                parent,
                roadId,
                sectionId,
                laneId,
                opacity: 0, // 透明度为0，隐藏车道蒙层
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
    const interactionStore = useLaneAttrInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.editHelper.dispose()
    this.unsubscribeRoadStore && this.unsubscribeRoadStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
  }
}

export default new EditLaneAttrPlugin()
