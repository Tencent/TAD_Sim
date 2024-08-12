import { CatmullRomCurve3, MathUtils, Vector2, Vector3 } from 'three'
import pool from '../worker'
import { type IRoadSignConfig, getRoadSignConfig } from '@/config/roadSign'
import {
  type ISignalBoardConfig,
  getSignalBoardConfig,
} from '@/config/signalBoard'
import {
  type ITrafficLightConfig,
  getTrafficLightConfig,
} from '@/config/trafficLight'
import { type IOtherConfig, getOtherConfig } from '@/config/other'
import { type IPoleConfig, getPoleConfig } from '@/config/pole'
import {
  type IParkingSpaceConfig,
  getParkingSpaceConfig,
} from '@/config/parkingSpace'
import { getJunction, getObject, getRoad, setObject } from '@/utils/mapCache'
import {
  createDefaultObjectData,
  getAlongRoadFrontPoint,
} from '@/stores/object/common'
import {
  CatmullromTension,
  CurveType,
  DefaultColor,
  PI,
  fixedPrecision,
  getLocationByST,
  getLookAtPointByAngle,

  getLookAtPointByYaw,
  getValidDegreeInRange,
  halfPI,
  transformVec3,
  transformVec3ByObject,
} from '@/utils/common3d'
import { getBoundaryById } from '@/stores/road/boundary'
import { Constant, getDataById } from '@/utils/business'
import {
  type ISensorConfig,
  getCameraParams,
  getLidarParams,
  getRadarParams,
  getRsuParams,
  getSensorConfig,
} from '@/config/sensor'
import { getOriginMapCache, uppercaseFirstChar } from '@/utils/tools'
import { globalConfig } from '@/utils/preset'
import { parseGuideLaneLine } from '@/stores/object/customRoadSign/guideLaneLine'
import { parseVariableDirectionLaneLine } from '@/stores/object/customRoadSign/variableDirectionLaneLine'
import { parseLongitudinalDeceleration } from '@/stores/object/customRoadSign/longitudinalDecelerationMarking'
import { parseLateralDeceleration } from '@/stores/object/customRoadSign/lateralDecelerationMarking'
import { parseSemicircleLine } from '@/stores/object/customRoadSign/semicircleLine'
import { loadParseIntersectionGuideLine } from '@/stores/object/customRoadSign/intersectionGuideLine'
import {
  type ICustomModelConfig,
  useModelCacheStore,
} from '@/stores/object/modelCache'

// 由于道路级别元素在之前的解析过程中，被放入了 junction.laneLinks 层级中，不方便寻找
// 在解析物体元素时，需要通过 laneLinkId 找到解析后对应的 junction 和 laneLink 数据
// 提供一个临时的对象缓存，在开始解析物体时赋值，结束解析物体时销毁
let _originLaneLinks: Array<biz.ICommonLaneLink> = []
// 通过映射表来缓存跟 roadId 和 junction 绑定的杆数据，便于查找
const poleInRoad: Map<string, Array<string>> = new Map()
// 不用 laneLinkId 作为 key 的原因：自建地图
const poleInJunction: Map<string, Array<string>> = new Map()

/**
 * 解析物体的总入口
 * @param objects
 */
export async function parseObject (params: {
  objects: Array<biz.ICommonObject>
  laneLinks: Array<biz.ICommonLaneLink>
  // 是否是旧版编辑器产出的地图文件
  isOldMap?: boolean
  // 解析完但还未同步到 store 中的控制点状态
  controlPoints?: Array<biz.IControlPoint>
}) {
  const { objects, laneLinks, isOldMap = false, controlPoints = [] } = params
  // 解析后的物体 id 集合，需要同步到 store 中
  const _objectIds: Array<string> = []
  _originLaneLinks = [...laneLinks]

  // 记录已经解析一部分属性的停车位 id
  const parkingSpaceIds: Array<string> = []

  // 需要依赖 config 配置文件中的每个大类的判断方法，快速查找到对应物体的属性
  // 跟保存逻辑类似，第一轮先解析杆、路面标识、其他类型，第二轮再解析杆容器中的标志牌、信号灯
  const presetParsePromises = []

  const modelCacheStore = useModelCacheStore()

  for (const objectData of objects) {
    const { name } = objectData

    let queryCustomModelRes: ICustomModelConfig | null = null
    // 判断 userdata 中是否存在自定义模型的标识字段
    let isCustomModel = false
    const { userdata: customModelUserData = [] } = objectData
    for (const _userData of customModelUserData) {
      const { code, value } = _userData
      if (code === 'custom_model' && value) {
        isCustomModel = true
        queryCustomModelRes = modelCacheStore.getCustomModelConfigByName(value)
      }
    }

    if (queryCustomModelRes && isCustomModel) {
      // 如果是自定义导入模型，则单独解析
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            const customModelData = parseCustomModel({
              config: queryCustomModelRes!,
              data: objectData,
            })
            if (!customModelData) {
              resolve('')
              return
            }
            _objectIds.push(customModelData.id)
            resolve(customModelData.id)
          })
        })(),
      )
      continue
    }

    // 通过物体的名称，检索对应的配置
    const queryPoleRes = getPoleConfig(name)
    const queryRoadSignRes = getRoadSignConfig(name)
    const queryOtherRes = getOtherConfig(name)
    const queryParkingSpaceRes = getParkingSpaceConfig(name)

    // 如果匹配到响应的物体配置，则调用对应的解析逻辑
    // 优先处理杆、路面标识、其他
    if (queryPoleRes) {
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            const poleData = parsePole({
              config: queryPoleRes,
              data: objectData,
              isOldMap,
            })
            if (!poleData) {
              resolve('')
              return
            }
            _objectIds.push(poleData.id)
            resolve(poleData.id)
          })
        })(),
      )
      continue
    } else if (queryRoadSignRes && !queryParkingSpaceRes) {
      // 解析路面标识结果存在，解析停车位结果不存在，说明是非停车位的路面标识
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            const roadSignData = parseRoadSign({
              config: queryRoadSignRes,
              data: objectData,
              isOldMap,
            })
            if (!roadSignData) {
              resolve('')
              return
            }
            _objectIds.push(roadSignData.id)
            resolve(roadSignData.id)
          })
        })(),
      )
      continue
    } else if (queryParkingSpaceRes) {
      // 停车位单独处理
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            const parkingSpaceData = parseParkingSpace({
              config: queryParkingSpaceRes,
              data: objectData,
              isOldMap,
            })
            if (!parkingSpaceData) {
              resolve('')
              return
            }
            // 单独缓存已经处理的停车位 id
            parkingSpaceIds.push(parkingSpaceData.id)
            _objectIds.push(parkingSpaceData.id)
            resolve(parkingSpaceData.id)
          })
        })(),
      )
      continue
    } else if (queryOtherRes) {
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            const otherData = parseOther({
              config: queryOtherRes,
              data: objectData,
              isOldMap,
            })
            if (!otherData) {
              resolve('')
              return
            }
            _objectIds.push(otherData.id)
            resolve(otherData.id)
          })
        })(),
      )
      continue
    }
  }

  await Promise.all(presetParsePromises)

  // 单独解析所有停车位的顶点属性
  const calcParkingSpaceGeoAttr = []
  for (const id of parkingSpaceIds) {
    const parkingSpaceData = getObject(id) as biz.IParkingSpace
    if (!parkingSpaceData) continue
    const {
      roadId = '',
      s,
      t,
      count,
      width,
      length,
      lineWidth,
      margin,
      yaw,
      innerAngle,
    } = parkingSpaceData
    const road = getRoad(roadId)
    if (!road) continue

    // 通过 st 坐标计算新的位置
    const {
      s: newS, // 使用投影到 s 轴上的新坐标
      refLinePoint,
      targetPoint,
      tangent,
      percent,
    } = getLocationByST({
      s,
      t,
      useElevationTangent: true,
      elevationPath: road.elevationPath,
      curvePath: road.keyPath,
    })

    // 虚线的限时停车位
    const isDashed = parkingSpaceData.name === 'Time_Limit_Parking_Space_Mark'

    // 组装 worker 线程计算所需的条件
    const payload = {
      curvePathPoints: road.keyPath.points,
      refLineLocation: {
        s: newS,
        t,
        percent,
        tangent,
        closestPointOnRefLine: refLinePoint,
      },
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

    calcParkingSpaceGeoAttr.push(
      (() => {
        return new Promise((resolve) => {
          // 通过 webWorker 线程并行且异步计算所有停车位的顶点属性
          pool.exec('getCornerVerticesAttr', [payload]).then((data) => {
            const { geoAttrs, cornerPoints, sCoordinates, cornerSTPoints } =
              data

            // 将计算好的属性，同步到停车位数据中
            parkingSpaceData.sCoordinates = sCoordinates
            parkingSpaceData.geoAttrs = geoAttrs
            parkingSpaceData.cornerPoints = cornerPoints
            parkingSpaceData.cornerSTPoints = cornerSTPoints
            parkingSpaceData.s = newS
            parkingSpaceData.position = {
              x: targetPoint.x,
              y: targetPoint.y,
              z: targetPoint.z,
            }

            // 更新缓存中停车位的数据
            setObject(parkingSpaceData.id, parkingSpaceData)

            resolve(id)
          })
        })
      })(),
    )
  }
  // 统一计算所有停车位的顶点数据
  await Promise.all(calcParkingSpaceGeoAttr)

  // 再处理需要依赖杆容器的信号灯和标志牌【重难点：怎么将基于参考线坐标系的 st 坐标，转换成基于杆容器的局部笛卡尔坐标】
  const parseObjectOnPolePromises = []
  for (const objectData of objects) {
    const { id, name, type, subtype } = objectData
    // 如果在第一轮已经遍历处理过了，则忽略
    if (_objectIds.includes(id)) continue

    const querySignalBoardRes = getSignalBoardConfig(type, subtype)
    const queryTrafficLightRes = getTrafficLightConfig(type, subtype)
    const querySensorRes = getSensorConfig(name)

    const config = queryTrafficLightRes || querySignalBoardRes || querySensorRes
    if (!config) continue
    const { mainType } = config

    parseObjectOnPolePromises.push(
      (() => {
        return new Promise((resolve) => {
          // 解析杆上的物体
          const targetData = parseObjectOnPole({
            type: mainType as 'trafficLight' | 'signalBoard' | 'sensor',
            config,
            data: objectData,
            isOldMap,
          })
          if (!targetData) {
            resolve('')
            return
          }

          _objectIds.push(targetData.id)
          resolve(targetData.id)
        })
      })(),
    )
  }

  await Promise.all(parseObjectOnPolePromises)

  // 获取接口返回的原始地图数据
  const originMapCache = getOriginMapCache()

  // 判断加载的物体中，是否存在虚拟的隧道 Tunnel 物体
  for (const objectData of objects) {
    const { name, roadid = '-1', s: tunnelS, length: tunnelLength } = objectData
    if (name !== 'Tunnel') continue

    // 获取虚拟隧道物体归属的道路
    const road = getRoad(roadid) as biz.ITunnel
    if (!road) continue

    let _tunnelS = Number(tunnelS)
    let _tunnelLength = Number(tunnelLength)
    if (originMapCache) {
      // 获取原始的道路长度
      let targetOriginRoad: biz.ICommonRoad | null = null
      for (const originRoad of originMapCache.roads) {
        if (targetOriginRoad) continue
        if (originRoad.id === roadid) {
          targetOriginRoad = originRoad
        }
      }
      if (targetOriginRoad) {
        // 基于原始的道路长度和隧道属性的比例，在新的道路长度下重新计算的新的隧道属性
        const originRoadLength = fixedPrecision(Number(targetOriginRoad.length))
        _tunnelS = (_tunnelS / originRoadLength) * road.length
        _tunnelLength = (_tunnelLength / originRoadLength) * road.length
      }
    }

    // 将道路设置成隧道类型
    road.roadType = 'tunnel'
    // 挂载隧道相关的属性
    road.tunnelS = fixedPrecision(_tunnelS)
    road.tunnelLength = fixedPrecision(_tunnelLength)
  }

  // 手动销毁
  _originLaneLinks.length = 0
  parkingSpaceIds.length = 0
  poleInRoad.clear()
  poleInJunction.clear()

  return _objectIds
}

