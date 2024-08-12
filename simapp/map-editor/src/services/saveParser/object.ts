import { MathUtils, Vector2 } from 'three'
import {
  needUpdateOtherName,
  needUpdateRoadSignName,
} from '../loadParser/object'
import { useObjectStore } from '@/stores/object'
import {
  PI,
  fixedPrecision,
  getClosestPointFromCurvePath,
  getTValue,
  getValidDegreeInRange,
  getWorldLocation,
  getYawInJunction,
  halfPI,
} from '@/utils/common3d'
import { getJunction, getObject, getRoad } from '@/utils/mapCache'
import { getPoleConfig } from '@/config/pole'
import pool from '@/services/worker'
import { getRoadSignConfig } from '@/config/roadSign'
import { globalConfig } from '@/utils/preset'
import { useJunctionStore } from '@/stores/junction'
import root3d from '@/main3d'
import { applyDefaultControlJunctionAndRoad } from '@/stores/object/common'
import { MapVersion } from '@/utils/business'
import { useFileStore } from '@/stores/file'
import { useRoadStore } from '@/stores/road'
import { genObjectId } from '@/utils/guid'
import { getOriginMapCache } from '@/utils/tools'
import { saveParseIntersectionGuideLine } from '@/stores/object/customRoadSign/intersectionGuideLine'
import { useModelCacheStore } from '@/stores/object/modelCache'

// 创建一个空的默认透传物体数据结构
function createEmptyCommonObject () {
  const commonObject: biz.ICommonObject = {
    id: '',
    type: '',
    subtype: '',
    name: '',
    roadid: '-1',
    lanelinkid: '-1',
    s: '0',
    t: '0',
    zOffset: '0', // 默认为 0，对于杆上的物体需要关注
    validLength: '0',
    orientation: '+', // 朝向默认为 +
    radius: '0',
    length: '0',
    width: '0',
    height: '0',
    hdg: '0',
    pitch: '0',
    roll: '0',
  }
  return commonObject
}

// 创建一个重复属性的默认对象
function createEmptyRepeatProperty () {
  const repeatProperty: biz.IRepeatProperty = {
    s: '0',
    length: '0',
    distance: '0',
    tStart: '0',
    tEnd: '0',
    widthStart: '0',
    widthEnd: '0',
    lengthStart: '0',
    lengthEnd: '0',
    heightStart: '0',
    heightEnd: '0',
    zOffsetStart: '0',
    zOffsetEnd: '0',
  }
  return repeatProperty
}

