// 信号灯特有的方法
import { getJunction, getObject, getRoad, setObject } from '@/utils/mapCache'
import { getTrafficLightConfigByName } from '@/config/trafficLight'
import { orderByAsc } from '@/utils/common3d'

// 更新信号灯的灯态
export function updateTrafficLightType (params: {
  objectId: string
  typeName: string
}) {
  const { objectId, typeName } = params
  const lightData = getObject(objectId) as biz.ITrafficLight
  if (!lightData) return
  const newConfig = getTrafficLightConfigByName(typeName)
  if (!newConfig) return

  const { name, showName, type, subtype } = newConfig

  lightData.name = name
  lightData.showName = showName
  lightData.type = type
  lightData.subtype = subtype

  setObject(lightData.id, lightData)

  return lightData
}
// 更新信号灯控制的道路
export function updateTrafficLightControlRoad (params: {
  objectId: string
  roadId: string
}) {
  const { objectId, roadId } = params
  const lightData = getObject(objectId) as biz.ITrafficLight
  if (!lightData) return

  // 可能传入空的 roadId，说明清空了控制的道路
  lightData.controlRoadId = roadId

  setObject(lightData.id, lightData)

  return lightData
}
// 更新信号灯控制的路口
export function updateTrafficLightControlJunction (params: {
  objectId: string
  junctionId: string
}) {
  const { objectId, junctionId } = params
  const lightData = getObject(objectId) as biz.ITrafficLight
  if (!lightData) return

  // 可能传入空的 junctionId，说明清空了控制的路口
  lightData.controlJunctionId = junctionId

  // 在更新控制路口时，判断当前控制的道路是否跟目标路口连通，如果不连通则重置控制的道路
  const junction = getJunction(junctionId)
  if (junction) {
    // 当前路口是否有目标的控制道路
    let hasControlRoad = false
    for (const linkRoad of junction.linkRoads) {
      if (hasControlRoad) continue
      const [roadId] = linkRoad.split('_')
      if (lightData.controlRoadId && lightData.controlRoadId === roadId) {
        hasControlRoad = true
      }
    }

    if (!hasControlRoad) {
      // 如果没有目标控制的道路，则直接重置
      lightData.controlRoadId = ''
    }
  }

  setObject(lightData.id, lightData)

  return lightData
}

// 主动更新信控配置。由于道路结构的变化，会导致已绑定的控制道路和路口数据不一定准确
// 在保存前也要调用！！
export function updateTrafficLightSignalControl (id: string | Array<string>) {
  // 统一转换成数组来处理
  if (!Array.isArray(id)) {
    id = [id]
  }

  for (const _id of id) {
    const lightData = getObject(_id) as biz.ITrafficLight
    if (!lightData) continue
    const { mainType } = lightData
    // 如果不是信号灯，则跳过
    if (mainType !== 'trafficLight') continue

    const { controlRoadId, controlJunctionId } = lightData
    if (controlJunctionId) {
      const junction = getJunction(controlJunctionId)
      if (!junction) {
        // 信号灯绑定了控制的路口，但是没找到对应的路口数据，需要取消绑定
        lightData.controlJunctionId = ''
      }
    }
    if (controlRoadId) {
      const road = getRoad(controlRoadId)
      if (!road) {
        // 信号灯绑定了控制的道路，但是没找到对应的道路数据，需要取消绑定
        lightData.controlRoadId = ''
      }
    }

    setObject(lightData.id, lightData)
  }
}

// 获取某一个信号灯可控制的所有备选道路和路口集合
export function getOptionalControlRoadAndJunction (lightId: string) {
  // 有效的备选道路和路口集合
  const _roadIds: Array<string> = []
  const _junctionIds: Array<string> = []
  const lightData = getObject(lightId) as biz.ITrafficLight
  if (!lightData) {
    return {
      roadId: _roadIds,
      junctionId: _junctionIds,
    }
  }

  const { roadId, junctionId, controlJunctionId } = lightData

  if (controlJunctionId) {
    // 如果已经有控制的路口
    const controlJunction = getJunction(controlJunctionId)
    if (controlJunction) {
      // 备选道路只能是目标路口连通的道路
      _roadIds.push(
        ...controlJunction.linkRoads.map(linkRoad => linkRoad.split('_')[0]),
      )
      // 备选的路口
      if (junctionId) {
        // 当前路口即为备选路口
        _junctionIds.push(junctionId)
      } else {
        const road = getRoad(roadId)
        if (road) {
          // 当前道路连通的路口作为备选
          _junctionIds.push(...road.linkJunction)
        }
      }
    }
  } else {
    // 如果没有已经控制的路口

    // 如果信号灯所在的杆，是放置在路口中的
    if (junctionId) {
      const junction = getJunction(junctionId)
      if (junction) {
        // 当前路口即为备选路口
        _junctionIds.push(junctionId)
        // 获取当前路口连通的所有道路
        _roadIds.push(
          ...junction.linkRoads.map(linkRoad => linkRoad.split('_')[0]),
        )
      }
    } else {
      // 如果信号的所在的杆，是放置在道路上的，需要获取当前道路连通的路口
      const road = getRoad(roadId)
      if (road) {
        // 当前道路连通的路口作为备选
        _junctionIds.push(...road.linkJunction)
        if (_junctionIds.length < 1) {
          // 会存在孤立道路没有连接路口的情况
          // 则信号灯所处的道路，作为可控制的备选道路
          _roadIds.push(roadId)
        } else {
          // 获取备选路口所连通的道路
          for (const _junctionId of _junctionIds) {
            const junction = getJunction(_junctionId)
            if (junction) {
              _roadIds.push(
                ...junction.linkRoads.map(linkRoad => linkRoad.split('_')[0]),
              )
            }
          }
        }
      }
    }
  }

  // 按照id从小到大排序
  _roadIds.sort(orderByAsc)
  _junctionIds.sort(orderByAsc)

  return {
    roadId: _roadIds,
    junctionId: _junctionIds,
  }
}