// ---------- 以下是辅助计算属性的工具函数 ----------

interface IOptionInPole {
  poleId: string
  onVerticalPart: boolean // 位于杆的竖直部分
  yaw: number
  tangent: common.vec3
  targetPoint: common.vec3
  localAngle?: 0 | 180 // 物体在横杆上的角度，仅支持 0 和 180°
}

interface IGetLocalDataParams extends IOptionInPole {
  zOffset: number
}

interface ILocalData {
  position: common.vec3 // 局部坐标
  angle: number // 局部角度
}

/**
 * 杆上物体角度，跟杆自身角度的关系：
 * poleAngle = poleTangentAngle + poleYaw
 * lightAngle = poleAngle + 90°固定夹角 + lightLocalAngle
 *            = lightTangentAngle + lightYaw
 * poleAngle: 杆自身朝向，跟水平向右基准方向的夹角（即世界坐标系下的绕 y 轴的角度）
 * poleTangentAngle: 杆的位置，在 s 轴参考线上投影点的切线方向，跟水平向右基准方向的夹角
 * poleYaw: 杆在参考线坐标系下的偏航角
 * lightAngle(使用 light 代表杆上的一类物体而已): 信号灯自身的朝向，跟水平向右基准方向的夹角
 * lightLocalAngle: 信号灯在杆容器的局部坐标系下的旋转角度
 * lightTangentAngle: 信号灯所在的 st 坐标位置，对应 s 轴参考线投影点的切线方向，跟水平向右基准方向的夹角
 * lightYaw: 信号灯在参考线坐标系下的偏航角
 */

// 基于找到的杆，获取物体在杆容器中的局部属性
function getLocalDataInPole (params: IGetLocalDataParams) {
  const {
    poleId,
    onVerticalPart,
    yaw,
    targetPoint,
    tangent,
    localAngle = 0, // 仅在横杆的水平延伸部分使用
    zOffset,
  } = params

  const poleData = getObject(poleId)
  if (!poleData) return null
  const { name: poleName } = poleData
  const poleConfig = getPoleConfig(poleName)
  if (!poleConfig) return null

  let localData: ILocalData | null = null
  const { position: polePosition, lookAtPoint: poleLookAtPoint } = poleData
  if (!poleLookAtPoint) return null

  // 杆上物体跟杆固定的朝向偏差
  const fixedAngle = 90

  if (onVerticalPart) {
    // 在垂直竖杆上

    // 局部角度计算
    // 杆上物体在 s 轴投影点的切线方向到水平向右基准方向的夹角
    const tangentVec2 = new Vector2(tangent.z, tangent.x)
    const tangentAngle = MathUtils.radToDeg(tangentVec2.angle())

    // 获取杆自身朝向到水平向右基准方向的夹角
    const poleDirectionVec2 = new Vector2(
      poleLookAtPoint.z - polePosition.z,
      poleLookAtPoint.x - polePosition.x,
    )
    const poleAngle = MathUtils.radToDeg(poleDirectionVec2.angle())

    const localAngle = tangentAngle + yaw - fixedAngle - poleAngle
    // 将局部坐标调整在 [-180, 180] 区间
    const _localAngle = fixedPrecision(getValidDegreeInRange(localAngle))

    // 将局部角度转换成弧度制
    const localRad = MathUtils.degToRad(_localAngle)

    // 局部坐标计算
    // 竖杆的半径
    const radius = poleConfig.style.vRadius
    // 绕竖杆圆心旋转局部角度后的新坐标
    const localPosition = new Vector3(
      radius * Math.cos(-localRad),
      zOffset,
      radius * Math.sin(-localRad),
    )

    localData = {
      position: localPosition,
      angle: _localAngle,
    }
  } else {
    // 在水平横杆上
    const { hRadius, height } = poleConfig.style
    if (!hRadius) return null

    // 在横杆水平部分的一侧
    const _x = localAngle === 0 ? hRadius : -hRadius

    // 横杆方向在水平面上的投影
    const projectPoleDirection = new Vector3(
      poleLookAtPoint.x - polePosition.x,
      0,
      poleLookAtPoint.z - polePosition.z,
    ).normalize()

    // 在水平面的投影坐标
    const projectObjectPosition = new Vector3(targetPoint.x, 0, targetPoint.z)
    const projectPolePosition = new Vector3(polePosition.x, 0, polePosition.z)

    // 从杆位置到杆上物体的投影方向
    const projectDirection = projectObjectPosition
      .clone()
      .sub(projectPolePosition)
    // 获取两个方向向量的夹角
    const alphaAngle = projectDirection.angleTo(projectPoleDirection)
    // 物体到横杆的距离
    const _z = Math.abs(projectDirection.length() * Math.cos(alphaAngle))

    // 水平横杆的高度（放置横杆上，忽略 zOffset 传过来的高度）
    const objectHeight = height - hRadius
    const localPosition = {
      x: _x,
      y: objectHeight,
      z: _z,
    }

    localData = {
      position: localPosition,
      // 如果在横杆水平延伸部分，则可以直接使用角度
      angle: localAngle,
    }
  }

  return localData
}