// 提取物体级别的元素
export async function pickUpObjectData () {
  const _objects: Array<biz.ICommonObject> = []
  const objectStore = useObjectStore()
  const fileStore = useFileStore()
  const { mapVersion } = fileStore
  // 是否是旧版 v1.0 版本的地图
  let isOldMap = false
  if (mapVersion === MapVersion.v1) {
    isOldMap = true
  }

  // 用来临时存储杆解析后的数据
  const poleMap: Map<string, biz.ICommonObject> = new Map()

  // 没有关联可以并行处理的前置异步解析
  const presetParsePromises = []

  // 第一轮遍历，优先处理杆【主要是杆】、路面标识、其他类型
  for (const objectId of objectStore.ids) {
    const objectData = getObject(objectId)
    if (!objectData) continue
    // 从前端侧存储的源数据中，提取要透传的属性
    const { type, subtype, mainType } = objectData

    // 第一轮先跳过杆上元素：标志牌、信号灯、传感器
    if (
      mainType === 'signalBoard' ||
      mainType === 'trafficLight' ||
      mainType === 'sensor'
    ) {
      continue
    }

    // 创建一个空的默认透传物体数据结构
    const _object: biz.ICommonObject = createEmptyCommonObject()
    _object.id = objectId
    _object.type = type
    _object.subtype = subtype
    // 由于计算路口中的参考线坐标属性，可能会存在性能问题
    // 因此将可能直接放置在路口中的物体解析过程封装成异步函数，调用 webWorker 线程，并行执行提升解析性能
    if (mainType === 'pole') {
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            parsePole({
              originData: objectData as biz.IPole,
              targetData: _object,
              isOldMap,
            }).then((data) => {
              // 如果解析完以后，roadId 和 laneLinkId 均无效，则该物体解析失败不保存到透传数据结构中
              if (!data || (data.roadid === '-1' && data.lanelinkid === '-1')) {
                resolve('')
                return
              }

              // 临时保存解析后的杆数据，用于后续解析杆上的信号灯、标志牌等
              poleMap.set(objectId, data)
              _objects.push(data)
              resolve(data.id)
            })
          })
        })(),
      )
      continue
    } else if (mainType === 'roadSign') {
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            parseRoadSign({
              originData: objectData as biz.IRoadSign,
              targetData: _object,
              isOldMap,
            }).then((data) => {
              // 如果解析完以后，roadId 和 laneLinkId 均无效，则该物体解析失败不保存到透传数据结构中
              if (!data || (data.roadid === '-1' && data.lanelinkid === '-1')) {
                resolve('')
                return
              }
              _objects.push(data)
              resolve(data.id)
            })
          })
        })(),
      )
      continue
    } else if (mainType === 'parkingSpace') {
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            parseParkingSpace({
              originData: objectData as biz.IParkingSpace,
              targetData: _object,
              isOldMap,
            }).then((data) => {
              // 如果解析完以后，roadId 和 laneLinkId 均无效，则该物体解析失败不保存到透传数据结构中
              if (!data || (data.roadid === '-1' && data.lanelinkid === '-1')) {
                resolve('')
                return
              }
              _objects.push(data)
              resolve(data.id)
            })
          })
        })(),
      )
      continue
    } else if (mainType === 'other') {
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            parseOther({
              originData: objectData as biz.IOther,
              targetData: _object,
              isOldMap,
            }).then((data) => {
              // 如果解析完以后，roadId 和 laneLinkId 均无效，则该物体解析失败不保存到透传数据结构中
              if (!data || (data.roadid === '-1' && data.lanelinkid === '-1')) {
                resolve('')
                return
              }
              _objects.push(data)
              resolve(data.id)
            })
          })
        })(),
      )
      continue
    } else if (mainType === 'customModel') {
      presetParsePromises.push(
        (() => {
          return new Promise((resolve) => {
            parseCustomModel({
              originData: objectData as biz.ICustomModel,
              targetData: _object,
            }).then((data) => {
              // 如果解析完以后，roadId 和 laneLinkId 均无效，则该物体解析失败不保存
              if (!data || (data.roadid === '-1' && data.lanelinkid === '-1')) {
                resolve('')
                return
              }
              _objects.push(data)
              resolve(data.id)
            })
          })
        })(),
      )
      continue
    }
  }

  // 执行第一轮先行的解析逻辑
  await Promise.all(presetParsePromises)

  // 第二轮遍历，只处理标志牌和信号灯。需要依赖外层的杆容器解析后的属性
  const parseObjectOnPolePromises = []
  for (const objectId of objectStore.ids) {
    const objectData = getObject(objectId) as
      | biz.ISignalBoard
      | biz.ITrafficLight
    if (!objectData) continue
    const { mainType, type, subtype, poleId } = objectData
    if (
      mainType !== 'signalBoard' &&
      mainType !== 'trafficLight' &&
      mainType !== 'sensor'
    ) {
      continue
    }

    // 获取对应的杆容器的解析后属性
    const poleData = poleMap.get(poleId)
    if (!poleData) continue

    const _object = createEmptyCommonObject()
    _object.id = objectId
    _object.type = type
    _object.subtype = subtype

    parseObjectOnPolePromises.push(
      (() => {
        return new Promise((resolve) => {
          parseObjectOnPole({
            type: mainType,
            originData: objectData as biz.ISignalBoard,
            targetData: _object,
            isOldMap,
          }).then((data) => {
            if (!data || (data.roadid === '-1' && data.lanelinkid === '-1')) {
              resolve('')
              return
            }

            _objects.push(data)
            resolve(data.id)
          })
        })
      })(),
    )
  }

  await Promise.all(parseObjectOnPolePromises)

  // 解析道路，如果有隧道需要将隧道以物体的形式存储
  const roadStore = useRoadStore()
  for (const _roadId of roadStore.ids) {
    const road = getRoad(_roadId) as biz.ITunnel
    if (!road) continue
    // 如果是隧道
    if (road.roadType === 'tunnel') {
      const { tunnelS, tunnelLength } = road

      // 获取当前道路上，所有车道的宽度总和
      let roadWidth = 0
      road.sections[0].lanes.forEach((lane) => {
        roadWidth += lane.normalWidth
      })

      // 匹配在缓存中是否存在原有的虚拟隧道物体
      let originTunnelObject: biz.ICommonObject | null = null
      const originMapCache = getOriginMapCache()
      if (originMapCache) {
        for (const originObject of originMapCache.objects) {
          if (originTunnelObject) continue
          const { name, roadid = '' } = originObject
          if (name === 'Tunnel' && roadid === _roadId) {
            originTunnelObject = originObject
          }
        }
      }
      // 如果原先存在关联的虚拟隧道物体，则沿用之前的 id；否则新建一个 id
      const tunnelId = originTunnelObject ?
        originTunnelObject.id :
        genObjectId()

      // 创建一个隧道的物体
      const tunnelObject: biz.ICommonObject = {
        id: tunnelId,
        type: 'none',
        subtype: '',
        name: 'Tunnel', // 主要通过 name 属性来识别
        roadid: _roadId,
        lanelinkid: '-1', // 跟 lanelink 无关
        // s: '0', // 从道路 s 为 0 处开始
        s: String(fixedPrecision(tunnelS)),
        t: '0',
        zOffset: '0',
        validLength: '',
        orientation: '+', // 默认朝向为 +
        radius: '0',
        // length: String(fixedPrecision(road.length)), // 长度为道路的长度
        length: String(fixedPrecision(tunnelLength)), // 隧道单独的长度
        width: String(fixedPrecision(roadWidth)), // 当前道路的宽度
        height: '0',
        hdg: '0',
        pitch: '0',
        roll: '0',
      }

      _objects.push(tunnelObject)
    }
  }

  return _objects
}

