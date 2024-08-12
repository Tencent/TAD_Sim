// 跟 object 元素相关的通用算法
import { CatmullRomCurve3, Vector2, Vector3 } from 'three'
import { useObjectStore } from './index'
import {
  CatmullromTension,
  CurveType,
  fixedPrecision,
  getClosestPointFromCurvePath,
  getTValue,
  getValidRadInRange,
  getVerticalVector,
  getWorldLocation,
  halfPI,
  transformVec3ByObject,
} from '@/utils/common3d'
import { getJunction, getObject, getRoad, setObject } from '@/utils/mapCache'
import root3d from '@/main3d'

// 判断某一个自定义模型是否在地图场景中有使用
export function isCustomModelInUsed (modelName: string) {
  // 判断当前场景中，是否有待删除的自定义模型
  let modelInUsed = false
  const objectStore = useObjectStore()
  for (const id of objectStore.ids) {
    if (modelInUsed) continue

    const objectData = getObject(id)
    if (!objectData) continue

    const { name, mainType } = objectData
    if (mainType === 'customModel' && name === modelName) {
      modelInUsed = true
    }
  }

  return modelInUsed
}

// 获取道路的section中，百分比对应的目标section的车道总宽度
export function getRoadWidth (params: { road: biz.IRoad, percent: number }) {
  const { road, percent } = params
  const { sections } = road
  // 通过百分比获取目标的 section
  let targetSection: biz.ISection | null = null
  for (const section of sections) {
    if (targetSection) continue

    const { pStart, pEnd } = section
    if (percent >= pStart && percent <= pEnd) {
      targetSection = section
    }
  }
  // 如果比例超出 [0, 1] 的范围
  if (!targetSection) {
    if (percent < 0) {
      targetSection = sections[0]
    } else if (percent > 1) {
      targetSection = sections[sections.length - 1]
    }
  }

  if (!targetSection) return 0

  let forwardLaneWidth = 0
  let reverseLaneWidth = 0
  targetSection.lanes.forEach((lane) => {
    if (Number(lane.id) <= 0) {
      // 正向车道
      forwardLaneWidth += lane.normalWidth
    } else {
      // 反向车道
      reverseLaneWidth += lane.normalWidth
    }
  })

  return {
    forwardWidth: forwardLaneWidth,
    reverseWidth: reverseLaneWidth,
  }
}

/**
 * 获取除 rsu 以外的所有传感器设备的列表
 */
export function getAllSensorExceptRsu (ids?: Array<string>) {
  let _ids = []
  if (ids && ids.length > 0) {
    _ids = ids
  } else {
    _ids = useObjectStore().ids
  }

  const sensorList: Array<biz.ISensor> = []
  for (const id of _ids) {
    const object = getObject(id)
    if (!object) continue
    if (object.mainType === 'sensor') {
      sensorList.push(object as biz.ISensor)
    }
  }

  return sensorList
}

/**
 * 获取所有 rsu 设备的列表
 * @param ids 待遍历的物体元素 id 列表
 */
export function getAllRsuId (ids?: Array<string>) {
  let _ids = []
  if (ids && ids.length > 0) {
    _ids = ids
  } else {
    // 如果没有传入 id，则使用 objectStore 中的所有 id 集合
    _ids = useObjectStore().ids
  }
  const rsuId = []
  for (const id of _ids) {
    const object = getObject(id)
    if (!object) continue
    if (object.mainType === 'sensor' && object.name === 'RSU') {
      // 只获取 rsu 设备
      rsuId.push(id)
    }
  }
  return rsuId
}

/**
 * 获取一个信号灯，自动填充的信控配置（不一定准确，需要用户 double check）
 * 如果信号灯的信控配置为空，则将计算好的控制路口或道路，直接赋值挂载到数据结构中
 * @param lightId
 */