// 找到物体在道路上最匹配的杆，和在杆容器中的局部位置属性
function getMatchPole (params: {
  curvePath: biz.ICurve3
  elevationPath?: biz.ICurve3
  s: number
  t: number
  yaw: number // 物体相对于道路参考线的偏航角【角度制】
  zOffset: number // 物体的高度
  targetPoleId: string // 目标杆的 id
  roadId?: string // 当前物体所在的道路 id
  junctionId?: string // 当前物体所在的路口 id（跟 roadId 不同时存在）
}) {
  const {
    curvePath,
    elevationPath,
    s,
    t,
    yaw,
    targetPoleId,
    roadId = '',
    junctionId = '',
  } = params

  // 基于参考线，计算 st 坐标对应的实际位置
  const {
    targetPoint, // 世界坐标系的位置
    // 校正后合理有效的 st 坐标
    tangent, // 在考线上投影点的切线方向向量
  } = getLocationByST({
    curvePath,
    elevationPath,
    s,
    t,
  })

  // 物体在水平面的投影坐标
  const projectObjectPosition = new Vector3(targetPoint.x, 0, targetPoint.z)
  const tangentVec2 = new Vector2(tangent.z, tangent.x)

  // 匹配杆过程中，可接受的误差范围
  const distDeviation = 0.2 // 竖杆上的距离误差 20cm
  const horizontalDistDeviation = 1 // 横杆上的距离误差 1m
  const angleDeviation = 15 // 角度误差 15°

  let poleIds
  if (targetPoleId) {
    // 如果存在目标的杆，则直接计算在目标杆上相关数据
    poleIds = [targetPoleId]
  } else {
    // 否则从道路或路口中的所有杆中，匹配最优的杆
    if (roadId) {
      poleIds = poleInRoad.get(roadId)
    } else if (junctionId) {
      poleIds = poleInJunction.get(junctionId)
    }
  }
  if (!poleIds || poleIds.length < 1) return null

  // 匹配到符合要求的杆，对应的参数集合
  let optimalPoleOption: IOptionInPole | null = null

  // 如果跟竖杆的距离在误差范围内，且遍历到最后都没有匹配的杆，则使用该备份选项
  let backupVerticalOption: IOptionInPole | null = null

  for (const poleId of poleIds) {
    // 如果已经找到最匹配的杆了，则不继续往下执行
    if (optimalPoleOption) continue

    const poleData = getObject(poleId)
    if (!poleData) continue
    const {
      position: polePosition,
      name: poleName,
      lookAtPoint: poleLookAtPoint,
    } = poleData
    const poleConfig = getPoleConfig(poleName)
    if (!poleConfig) continue
    // 是否是单纯的竖杆
    let isVerticalPole = false
    if (
      poleConfig.name === 'Vertical_Pole' ||
      poleConfig.name === 'Pillar_Pole_6m' ||
      poleConfig.name === 'Pillar_Pole_3m'
    ) {
      isVerticalPole = true
    }

    // 假设先基于竖杆上的情况做判断
    // 竖杆的半径
    const { vRadius } = poleConfig.style
    // 计算杆的位置和物体的位置，在水平面投影点的距离
    const projectPolePosition = new Vector3(polePosition.x, 0, polePosition.z)
    // 从物体位置到杆圆心的距离【跟竖杆的半径长度做对比】
    const calcDist = fixedPrecision(
      projectObjectPosition.distanceTo(projectPolePosition),
    )

    // 从杆圆心指向物体的方向
    // 【注意】经过测试发现，杆和杆上物体的世界坐标系下坐标，计算的朝向角度可能存在较大的误差
    const directionVec2 = new Vector2(
      projectObjectPosition.z - projectPolePosition.z,
      projectObjectPosition.x - projectPolePosition.x,
    )
    // 物体朝向跟水平向右基准方向的夹角
    const directionDeg = MathUtils.radToDeg(directionVec2.angle())

    // 物体在 s 轴投影点的切线方向，跟水平向右基准方向的夹角
    const tangentDeg = MathUtils.radToDeg(tangentVec2.angle())

    // 计算出来的物体偏航角，保证在 [-180, 180] 范围中【用于跟加载时的偏航角做对比】
    const calcYaw = getValidDegreeInRange(directionDeg - tangentDeg)

    // 距离的误差
    const deltaDist = Math.abs(calcDist - vRadius)
    // 夹角的误差，且将误差控制在 [-180, 180] 度范围中
    const deltaAngle = getValidDegreeInRange(Math.abs(calcYaw - yaw))

    // 如果当前杆是竖杆
    if (isVerticalPole) {
      if (targetPoleId) {
        // 如果是竖杆，且指定了目标杆 id，无需计算误差，可直接保存数据
        optimalPoleOption = {
          poleId,
          onVerticalPart: true,
          yaw,
          tangent,
          targetPoint,
        }
        continue
      } else {
        // 如果距离在误差范围中
        if (deltaDist <= distDeviation) {
          // 缓存备份的竖杆上的配置
          backupVerticalOption = {
            poleId,
            onVerticalPart: true,
            yaw,
            tangent,
            targetPoint,
          }

          // 且夹角在误差范围中
          if (deltaAngle <= angleDeviation) {
            // 说明当前杆是匹配到最优的杆，可以基于当前杆来计算物体在杆容器中的局部坐标
            optimalPoleOption = {
              poleId,
              onVerticalPart: true,
              yaw,
              tangent,
              targetPoint,
            }
            continue
          }
        }
      }
    } else {
      // 如果是横杆，需要区分在竖直区域还是水平区域
      if (deltaDist <= distDeviation) {
        // 如果是横杆的情况下，以上竖直部分判断逻辑还能匹配上，说明物体处于横杆的竖直部分
        optimalPoleOption = {
          poleId,
          onVerticalPart: true, // 位于横杆的竖直部分
          yaw,
          tangent,
          targetPoint,
        }
        // 找到匹配的杆，可以跳过后续的逻辑
        continue
      }

      if (!poleLookAtPoint) continue
      // 横杆的半径
      const { hRadius = 0 } = poleConfig.style
      if (!hRadius) continue

      // 此时物体的距离或者朝向的角度，至少有一项不符合，判断是否处于横杆的水平延伸部分

      // 杆上物体的角度 = 物体位置在 s 轴投影点的切线方向到水平向右基准方向夹角 + 物体的偏航角（控制在 [-180, 180] 区间）
      const objectAngle = getValidDegreeInRange(tangentDeg + yaw)

      // 假如物体在横杆水平部分的局部角度为 0
      const localAngle1 = 0
      // 假如物体在横杆水平部分的局部角度为 180°
      const localAngle2 = 180

      // 获取杆自身朝向到水平向右基准方向的夹角（由于杆在道路上和路口中，表示角度的字段不一样，但都有 lookAtPoint）
      const poleDirectionVec2 = new Vector2(
        poleLookAtPoint.z - polePosition.z,
        poleLookAtPoint.x - polePosition.x,
      )
      const poleAngle = MathUtils.radToDeg(poleDirectionVec2.angle())
      // 杆上物体跟杆的朝向存在 90° 的偏差
      const fixedAngle = 90
      // 通过杆的角度，和预设的杆内物体局部角度计算出来的物体朝向角度
      const objectAngle1 = getValidDegreeInRange(
        poleAngle + fixedAngle + localAngle1,
      )
      const objectAngle2 = getValidDegreeInRange(
        poleAngle + fixedAngle + localAngle2,
      )

      // 夹角的误差
      const deltaAngle1 = Math.abs((objectAngle - objectAngle1) % 360)
      const deltaAngle2 = Math.abs((objectAngle - objectAngle2) % 360)

      // 横杆方向在水平面上的投影
      const projectPoleDirection = new Vector3(
        poleLookAtPoint.x - polePosition.x,
        0,
        poleLookAtPoint.z - polePosition.z,
      ).normalize()

      // 从杆位置到杆上物体的投影方向
      const projectDirection = projectObjectPosition
        .clone()
        .sub(projectPolePosition)
      // 获取两个方向向量的夹角
      const alphaAngle = projectDirection.angleTo(projectPoleDirection)
      // 物体到横杆的距离
      const projectDist = Math.abs(
        projectDirection.length() * Math.sin(alphaAngle),
      )

      // 放置在横杆上的距离的误差
      const deltaProjectDist = Math.abs(projectDist - hRadius)

      // 如果距离在误差范围内
      if (deltaProjectDist <= horizontalDistDeviation) {
        if (deltaAngle1 <= angleDeviation) {
          optimalPoleOption = {
            poleId,
            onVerticalPart: false, // 位于横杆的水平延伸部分
            yaw,
            tangent,
            targetPoint,
            localAngle: 0, // 处于杆的朝向左侧
          }
          continue
        } else if (deltaAngle2 <= angleDeviation) {
          optimalPoleOption = {
            poleId,
            onVerticalPart: false, // 位于横杆的水平延伸部分
            yaw,
            tangent,
            targetPoint,
            localAngle: 180, // 处于杆的朝向右侧
          }
          continue
        }
      }

      // 如果解析到最后，在有目标 poleId 的情况下，还没能解析到符合要求的数据
      // 则强行赋值在横杆的竖直部分作为兜底，此时放置的位置大概率是存在偏差的
      if (targetPoleId && !optimalPoleOption) {
        optimalPoleOption = {
          poleId,
          onVerticalPart: true, // 位于横杆的竖直部分
          yaw,
          tangent,
          targetPoint,
        }
      }
    }
  }

  // 如果遍历到最后都没有精准匹配到的杆，且存在距离在误差范围内的备用竖杆配置，则使用备份
  if (backupVerticalOption && !optimalPoleOption) {
    optimalPoleOption = backupVerticalOption
  }

  return optimalPoleOption
}