// ---------- 物体解析使用的工具函数 ----------

// 获取杆容器自身的朝向，到水平向右基准方向的夹角
function getPoleDirectionDeg (
  position: common.vec3, // 杆容器的坐标
  lookAtPoint: common.vec3, // 杆容器所看向的点
) {
  // 先计算杆容器的朝向，到水平向右基准方向的夹角
  const deg = MathUtils.radToDeg(
    new Vector2(lookAtPoint.z - position.z, lookAtPoint.x - position.x)
      .normalize()
      .angle(),
  )
  return deg
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

// 获取杆上的物体在参考线坐标系下的偏航角
function getObjectYawInPole (params: {
  objectAngle: number // 杆上物体在杆容器中的局部夹角
  objectTangent: common.vec3 // 杆上物体在 s 轴投影点的切线方向
  poleAngle: number // 杆自身朝向到水平向右基准方向的夹角
}) {
  const { objectAngle, objectTangent, poleAngle } = params
  const tangentVec2 = new Vector2(objectTangent.z, objectTangent.x)
  // 切线方向到水平向右基准方向的夹角
  const tangentDeg = MathUtils.radToDeg(tangentVec2.angle())

  // 杆上的物体，本身跟杆存在固定 90° 的朝向偏差
  const fixedAngle = 90
  // 杆上物体朝向到水平向右基准方向的夹角 = 杆的角度 + 固定偏差 + 杆上物体在杆容器中的角度
  const objectDirectionDeg = poleAngle + fixedAngle + objectAngle

  // 偏航角
  const yaw = objectDirectionDeg - tangentDeg
  // 将偏航角限制在 [-180, 180] 度的范围中
  const _yaw = fixedPrecision(getValidDegreeInRange(yaw))

  return _yaw
}

// ---------- 保存解析物体数据的入口函数 ----------
async function parseParkingSpace (params: {
  originData: biz.IParkingSpace
  targetData: biz.ICommonObject
  isOldMap: boolean
}) {
  const { originData, targetData, isOldMap = false } = params
  const {
    name,
    type,
    subtype,
    length,
    width,
    lineWidth,
    color,
    innerAngle,
    margin,
    s,
    t,
    yaw,
    roadId = '',
    junctionId = '',
    sCoordinates,
    cornerSTPoints, // st 坐标系下的角点
  } = originData

  const _s = String(fixedPrecision(s))
  const _t = String(fixedPrecision(t))
  const _width = String(fixedPrecision(width))
  const _length = String(fixedPrecision(length))
  const _lineWidth = String(fixedPrecision(lineWidth))
  const _color = color.toLowerCase() // 转换成全部小写的颜色字符串

  targetData.name = name
  targetData.type = type
  targetData.subtype = subtype
  // s 坐标指的是第一个停车位中心点 s 坐标
  targetData.s = _s
  targetData.t = _t
  // 单个停车位的宽度和长度
  targetData.width = _width
  targetData.length = _length
  // 一组停车位共用一个偏航角【保存成弧度制】
  targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))

  // repeat 属性
  const repeatProperty = createEmptyRepeatProperty()

  if (roadId) {
    // 道路上的停车位
    targetData.roadid = roadId

    // 停车位内部夹角，用 userdata 保存
    const userdata: Array<biz.IUserData> = []
    userdata.push({
      code: 'parking_angle',
      value: String(fixedPrecision(innerAngle)),
    })
    targetData.userdata = userdata

    // 停车位描边 outlines 和样式 markings 保存
    const outlines: Array<biz.IOutline> = []
    const markings: Array<biz.IMarking> = []
    // 局部的角点 id
    let basicId = 0
    for (let i = 0; i < cornerSTPoints.length; i++) {
      // 一组停车位的4个角点
      const stPoints = cornerSTPoints[i]
      // 固定是4个角点
      const fixedLength = 4
      // 前端侧计算的角点是按顺时针排序的
      // 透传给后端的角点，需要逆时针排序，所以从后往前遍历
      for (let j = fixedLength - 1; j >= 0; j--) {
        // 循环到下一个角点的索引
        const nextIndex = j - 1 < 0 ? j - 1 + fixedLength : j - 1
        // 基于基准 id 定义每个角点的 id
        const currentId = String(basicId + (fixedLength - j - 1))
        const nextPointId = String(basicId + (fixedLength - nextIndex - 1))

        const currentPoint = stPoints[j]
        // outlint 每一次迭代只加当前点
        outlines.push({
          id: currentId,
          s: String(fixedPrecision(currentPoint.x)),
          t: String(fixedPrecision(currentPoint.y)),
        })
        // marking 每一次迭代，需要考虑下一个点的连接
        markings.push({
          cornerreferenceid: `${currentId}_${nextPointId}`,
          width: _lineWidth,
          color: _color,
        })
      }
      // 基准 id 自增
      basicId += fixedLength
    }
    targetData.outlines = outlines
    targetData.markings = markings

    // 兼容旧版的保存
    if (isOldMap) {
      // 单个停车位的长度，乘以一组停车位的数量
      targetData.length = String(fixedPrecision(length * sCoordinates.length))
    }

    // 更新重复属性
    repeatProperty.s = _s
    // 第一个停车位中心点到最后一个停车位中心点的 s 距离
    const firstS = sCoordinates[0]
    const lastS = sCoordinates[sCoordinates.length - 1]
    repeatProperty.length = String(fixedPrecision(lastS - firstS))
    // 相邻停车位中心点的间距
    repeatProperty.distance = String(fixedPrecision(margin))
    repeatProperty.tStart = _t
    repeatProperty.tEnd = _t
    repeatProperty.widthStart = _width
    repeatProperty.widthEnd = _width
    repeatProperty.lengthStart = _length
    repeatProperty.lengthEnd = _length
  } else if (junctionId) {
    // 暂不支持路口停车位的保存
  }

  // 将重复属性挂载到通用数据格式中
  targetData.repeat = repeatProperty

  return targetData
}