export function applyDefaultControlJunctionAndRoad (lightId: string) {
  const lightData = getObject(lightId) as biz.ITrafficLight
  if (!lightData) return
  // 如果控制的路口和道路都存在，则直接返回
  const { controlRoadId, controlJunctionId, poleId, onVerticalPole, position } =
    lightData
  if (controlJunctionId && controlRoadId) return

  // 获取杆的数据
  const poleData = getObject(poleId) as biz.IPole
  if (!poleData) return

  // 如果已经有配置好的参数，则直接使用
  let _controlJunctionId = controlJunctionId || ''
  let _controlRoadId = controlRoadId || ''

  // 如果没有控制的路口，需要基于杆所在的位置来计算
  if (!_controlJunctionId) {
    // 先计算控制的路口
    if (poleData.junctionId) {
      // 如果杆是放在路口中的，则默认控制这个路口
      _controlJunctionId = poleData.junctionId
    } else if (poleData.roadId) {
      // 如果是放置在道路上的，需要根据当前杆在道路上的比例来判断是取道路首还是尾连接的路口
      const road = getRoad(poleData.roadId)
      if (road) {
        // 获取此时道路连接路口的关系
        const { length: roadLength, linkJunction } = road
        if (linkJunction.length === 1) {
          // 如果只连接了一个路口，则直接使用单独的路口
          _controlJunctionId = linkJunction[0]
        } else if (linkJunction.length > 1) {
          // 当前杆在道路参考线的比例
          const polePercent = fixedPrecision(poleData.s / roadLength)
          // 在比例 [0, 0.5] 区间范围内，都认为是跟道路首端的路口关联
          const isHead = polePercent <= 0.5
          let _controlJunctionId = ''
          for (const _linkJunctionId of linkJunction) {
            // 如果已经找到了第一个匹配的路口了，则无须继续遍历了
            if (_controlJunctionId) continue
            const junction = getJunction(_linkJunctionId)
            if (!junction) continue
            for (const linkRoad of junction.linkRoads) {
              const [junctionLinkRoadId, percent] = linkRoad.split('_')
              if (junctionLinkRoadId === poleData.roadId) {
                if (
                  (percent === '0' && isHead) ||
                  (percent === '1' && !isHead)
                ) {
                  // 道路 id 和首尾的比例也能匹配上
                  _controlJunctionId = _linkJunctionId
                }
              }
            }
          }
          if (!_controlJunctionId) {
            // 如果遍历后依然没有找到匹配的路口，则默认使用道路第一个连接的路口
            _controlJunctionId = linkJunction[0]
          }
        }

        // 如果道路没有连接任何路口，则不需要绑定为空
      }
    }
  }

  // 如果已经有控制的道路，则不用计算默认的
  if (!_controlRoadId) {
    if (poleData.roadId) {
      // 如果杆放在在道路上，则默认使用杆所在的道路
      _controlRoadId = poleData.roadId
    } else if (poleData.junctionId && _controlJunctionId) {
      // 如果杆放在路口中，且有控制的路口，则直接基于控制路口连通的道路来判断
      const targetJunction = getJunction(_controlJunctionId)
      // 获取信号灯世界坐标系坐标
      const worldPosition = getWorldLocation({
        objectId: lightData.id,
        parent: root3d.mapElementsContainer,
      })
      // 信号灯所在杆的世界坐标系坐标
      const poleWorldPosition = getWorldLocation({
        objectId: poleId,
        parent: root3d.mapElementsContainer,
      })

      if (targetJunction && worldPosition && poleWorldPosition) {
        const wp = new Vector2(worldPosition.z, worldPosition.x)

        let lightDirection = new Vector3()
        // 计算信号灯在世界坐标系下的朝向
        if (onVerticalPole) {
          // 在竖杆上
          lightDirection = worldPosition
            .clone()
            .sub(poleWorldPosition)
            .normalize()
        } else {
          // 在横杆上
          const poleGroup = root3d.mapElementsContainer.getObjectByProperty(
            'objectId',
            poleId,
          )
          if (!poleGroup) return

          // 先更新杆容器的世界矩阵
          poleGroup.updateMatrixWorld()
          // 横杆水平延伸部分中心的世界坐标
          const posOnPole = new Vector3(0, position.y, position.z)
          // 将横杆中心的坐标，转换成世界坐标系下的坐标
          posOnPole.applyMatrix4(poleGroup.matrixWorld)
          // 横杆上信号灯的朝向
          lightDirection = worldPosition.clone().sub(posOnPole).normalize()
        }
        // 信号灯的平面朝向
        const wd = new Vector2(lightDirection.z, lightDirection.x)
        // 信号灯的平面朝向的反向向量（用于跟道路行驶方向来计算夹角匹配最优的控制道路）
        const reverseWd = wd.clone().negate()
        const basicAngle = reverseWd.angle()
        const matchOption = {
          roadId: '', // 匹配的道路 id
          angle: 0, // 偏差的角度
        }

        for (const refRoad of targetJunction.refRoads) {
          const { leftPoint, rightPoint, roadId } = refRoad
          const rp = new Vector2(rightPoint.z, rightPoint.x)
          const lp = new Vector2(leftPoint.z, leftPoint.x)
          // 左右两个点的中心点
          const center = rp.clone().add(lp).addScalar(0.5)
          // 获取每一条车道的行驶方向向量
          const vertical = rp.clone().sub(lp).normalize()
          const direction = new Vector2(-vertical.y, vertical.x)

          // 从道路指向信号灯位置的方向
          const roadToLight = wp.clone().sub(center).normalize()

          // 先判断行驶方向是否同向
          const dotRes = roadToLight.dot(direction)
          // 如果是反向说明不符合
          if (dotRes < 0) continue

          // 计算信号灯方向，跟道路行驶方向的夹角
          const angle = direction.angle()
          // 计算跟灯朝向的绝对角度差
          const deltaAngle = Math.abs(getValidRadInRange(basicAngle - angle))

          // 第一次匹配，或匹配到更优的道路，则更新
          if (!matchOption.roadId || matchOption.angle > deltaAngle) {
            matchOption.roadId = roadId
            matchOption.angle = deltaAngle
          }
        }

        if (matchOption && matchOption.roadId) {
          _controlRoadId = matchOption.roadId
        }
      }
    }
  }

  // 将计算好的默认值赋值给信号灯
  lightData.controlJunctionId = _controlJunctionId
  lightData.controlRoadId = _controlRoadId
}

