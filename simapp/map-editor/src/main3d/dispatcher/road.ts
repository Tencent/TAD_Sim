import { union, uniq } from 'lodash'
import { disposeRoad, renderRoad, updateTunnelMask } from '../render/road'
import root3d from '../index'
import { updateObjectInRoad } from './common'
import { useRoadStore } from '@/stores/road'
import { useJunctionStore } from '@/stores/junction'
import { useHistoryStore } from '@/stores/history'
import { getJunction, getRoad } from '@/utils/mapCache'
import i18n from '@/locales'
import { useObjectStore } from '@/stores/object/index'
import { getObjectInRoad } from '@/stores/object/common'

// 初始化道路级别元素的派发器
export function initRoadDispatcher () {
  const roadStore = useRoadStore()
  const junctionStore = useJunctionStore()
  const historyStore = useHistoryStore()

  roadStore.$reset()

  const container = root3d.mapElementsContainer

  // 道路的渲染调度
  const unsubscribeRoadStore = roadStore.$onAction(
    ({ name, store, args, after, onError }) => {
      // before
      let oneRoad: biz.IRoad | undefined
      after(async (res) => {
        let needUpdateJunction = false
        switch (name) {
          case 'applyMapFileState': {
            const [newState] = args
            if (!newState || newState.ids.length < 1) return
            const renderAllRoad = []
            for (const roadId of newState.ids) {
              const road = getRoad(roadId)
              if (road) {
                // 首次渲染不需要销毁之前的同元素
                renderAllRoad.push(
                  renderRoad({ road, parent: container, preDispose: false }),
                )
              }
            }
            await Promise.all(renderAllRoad)
            return
          }
          case 'applyState': {
            // // 还原状态时，增量对道路元素做重绘
            const { lastDiffIds, diff: diffState } = args[0]
            if (!lastDiffIds || !diffState) return
            // 通过 diffState 和 lastDiffIds 求并集判断有多少变化的道路
            const diffIds = union(lastDiffIds, diffState.ids)
            for (const _diffId of diffIds) {
              disposeRoad({
                roadId: _diffId,
                parent: container,
              })
            }

            // 重绘上一个状态的道路
            for (const _diffRoadId of lastDiffIds) {
              const _road = getRoad(_diffRoadId)
              if (_road) {
                renderRoad({
                  road: _road,
                  parent: container,
                })
              }
            }
            root3d.core.render()
            return
          }
          case 'updateRoadKeyPath': {
            const { roadId, isPure = false } = args[0]
            oneRoad = getRoad(roadId)
            if (!oneRoad) return
            // 由于在 editRoad 的订阅逻辑中，已经更新了junction的结构，此处不需要重复更新路口结构
            needUpdateJunction = false
            renderRoad({
              road: oneRoad,
              parent: container,
            })

            // 如果是单纯地更新道路结构，则可以直接返回
            if (isPure) return

            break
          }
          case 'updateLaneWidth':
          case 'addLane':
          case 'removeLane': {
            // 如果返回结果为 false，说明更新车道结构失败，不重新渲染
            if (!res) return
            const { roadId } = args[0]
            oneRoad = getRoad(roadId)
            if (!oneRoad) return
            needUpdateJunction = true
            renderRoad({
              road: oneRoad,
              parent: container,
            })
            break
          }
          case 'addSectionWithTween':
          case 'addSectionOnly': {
            const { roadId } = args[0]
            oneRoad = getRoad(roadId)
            if (!oneRoad) return
            needUpdateJunction = true
            if (name === 'addSectionOnly') {
              needUpdateJunction = false
            }
            renderRoad({
              road: oneRoad,
              parent: container,
            })
            break
          }
          case 'removeRoad': {
            const [roadId] = args
            if (!roadId) return
            // 由于删除控制点也会涉及删除 road，因此没有在该方法中保存操作记录
            disposeRoad({
              roadId,
              parent: container,
            })
            break
          }
          case 'updateLaneAttr': {
            const { attrName, roadId } = args[0]
            if (
              attrName === 'speedlimit' ||
              attrName === 'friction' ||
              attrName === 'sOffset'
            ) {
              const _roads = []
              const _road = roadStore.getRoadById(roadId)
              _road && _roads.push(_road)
              // 更新车道静态属性，diff 的元素只有 road
              let title = i18n.global.t('actions.lane.modifyLaneProperty')
              // 修改限速提供单独的日志文案提示
              if (attrName === 'speedlimit') {
                title = i18n.global.t('actions.lane.modifyLaneSpeedLimit')
              }
              historyStore.save({
                title,
                roadId,
                roads: _roads,
              })
            } else if (attrName === 'type') {
              const _roads = []
              const _road = roadStore.getRoadById(roadId)
              if (_road) {
                _roads.push(_road)
                // 更新道路类型，需要重绘道路
                renderRoad({
                  road: _road,
                  parent: container,
                })
              }

              historyStore.save({
                title: i18n.global.t('actions.lane.modifyLaneType'),
                roadId,
                roads: _roads,
              })

              root3d.core.render()
            }
            // 只更新静态属性，没有三维元素的更新，暂不需要重绘
            return
          }
          case 'updateLaneBoundaryMark': {
            // 只有更新成功以后才会触发后续操作
            if (!res) return
            const { roadId } = args[0]
            oneRoad = getRoad(roadId)
            const _roads = []
            if (oneRoad) {
              _roads.push(oneRoad)
              renderRoad({
                road: oneRoad,
                parent: container,
              })
            }
            // 更新车道边界线样式，diff 元素只有 road
            historyStore.save({
              title: i18n.global.t('actions.boundary.modifyLaneBoundaryStyle'),
              roadId,
              roads: _roads,
            })
            root3d.core.render()
            return
          }
          case 'updateArcRoad': {
            const roadIds = res as Array<string>
            const roads: Array<biz.IRoad> = []
            // 跟道路关联的路口（可能存在重复）
            const _junctionIds: Array<string> = []
            for (const _id of roadIds) {
              const road = getRoad(_id)
              if (!road) continue
              roads.push(road)
              _junctionIds.push(...road.linkJunction)
            }
            const diffJunctionIds = uniq(_junctionIds)
            const junctions: Array<biz.IJunction> = []

            // 获取跟道路关联的物体
            const { objectId, objects } = updateObjectInRoad(roadIds)

            // 更新跟道路关联的路口结构
            const updatePromise: Array<Promise<string>> = []
            for (const junctionId of diffJunctionIds) {
              const junction = getJunction(junctionId)
              if (!junction) continue
              junctions.push(junction)
              // 跟变更的车道相连的交叉路口，直接调用一次更新，并立即重绘
              updatePromise.push(
                (() => {
                  return new Promise((resolve) => {
                    junctionStore
                      .updateJunction({
                        junction,
                        isPureUpdate: true,
                      })
                      .then(() => {
                        resolve(junction.id)
                      })
                  })
                })(),
              )
            }
            await Promise.all(updatePromise)

            historyStore.save({
              title: i18n.global.t('actions.circleRoad.updateArcRoadAngle'),
              roads,
              junctions,
              objects,
              roadId: roadIds,
              junctionId: diffJunctionIds,
              objectId,
            })

            root3d.core.render()
            return
          }
          case 'createCircleRoad': {
            const roadIds = res as Array<string>
            const roads: Array<biz.IRoad> = []
            for (const _id of roadIds) {
              const road = getRoad(_id)
              if (!road) continue
              roads.push(road)
            }

            historyStore.save({
              title: i18n.global.t('actions.circleRoad.createArcRoads'),
              roadId: roadIds,
              roads,
            })

            root3d.core.render()
            return
          }
          case 'updateTunnelAttr': {
            const { saveRecord, needRedraw } = args[0]
            const roadIds = res as Array<string>
            const roads: Array<biz.ITunnel> = []
            for (const _id of roadIds) {
              const road = getRoad(_id) as biz.ITunnel
              if (!road) continue
              roads.push(road)
            }

            if (saveRecord) {
              historyStore.save({
                title: i18n.global.t('actions.road.updateTunnelProperty'),
                roadId: roadIds,
                roads,
              })
            }

            if (needRedraw) {
              // 渲染层更新隧道的效果
              for (const road of roads) {
                updateTunnelMask({
                  road,
                  parent: container,
                })
              }
            }
            root3d.core.render()
            return
          }
          default:
            break
        }

        // 记录当前 road 连通的 junctionId
        const _roads = []
        const _junctions = []
        const diffJunctionId = []

        if (oneRoad) {
          _roads.push(oneRoad)

          const _objects = []

          // 跟车道结构相关的操作，需要调整当前道路中车道上路面标识的位置
          if (
            name === 'addLane' ||
            name === 'removeLane' ||
            name === 'updateLaneWidth'
          ) {
            // 获取修改的车道 id
            const { laneId } = args[0]
            const targetLaneIdValue = Math.abs(Number(laneId))

            const objectStore = useObjectStore()
            const objectOnRoad = getObjectInRoad({
              roadId: oneRoad.id,
              ids: objectStore.ids,
            })
            for (const _object of objectOnRoad) {
              const { mainType, id: objectId } = _object
              // 基于当前道路中新的车道结构，更新车道上路面标识的位置
              // 停车位跟道路参考线关联，车道结构的改变不会联动更新停车位
              if (mainType !== 'roadSign') continue
              // 获取当前路面标识关联的车道
              const { laneId: originLaneId, name: roadSignName } = _object

              // 如果是横跨道路的路面标识，需要特殊处理
              if (
                roadSignName === 'Stop_Line' ||
                roadSignName === 'Crosswalk_Line' ||
                roadSignName === 'Crosswalk_with_Left_and_Right_Side'
              ) {
                const roadSignData = objectStore.updateRoadSignByLane({
                  objectId,
                })
                if (roadSignData) {
                  _objects.push(roadSignData)
                }
                continue
              }

              const originLaneIdValue = Math.abs(Number(originLaneId))

              // 如果路面标识所在车道 id 的绝对值小于修改的车道 id，则无须更新
              if (originLaneIdValue < targetLaneIdValue) continue

              if (name === 'removeLane') {
                // 删除车道
                if (originLaneIdValue === targetLaneIdValue) {
                  // 如果删除的车道为路面标识所在的车道，则路面标识需要连带删除
                  const roadSignId = objectStore.removeRoadSign(objectId, false)
                  if (roadSignId) {
                    _objects.push(_object)
                  }
                } else {
                  const roadSignData = objectStore.updateRoadSignByLane({
                    objectId,
                    type: 'remove',
                  })
                  if (roadSignData) {
                    // 将更新过状态的路面标识数据，同步到操作记录中
                    _objects.push(roadSignData)
                  }
                }
              } else if (name === 'updateLaneWidth') {
                // 修改车道宽度，不存在车道数量的改变
                const roadSignData = objectStore.updateRoadSignByLane({
                  objectId,
                })
                if (roadSignData) {
                  // 将更新过状态的路面标识数据，同步到操作记录中
                  _objects.push(roadSignData)
                }
              } else if (name === 'addLane') {
                // 新增车道是在右侧边界线新增一条同属性车道，对目标车道没影响。只更新目标车道外层的路面标识
                if (originLaneIdValue !== targetLaneIdValue) {
                  // 此处是目标车道外侧车道
                  const roadSignData = objectStore.updateRoadSignByLane({
                    objectId,
                    type: 'add',
                  })
                  if (roadSignData) {
                    // 将更新过状态的路面标识数据，同步到操作记录中
                    _objects.push(roadSignData)
                  }
                }
              }
            }
          }

          // 需要在操作记录中
          const diffObjectId: Array<string> = _objects.map(
            objectData => objectData.id,
          )

          // 更新连通路口的结构在保存操作记录之前，保证撤销重做能还原路口效果
          // 对于车道结构改变的，判断交叉路口是否有跟这条道路相连，如果有，则对应的交叉路口需要做一遍重绘
          if (needUpdateJunction) {
            const updatePromise: Array<Promise<string>> = []
            for (const junctionId of oneRoad.linkJunction) {
              const junction = getJunction(junctionId)
              diffJunctionId.push(junctionId)
              if (junction) {
                _junctions.push(junction)
                // 跟变更的车道相连的交叉路口，直接调用一次更新，并立即重绘
                updatePromise.push(
                  (() => {
                    return new Promise((resolve) => {
                      junctionStore
                        .updateJunction({
                          junction,
                          isPureUpdate: true,
                        })
                        .then(() => {
                          resolve(junction.id)
                        })
                    })
                  })(),
                )
              }
            }
            await Promise.all(updatePromise)
          }

          // 操作记录的描述
          let title = ''
          // 统一对日志内容
          switch (name) {
            // 在保存跟车道宽度、数量相关的操作记录时，需要带上对应连通的 junctionId
            case 'updateLaneWidth':
              title = i18n.global.t('actions.lane.modifyWidth')
              break
            case 'addLane':
              title = i18n.global.t('actions.lane.add')
              break
            case 'removeLane':
              title = i18n.global.t('actions.lane.remove')
              break
            case 'addSectionWithTween':
              // TODO 添加平滑过渡的 section 效果，有可能涉及 junction 的连通关系改变
              // historyStore.save({})
              break
            case 'addSectionOnly':
              // TODO 单纯划分 section，不涉及路口处 junction 连通关系改变
              // historyStore.save({})
              break
            default:
              break
          }

          if (title) {
            historyStore.save({
              title,
              roadId: oneRoad.id,
              roads: _roads,
              junctionId: diffJunctionId,
              junctions: _junctions,
              objectId: diffObjectId,
              objects: _objects,
            })
          }
        }

        root3d.core.render()
      })

      onError((err) => {
        console.log(err)
      })
    },
  )

  return unsubscribeRoadStore
}
