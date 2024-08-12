import { Color, type Intersection, MOUSE, Mesh } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditCrgHelper from './main3d'
import {
  resetAllRoadMaskColor,
  roadMaskColor,
  roadMaskOpacity,
  updateRoadMaskColor,
} from './main3d/draw'
import { useCrgInteractionStore } from './store/interaction'
import {
  getSelected,
  registryHoverElement,
  setSelected,
} from '@/main3d/controls/dragControls'
import root3d from '@/main3d'
import { useRoadStore } from '@/stores/road'
import { useHistoryStore } from '@/stores/history'
import { getRoad } from '@/utils/mapCache'
import i18n from '@/locales'

class EditCrgPlugin extends TemplatePlugin {
  editHelper: EditCrgHelper
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
              case 'roadMask':
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
          const interactionStore = useCrgInteractionStore()
          switch (selected.object.name) {
            case 'roadMask': {
              // 选中道路，由于蒙层的层级比车道网格的层级高，会优先选中对应的蒙层

              // @ts-expect-error
              const { roadId } = selected.object
              if (!roadId) return

              const { roadId: currentRoadId } = interactionStore
              if (currentRoadId !== roadId) {
                if (currentRoadId) {
                  // 如果之前存在选中的道路，则先取消选中
                  interactionStore.unselectRoad()
                }

                // 如果存在差异，则选中当前的道路
                interactionStore.selectRoad(roadId)
              }

              break
            }
            case 'ground': {
              // 鼠标左键点击 ground，取消当前 road 的选中状态
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
        if (elementName === 'roadMask') {
          // 展示道路蒙层的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (!new Color(roadMaskColor.selected).equals(material.color)) {
              material.color.set(roadMaskColor.hovered)
              material.opacity = roadMaskOpacity
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
        if (elementName === 'roadMask') {
          // 取消道路蒙层的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (!new Color(roadMaskColor.selected).equals(material.color)) {
              material.color.set(roadMaskColor.normal)
              material.opacity = 0
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

    this.editHelper = new EditCrgHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()

    this.addDragListener()

    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['roadMask'],
    })
  }

  initDispatchers () {
    const interactionStore = useCrgInteractionStore()
    const roadStore = useRoadStore()
    const historyStore = useHistoryStore()
    interactionStore.$reset()

    const parent = this.editHelper.container
    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          switch (name) {
            case 'applyState': {
              // 更新时间戳，保证撤销、重做能够稳定有效 diff
              store.updateTimestamp()

              // 先重置所有道路蒙层的颜色
              resetAllRoadMaskColor(parent)

              if (store.roadId) {
                updateRoadMaskColor({
                  status: 'selected',
                  roadId: store.roadId,
                  parent,
                })
              }

              this.render()
              break
            }
            case 'selectRoad': {
              // 更新时间戳
              store.updateTimestamp()

              // 更新选中道路蒙层的颜色
              updateRoadMaskColor({
                status: 'selected',
                roadId: store.roadId,
                parent,
              })

              this.render()
              break
            }
            case 'unselectRoad': {
              store.updateTimestamp()

              // 重置所有道路蒙层的颜色
              resetAllRoadMaskColor(parent)

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
    this.unsubscribeRoadStore = roadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          switch (name) {
            case 'addCrg': {
              const roadId = res as string
              const road = getRoad(roadId)
              if (!road) return

              interactionStore.updateTimestamp()
              historyStore.save({
                title: i18n.global.t('actions.crg.addCrg'),
                roadId,
                roads: [road],
              })
              break
            }
            case 'updateCrg': {
              const roadId = res as string
              const road = getRoad(roadId)
              if (!road) return

              interactionStore.updateTimestamp()
              historyStore.save({
                title: i18n.global.t('actions.crg.updateCrg'),
                roadId,
                roads: [road],
              })
              break
            }
            case 'removeCrg': {
              const roadId = res as string
              const road = getRoad(roadId)
              if (!road) return

              interactionStore.updateTimestamp()
              historyStore.save({
                title: i18n.global.t('actions.crg.removeCrg'),
                roadId,
                roads: [road],
              })
              break
            }
          }
        })
      },
    )
  }

  onActivate () {}
  onDeactivate () {
    // 在插件取消激活时，重置插件内所有的交互状态
    const interactionStore = useCrgInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.editHelper.dispose()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
    this.unsubscribeRoadStore && this.unsubscribeRoadStore()
  }
}

export default new EditCrgPlugin()