/**
 * 获取一个点到距离车道中心线最近的点
 * @param params
 */
export function getClosestPointInLane (params: {
  lane: biz.ILane
  targetPoint: common.vec3
}) {
  const { lane, targetPoint } = params
  // 获取当前车道中心线采样点的对应曲线
  const { samplePoints } = lane
  const laneCenterCurvePath = new CatmullRomCurve3(
    transformVec3ByObject(samplePoints),
  )
  laneCenterCurvePath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    laneCenterCurvePath.tension = CatmullromTension
  }

  // 鼠标射线交互的点，到车道中心线的最近点
  return getClosestPointFromCurvePath({
    curvePath: laneCenterCurvePath,
    point: targetPoint,
  })
}

// 创建默认的物体数据结构
export function createDefaultObjectData (): biz.IObject {
  return {
    id: '',
    name: '',
    showName: '', // 展示的名称，主要在前端使用
    mainType: '', // 大的分类，主要在前端侧使用
    type: '',
    subtype: '',
    roadId: '',
    sectionId: '',
    laneId: '',
    junctionId: '',
    laneLinkId: '',
    position: { x: 0, y: 0, z: 0 },
    scale: { x: 0, y: 0, z: 0 },
    yaw: 0,
    angle: 0,
    s: 0,
    t: 0,
    closestPoint: null,
    closestPointTangent: null,
    closestPointPercent: 0,
    lookAtPoint: null,
    radius: 0,
    width: 0,
    height: 0,
    length: 0,
  }
}

