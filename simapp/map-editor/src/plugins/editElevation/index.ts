import { type Intersection, MOUSE } from 'three'
import { cloneDeep } from 'lodash'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditElevationHelper from './main3d/'
import { useElevationInteractionStore } from './store/interaction'
import { useHistoryStore } from '@/stores/history'
import { useRoadStore } from '@/stores/road'
import root3d from '@/main3d'
import { getSelected, setSelected } from '@/main3d/controls/dragControls'
import i18n from '@/locales'
import { getJunction, getRoad } from '@/utils/mapCache'
import { useJunctionStore } from '@/stores/junction'
import { updateObjectInRoad } from '@/main3d/dispatcher/common'
import root2d from '@/main2d'
import {
  createHighlightRoadMask,
  disposeHighlightRoadMask,
  renderRoad,
} from '@/main3d/render/road'
import { handleIntersectionGuideLineByRoadId } from '@/stores/object/customRoadSign/intersectionGuideLine'
import { useObjectStore } from '@/stores/object'

class EditElevationPlugin extends TemplatePlugin {
  editHelper: EditElevationHelper
  unsubscribeRoadStore: Function
  unsubscribeInteractionStore: Function
  constructor () {
    super()
    this.config = config

    // 覆写模板中定义的空的交互事件
    this.dragDispatchers = {
      dragStart: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const { intersections, pointerdownButton } = options.object
        if (pointerdownButton !== MOUSE.LEFT) return

        // 如果是左键点击，则选中对应的元素包括：整条道路、参考线
        intersections.forEach((intersection: Intersection) => {
          if (getSelected()) return

          const { object } = intersection

          switch (object.name) {
            case 'lane': {
              setSelected(intersection)
              break
            }
            case 'ground': {
              setSelected(intersection)
              break
            }
            default:
              break
          }
        })
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
        if (pointerdownButton !== MOUSE.LEFT) return
        const selected = getSelected()
        if (!selected) return
        const interactionStore = useElevationInteractionStore()
        switch (selected.object.name) {
          case 'lane': {
            // 选中一条车道对应的道路
            // @ts-expect-error
            const { roadId } = selected.object
            if (!roadId) return
            // 取消上一个道路的选中
            if (interactionStore.roadId && interactionStore.roadId !== roadId) {
              interactionStore.unselectRoad()
            }
            interactionStore.selectRoad(roadId)

            break
          }
          case 'ground': {
            // 鼠标左键点击 ground，则取消当前道路、控制点的选中状态
            interactionStore.unselectRoad()
            break
          }
          default:
            break
        }
      },
      hoverOn: (options: common.IDragOptions) => {
        if (!this.enabled) return undefined
      },
      hoverOff: (options: common.IDragOptions) => {
        if (!this.enabled) return undefined
      },
    }
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditElevationHelper({
      render: this.render,
    })
    this.editHelper.init()

    this.scene.add(this.editHelper.container)

    this.initDispatchers()

    this.addDragListener()
  }

  initDispatchers () {
    const roadStore = useRoadStore()
    const interactionStore = useElevationInteractionStore()
    const historyStore = useHistoryStore()
    interactionStore.$reset()

    const parent = this.editHelper.container

    // 监听道路编辑模式下，控制点移动的方法
    this.unsubscribeRoadStore = roadStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after(async (res) => {
          // 如果当前插件不可用，则不响应道路编辑 store 的操作
          if (!this.enabled) return

          // 目前只响应高程的更新
          if (name !== 'updateRoadElevation') return
          // 更新控制点的类型
          const actionType = args[1] || 'update'
          interactionStore.updateTimestamp()

          const _res = res as Array<string>

          // 获取当前参考线控制点集对应的 road 连通的 junctionId，需要联动更新
          const _roads = []
          const _junctions = []
          const diffRoadId = []
          const diffJunctionId: Array<string> = []

          // 将可能存在更新的道路和路口进行汇总，缓存到操作记录中
          for (const roadId of _res) {
            const _road = roadStore.getRoadById(roadId)
            if (_road) {
              renderRoad({
                road: _road,
                parent: root3d.mapElementsContainer,
              })

              diffRoadId.push(roadId)
              _road && _roads.push(_road)
              for (const _junctionId of _road.linkJunction) {
                // 判断是否有重复的 junction
                if (!diffJunctionId.includes(_junctionId)) {
                  diffJunctionId.push(_junctionId)
                }
              }
            }
          }

          // 触发二维场景高程的更新
          const currentRoad = getRoad(interactionStore.roadId)
          if (currentRoad && currentRoad.elevationPath) {
            const points = cloneDeep(currentRoad.elevationPath.points)

            root2d.updateControlPoints({
              roadId: currentRoad.id,
              points,
              isSync: true,
            })
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
                    junctionStore
                      .updateJunction({
                        junction: _junction,
                        isPureUpdate: true,
                      })
                      .then(() => {
                        resolve(_junction.id)
                      })
                  })
                })(),
              )
            }
          }
          // 等待所有的路口数据更新完毕，再触发操作记录保存
          await Promise.all(updateJunctionPromise)

          // 道路高程调整时，跟当前道路关联的路口导向线需要联动更新
          handleIntersectionGuideLineByRoadId({
            roadIds: diffRoadId,
            objectIds: objectId,
            objects,
            handler: (objectId: string) => {
              const objectStore = useObjectStore()
              // 更新路口导向线的渲染效果
              objectStore.updateRoadSignByJunction(objectId)
            },
          })

          // 根据更新控制点的类型，调整输出日志的文案
          let title = i18n.global.t(
            'actions.elevation.updateElevationControlPointHeight',
          )
          if (actionType === 'add') {
            title = i18n.global.t('actions.elevation.addElevationControlPoint')
          } else if (actionType === 'remove') {
            title = i18n.global.t(
              'actions.elevation.removeElevationControlPoint',
            )
          }

          // 保存更新参考线控制点高度的操作记录
          historyStore.save({
            title,
            roadId: diffRoadId,
            junctionId: diffJunctionId,
            roads: _roads,
            junctions: _junctions,
            objectId,
            objects,
          })

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

          this.render()
        })

        onError((err) => {
          console.log(err)
        })
      },
    )

    // 交互部分的方法监听
    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          if (!this.enabled) return
          switch (name) {
            case 'applyState': {
              store.updateTimestamp()

              const [currentState, diffState] = args
              const { roadId: currentRoadId } = currentState

              // 先销毁原有的道路高亮蒙层
              disposeHighlightRoadMask(parent)
              const currentRoad = getRoad(currentRoadId)
              if (currentRoad) {
                // 再渲染选中道路的高亮蒙层
                createHighlightRoadMask({
                  road: currentRoad,
                  parent,
                })
              }

              this.render()
              return
              break
            }
            case 'selectRoad': {
              store.updateTimestamp()
              // 先销毁原有的道路高亮蒙层
              disposeHighlightRoadMask(parent)
              const currentRoad = getRoad(store.roadId)
              if (currentRoad) {
                // 再渲染选中道路的高亮蒙层
                createHighlightRoadMask({
                  road: currentRoad,
                  parent,
                })
              }
              this.render()
              break
            }
            case 'unselectRoad': {
              store.updateTimestamp()
              // 销毁道路高亮蒙层
              disposeHighlightRoadMask(parent)
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
    const interactionStore = useElevationInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.removeDragListener()
    this.editHelper.dispose()
    this.unsubscribeRoadStore && this.unsubscribeRoadStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
  }
}

export default new EditElevationPlugin()
