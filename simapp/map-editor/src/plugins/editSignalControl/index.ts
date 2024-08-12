import { Color, type Intersection, MOUSE, Mesh } from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import EditSignalControlHelper from './main3d'
import config from './config.json'
import { useSignalControlInteractionStore } from './store/interaction'
import {
  createTextSpriteInJunction,
  createTextSpriteInRoad,
  disposeAllTextSprite,
  junctionMaskColor,
  junctionMaskOpacity,
  resetAllMaskColor,
  roadMaskColor,
  roadMaskOpacity,
  updateMaskColor,
  updateTrafficLightType,
} from './main3d/draw'
import root3d from '@/main3d'
import {
  getSelected,
  registryHoverElement,
  setSelected,
} from '@/main3d/controls/dragControls'
import { useHistoryStore } from '@/stores/history'
import i18n from '@/locales'
import {
  getOptionalControlRoadAndJunction,
  updateTrafficLightSignalControl,
} from '@/stores/object/trafficLight'
import { useObjectStore } from '@/stores/object'
import { getJunction, getObject } from '@/utils/mapCache'
import {
  disposeBox3Helper,
  updateBox3HelperById,
  updateObjectOnPole,
  updateObjectOnPoleById,
} from '@/main3d/render/object'

class EditSignalControlPlugin extends TemplatePlugin {
  editHelper: EditSignalControlHelper
  // 备选的道路 id 集合
  optionalRoadId: Array<string>
  // 备选的路口 id 集合
  optionalJunctionId: Array<string>
  unsubscribeObjectStore: Function
  unsubscribeInteractionStore: Function
  constructor () {
    super()
    this.config = config
    this.optionalRoadId = []
    this.optionalJunctionId = []
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
              case 'roadMask': {
                setSelected(intersection)
                break
              }
              case 'junctionMask': {
                setSelected(intersection)
                break
              }
              case 'trafficLight': {
                setSelected(intersection)
                break
              }
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
          const interactionStore = useSignalControlInteractionStore()
          switch (selected.object.name) {
            // 道路的虚拟辅助网格（用于点选和高亮）
            case 'roadMask': {
              const lightData = interactionStore.currentTrafficLight
              if (!lightData) return
              // 判断是否可以点选道路
              if (!interactionStore.canSelectRoad) return
              // @ts-expect-error
              const { roadId = '' } = selected.object
              if (!roadId) return
              // 判断当前的道路是否处于备选集合中
              if (!this.optionalRoadId.includes(roadId)) return

              // 获取当前已经选中的控制道路
              const { controlRoadId } = lightData
              const objectStore = useObjectStore()

              let _controlRoadId = controlRoadId
              if (controlRoadId === roadId) {
                // 如果点击的道路跟已经选中控制的道路 id 一致，则取消当前道路的控制
                _controlRoadId = ''
              } else {
                // 否则选中新的道路
                _controlRoadId = roadId
              }
              objectStore.updateTrafficLightControlRoad({
                objectId: lightData.id,
                roadId: _controlRoadId,
              })

              break
            }
            // 路口的虚拟辅助网格（用于点选和高亮）
            case 'junctionMask': {
              const lightData = interactionStore.currentTrafficLight
              if (!lightData) return
              // 判断是否可以点选路口
              if (!interactionStore.canSelectJunction) return
              // @ts-expect-error
              const { junctionId = '' } = selected.object
              // 判断当前的路口是否在备选集合中
              if (!this.optionalJunctionId.includes(junctionId)) return

              // 获取当前已经选中的控制路口
              const { controlJunctionId } = lightData
              const objectStore = useObjectStore()
              let newJunctionId = junctionId
              if (controlJunctionId === junctionId) {
                // 如果已选中的路口再次点击选中，则取消对它的控制
                newJunctionId = ''
              }
              objectStore.updateTrafficLightControlJunction({
                objectId: lightData.id,
                junctionId: newJunctionId,
              })

              break
            }
            // 信号灯
            case 'trafficLight': {
              // @ts-expect-error
              const { objectId } = selected.object
              const { trafficLightId: lastTrafficLightId } = interactionStore
              // 如果存在上一次选中的信号灯
              if (lastTrafficLightId) {
                if (lastTrafficLightId === objectId) {
                  // 如果是同一个信号灯，则不响应
                  return
                } else {
                  // 如果不是同一个信号灯，则需要取消上一个信号灯的选中状态
                  interactionStore.unselectTrafficLight(lastTrafficLightId)
                }
              }
              // 选中目标的信号灯
              interactionStore.selectTrafficLight(objectId)
              break
            }
            case 'ground':
              // 如果是选择路口或者道路的交互模式，则点击空白处不响应
              if (
                !interactionStore.canSelectRoad ||
                !interactionStore.canSelectJunction
              ) {
                return
              }
              interactionStore.unselectTrafficLight()
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
        const interactionStore = useSignalControlInteractionStore()
        // 选中道路
        if (interactionStore.canSelectRoad) {
          if (elementName === 'roadMask') {
            // 展示道路蒙层的 hover 效果
            const targetObject = root3d.getObjectByUuid(elementUuid)
            if (targetObject instanceof Mesh) {
              // @ts-expect-error
              const { roadId } = targetObject
              // 如果 hover 的道路不在备选的集合中，则不响应高亮
              if (!this.optionalRoadId.includes(roadId)) return

              const { material } = targetObject
              if (
                !new Color(roadMaskColor.selected).equals(material.color)
              ) {
                material.color.set(roadMaskColor.hovered)
                material.opacity = roadMaskOpacity
                material.needsUpdate = true
                this.render()
              }
            }
          }
        } else if (interactionStore.canSelectJunction) {
          // 选中路口
          if (elementName === 'junctionMask') {
            // 展示路口蒙层的 hover 效果
            const targetObject = root3d.getObjectByUuid(elementUuid)
            if (targetObject instanceof Mesh) {
              // @ts-expect-error
              const { junctionId } = targetObject
              // 如果 hover 的路口不在备选的集合中，则不响应高亮
              if (!this.optionalJunctionId.includes(junctionId)) return

              const { material } = targetObject
              if (
                !new Color(junctionMaskColor.selected).equals(material.color)
              ) {
                material.color.set(junctionMaskColor.hovered)
                material.opacity = junctionMaskOpacity
                material.needsUpdate = true
                this.render()
              }
            }
          }
        } else {
          // 以上两者都不是
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (!targetObject) return

          // 信号灯的 hover 效果
          if (elementName === 'trafficLight') {
            updateObjectOnPole({
              type: elementName,
              object: targetObject,
              status: 'hovered',
            })
            this.render()
          }
        }
      },
      hoverOff: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { elementName, elementUuid } = options.object
        if (!elementName || !elementUuid) return
        const interactionStore = useSignalControlInteractionStore()
        if (interactionStore.canSelectRoad) {
          if (elementName === 'roadMask') {
            // 取消道路蒙层的 hover 效果
            const targetObject = root3d.getObjectByUuid(elementUuid)
            if (targetObject instanceof Mesh) {
              // @ts-expect-error
              const { roadId } = targetObject
              // 如果 hover 的道路不在备选的集合中，则不响应
              if (!this.optionalRoadId.includes(roadId)) return

              const { material } = targetObject
              if (
                !new Color(roadMaskColor.selected).equals(material.color)
              ) {
                material.color.set(roadMaskColor.normal)
                material.opacity = 0
                material.needsUpdate = true
                this.render()
              }
            }
          }
        } else if (interactionStore.canSelectJunction) {
          if (elementName === 'junctionMask') {
            // 取消路口蒙层的 hover 效果
            const targetObject = root3d.getObjectByUuid(elementUuid)
            if (targetObject instanceof Mesh) {
              // @ts-expect-error
              const { junctionId } = targetObject
              // 如果 hover 的路口不在备选的集合中，则不响应
              if (!this.optionalJunctionId.includes(junctionId)) return

              const { material } = targetObject
              if (
                !new Color(junctionMaskColor.selected).equals(material.color)
              ) {
                material.color.set(junctionMaskColor.normal)
                material.opacity = 0
                material.needsUpdate = true
                this.render()
              }
            }
          }
        } else {
          // 以上两者都不是
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (!targetObject) return

          // 信号灯的 hover 效果
          if (elementName === 'trafficLight') {
            updateObjectOnPole({
              type: elementName,
              object: targetObject,
              status: 'normal',
            })
            this.render()
          }
        }
      },
    }
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditSignalControlHelper({
      render: this.render,
    })

    this.editHelper.init()

    this.scene.add(this.editHelper.container)
    this.initDispatchers()

    this.addDragListener()

    // 注册当前模式下，可支持 hover 交互元素的名称
    registryHoverElement({
      pluginName: this.config.name,
      objectName: [
        'roadMask', // 响应道路的蒙层
        'junctionMask', // 响应路口的蒙层
        'trafficLight', // 响应信号灯
      ],
    })
  }

  initDispatchers () {
    const interactionStore = useSignalControlInteractionStore()
    const objectStore = useObjectStore()
    const historyStore = useHistoryStore()
    interactionStore.$reset()

    // 辅助元素所在的容器
    const parent = this.editHelper.container
    // 实际地图元素所在的容器
    const mapContainer = root3d.mapElementsContainer

    // 由于是通过 objectStore 更新信控配置，需要监听 objectStore 的方法
    this.unsubscribeObjectStore = objectStore.$onAction(
      ({ name, store, args, after, onError }) => {
        // 获取状态更新前，信号灯控制的路口和道路信息
        let originControlRoad: string
        let originControlJunction: string
        if (name === 'updateTrafficLightControlRoad') {
          const { objectId } = args[0]
          const originObjectData = getObject(objectId) as biz.ITrafficLight
          if (originObjectData) {
            // 记录上一个状态控制的道路，用于销毁的渲染
            originControlRoad = originObjectData.controlRoadId
          }
        }
        if (name === 'updateTrafficLightControlJunction') {
          const { objectId } = args[0]
          const originObjectData = getObject(objectId) as biz.ITrafficLight
          if (originObjectData) {
            // 记录上一个状态控制的路口，用于销毁的渲染
            originControlJunction = originObjectData.controlJunctionId
          }
        }

        after(async (res) => {
          switch (name) {
            case 'applyState': {
              interactionStore.updateTimestamp()
              // updateTrafficLightType 在撤销重做时的模型还原，是在 dispatcher/object 中执行的，因此在信控配置模式不需要重复更新
              // 控制道路和路口的状态恢复，主要在 interactionStore 中定义【因为不涉及实际地图元素的更新】
              break
            }
            case 'updateTrafficLightType': {
              const lightId = res as string
              const lightData = getObject(lightId) as biz.ITrafficLight
              if (!lightData) return

              interactionStore.updateTimestamp()

              // 从渲染层更新信号灯模型
              await updateTrafficLightType({
                data: lightData,
                parent: mapContainer,
              })

              // 在完成新的信号灯模型渲染后，让新的模型处于选中交互效果状态
              updateObjectOnPoleById({
                type: 'trafficLight',
                objectId: lightId,
                parent: mapContainer,
                status: 'selected',
                isForced: true,
              })

              // 保存操作记录
              historyStore.save({
                title: i18n.global.t(
                  'actions.signalControl.updateTrafficLightType',
                ),
                objectId: lightId,
                objects: [lightData],
              })

              // 更新信号灯的高亮包围盒边框
              updateBox3HelperById(lightId)

              this.render()
              break
            }
            case 'updateTrafficLightControlRoad': {
              const lightId = res as string
              const lightData = getObject(lightId) as biz.ITrafficLight
              if (!lightData) return

              interactionStore.updateTimestamp()

              // 取消原有道路蒙层的高亮颜色显示
              updateMaskColor({
                roadId: originControlRoad,
                parent,
                status: 'normal',
              })
              // 激活选中的道路蒙层颜色
              updateMaskColor({
                roadId: lightData.controlRoadId,
                parent,
                status: 'selected',
              })

              // 保存操作记录
              historyStore.save({
                title: i18n.global.t(
                  'actions.signalControl.updateTrafficLightControlRoad',
                ),
                objectId: lightId,
                objects: [lightData],
              })

              this.render()
              break
            }
            case 'updateTrafficLightControlJunction': {
              const lightId = res as string
              const lightData = getObject(lightId) as biz.ITrafficLight
              if (!lightData) return

              interactionStore.updateTimestamp()

              // 先删除所有的辅助文字精灵
              disposeAllTextSprite(parent)
              // 重置所有的道路和路口蒙层状态
              resetAllMaskColor({
                parent,
              })

              selectTrafficLightHandler(lightData)

              // 保存操作记录
              historyStore.save({
                title: i18n.global.t(
                  'actions.signalControl.updateTrafficLightBelongingJunction',
                ),
                objectId: lightId,
                objects: [lightData],
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

    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after(async (res) => {
          switch (name) {
            case 'applyState': {
              interactionStore.updateTimestamp()

              const [currentState] = args
              const { trafficLightId } = currentState
              // 清空备选的集合
              this.optionalRoadId = []
              this.optionalJunctionId = []

              // 先删除所有的辅助文字精灵
              disposeAllTextSprite(parent)
              // 重置所有的道路和路口蒙层状态
              resetAllMaskColor({
                parent,
              })

              // 重新选中信号灯
              if (trafficLightId) {
                // 强制更新，保证渲染层逻辑能够执行
                interactionStore.selectTrafficLight(trafficLightId, true)
              }

              this.render()

              break
            }
            case 'selectTrafficLight': {
              const lastTrafficLightId = res as string
              const { trafficLightId } = store
              interactionStore.updateTimestamp()

              if (lastTrafficLightId && lastTrafficLightId !== trafficLightId) {
                // 如果存在上一次的信号灯，则取消对应的交互效果
                updateObjectOnPoleById({
                  type: 'trafficLight',
                  objectId: lastTrafficLightId,
                  parent: mapContainer,
                  status: 'normal',
                  isForced: true,
                })
              }
              if (trafficLightId) {
                const lightData = getObject(trafficLightId) as biz.ITrafficLight
                if (lightData) {
                  selectTrafficLightHandler(lightData)
                }

                // 选中信号灯的交互效果
                updateObjectOnPoleById({
                  type: 'trafficLight',
                  objectId: trafficLightId,
                  parent: mapContainer,
                  status: 'selected',
                  isForced: true,
                })

                updateBox3HelperById(trafficLightId)
              }

              this.render()
              break
            }
            case 'unselectTrafficLight': {
              interactionStore.updateTimestamp()

              const lastTrafficLightId = res as string
              // 如果没有要取消选中的信号的，则直接返回
              if (!lastTrafficLightId) return

              // 取消信号灯的选中交互效果
              updateObjectOnPoleById({
                type: 'trafficLight',
                objectId: lastTrafficLightId,
                parent: mapContainer,
                status: 'normal',
                isForced: true,
              })

              // 还原所有蒙层的状态
              resetAllMaskColor({ parent })

              // 销毁所有辅助的文字精灵提示
              disposeAllTextSprite(parent)

              // 清空备选的集合
              this.optionalRoadId = []
              this.optionalJunctionId = []

              // 销毁包围盒外框
              disposeBox3Helper()

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

    // 选中信号灯三维场景渲染层的更新
    const selectTrafficLightHandler = (lightData: biz.ITrafficLight) => {
      const { id: lightId, controlJunctionId, controlRoadId } = lightData
      // 针对选中的信号灯，展示当前信号灯相关的辅助元素
      const { roadId: optionalRoadId, junctionId: optionalJunctionId } =
        getOptionalControlRoadAndJunction(lightId)

      // 已控制的道路蒙层高亮
      updateMaskColor({
        roadId: controlRoadId,
        status: 'selected',
        parent,
      })
      // 已控制的路口蒙层高亮
      updateMaskColor({
        junctionId: controlJunctionId,
        status: 'selected',
        parent,
      })

      // 选中信号灯以后，同步可备选的道路和路口 id
      this.optionalRoadId = optionalRoadId
      this.optionalJunctionId = optionalJunctionId

      // 在路口的中心创建文字提示
      for (const _junctionId of optionalJunctionId) {
        createTextSpriteInJunction({
          junctionId: _junctionId,
          content: `${i18n.global.t('desc.junction')}-${_junctionId}`,
          parent,
        })

        const junction = getJunction(_junctionId)
        // 如果存在已经选中的路口，则指显示目标路口
        if (controlJunctionId && controlJunctionId !== _junctionId) {
          continue
        }
        if (junction) {
          for (const refRoad of junction.refRoads) {
            const { roadId, leftPoint, rightPoint } = refRoad
            // 两个边界线端点的中点
            const pos = {
              x: (leftPoint.x + rightPoint.x) / 2,
              y: (leftPoint.y + rightPoint.y) / 2,
              z: (leftPoint.z + rightPoint.z) / 2,
            }
            // 根据路口关联的道路，渲染道路连通路口侧的文字精灵
            createTextSpriteInRoad({
              content: `${i18n.global.t('desc.road')}-${roadId}`,
              position: pos,
              parent,
            })
          }
        }
      }
    }
  }

  onActivate () {
    // 在信控编辑模式激活时，更新当前所有信号灯的信控配置，保证绑定的道路和路口都是有效的
    const objectStore = useObjectStore()
    updateTrafficLightSignalControl(objectStore.ids)
  }

  onDeactivate () {
    // 在插件取消激活时，重置插件内所有的交互状态
    const interactionStore = useSignalControlInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose (): void {
    this.editHelper.dispose()
    this.unsubscribeObjectStore && this.unsubscribeObjectStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
  }
}

export default new EditSignalControlPlugin()
