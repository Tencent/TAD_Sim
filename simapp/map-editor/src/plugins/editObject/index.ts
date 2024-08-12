import { cloneDeep, throttle } from 'lodash'
import {
  Color,
  type Intersection,
  MOUSE,
  Mesh,
  Quaternion,
  Vector3,
} from 'three'
import TemplatePlugin, { type IInitPluginParams } from '../template/plugin'
import config from './config.json'
import EditObjectHelper from './main3d'
import { useObjectInteractionStore } from './store/interaction'
import {
  disposeVirtualObject,
  junctionMaskColor,
  junctionMaskOpacity,
  renderVirtualCrosswalk,
  renderVirtualCrosswalkWithSide,
  renderVirtualCustomModel,
  renderVirtualCustomRoadSign,
  renderVirtualElementOnPole,
  renderVirtualOther,
  renderVirtualParkingSpace,
  renderVirtualPole,
  renderVirtualRoadSign,
  renderVirtualStopLine,
  setAllTextSpriteVisible,
  updateJunctionMaskColor,
} from './main3d/draw'
import { axisY, getVerticalVector, halfPI } from '@/utils/common3d'
import { calcRoadSignLocationInRoad } from '@/stores/object/roadSign'
import { getObject, getRoad } from '@/utils/mapCache'
import { getLocationInRoad } from '@/stores/object/common'
import { useObjectStore } from '@/stores/object'
import {
  getSelected,
  registryHasPoleGroupElements,
  registryHoverElement,
  registryIgnoreDragOffsetElements,
  setInteractionPlane,
  setSelected,
} from '@/main3d/controls/dragControls'
import {
  disposeBox3Helper,
  hideOriginObject,
  showOriginObject,
  updateBox3HelperById,
  updateObjectOnPole,
  updateObjectOnPoleById,
} from '@/main3d/render/object'
import root3d from '@/main3d/index'
import {
  calcPositionOnHorizontalPole,
  calcPositionOnVerticalPole,
} from '@/stores/object/pole'
import { getCornerVerticesAttr, inValidArea } from '@/utils/business'
import { getOtherConfig } from '@/config/other'
import { getRoadSignConfig } from '@/config/roadSign'
import { warningMessage } from '@/utils/common'
import i18n from '@/locales'
import { useJunctionStore } from '@/stores/junction'
import {
  updateRoadSignColor,
  updateRoadSignColorById,
} from '@/main3d/render/roadSign'
import {
  updateParkingSpaceColor,
  updateParkingSpaceColorById,
} from '@/main3d/render/parkingSpace'
import { updatePoleColor, updatePoleColorById } from '@/main3d/render/pole'
import { updateOtherObject, updateOtherObjectById } from '@/main3d/render/other'
import { getStopLineAttrOnRoad } from '@/stores/object/customRoadSign/stopLine'
import { getCrosswalkAttrOnRoad } from '@/stores/object/customRoadSign/crosswalk'
import { getGuideLaneLineAttr } from '@/stores/object/customRoadSign/guideLaneLine'
import { getVariableDirectionLaneLineAttr } from '@/stores/object/customRoadSign/variableDirectionLaneLine'
import { getCrosswalkWithSideAttrOnRoad } from '@/stores/object/customRoadSign/crosswalkWithSide'
import { getLongitudinalDecelerationAttr } from '@/stores/object/customRoadSign/longitudinalDecelerationMarking'
import { getLateralDecelerationAttr } from '@/stores/object/customRoadSign/lateralDecelerationMarking'
import { getSemicircleLineAttr } from '@/stores/object/customRoadSign/semicircleLine'
import {
  updateCustomModel,
  updateCustomModelById,
} from '@/main3d/render/customModel'
import { useModelCacheStore } from '@/stores/object/modelCache'
import { getPermission } from '@/utils/permission'