// 提取杆的透传属性
async function parsePole (params: {
  originData: biz.IPole
  targetData: biz.ICommonObject
  isOldMap: boolean
}) {
  const { originData, targetData, isOldMap = false } = params
  const {
    name,
    roadId = '',
    junctionId = '',
    yaw,
    s,
    t,
    radius,
    height,
  } = originData

  targetData.radius = String(radius)
  targetData.height = String(height)
  targetData.name = name

  const poleConfig = getPoleConfig(name)
  if (!poleConfig) return null
  // 由于杆的竖直部分是圆柱，在 s 轴上的投影有效长度，应该始终为竖直部分的直径竖直
  targetData.validLength = String(poleConfig.style.vRadius * 2)

  if (roadId) {
    // 道路上的杆
    targetData.roadid = roadId
    // 精度调整
    targetData.s = String(fixedPrecision(s))
    targetData.t = String(fixedPrecision(t))
    // 偏航角保存成弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  } else if (junctionId) {
    const junction = getJunction(junctionId)
    if (!junction || junction.laneLinks.length < 1) return null

    const { position, angle } = originData
    const payload = {
      laneLinks: junction.laneLinks,
      point: position,
      // 先不指定 laneLink 的 id
    }
    // 通过 webWorker 线程计算在路口中的 st 坐标属性
    const locationRes = await pool.exec('getLocationInJunction', [payload])

    if (!locationRes) return null
    targetData.lanelinkid = locationRes.laneLinkId
    targetData.roadid = locationRes.roadid
    targetData.s = String(fixedPrecision(locationRes.s))
    targetData.t = String(fixedPrecision(locationRes.t))
    // 计算 laneLink 参考线坐标系下的偏航角
    const yaw = getYawInJunction({
      angle,
      tangent: locationRes.tangent,
    })
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  }

  // 兼容旧版
  if (isOldMap) {
    const tempHdg = Number(targetData.hdg)
    targetData.hdg = String(fixedPrecision(tempHdg - halfPI))
  }

  return targetData
}
// 提取路面标识的属性
async function parseRoadSign (params: {
  originData: biz.IRoadSign
  targetData: biz.ICommonObject
  isOldMap: boolean
}) {
  const { originData, targetData, isOldMap = false } = params
  const {
    name,
    roadId = '',
    junctionId = '',
    s,
    t,
    yaw,
    width,
    length,
    height,
  } = originData

  if (name === 'Intersection_Guide_Line') {
    // 针对路口导向线，特殊处理
    return saveParseIntersectionGuideLine({
      originData,
      targetData,
    })
  }

  // 在道路中和路口中的通用属性解析
  // 路面标识存在规范的 name 字段
  targetData.name = name
  targetData.width = String(fixedPrecision(width))
  targetData.height = String(fixedPrecision(height))
  targetData.length = String(fixedPrecision(length))

  if (roadId) {
    // 如果物体是跟道路关联的
    targetData.roadid = roadId
    // 精度调整
    targetData.s = String(fixedPrecision(s))
    targetData.t = String(fixedPrecision(t))

    const config = getRoadSignConfig(name)
    // 偏航角保存成弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  } else if (junctionId) {
    const junction = getJunction(junctionId)
    if (!junction || junction.laneLinks.length < 1) return null

    const { position, angle } = originData
    const payload = {
      laneLinks: junction.laneLinks,
      point: position,
      // 先不指定 laneLink 的 id
    }
    // 通过 webWorker 线程计算在路口中的 st 坐标属性
    const locationRes = await pool.exec('getLocationInJunction', [payload])

    if (!locationRes) return null
    targetData.lanelinkid = locationRes.laneLinkId
    targetData.roadid = locationRes.roadid
    targetData.s = String(fixedPrecision(locationRes.s))
    targetData.t = String(fixedPrecision(locationRes.t))
    // 计算 laneLink 参考线坐标系下的偏航角
    const _yaw = getYawInJunction({
      angle,
      tangent: locationRes.tangent,
    })
    // 偏航角用弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(_yaw)))
  }

  // 兼容旧版
  if (isOldMap) {
    if (needUpdateRoadSignName.includes(name)) {
      const tempHdg = Number(targetData.hdg)
      targetData.hdg = String(fixedPrecision(tempHdg - PI))
    }
  }

  return targetData
}