// 通过 st 坐标判断物体可能跟哪个车道关联
function getLaneByST (params: { road: biz.IRoad, s: number, t: number }) {
  // 车道中物体的 t，应该是负值
  const { road, s, t } = params
  const _t = Math.abs(t)
  const { sections, length, keyPath } = road
  // 确保比例在 [0,1] 范围内
  const ratio = Math.max(0, Math.min(s / length, 1))

  let targetSectionId = ''
  let targetLaneId = ''
  for (const section of sections) {
    const { id: sectionId, pStart, pEnd, boundarys, lanes } = section
    // 忽略不在百分比范围中的 section
    if (ratio < pStart || ratio > pEnd) continue
    targetSectionId = sectionId
    for (const lane of lanes) {
      // 获取车道左、右边界首和尾的采样点，到道路参考线的平均距离
      const { id: laneId, lbid, rbid } = lane
      const lBoundary = getBoundaryById(boundarys, lbid)
      const rBoundary = getBoundaryById(boundarys, rbid)
      if (!lBoundary || !rBoundary) continue

      // 车道首部左右边界线采样点到参考线的平均距离
      const refLineHeadPoint = keyPath.getPointAt(pStart)
      const leftHeadPoint = transformVec3(lBoundary.samplePoints[0])
      const rightHeadPoint = transformVec3(rBoundary.samplePoints[0])
      const headMinDist = leftHeadPoint.distanceTo(refLineHeadPoint)
      const headMaxDist = rightHeadPoint.distanceTo(refLineHeadPoint)

      // 车道尾部左右边界线采样点到参考线的平均距离
      const refLineTailPoint = keyPath.getPointAt(pEnd)
      const leftTailPoint = transformVec3(
        lBoundary.samplePoints[lBoundary.samplePoints.length - 1],
      )
      const rightTailPoint = transformVec3(
        rBoundary.samplePoints[rBoundary.samplePoints.length - 1],
      )
      const tailMinDist = leftTailPoint.distanceTo(refLineTailPoint)
      const tailMaxDist = rightTailPoint.distanceTo(refLineTailPoint)

      // 判断 t 的绝对值是否在车道区间中
      if (
        (_t >= headMinDist && _t <= headMaxDist) ||
        (_t >= tailMinDist && _t <= tailMaxDist)
      ) {
        targetLaneId = laneId
        return {
          sectionId: targetSectionId,
          laneId: targetLaneId,
        }
      }
    }
  }

  return {
    sectionId: targetSectionId,
    laneId: targetLaneId,
  }
}

// 通过 laneLinkId 找到对应路口和车道连接线数据
function getLaneLinkById (laneLinkId: string) {
  const originLaneLink = getDataById(
    _originLaneLinks,
    laneLinkId,
  ) as biz.ICommonLaneLink
  if (!originLaneLink) return null
  const { junctionid } = originLaneLink
  const junction = getJunction(junctionid)
  if (!junction) return null
  const laneLink = getDataById(junction.laneLinks, laneLinkId) as biz.ILaneLink
  if (!laneLink) return null

  return {
    junction,
    laneLink,
  }
}

// 基于车道连接线的采样点形成的参考线曲线，获取 st 坐标对应的实际坐标
function getLocationByLaneLinkST (params: {
  laneLink: biz.ILaneLink
  s: number
  t: number
  yaw: number
}) {
  const { laneLink, s, t, yaw } = params
  const { samplePoints } = laneLink
  const curvePath = new CatmullRomCurve3(transformVec3ByObject(samplePoints))
  curvePath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    curvePath.tension = CatmullromTension
  }

  const location = getLocationByST({
    curvePath,
    s,
    t,
  })

  // 基于 laneLink 的参考线，计算全局的角度
  const { tangent } = location
  // 水平面的投影坐标
  const tangentVec2 = new Vector2(tangent.z, tangent.x)
  // 计算切线方向到水平横轴的夹角
  const tangentDeg = MathUtils.radToDeg(tangentVec2.angle())
  // 世界坐标系下的角度，且将角度控制在 [-180, 180] 度的范围中
  const angle = getValidDegreeInRange(tangentDeg + yaw)

  return {
    ...location,
    angle,
  }
}

// ---------- 以下是解析各种物体的入口函数 ----------