class EditObjectPlugin extends TemplatePlugin {
  editHelper: EditObjectHelper
  unsubscribeInteractionStore: Function
  unsubscribeObjectStore: Function
  unsubscribeModelCacheStore: Function
  constructor () {
    super()
    this.config = config

    // 覆写模板中定义的交互逻辑
    this.dragDispatchers = {
      dragStart: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const { intersections, pointerdownButton } = options.object
        if (pointerdownButton !== MOUSE.LEFT) return

        const interactionStore = useObjectInteractionStore()
        intersections.forEach((intersection: Intersection) => {
          if (getSelected()) return

          const { object } = intersection
          // @ts-expect-error
          const { objectId } = object

          // 如果是选择 rsu 关联的路口交互模式，应该只响应 junctionMask 元素
          // 如果是选择传感器关联的通信单元模式，应该只响应 sensor 元素，且 name 为 RSU
          if (interactionStore.canSelectJunction) {
            if (object.name !== 'junctionMask') return
          }
          if (interactionStore.canSelectRsu) {
            if (object.name !== 'sensor') return
          }

          switch (object.name) {
            // 路口辅助蒙层
            case 'junctionMask': {
              // 由于路口蒙层层级相对于路面标识的层级要高，在非选中路口的模式下，忽略路口蒙层的选中
              if (!interactionStore.canSelectJunction) return

              setSelected(intersection)
              break
            }
            case 'ground': {
              setSelected(intersection)
              break
            }
            // 杆
            case 'verticalPoleTop': // 竖杆的顶部
            case 'horizontalPoleTop': // 横杆的顶部
            case 'verticalPole':
            case 'horizontalPole': {
              setSelected(intersection)
              // 获取网格外层的杆容器
              const poleContainer = object.parent
              if (!poleContainer) return

              // 获取杆的数据中缓存的切线和参考线最近点相关数据
              const poleData = getObject(objectId) as biz.IPole
              let normal = axisY
              if (poleData) {
                const { roadId, closestPoint, closestPointTangent } = poleData
                if (roadId && closestPoint && closestPointTangent) {
                  // 获取当前物体在道路上的坡度法向量
                  const vertical =
                    getVerticalVector(closestPointTangent).normalize()
                  normal = new Vector3(
                    closestPointTangent.x,
                    closestPointTangent.y,
                    closestPointTangent.z,
                  )
                  normal.applyAxisAngle(vertical, halfPI).normalize()
                }
              }

              // 设置交互平面为杆容器所在坐标的水平面
              setInteractionPlane({
                normal,
                point: intersection.point,
              })
              // 如果已经选中了杆，则禁用视角控制器
              if (interactionStore.poleId === objectId) {
                root3d.setViewControlsEnabled(false)
              }
              break
            }
            // 交通信号灯、信号标志牌、传感器，处理逻辑一样
            case 'signalBoard':
            case 'trafficLight':
            case 'sensor': {
              // 获取网格的外层容器对象
              const objectGroup = object.parent
              if (!objectGroup) return
              // @ts-expect-error
              const { poleId } = object
              const { position: groupPosition } = objectGroup
              const objectData = getObject(objectId) as
                | biz.ITrafficLight
                | biz.ISignalBoard
                | biz.ISensor

              // 如果是传感器选择关联的通信单元交互模式下，只响应 rsu 元素的点击
              if (interactionStore.canSelectRsu) {
                if (objectData.name !== 'RSU') return
              }

              if (!objectData) return

              setSelected(intersection)

              // 通过 poleId 获取杆容器的三维场景对象
              const poleGroup = root3d.mapElementsContainer.getObjectByProperty(
                'objectId',
                poleId,
              )
              if (!poleGroup) return
              const { onVerticalPole } = objectData
              // 物体在杆上的局部坐标
              const objectPosition = onVerticalPole ?
                new Vector3(groupPosition.x, 0, groupPosition.z) :
                new Vector3(groupPosition.x, 0, 0)
              // 将物体局部坐标转换成世界坐标系下的绝对坐标
              objectPosition.applyMatrix4(poleGroup.matrixWorld)
              // 获取杆容器的世界坐标
              const polePosition = new Vector3()
              poleGroup.getWorldPosition(polePosition)

              // 从杆指向物杆上物体的法向量（用于确定拖拽平面）
              const normalVec3 = objectPosition
                .clone()
                .sub(polePosition)
                .normalize()
              // 如果是放置在横杆上的物体，则移动平面的法向量带有一定垂直方向分量
              // 保证正交视角下横杆上物体的移动
              if (!onVerticalPole) {
                normalVec3.y = 1
              }
              normalVec3.normalize()

              // 如果外层的容器存在，则控制拖拽的基准平面
              setInteractionPlane({
                normal: normalVec3,
                point: intersection.point,
              })

              const _objectId = interactionStore[`${object.name}Id`]
              if (_objectId && _objectId === objectId) {
                // 如果已经选中了物体，则禁用视角控制器
                root3d.setViewControlsEnabled(false)
              }
              break
            }
            // 路面标识
            case 'roadSign': {
              setSelected(intersection)

              // 获取路面标线的数据中缓存的切线和参考线最近点相关数据
              const roadSignData = getObject(objectId) as biz.IRoadSign
              let normal = axisY
              if (roadSignData) {
                const roadSignConfig = getRoadSignConfig(roadSignData.name)
                if (roadSignConfig) {
                  const { roadId, closestPoint, closestPointTangent } =
                    roadSignData
                  if (roadId && closestPoint && closestPointTangent) {
                    // 获取当前物体在道路上的坡度法向量
                    const vertical =
                      getVerticalVector(closestPointTangent).normalize()
                    normal = new Vector3(
                      closestPointTangent.x,
                      closestPointTangent.y,
                      closestPointTangent.z,
                    )
                    normal.applyAxisAngle(vertical, halfPI).normalize()
                  }
                }
              }

              // 由于在开始拖拽的阶段，不确定是否会移动控制点，需要提前设置移动的基准平面
              setInteractionPlane({
                normal,
                // 路面标识几乎是贴合路面，基准平面的参考点可直接使用鼠标射线交互点
                point: intersection.point,
              })

              // 如果此时选中了路面标识，则禁用视角控制器
              if (interactionStore.roadSignId === objectId) {
                root3d.setViewControlsEnabled(false)
              }
              break
            }
            case 'other': {
              setSelected(intersection)

              // 获取数据中缓存的切线和参考线最近点相关数据
              const otherData = getObject(objectId) as biz.IOther
              let normal = axisY
              if (otherData) {
                const { roadId, closestPoint, closestPointTangent } = otherData
                if (roadId && closestPoint && closestPointTangent) {
                  // 获取当前物体在道路上的坡度法向量
                  const vertical =
                    getVerticalVector(closestPointTangent).normalize()
                  normal = new Vector3(
                    closestPointTangent.x,
                    closestPointTangent.y,
                    closestPointTangent.z,
                  )
                  normal.applyAxisAngle(vertical, halfPI).normalize()
                }
              }

              // 由于在开始拖拽的阶段，不确定是否会移动控制点，需要提前设置移动的基准平面
              setInteractionPlane({
                normal,
                point: intersection.point,
              })

              // 如果此时选中了其他类型物体，则禁用视角控制器
              if (interactionStore.otherId === objectId) {
                root3d.setViewControlsEnabled(false)
              }
              break
            }
            // 自定义的路面标线（通过geoAttrs创建几何体）
            case 'customRoadSign':
            case 'parkingSpace': {
              setSelected(intersection)

              setInteractionPlane({
                normal: axisY,
                point: intersection.point,
              })

              if (interactionStore.roadSignId === objectId) {
                root3d.setViewControlsEnabled(false)
              }
              break
            }
            // 自定义导入的模型
            case 'customModel': {
              setSelected(intersection)

              // 获取数据中缓存的切线和参考线最近点相关数据
              const modelData = getObject(objectId) as biz.ICustomModel
              let normal = axisY
              if (modelData) {
                const { roadId, closestPoint, closestPointTangent } = modelData
                if (roadId && closestPoint && closestPointTangent) {
                  // 获取当前物体在道路上的坡度法向量
                  const vertical =
                    getVerticalVector(closestPointTangent).normalize()
                  normal = new Vector3(
                    closestPointTangent.x,
                    closestPointTangent.y,
                    closestPointTangent.z,
                  )
                  normal.applyAxisAngle(vertical, halfPI).normalize()
                }
              }

              // 由于在开始拖拽的阶段，不确定是否会移动控制点，需要提前设置移动的基准平面
              setInteractionPlane({
                normal,
                point: intersection.point,
              })

              // 如果此时选中了自定义模型物体，则禁用视角控制器
              if (interactionStore.customModelId === objectId) {
                root3d.setViewControlsEnabled(false)
              }
              break
            }
            default:
              break
          }
        })
      },
      drag: (options: common.IDragOptions) => {
        if (!this.enabled) return
        const selected = getSelected()
        if (!selected) return

        const interactionStore = useObjectInteractionStore()

        // @ts-expect-error
        const { name: objectName, objectId } = selected.object

        // 拖拽前判断是否已经选中了物体
        if (
          objectName === 'roadSign' &&
          interactionStore.roadSignId === objectId
        ) {
          // 拖拽虚拟的辅助道路元素
          this.dragPreviewRoadSign(options)
        } else if (
          objectName === 'signalBoard' &&
          interactionStore.signalBoardId === objectId
        ) {
          this.dragPreviewObjectOnPole(options, 'signalBoard')
        } else if (
          objectName === 'trafficLight' &&
          interactionStore.trafficLightId === objectId
        ) {
          this.dragPreviewObjectOnPole(options, 'trafficLight')
        } else if (
          objectName === 'sensor' &&
          interactionStore.sensorId === objectId
        ) {
          this.dragPreviewObjectOnPole(options, 'sensor')
        } else if (
          (objectName === 'verticalPole' ||
          objectName === 'horizontalPole' ||
          objectName === 'verticalPoleTop' ||
          objectName === 'horizontalPoleTop') &&
          interactionStore.poleId === objectId
        ) {
          this.dragPreviewPole(options)
        } else if (
          objectName === 'other' &&
          interactionStore.otherId === objectId
        ) {
          const otherData = getObject(objectId) as biz.IOther
          if (otherData.name === 'PedestrianBridge') {
            const canMove = getPermission(
              'action.mapEditor.models.other.bridge.props.position.enable',
            )
            if (!canMove) return
          }

          this.dragPreviewOther(options)
        } else if (
          objectName === 'parkingSpace' &&
          interactionStore.roadSignId === objectId
        ) {
          // 停车位属于 roadSign 的特殊情况
          this.dragPreviewParkingSpace(options)
        } else if (
          objectName === 'customRoadSign' &&
          interactionStore.roadSignId === objectId
        ) {
          // 通过 geoAttrs 创建的自定义路面标线
          this.dragPreviewCustomRoadSign(options)
        } else if (
          objectName === 'customModel' &&
          interactionStore.customModelId === objectId
        ) {
          // 自定义模型的拖拽
          this.dragPreviewCustomModel(options)
        }
      },
      dragEnd: throttle((options: common.IDragOptions) => {
        if (!this.enabled) return
        const selected = getSelected()
        if (!selected) {
          // 兜底逻辑：销毁不同类型物体的预览放置元素，避免在拖拽结束后还有未删除的虚拟放置元素
          this.editHelper.clearVirtualElements()
          return
        }
        // 不管选中什么物体，在拖拽结束后，恢复视角控制器
        root3d.setViewControlsEnabled(true)

        const interactionStore = useObjectInteractionStore()
        // 拖拽的前提是先 click 点选物体

        // @ts-expect-error
        const { name: objectName, objectId } = selected.object

        if (
          objectName === 'roadSign' &&
          interactionStore.roadSignId === objectId
        ) {
          // 路面标识
          this.dragEndRoadSign(options)
        } else if (
          (objectName === 'verticalPole' ||
          objectName === 'horizontalPole' ||
          objectName === 'verticalPoleTop' ||
          objectName === 'horizontalPoleTop') &&
          interactionStore.poleId === objectId
        ) {
          // 杆
          this.dragEndPole(options)
        } else if (
          objectName === 'trafficLight' &&
          interactionStore.trafficLightId === objectId
        ) {
          // 交通灯
          this.dragEndObjectOnPole(options, 'trafficLight')
        } else if (
          objectName === 'sensor' &&
          interactionStore.sensorId === objectId
        ) {
          // 传感器
          this.dragEndObjectOnPole(options, 'sensor')
        } else if (
          objectName === 'signalBoard' &&
          interactionStore.signalBoardId === objectId
        ) {
          // 信号标志牌
          this.dragEndObjectOnPole(options, 'signalBoard')
        } else if (
          objectName === 'other' &&
          interactionStore.otherId === objectId
        ) {
          // 其他类型物体
          this.dragEndOther(options)
        } else if (
          objectName === 'parkingSpace' &&
          interactionStore.roadSignId === objectId
        ) {
          // 停车位（属于 roadSign的特例）
          this.dragEndParkingSpace(options)
        } else if (
          objectName === 'customRoadSign' &&
          interactionStore.roadSignId === objectId
        ) {
          // 通过 geoAttrs 创建的自定义标线
          this.dragEndCustomRoadSign(options)
        } else if (
          objectName === 'customModel' &&
          interactionStore.customModelId === objectId
        ) {
          // 自定义导入模型
          this.dragEndCustomModel(options)
        }

        // 兜底逻辑：销毁不同类型物体的预览放置元素，避免在拖拽结束后还有未删除的虚拟放置元素
        this.editHelper.clearVirtualElements()
      }, 100),
      click: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { pointerdownButton } = options.object
        if (pointerdownButton !== MOUSE.LEFT) return

        const selected = getSelected()
        if (!selected) return

        // click 操作结束后，也会恢复视角控制器
        root3d.setViewControlsEnabled(true)

        const interactionStore = useObjectInteractionStore()
        // @ts-expect-error
        const { objectId } = selected.object
        switch (selected.object.name) {
          // 杆
          case 'verticalPoleTop': // 竖杆顶部
          case 'horizontalPoleTop': // 横杆顶部
          case 'verticalPole':
          case 'horizontalPole': {
            if (!objectId) return
            interactionStore.selectPole(objectId)
            break
          }
          // 交通信号灯
          case 'trafficLight': {
            if (!objectId) return
            interactionStore.selectTrafficLight(objectId)
            break
          }
          // 信号标志牌
          case 'signalBoard': {
            if (!objectId) return
            interactionStore.selectSignalBoard(objectId)
            break
          }
          // 传感器
          case 'sensor': {
            if (!objectId) return

            // 如果当前是传感器选择关联的通信单元的交互模式
            if (interactionStore.canSelectRsu) {
              // 选中的通信单元
              const sensorData = getObject(objectId) as biz.ISensor
              // 当前选中的传感器
              const { currentSensor } = interactionStore
              if (currentSensor && sensorData && sensorData.name === 'RSU') {
                // 一个传感器只能关联一个通信单元
                let belongRsu = objectId
                if (currentSensor.deviceParams.BelongRSU === objectId) {
                  // 如果之前已经关联了通信单元，则再次点击取消关联
                  belongRsu = ''
                }

                const objectStore = useObjectStore()
                objectStore.updateSensorDeviceParams({
                  objectId: currentSensor.id,
                  deviceParams: {
                    ...currentSensor.deviceParams,
                    BelongRSU: belongRsu,
                  },
                })
              }
            } else {
              interactionStore.selectSensor(objectId)
            }
            break
          }
          // 路面标识
          case 'customRoadSign':
          case 'roadSign': {
            if (!objectId) return
            interactionStore.selectRoadSign(objectId)
            break
          }
          // 其他类型
          case 'other': {
            if (!objectId) return
            interactionStore.selectOther(objectId)
            break
          }
          // 自定义导入模型
          case 'customModel': {
            if (!objectId) return
            interactionStore.selectCustomModel(objectId)
            break
          }
          case 'parkingSpace': {
            if (!objectId) return
            interactionStore.selectRoadSign(objectId)

            // 由于 drag 极小距离是不会触发 dragEnd 事件的，此时可能会存在 dragPreview 事件的触发
            // 需要将虚拟辅助的停车位销毁

            // 完成拖拽后，恢复原始物体的显示状态
            showOriginObject(objectId)
            // 销毁预览放置的元素
            disposeVirtualObject({
              parent: this.editHelper.container,
              hasGroup: true,
            })
            root3d.core.render()
            break
          }
          // 地面
          case 'ground':
            // 鼠标左键点击 ground，取消当前的选中状态
            interactionStore.unselectExceptSomeone()
            break
          // 路口辅助蒙层
          case 'junctionMask': {
            const sensorData = interactionStore.currentSensor
            if (
              !sensorData ||
              sensorData.name !== 'RSU' ||
              !interactionStore.canSelectJunction
            ) {
              return
            }
            // @ts-expect-error
            const { junctionId = '' } = selected.object
            const junctionStore = useJunctionStore()
            // 判断当前的路口是否在有效的路口元素中
            if (!junctionStore.ids.includes(junctionId)) return

            // 获取当前已经选中的关联路口
            const { JunctionIDs } = sensorData.deviceParams
            const objectStore = useObjectStore()
            const newJunctionIds = [...JunctionIDs]
            const index = newJunctionIds.indexOf(junctionId)
            if (index > -1) {
              // 如果选中的路口已经在关联的路口中了，则解除跟它的关联
              newJunctionIds.splice(index, 1)
            } else {
              // 否则将其添加到关联的路口集合中
              newJunctionIds.push(junctionId)
            }

            objectStore.updateSensorDeviceParams({
              objectId: sensorData.id,
              deviceParams: {
                ...sensorData.deviceParams,
                JunctionIDs: newJunctionIds,
              },
              isUpdateRsuJunctionIds: true,
            })

            break
          }
          default:
            break
        }
      },
      hoverOn: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { elementName, elementUuid } = options.object
        if (!elementName || !elementUuid) return
        const interactionStore = useObjectInteractionStore()
        const junctionStore = useJunctionStore()
        if (interactionStore.canSelectJunction) {
          // 在能够选择 rsu 关联路口的模式下，hover 路口展示对应的高亮效果
          if (elementName === 'junctionMask') {
            // 展示路口蒙层的 hover 效果
            const targetObject = root3d.getObjectByUuid(elementUuid)
            if (targetObject instanceof Mesh) {
              // @ts-expect-error
              const { junctionId } = targetObject
              // 如果 hover 的路口不在备选的集合中，则不响应高亮
              if (!junctionStore.ids.includes(junctionId)) return

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
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (!targetObject) return
          switch (elementName) {
            case 'roadSign':
            case 'customRoadSign':
              updateRoadSignColor({
                object: targetObject,
                status: 'hovered',
              })
              this.render()
              break
            case 'signalBoard':
            case 'trafficLight':
            case 'sensor': {
              // 杆上的物体，使用同一套的高亮交互效果
              updateObjectOnPole({
                type: elementName,
                object: targetObject,
                status: 'hovered',
              })
              this.render()
              break
            }
            case 'parkingSpace':
              // 停车位相较于常规的路面标线，有单独的更新交互效果的逻辑
              updateParkingSpaceColor({
                object: targetObject,
                status: 'hovered',
              })
              this.render()
              break
            case 'verticalPoleTop':
            case 'horizontalPoleTop':
            case 'horizontalPole':
            case 'verticalPole': {
              updatePoleColor({
                object: targetObject,
                status: 'hovered',
              })
              this.render()
              break
            }
            case 'other':
              updateOtherObject({
                object: targetObject,
                status: 'hovered',
              })
              this.render()
              break
            case 'customModel':
              updateCustomModel({
                object: targetObject,
                status: 'hovered',
              })
              this.render()
              break
            default:
              break
          }
        }
      },
      hoverOff: (options: common.IDragOptions) => {
        if (!this.enabled) return

        const { elementName, elementUuid } = options.object
        if (!elementName || !elementUuid) return
        const interactionStore = useObjectInteractionStore()
        const junctionStore = useJunctionStore()
        if (interactionStore.canSelectJunction) {
          if (elementName === 'junctionMask') {
            // 取消路口蒙层的 hover 效果
            const targetObject = root3d.getObjectByUuid(elementUuid)
            if (targetObject instanceof Mesh) {
              // @ts-expect-error
              const { junctionId } = targetObject
              // 如果 hover 的路口不在备选的集合中，则不响应
              if (!junctionStore.ids.includes(junctionId)) return

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
          const targetObject = root3d.getObjectByUuid(elementUuid)
          if (!targetObject) return
          switch (elementName) {
            case 'roadSign':
            case 'customRoadSign':
              updateRoadSignColor({
                object: targetObject,
                status: 'normal',
              })
              this.render()
              break
            case 'signalBoard':
            case 'trafficLight':
            case 'sensor': {
              // 杆上的物体，使用同一套的高亮交互效果
              updateObjectOnPole({
                type: elementName,
                object: targetObject,
                status: 'normal',
              })
              this.render()
              break
            }
            case 'parkingSpace':
              updateParkingSpaceColor({
                object: targetObject,
                status: 'normal',
              })
              this.render()
              break
            case 'verticalPoleTop':
            case 'horizontalPoleTop':
            case 'horizontalPole':
            case 'verticalPole': {
              updatePoleColor({
                object: targetObject,
                status: 'normal',
              })
              this.render()
              break
            }
            case 'other':
              updateOtherObject({
                object: targetObject,
                status: 'normal',
              })
              this.render()
              break
            case 'customModel':
              updateCustomModel({
                object: targetObject,
                status: 'normal',
              })
              this.render()
              break
            default:
              break
          }
        }
      },
    }
  }

  init (params: IInitPluginParams) {
    this.render = params.render
    this.scene = params.scene

    this.editHelper = new EditObjectHelper({
      render: this.render,
    })
    this.editHelper.init()
    this.scene.add(this.editHelper.container)

    this.initDispatchers()

    this.addDragListener()

    // 注册可 hover 交互的元素
    registryHoverElement({
      pluginName: this.config.name,
      // 响应 hover 效果的元素 name
      objectName: [
        'junctionMask', // 路口蒙层
        'roadSign', // 常规路面标识
        'customRoadSign', // 自定义的路面标识
        'parkingSpace', // 停车位
        'signalBoard', // 标志牌
        'trafficLight', // 信号灯
        'sensor', // 传感器
        'other', // 其他类型
        'customModel', // 自定义导入模型
        // 杆
        'verticalPoleTop',
        'horizontalPoleTop',
        'verticalPole',
        'horizontalPole',
      ],
    })

    // 注册外层有 Group 容器的元素
    registryHasPoleGroupElements([
      'signalBoard',
      'trafficLight',
      'sensor',
      'verticalPole',
      'horizontalPole',
    ])

    // 忽略拖拽停车位过程中的 offset 计算
    registryIgnoreDragOffsetElements(['parkingSpace', 'customRoadSign'])
  }

  // -------------------- 自定义导入的模型 start --------------------
  dragPreviewCustomModel (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return

    // @ts-expect-error
    const { objectId, roadId = '', junctionId = '' } = selected.object
    const parent = this.editHelper.container

    // 获取源数据
    const modelData = getObject(objectId)
    if (!modelData) return
    const modelCacheStore = useModelCacheStore()
    const modelConfig = modelCacheStore.getCustomModelConfigByName(
      modelData.name,
    )
    if (!modelConfig) return

    // 拖拽过程中，隐藏原始的实际物体
    hideOriginObject(objectId)

    // 判断当前物体移动到的位置，是否处于有效的放置区域
    const validRes = inValidArea({
      objects: root3d.proxyScene.children,
      camera: root3d.core.mainCamera,
      point: newPosition,
      roadId,
      junctionId,
    })

    if (!validRes) return

    const {
      roadId: targetRoadId = '',
      junctionId: targetJunctionId = '',
      point: validPosition = null,
    } = validRes

    if (roadId && targetRoadId) {
      const locationData = getLocationInRoad({
        roadId: targetRoadId,
        point: validPosition || newPosition,
        useElevation: true,
      })
      if (!locationData) return

      // 渲染虚拟放置的物体
      renderVirtualCustomModel({
        objectId,
        parent,
        position: validPosition || newPosition,
        // roadId 跟 refLineLocation 同时传入
        roadId: targetRoadId,
        refLineLocation: locationData,
      })
    } else if (junctionId && targetJunctionId) {
      // 在路口中移动虚拟的自定义模型物体
      renderVirtualCustomModel({
        objectId,
        parent,
        position: validPosition || newPosition,
      })
    }

    root3d.core.render()
  }

  dragEndCustomModel (options: common.IDragOptions) {
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object
    const objectData = getObject(objectId)
    if (!objectData) return
    const objectStore = useObjectStore()
    const parent = this.editHelper.container

    // 在完成拖拽后，恢复原始的物体显示状态
    showOriginObject(objectId)

    // 从三维虚拟辅助放置的元素中取最后放置的坐标属性
    const virtualCustomModel = parent.getObjectByName('virtualCustomModel')
    if (!virtualCustomModel) return

    // @ts-expect-error
    const { refLineLocation, roadId = '' } = virtualCustomModel
    const { position: lastPosition } = virtualCustomModel

    // 是否手动触发更新
    let needRender = false
    if (roadId) {
      if (objectData.roadId === roadId) {
        // 在道路中
        objectStore.moveCustomModel({
          objectId,
          point: lastPosition,
          roadId,
          refLineLocation,
        })
      } else {
        // 暂不支持跨道路移动自定义模型
        warningMessage({
          content: i18n.global.t(
            'desc.tips.notSupportedMoveCustomModelAcrossTheRoad',
          ),
        })
        needRender = true

        // 由于原始物体的位置未更新，但需要将包围盒外框的位置重置
        updateBox3HelperById(objectId)
      }
    } else {
      // 路口中
      objectStore.moveCustomModel({
        objectId,
        point: lastPosition,
      })
    }

    // 销毁预览放置的元素
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })

    if (needRender) {
      root3d.core.render()
    }
  }

  // -------------------- 自定义导入的模型 end --------------------
  // -------------------- 通过 geoAttrs 创建的特殊路面标线 start --------------------
  dragPreviewCustomRoadSign (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId, roadId, sectionId, laneId, junctionId } = selected.object
    const parent = this.editHelper.container

    // 获取源数据
    const roadSignData = getObject(objectId)
    if (!roadSignData) return
    const roadSignConfig = getRoadSignConfig(roadSignData.name)
    if (!roadSignConfig) return

    const validRes = inValidArea({
      objects: root3d.proxyScene.children,
      camera: root3d.core.mainCamera,
      point: newPosition,
      roadId,
      junctionId,
    })
    if (!validRes) return

    // 先销毁旧的辅助元素
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })
    // 在拖拽操作过程中，隐藏原始的实际物体
    hideOriginObject(objectId)

    const { name } = roadSignConfig
    let attr
    if (name === 'Road_Guide_Lane_Line') {
      // 导向车道线属性
      attr = getGuideLaneLineAttr({
        roadId,
        sectionId,
        laneId,
        point: newPosition,
        length: roadSignData.length,
      })
    } else if (name === 'Variable_Direction_Lane_Line') {
      // 可变导向车道线属性
      attr = getVariableDirectionLaneLineAttr({
        roadId,
        sectionId,
        laneId,
        point: newPosition,
        length: roadSignData.length,
      })
    } else if (name === 'Longitudinal_Deceleration_Marking') {
      // 车行道纵向减速标线
      attr = getLongitudinalDecelerationAttr({
        roadId,
        sectionId,
        laneId,
        point: newPosition,
        length: roadSignData.length,
      })
    } else if (name === 'Lateral_Deceleration_Marking') {
      // 车行道横向减速标线
      attr = getLateralDecelerationAttr({
        roadId,
        sectionId,
        laneId,
        point: newPosition,
        length: roadSignData.length,
      })
    } else if (name === 'White_Semicircle_Line_Vehicle_Distance_Confirmation') {
      // 白色半圆状车距确认线
      attr = getSemicircleLineAttr({
        roadId,
        sectionId,
        laneId,
        point: newPosition,
        length: roadSignData.length,
      })
    }

    if (!attr) return
    // 预览的几何体属性
    const { geoAttrs } = attr
    renderVirtualCustomRoadSign({
      objectId,
      parent,
      point: newPosition,
      roadId,
      sectionId,
      laneId,
      geoAttrs,
    })

    root3d.core.render()
  }

  dragEndCustomRoadSign (options: common.IDragOptions) {
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object
    const objectStore = useObjectStore()
    // 在完成拖拽后，恢复原始的物体显示状态
    showOriginObject(objectId)
    // 获取源数据
    const roadSignData = getObject(objectId)
    if (!roadSignData) return
    const roadSignConfig = getRoadSignConfig(roadSignData.name)
    if (!roadSignConfig) return
    // 是否是自由移动的路面标识
    const { freeMoveInRoad = false } = roadSignConfig
    const { junctionId = '' } = roadSignData
    const parent = this.editHelper.container
    // 从三维虚拟辅助放置的元素中取最后放置的坐标属性
    const virtualRoadSign = parent.getObjectByName('virtualRoadSign')
    if (!virtualRoadSign) return
    // @ts-expect-error
    const { roadId = '', intersectionPoint } = virtualRoadSign
    // 是否手动触发更新
    const needRender = false
    if (roadId) {
      objectStore.moveCustomRoadSign({
        objectId,
        point: intersectionPoint,
      })
    }
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })
    if (needRender) {
      root3d.core.render()
    }
  }

  // -------------------- 通过 geoAttrs 创建的特殊路面标线 end --------------------
  // -------------------- 停车位 start --------------------
  dragPreviewParkingSpace (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return

    // @ts-expect-error
    const { objectId, roadId, junctionId = '' } = selected.object
    const parent = this.editHelper.container

    const objectData = getObject(objectId) as biz.IParkingSpace
    if (!objectData) return

    // 判断停车位移动的位置，是否处于有效的可放置区域
    const validRes = inValidArea({
      objects: root3d.proxyScene.children,
      camera: root3d.core.mainCamera,
      point: newPosition,
      roadId,
      junctionId,
    })

    if (!validRes) return

    const {
      width,
      length,
      innerAngle,
      lineWidth,
      count,
      margin,
      yaw = 0,
      color,
    } = objectData

    // 由于停车位的网格是通过 vertices 绘制的，在每次渲染新的辅助停车位之前，先销毁旧的辅助停车位
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })
    // 拖拽过程中，隐藏原始的实际物体（注意：需要在销毁方法之后执行）
    hideOriginObject(objectId)

    const {
      roadId: targetRoadId = '',
      junctionId: targetJunctionId = '',
      point: validPosition = null,
    } = validRes

    if (roadId && targetRoadId) {
      const road = getRoad(targetRoadId)
      if (!road) return

      // 计算停车位放置点跟参考线相关的位置属性
      const locationRes = getLocationInRoad({
        point: validPosition || newPosition,
        roadId: targetRoadId,
        useElevation: true,
      })
      if (!locationRes) return

      // 虚线的限时停车位
      const isDashed = objectData.name === 'Time_Limit_Parking_Space_Mark'

      // 拖拽过程中实时调用同步的方法计算一个停车位的坐标（通过webWorker线程可能通信本身就存在延迟）
      const payload = {
        curvePathPoints: road.keyPath.points,
        refLineLocation: locationRes,
        count,
        width,
        length,
        margin,
        lineWidth,
        yaw,
        innerAngle,
        isDashed,
      }
      if (road.elevationPath) {
        payload.elevationPathPoints = road.elevationPath.points
      }
      // 渲染预览停车位，只需要顶点信息即可
      const { geoAttrs } = getCornerVerticesAttr(payload)
      if (!geoAttrs || geoAttrs.length < 1) return

      // 渲染虚拟放置的停车位
      renderVirtualParkingSpace({
        objectId,
        color,
        parent,
        geoAttrs,
        roadId: targetRoadId,
        refLineLocation: locationRes,
        intersectionPoint: validPosition || newPosition,
      })
    } else if (junctionId && targetJunctionId) {
      // 暂时先不支持路口中的停车位
    }

    root3d.core.render()
  }

  dragEndParkingSpace (options: common.IDragOptions) {
    const selected = getSelected()
    if (!selected) return

    // @ts-expect-error
    const { objectId } = selected.object
    const parent = this.editHelper.container
    const objectStore = useObjectStore()

    const objectData = getObject(objectId) as biz.IParkingSpace
    if (!objectData) return

    // 完成拖拽后，恢复原始物体的显示状态
    showOriginObject(objectId)

    // 从三维虚拟放置的元素中获取最后放置的坐标属性
    const virtualParkingSpace = parent.getObjectByName('virtualParkingSpace')
    const {
      refLineLocation,
      roadId = '',
      intersectionPoint,
    } = virtualParkingSpace

    // 是否手动触发更新
    let needRender = false
    if (roadId) {
      // 道路上
      if (objectData.roadId === roadId) {
        objectStore.moveParkingSpace({
          objectId,
          point: intersectionPoint,
          roadId,
          refLineLocation,
        })
      } else {
        // 暂不支持跨道路放置停车位
        warningMessage({
          content: i18n.global.t(
            'desc.tips.notSupportedMoveParkingSpaceAcrossTheRoad',
          ),
        })
        needRender = true
      }
    }

    // 销毁预览放置的元素
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })

    if (needRender) {
      root3d.core.render()
    }
  }

  // -------------------- 停车位 end --------------------
  // -------------------- 其他类型 start --------------------
  dragPreviewOther (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return

    // @ts-expect-error
    const { objectId, roadId = '', junctionId = '' } = selected.object
    const parent = this.editHelper.container

    // 获取源数据
    const otherData = getObject(objectId)
    if (!otherData) return
    const otherConfig = getOtherConfig(otherData.name)
    if (!otherConfig) return

    // 拖拽过程中，隐藏原始的实际物体
    hideOriginObject(objectId)

    // 判断当前物体移动到的位置，是否处于有效的放置区域
    const validRes = inValidArea({
      objects: root3d.proxyScene.children,
      camera: root3d.core.mainCamera,
      point: newPosition,
      roadId,
      junctionId,
    })

    if (!validRes) return

    const {
      roadId: targetRoadId = '',
      junctionId: targetJunctionId = '',
      point: validPosition = null,
    } = validRes

    if (roadId && targetRoadId) {
      const locationData = getLocationInRoad({
        roadId: targetRoadId,
        point: validPosition || newPosition,
        useElevation: true,
      })
      if (!locationData) return

      // 渲染虚拟放置的物体
      renderVirtualOther({
        objectId,
        parent,
        position: validPosition || newPosition,
        // roadId 跟 refLineLocation 同时传入
        roadId: targetRoadId,
        refLineLocation: locationData,
      })
    } else if (junctionId && targetJunctionId) {
      // 在路口中移动虚拟的其他物体
      renderVirtualOther({
        objectId,
        parent,
        position: validPosition || newPosition,
      })
    }
    root3d.core.render()
  }

  dragEndOther (options: common.IDragOptions) {
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object
    const objectData = getObject(objectId)
    if (!objectData) return
    const objectStore = useObjectStore()
    const parent = this.editHelper.container

    // 在完成拖拽后，恢复原始的物体显示状态
    showOriginObject(objectId)

    // 从三维虚拟辅助放置的元素中取最后放置的坐标属性
    const virtualOther = parent.getObjectByName('virtualOther')
    if (!virtualOther) return

    // @ts-expect-error
    const { refLineLocation, roadId = '' } = virtualOther
    const { position: lastPosition } = virtualOther

    // 是否手动触发更新
    let needRender = false
    if (roadId) {
      if (objectData.roadId === roadId) {
        // 在道路中
        objectStore.moveOther({
          objectId,
          point: lastPosition,
          roadId,
          refLineLocation,
        })
      } else {
        // 暂不支持跨道路移动其他物体
        warningMessage({
          content: i18n.global.t(
            'desc.tips.notSupportedMoveOtherTypeObjectAcrossTheRoad',
          ),
        })
        needRender = true

        // 由于原始物体的位置未更新，但需要将包围盒外框的位置重置
        updateBox3HelperById(objectId)
      }
    } else {
      // 路口中
      objectStore.moveOther({
        objectId,
        point: lastPosition,
      })
    }

    // 销毁预览放置的元素
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })

    if (needRender) {
      root3d.core.render()
    }
  }

  // -------------------- 其他类型 end --------------------
  // -------------------- 路面标识 start --------------------
  // 拖拽预览的路面标识
  dragPreviewRoadSign (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId, roadId, sectionId, laneId, junctionId } = selected.object
    const parent = this.editHelper.container

    // 获取源数据
    const roadSignData = getObject(objectId)
    if (!roadSignData) return
    const roadSignConfig = getRoadSignConfig(roadSignData.name)
    if (!roadSignConfig) return

    // 是否是自由移动的路面标识
    const { freeMoveInRoad = false } = roadSignConfig

    // 在拖拽操作过程中，隐藏原始的实际物体
    hideOriginObject(objectId)

    const { name } = roadSignConfig
    // 如果是人行横道线，可以将其移动到道路首尾延长线一定范围处，即路口中，但任归属于道路
    if (name === 'Crosswalk_Line') {
      const targetRoadId = roadSignData.roadId
      // 计算当前位置在对应道路参考线的相对 st 坐标（s值可超出[0, length]之外的一定范围）
      const crosswalkAttr = getCrosswalkAttrOnRoad({
        objectId,
        roadId: targetRoadId,
        point: newPosition,
        crossBorder: true,
      })
      if (!crosswalkAttr) return
      const {
        length,
        s,
        t,
        position,
        projectNormal,
        closestPoint,
        closestPointPercent,
        closestPointTangent,
      } = crosswalkAttr
      const locationData = {
        s,
        t,
        percent: closestPointPercent,
        tangent: closestPointTangent,
        closestPointOnRefLine: closestPoint,
        closestPointInLane: position,
        normal: projectNormal,
      }
      // 渲染预览的人行横道辅助元素
      renderVirtualCrosswalk({
        objectId,
        parent,
        point: position || newPosition,
        roadId: targetRoadId,
        refLineLocation: locationData,
        length,
      })
      // 在符合预期的交互位置
      root3d.core.render()
      return
    } else if (name === 'Crosswalk_with_Left_and_Right_Side') {
      const targetRoadId = roadSignData.roadId
      // 计算当前位置在对应道路参考线的相对 st 坐标（s值可超出[0, length]之外的一定范围）
      const crosswalkAttr = getCrosswalkWithSideAttrOnRoad({
        objectId,
        roadId: targetRoadId,
        point: newPosition,
        crossBorder: true,
      })
      if (!crosswalkAttr) return
      const {
        length,
        s,
        t,
        position,
        projectNormal,
        closestPoint,
        closestPointPercent,
        closestPointTangent,
      } = crosswalkAttr
      const locationData = {
        s,
        t,
        percent: closestPointPercent,
        tangent: closestPointTangent,
        closestPointOnRefLine: closestPoint,
        closestPointInLane: position,
        normal: projectNormal,
      }
      // 渲染预览的人行横道辅助元素
      renderVirtualCrosswalkWithSide({
        objectId,
        parent,
        point: position || newPosition,
        roadId: targetRoadId,
        refLineLocation: locationData,
        length,
      })
      // 在符合预期的交互位置
      root3d.core.render()
      return
    } else if (name === 'Stop_Line') {
      // 拖拽过程中，判断当前 section 的长度，自动调整尺寸
      // 判断当前物体移动到的位置，是否处于有效的放置区域
      const validRes = inValidArea({
        objects: root3d.proxyScene.children,
        camera: root3d.core.mainCamera,
        point: newPosition,
        roadId,
        junctionId,
      })
      if (!validRes) return

      const { roadId: targetRoadId } = validRes
      if (!targetRoadId) return
      const stopLineAttr = getStopLineAttrOnRoad({
        roadId: targetRoadId,
        point: newPosition,
      })
      if (!stopLineAttr) return
      const {
        length,
        s,
        t,
        position,
        projectNormal,
        closestPoint,
        closestPointPercent,
        closestPointTangent,
      } = stopLineAttr
      const locationData = {
        s,
        t,
        percent: closestPointPercent,
        tangent: closestPointTangent,
        closestPointOnRefLine: closestPoint,
        closestPointInLane: position,
        normal: projectNormal,
      }
      renderVirtualStopLine({
        objectId,
        parent,
        point: position || newPosition,
        roadId: targetRoadId,
        refLineLocation: locationData,
        length,
      })

      root3d.core.render()
      return
    }

    if (freeMoveInRoad && roadId) {
      // 判断当前物体移动到的位置，是否处于有效的放置区域
      const validRes = inValidArea({
        objects: root3d.proxyScene.children,
        camera: root3d.core.mainCamera,
        point: newPosition,
        roadId,
        junctionId,
      })

      if (!validRes) return

      const { roadId: targetRoadId = '', point: validPosition = null } =
        validRes
      if (targetRoadId) {
        // 在道路上自由移动
        const locationData = getLocationInRoad({
          point: validPosition || newPosition,
          roadId: targetRoadId,
          useElevation: true,
        })
        if (!locationData) return

        // 道路上自由放置的路面标识（需要准确的世界坐标）
        // 在拖拽预览放置时可以移动到其他道路上（需要鼠标射线指向的道路 id）
        renderVirtualRoadSign({
          objectId,
          parent,
          point: validPosition || newPosition,
          roadId: targetRoadId,
          refLineLocation: locationData,
        })
      }
    } else if (!freeMoveInRoad && roadId && sectionId && laneId) {
      // 计算在车道上移动的预放置位置
      const locationData = calcRoadSignLocationInRoad({
        roadId,
        sectionId,
        laneId,
        point: newPosition,
      })
      if (!locationData) return
      // 车道上渲染预览的虚拟放置元素（不需要准确的世界坐标）
      // 限制于当前道路当前车道（不需要传 roadId）
      renderVirtualRoadSign({
        objectId,
        parent,
        refLineLocation: locationData,
      })
    } else if (junctionId) {
      // 计算在路口中移动的预览位置

      // 由于路口中的路面标识是可自由移动，因此需要判断是否处于有效的可放置区域
      const validRes = inValidArea({
        objects: root3d.proxyScene.children,
        camera: root3d.core.mainCamera,
        point: newPosition,
        junctionId,
        junctionAreaName: ['junction'],
      })
      // 如果放置的位置无效，则返回不更新
      if (!validRes) return

      // 路口中的路面标识，暂时没有相对于参考线的相对属性
      renderVirtualRoadSign({
        objectId,
        parent,
        point: validRes.point || newPosition,
      })
    }

    root3d.core.render()
  }

  // 拖拽结束放置路面标识
  dragEndRoadSign (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object
    const objectStore = useObjectStore()

    // 在完成拖拽后，恢复原始的物体显示状态
    showOriginObject(objectId)

    // 获取源数据
    const roadSignData = getObject(objectId)
    if (!roadSignData) return
    const roadSignConfig = getRoadSignConfig(roadSignData.name)
    if (!roadSignConfig) return

    // 是否是自由移动的路面标识
    const { freeMoveInRoad = false } = roadSignConfig

    const { junctionId = '' } = roadSignData
    const parent = this.editHelper.container

    // 从三维虚拟辅助放置的元素中取最后放置的坐标属性
    const virtualRoadSign = parent.getObjectByName('virtualRoadSign')
    if (!virtualRoadSign) return

    // @ts-expect-error
    const { refLineLocation, roadId = '' } = virtualRoadSign
    const { position: lastPosition } = virtualRoadSign

    // 是否手动触发更新
    let needRender = false

    if (junctionId) {
      // 放置在路口中
      objectStore.moveRoadSign({
        objectId,
        point: lastPosition,
      })
    } else {
      // 放置在道路中
      if (!freeMoveInRoad) {
        // 常规的放置在车道上的路面标识
        // 基于射线交互的位置重新算最后的坐标
        if (roadSignConfig.name === 'Stop_Line') {
          // 停止线
          if (roadSignData.roadId === roadId) {
            objectStore.moveRoadSign({
              objectId,
              point: lastPosition,
            })
          } else {
            // 暂不支持跨道路移动可自由移动的路面标识
            warningMessage({
              content: i18n.global.t(
                'desc.tips.notSupportedMoveRoadSignAcrossTheRoad',
              ),
            })
            needRender = true

            // 由于原始物体的位置未更新，但需要将包围盒外框的位置重置
            updateBox3HelperById(objectId)
          }
        } else if (
          roadSignConfig.name === 'Crosswalk_Line' ||
          roadSignConfig.name === 'Crosswalk_with_Left_and_Right_Side'
        ) {
          // 人行横道，不需要判断 roadId 是否相符
          objectStore.moveRoadSign({
            objectId,
            // 由于移动范围存在限制，不能使用 newPosition
            point: lastPosition,
          })
        } else {
          objectStore.moveRoadSign({
            objectId,
            point: newPosition,
          })
        }
      } else {
        // 由于是自由移动，判断当前目标道路是否跟原有归属道路一致
        if (roadSignData.roadId === roadId) {
          // 在同一条道路中
          objectStore.moveRoadSign({
            objectId,
            point: lastPosition,
            roadId,
            refLineLocation,
          })
        } else {
          // 暂不支持跨道路移动可自由移动的路面标识
          warningMessage({
            content: i18n.global.t(
              'desc.tips.notSupportedMoveRoadSignAcrossTheRoad',
            ),
          })
          needRender = true

          // 由于原始物体的位置未更新，但需要将包围盒外框的位置重置
          updateBox3HelperById(objectId)
        }
      }
    }

    // 销毁预览放置的路面标识
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })

    if (needRender) {
      root3d.core.render()
    }
  }

  // -------------------- 路面标识 end --------------------
  // -------------------- 杆 start --------------------
  dragPreviewPole (options: common.IDragOptions) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object
    const poleData = getObject(objectId) as biz.IPole
    if (!poleData) return
    const { roadId = '', junctionId = '' } = poleData
    const parent = this.editHelper.container

    // 拖拽过程中，隐藏原始的实际物体
    hideOriginObject(objectId)

    // 判断杆在移动到当前位置，是否处于有效得可放置区域
    const validRes = inValidArea({
      objects: root3d.proxyScene.children,
      camera: root3d.core.mainCamera,
      point: newPosition,
      roadId,
      junctionId,
    })

    if (!validRes) return

    const {
      roadId: targetRoadId = '',
      junctionId: targetJunctionId = '',
      point: validPosition = null,
    } = validRes

    if (roadId && targetRoadId) {
      // 计算杆在参考线坐标系下的相对关系
      const locationData = getLocationInRoad({
        roadId: targetRoadId,
        point: validPosition || newPosition,
        useElevation: true,
      })
      if (!locationData) return

      // 渲染虚拟放置的杆
      renderVirtualPole({
        objectId,
        parent,
        position: validPosition || newPosition,
        // roadId 跟 refLineLocation 同时传入
        roadId: targetRoadId,
        refLineLocation: locationData,
      })
    } else if (junctionId && targetJunctionId) {
      // 在路口中移动杆
      renderVirtualPole({
        objectId,
        parent,
        position: validPosition || newPosition,
      })
    }
    root3d.core.render()
  }

  // 拖拽结束放置杆
  dragEndPole (options: common.IDragOptions) {
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object
    const objectData = getObject(objectId)
    if (!objectData) return
    const objectStore = useObjectStore()
    const parent = this.editHelper.container

    // 在完成拖拽后，恢复原始的物体显示状态
    showOriginObject(objectId)

    // 从三维虚拟放置的元素中取最后放置的坐标属性
    const virtualPole = parent.getObjectByName('virtualPole')

    if (!virtualPole) return

    // @ts-expect-error
    const { refLineLocation, roadId = '' } = virtualPole
    const { position: lastPosition } = virtualPole

    // 是否手动触发更新
    let needRender = false
    // 将上一次的位置信息传入
    if (roadId) {
      if (objectData.roadId === roadId) {
        // 道路中
        objectStore.movePole({
          objectId,
          point: lastPosition,
          roadId,
          refLineLocation,
        })
      } else {
        // 暂不支持跨道路放置杆
        warningMessage({
          content: i18n.global.t('desc.tips.notSupportedMovePoleAcrossTheRoad'),
        })
        needRender = true

        // 由于原始物体的位置未更新，但需要将包围盒外框的位置重置
        updateBox3HelperById(objectId)
      }
    } else {
      // 路口中
      objectStore.movePole({
        objectId,
        point: lastPosition,
      })
    }

    // 销毁预览放置的元素
    disposeVirtualObject({
      parent,
      hasGroup: true,
    })

    if (needRender) {
      root3d.core.render()
    }
  }

  // -------------------- 杆 end --------------------
  // -------------------- 杆上物体（信号灯、标志牌、传感器）start --------------------
  dragPreviewObjectOnPole (
    options: common.IDragOptions,
    type: 'trafficLight' | 'signalBoard' | 'sensor',
  ) {
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object
    const objectData = getObject(objectId) as
      | biz.ITrafficLight
      | biz.ISignalBoard
      | biz.ISensor
    if (!objectData) return
    const { poleId, onVerticalPole } = objectData
    if (!poleId) return
    // 获取所在杆容器的三维对象
    const poleGroup = root3d.mapElementsContainer.getObjectByProperty(
      'objectId',
      poleId,
    )
    if (!poleGroup) return
    // 获取物体外层的容器
    const objectGroup = poleGroup.getObjectByProperty('objectId', objectId)
    if (!objectGroup) return

    const parent = this.editHelper.container
    // 拖拽过程中，隐藏原始的实际物体
    hideOriginObject(objectId)

    let position: Vector3
    if (onVerticalPole) {
      const _point = calcPositionOnVerticalPole({ poleId, point: newPosition })
      if (!_point) return
      // 竖杆只取高度
      position = new Vector3(
        objectGroup.position.x,
        _point.y,
        objectGroup.position.z,
      )
    } else {
      const _point = calcPositionOnHorizontalPole({
        poleId,
        point: newPosition,
      })
      if (!_point) return
      // 横杆只取到竖杆的水平距离
      position = new Vector3(
        objectGroup.position.x,
        objectGroup.position.y,
        _point.z,
      )
    }

    // 将坐标转换成杆容器所处坐标系的坐标
    position.applyMatrix4(poleGroup.matrix)
    const quaternion = new Quaternion()
    // 物体外层容器在世界坐标系下的旋转四元数
    objectGroup.getWorldQuaternion(quaternion)

    // 预览虚拟物体的名称，便于后续查找
    let virtualName = ''
    if (type === 'trafficLight') {
      virtualName = 'virtualTrafficLight'
    } else if (type === 'signalBoard') {
      virtualName = 'virtualSignalBoard'
    } else if (type === 'sensor') {
      virtualName = 'virtualSensor'
    }

    // 渲染预览的虚拟放置元素
    renderVirtualElementOnPole({
      objectId,
      parent,
      // 此处的位置和旋转四元数，均已转换为绝对坐标系下
      position,
      quaternion,
      name: virtualName,
    })

    root3d.core.render()
  }

  // 拖拽结束放置杆上的物体
  dragEndObjectOnPole (
    options: common.IDragOptions,
    type: 'trafficLight' | 'signalBoard' | 'sensor',
  ) {
    // newPosition 为拖拽过程中，鼠标射线跟基准平面的交点
    const { newPosition } = options.object
    const selected = getSelected()
    if (!selected) return
    // @ts-expect-error
    const { objectId } = selected.object

    // 在完成拖拽后，恢复原始的物体显示状态
    showOriginObject(objectId)

    const objectStore = useObjectStore()
    switch (type) {
      case 'trafficLight':
        objectStore.moveTrafficLight({
          objectId,
          point: newPosition,
        })
        break
      case 'signalBoard':
        objectStore.moveSignalBoard({
          objectId,
          point: newPosition,
        })
        break
      case 'sensor':
        objectStore.moveSensor({
          objectId,
          point: newPosition,
        })
        break
      default:
        break
    }

    // 销毁预览放置的元素
    disposeVirtualObject({
      parent: this.editHelper.container,
      hasGroup: true,
    })
  }
  // -------------------- 杆上物体（信号灯、标志牌、传感器）end --------------------

  initDispatchers () {
    const objectStore = useObjectStore()
    const interactionStore = useObjectInteractionStore()
    const junctionStore = useJunctionStore()
    const modelCacheStore = useModelCacheStore()
    const parent = this.editHelper.container
    // 装地图元素的容器
    const rootContainer = root3d.mapElementsContainer

    this.unsubscribeInteractionStore = interactionStore.$onAction(
      ({ name, store, args, after, onError }) => {
        // 获取上一个状态 rsu 关联路口的数据
        let lastRsu: biz.ISensor
        const { currentSensor } = store
        if (currentSensor && currentSensor.name === 'RSU') {
          if (name === 'unselectSensor' || name === 'selectSensor') {
            lastRsu = currentSensor
          }
        }

        after((res) => {
          switch (name) {
            case 'applyState': {
              const [currentState] = args
              const {
                poleId,
                roadSignId,
                signalBoardId,
                trafficLightId,
                sensorId,
                otherId,
                customModelId,
              } = currentState

              // 在撤销重做时，无论之前的状态是否有选中的元素，都先销毁原有的外围盒外框
              disposeBox3Helper()

              if (poleId) {
                store.selectPole(poleId)
              } else if (roadSignId) {
                store.selectRoadSign(roadSignId)
              } else if (signalBoardId) {
                store.selectSignalBoard(signalBoardId)
              } else if (trafficLightId) {
                store.selectTrafficLight(trafficLightId)
              } else if (sensorId) {
                store.selectSensor(sensorId)
              } else if (otherId) {
                store.selectOther(otherId)
              } else if (customModelId) {
                store.selectCustomModel(customModelId)
              }

              store.updateTimestamp()

              // 隐藏关联路口的辅助元素
              setAllTextSpriteVisible({
                visible: false,
                parent,
              })
              // 取消路口的高亮
              updateJunctionMaskColor({
                status: 'normal',
                junctionId: junctionStore.ids,
                parent,
              })
              this.render()

              break
            }
            case 'selectPole': {
              const lastPoleId = res as string
              const { poleId } = store
              if (lastPoleId && lastPoleId !== poleId) {
                // 如果存在上一次的杆，则取消对应杆的高亮交互
                updatePoleColorById({
                  objectId: lastPoleId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
              }
              if (poleId) {
                updatePoleColorById({
                  objectId: poleId,
                  parent: rootContainer,
                  status: 'selected',
                  isForced: true,
                })
                // 杆的包围盒外框
                updateBox3HelperById(poleId)
              }
              this.render()
              break
            }
            case 'unselectPole': {
              const lastPoleId = res as string
              if (lastPoleId) {
                updatePoleColorById({
                  objectId: lastPoleId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
                // 销毁包围盒外框
                disposeBox3Helper()
                this.render()
              }
              break
            }
            case 'selectRoadSign': {
              const lastRoadSignId = res as string
              const { roadSignId } = store

              if (lastRoadSignId && lastRoadSignId !== roadSignId) {
                // 如果存在上一次的路面标线，则取消对应的高亮效果
                // 判断路面标线是否是停车位
                const lastRoadSignData = getObject(lastRoadSignId)
                if (
                  lastRoadSignData &&
                  lastRoadSignData.mainType === 'parkingSpace'
                ) {
                  // 停车位
                  updateParkingSpaceColorById({
                    objectId: lastRoadSignId,
                    parent: rootContainer,
                    status: 'normal',
                    isForced: true,
                  })
                } else {
                  updateRoadSignColorById({
                    objectId: lastRoadSignId,
                    parent: rootContainer,
                    status: 'normal',
                    isForced: true,
                  })
                }
              }

              if (roadSignId) {
                // 选中路面标识设置高亮的颜色
                // 判断是否是停车位
                const currentRoadSign = getObject(roadSignId)
                if (
                  currentRoadSign &&
                  currentRoadSign.mainType === 'parkingSpace'
                ) {
                  // 停车位
                  updateParkingSpaceColorById({
                    objectId: roadSignId,
                    parent: rootContainer,
                    status: 'selected',
                    isForced: true,
                  })
                  // 如果是选中停车位，需要主动将高亮包围盒外框删除
                  disposeBox3Helper()
                } else {
                  updateRoadSignColorById({
                    objectId: roadSignId,
                    parent: rootContainer,
                    status: 'selected',
                    isForced: true,
                  })

                  // 展示路面标线的包围盒外框
                  updateBox3HelperById(roadSignId)
                }
              }

              this.render()

              break
            }
            case 'unselectRoadSign': {
              const lastRoadSignId = res as string
              if (lastRoadSignId) {
                // 判断当前取消选中的路面标线，是否是停车位
                const roadSignData = getObject(lastRoadSignId)
                if (roadSignData && roadSignData.mainType === 'parkingSpace') {
                  // 如果是停车位
                  updateParkingSpaceColorById({
                    objectId: lastRoadSignId,
                    parent: rootContainer,
                    status: 'normal',
                    isForced: true,
                  })
                } else {
                  updateRoadSignColorById({
                    objectId: lastRoadSignId,
                    parent: rootContainer,
                    status: 'normal',
                    isForced: true,
                  })
                  // 取消选中的时候，销毁之前包围盒外框
                  disposeBox3Helper()
                }
                this.render()
              }
              break
            }
            case 'selectSensor': {
              const lastSensorId = res as string
              const { currentSensor, sensorId } = store
              if (lastRsu) {
                // 如果选中一个传感器之前，上一个传感器是通信单元，则需要取消对应的高亮元素选中效果
                setAllTextSpriteVisible({
                  visible: false,
                  parent,
                })

                // 取消路口的高亮
                updateJunctionMaskColor({
                  status: 'normal',
                  junctionId: lastRsu.deviceParams.JunctionIDs,
                  parent,
                })
              }
              if (currentSensor && currentSensor.name === 'RSU') {
                // 如果是通信单元，选中后展示对应关联的路口

                // 控制路口文字提示的显示
                setAllTextSpriteVisible({
                  visible: true,
                  parent,
                })

                // 高亮展示对应的路口
                updateJunctionMaskColor({
                  status: 'selected',
                  junctionId: currentSensor.deviceParams.JunctionIDs,
                  parent,
                })
              }
              if (lastSensorId && lastSensorId !== sensorId) {
                updateObjectOnPoleById({
                  type: 'sensor',
                  objectId: lastSensorId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
              }
              if (sensorId) {
                updateObjectOnPoleById({
                  type: 'sensor',
                  objectId: sensorId,
                  parent: rootContainer,
                  status: 'selected',
                  isForced: true,
                })

                // 展示包围盒外框
                updateBox3HelperById(sensorId)
              }
              this.render()
              break
            }
            case 'unselectSensor': {
              const lastSensorId = res as string
              if (lastRsu) {
                // 如果是取消通信单元的选中，则取消对应关联道路的高亮效果

                // 控制路口文字提示的隐藏
                setAllTextSpriteVisible({
                  visible: false,
                  parent,
                })

                // 取消路口的高亮
                updateJunctionMaskColor({
                  status: 'normal',
                  junctionId: junctionStore.ids,
                  parent,
                })
              }
              if (lastSensorId) {
                updateObjectOnPoleById({
                  type: 'sensor',
                  objectId: lastSensorId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
                // 销毁包围盒外框
                disposeBox3Helper()
              }
              this.render()
              break
            }
            case 'selectSignalBoard': {
              const lastSignalBoardId = res as string
              const { signalBoardId } = store
              if (lastSignalBoardId && lastSignalBoardId !== signalBoardId) {
                // 如果存在上一次的标志牌，则取消对应的交互效果
                updateObjectOnPoleById({
                  type: 'signalBoard',
                  objectId: lastSignalBoardId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
              }
              if (signalBoardId) {
                // 选中的标志牌提供交互效果
                updateObjectOnPoleById({
                  type: 'signalBoard',
                  objectId: signalBoardId,
                  parent: rootContainer,
                  status: 'selected',
                  isForced: true,
                })
                // 展示包围盒外框
                updateBox3HelperById(signalBoardId)
              }
              this.render()
              break
            }
            case 'unselectSignalBoard': {
              const lastSignalBoardId = res as string
              if (lastSignalBoardId) {
                updateObjectOnPoleById({
                  type: 'signalBoard',
                  objectId: lastSignalBoardId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
                disposeBox3Helper()
                this.render()
              }
              break
            }
            case 'selectTrafficLight': {
              const lastTrafficLightId = res as string
              const { trafficLightId } = store
              if (lastTrafficLightId && lastTrafficLightId !== trafficLightId) {
                // 如果存在上一次的信号灯，则取消对应的交互效果
                updateObjectOnPoleById({
                  type: 'trafficLight',
                  objectId: lastTrafficLightId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
              }
              if (trafficLightId) {
                // 选中的信号灯提供交互效果
                updateObjectOnPoleById({
                  type: 'trafficLight',
                  objectId: trafficLightId,
                  parent: rootContainer,
                  status: 'selected',
                  isForced: true,
                })
                // 展示包围盒外框
                updateBox3HelperById(trafficLightId)
              }
              this.render()

              break
            }
            case 'unselectTrafficLight': {
              const lastTrafficLightId = res as string
              if (lastTrafficLightId) {
                updateObjectOnPoleById({
                  type: 'trafficLight',
                  objectId: lastTrafficLightId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
                disposeBox3Helper()
                this.render()
              }
              break
            }
            case 'selectOther': {
              const lastOtherId = res as string
              const { otherId } = store
              if (lastOtherId && lastOtherId !== otherId) {
                updateOtherObjectById({
                  objectId: lastOtherId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
              }
              if (otherId) {
                updateOtherObjectById({
                  objectId: otherId,
                  parent: rootContainer,
                  status: 'selected',
                  isForced: true,
                })
                // 展示包围盒外框
                updateBox3HelperById(otherId)
              }
              this.render()
              break
            }
            case 'unselectOther': {
              const lastOtherId = res as string
              if (lastOtherId) {
                updateOtherObjectById({
                  objectId: lastOtherId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
                disposeBox3Helper()
                this.render()
              }
              break
            }
            // 自定义模型
            case 'selectCustomModel': {
              const latsModelId = res as string
              const { customModelId } = store
              if (latsModelId && latsModelId !== customModelId) {
                updateCustomModelById({
                  objectId: latsModelId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
              }
              if (customModelId) {
                updateCustomModelById({
                  objectId: customModelId,
                  parent: rootContainer,
                  status: 'selected',
                  isForced: true,
                })
                // 展示包围盒外框
                updateBox3HelperById(customModelId)
              }
              this.render()
              break
            }
            case 'unselectCustomModel': {
              const lastModelId = res as string
              if (lastModelId) {
                updateCustomModelById({
                  objectId: lastModelId,
                  parent: rootContainer,
                  status: 'normal',
                  isForced: true,
                })
                disposeBox3Helper()
                this.render()
              }
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

    // 在物体编辑模式中，也订阅一次 objectStore 的事件触发，主要用于实现物体的选中
    this.unsubscribeObjectStore = objectStore.$onAction(
      ({ name, store, args, after, onError }) => {
        let lastJunctionIds = []
        // 如果是更新了 rsu 关联的路口，则需要记录上一个状态控制的路口
        if (name === 'updateSensorDeviceParams') {
          const { isUpdateRsuJunctionIds } = args[0]
          const { currentSensor } = interactionStore
          if (isUpdateRsuJunctionIds && currentSensor) {
            lastJunctionIds = cloneDeep(currentSensor.deviceParams.JunctionIDs)
          }
        }

        after((res) => {
          switch (name) {
            // 应用操作记录
            case 'applyMapFileState':
            case 'applyState':
            case 'movePole': // 杆
            case 'movePoleByST':
            case 'rotatePole':
            case 'removePole':
            case 'moveRoadSign': // 路面标识
            case 'moveCustomRoadSign':
            case 'moveRoadSignByST':
            case 'rotateRoadSign':
            case 'updateRoadSignSize':
            case 'removeRoadSign':
            case 'moveParkingSpace': // 停车位
            case 'moveParkingSpaceByST':
            case 'rotateParkingSpace':
            case 'updateParkingSpaceProperty':
            case 'removeParkingSpace':
            case 'moveSignalBoard': // 标志牌
            case 'moveSignalBoardByHeightAndDist':
            case 'rotateSignalBoard':
            case 'toggleSignalBoardDirection':
            case 'removeSignalBoard':
            case 'moveTrafficLight': // 交通灯
            case 'moveTrafficLightByHeightAndDist':
            case 'rotateTrafficLight':
            case 'toggleTrafficLightDirection':
            case 'removeTrafficLight':
            case 'moveSensor': // 传感器
            case 'moveSensorByHeightAndDist':
            case 'rotateSensor':
            case 'toggleSensorDirection':
            case 'removeSensor':
            case 'moveCustomModel': // 自定义模型
            case 'moveCustomModelByST':
            case 'rotateCustomModel':
            case 'updateCustomModelSize':
            case 'moveOther': // 其他类型
            case 'moveOtherByST':
            case 'updateOtherSize':
            case 'updateBridgeSpan':
            case 'rotateOther':
            case 'removeOther': {
              // 只要更新了实际物体元素的属性，都需要更新一下时间戳，用于手动更新当前选中元素的 getter 属性
              interactionStore.updateTimestamp()
              break
            }
            case 'addPole': {
              const poleId = res as string
              if (!poleId) return
              interactionStore.updateTimestamp()
              interactionStore.selectPole(poleId)
              break
            }
            case 'addRoadSign': {
              const roadSignId = res as string
              if (!roadSignId) return
              interactionStore.updateTimestamp()
              interactionStore.selectRoadSign(roadSignId)
              break
            }
            case 'addTrafficLight': {
              const trafficLightId = res as string
              if (!trafficLightId) return
              interactionStore.updateTimestamp()
              interactionStore.selectTrafficLight(trafficLightId)
              break
            }
            case 'addSignalBoard': {
              const signalBoardId = res as string
              if (!signalBoardId) return
              interactionStore.updateTimestamp()
              interactionStore.selectSignalBoard(signalBoardId)
              break
            }
            case 'addSensor': {
              const sensorId = res as string
              if (!sensorId) return
              interactionStore.updateTimestamp()
              interactionStore.selectSensor(sensorId)
              break
            }
            case 'addOther': {
              const otherId = res as string
              if (!otherId) return
              interactionStore.updateTimestamp()
              interactionStore.selectOther(otherId)
              break
            }
            case 'addCustomModel': {
              const modelId = res as string
              if (!modelId) return
              interactionStore.updateTimestamp()
              interactionStore.selectCustomModel(modelId)
              break
            }
            // 如果是删除自定义模型（有可能是因为删除了模型配置引发的联动删除）
            case 'removeCustomModel': {
              interactionStore.updateTimestamp()
              const modelData = res as biz.ICustomModel
              if (!modelData) return
              // 如果是选中状态，则取消选中
              if (interactionStore.customModelId === modelData.id) {
                disposeBox3Helper()
                this.render()
              }
              break
            }
            case 'addParkingSpace': {
              const parkingSpaceId = res as string
              if (!parkingSpaceId) return
              interactionStore.updateTimestamp()
              // 停车位属于路面标识的特殊一种
              interactionStore.selectRoadSign(parkingSpaceId)
              break
            }
            case 'copyParkingSpace': {
              const parkingSpaceId = res as string
              if (!parkingSpaceId) return
              interactionStore.updateTimestamp()
              // 复制停车位，需要取消上一个停车位的选中效果，重新选中新复制的停车位
              const lastRoadSignId = interactionStore.roadSignId
              if (lastRoadSignId) {
                interactionStore.unselectRoadSign(lastRoadSignId)
              }
              interactionStore.selectRoadSign(parkingSpaceId)
              break
            }
            case 'updateSensorDeviceParams': {
              interactionStore.updateTimestamp()

              const { isUpdateRsuJunctionIds } = args[0]
              const { currentSensor } = interactionStore
              // 如果指明了更新 rsu 关联的路口，则需要联动调整对应的路口高亮效果
              if (isUpdateRsuJunctionIds && currentSensor) {
                // 取消上一个状态关联路口的高亮状态
                if (lastJunctionIds.length > 0) {
                  updateJunctionMaskColor({
                    status: 'normal',
                    junctionId: lastJunctionIds,
                    parent,
                  })
                }

                // 激活当前状态关联路口的高亮状态
                updateJunctionMaskColor({
                  status: 'selected',
                  junctionId: currentSensor.deviceParams.JunctionIDs,
                  parent,
                })

                this.render()
              }

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

    this.unsubscribeModelCacheStore = modelCacheStore.$onAction(
      ({ name, store, args, after, onError }) => {
        after((res) => {
          switch (name) {
            case 'updateCustomModelConfig': {
              // 如果更新了自定义模型的配置，则取消选中状态
              interactionStore.unselectCustomModel()
              interactionStore.updateTimestamp()
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
    const interactionStore = useObjectInteractionStore()
    interactionStore.$reset()
    this.editHelper.clear()
  }

  dispose () {
    this.editHelper.dispose()
    this.unsubscribeObjectStore && this.unsubscribeObjectStore()
    this.unsubscribeInteractionStore && this.unsubscribeInteractionStore()
    this.unsubscribeModelCacheStore && this.unsubscribeModelCacheStore()
  }
}

export default new EditObjectPlugin()