// 针对摄像头、激光雷达、毫米波雷达、rsu 4 种传感器类型，提取对应的设备参数
function getSensorDeviceParams (params: {
  deviceParams: any
  sensorName: string
  isCloud: boolean // 根据是云端还是单机版，调整要提取的内容
}) {
  const { deviceParams = null, sensorName, isCloud = false } = params
  const userdata: Array<biz.IUserData> = []

  // 保存之前，校验传感器关联参数是否有效：
  // 1、通信单元关联的路口有效性
  // 2、传感器关联的通信单元有效性
  if (
    sensorName === 'Camera' ||
    sensorName === 'Lidar' ||
    sensorName === 'Millimeter_Wave_Radar'
  ) {
    // 校验通信单元有效性
    const rsuId = deviceParams.BelongRSU
    if (rsuId) {
      const rsuData = getObject(rsuId)
      if (!rsuData) {
        // 如果通信单元不存在，则清空
        deviceParams.BelongRSU = ''
      }
    }
  } else if (sensorName === 'RSU') {
    // 校验路口的有效性
    const junctionStore = useJunctionStore()
    const validJunctionIds: Array<string> = []
    deviceParams.JunctionIDs.forEach((id: string) => {
      if (junctionStore.ids.includes(id)) {
        validJunctionIds.push(id)
      }
    })
    // 只保留有效的路口 id
    deviceParams.JunctionIDs = validJunctionIds
  }

  if (isCloud) {
    // 如果是云端版，则只需要透传配置设备的 id 和关联的设备或路口
    switch (sensorName) {
      case 'Camera':
      case 'Lidar':
      case 'Millimeter_Wave_Radar': {
        // 透传关联的通信单元和已经配置好的传感器配置 id
        userdata.push(
          {
            code: 'V2X_idx',
            value: deviceParams.V2X_idx,
          },
          {
            code: 'BelongRSU',
            value: deviceParams.BelongRSU,
          },
        )
        break
      }
      case 'RSU': {
        // 通信单元只透传关联的路口 id 和已配置好的通信单元 id
        userdata.push(
          {
            code: 'V2X_idx',
            value: deviceParams.V2X_idx,
          },
          {
            code: 'JunctionIDs',
            // 关联的路口 id，使用 , 逗号进行分隔保存成字符串
            value: deviceParams.JunctionIDs.join(','),
          },
        )
        break
      }
      default:
        break
    }
    return userdata
  }

  switch (sensorName) {
    case 'Camera': {
      // 摄像头需要根据 IntrinsicType 内参形式，调整透传的参数
      const { IntrinsicType } = deviceParams
      // 忽略不透传的参数名称
      const ignoreParamNames = []
      // 0 表示矩阵，1 表示 FOV，2 表示感光器
      if (IntrinsicType === '0') {
        ignoreParamNames.push(
          'FovHorizontal',
          'FovVertical',
          'CcdWidth',
          'CcdHeight',
          'CcdFocal',
        )
      } else if (IntrinsicType === '1') {
        ignoreParamNames.push(
          'IntrinsicMat',
          'CcdWidth',
          'CcdHeight',
          'CcdFocal',
        )
      } else if (IntrinsicType === '2') {
        ignoreParamNames.push('IntrinsicMat', 'FovHorizontal', 'FovVertical')
      }
      for (const key of Object.keys(deviceParams)) {
        if (ignoreParamNames.includes(key)) continue
        // 畸变参数
        if (key === 'Distortion') {
          const { k1, k2, k3, p1, p2 } = deviceParams[key]
          const val = `${k1},${k2},${k3},${p1},${p2}`
          userdata.push({
            code: key,
            value: val,
          })
        } else if (key === 'IntrinsicMat') {
          const { fx, skew, cx, param4, fy, cy, param7, param8, param9 } =
            deviceParams[key]
          const val = `${fx},${skew},${cx},${param4},${fy},${cy},${param7},${param8},${param9}`
          userdata.push({
            code: key,
            value: val,
          })
        } else {
          userdata.push({
            code: key,
            value: deviceParams[key],
          })
        }
      }
      break
    }
    case 'Lidar':
    case 'Millimeter_Wave_Radar':
    case 'RSU':
      for (const key of Object.keys(deviceParams)) {
        if (sensorName === 'RSU' && key === 'JunctionIDs') {
          // rsu 保存关联的路口，使用 , 逗号分隔的字符串
          userdata.push({
            code: key,
            value: deviceParams[key].join(','),
          })
        } else {
          userdata.push({
            code: key,
            value: deviceParams[key],
          })
        }
      }
      break
    default:
      break
  }
  return userdata
}

