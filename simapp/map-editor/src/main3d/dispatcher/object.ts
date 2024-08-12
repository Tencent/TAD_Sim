import { union } from 'lodash'
import { createPole, movePole, rotatePole } from '../render/pole'
import {
  createRoadSign,
  moveRoadSign,
  resetAllRoadSignColor,
  rotateRoadSign,
  updateRoadSignSize,
} from '../render/roadSign'
import {
  createSignalBoard,
  moveSignalBoard,
  rotateSignalBoard,
} from '../render/signalBoard'
import {
  createTrafficLight,
  moveTrafficLight,
  rotateTrafficLight,
} from '../render/trafficLight'
import {
  createOther,
  moveOther,
  rotateOther,
  updateOtherSize,
} from '../render/other'
import { createParkingSpace } from '../render/parkingSpace'
import { createSensor, moveSensor, rotateSensor } from '../render/sensor'
import {
  disposeBox3Helper,
  disposeOneObject,
  updateBox3HelperById,
} from '../render/object'
import {
  createCustomModel,
  moveCustomModel,
  rotateCustomModel,
  updateCustomModelSize,
} from '../render/customModel'
import { useObjectStore } from '@/stores/object'
import { getObjectOnPole, syncObjectSize } from '@/stores/object/common'
import { getObject } from '@/utils/mapCache'
import root3d from '@/main3d/index'
import { useHistoryStore } from '@/stores/history'
import i18n from '@/locales'
import { useObjectInteractionStore } from '@/plugins/editObject/store/interaction'
import { usePluginStore } from '@/stores/plugin'
import { useSignalControlInteractionStore } from '@/plugins/editSignalControl/store/interaction'

/**
 * 通过 id 对物体进行重绘
 * @param ids
 * @param isLoadFile 是否是加载地图文件时的重绘
 */
async function redrawObjectById (ids: Array<string>, isLoadFile = false) {
  const parent = root3d.mapElementsContainer
  // 重绘
  const asyncRenderFns = []
  for (const _id of ids) {
    const objectData = getObject(_id)
    if (objectData) {
      // 重绘之前，先销毁原来的元素
      disposeOneObject({
        objectId: objectData.id,
        parent,
      })

      const { mainType } = objectData
      switch (mainType) {
        case 'pole': {
          // 同步的渲染方法直接执行。且能保证在标志牌和信号灯渲染时，杆一定存在
          createPole({
            data: objectData as biz.IPole,
            parent,
          })
          break
        }
        // 停车位是特殊的路面标线，可以同步绘制
        case 'parkingSpace': {
          createParkingSpace({
            data: objectData as biz.IParkingSpace,
            parent,
          })
          break
        }
        case 'roadSign': {
          asyncRenderFns.push(
            new Promise((resolve) => {
              createRoadSign({
                data: objectData as biz.IRoadSign,
                parent,
              }).then((data) => {
                // 是加载地图文件时的重绘
                if (data && isLoadFile) {
                  const { width, length, height } = data
                  // 同步模型的初始尺寸
                  syncObjectSize({
                    objectId: objectData.id,
                    basicWidth: width,
                    basicLength: length,
                    basicHeight: height,
                  })
                }
                resolve(objectData.id)
              })
            }),
          )
          break
        }
        case 'signalBoard': {
          asyncRenderFns.push(
            new Promise((resolve) => {
              createSignalBoard({
                data: objectData as biz.ISignalBoard,
                parent,
              }).then((data) => {
                if (data) {
                  const { width, length, height } = data
                  syncObjectSize({
                    objectId: objectData.id,
                    width,
                    length,
                    height,
                  })
                }
                resolve(objectData.id)
              })
            }),
          )
          break
        }
        case 'trafficLight': {
          asyncRenderFns.push(
            new Promise((resolve) => {
              createTrafficLight({
                data: objectData as biz.ITrafficLight,
                parent,
              }).then((data) => {
                if (data) {
                  const { width, length, height } = data
                  syncObjectSize({
                    objectId: objectData.id,
                    width,
                    length,
                    height,
                  })
                }
                resolve(objectData.id)
              })
            }),
          )
          break
        }
        case 'sensor': {
          asyncRenderFns.push(
            new Promise((resolve) => {
              createSensor({
                data: objectData as biz.ISensor,
                parent,
              }).then((data) => {
                if (data) {
                  const { width, length, height } = data
                  syncObjectSize({
                    objectId: objectData.id,
                    width,
                    length,
                    height,
                  })
                }
                resolve(objectData.id)
              })
            }),
          )
          break
        }
        case 'other': {
          asyncRenderFns.push(
            new Promise((resolve) => {
              createOther({
                data: objectData as biz.IOther,
                parent,
              }).then((data) => {
                // 是加载地图文件时的重绘
                if (data && isLoadFile) {
                  const { width, length, height } = data
                  // 同步模型的初始尺寸
                  syncObjectSize({
                    objectId: objectData.id,
                    basicWidth: width,
                    basicLength: length,
                    basicHeight: height,
                  })
                }
                resolve(objectData.id)
              })
            }),
          )
          break
        }
        case 'customModel': {
          asyncRenderFns.push(
            new Promise((resolve) => {
              createCustomModel({
                data: objectData as biz.ICustomModel,
                parent,
              }).then((data) => {
                if (data && isLoadFile) {
                  const { width, length, height } = data
                  // 同步模型的初始尺寸
                  syncObjectSize({
                    objectId: objectData.id,
                    basicWidth: width,
                    basicLength: length,
                    basicHeight: height,
                  })
                }

                resolve(objectData.id)
              })
            }),
          )
          break
        }
        default:
          break
      }
    }
  }
  // 对于异步的渲染方法，等全部渲染结束再渲染
  await Promise.all(asyncRenderFns)

  const pluginStore = usePluginStore()
  // 如果是物体编辑模式
  if (pluginStore.currentPlugin === 'editObject') {
    // 先重置所有路面标线的颜色
    resetAllRoadSignColor(parent)

    const objectInteractionStore = useObjectInteractionStore()
    // 由于通过操作记录重新创建物体元素，是一个异步过程
    // 可能 objectInteractionStore 中同步的选中物体逻辑未能使三维场景中的元素实现选中高亮的效果
    // 需要在异步创建逻辑执行完以后，再执行一次选中
    const {
      poleId,
      roadSignId,
      signalBoardId,
      trafficLightId,
      sensorId,
      otherId,
      customModelId,
    } = objectInteractionStore
    if (poleId) {
      objectInteractionStore.selectPole(poleId)
    } else if (roadSignId) {
      objectInteractionStore.selectRoadSign(roadSignId)
    } else if (signalBoardId) {
      objectInteractionStore.selectSignalBoard(signalBoardId)
    } else if (trafficLightId) {
      objectInteractionStore.selectTrafficLight(trafficLightId)
    } else if (sensorId) {
      objectInteractionStore.selectSensor(sensorId)
    } else if (otherId) {
      objectInteractionStore.selectOther(otherId)
    } else if (customModelId) {
      objectInteractionStore.selectCustomModel(customModelId)
    }

    objectInteractionStore.updateTimestamp()
  } else if (pluginStore.currentPlugin === 'editSignalControl') {
    // 信控编辑
    const signalControlInteraction = useSignalControlInteractionStore()

    const { trafficLightId } = signalControlInteraction
    if (trafficLightId) {
      // 在信号灯完成重绘以后，强制选中信号灯
      signalControlInteraction.selectTrafficLight(trafficLightId, true)
      signalControlInteraction.updateTimestamp()
    }
  }

  root3d.core.render()

  // 由于fbx模型中有加载纹理，再调用一次定时的重绘
  root3d.core.renderByTimer()
}

