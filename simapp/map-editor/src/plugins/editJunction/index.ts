import { Color, type Intersection, MOUSE, Mesh } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import EditJunctionHelper from './main3d'
import config from './config.json'
import {
  RoadEndEdgeColor,
  createHighlightJunctionMask,
  disposeHighlightJunctionMask,
  updateRoadEndEdgeColor,
} from './main3d/draw'
import { useJunctionInteractionStore } from './store/interaction'
import { useJunctionStore } from '@/stores/junction'
import root3d from '@/main3d'
import {
  getSelected,
  registryHoverElement,
  setSelected,
} from '@/main3d/controls/dragControls'
import { useHistoryStore } from '@/stores/history'
import i18n from '@/locales'
import { getJunction } from '@/utils/mapCache'
import { handleIntersectionGuideLineByRoadId } from '@/stores/object/customRoadSign/intersectionGuideLine'
import { useObjectStore } from '@/stores/object'

class EditJunctionPlugin extends TemplatePlugin {
  editHelper: EditJunctionHelper
  unsubscribeJunctionStore: Function
  unsubscribeInteractionStore: Function
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
          // 如果是左键点击，则选中对应元素包括：路口、道路首尾端面、地平面
          intersections.forEach((intersection: Intersection) => {
            if (getSelected()) return

            const { object } = intersection
            switch (object.name) {
              case 'roadEndEdge':
              case 'junction':
              case 'ground':
                setSelected(intersection)
                break
              default:
                break
            }
          })
        }
      },
      drag: () => {
        if (!this.enabled) return undefined
      },
      dragEnd: () => {
        if (!this.enabled) return undefined
      },
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const { pointerdownButton } = options.object
        if (pointerdownButton === MOUSE.LEFT) {
          const selected = getSelected()

          if (!selected) return
          const interactionStore = useJunctionInteractionStore()
          switch (selected.object.name) {
            case 'roadEndEdge': {
              // @ts-expect-error
              const { roadId, percent, direction } = selected.object
              if (roadId && percent) {
                const _linkRoad = `${roadId}_${percent}_${direction}`
                // 先判断当前 linkRoad 是否有已经绑定的路口，如果有，则选取第一个绑定的路口
                const junctionStore = useJunctionStore()
                const junction = junctionStore.getJunctionByLinkRoad(_linkRoad)
                if (junction) {
                  if (!interactionStore.junctionId) {
                    // 直接选中路口
                    interactionStore.clearRoadEndEdge()
                    interactionStore.selectJunction(junction.id)
                  } else {
                    // 判断存在绑定关系的路口，跟选中的路口是否一致
                    if (interactionStore.junctionId === junction.id) {
                      // 取消连接
                      interactionStore.unselectRoadEndEdge(_linkRoad)
                    } else {
                      // 取消原有选中的路口
                      interactionStore.unselectJunction()
                      // 选中当前 linkRoad 对应的路口
                      interactionStore.selectJunction(junction.id)
                    }
                  }
                } else {
                  if (interactionStore.judgeExist(_linkRoad)) {
                    // 如果当前选中的 linkRoad 已存在，则将其取消连接
                    interactionStore.unselectRoadEndEdge(_linkRoad)
                  } else {
                    // 如果当前选中的 linkRoad 不存在，则将其连接到路口中
                    interactionStore.selectRoadEndEdge(_linkRoad)
                  }
                }
              }
              break
            }
            case 'junction': {
              // @ts-expect-error
              const { junctionId } = selected.object
              if (junctionId) {
                const junctionStore = useJunctionStore()
                const junction = junctionStore.getJunctionById(junctionId)
                if (!junction) return

                // 如果已经选中了，则直接返回
                if (interactionStore.junctionId === junctionId) return

                // 取消上一次的选中状态
                interactionStore.unselectJunction()

                // 记录新的选中状态
                interactionStore.selectJunction(junctionId)
              }
              break
            }
            case 'ground':
              // 鼠标左键点击 ground，则取消当前路口和道路首尾端面的选中状态
              interactionStore.unselectJunction()
              break

            default:
              break
          }
        }
      },
      hoverOn: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { elementName, elementUuid } = options.object
        if (!elementName || !elementUuid) return
        if (elementName === 'roadEndEdge') {
          // 展示道路首尾端面的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (
              !new Color(RoadEndEdgeColor.selected).equals(material.color)
            ) {
              material.color.set(RoadEndEdgeColor.hovered)
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
        if (elementName === 'roadEndEdge') {
          // 取消道路首尾端面的 hover 效果
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (targetObject instanceof Mesh) {
            const { material } = targetObject
            if (
              !new Color(RoadEndEdgeColor.selected).equals(material.color)
            ) {
              material.color.set(RoadEndEdgeColor.normal)
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

    this.editHelper = new EditJunctionHelper({
      render: this.render,
    })

    this.editHelper.init()

    this.scene.add(this.editHelper.container)
    this.initDispatchers()

    this.addDragListener()

    // 注册当前模式下，可支持 hover 交互元素的名称
    registryHoverElement({
      pluginName: this.config.name,
      objectName: ['junction', 'roadEndEdge'], // 响应路口自身闭合区域以及道路首尾端面
    })
  }

  initDispatchers () {
    const interactionStore = useJunctionInteractionStore()
    const historyStore = useHistoryStore()
    const junctionStore = useJunctionStore()
    interactionStore.$reset()

    const parent = this.editHelper.container

    this.unsubscribeJunctionStore = junctionStore.$onAction(
      ({ name, after }) => {
        after(() => {
          switch (name) {
            case 'applyState':
            case 'connectLinkRoad':
            case 'disconnectLinkRoad': {
              // 在三维场景中更新路口对应的连通车道关系时，手动触发时间戳更新
              interactionStore.updateTimestamp()
              break
            }
          }
        })
      },
    )

    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after(async (res) => {
          switch (name) {
            case 'applyState': {
              const [currentState, diffState] = args
              const { roadEndEdges: currentRoadEndEdges } = currentState
              const { roadEndEdges: diffRoadEndEdges, junctionId } = diffState

              interactionStore.updateTimestamp()

              for (const index in diffRoadEndEdges) {
                if (!diffRoadEndEdges[index]) continue
                updateRoadEndEdgeColor({
                  parent,
                  linkRoad: diffRoadEndEdges[index],
                  status: 'normal',
                })
              }
              for (const index in currentRoadEndEdges) {
                if (!currentRoadEndEdges[index]) continue
                updateRoadEndEdgeColor({
                  parent,
                  linkRoad: currentRoadEndEdges[index],
                  status: 'selected',
                })
              }

              // 销毁旧的高亮蒙层
              disposeHighlightJunctionMask(parent)
              // 渲染新的高亮蒙层
              const currentJunction = getJunction(interactionStore.junctionId)
              if (currentJunction) {
                createHighlightJunctionMask({
                  junction: currentJunction,
                  parent,
                })
              }

              this.render()
              break
            }
            case 'selectRoadEndEdge': {
              const linkRoad = args[0]
              if (!linkRoad) return

              interactionStore.updateTimestamp()

              updateRoadEndEdgeColor({
                parent,
                linkRoad,
                status: 'selected',
              })
              const { roadEndEdges, junctionId } = store
              const junctionStore = useJunctionStore()
              if (junctionId) {
                // 如果当前有选中的路口 id（说明已连接的道路大于等于 2 条），则向路口中添加新的 linkRoad
                const connectRes = await junctionStore.connectLinkRoad({
                  junctionId,
                  linkRoad,
                })
                const _junctions = []
                const _junction = junctionStore.getJunctionById(junctionId)
                _junction && _junctions.push(_junction)
                if (connectRes) {
                  // 修改连通关系只涉及 junctionId 相关的 diff
                  historyStore.save({
                    title: i18n.global.t('actions.linkRoad.connect'),
                    junctionId,
                    junctions: _junctions,
                  })
                }

                // 销毁旧的高亮蒙层
                disposeHighlightJunctionMask(parent)
                // 渲染新的高亮蒙层
                if (_junction) {
                  createHighlightJunctionMask({
                    junction: _junction,
                    parent,
                  })
                }
              } else {
                // 如果没有选中的路口，判断已选中的 linkRoad 是否超过 2 条，如果超过 2 条则自动创建新的路口
                if (roadEndEdges.length > 1) {
                  const _junctionId = await junctionStore.connectLinkRoad({
                    junctionId: '',
                    linkRoad: [...roadEndEdges],
                  })
                  if (_junctionId) {
                    // 选中新建的路口
                    store.selectJunction(_junctionId)
                    const _junctions = []
                    const _junction = junctionStore.getJunctionById(_junctionId)
                    _junction && _junctions.push(_junction)

                    // 修改连通关系只涉及 junctionId 相关的 diff
                    historyStore.save({
                      title: i18n.global.t('actions.linkRoad.connect'),
                      junctionId: _junctionId,
                      junctions: _junctions,
                    })
                  }
                }
              }

              this.render()
              break
            }
            case 'unselectRoadEndEdge': {
              const linkRoad = args[0]
              if (!linkRoad) return

              interactionStore.updateTimestamp()

              updateRoadEndEdgeColor({
                parent,
                linkRoad,
                status: 'normal',
              })

              // 判断当前有没有选中的路口，如果有，则将当前选中的 linkRoad 取消连接，更新路口结构
              const { junctionId } = store
              if (junctionId) {
                const junctionStore = useJunctionStore()
                const junction = junctionStore.getJunctionById(junctionId)
                if (junction) {
                  if (junction.linkRoads.length > 2) {
                    // 如果路口由 3 条及以上的道路连接而成，在取消某一条道路连接后，需要更新路口结构
                    const disconnectRes =
                      await junctionStore.disconnectLinkRoad({
                        junctionId,
                        linkRoad,
                      })
                    if (disconnectRes) {
                      const objectIds: Array<string> = []
                      const objects: Array<biz.IObject> = []
                      const roadIds = []
                      // 将取消关联的道路，添加到数组中
                      if (Array.isArray(linkRoad)) {
                        linkRoad.forEach((item) => {
                          roadIds.push(item.split('_')[0])
                        })
                      } else {
                        roadIds.push(linkRoad.split('_')[0])
                      }
                      // 判断当前解除链接的道路，是否是跟路口导向线关联的道路，如果是，则需要删除路口导向线
                      handleIntersectionGuideLineByRoadId({
                        roadIds,
                        objectIds,
                        objects,
                        handler: (objectId: string) => {
                          const objectStore = useObjectStore()
                          // 删除路口导向线
                          objectStore.removeRoadSign(objectId, false)
                        },
                      })

                      const { junction: targetJunction } = disconnectRes
                      const _junctions = [targetJunction]
                      // 修改连通关系只涉及 junctionId 相关的 diff
                      historyStore.save({
                        title: i18n.global.t('actions.linkRoad.disconnect'),
                        junctionId,
                        junctions: _junctions,
                        objectId: objectIds,
                        objects,
                      })
                      // 销毁旧的高亮蒙层
                      disposeHighlightJunctionMask(parent)
                      // 渲染新的高亮蒙层
                      if (targetJunction) {
                        createHighlightJunctionMask({
                          junction: targetJunction,
                          parent,
                        })
                      }
                    }
                  } else {
                    store.unselectJunction()
                    // 如果路口本来由 2 条道路连接而成，在取消某一条后不足以形成路口，则直接删除路口
                    const removeJunctionRes = junctionStore.removeJunction(
                      junction.id,
                    )

                    if (removeJunctionRes) {
                      // 将跟删除的路口关联删除的所有物体，也保存在操作记录中
                      const {
                        junction: removedJunction,
                        objectId,
                        objects,
                      } = removeJunctionRes

                      // 修改连通关系只涉及 junctionId 相关的 diff
                      historyStore.save({
                        title: i18n.global.t('actions.junction.remove'),
                        junctionId: junction.id,
                        objectId,
                        junctions: [removedJunction],
                        objects,
                      })
                    }
                  }
                }
              } else {
                // 如果没有 junction，并且是取消选中，说明此时已选中的 linkRoad 数量不足 2 个，没有形成路口
              }

              this.render()
              break
            }
            case 'clearRoadEndEdge': {
              if (res && res.length > 0) {
                interactionStore.updateTimestamp()

                // 重置所有取消选中的端面颜色
                updateRoadEndEdgeColor({
                  parent,
                  linkRoad: res,
                  status: 'normal',
                })
                this.render()
              }
              break
            }
            case 'selectJunction': {
              const junctionId = args[0]
              if (!junctionId) return
              const junctionStore = useJunctionStore()
              const junction = junctionStore.getJunctionById(junctionId)
              if (!junction) return

              // 选中路口时，高亮当前路口连接的所有道路端面元素
              updateRoadEndEdgeColor({
                parent,
                linkRoad: junction.linkRoads,
                status: 'selected',
              })

              // 销毁原有的路口高亮蒙层
              disposeHighlightJunctionMask(parent)
              // 创建当前路口区域的高亮蒙层
              createHighlightJunctionMask({
                junction,
                parent,
              })

              this.render()
              break
            }
            case 'unselectJunction': {
              interactionStore.updateTimestamp()
              // 销毁原有的路口高亮蒙层
              disposeHighlightJunctionMask(parent)

              this.render()
              break
            }
            case 'removeCurrentSelectedJunction': {
              if (!res) return
              const { junction: removedJunction, objects, objectId } = res

              historyStore.save({
                title: i18n.global.t('actions.junction.remove'),
                junctionId: removedJunction.id,
                objectId,
                junctions: [removedJunction],
                objects,
              })

              // 在操作日志记录了删除的路口 id 后，再取消交互状态中记录的路口 id
              store.unselectJunction()
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
    const interactionStore = useJunctionInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose (): void {
    this.editHelper.dispose()
    this.unsubscribeJunctionStore && this.unsubscribeJunctionStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
  }
}

export default new EditJunctionPlugin()