// 同步物体的尺寸
export function syncObjectSize (params: {
  objectId: string
  radius?: number
  width?: number
  height?: number
  length?: number
  basicWidth?: number
  basicLength?: number
  basicHeight?: number
}) {
  const {
    objectId,
    radius,
    width,
    height,
    length,
    basicWidth,
    basicLength,
    basicHeight,
  } = params
  const objectData = getObject(objectId)
  if (!objectData) return
  if (radius) {
    objectData.radius = radius
  }
  if (width) {
    objectData.width = width
  }
  if (height) {
    objectData.height = height
  }
  if (length) {
    objectData.length = length
  }
  if (
    objectData.mainType === 'roadSign' ||
    objectData.mainType === 'other' ||
    objectData.mainType === 'customModel'
  ) {
    const _objectData = objectData as
      | biz.IOther
      | biz.IRoadSign
      | biz.ICustomModel
    if (basicWidth) {
      _objectData.basicWidth = basicWidth
      // 如果默认的尺寸不存在（或不合理），用原始尺寸代替
      if (objectData.width === 0) {
        _objectData.width = basicWidth
      }
    }
    if (basicLength) {
      _objectData.basicLength = basicLength
      if (objectData.length === 0) {
        _objectData.length = basicLength
      }
    }
    if (basicHeight) {
      _objectData.basicHeight = basicHeight
      if (objectData.height === 0) {
        _objectData.height = basicHeight
      }
    }
  }
  setObject(objectId, objectData)
}

// 获取某一根杆上的物体
export function getObjectOnPole (params: {
  poleId: string
  ids: Array<string>
}) {
  const { poleId: targetPoleId, ids } = params
  const objects = []
  for (const _objectId of ids) {
    if (_objectId === targetPoleId) continue
    const objectData = getObject(_objectId)
    if (!objectData) continue

    // 如果当前物体有杆的 id，并且跟当前杆 id 一致
    const { poleId = '' } = objectData as
      | biz.ISignalBoard
      | biz.ITrafficLight
      | biz.ISensor
    if (poleId === targetPoleId) {
      objects.push(objectData)
    }
  }
  return objects
}

// 获取跟某一条道路关联的物体
export function getObjectInRoad (params: {
  roadId: string
  ids: Array<string>
}) {
  const { roadId: targetRoadId, ids } = params
  if (!targetRoadId || targetRoadId === '-1') return []

  // 跟道路关联的物体集合（杆上的标志牌、信号灯也绑定了 roadId）
  const objects = []

  // 先处理跟道路关联的物体
  for (const _objectId of ids) {
    const objectData = getObject(_objectId)
    if (!objectData) continue

    const { roadId = '' } = objectData
    if (roadId === targetRoadId) {
      // 如果物体绑定的 roadId 跟目标道路一致，则缓存起来
      objects.push(objectData)
    }
  }

  return objects
}

// 获取跟某一个路口关联的物体
export function getObjectInJunction (params: {
  junctionId: string
  ids: Array<string>
}) {
  const { junctionId: targetJunctionId, ids } = params
  if (!targetJunctionId || targetJunctionId === '-1') return []

  const objects = []
  for (const _objectId of ids) {
    const objectData = getObject(_objectId)
    if (!objectData) continue

    const { junctionId = '' } = objectData
    if (junctionId === targetJunctionId) {
      objects.push(objectData)
    }
  }
  return objects
}