export function initObjectDispatcher () {
  const objectStore = useObjectStore()
  const historyStore = useHistoryStore()
  const objectInteractionStore = useObjectInteractionStore()

  objectStore.$reset()

  const parent = root3d.mapElementsContainer

  // 物体的渲染调度
  const unsubscribeObjectStore = objectStore.$onAction(
    ({ name, store, args, after, onError }) => {
      after(async (res) => {
        switch (name) {
          // 将文件加载的物体数据渲染出来
          case 'applyMapFileState': {
            const [newState] = args
            if (!newState || newState.ids.length < 1) return

            // 对从文件中解析的物体，进行绘制
            await redrawObjectById(newState.ids, true)

            break
          }
          // 应用操作记录中的物体状态
          case 'applyState': {
            const { lastDiffIds, diff: diffState } = args[0]
            if (!lastDiffIds || !diffState) return
            const diffIds = union(lastDiffIds, diffState.ids)
            for (const _diffId of diffIds) {
              // 销毁原有的模型
              disposeOneObject({
                objectId: _diffId,
                parent,
              })
            }

            await redrawObjectById(lastDiffIds)
            break
          }
          // ---------- 杆 start ----------
          case 'addPole': {
            const poleId = res as string
            if (!poleId) return
            const poleData = getObject(poleId) as biz.IPole
            if (!poleData) return

            createPole({
              data: poleData,
              parent,
            })

            historyStore.save({
              title: i18n.global.t('actions.object.addPoleModel'),
              objectId: poleId,
              objects: [poleData],
            })

            // 路面标线，同步完尺寸触发一下地图编辑器的时间戳的更新
            objectInteractionStore.updateTimestamp()
            objectInteractionStore.selectPole(poleId)

            root3d.core.render()
            break
          }
          case 'movePole':
          case 'movePoleByST': {
            const { saveRecord } = args[0]
            const poleId = res as string
            if (!poleId) return
            const poleData = getObject(poleId) as biz.IPole
            if (!poleData) return

            movePole({
              data: poleData,
              parent,
            })

            // 在撤销重做过程中，要先销毁场景中的元素，因此在旋转杆时，操作记录中需要一并存入杆上所有物体的数据
            // 获取当前杆上所有的物体
            const objectOnPole = getObjectOnPole({
              poleId,
              ids: store.ids,
            })
            const objectIdOnPole = objectOnPole.map(
              objectData => objectData.id,
            )

            // 如果是通过 st 坐标更新位置，且没有要求保存操作记录，说明在其他的方法中调用该方法，只需要更新状态
            if (name === 'movePoleByST' && !saveRecord) return

            historyStore.save({
              title: i18n.global.t('actions.object.movePolePosition'),
              objectId: [poleId, ...objectIdOnPole],
              objects: [poleData, ...objectOnPole],
            })

            updateBox3HelperById(poleId)

            root3d.core.render()
            break
          }
          case 'rotatePole': {
            const poleId = res as string
            if (!poleId) return
            const poleData = getObject(poleId) as biz.IPole
            if (!poleData) return

            rotatePole({
              data: poleData,
              parent,
            })

            // 在撤销重做过程中，要先销毁场景中的元素，因此在旋转杆时，操作记录中需要一并存入杆上所有物体的数据
            // 获取当前杆上所有的物体
            const objectOnPole = getObjectOnPole({
              poleId,
              ids: store.ids,
            })
            const objectIdOnPole = objectOnPole.map(
              objectData => objectData.id,
            )

            historyStore.save({
              title: i18n.global.t('actions.object.rotatePoleAngle'),
              objectId: [poleId, ...objectIdOnPole],
              objects: [poleData, ...objectOnPole],
            })

            updateBox3HelperById(poleId)

            root3d.core.render()
            break
          }
          case 'removePole': {
            const [, saveRecord] = args

            const removeObjectList = res as Array<biz.IObject>
            // 如果没有返回已删除的元素则不响应
            if (removeObjectList.length < 1) return

            const objectIds = removeObjectList.map(
              objectData => objectData.id,
            )

            // 从场景中销毁删除的物体元素（包含杆和杆上的物体）
            for (const objectId of objectIds) {
              disposeOneObject({
                objectId,
                parent,
              })
            }

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removePoleModel'),
                objectId: objectIds,
                objects: removeObjectList,
              })
              disposeBox3Helper()
            }

            root3d.core.render()
            break
          }
          // ---------- 杆 end ----------
          // ---------- 路面标识 start ----------
          case 'addRoadSign': {
            const roadSignId = res as string
            if (!roadSignId) return
            const roadSignData = getObject(roadSignId) as biz.IRoadSign
            if (!roadSignData) return

            // 先等路标元素创建完，再渲染
            const addRes = await createRoadSign({
              data: roadSignData,
              parent,
            })
            // 由于数据层最开始不知道模型的大小，在场景中渲染后回传给数据层进行保存
            if (addRes) {
              const { width, length, height } = addRes
              syncObjectSize({
                objectId: roadSignId,
                width,
                length,
                height,
                // 新增物体时，使用模型加载原始的尺寸信息
                basicWidth: width,
                basicLength: length,
                basicHeight: height,
              })

              // 路面标线，同步完尺寸触发一下地图编辑器的时间戳的更新
              objectInteractionStore.updateTimestamp()
              objectInteractionStore.selectRoadSign(roadSignId)
            }

            historyStore.save({
              title: i18n.global.t('actions.object.addRoadSignModel'),
              objectId: roadSignId,
              objects: [roadSignData],
            })

            root3d.core.render()
            break
          }
          case 'updateRoadSignSize': {
            const roadSignId = res as string
            if (!roadSignId) return
            const roadSignData = getObject(roadSignId) as biz.IRoadSign
            if (!roadSignData) return

            const { name } = roadSignData

            if (
              name === 'Non_Motor_Vehicle_Area' ||
              name === 'Crosswalk_Line' ||
              name === 'Crosswalk_with_Left_and_Right_Side' ||
              name === 'Road_Guide_Lane_Line' ||
              name === 'Variable_Direction_Lane_Line' ||
              name === 'Longitudinal_Deceleration_Marking' ||
              name === 'Lateral_Deceleration_Marking' ||
              name === 'White_Semicircle_Line_Vehicle_Distance_Confirmation'
            ) {
              // 如果是自定义渲染的路面标线（人行横道、非机动车禁驶区）
              // 需要先销毁原来的物体，重新创建新的
              disposeOneObject({
                objectId: roadSignId,
                parent,
              })

              // 重新创建
              createRoadSign({
                data: roadSignData,
                parent,
              })
            } else {
              // 更新三维场景中路面标识的尺寸
              updateRoadSignSize({
                data: roadSignData,
                parent,
              })
            }

            historyStore.save({
              title: i18n.global.t('actions.object.updateRoadSignSize'),
              objectId: roadSignId,
              objects: [roadSignData],
            })

            // 如果是用户主动操作调用的位置移动，则需要更新包围盒外框效果
            updateBox3HelperById(roadSignId)
            root3d.core.render()
            break
          }
          case 'moveRoadSign':
          case 'moveCustomRoadSign':
          case 'moveRoadSignByST': {
            const { saveRecord } = args[0]
            const roadSignId = res as string
            if (!roadSignId) return
            const roadSignData = getObject(roadSignId) as biz.IRoadSign
            if (!roadSignData) return

            const { name: roadSignName } = roadSignData

            if (
              roadSignName === 'Stop_Line' ||
              roadSignName === 'Crosswalk_Line' ||
              roadSignName === 'Crosswalk_with_Left_and_Right_Side' ||
              roadSignName === 'Road_Guide_Lane_Line' ||
              roadSignName === 'Variable_Direction_Lane_Line' ||
              roadSignName === 'Longitudinal_Deceleration_Marking' ||
              roadSignName === 'Lateral_Deceleration_Marking' ||
              roadSignName ===
              'White_Semicircle_Line_Vehicle_Distance_Confirmation'
            ) {
              // 先销毁旧的
              disposeOneObject({
                objectId: roadSignId,
                parent,
              })
              // 再重新创建新的
              await createRoadSign({
                data: roadSignData,
                parent,
              })
            } else {
              moveRoadSign({
                data: roadSignData,
                parent,
              })
            }

            // 如果是通过 st 坐标更新位置，且没有要求保存操作记录，说明在其他的方法中调用该方法，只需要更新状态
            if (name === 'moveRoadSignByST' && !saveRecord) return

            historyStore.save({
              title: i18n.global.t('actions.object.moveRoadSignPosition'),
              objectId: roadSignId,
              objects: [roadSignData],
            })

            // 如果是用户主动操作调用的位置移动，则需要更新包围盒外框效果
            updateBox3HelperById(roadSignId)
            root3d.core.render()
            break
          }
          case 'rotateRoadSign': {
            const roadSignId = res as string
            if (!roadSignId) return
            const roadSignData = getObject(roadSignId) as biz.IRoadSign
            if (!roadSignData) return

            rotateRoadSign({
              data: roadSignData,
              parent,
            })

            historyStore.save({
              title: i18n.global.t('actions.object.rotateRoadSignAngle'),
              objectId: roadSignId,
              objects: [roadSignData],
            })

            // 如果是用户主动操作调用的位置移动，则需要更新包围盒外框效果
            updateBox3HelperById(roadSignId)
            root3d.core.render()
            break
          }
          case 'removeRoadSign': {
            const [, saveRecord] = args

            const roadSignData = res as biz.IRoadSign
            if (!roadSignData) return

            disposeOneObject({
              objectId: roadSignData.id,
              parent,
            })

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removeRoadSignModel'),
                objectId: roadSignData.id,
                objects: [roadSignData],
              })

              // 主动销毁包围盒外框
              disposeBox3Helper()
            }

            root3d.core.render()
            break
          }
          case 'updateRoadSignByLane': {
            const roadSignData = res as biz.IRoadSign
            if (!roadSignData) return

            const { name } = roadSignData
            if (
              name === 'Crosswalk_Line' ||
              name === 'Crosswalk_with_Left_and_Right_Side' ||
              name === 'Stop_Line' ||
              name === 'Road_Guide_Lane_Line' ||
              name === 'Variable_Direction_Lane_Line' ||
              name === 'Longitudinal_Deceleration_Marking' ||
              name === 'Lateral_Deceleration_Marking' ||
              name === 'White_Semicircle_Line_Vehicle_Distance_Confirmation'
            ) {
              // 特殊的路面标线，需要先销毁再重新创建
              disposeOneObject({
                objectId: roadSignData.id,
                parent,
              })
              createRoadSign({
                data: roadSignData,
                parent,
              })
            } else {
              // 更新路面标识的位置，无须调用重绘函数
              moveRoadSign({
                data: roadSignData,
                parent,
              })
            }
            break
          }
          case 'updateRoadSignByJunction': {
            const roadSignData = res as biz.IRoadSign
            if (!roadSignData) return

            const { name } = roadSignData
            if (name === 'Intersection_Guide_Line') {
              // 特殊的路面标线，需要先销毁再重新创建
              disposeOneObject({
                objectId: roadSignData.id,
                parent,
              })
              createRoadSign({
                data: roadSignData,
                parent,
              })
            }
            break
          }
          // ---------- 路面标识 end ----------
          // ---------- 标志牌 start ----------
          case 'addSignalBoard': {
            const signalBoardId = res as string
            if (!signalBoardId) return
            const signalBoardData = getObject(signalBoardId) as biz.ISignalBoard
            if (!signalBoardData) return
            // 先等信号标志牌的模型导入完成，再渲染
            const addRes = await createSignalBoard({
              data: signalBoardData,
              parent,
            })
            // 由于数据层最开始不知道模型的大小，在场景中渲染后回传给数据层进行保存
            if (addRes) {
              const { width, length, height } = addRes
              syncObjectSize({
                objectId: signalBoardId,
                width,
                length,
                height,
              })

              objectInteractionStore.updateTimestamp()
              objectInteractionStore.selectSignalBoard(signalBoardId)
            }

            historyStore.save({
              title: i18n.global.t('actions.object.addSignalBoardModel'),
              objectId: signalBoardId,
              objects: [signalBoardData],
            })

            root3d.core.render()
            break
          }
          case 'moveSignalBoard':
          case 'moveSignalBoardByHeightAndDist': {
            const signalBoardId = res as string
            if (!signalBoardId) return
            const signalBoardData = getObject(signalBoardId) as biz.ISignalBoard
            if (!signalBoardData) return
            moveSignalBoard({
              data: signalBoardData,
              parent,
            })
            historyStore.save({
              title: i18n.global.t('actions.object.moveSignalBoardPosition'),
              objectId: signalBoardId,
              objects: [signalBoardData],
            })
            updateBox3HelperById(signalBoardId)
            root3d.core.render()
            break
          }
          case 'toggleSignalBoardDirection':
          case 'rotateSignalBoard': {
            const signalBoardId = res as string
            if (!signalBoardId) return
            const signalBoardData = getObject(signalBoardId) as biz.ISignalBoard
            if (!signalBoardData) return
            rotateSignalBoard({
              data: signalBoardData,
              parent,
            })
            historyStore.save({
              title: i18n.global.t('actions.object.rotateSignalBoardAngle'),
              objectId: signalBoardId,
              objects: [signalBoardData],
            })
            updateBox3HelperById(signalBoardId)
            root3d.core.render()
            break
          }
          case 'removeSignalBoard': {
            const [, saveRecord] = args

            const signalBoardData = res as biz.ISignalBoard | undefined
            if (!signalBoardData) return

            disposeOneObject({
              objectId: signalBoardData.id,
              parent,
            })

            // 只有调用方法时主动设置需要触发记录保存，才会执行
            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removeSignalBoardModel'),
                objectId: signalBoardData.id,
                objects: [signalBoardData],
              })
              disposeBox3Helper()
            }

            root3d.core.render()
            break
          }
          // ---------- 标志牌 end ----------
          // ---------- 信号灯 start ----------
          case 'addTrafficLight': {
            const trafficLightId = res as string
            if (!trafficLightId) return
            const trafficLightData = getObject(
              trafficLightId,
            ) as biz.ITrafficLight
            if (!trafficLightData) return
            // 先等交通信号灯的模型和纹理导入完成，再渲染
            const addRes = await createTrafficLight({
              data: trafficLightData,
              parent,
            })
            // 由于数据层最开始不知道模型的大小，在场景中渲染后回传给数据层进行保存
            if (addRes) {
              const { width, length, height } = addRes
              syncObjectSize({
                objectId: trafficLightId,
                width,
                length,
                height,
              })
              objectInteractionStore.updateTimestamp()
              objectInteractionStore.selectTrafficLight(trafficLightId)
            }

            historyStore.save({
              title: i18n.global.t('actions.object.addTrafficLightModel'),
              objectId: trafficLightId,
              objects: [trafficLightData],
            })

            root3d.core.render()

            // 由于fbx模型中有加载纹理，再调用一次定时的重绘
            root3d.core.renderByTimer()
            break
          }
          case 'moveTrafficLight':
          case 'moveTrafficLightByHeightAndDist': {
            const trafficLightId = res as string
            if (!trafficLightId) return
            const trafficLightData = getObject(
              trafficLightId,
            ) as biz.ITrafficLight
            if (!trafficLightData) return
            moveTrafficLight({
              data: trafficLightData,
              parent,
            })
            historyStore.save({
              title: i18n.global.t('actions.object.moveTrafficLightPosition'),
              objectId: trafficLightId,
              objects: [trafficLightData],
            })
            updateBox3HelperById(trafficLightId)
            root3d.core.render()
            break
          }
          case 'toggleTrafficLightDirection':
          case 'rotateTrafficLight': {
            const trafficLightId = res as string
            if (!trafficLightId) return
            const trafficLightData = getObject(
              trafficLightId,
            ) as biz.ITrafficLight
            if (!trafficLightData) return
            rotateTrafficLight({
              data: trafficLightData,
              parent,
            })
            historyStore.save({
              title: i18n.global.t('actions.object.rotateTrafficLightAngle'),
              objectId: trafficLightId,
              objects: [trafficLightData],
            })
            updateBox3HelperById(trafficLightId)
            root3d.core.render()
            break
          }
          case 'removeTrafficLight': {
            const [, saveRecord] = args

            const trafficLightData = res as biz.ITrafficLight | undefined
            if (!trafficLightData) return

            disposeOneObject({
              objectId: trafficLightData.id,
              parent,
            })

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removeTrafficLightModel'),
                objectId: trafficLightData.id,
                objects: [trafficLightData],
              })
              disposeBox3Helper()
            }

            root3d.core.render()
            break
          }
          // ---------- 信号灯 end ----------
          // ---------- 传感器 start ----------
          case 'addSensor': {
            const sensorId = res as string
            if (!sensorId) return
            const sensorData = getObject(sensorId) as biz.ISensor
            if (!sensorData) return
            // 先等传感器的模型和纹理导入完成，再渲染
            const addRes = await createSensor({
              data: sensorData,
              parent,
            })
            // 由于数据层最开始不知道模型的大小，在场景中渲染后回传给数据层进行保存
            if (addRes) {
              const { width, length, height } = addRes
              syncObjectSize({
                objectId: sensorId,
                width,
                length,
                height,
              })
              objectInteractionStore.updateTimestamp()
              objectInteractionStore.selectSensor(sensorId)
            }

            historyStore.save({
              title: i18n.global.t('actions.object.addSensorModel'),
              objectId: sensorId,
              objects: [sensorData],
            })

            root3d.core.render()

            // 由于fbx模型中有加载纹理，再调用一次定时的重绘
            root3d.core.renderByTimer()
            break
          }
          case 'moveSensor':
          case 'moveSensorByHeightAndDist': {
            const sensorId = res as string
            if (!sensorId) return
            const sensorData = getObject(sensorId) as biz.ISensor
            if (!sensorData) return
            moveSensor({
              data: sensorData,
              parent,
            })
            historyStore.save({
              title: i18n.global.t('actions.object.moveSensorPosition'),
              objectId: sensorId,
              objects: [sensorData],
            })
            updateBox3HelperById(sensorId)
            root3d.core.render()
            break
          }
          case 'toggleSensorDirection':
          case 'rotateSensor': {
            const sensorId = res as string
            if (!sensorId) return
            const sensorData = getObject(sensorId) as biz.ISensor
            if (!sensorData) return
            rotateSensor({
              data: sensorData,
              parent,
            })
            historyStore.save({
              title: i18n.global.t('actions.object.rotateSensorAngle'),
              objectId: sensorId,
              objects: [sensorData],
            })
            updateBox3HelperById(sensorId)
            root3d.core.render()
            break
          }
          case 'removeSensor': {
            const [, saveRecord] = args

            const sensorData = res as biz.ISensor | undefined
            if (!sensorData) return

            disposeOneObject({
              objectId: sensorData.id,
              parent,
            })

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removeSensorModel'),
                objectId: sensorData.id,
                objects: [sensorData],
              })
              disposeBox3Helper()
            }

            root3d.core.render()
            break
          }
          case 'updateSensorDeviceParams': {
            // 判断是否需要保存操作记录
            const { saveRecord = false } = args[0]
            const sensorId = res as string
            if (!sensorId) return
            const sensorData = getObject(sensorId) as biz.ISensor
            if (!sensorData) return
            // 修改传感器参数配置，需要保存操作记录
            historyStore.save({
              title: i18n.global.t(
                'actions.object.updateSensorDeviceParameterConfiguration',
              ),
              objectId: sensorId,
              objects: [sensorData],
            })
            break
          }
          // ---------- 传感器 end ----------
          // ---------- 其他类型物体 start ----------
          case 'addOther': {
            const otherId = res as string
            if (!otherId) return
            const otherData = getObject(otherId) as biz.IOther
            if (!otherData) return
            // 先等模型和纹理导入完成，再渲染
            const addRes = await createOther({
              data: otherData,
              parent,
            })
            // 由于数据层最开始不知道模型的大小，在场景中渲染后回传给数据层进行保存
            if (addRes) {
              const { width, length, height } = addRes
              syncObjectSize({
                objectId: otherId,
                width,
                length,
                height,
                // 新增物体时，使用模型加载原始的尺寸信息
                basicWidth: width,
                basicLength: length,
                basicHeight: height,
              })

              // 其他类型物体，同步完尺寸触发一下地图编辑器的时间戳的更新
              objectInteractionStore.updateTimestamp()
              objectInteractionStore.selectOther(otherId)
            }

            historyStore.save({
              title: i18n.global.t('actions.object.addOtherTypeModel'),
              objectId: otherId,
              objects: [otherData],
            })

            root3d.core.render()

            // 由于fbx模型中有加载纹理，再调用一次定时的重绘
            root3d.core.renderByTimer()
            break
          }
          case 'updateOtherSize': {
            const otherId = res as string
            if (!otherId) return
            const otherData = getObject(otherId) as biz.IOther
            if (!otherData) return

            // 更新三维场景中其他类型物体的尺寸
            updateOtherSize({
              data: otherData,
              parent,
            })

            historyStore.save({
              title: i18n.global.t('actions.object.updateObjectSize'),
              objectId: otherId,
              objects: [otherData],
            })

            updateBox3HelperById(otherId)

            root3d.core.render()
            break
          }
          case 'updateBridgeSpan': {
            const bridgeId = res as string
            if (!bridgeId) return
            const bridgeData = getObject(bridgeId) as biz.IOther
            if (!bridgeData) return

            // 由于可能涉及内部模型的数量改动，干脆直接销毁掉原来的模型
            disposeOneObject({
              objectId: bridgeId,
              parent,
            })

            // 先等模型和纹理导入完成，再渲染
            const addRes = await createOther({
              data: bridgeData,
              parent,
            })
            // 由于数据层最开始不知道模型的大小，在场景中渲染后回传给数据层进行保存
            if (addRes) {
              const { width, length, height } = addRes
              syncObjectSize({
                objectId: bridgeId,
                width,
                length,
                height,
                // 新增物体时，使用模型加载原始的尺寸信息
                basicWidth: width,
                basicLength: length,
                basicHeight: height,
              })

              // 其他类型物体，同步完尺寸触发一下地图编辑器的时间戳的更新
              objectInteractionStore.updateTimestamp()
            }

            historyStore.save({
              title: i18n.global.t('actions.object.updateObjectSize'),
              objectId: bridgeId,
              objects: [bridgeData],
            })

            updateBox3HelperById(bridgeId)

            root3d.core.render()
            break
          }
          case 'moveOther':
          case 'moveOtherByST': {
            const { saveRecord } = args[0]
            const otherId = res as string
            if (!otherId) return
            const otherData = getObject(otherId) as biz.IOther
            if (!otherData) return

            // 场景中移动物体
            moveOther({
              data: otherData,
              parent,
            })

            // 如果是通过 st 坐标更新位置，且没有要求保存操作记录，说明在其他的方法中调用该方法，只需要更新状态
            if (name === 'moveOtherByST' && !saveRecord) return

            historyStore.save({
              title: i18n.global.t('actions.object.moveObjectPosition'),
              objectId: otherId,
              objects: [otherData],
            })

            updateBox3HelperById(otherId)

            root3d.core.render()
            break
          }
          case 'rotateOther': {
            const otherId = res as string
            if (!otherId) return
            const otherData = getObject(otherId) as biz.IOther
            if (!otherData) return

            // 场景中旋转物体角度
            rotateOther({
              data: otherData,
              parent,
            })

            historyStore.save({
              title: i18n.global.t('actions.object.rotateObjectAngle'),
              objectId: otherId,
              objects: [otherData],
            })

            updateBox3HelperById(otherId)

            root3d.core.render()
            break
          }
          case 'removeOther': {
            const [, saveRecord] = args

            const otherData = res as biz.IOther
            if (!otherData) return

            disposeOneObject({
              objectId: otherData.id,
              parent,
            })

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removeOtherTypeModel'),
                objectId: otherData.id,
                objects: [otherData],
              })
              disposeBox3Helper()
            }

            root3d.core.render()
            break
          }
          // ---------- 其他类型物体 end ----------
          // ---------- 停车位 start ----------
          case 'addParkingSpace':
          case 'copyParkingSpace': {
            const parkingSpaceId = res as string
            if (!parkingSpaceId) return
            const parkingSpaceData = getObject(
              parkingSpaceId,
            ) as biz.IParkingSpace
            if (!parkingSpaceData) return
            createParkingSpace({
              data: parkingSpaceData,
              parent,
            })

            if (name === 'addParkingSpace') {
              historyStore.save({
                title: i18n.global.t('actions.object.addParkingSpace'),
                objectId: parkingSpaceId,
                objects: [parkingSpaceData],
              })

              objectInteractionStore.updateTimestamp()
            } else if (name === 'copyParkingSpace') {
              historyStore.save({
                title: i18n.global.t('actions.object.copyParkingSpace'),
                objectId: parkingSpaceId,
                objects: [parkingSpaceData],
              })
            }

            root3d.core.render()
            break
          }
          case 'moveParkingSpace':
          case 'moveParkingSpaceByST':
          case 'rotateParkingSpace':
          case 'updateParkingSpaceProperty': {
            const parkingSpaceId = res as string
            if (!parkingSpaceId) return
            const parkingSpaceData = getObject(
              parkingSpaceId,
            ) as biz.IParkingSpace
            if (!parkingSpaceData) return
            // 先销毁旧的停车位
            disposeOneObject({
              objectId: parkingSpaceId,
              parent,
            })

            // 每一次更新停车位的三维效果，都是重新渲染新的停车位
            createParkingSpace({
              data: parkingSpaceData,
              parent,
            })

            // 如果是通过 st 坐标更新位置，且没有要求保存操作记录，说明在其他的方法中调用该方法，只需要更新状态
            if (name === 'moveParkingSpaceByST' && !args[0].saveRecord) return

            let title = ''
            if (
              name === 'moveParkingSpace' ||
              name === 'moveParkingSpaceByST'
            ) {
              title = i18n.global.t('actions.object.moveParkingSpace')
            } else if (name === 'rotateParkingSpace') {
              title = i18n.global.t('actions.object.rotateParkingSpace')
            } else if (name === 'updateParkingSpaceProperty') {
              title = i18n.global.t('actions.object.updateParkingSpaceProperty')
            }
            if (title) {
              historyStore.save({
                title,
                objectId: parkingSpaceId,
                objects: [parkingSpaceData],
              })
            }
            root3d.core.render()
            break
          }
          case 'removeParkingSpace': {
            const [, saveRecord] = args
            const parkingSpaceData = res as biz.IParkingSpace
            if (!parkingSpaceData) return
            // 删除停车位（如果是重复连续的停车位，一同删除）
            disposeOneObject({
              objectId: parkingSpaceData.id,
              parent,
            })

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removeParkingSpace'),
                objectId: parkingSpaceData.id,
                objects: [parkingSpaceData],
              })
            }

            root3d.core.render()
            break
            break
          }
          // ---------- 停车位 end ----------
          // ---------- 自定义模型 start ----------
          case 'addCustomModel': {
            const customModelId = res as string
            if (!customModelId) return
            const customModelData = getObject(customModelId) as biz.ICustomModel
            if (!customModelData) return

            // 从渲染层创建对应模型
            const addRes = await createCustomModel({
              data: customModelData,
              parent,
            })
            if (addRes) {
              const { length, width, height } = addRes
              syncObjectSize({
                objectId: customModelId,
                width,
                length,
                height,
                // 新增物体使用导入模型时原始的模型尺寸
                basicLength: length,
                basicWidth: width,
                basicHeight: height,
              })

              objectInteractionStore.updateTimestamp()
              objectInteractionStore.selectCustomModel(customModelId)
            }

            historyStore.save({
              title: i18n.global.t('actions.object.addCustomModel'),
              objectId: customModelId,
              objects: [customModelData],
            })

            root3d.core.render()

            // 由于fbx模型中有加载纹理，再调用一次定时的重绘
            root3d.core.renderByTimer()
            break
          }
          case 'moveCustomModel':
          case 'moveCustomModelByST': {
            const { saveRecord } = args[0]
            const modelId = res as string
            if (!modelId) return
            const modelData = getObject(modelId) as biz.ICustomModel
            if (!modelData) return

            // 场景中移动自定义模型
            moveCustomModel({
              data: modelData,
              parent,
            })

            // 如果是通过 st 坐标更新位置，且没有保存操作记录，说明在其他的方法中调用该方法，只需要更新状态
            if (name === 'moveCustomModelByST' && !saveRecord) return

            historyStore.save({
              title: i18n.global.t('actions.object.moveCustomModel'),
              objectId: modelId,
              objects: [modelData],
            })

            updateBox3HelperById(modelId)

            root3d.core.render()
            break
          }
          case 'updateCustomModelSize': {
            const modelId = res as string
            if (!modelId) return
            const modelData = getObject(modelId) as biz.ICustomModel
            if (!modelData) return

            // 更新三维场景中自定义模型物体的尺寸
            updateCustomModelSize({
              data: modelData,
              parent,
            })

            historyStore.save({
              title: i18n.global.t('actions.object.updateCustomModelSize'),
              objectId: modelId,
              objects: [modelData],
            })

            updateBox3HelperById(modelId)

            root3d.core.render()
            break
          }
          case 'rotateCustomModel': {
            const modelId = res as string
            if (!modelId) return
            const modelData = getObject(modelId) as biz.ICustomModel
            if (!modelData) return
            // 场景中旋转自定义模型
            rotateCustomModel({
              data: modelData,
              parent,
            })

            historyStore.save({
              title: i18n.global.t('actions.object.rotateCustomModel'),
              objectId: modelId,
              objects: [modelData],
            })

            updateBox3HelperById(modelId)

            root3d.core.render()
            break
          }
          case 'removeCustomModel': {
            const [, saveRecord] = args
            const modelData = res as biz.ICustomModel
            if (!modelData) return

            disposeOneObject({
              objectId: modelData.id,
              parent,
            })

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.object.removeCustomModel'),
                objectId: modelData.id,
                objects: [modelData],
              })
              disposeBox3Helper()
            }

            root3d.core.render()
            break
          }
          // ---------- 自定义模型 end ----------
          default:
            break
        }
      })
      onError((err) => {
        console.log(err)
      })
    },
  )

  return unsubscribeObjectStore
}