function parseParkingSpace (params: {
  config: IParkingSpaceConfig
  data: biz.ICommonObject
  isOldMap?: boolean
}) {
  const { config, data: commonData, isOldMap = false } = params
  const {
    id: objectId,
    roadid: roadId,
    lanelinkid: laneLinkId,
    s,
    t,
    hdg,
    length,
    width,
    repeat = null,
    userdata = [],
    markings = [],
  } = commonData

  const { name, type, subtype, mainType, showName, lineWidth } = config
  // 创建前端侧的停车位数据结构
  const parkingSpaceData: biz.IParkingSpace = {
    ...createDefaultObjectData(),
    // color、lineWidth、innerAngle 属性不是挂载在 repeat 属性中，先提供默认值后续再更新
    color: 'White', // 默认为白色
    lineWidth, // 使用默认的线宽
    innerAngle: 90, // 默认为矩形停车位
    width: 0,
    length: 0,
    margin: 0,
    count: 1,
    geoAttrs: [],
    cornerPoints: [],
    sCoordinates: [],
    cornerSTPoints: [],
  }
  // 更新停车位的描述属性
  parkingSpaceData.id = objectId
  parkingSpaceData.mainType = mainType
  parkingSpaceData.name = name
  parkingSpaceData.showName = showName
  parkingSpaceData.type = type
  parkingSpaceData.subtype = subtype

  if (roadId && roadId !== '-1') {
    // 道路上的停车位

    // 偏航角【将弧度制保存成角度制】
    parkingSpaceData.yaw = fixedPrecision(MathUtils.radToDeg(Number(hdg)))
    // 每个停车位的尺寸
    parkingSpaceData.width = fixedPrecision(Number(width))
    parkingSpaceData.length = fixedPrecision(Number(length))
    // 第一个停车位几何中心点的参考线坐标
    parkingSpaceData.s = fixedPrecision(Number(s))
    parkingSpaceData.t = fixedPrecision(Number(t))
    // 绑定关联的道路
    parkingSpaceData.roadId = roadId

    // repeat 属性不一定存在，如果只是一个单独的停车位，可能不存在 repeat 属性
    if (repeat) {
      // 如果 repeat 属性存在，则更新重复停车位相关的属性
      const { s: repeatS, length: repeatLength, distance } = repeat
      // 用重复停车位的 s 坐标更新原有的 s 坐标
      parkingSpaceData.s = fixedPrecision(Number(repeatS))
      // 重复的停车位个数
      const repeatCount =
        Math.round(Number(repeatLength) / Number(distance)) + 1
      parkingSpaceData.count = repeatCount
      // 相邻停车位几何中心点的距离
      parkingSpaceData.margin = fixedPrecision(Number(distance))

      // 旧版停车位的长度
      if (isOldMap) {
        parkingSpaceData.length = fixedPrecision(Number(length) / repeatCount)
      }
    }

    // 从 userdata 中读取内部夹角
    if (userdata && userdata.length > 0) {
      for (const _userdata of userdata) {
        if (_userdata.code === 'parking_angle') {
          parkingSpaceData.innerAngle = fixedPrecision(Number(_userdata.value))
        }
      }
    }
    // 从 markings 中读取样式
    if (markings && markings.length > 0) {
      const { width: lineWidth, color } = markings[0]
      // 考虑可能传入 '0.00' 字符串的情况
      const _lineWidth =
        Number(lineWidth) === 0 ? config.lineWidth : Number(lineWidth)
      parkingSpaceData.lineWidth = fixedPrecision(_lineWidth)
      // 转换成首字母大写的颜色字符串
      const _color = uppercaseFirstChar(color) as common.colorType
      if (DefaultColor[_color]) {
        parkingSpaceData.color = _color
      }
    }
  } else if (laneLinkId && laneLinkId !== '-1') {
    // 暂不支持路口中放置停车位
  }

  setObject(parkingSpaceData.id, parkingSpaceData)

  return parkingSpaceData
}

// 解析杆
function parsePole (params: {
  config: IPoleConfig
  data: biz.ICommonObject
  isOldMap?: boolean
}) {
  const { config, data: commonData, isOldMap = false } = params
  const {
    id: objectId,
    roadid: roadId,
    lanelinkid: laneLinkId,
    s,
    t,
    hdg,
  } = commonData
  // 需要将 st 值转换成 number 类型
  const _s = Number(s)
  const _t = Number(t)
  const _hdg = isOldMap ? Number(hdg) + halfPI : Number(hdg)
  const { name, showName, type, subtype } = config
  const poleData: biz.IPole = createDefaultObjectData()
  poleData.id = objectId
  poleData.mainType = 'pole'
  poleData.name = name
  poleData.showName = showName
  poleData.type = type
  poleData.subtype = subtype

  const poleConfig = getPoleConfig(name)
  if (!poleConfig) return null

  // 同步尺寸
  if (name === 'Cross_Pole' || name === 'Cantilever_Pole') {
    // 横杆
    const { style } = poleConfig
    poleData.radius = style.vRadius
    poleData.height = fixedPrecision(style.height)
  } else {
    // 竖杆
    // 半径
    const { style } = poleConfig
    poleData.radius = style.vRadius
    // 高度
    poleData.height = fixedPrecision(style.height)
  }

  if (roadId && roadId !== '-1' && laneLinkId === '-1') {
    const road = getRoad(roadId)
    if (!road) return

    // 基于道路的参考线，计算 st 坐标对应的实际位置
    const {
      targetPoint,
      // 校正后合理有效的 st 坐标
      s: validS,
      t: validT,
      percent,
      tangent,
      refLinePoint,
    } = getLocationByST({
      curvePath: road.keyPath,
      elevationPath: road.elevationPath,
      s: _s,
      t: _t,
    })
    // 更新杆在道路上的相关属性
    poleData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
    poleData.roadId = roadId
    poleData.s = validS
    poleData.t = validT
    poleData.closestPoint = refLinePoint
    poleData.closestPointPercent = percent
    poleData.closestPointTangent = tangent

    // 通过偏航角计算杆看向的点【将弧度制转换成角度制】
    const _yaw = fixedPrecision(MathUtils.radToDeg(_hdg))
    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: refLinePoint,
      tangent,
      point: targetPoint,
      yaw: _yaw,
    })
    poleData.lookAtPoint = lookAtPoint
    poleData.yaw = _yaw

    // 将杆缓存到对应 roadId 的映射表中
    if (!poleInRoad.has(roadId)) {
      poleInRoad.set(roadId, [poleData.id])
    } else {
      const poleIds = poleInRoad.get(roadId)
      if (poleIds && !poleIds.includes(poleData.id)) {
        poleInRoad.set(roadId, [...poleIds, poleData.id])
      }
    }
  } else if (laneLinkId && laneLinkId !== '-1') {
    // 将路口中的 laneLink 参考线 st 坐标，转换成在路口中的绝对坐标
    const queryRes = getLaneLinkById(laneLinkId)
    if (!queryRes) return null
    const { junction, laneLink } = queryRes
    const { id: junctionId } = junction

    // 通过 laneLink 采样点形成的参考线获取 st 参考线坐标对应的实际位置
    const { targetPoint, angle } = getLocationByLaneLinkST({
      laneLink,
      s: _s,
      t: _t,
      yaw: fixedPrecision(MathUtils.radToDeg(_hdg)),
    })
    poleData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
    poleData.junctionId = junctionId
    // 由于路口的路面标识可以调整角度，因此需要基于 laneLink 的偏航角计算路口的全局角度
    poleData.angle = angle
    // 针对路口中的物体，基于全局角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle,
    })
    poleData.lookAtPoint = lookAtPoint

    // 将杆缓存到对应 junctionId 的映射表中
    if (!poleInJunction.has(junctionId)) {
      poleInJunction.set(junctionId, [poleData.id])
    } else {
      const poleIds = poleInJunction.get(junctionId)
      if (poleIds && !poleIds.includes(poleData.id)) {
        poleInJunction.set(junctionId, [...poleIds, poleData.id])
      }
    }
  }

  setObject(poleData.id, poleData)

  return poleData
}

// 需要针对旧版编辑器兼容的路面标线名称（基本都是路口元素）
export const needUpdateRoadSignName = [
  'Turn_Left_Waiting', // 左转弯待转区
]

