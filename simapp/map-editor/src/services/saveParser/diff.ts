import { cloneDeep } from 'lodash'
import { pickUpMapElementData } from './index'
import { useHistoryStore } from '@/stores/history'
import { getOriginMapCache } from '@/utils/tools'
import { getDataById } from '@/utils/business'
import { getRoad } from '@/utils/mapCache'
import { useModelCacheStore } from '@/stores/object/modelCache'

interface IDiffElementData {
  cmdOptions: Array<biz.ICmdOption>
  roads: Array<biz.ICommonRoad>
  lanelinks: Array<biz.ICommonLaneLink>
  objects: Array<biz.ICommonObject>
}

export async function getDiffElementData (): Promise<IDiffElementData | null> {
  // 前置判断，如果没有打开的地图文件，则属于新建保存，直接全量保存，不需要走这个逻辑
  const originMapCache = getOriginMapCache()
  if (!originMapCache) return null

  const currentData = await pickUpMapElementData()

  // 通过 historyStore 中记录的 diff 地图元素 id，来判断哪些地图元素属于增量部分
  const historyStore = useHistoryStore()
  const diffRoadIdSet: Set<string> = new Set()
  const diffJunctionIdSet: Set<string> = new Set()
  const diffObjectIdSet: Set<string> = new Set()

  // 整理操作记录中，记录的 diff 地图元素 id
  for (const record of historyStore.records) {
    const { roadId, junctionId, objectId } = record
    for (const _roadId of roadId) {
      if (!diffRoadIdSet.has(_roadId)) {
        diffRoadIdSet.add(_roadId)
      }
    }
    // 当前存在 diff 差别的路口 id
    // 意味着，modify 的 laneLink 的 junctionId 属性，需要保证其 junctionId 在 Set 集合中存在
    for (const _junctionId of junctionId) {
      if (!diffJunctionIdSet.has(_junctionId)) {
        diffJunctionIdSet.add(_junctionId)
      }
    }

    // 从操作记录中，获取更新了的物体 id
    for (const _objectId of objectId) {
      if (!diffObjectIdSet.has(_objectId)) {
        diffObjectIdSet.add(_objectId)
      }
    }
  }

  // 遍历 road 级别元素，调整增量数据的透传配置
  const diffRoadCmdOption: Array<biz.ICmdOption> = []
  const diffRoadData = []
  // 遍历 object 级别元素，调整增量数据的透传配置
  const diffObjectCmdOption: Array<biz.ICmdOption> = []
  const diffObjectData = []
  for (const roadId of diffRoadIdSet.values()) {
    const currentRoad = getDataById(currentData.roads, roadId)
    const originRoad = getDataById(originMapCache.roads, roadId)
    if (originRoad && currentRoad) {
      // 源数据有，新数据也有，属于更新
      diffRoadCmdOption.push({
        cmd: 'update',
        type: 'road',
        id: roadId,
      })
      diffRoadData.push(currentRoad)

      // 如果当前道路是隧道类型
      const currentStoreRoad = getRoad(roadId)
      if (currentStoreRoad && currentStoreRoad.roadType === 'tunnel') {
        let targetTunnelObject: biz.ICommonObject | null = null
        // 从现有的物体中，找到跟当前道路关联的虚拟隧道物体
        for (const currentObject of currentData.objects) {
          if (targetTunnelObject) continue
          const { name, roadid = '' } = currentObject
          if (name === 'Tunnel' && roadid === roadId) {
            targetTunnelObject = currentObject
          }
        }
        if (targetTunnelObject) {
          // 联动更新对应的隧道虚拟物体
          diffObjectCmdOption.push({
            cmd: 'update',
            type: 'object',
            id: targetTunnelObject.id,
          })
          diffObjectData.push(targetTunnelObject)
        }
      }
    } else if (originRoad && !currentRoad) {
      // 源数据有，新数据没有，属于删除
      diffRoadCmdOption.push({
        cmd: 'delete',
        type: 'road',
        id: roadId,
      })

      // 判断原有的物体列表中，是否有跟当前删除的道路关联的
      let removeTunnelObject: biz.ICommonObject | null = null
      for (const originObject of originMapCache.objects) {
        if (removeTunnelObject) continue
        const { name, roadid = '' } = originObject
        if (name === 'Tunnel' && roadid === roadId) {
          removeTunnelObject = originObject
        }
      }
      // 如果存在关联的虚拟物体，也需要一起删掉
      if (removeTunnelObject) {
        diffObjectCmdOption.push({
          cmd: 'delete',
          type: 'object',
          id: removeTunnelObject.id,
        })
      }
    } else if (!originRoad && currentRoad) {
      // 源数据没有，新数据有，属于新增
      diffRoadCmdOption.push({
        cmd: 'add',
        type: 'road',
        id: roadId,
      })
      diffRoadData.push(currentRoad)

      // 如果是新增了一条隧道
      const currentStoreRoad = getRoad(roadId)
      if (currentStoreRoad && currentStoreRoad.roadType === 'tunnel') {
        let addTunnelObject: biz.ICommonObject | null = null
        // 从现有的物体中，找到跟当前道路关联的虚拟隧道物体
        for (const currentObject of currentData.objects) {
          if (addTunnelObject) continue
          const { name, roadid = '' } = currentObject
          if (name === 'Tunnel' && roadid === roadId) {
            addTunnelObject = currentObject
          }
        }
        if (addTunnelObject) {
          // 联动新增一个隧道的虚拟物体
          diffObjectCmdOption.push({
            cmd: 'add',
            type: 'object',
            id: addTunnelObject.id,
          })
          diffObjectData.push(addTunnelObject)
        }
      }
    } else {
      // 源数据和新数据都没有，没意义
    }
  }

  // 遍历 junction 级别元素，由于 junction 不参与透传，实际是为了获取 laneLink 的增量
  const diffLaneLinkCmdOption: Array<biz.ICmdOption> = []
  const diffLaneLinkData = []
  // 当前数据中，遍历过的 laneLinkId 集合
  const traversedLinkIdSet = new Set()
  // 只有 junction 存在增量才会取 laneLink 的增量数据
  if (diffJunctionIdSet.size) {
    // 先遍历源数据的
    for (const oLaneLink of originMapCache.lanelinks) {
      const { id, junctionid } = oLaneLink
      // 只处理在 diff junction 中的 laneLink
      if (!diffJunctionIdSet.has(junctionid)) continue

      const currentLaneLink = getDataById(currentData.lanelinks, id)

      // 如果当前 link 存在
      if (currentLaneLink) {
        // 源数据有，新数据也有，属于更新
        diffLaneLinkCmdOption.push({
          cmd: 'update',
          type: 'lanelink',
          id,
        })
        diffLaneLinkData.push(currentLaneLink)
        // 缓存当前数据中已遍历的 laneLinkId
        traversedLinkIdSet.add(id)
      } else {
        // 源数据有，新数据没有，属于删除
        diffLaneLinkCmdOption.push({
          cmd: 'delete',
          type: 'lanelink',
          id,
        })
      }
    }
    // 再遍历当前数据的
    for (const cLaneLink of currentData.lanelinks) {
      const { id, junctionid } = cLaneLink
      // 只处理在 diff junction 中的 laneLink
      if (!diffJunctionIdSet.has(junctionid)) continue
      // 再排除掉之前已经处理过的 laneLink
      if (traversedLinkIdSet.has(id)) continue

      // 剩下的部分，即为 diff junction 中，源数据 laneLink 中没有的，属于新增
      diffLaneLinkCmdOption.push({
        cmd: 'add',
        type: 'lanelink',
        id,
      })
      diffLaneLinkData.push(cLaneLink)
    }
  }

  for (const objectId of diffObjectIdSet.values()) {
    const currentObject = getDataById(currentData.objects, objectId)
    const originObject = getDataById(originMapCache.objects, objectId)
    if (originObject && currentObject) {
      // 源数据有，新数据也有，属于更新
      diffObjectCmdOption.push({
        cmd: 'update',
        type: 'object',
        id: objectId,
      })
      diffObjectData.push(currentObject)
    } else if (originObject && !currentObject) {
      // 源数据有，新数据没有，属于删除
      diffObjectCmdOption.push({
        cmd: 'delete',
        type: 'object',
        id: objectId,
      })
    } else if (!originObject && currentObject) {
      // 源数据没有，新数据有，属于新增
      diffObjectCmdOption.push({
        cmd: 'add',
        type: 'object',
        id: objectId,
      })
      diffObjectData.push(currentObject)
    }
  }

  // 手动清理三个 Set
  diffRoadIdSet.clear()
  diffJunctionIdSet.clear()
  diffObjectIdSet.clear()
  traversedLinkIdSet.clear()

  const allCmdOptions = [
    ...diffRoadCmdOption,
    ...diffLaneLinkCmdOption,
    ...diffObjectCmdOption,
  ]

  // 在左侧面板中删除自定义模型时（这一步不会被 historyStore 记录，因此不会参与 records diff 的比较，需要单独添加），场景内的物体被删除了，也需要有对应增量操作。
  const modelCacheStore = useModelCacheStore()
  if (modelCacheStore.hasDeletedObjWhenDeletingCustomModel) {
    modelCacheStore.deletedObjId.forEach((id) => {
      allCmdOptions.push({ id, cmd: 'delete', type: 'object' })
    })
  }

  // 如果 diff 后发现没有更新，则返回空
  if (allCmdOptions.length < 1) {
    return null
  }
  return {
    cmdOptions: allCmdOptions,
    roads: diffRoadData as Array<biz.ICommonRoad>,
    lanelinks: diffLaneLinkData as Array<biz.ICommonLaneLink>,
    objects: diffObjectData as Array<biz.ICommonObject>,
  }
}