// 计算鼠标射线交互点在道路上的计算后位置信息
export function getLocationInRoad (params: {
  point: common.vec3
  roadId: string
  useElevation?: boolean // 是否使用高程曲线对应的切线分量（杆不适用）
  crossBorder?: boolean // 是否支持跨越道路参考线的范围
  crossBorderRange?: number // 支持跨越的距离
}): biz.IRefLineLocation | null {
  const {
    roadId,
    point,
    useElevation = false,
    crossBorder = false,
    crossBorderRange: range = 0,
  } = params
  const intersectionPoint = new Vector3(point.x, point.y, point.z)
  const road = getRoad(roadId)
  if (!road) return null

  const { keyPath } = road
  // 道路参考线的原始长度
  const originPathLength = keyPath.getLength()
  const {
    point: originClosestPoint,
    tangent: originTangent,
    percent: originPercent,
  } = getClosestPointFromCurvePath({
    curvePath: keyPath,
    point: intersectionPoint,
  })

  const closestPoint: Vector3 = originClosestPoint
  const tangent: Vector3 = originTangent
  const percent: number = originPercent
  let s: number = fixedPrecision(road.length * percent) // s 坐标
  // 计算交互点跟参考线上最近目标点的距离，作为 t 坐标
  let t: number = fixedPrecision(
    getTValue({
      tangent,
      refLinePoint: closestPoint,
      targetPoint: intersectionPoint,
    }),
  )

  // 如果percent比例很接近 0 或 1，说明物体放置的位置接近边界范围
  // 【核心】此时可以基于原有的路径，首尾沿切线延长线拓展指定的范围，重新计算到路径的投影关键位置
  if (crossBorder && range) {
    if (percent < 0.05 || percent > 0.95) {
      // 首部的切线方向
      const headTangent = keyPath.getTangentAt(0).normalize()
      const headPoint = keyPath.getPointAt(0)
      // 在首部切线方向的延长点
      const headAlongPoint = headPoint
        .clone()
        .addScaledVector(headTangent, -range)
      // 尾部的切线方向
      const tailTangent = keyPath.getTangentAt(1).normalize()
      const tailPoint = keyPath.getPointAt(1)
      // 在尾部切线方向的延长点
      const tailAlongPoint = tailPoint
        .clone()
        .addScaledVector(tailTangent, range)

      // 基于原有的道路参考线，拓展延长点重新创建一条虚拟的参考线，范围更大
      const points = transformVec3ByObject(keyPath.points)
      points.unshift(headAlongPoint)
      points.push(tailAlongPoint)

      const newPath = new CatmullRomCurve3(points)
      newPath.curveType = CurveType
      if (CurveType === 'catmullrom') {
        newPath.tension = CatmullromTension
      }
      // 新路径的长度
      const newPathLength = newPath.getLength()

      const {
        point: newClosestPoint,
        tangent: newTangent,
        percent: newPercent,
      } = getClosestPointFromCurvePath({
        curvePath: newPath,
        point: intersectionPoint,
      })
      // 如果新的比例是偏向头部
      const offset = newPathLength * newPercent
      const newS = offset - range
      const newT = fixedPrecision(
        getTValue({
          tangent: newTangent,
          refLinePoint: newClosestPoint,
          targetPoint: intersectionPoint,
        }),
      )

      // 基于新的虚拟路径计算出来的位置属性，更新原有的属性
      s = newS
      t = newT
    }
  }

  // 切线是否应用高程的坡度
  if (useElevation && road.elevationPath) {
    const elevationTangent = road.elevationPath.getTangentAt(percent)
    // 先将原切向向量归一化
    tangent.normalize()
    // 计算高度分量跟s轴分量的占比（x不可能为0）
    const ratio = elevationTangent.y / elevationTangent.x
    tangent.y = ratio
    // 调整高度的分量后，再次归一化
    tangent.normalize()

    // 应用高程，对应的道路参考线上最近的投影点也需要更新高度
    const elevationPoint = road.elevationPath.getPointAt(percent)
    closestPoint.y = elevationPoint.y
  }

  // 将切线方向，绕投影点旋转 90°得到此处的法向量（不管道路是否有高程坡度，都有该法向量）
  // 在道路平面内的垂直向量
  const vertical = getVerticalVector(tangent).normalize()
  const normal = tangent.clone()
  // 切线方向绕投影点的垂直方向旋转 90°
  normal.applyAxisAngle(vertical, halfPI).normalize()

  return {
    s,
    t,
    tangent,
    percent,
    closestPointOnRefLine: closestPoint,
    normal,
  }
}

// 计算沿着道路方向前方的点
export function getAlongRoadFrontPoint (params: {
  point: common.vec3
  tangent: common.vec3
  isForward?: boolean // 是否是正向车道
}) {
  const { point, tangent, isForward = true } = params
  const _point = new Vector3(point.x, point.y, point.z)
  const _tangent = new Vector3(tangent.x, tangent.y, tangent.z)
  const frontPoint = _point
    .clone()
    .addScaledVector(_tangent, isForward ? 1 : -1)
  return frontPoint
}