// 解析路面标识
function parseRoadSign (params: {
  config: IRoadSignConfig
  data: biz.ICommonObject
  isOldMap?: boolean
}) {
  const { config, data: commonData, isOldMap = false } = params
  const {
    id: objectId,
    roadid: roadId,
    lanelinkid: laneLinkId,
    s,
    t,
    hdg,
  } = commonData
  let { width, length } = commonData

  // 对于特殊的通过 geoAttrs 创建的路面标线，走单独的创建逻辑
  switch (config.name) {
    case 'Road_Guide_Lane_Line':
      // 导向车道线
      return parseGuideLaneLine(params)
    case 'Variable_Direction_Lane_Line':
      // 可变导向车道线
      return parseVariableDirectionLaneLine(params)
    case 'Longitudinal_Deceleration_Marking':
      // 纵向减速标线
      return parseLongitudinalDeceleration(params)
    case 'Lateral_Deceleration_Marking':
      // 纵向减速标线
      return parseLateralDeceleration(params)
    case 'White_Semicircle_Line_Vehicle_Distance_Confirmation':
      // 白色半圆状车距确认线
      return parseSemicircleLine(params)
    case 'Intersection_Guide_Line': {
      // 路口导向线
      const queryLinkRes = getLaneLinkById(laneLinkId)
      if (!queryLinkRes) return null
      const { junction, laneLink } = queryLinkRes
      return loadParseIntersectionGuideLine({
        config,
        data: commonData,
        junction,
        laneLink,
      })
    }
    default:
      break
  }

  // 基于透传数据的 width, length 计算路面标识的纹理比例
  let ratio = Number(width) / Number(length)

  // 旧版公交专用车道线的兼容
  if (isOldMap && config.name === 'Bus_Only_Lane_Line') {
    length = '0'
    width = '0'
    ratio = 0.077
  }

  const { basicSize, name, showName, type, subtype } = config
  const size = [basicSize, basicSize / ratio]

  // 需要将 st 值转换成 number 类型
  const _s = Number(s)
  const _t = Number(t)
  let _hdg = Number(hdg)

  // 兼容旧版的标线
  if (isOldMap) {
    if (needUpdateRoadSignName.includes(name)) {
      _hdg += PI
    }
  }

  const roadSignData: biz.IRoadSign = {
    ...createDefaultObjectData(),
    size,
    basicWidth: 0,
    basicLength: 0,
    basicHeight: 0,
  }
  roadSignData.id = objectId
  roadSignData.mainType = 'roadSign'
  roadSignData.name = name
  roadSignData.showName = showName
  roadSignData.type = type
  roadSignData.subtype = subtype
  roadSignData.length = fixedPrecision(Number(length))
  roadSignData.width = fixedPrecision(Number(width))

  // TODO 对于特殊的路面标识需要特殊处理
  // 是否允许路面标线超过道路参考线坐标系的范围
  let crossBorder = false
  if (
    name === 'Crosswalk_Line' ||
    name === 'Crosswalk_with_Left_and_Right_Side'
  ) {
    crossBorder = true
  }

  if (roadId && roadId !== '-1' && laneLinkId === '-1') {
    // 道路上的路面标识
    const road = getRoad(roadId)
    if (!road) return null

    // 考虑可自由移动的路面标线（目前有残疾人车位标识）
    if (config.freeMoveInRoad) {
      // 基于道路的参考线，计算 st 坐标对应的实际位置
      const {
        targetPoint,
        // 校正后合理有效的 st 坐标
        s: validS,
        t: validT,
        percent,
        tangent,
        refLinePoint,
        normal,
      } = getLocationByST({
        curvePath: road.keyPath,
        useElevationTangent: true,
        elevationPath: road.elevationPath,
        s: _s,
        t: _t,
        crossBorder,
      })
      // 更新杆在道路上的相关属性
      roadSignData.position = {
        x: targetPoint.x,
        y: targetPoint.y,
        z: targetPoint.z,
      }
      roadSignData.roadId = roadId
      roadSignData.s = validS
      roadSignData.t = validT
      roadSignData.closestPoint = refLinePoint
      roadSignData.closestPointPercent = percent
      roadSignData.closestPointTangent = tangent
      roadSignData.projectNormal = normal

      // 通过偏航角计算看向的点【偏航角从弧度制转换成角度制】
      const _yaw = fixedPrecision(MathUtils.radToDeg(_hdg))
      const lookAtPoint = getLookAtPointByYaw({
        pointOnRefLine: refLinePoint,
        tangent,
        point: targetPoint,
        yaw: _yaw,
        useElevation: true,
      })
      roadSignData.lookAtPoint = lookAtPoint
      roadSignData.yaw = _yaw
    } else {
      // 判断当前 st 坐标的物体跟哪个 lane 关联

      const { sectionId, laneId } = getLaneByST({
        road,
        s: _s,
        t: _t,
      })
      if (sectionId && laneId) {
        roadSignData.sectionId = sectionId
        roadSignData.laneId = laneId
      }

      // 基于道路的参考线，计算 st 坐标对应的实际位置
      const {
        targetPoint,
        // 校正后合理有效的 st 坐标
        s: validS,
        t: validT,
        percent,
        tangent,
        refLinePoint,
        normal,
      } = getLocationByST({
        curvePath: road.keyPath,
        useElevationTangent: true,
        elevationPath: road.elevationPath,
        s: _s,
        t: _t,
      })
      // 更新路面标识在道路上的相关属性
      roadSignData.position = {
        x: targetPoint.x,
        y: targetPoint.y,
        z: targetPoint.z,
      }
      roadSignData.roadId = roadId
      roadSignData.s = validS
      roadSignData.t = validT
      roadSignData.closestPoint = refLinePoint
      roadSignData.closestPointPercent = percent
      roadSignData.closestPointTangent = tangent
      roadSignData.projectNormal = normal

      // 手动控制路标看向的点为沿着道路前方的点
      const frontPoint = getAlongRoadFrontPoint({
        tangent,
        point: targetPoint,
        isForward: validT <= 0,
      })
      roadSignData.lookAtPoint = frontPoint
    }
  } else if (laneLinkId && laneLinkId !== '-1') {
    // 路口中的路面标识
    // 将路口中的 laneLink 参考线 st 坐标，转换成在路口中的绝对坐标
    const queryRes = getLaneLinkById(laneLinkId)
    if (!queryRes) return null
    const { junction, laneLink } = queryRes

    // 通过 laneLink 采样点形成的参考线获取 st 参考线坐标对应的实际位置
    const { targetPoint, angle } = getLocationByLaneLinkST({
      laneLink,
      s: _s,
      t: _t,
      yaw: fixedPrecision(MathUtils.radToDeg(_hdg)),
    })
    roadSignData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
    roadSignData.junctionId = junction.id
    // 由于路口的路面标识可以调整角度，因此需要基于 laneLink 的偏航角计算路口的全局角度
    roadSignData.angle = angle
    // 针对路口中的物体，基于全局角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle,
    })
    roadSignData.lookAtPoint = lookAtPoint
  }

  setObject(roadSignData.id, roadSignData)

  return roadSignData
}

