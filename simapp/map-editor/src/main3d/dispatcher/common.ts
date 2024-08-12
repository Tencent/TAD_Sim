import root3d from '../index'
import { reRenderCustomModel } from '../render/customModel'
import { useObjectStore } from '@/stores/object'
import { getObjectInRoad } from '@/stores/object/common'
import { getObject } from '@/utils/mapCache'
import { useModelCacheStore } from '@/stores/object/modelCache'

// 更新道路结构时，联动地更新道路上所有物体
export function updateObjectInRoad (roadId: Array<string>) {
  // 跟道路关联的原始物体集合
  const originObjects: Array<biz.IObject> = []
  const objectStore = useObjectStore()
  for (const _roadId of roadId) {
    const objectOnRoad = getObjectInRoad({
      roadId: _roadId,
      ids: objectStore.ids,
    })
    originObjects.push(...objectOnRoad)
  }

  // 更新了属性后的物体集合
  const objects: Array<biz.IObject> = []
  // 遍历跟道路关联的物体，重新计算部分物体新的位置
  for (const _object of originObjects) {
    const { mainType, id: objectId, s, t } = _object
    // 判断大类型如果是杆、路面标识、other，则需要基于原有的 st 坐标更新物体的位置
    if (mainType === 'pole') {
      const poleId = objectStore.movePoleByST({
        objectId,
        s,
        t,
        // 当前更新位置方法不单独触发记录保存的逻辑
        saveRecord: false,
      })
      if (poleId) {
        const poleData = getObject(poleId)
        if (poleData) {
          objects.push(poleData)
        }
      }
    } else if (mainType === 'roadSign') {
      const roadSignId = objectStore.moveRoadSignByST({
        objectId,
        s,
        t,
        // 当前更新位置方法不单独触发记录保存的逻辑
        saveRecord: false,
      })
      if (roadSignId) {
        const roadSignData = getObject(roadSignId)
        if (roadSignData) {
          objects.push(roadSignData)
        }
      }
    } else if (mainType === 'parkingSpace') {
      const parkingSpaceId = objectStore.moveParkingSpaceByST({
        objectId,
        s,
        t,
        saveRecord: false,
      })
      if (parkingSpaceId) {
        const parkingSpaceData = getObject(parkingSpaceId)
        if (parkingSpaceData) {
          objects.push(parkingSpaceData)
        }
      }
    } else if (mainType === 'other') {
      const otherId = objectStore.moveOtherByST({
        objectId,
        s,
        t,
        // 当前更新位置方法不单独触发记录保存的逻辑
        saveRecord: false,
      })
      if (otherId) {
        const otherData = getObject(otherId)
        if (otherData) {
          objects.push(otherData)
        }
      }
    } else if (mainType === 'customModel') {
      const customModelId = objectStore.moveCustomModelByST({
        objectId,
        s,
        t,
        // 当前更新位置方法不单独触发记录保存的逻辑
        saveRecord: false,
      })
      if (customModelId) {
        const modelData = getObject(customModelId)
        if (modelData) {
          objects.push(modelData)
        }
      }
    } else if (
      mainType === 'signalBoard' ||
      mainType === 'trafficLight' ||
      mainType === 'sensor'
    ) {
      // 如果大类型是标志牌和信号灯，则在杆容器中的相对属性没有变化，可直接缓存到 diff 数组中
      objects.push(_object)
    }
  }

  // 物体的 id 集合
  const objectId = objects.map(objectData => objectData.id)

  return {
    objects,
    objectId,
  }
}

// 删除道路时，联动地删除道路上所有的物体
export function removeObjectInRoad (roadId: Array<string>) {
  // 跟道路关联的物体集合
  const objects: Array<biz.IObject> = []
  const objectStore = useObjectStore()
  for (const _roadId of roadId) {
    const objectOnRoad = getObjectInRoad({
      roadId: _roadId,
      ids: objectStore.ids,
    })
    objects.push(...objectOnRoad)
  }

  // 删除跟当前道路关联的物体
  for (const _object of objects) {
    const { mainType, id: objectId } = _object
    switch (mainType) {
      case 'pole':
        // 删除杆时，会联动删除杆上的物体（后续对于标志牌和信号灯的删除，其实没必要单独执行）
        objectStore.removePole(objectId, false)
        break
      case 'roadSign':
        objectStore.removeRoadSign(objectId, false)
        break
      case 'parkingSpace':
        objectStore.removeParkingSpace(objectId, false)
        break
      case 'other':
        objectStore.removeOther(objectId, false)
        break
      case 'signalBoard':
        objectStore.removeSignalBoard(objectId, false)
        break
      case 'trafficLight':
        objectStore.removeTrafficLight(objectId, false)
        break
      // 自定义导入的模型
      case 'customModel':
        objectStore.removeCustomModel(objectId, false)
        break
      default:
        break
    }
  }
  // 删除物体的 id 集合
  const objectId = objects.map(objectData => objectData.id)

  return {
    objects,
    objectId,
  }
}

// 跟自定义模型配置相关的事件触发
export function initModelCacheDispatcher () {
  const modelCacheStore = useModelCacheStore()

  const unsubscribeModelCacheStore = modelCacheStore.$onAction(
    ({ name, store, args, after, onError }) => {
      after(async (res) => {
        switch (name) {
          case 'updateCustomModelConfig': {
            const ids = res as Array<string>
            const renderPromises = []
            for (const id of ids) {
              const objectData = getObject(id) as biz.ICustomModel
              if (!objectData) continue
              renderPromises.push(
                (() => {
                  return reRenderCustomModel({
                    data: objectData,
                    parent: root3d.mapElementsContainer,
                  })
                })(),
              )
            }

            await Promise.all(renderPromises)

            root3d.core.render()
            // 由于 fbx 模型中有加载纹理，再调用一次定时的重绘
            root3d.core.renderByTimer()
            break
          }
          default: {
            break
          }
        }
      })
      onError((err) => {
        console.log(err)
      })
    },
  )

  return unsubscribeModelCacheStore
}