/**
 * 解析杆上的信号灯、标志牌、传感器的逻辑大体类似，但需要针对具体的类型做解析
 * 1、信号灯需要解析信控配置
 * 2、传感器需要解析摄像头的俯仰角，以及传感器参数
 */
async function parseObjectOnPole (params: {
  type: 'trafficLight' | 'signalBoard' | 'sensor'
  originData: biz.ITrafficLight | biz.ISignalBoard | biz.ISensor
  targetData: biz.ICommonObject
  isOldMap: boolean
}) {
  const { type: mainType, originData, targetData, isOldMap = false } = params

  const {
    name,
    type,
    subtype,
    position,
    width,
    height,
    length,
    roadId,
    junctionId,
    poleId,
    angle,
  } = originData

  const poleData = getObject(poleId)
  if (!poleData) return null

  targetData.name = name
  targetData.type = type
  targetData.subtype = subtype

  // 尺寸
  targetData.width = String(fixedPrecision(width))
  targetData.height = String(fixedPrecision(height))
  targetData.length = String(fixedPrecision(length))

  // 在杆中的局部坐标，可以直接获取到距离地面的高度
  targetData.zOffset = String(fixedPrecision(position.y))

  // 需要从渲染层，获取到杆上物体在世界坐标系下的实际定位
  const worldPosition = getWorldLocation({
    objectId: targetData.id,
    parent: root3d.mapElementsContainer,
  })
  if (!worldPosition) return null

  if (roadId && roadId !== '-1') {
    // 杆放置在道路上，杆上物体需要绑定 roadId
    if (!poleData.lookAtPoint) return null
    const road = getRoad(roadId)
    if (!road) return null
    targetData.roadid = roadId

    // 基于物体的世界坐标，计算在道路参考线上的 st 坐标
    const {
      point: closestPoint,
      tangent,
      percent,
    } = getClosestPointFromCurvePath({
      curvePath: road.keyPath,
      point: worldPosition,
    })
    const s = fixedPrecision(road.length * percent)
    const t = getTValue({
      tangent,
      refLinePoint: closestPoint,
      targetPoint: worldPosition,
    })

    targetData.s = String(fixedPrecision(s))
    targetData.t = String(fixedPrecision(t))

    // 先计算杆的角度
    const poleAngle = getPoleDirectionDeg(
      poleData.position,
      poleData.lookAtPoint,
    )

    // 杆上物体自身朝向，跟它所对应的 s 轴投影点切线方向的夹角，即为偏航角
    const yaw = getObjectYawInPole({
      objectAngle: Number(angle),
      objectTangent: tangent,
      poleAngle,
    })
    // 偏航角用弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  } else if (junctionId) {
    // 杆放置在路口中，杆上的物体也会拥有 junctionId 属性
    const junction = getJunction(junctionId)
    if (!junction) return

    const payload = {
      laneLinks: junction.laneLinks,
      point: worldPosition,
    }
    // 通过 webWorker 线程计算在路口中的 st 坐标属性
    // 【注意】杆上的物体（尤其是横杆上的物体）对应的最优 laneLink，跟所在杆容器关联的 laneLink 不一定是同一条
    const locationRes = await pool.exec('getLocationInJunction', [payload])

    if (!locationRes) return null
    targetData.lanelinkid = locationRes.laneLinkId
    targetData.roadid = locationRes.roadid
    targetData.s = String(fixedPrecision(locationRes.s))
    targetData.t = String(fixedPrecision(locationRes.t))

    // 计算杆上物体在 laneLink 参考线坐标系下的偏航角
    const yaw = getObjectYawInPole({
      objectAngle: Number(angle),
      objectTangent: locationRes.tangent,
      // 由于路口中的杆使用 angle 控制角度，可以直接传入
      poleAngle: poleData.angle,
    })
    // 偏航角用弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  }

  if (mainType === 'trafficLight') {
    // 如果信号灯没有手动配置信控路口和道路，则自动赋值
    applyDefaultControlJunctionAndRoad(originData.id)

    // 通过 userdata 保存信控配置
    const { controlJunctionId, controlRoadId } = originData as biz.ITrafficLight
    const userdata = []
    if (controlJunctionId) {
      userdata.push({
        code: 'relate_control',
        value: controlJunctionId,
      })
    }
    if (controlRoadId) {
      userdata.push({
        code: 'relate_road',
        value: controlRoadId,
      })
    }
    if (userdata.length > 0) {
      // 将信控配置保存到目标数据结构中
      targetData.userdata = userdata
    }
  } else if (mainType === 'signalBoard') {
    //
  } else if (mainType === 'sensor') {
    const { deviceParams } = originData as biz.ISensor
    // 传感器设备参数，以及摄像头俯仰角
    const userdata = getSensorDeviceParams({
      deviceParams,
      sensorName: name,
      isCloud: globalConfig.isCloud,
    })
    if (userdata.length > 0) {
      targetData.userdata = userdata
    }
  }

  // 先判断是否存在 userdata 属性挂载在目标对象中
  if (!targetData.userdata) {
    targetData.userdata = []
  }
  // 将杆的 id 绑定到 userdata 中
  const poleInfo: biz.IUserData = {
    code: 'pole_id',
    value: poleId,
  }
  targetData.userdata.push(poleInfo)

  // 兼容旧版
  if (isOldMap) {
    if (mainType === 'trafficLight' || mainType === 'signalBoard') {
      const tempHdg = Number(targetData.hdg)
      targetData.hdg = String(fixedPrecision(tempHdg - PI))
    }
  }

  return targetData
}

// 提取其他类型物体的属性
async function parseOther (params: {
  originData: biz.IOther
  targetData: biz.ICommonObject
  isOldMap: boolean
}) {
  const { originData, targetData, isOldMap = false } = params
  const {
    name,
    type,
    subtype,
    roadId = '',
    junctionId = '',
    s,
    t,
    yaw,
    width,
    length,
    height,
  } = originData

  // 标识
  targetData.name = name
  targetData.type = type
  targetData.subtype = subtype

  // 尺寸
  targetData.width = String(fixedPrecision(width))
  targetData.length = String(fixedPrecision(length))
  targetData.height = String(fixedPrecision(height))

  if (roadId) {
    // 道路上
    targetData.roadid = roadId
    targetData.s = String(fixedPrecision(s))
    targetData.t = String(fixedPrecision(t))

    // 偏航角用弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  } else if (junctionId) {
    // 路口中
    const junction = getJunction(junctionId)
    if (!junction || junction.laneLinks.length < 1) return null

    const { position, angle } = originData
    const payload = {
      laneLinks: junction.laneLinks,
      point: position,
    }
    const locationRes = await pool.exec('getLocationInJunction', [payload])

    if (!locationRes) return null

    targetData.lanelinkid = locationRes.laneLinkId
    targetData.roadid = locationRes.roadid
    targetData.s = String(fixedPrecision(locationRes.s))
    targetData.t = String(fixedPrecision(locationRes.t))

    // 计算 laneLink 参考线坐标系下的偏航角
    const yaw = getYawInJunction({
      angle,
      tangent: locationRes.tangent,
    })
    // 偏航角用弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  }

  // 兼容旧版
  if (isOldMap) {
    if (needUpdateOtherName.includes(name)) {
      const tempHdg = Number(targetData.hdg)
      targetData.hdg = String(fixedPrecision(tempHdg - halfPI))
    }
  }

  // 如果是人行天桥，需要将跨度通过 userdata 保存
  if (name === 'PedestrianBridge' && originData.span) {
    // 先判断是否存在 userdata 属性挂载在目标对象中
    if (!targetData.userdata) {
      targetData.userdata = []
    }
    const spanInfo: biz.IUserData = {
      code: 'bridge_span',
      value: String(fixedPrecision(originData.span)),
    }
    targetData.userdata.push(spanInfo)
  }

  return targetData
}

// 提取自定义导入模型物体的属性
async function parseCustomModel (params: {
  originData: biz.ICustomModel
  targetData: biz.ICommonObject
}) {
  const { originData, targetData } = params
  const {
    name,
    roadId = '',
    junctionId = '',
    s,
    t,
    yaw,
    width,
    length,
    height,
  } = originData

  // 标识
  targetData.name = ''
  targetData.type = 'custom'
  targetData.subtype = ''

  // 传给后端的 name 字段，需要从 variable 获取
  const modelCacheStore = useModelCacheStore()
  const model = modelCacheStore.getCustomModelConfigByName(name)
  if (model) {
    targetData.name = model.variable
  }

  // 尺寸
  targetData.width = String(fixedPrecision(width))
  targetData.length = String(fixedPrecision(length))
  targetData.height = String(fixedPrecision(height))

  if (roadId) {
    // 在道路上
    targetData.roadid = roadId
    targetData.s = String(fixedPrecision(s))
    targetData.t = String(fixedPrecision(t))

    // 偏航角使用弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  } else if (junctionId) {
    // 在路口中
    const junction = getJunction(junctionId)
    if (!junction || junction.laneLinks.length < 1) return null

    const { position, angle } = originData
    const payload = {
      laneLinks: junction.laneLinks,
      point: position,
    }
    const locationRes = await pool.exec('getLocationInJunction', [payload])

    if (!locationRes) return null

    targetData.lanelinkid = locationRes.laneLinkId
    targetData.roadid = locationRes.roadid
    targetData.s = String(fixedPrecision(locationRes.s))
    targetData.t = String(fixedPrecision(locationRes.t))

    // 计算 laneLink 参考线坐标系下得偏航角
    const yaw = getYawInJunction({
      angle,
      tangent: locationRes.tangent,
    })
    // 偏航角使用弧度制
    targetData.hdg = String(fixedPrecision(MathUtils.degToRad(yaw)))
  }

  // 对于自定义模型的解析，需要在 userdata 中添加标识
  if (!targetData.userdata) {
    targetData.userdata = []
  }
  const flagInfo: biz.IUserData = {
    code: 'custom_model',
    value: name,
  }
  targetData.userdata.push(flagInfo)

  return targetData
}

// // 计算物体在参考线坐标系 s 轴上投影的有效长度
// function getValidLength(params: { objectId: string; mainType: string }) {
//   // TODO 可能需要通过包围盒来计算
//   const { mainType, objectId } = params
//   if (
//     mainType !== 'roadSign' &&
//     mainType !== 'trafficLight' &&
//     mainType !== 'signalBoard' &&
//     mainType !== 'other'
//   ) {
//     return null
//   }

//   // 地图元素的三维容器
//   const mapContainer = root3d.mapElementsContainer
//   // 可能找到的是外层的容器
//   const object = mapContainer.getObjectByProperty('objectId', objectId)
//   if (!object) return null
//   // 杆（包括横杆）都视为竖直圆柱型，有效长度即为竖杆直径
//   // 该方法主要针对路面标识、信号灯、标志牌、其他物体【这四类均有外层Group容器】
//   const targetMesh = object.children[0] as Mesh
//   if (!targetMesh) return null
// }