// 将 diff 的数据，通过 add, update, delete 的配置项，更新本地缓存的源地图数据
export function updateOriginMapCacheByDiff (data: IDiffElementData) {
  // 前置判断，如果没有打开的地图文件，则属于新建保存，直接全量保存，不需要走这个逻辑
  const originMapCache = getOriginMapCache()
  if (!originMapCache) return

  const { cmdOptions, roads, lanelinks, objects } = data
  for (const option of cmdOptions) {
    const { cmd, type: elementType, id: objectId } = option
    // 取的是源数据数组引用
    let originElements: Array<biz.ICommonElement>
    let targetElements: Array<biz.ICommonElement>
    if (elementType === 'road') {
      originElements = originMapCache.roads
      targetElements = roads
    } else if (elementType === 'lanelink') {
      originElements = originMapCache.lanelinks
      targetElements = lanelinks
    } else if (elementType === 'object') {
      originElements = originMapCache.objects
      targetElements = objects
    }

    // @ts-expect-error
    if (!originElements || !targetElements) continue

    if (cmd === 'add') {
      const targetItem = targetElements.find(data => data.id === objectId)
      if (targetItem) {
        // 向源数据中插入新的数据
        originElements.push(cloneDeep(targetItem))
      }
    } else if (cmd === 'delete') {
      const index = originElements.findIndex(data => data.id === objectId)
      if (index > -1) {
        // 将目标数据从源数据的记录中删除
        originElements.splice(index, 1)
      }
    } else if (cmd === 'update') {
      const targetItem = targetElements.find(data => data.id === objectId)
      const originIndex = originElements.findIndex(
        data => data.id === objectId,
      )
      if (targetItem && originIndex > -1) {
        // 将目标数据，替换到源数据中对应位置的旧数据
        originElements[originIndex] = cloneDeep(targetItem)
      }
    }
  }
}