// 从透传的 userdata 中解析传感器设备参数，如果 userdata 中没有对应的数据，则使用默认的
function parseSensorDeviceParams (params: {
  userdata: Array<biz.IUserData>
  sensorName: string
  isCloud: boolean
}) {
  const { userdata = [], sensorName, isCloud = false } = params
  let deviceParams: any = null
  // 先使用默认值
  if (sensorName === 'Camera') {
    deviceParams = getCameraParams()
  } else if (sensorName === 'Lidar') {
    deviceParams = getLidarParams()
  } else if (sensorName === 'Millimeter_Wave_Radar') {
    deviceParams = getRadarParams()
  } else if (sensorName === 'RSU') {
    deviceParams = getRsuParams()
  }
  if (!deviceParams) return null
  const keys = Object.keys(deviceParams)

  if (isCloud) {
    // 云端环境
    switch (sensorName) {
      case 'Camera':
      case 'Lidar':
      case 'Millimeter_Wave_Radar': {
        // 只取关联的通信单元和配置的传感器参数 id
        for (const _userdata of userdata) {
          const { code, value } = _userdata
          if (code === 'V2X_idx') {
            deviceParams.V2X_idx = value
          } else if (code === 'BelongRSU') {
            deviceParams.BelongRSU = value
          }
        }
        break
      }
      case 'RSU': {
        for (const _userdata of userdata) {
          const { code, value } = _userdata
          if (code === 'V2X_idx') {
            deviceParams.V2X_idx = value
          } else if (code === 'JunctionIDs') {
            // rsu 关联的路口，保存时使用 , 逗号分隔的字符串，在重新加载时需要转换成数组
            deviceParams.JunctionIDs = value.split(',').filter(val => !!val)
          }
        }
        break
      }
      default:
        break
    }
  } else {
    // 单机版环境
    switch (sensorName) {
      case 'Camera': {
        for (const _userdata of userdata) {
          const { code, value } = _userdata
          if (code === 'Distortion') {
            // 畸变参数
            const [k1, k2, k3, p1, p2] = value.split(',')
            deviceParams[code] = {
              k1,
              k2,
              k3,
              p1,
              p2,
            }
          } else if (code === 'IntrinsicMat') {
            // 内参矩阵
            const [fx, skew, cx, param4, fy, cy, param7, param8, param9] =
              value.split(',')
            deviceParams[code] = {
              fx,
              skew,
              cx,
              param4,
              fy,
              cy,
              param7,
              param8,
              param9,
            }
          } else {
            // 常规的参数
            if (keys.includes(code)) {
              deviceParams[code] = value
            }
          }
        }
        break
      }
      case 'Lidar':
      case 'Millimeter_Wave_Radar':
      case 'RSU': {
        for (const _userdata of userdata) {
          const { code, value } = _userdata
          // 如果是传感器设备参数的属性，则更新
          if (sensorName === 'RSU' && code === 'JunctionIDs') {
            // rsu 关联的路口，保存时使用 , 逗号分隔的字符串，重新加载时转换成数组
            if (value) {
              deviceParams[code] = value.split(',')
            }
          } else {
            // 如果定义了设备参数，且返回了有效的值
            if (keys.includes(code)) {
              deviceParams[code] = value
            }
          }
        }
        break
      }
      default:
        break
    }
  }

  return deviceParams
}

/**
 * 解析杆上的信号灯、标志牌、传感器的逻辑大体类似，但需要针对具体的类型做解析
 * 1、信号灯需要解析信控配置
 * 2、传感器需要解析摄像头的俯仰角，以及传感器参数
 */
function parseObjectOnPole (params: {
  type: 'trafficLight' | 'signalBoard' | 'sensor'
  config: ITrafficLightConfig | ISignalBoardConfig | ISensorConfig
  data: biz.ICommonObject
  isOldMap?: boolean
}) {
  const { type: mainType, config, data: commonData, isOldMap = false } = params
  const {
    id: objectId,
    roadid: roadId,
    lanelinkid: laneLinkId,
    s,
    t,
    hdg,
    zOffset,
    userdata = [],
  } = commonData
  const _s = Number(s)
  const _t = Number(t)
  let _hdg = Number(hdg)

  const { name, showName, type, subtype } = config
  let targetData: biz.ITrafficLight | biz.ISignalBoard | biz.ISensor | '' = ''
  if (mainType === 'trafficLight') {
    targetData = {
      ...createDefaultObjectData(),
      poleId: '',
      onVerticalPole: true,
      controlJunctionId: '',
      controlRoadId: '',
    }
    // 兼容旧版的文件
    if (isOldMap) {
      _hdg += PI
    }
  } else if (mainType === 'signalBoard') {
    targetData = {
      ...createDefaultObjectData(),
      poleId: '',
      onVerticalPole: true,
    }
    // 兼容旧版的文件
    if (isOldMap) {
      _hdg += PI
    }
  } else if (mainType === 'sensor') {
    targetData = {
      ...createDefaultObjectData(),
      poleId: '',
      onVerticalPole: true,
    }
  }
  if (!targetData) return null

  targetData.id = objectId
  targetData.mainType = mainType
  targetData.name = name
  targetData.showName = showName
  targetData.type = type
  targetData.subtype = subtype

  let _optionInPole: IOptionInPole

  // 解析 userdata 中是否有所在杆的 id
  let targetPoleId = ''
  for (const _userdata of userdata) {
    if (_userdata.code === 'pole_id') {
      targetPoleId = _userdata.value
    }
  }

  if (roadId && roadId !== '-1' && laneLinkId === '-1') {
    // 道路上的信号灯
    const road = getRoad(roadId)
    if (!road) return null

    // 计算当前信号灯，在道路上最匹配的杆，以及在杆容器中的局部位置
    const optionInPole = getMatchPole({
      curvePath: road.keyPath,
      elevationPath: road.elevationPath,
      s: _s,
      t: _t,
      yaw: fixedPrecision(MathUtils.radToDeg(_hdg)),
      zOffset: Number(zOffset),
      roadId,
      targetPoleId, // 从 userdata 中解析出来的目标杆的 id
    })
    if (!optionInPole) return null

    _optionInPole = optionInPole
    targetData.roadId = roadId
  } else if (laneLinkId && laneLinkId !== '-1') {
    // 路口中的信号灯，找同 laneLinkId 的杆容器
    const queryLaneLinkRes = getLaneLinkById(laneLinkId)
    if (!queryLaneLinkRes) return
    const { laneLink, junction } = queryLaneLinkRes
    // 获取 laneLink 采样点对应的曲线路径
    const { samplePoints } = laneLink
    const curvePath = new CatmullRomCurve3(transformVec3ByObject(samplePoints))
    curvePath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      curvePath.tension = CatmullromTension
    }
    const optionInPole = getMatchPole({
      curvePath,
      s: _s,
      t: _t,
      yaw: fixedPrecision(MathUtils.radToDeg(_hdg)),
      zOffset: Number(zOffset),
      junctionId: junction.id,
      targetPoleId, // 从 userdata 中解析出来的目标杆的 id
    })
    if (!optionInPole) return null
    _optionInPole = optionInPole
    targetData.junctionId = junction.id
  }

  // @ts-expect-error
  if (!_optionInPole) return null

  // 是否在杆的竖直部分
  const { onVerticalPart, poleId } = _optionInPole

  // 计算在杆容器中的局部坐标
  const locationRes = getLocalDataInPole({
    ..._optionInPole,
    zOffset: Number(zOffset),
  })
  if (!locationRes) return null
  const { position: localPosition, angle: localAngle } = locationRes

  targetData.position = localPosition
  targetData.angle = localAngle
  targetData.onVerticalPole = onVerticalPart
  targetData.poleId = poleId

  if (mainType === 'trafficLight') {
    // 信控配置解析
    const { userdata = [] } = commonData
    const _targetData = targetData as biz.ITrafficLight
    for (const _userData of userdata) {
      const { code, value } = _userData
      if (code === 'relate_control') {
        _targetData.controlJunctionId = value
      } else if (code === 'relate_road') {
        // 兼容之前已经设置了控制道路的信控配置
        const controlRoadId = value.split('|')[0]
        _targetData.controlRoadId = controlRoadId
      }
    }
  } else if (mainType === 'sensor') {
    // TODO 传感器参数，摄像头俯仰角的解析
    const { userdata = [] } = commonData
    const _targetData = targetData as biz.ISensor
    const deviceParams = parseSensorDeviceParams({
      userdata,
      sensorName: name,
      isCloud: globalConfig.isCloud,
    })
    if (deviceParams) {
      _targetData.deviceParams = deviceParams
    }
  } else if (mainType === 'signalBoard') {
    //
  }

  // 将解析好的源数据保存到缓存中
  setObject(targetData.id, targetData)

  return targetData
}

// 需要适配的其他类型物体的名称
export const needUpdateOtherName = [
  'Protrusion', // 凸起
  'Reflective_Road_Sign', // 反光路标
  'Parking_Hole', // 停车杆
  'Ground_Lock', // 地锁
  'Plastic_Vehicle_Stopper', // 塑胶挡车器
  'Parking_Limit_Position_Pole_2m', // U型挡车器
  'Support_Vehicle_Stopper', // 支撑型挡车器
  'Charging_Station', // 充电桩
  'Tree', // 树木
  'Building', // 居民楼
]

// 解析其他类型物体
function parseOther (params: {
  config: IOtherConfig
  data: biz.ICommonObject
  isOldMap?: boolean
}) {
  const { config, data: commonData, isOldMap = false } = params
  const {
    id: objectId,
    roadid: roadId,
    lanelinkid: laneLinkId,
    s,
    t,
    hdg,
    width,
    length,
    height,
  } = commonData
  const { name, showName, type, subtype, isPlaneModel, basicSize, imageRatio } =
    config
  const otherData: biz.IOther = {
    ...createDefaultObjectData(),
    isPlaneModel,
    basicWidth: 0,
    basicLength: 0,
    basicHeight: 0,
  }

  let _hdg = Number(hdg)
  if (isOldMap) {
    // 兼容旧版地图编辑器的物体角度
    if (needUpdateOtherName.includes(name)) {
      _hdg += halfPI
    }
  }

  // 如果是平面的模型
  if (isPlaneModel && imageRatio && basicSize) {
    const size = [basicSize, basicSize / imageRatio]
    otherData.size = size
  }
  const _s = Number(s)
  const _t = Number(t)
  otherData.id = objectId
  otherData.mainType = 'other'
  otherData.name = name
  otherData.showName = showName
  otherData.type = type
  otherData.subtype = subtype
  // 同步尺寸
  otherData.width = fixedPrecision(Number(width))
  otherData.length = fixedPrecision(Number(length))
  otherData.height = fixedPrecision(Number(height))
  if (name === 'Building' || name === 'BusStation') {
    // 长宽异常情况，需要手动调整
    if (otherData.width > otherData.length) {
      const temp = otherData.length
      otherData.length = otherData.width
      otherData.width = temp
    }
  }

  if (roadId && roadId !== '-1' && laneLinkId === '-1') {
    // 道路上
    const road = getRoad(roadId)
    if (!road) return null

    // 基于道路的参考线，计算 st 坐标对应的实际位置
    const {
      targetPoint,
      // 校正后合理有效的 st 坐标
      s: validS,
      t: validT,
      percent,
      tangent,
      refLinePoint,
      normal,
    } = getLocationByST({
      curvePath: road.keyPath,
      useElevationTangent: true,
      elevationPath: road.elevationPath,
      s: _s,
      t: _t,
    })
    // 更新在道路上的相关属性
    otherData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
    otherData.roadId = roadId
    otherData.s = validS
    otherData.t = validT
    otherData.closestPoint = refLinePoint
    otherData.closestPointPercent = percent
    otherData.closestPointTangent = tangent
    otherData.projectNormal = normal

    // 通过偏航角计算看向的点【偏航角从弧度制转换成角度制】
    const _yaw = fixedPrecision(MathUtils.radToDeg(_hdg))
    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: refLinePoint,
      tangent,
      point: targetPoint,
      yaw: _yaw,
      useElevation: true,
    })
    otherData.lookAtPoint = lookAtPoint
    otherData.yaw = _yaw
  } else if (laneLinkId && laneLinkId !== '-1') {
    // 路口中
    // 将路口中的 laneLink 参考线 st 坐标，转换成在路口中的绝对坐标
    const queryRes = getLaneLinkById(laneLinkId)
    if (!queryRes) return null
    const { junction, laneLink } = queryRes
    const { id: junctionId } = junction

    // 通过 laneLink 采样点形成的参考线获取 st 参考线坐标对应的实际位置
    const { targetPoint, angle } = getLocationByLaneLinkST({
      laneLink,
      s: _s,
      t: _t,
      yaw: fixedPrecision(MathUtils.radToDeg(_hdg)),
    })
    otherData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
    otherData.junctionId = junctionId
    // 由于路口的路面标识可以调整角度，因此需要基于 laneLink 的偏航角计算路口的全局角度
    otherData.angle = angle
    // 针对路口中的物体，基于全局角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle,
    })
    otherData.lookAtPoint = lookAtPoint
  }

  // 如果是人行天桥，从userdata中解析跨度参数
  if (name === 'PedestrianBridge') {
    const { userdata = [] } = commonData
    const _targetData = otherData as biz.IOther
    // 天桥跨度先使用默认值
    let span = Constant.bridgeSpan
    for (const _userData of userdata) {
      const { code, value } = _userData
      if (code === 'bridge_span') {
        // 如果 userdata 中存在天桥跨度的透传参数，则使用透传的值
        span = fixedPrecision(Number(value))
      }
    }
    // 将跨度挂载到天桥的数据结构中
    _targetData.span = span
  }

  setObject(otherData.id, otherData)

  return otherData
}

// 解析自定义导入模型的物体
function parseCustomModel (params: {
  config: ICustomModelConfig
  data: biz.ICommonObject
}) {
  const { config, data: commonData } = params
  const {
    id: objectId,
    roadid: roadId,
    lanelinkid: laneLinkId,
    s,
    t,
    hdg,
    width,
    length,
    height,
  } = commonData
  const {
    name,
    width: basicWidth,
    length: basicLength,
    height: basicHeight,
  } = config

  const modelData: biz.ICustomModel = {
    ...createDefaultObjectData(),
    basicWidth,
    basicLength,
    basicHeight,
  }

  const _hdg = Number(hdg)

  const _s = Number(s)
  const _t = Number(t)
  modelData.id = objectId
  modelData.mainType = 'customModel'
  modelData.name = name
  modelData.showName = name
  modelData.type = ''
  modelData.subtype = ''
  // 同步尺寸
  modelData.width = fixedPrecision(Number(width))
  modelData.length = fixedPrecision(Number(length))
  modelData.height = fixedPrecision(Number(height))

  if (roadId && roadId !== '-1' && laneLinkId === '-1') {
    // 在道路上
    const road = getRoad(roadId)
    if (!road) return null

    // 基于道路的参考线，计算 st 坐标对应的实际位置
    const {
      targetPoint,
      // 校正后合理有效的 st 坐标
      s: validS,
      t: validT,
      percent,
      tangent,
      refLinePoint,
      normal,
    } = getLocationByST({
      curvePath: road.keyPath,
      useElevationTangent: true,
      elevationPath: road.elevationPath,
      s: _s,
      t: _t,
    })

    // 更新在道路上的相关属性
    modelData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
    modelData.roadId = roadId
    modelData.s = validS
    modelData.t = validT
    modelData.closestPoint = refLinePoint
    modelData.closestPointPercent = percent
    modelData.closestPointTangent = tangent
    modelData.projectNormal = normal

    // 通过偏航角计算看向的点【偏航角从弧度制转换成角度制】
    const _yaw = fixedPrecision(MathUtils.radToDeg(_hdg))
    const lookAtPoint = getLookAtPointByYaw({
      pointOnRefLine: refLinePoint,
      tangent,
      point: targetPoint,
      yaw: _yaw,
      useElevation: true,
    })
    modelData.lookAtPoint = lookAtPoint
    modelData.yaw = _yaw
  } else if (laneLinkId && laneLinkId !== '-1') {
    // 在路口中
    // 将路口中的 laneLink 参考线 st 坐标，转换成在路口中的绝对坐标
    const queryRes = getLaneLinkById(laneLinkId)
    if (!queryRes) return null
    const { junction, laneLink } = queryRes
    const { id: junctionId } = junction

    // 通过 laneLink 采样点形成的参考线获取 st 参考线坐标对应的实际位置
    const { targetPoint, angle } = getLocationByLaneLinkST({
      laneLink,
      s: _s,
      t: _t,
      yaw: fixedPrecision(MathUtils.radToDeg(_hdg)),
    })
    modelData.position = {
      x: targetPoint.x,
      y: targetPoint.y,
      z: targetPoint.z,
    }
    modelData.junctionId = junctionId
    // 由于路口的路面标识可以调整角度，因此需要基于 laneLink 的偏航角计算路口的全局角度
    modelData.angle = angle
    // 针对路口中的物体，基于全局角度来更新物体的朝向目标
    const lookAtPoint = getLookAtPointByAngle({
      point: targetPoint,
      angle,
    })
    modelData.lookAtPoint = lookAtPoint
  }

  setObject(modelData.id, modelData)

  return modelData
}
