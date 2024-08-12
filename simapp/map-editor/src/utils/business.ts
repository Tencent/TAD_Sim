/**
 * 跟具体业务场景相关的通用方法
 */
import {
  CatmullRomCurve3,
  type Intersection,
  MathUtils,
  type Object3D,
  type OrthographicCamera,
  type PerspectiveCamera,
  Raycaster,
  Vector2,
  Vector3,
} from 'three'
import NP from 'number-precision'
import {
  CatmullromTension,
  CurveType,
  DefaultColor,
  axisY,
  compareTwoNumberInDeviation,
  compareTwoVector3InDeviation,
  fixedPrecision,
  getBezier2WithDirection,
  getBezier3WithDirection,
  getClosestPointFromCurvePath,
  getLocationByST,
  getSamplePointByOffset,
  getTValue,
  getVerticalVector,
  sortByRenderOrder,
  transformVec3,
  transformVec3ByFlatArray,
  transformVec3ByObject,
} from './common3d'
import { genUuid } from './guid'

// 地图文件类型规范
export const MapType = {
  sqlite: 'sqlite',
  xodr14: 'xodr 1.4',
  xodr15: 'xodr 1.5',
}

// 地图文件的版本
export const MapVersion = {
  v1: 'tadsim v1.0', // 旧版地图编辑器生产的自建地图
  v2: 'tadsim v2.0', // 重构版地图编辑器生产的自建地图
  third: 'three party', // 第三方软件生产的地图
}

export const Constant = {
  laneWidth: 3.5,
  // TODO 根据视角的远近来调整对应的采样间隔【重要】！！！
  // section 中过渡车道的平滑边界，控制点距离端点的距离跟当前车道长度的比例，跟 curveSegment 字段成反比，即 1/curveSegment
  curveSegment: 20, // 曲线划分的 segment 数
  junctionControlPointDistance: 4, // 由多条道路形成的交叉路口，端点距离控制点的距离
  junctionBCPRatio: 0.5, // BCP: Bezier Control Point，交叉路口贝塞尔控制点距离端点，跟两个端点距离之间的比例
  junctionEdgeSegment: 30, // 交叉路口贝塞尔边界的 segment 划分
  laneLinkSegment: 20, // 交叉路口 laneLink 线的 segment 划分，调整成 20 效果也还行，减少文件中保存的采样点数量

  // 平滑过渡补间边界线控制点的间隔
  tweenControlPointSegment: 10,
  // 平滑过渡补间边界线控制点的趋势控制默认指数
  tweenControlPointExponent: 1 / 4,
  // 平滑过渡车道补间采样点的采样间隔
  tweenSampleSegment: 20,
  // 基于道路中心线计算的偏移采样点采样间隔精度
  offsetPointsSamplePrecision: 8,

  // 保存地图时，中心线采样间隔
  centerlineSamplingSegment: 40,

  // 默认保留的小数位
  precision: 3,
  // 厘米级别误差
  cmDeviation: 0.01,
  // 毫米级误差
  mmDeviation: 0.001,

  // 辅助元素曲线采样点数量
  helperCurveSamplePointsNumber: 100,

  // 车道线标识常量
  markWidth: 0.15,
  markOffset: 0.1,
  markUnitLength: 4,
  markGapLength: 6,

  // 道路两侧辅助放置区域的偏移量
  /**
   * 参考线左侧的可放置区域尽可能小
   * 在保证能放某些物体（主要是杆）的前提下
   * 避免跟其他道路的辅助放置区域重合
   */
  maxTValue: 0.5, // 道路上物体，在正向道路参考线坐标系中，t值在反向道路则能延伸的最远距离，跟 expandLeftOffset 相关【正数】
  expandMaxOffset: 11,
  expandMinOffset: 0.5,

  // 竖杆和横杆辅助放置交互圆柱的半径比例
  polePlacementAreaRadiusRatio: 4,

  // 车道宽度范围 [0.4, 99]
  minLaneWidth: 0.4,
  maxLaneWidth: 99,

  // 高程的最大和最小值
  maxElevation: 1000,
  minElevation: -1000,

  // 天桥默认的跨度（双向双车道）
  bridgeSpan: 18,

  // 非机动车禁驶区标记的默认的尺寸，以及最小和最大的尺寸
  nonMotorVehicleSize: {
    default: 13,
    min: 9,
    max: 30,
  },
  // 人行横道
  crosswalkSize: {
    length: {
      default: 14, // 双向双车道（3.5米 * 4）
      max: 50,
      min: 3,
    },
    width: {
      default: 3, // 默认的宽度为 3 米
      max: 5,
      min: 3,
    },
    offset: 3, // 人行横道可延伸到道路外的基本偏移量
  },
  // 行人左右分道的人行横道尺寸
  crosswalkWithSideSize: {
    length: {
      default: 14, // 双向双车道（3.5米 * 4）
      max: 50,
      min: 3,
    },
    width: {
      default: 6.4, // 默认的宽度为 3 米
      max: 10.4, //
      min: 6.4, //
    },
    offset: 3, // 人行横道可延伸到道路外的基本偏移量
  },
  // 停止线
  stopLineSize: {
    length: 7, // 双车道 7 米
    width: 0.4,
  },
  // 单条导向停止线的长度和宽度（跟随车道边界线路径）
  // 可变导向车道线跟导向车道线，共用一套尺寸
  guideLaneLineSize: {
    length: 30, // 默认长度为 30 米
    width: 0.2,
  },
  // 纵向减速标记尺寸（跟随车道边界线路径）
  longitudinalDecelerationSize: {
    length: {
      default: 30, // 默认长度为 30 米
      min: 10,
    },
    width: 0.3,
  },
  // 横向减速标记尺寸（跟随车道边界线路径）
  lateralDecelerationSize: {
    length: {
      default: 37, // 17+20，有三段
      min: 1.35, // 2条白线+1份间隔的长度，135cm
    },
  },
  // 白色半圆状车距确认线（跟随车道边界线路径）
  semicircleLineSize: {
    length: {
      default: 50,
      min: 0.6, // 一个半圆状确认线的长度
    },
  },
  intersectionGuideLine: {
    width: 0.15, // 线宽 15cm
  },
}

export const DefaultLaneBoundaryMark = {
  SingleDashWhite: 0x0000002, // 白色的单虚线，中间边界线默认样式
  SingleSolidWhite: 0x0000001, // 白色的单实线，最外侧（最右侧）边界线默认样式
  DoubleSolidYellow: 0x0010011, // 黄色的双实线，最内侧（最左侧）边界线默认样式
}

export type IBoundaryShapeType =
  | 'Solid'
  | 'Broken'
  | 'SolidSolid'
  | 'BrokenBroken'
  | 'SolidBroken'
  | 'BrokenSolid'
  | 'None'
export const LaneBoundaryType = [
  'Solid',
  'Broken',
  'SolidSolid',
  'BrokenBroken',
  'SolidBroken',
  'BrokenSolid',
  'None',
]
export const LaneBoundaryColor = ['White', 'Yellow', 'Red', 'Green', 'Blue']

// 车道边界线的类型（按位提取）
export const LaneBoundaryMark = {
  // 单线的样式
  None: 0x0000000, // 无边界线
  Solid: 0x0000001, // 实线
  Broken: 0x0000002, // 虚线
  // 双线的样式
  Solid2: 0x0000010, // 无单独含义（与双线配合使用）
  Broken2: 0x0000020, // 无单独含义（与双线配合使用）
  SolidSolid: 0x0000011, // 双实线
  SolidBroken: 0x0000021, // 实虚线（内侧为实，外侧为虚）
  BrokenSolid: 0x0000012, // 虚实线（内侧为虚，外侧为实）
  BrokenBroken: 0x0000022, // 双虚线
  //
  Curb: 0x0000100,
  Fence: 0x0000200,
  RoadEdge: 0x0000400,
  // 粗细
  Bold: 0x0001000, // 加粗线
  // 颜色
  Yellow: 0x0010000, // 黄色线
  Yellow2: 0x0020000, // 无单独含义（与双线配合使用）
  YellowYellow: 0x0030000, // 双黄线
  Red: 0x0050000, // 红色线
  Green: 0x0060000, // 绿色线
  Blue: 0x0070000, // 蓝色线
  //
  LDM: 0x0100000,
  VGL: 0x0200000,
  Shielded: 0x1000000,
  Absent: 0x2000000,
}

// 可选择的车道类型
export const LaneType: Array<biz.ILaneType> = [
  'Driving',
  'Stop',
  'Shoulder',
  'Biking',
  'Sidewalk',
  'Border',
  'Restricted',
  'Parking',
  'Bidirectional',
  'ConnectingRamp',
  'Curb',
  'Entry',
  'Exit',
  'Median',
  'Offramp',
  'Onramp',
  'Rail',
  'RoadWorks',
  'Tram',
  'None', // 将None放到最后一个
]

export enum LaneTypeEnum {
  None = 0, // 无，描述了道路最远边缘处的空间，并无实际内容
  Driving = 1, // 驾驶，描述了一条正常可供行驶、不属于其他类型的到了
  Stop = 2, // 停，高速公路的硬路肩，用于紧急停车
  Shoulder = 3, // 路肩，描述了道路边缘的软边界
  Biking = 4, // 描述了专为骑自行车者保留的车道
  Sidewalk = 5, // 描述了允许行人在上面行走的道路
  Border = 6, // 描述了道路边缘的硬边界，其与正常可供行驶的车道拥有同样的高度
  Restricted = 7, // 限制，描述了不应有车辆在上面行驶的车道，该车道与行车道拥有相同高度，通常会使用实线以及虚线来隔开这类车道
  Parking = 8, // 描述了带停车位的车道
  // 不知道是什么类型，先注释掉
  // MwyEntry= 9,
  // MwyExit= 10,
  Bidirectional = 27, // 连续双向左转
  ConnectingRamp = 29, // 匝道
  Curb = 30, // 马路牙子
  Entry = 31, // 入口，描述了用于平行于主路路段的车道，主要用于加速
  Exit = 32, // 出口，描述了用于平行于主车路段的车道，主要用于减速
  Median = 33, // 分隔带，描述了位于不同方向车道间的车道。在城市中通常用来分割大型道路上不同方向的交通
  Offramp = 34, // 减速车道，驶出高速公路，驶向乡村或城市道路所需的匝道
  Onramp = 35, // 加速车道，有乡村或城市道路引向高速公路的匝道
  Rail = 36, // 铁路轨道
  RoadWorks = 37, // 施工车道
  Tram = 39, // 有轨电车道
}

// 每一种车道类型的默认摩擦力参数
export const LaneFriction = {
  Driving: 0.8,
  Stop: 0.8,
  Shoulder: 1,
  Biking: 0.8,
  Sidewalk: 0.8,
  Border: 1,
  Restricted: 0.8,
  Parking: 0.8,
  Bidirectional: 0.8,
  ConnectingRamp: 0.8,
  Curb: 1,
  Entry: 0.8,
  Exit: 0.8,
  Median: 1,
  Offramp: 0.8,
  Onramp: 0.8,
  Rail: 1,
  RoadWorks: 1,
  Tram: 1,
  None: 0.8,
}

// 三维场景渲染的效果
export enum RenderModel {
  simple = 0,
  normal = 1,
  rich = 2,
}

// 三维场景的交互模式
export enum OperationModel {
  select = 0,
  pan = 1,
  rotate = 2,
}

// 道路元素的渲染层级
export const RenderOrder = {
  boxHelper: 30, // 包围盒辅助线的渲染层级
  textMesh: 20, // 文字说明网格
  signalBoard: 16, // 信号标志牌
  trafficLight: 16, // 交通信号灯
  sensor: 16, // 传感器
  junctionLinkHTPoint: 15, // 车道连接线首尾点
  highLightLaneLink: 14, // linked / selected / hover 状态下的车道连接线
  laneLink: 14, // 车道中的连接线（即中心线）
  pole: 13, // 杆
  customModel: 13, // 自定义导入的三维模型
  other: 13, // 其他类型物体
  roadMask: 13, // 信控编辑模式下，道路辅助蒙层
  junctionMask: 13, // 信控编辑模式下，路口辅助蒙层
  circleRoadLine: 13, // 环形道路编辑模式，辅助线
  laneBoundaryHelperLine: 13, // 车道宽度或数量编辑模式下，车道边界辅助线
  roadSignPlaceholder: 12, // 自定义路面标线的虚拟占位区域
  roadSign: 11, // 路面标识(高于道路和路口的车道连接线，保证鼠标射线能优先检测到)
  parkingSpace: 10, // 停车位轮廓线
  parkingSpacePlaceholder: 9, // 停车位中间虚拟的占位区域
  rsuControlJunctionMask: 8, // 物体编辑模式，rsu 关联的路口蒙层，应该比roadSign（包括parkingSpace）层级要低
  controlPoint: 7, // 道路参考线控制点
  junctionLink: 7, // 路口中的车道连接线
  controlLine: 6, // 道路参考线曲线
  roadEndEdge: 5, // 路口编辑模式下，辅助交互的道路首尾端面
  laneBoundary: 4, // 车道边界线
  laneMask: 4, // 选中车道后的高亮透明蒙层
  tunnelMask: 3, // 隧道区域的蒙层
  lane: 2, // 车道自身
  junction: 2, // 路口自身网格
  placementArea: 1, // 道路和路口周围的辅助放置区域
}

/**
 * 从数组中找对应的元素数据
 * @param elements
 * @param id
 */
export function getDataById (elements: Array<biz.ICommonElement>, id: string) {
  for (const ele of elements) {
    if (ele.id === id) {
      return ele
    }
  }
  return null
}

/**
 * 判断当前放置的位置，是否在有效的区域范围中
 */
const raycaster = new Raycaster()
export function inValidArea (params: {
  point: common.vec3
  objects: Array<Object3D> // 被检测的物体集合
  camera: PerspectiveCamera | OrthographicCamera
  roadId?: string
  junctionId?: string
  roadAreaName?: Array<string> // 自定义道路检测网格的名称
  junctionAreaName?: Array<string> // 自定义路口检测网格的名称
}) {
  const {
    point,
    roadId = '',
    junctionId = '',
    objects,
    camera,
    roadAreaName = [],
    junctionAreaName = [],
  } = params
  if (!roadId && !junctionId) return false

  // 手动提升起点位置，避免被检测区域过高导致检测结果不准确
  const _point = new Vector3(point.x, point.y + 100, point.z)

  // 由于编辑物体模式下，存在路口sprite精灵文字说明，在射线检测时需要手动设置 camera 参数
  raycaster.camera = camera
  raycaster.set(_point, axisY.clone().negate())

  const intersections: Array<Intersection> = []
  raycaster.intersectObjects(objects, true, intersections)

  if (intersections.length < 1) return false

  // 检测区域的网格名称集合
  const _roadAreaName =
    roadAreaName.length < 1 ? ['lane', 'roadPlacementArea'] : roadAreaName
  const _junctionAreaName =
    junctionAreaName.length < 1 ?
        ['junction', 'junctionPlacementArea'] :
      junctionAreaName

  // 按照 renderOrder 对检测到的元素做排序
  intersections.sort(sortByRenderOrder)

  // 是否有目标道路 id 对应的网格元素(车道+周边的辅助放置区域)
  let hasTarget = false
  // 如果是道路上的杆，跟当前杆所在道路相邻的道路 id
  let neighborRoadId = ''
  // 射线检测有效区域内的交互点位置
  let validPosition: Vector3 | null = null
  intersections.forEach((i) => {
    const { point, object } = i
    const {
      name: objectName,
      roadId: objectRoadId,
      junctionId: objectJunctionId,
    } = object

    if (roadId) {
      // 原本是属于道路的杆，移动范围已经是当前道路和相邻的道路
      if (_roadAreaName.includes(objectName)) {
        if (!hasTarget) {
          if (roadId === objectRoadId) {
            // 检测到目标的道路
            hasTarget = true
            validPosition = point.clone()
          } else {
            // 如果在没有检测到目标道路前，已经检测到其他的道路
            neighborRoadId = objectRoadId
            validPosition = point.clone()
          }
        }
      }
    }

    if (junctionId) {
      // 原本是属于路口的杆，再怎么移动也不能出当前的路口区域
      if (hasTarget) return

      if (_junctionAreaName.includes(objectName)) {
        if (junctionId === objectJunctionId) {
          hasTarget = true
          validPosition = point.clone()
        }
      }
    }
  })

  if (roadId) {
    // 道路中的检测结果
    if (neighborRoadId) {
      // 如果提前检测到了相邻道路的 id，则视为将杆移动到相邻的道路上
      return {
        roadId: neighborRoadId,
        point: validPosition,
      }
    } else {
      if (hasTarget) {
        // 如果检测道路目标的道路，则还是在目标道路上移动
        return {
          roadId,
          point: validPosition,
        }
      } else {
        return null
      }
    }
  } else {
    // 路口中的检测结果
    return hasTarget ? { junctionId, point: validPosition } : null
  }
}

/**
 * 按位比较是否跟目标值相同
 * @param val
 * @param target
 * @returns
 */
function compareBit (val: number, target: number) {
  // 转换成16进制的字符串形式
  let valStr = val.toString(16)
  while (valStr.length < 7) {
    // 补足 0 至七位字符
    valStr = `0${valStr}`
  }
  let targetStr = target.toString(16)
  while (targetStr.length < 7) {
    // 补足 0
    targetStr = `0${targetStr}`
  }
  // 按位对字符进行比较
  for (let i = 0; i < targetStr.length; i++) {
    if (targetStr[i] === '0') continue
    if (targetStr[i] !== valStr[i]) return false
  }

  return true
}

/**
 * 根据车道边界线 mark 的数值，解析出当前车道的基础属性
 * 单线 | 双线
 * 实现 | 虚线 | 虚实结合
 * 颜色
 * 粗细
 * @param val
 */
export function parseLaneBoundaryMark (val: number) {
  let color = DefaultColor.White
  let isBold = false
  const lineStyle: Array<'dash' | 'solid'> = []

  // 形状的判断
  if (compareBit(val, LaneBoundaryMark.SolidSolid)) {
    lineStyle.push('solid', 'solid')
  } else if (compareBit(val, LaneBoundaryMark.BrokenBroken)) {
    lineStyle.push('dash', 'dash')
  } else if (compareBit(val, LaneBoundaryMark.BrokenSolid)) {
    lineStyle.push('dash', 'solid')
  } else if (compareBit(val, LaneBoundaryMark.SolidBroken)) {
    lineStyle.push('solid', 'dash')
  } else if (compareBit(val, LaneBoundaryMark.Broken)) {
    lineStyle.push('dash')
  } else if (compareBit(val, LaneBoundaryMark.Solid)) {
    lineStyle.push('solid')
  }

  // 粗细的判断
  if (compareBit(val, LaneBoundaryMark.Bold)) {
    isBold = true
  }

  // 颜色的判断
  if (
    compareBit(val, LaneBoundaryMark.Yellow) ||
    compareBit(val, LaneBoundaryMark.Yellow2) ||
    compareBit(val, LaneBoundaryMark.YellowYellow)
  ) {
    color = DefaultColor.Yellow
  } else if (compareBit(val, LaneBoundaryMark.Red)) {
    color = DefaultColor.Red
  } else if (compareBit(val, LaneBoundaryMark.Green)) {
    color = DefaultColor.Green
  } else if (compareBit(val, LaneBoundaryMark.Blue)) {
    color = DefaultColor.Blue
  }

  return {
    color,
    isBold,
    lineStyle,
  }
}

/**
 * 根据 section 的长度获取采样间隔
 * 规则：
 * 1、section 长度 < 50，分成 10 段，即 11 -> 21 个采样点
 * 2、50 < section 长度 < 100，分成 20 段，即 21 -> 31 个采样点
 * 3、100 < section 长度 < 300，分成 30 段，即 31 -> 41 个采样点
 * 4、section 长度 > 300，每隔 10 m 进行一次采样
 * @param length
 */
// TODO 采样点数量不够用
export function getSegmentByLength (length: number) {
  if (length <= 50) {
    return 20
  } else if (length <= 100) {
    return 30
  } else if (length <= 300) {
    return 40
  } else {
    const segment = Math.floor(length / 10) + 1
    return segment
  }
}

/**
 * 获取当前 section 匹配的采样间隔
 * @param section
 * @returns
 */
export function getMatchingSegment (section: biz.ISection) {
  const { length } = section
  // 基于 section 的长度来获取当前 section 的采样间隔
  const newSegment = getSegmentByLength(length)
  // 判断原来的 section 采样间隔是多少
  let originSegment
  if (section.lanes[0] && section.lanes[0].samplePoints.length > 0) {
    originSegment = section.lanes[0].samplePoints.length - 1
  }
  // 取采样间隔最大值
  const segment = originSegment ?
    Math.max(newSegment, originSegment) :
    newSegment
  return segment
}

/**
 * 通过车道的左右边界线采样点判断当前车道是否是平滑过渡车道
 * 如果是平滑过渡车道，则给出过渡趋势
 * @param params
 */
export function judgeTrendByBoundarySamplePoints (params: {
  leftPoints: Array<common.vec3>
  rightPoints: Array<common.vec3>
}) {
  const { leftPoints, rightPoints } = params
  const leftLength = leftPoints.length
  const rightLength = rightPoints.length

  // 获取前侧的 2 个点
  const firstLeftPoint = transformVec3(leftPoints[0])
  const firstRightPoint = transformVec3(rightPoints[0])
  const firstDistance = fixedPrecision(
    firstLeftPoint.distanceTo(firstRightPoint),
  )

  // 获取后侧的 2 个点
  const lastLeftPoint = transformVec3(leftPoints[leftLength - 1])
  const lastRightPoint = transformVec3(rightPoints[rightLength - 1])
  const lastDistance = fixedPrecision(lastLeftPoint.distanceTo(lastRightPoint))

  let isExtends
  let normalWidth

  // 对比两个距离的差值，可接收一定的误差范围
  const deviation = 0.5 // 暂定 10 厘米的误差，后续可根据实际情况调整
  const isEqual = compareTwoNumberInDeviation(
    firstDistance,
    lastDistance,
    deviation,
  )

  if (isEqual) {
    // 不存在过渡趋势，则取平均值
    normalWidth = fixedPrecision((firstDistance + lastDistance) / 2)
    return {
      isTransition: false,
      isExtends: undefined,
      normalWidth,
    }
  } else {
    isExtends = lastDistance > firstDistance
    // 存在过渡趋势，取最大的距离值
    normalWidth = Math.max(firstDistance, lastDistance)
    return {
      isTransition: true,
      isExtends,
      normalWidth,
    }
  }
}

/**
 * 获取平滑过渡边界线上的采样点
 * @param params
 * @returns
 */
export function getTransitionBoundarySamplePoints (params: {
  maxOffset: number
  minOffset: number
  pStart: number
  pEnd: number
  exponent?: number
  leftPoint: common.vec3 | Vector3
  rightPoint: common.vec3 | Vector3
  keyPath: biz.ICurve3
  isExtends: boolean
  segment: number
}) {
  const {
    maxOffset,
    minOffset,
    pStart,
    pEnd,
    exponent = Constant.tweenControlPointExponent,
    leftPoint,
    rightPoint,
    keyPath,
    isExtends,
    // 跟另一条车道边界线保持一样的采样间隔
    segment: originSegment = Constant.tweenSampleSegment,
  } = params
  // 平滑过渡补间曲线的控制点间隔，必须为偶数
  const tweenSegment = Constant.tweenControlPointSegment
  // 占 keyPath 的百分比差值
  const percentDelta = (pEnd - pStart) / tweenSegment
  // 偏移量差值
  const offsetDelta = (maxOffset - minOffset) / tweenSegment

  // 五分位点
  const { offsetPoint: middlePoint } = getSamplePointByOffset({
    keyPath,
    percent: (pStart + pEnd) / 2,
    offset: (minOffset + maxOffset) / 2,
  })

  const otherTweenPoints = []
  // 由于五分位点左右两侧的补间过渡形状呈翻转趋势，只需要迭代间隔的一半次数，且忽略首尾和中点
  for (let i = tweenSegment / 2 - 1; i > 0; i--) {
    const _p = (i / tweenSegment) * 2
    const { offsetPoint: p1 } = getSamplePointByOffset({
      keyPath,
      // percent 属于正常的等差值间隔
      percent: pStart + percentDelta * i,
      // offset 根据指数算法（目前发现指数算法比较合理）进行处理
      offset: minOffset + offsetDelta * i * _p ** exponent,
    })
    const { offsetPoint: p2 } = getSamplePointByOffset({
      keyPath,
      percent: pEnd - percentDelta * i,
      offset: maxOffset - offsetDelta * i * _p ** exponent,
    })
    otherTweenPoints.unshift(p1)
    otherTweenPoints.push(p2)
  }

  // 将首、尾、中 3 个关键控制点，跟其余补间控制点组装
  const tweenControlPoints = []
  if (isExtends) {
    tweenControlPoints.push(...otherTweenPoints)
    tweenControlPoints.splice(tweenSegment / 2 - 1, 0, middlePoint)
    tweenControlPoints.unshift(leftPoint)
    tweenControlPoints.push(rightPoint)
  } else {
    tweenControlPoints.push(...otherTweenPoints.reverse())
    tweenControlPoints.splice(tweenSegment / 2 - 1, 0, middlePoint)
    tweenControlPoints.unshift(rightPoint)
    tweenControlPoints.push(leftPoint)
  }
  // 将所有的补间控制点转换成三维的样条曲线
  const transitionPath = new CatmullRomCurve3(
    transformVec3ByObject(tweenControlPoints),
  )
  transitionPath.curveType = 'chordal'

  // 获取平滑过渡补间边界线的采样点
  const transitionPoints = getTweenSamplePointsByKeyPath({
    keyPath,
    pStart,
    pEnd,
    segment: originSegment,
    tweenCurve: transitionPath,
  })

  return {
    transitionPoints,
    controlPoints: tweenControlPoints,
  }
}

/**
 * 通过道路中心线以及起始和截止的百分比，获取该区间中 spaced 采样点向补间曲线投影射线形成的采样点
 * @param params
 */
export function getTweenSamplePointsByKeyPath (params: {
  keyPath: biz.ICurve3
  pStart: number
  pEnd: number
  segment: number
  tweenCurve: biz.ICurve3
}) {
  const { keyPath, pStart, pEnd, segment, tweenCurve } = params
  const deltaPercent = pEnd - pStart
  let space = NP.round(
    deltaPercent / segment,
    Constant.offsetPointsSamplePrecision,
  )
  // space 通过 round 计算可能存在四舍五入的效果，导致进位后最后一次迭代失败，需要最后一位向下取值
  if (space > deltaPercent / segment) {
    space = NP.minus(
      space,
      NP.divide(1, 10 ** Constant.offsetPointsSamplePrecision),
    )
  }
  const refPoints: Array<Vector3> = []
  const verticals: Array<Vector3> = []
  // 先获取道路中心线上的采样点
  for (let i = 0; i <= segment; i++) {
    // 跳过首尾 2 个道路中心线的采样点
    if (i === 0 || i === segment) continue
    const val = NP.plus(pStart, NP.times(i, space))
    const refPoint = keyPath.getPointAt(val)
    refPoints.push(refPoint)
    const tangent = keyPath.getTangentAt(val)
    const vertical = getVerticalVector(tangent)

    verticals.push(vertical)
  }

  // 获取平滑过渡补间曲线的首尾采样点
  const headTweenPoint = tweenCurve.getPointAt(0)
  const tailTweenPoint = tweenCurve.getPointAt(1)

  const tweenSamplePoints: Array<Vector3> = tweenCurve.getSpacedPoints(500)
  // 将平滑过渡的补间曲线，映射到 xz 平面上，即将 y 坐标视为 0
  const tweenSamplePointsInXZPlane = tweenSamplePoints.map(
    p => new Vector3(p.x, 0, p.z),
  )
  // 将道路中心线的 spaced 采样点映射到 xz 平面上
  const refPointsInXZPlane = refPoints.map(p => new Vector3(p.x, 0, p.z))

  // TODO 可能会相对来说比较耗时，后续可以考虑通过 webworker 来优化计算效率？？
  let sampleIndex = 0
  let lastAngle
  // 用于保存平滑过渡补间最佳采样点的索引
  const sampleIndexes = []

  // 嵌套的循环，针对 xz 平面上的每一个道路中心线采样点的映射，在 xz 平面上平滑过渡补间采样点的点集中
  // 获取两点形成的向量，跟车道中心线对应采样点的法向量夹角最小的某一个补间采样点
  for (let i = 0; i < refPointsInXZPlane.length; i++) {
    const refPoint = refPointsInXZPlane[i]
    const vertical = verticals[i]
    for (let j = sampleIndex; j < tweenSamplePointsInXZPlane.length; j++) {
      const samplePoint = tweenSamplePointsInXZPlane[j]
      // 道路中心线采样点至平滑过渡补间采样点形成的向量
      const subRes = samplePoint.clone().sub(refPoint)
      // 取以上的向量跟法向量的夹角
      const angle = Math.abs(subRes.angleTo(vertical))
      if (lastAngle === undefined || angle <= lastAngle) {
        lastAngle = angle
        continue
      } else if (angle > lastAngle) {
        // 如果新的夹角比上一次的夹角要大，则上一次的夹角即为最小的角度，上一个采样点视为最佳采样点
        sampleIndexes.push(j - 1)
        sampleIndex = j + 1
        // 重置用于对比的上一次夹角
        lastAngle = undefined
        // 已找到最佳的采样点就跳过后续的遍历
        break
      }
    }
  }
  const newTweenSamplePoints = sampleIndexes.map((sampleIndex, index) => {
    const p = tweenSamplePoints[sampleIndex]
    // 将 xz 平面的采样点，同步到跟车道中心线采样点对齐的高度
    p.y = refPoints[index].y
    return p
  })
  // 将首尾的补间采样点添加到采样点
  const _newTweenSamplePoints = [
    headTweenPoint,
    ...newTweenSamplePoints,
    tailTweenPoint,
  ]

  return _newTweenSamplePoints
}

/**
 * 由于平滑过渡边界线的采样点是基于道路中心线计算而来的，所以可以直接基于平滑过渡边界线的采样点，计算宽度为常量的外侧车道边界线
 * 如果距离过近，需要使用 keyPath 自身的切线法向量进行计算【fix: 修复第一种方式在两点距离过近时导致的法向量偏差】
 * @param params
 */
export function getRightBoundarySamplePointByLeft (params: {
  keyPath: biz.ICurve3
  keyPathSamplePoints: Array<Vector3 | common.vec3>
  leftBoundarySamplePoints: Array<Vector3 | common.vec3>
  offset: number
  pStart: number
  pEnd: number
  segment: number
}) {
  const {
    keyPathSamplePoints,
    leftBoundarySamplePoints,
    offset,
    keyPath,
    pStart,
    pEnd,
    segment,
  } = params
  const length = leftBoundarySamplePoints.length
  const samplePoints: Array<Vector3> = []
  // TODO 暂定的视为距离过近的最小距离，可根据实际情况做调整
  const minDist = 0.3
  // 法向量的集合
  let isNormalNeedUpdate = false
  const normals = []
  for (let i = 0; i < length; i++) {
    // 道路中心线的采样点
    const basicPoint = keyPathSamplePoints[i]
    // 左边界线采样点
    const leftPoint = leftBoundarySamplePoints[i]
    // 道路中心线采样点的法向量
    const normal = new Vector3(
      leftPoint.x - basicPoint.x,
      leftPoint.y - basicPoint.y,
      leftPoint.z - basicPoint.z,
    )
    // 判断法向量的长度是否过于小（也就是两点距离是否过近）
    const normalLength = normal.length()
    if (!isNormalNeedUpdate && normalLength < minDist) {
      isNormalNeedUpdate = true
    }

    normals.push(normal)
  }

  // 如果道路中心线的采样点跟左边界采样点距离过近，导致手动创建的法向量偏移
  if (isNormalNeedUpdate) {
    // 需要使用 keyPath 计算出来的法向量替换掉
    const keyPathNormals = getPathNormals({
      keyPath,
      pStart,
      pEnd,
      segment,
    })

    normals.forEach((normal: Vector3, index) => {
      if (normal.length() < minDist) {
        // 将距离过近的法向量替换成 keyPath 计算的法向量
        const keyPathNormal = keyPathNormals[index]
        normal.x = keyPathNormal.x
        normal.y = keyPathNormal.y
        normal.z = keyPathNormal.z
      }
    })
  }

  for (let i = 0; i < length; i++) {
    const normal = normals[i]
    const leftPoint = leftBoundarySamplePoints[i]
    normal.normalize()
    const rightPoint = new Vector3(
      leftPoint.x,
      leftPoint.y,
      leftPoint.z,
    ).addScaledVector(normal, offset)

    samplePoints.push(rightPoint)
  }
  return samplePoints
}

// 跟上一个方法接近的反向方法
export function getLeftBoundarySamplePointByRight (params: {
  keyPath: biz.ICurve3
  keyPathSamplePoints: Array<Vector3 | common.vec3>
  rightBoundarySamplePoints: Array<Vector3 | common.vec3>
  offset: number
  pStart: number
  pEnd: number
  segment: number
}) {
  const {
    keyPathSamplePoints,
    rightBoundarySamplePoints,
    offset,
    keyPath,
    pStart,
    pEnd,
    segment,
  } = params
  const length = rightBoundarySamplePoints.length
  const samplePoints: Array<Vector3> = []
  // TODO 暂定的视为距离过近的最小距离，可根据实际情况做调整
  const minDist = 0.3
  // 法向量的集合
  let isNormalNeedUpdate = false
  const normals = []
  for (let i = 0; i < length; i++) {
    // 道路中心线的采样点
    const basicPoint = keyPathSamplePoints[i]
    // 右边界线采样点
    const rightPoint = rightBoundarySamplePoints[i]
    // 道路中心线采样点的法向量
    const normal = new Vector3(
      rightPoint.x - basicPoint.x,
      rightPoint.y - basicPoint.y,
      rightPoint.z - basicPoint.z,
    )
    // 判断法向量的长度是否过于小（也就是两点距离是否过近）
    const normalLength = normal.length()
    if (!isNormalNeedUpdate && normalLength < minDist) {
      isNormalNeedUpdate = true
    }

    normals.push(normal)
  }

  // 如果道路中心线的采样点跟左边界采样点距离过近，导致手动创建的法向量偏移
  if (isNormalNeedUpdate) {
    // 需要使用 keyPath 计算出来的法向量替换掉
    const keyPathNormals = getPathNormals({
      keyPath,
      pStart,
      pEnd,
      segment,
    })

    normals.forEach((normal: Vector3, index) => {
      if (normal.length() < minDist) {
        // 将距离过近的法向量替换成 keyPath 计算的法向量
        const keyPathNormal = keyPathNormals[index]
        normal.x = keyPathNormal.x
        normal.y = keyPathNormal.y
        normal.z = keyPathNormal.z
      }
    })
  }

  for (let i = 0; i < length; i++) {
    const normal = normals[i]
    const rightPoint = rightBoundarySamplePoints[i]
    normal.normalize()
    const leftPoint = new Vector3(
      rightPoint.x,
      rightPoint.y,
      rightPoint.z,
    ).addScaledVector(normal, -offset)

    samplePoints.push(leftPoint)
  }
  return samplePoints
}

/**
 * 获取路径采样点的法向量
 * @param params
 */
export function getPathNormals (params: {
  keyPath: biz.ICurve3
  pStart: number
  pEnd: number
  segment: number
}) {
  const { keyPath, pStart, pEnd, segment } = params
  const deltaPercent = pEnd - pStart
  let space = NP.round(
    deltaPercent / segment,
    Constant.offsetPointsSamplePrecision,
  )
  if (space > deltaPercent / segment) {
    space = NP.minus(
      space,
      NP.divide(1, 10 ** Constant.offsetPointsSamplePrecision),
    )
  }

  const normals: Array<Vector3> = []
  for (let i = 0; i <= segment; i++) {
    const val = NP.plus(pStart, NP.times(i, space))
    const tangent = keyPath.getTangentAt(val)
    const normal = getVerticalVector(tangent)
    normals.push(normal)
  }
  return normals
}

/**
 * 获取跟曲线采样点平行的偏移采样点
 * // TODO 如果后续涉及路面有倾角，需要对算法进行调整
 * @param params
 */
export function getParallelCurveSamplePoints (params: {
  keyPath: biz.ICurve3
  elevationPath?: biz.ICurve3
  offset: number
  segment: number
  pStart?: number
  pEnd?: number
  // side=0 正向车道，side=1 反向车道
  side?: number
}) {
  const {
    keyPath,
    elevationPath,
    offset,
    pStart = 0,
    pEnd = 1,
    segment,
    side = 0,
  } = params
  const deltaPercent = pEnd - pStart
  let space = NP.round(
    deltaPercent / segment,
    Constant.offsetPointsSamplePrecision,
  )
  // space 通过 round 计算可能存在四舍五入的效果，导致进位后最后一次迭代失败，需要最后一位向下取值
  if (space > deltaPercent / segment) {
    space = NP.minus(
      space,
      NP.divide(1, 10 ** Constant.offsetPointsSamplePrecision),
    )
  }
  const refPoints: Array<Vector3> = []
  const offsetPoints: Array<Vector3> = []
  for (let i = 0; i <= segment; i++) {
    // 最大不超过 1
    const val = Math.min(NP.plus(pStart, NP.times(i, space)), 1)
    const refPoint = keyPath.getPointAt(val)
    const tangent = keyPath.getTangentAt(val)
    // 如果高程的曲线存在
    if (elevationPath) {
      const elevationPoint = elevationPath.getPointAt(val)
      // 使用高程曲线的高度
      refPoint.y = elevationPoint.y
    }
    const vertical = getVerticalVector(tangent)
    const offsetPoint = refPoint.clone()
    // 如果是反向车道，则基于道路参考线向另一个方向延伸
    offsetPoint.addScaledVector(vertical, side === 0 ? offset : -offset)
    refPoints.push(refPoint)
    // 都是同一个方向的采样点序列
    offsetPoints.push(offsetPoint)
  }
  return {
    refPoints,
    offsetPoints,
  }
}

/**
 * 获取路口几何体属性
 * @param payload
 */
export function getJunctionGeoAttr (payload: {
  refBoundary: Array<biz.IRefRoad>
  enableClosePoint: boolean
  isMultipleRoad: boolean
}) {
  const { refBoundary, enableClosePoint, isMultipleRoad } = payload
  const sortedRefBoundary = sortJunctionRefBoundary(refBoundary)
  // 获取当前交叉路口的闭合区域边界线的采样点
  const samplePoints = getBezierEdgeSamplePoints(
    sortedRefBoundary,
    enableClosePoint,
  )
  if (isMultipleRoad) {
    const attrRes = getJunctionGeoByMultipleRoad(samplePoints)
    return attrRes
  } else {
    const attrRes = geJunctionGeoByTwoRoad(samplePoints)
    return attrRes
  }
}

/**
 * 获取由 2 条道路连接成的路口几何体属性
 */
export function geJunctionGeoByTwoRoad (
  samplePoints: Array<Array<common.vec3>>,
) {
  const vertices: Array<number> = []
  const indices: Array<number> = []
  const [firstPoints, secondPoints] = samplePoints
  firstPoints.forEach((p) => {
    vertices.push(p.x, p.y, p.z)
  })
  secondPoints.forEach((p) => {
    vertices.push(p.x, p.y, p.z)
  })
  const totalLength = firstPoints.length + secondPoints.length
  let _index = 0
  while (_index < Constant.junctionEdgeSegment) {
    // face1
    indices.push(_index)
    indices.push(totalLength - _index - 1)
    indices.push(totalLength - _index - 2)

    // face2
    indices.push(_index)
    indices.push(totalLength - _index - 2)
    indices.push(_index + 1)

    _index++
  }

  const _vertices = new Float32Array(vertices)
  const _indices = new Float32Array(indices)

  return {
    vertices: _vertices,
    indices: _indices,
  }
}
/**
 * 获取由 2 条以上道路连接成的路口几何体属性
 */
export function getJunctionGeoByMultipleRoad (
  samplePoints: Array<Array<common.vec3>>,
) {
  const junctionCenter = new Vector3(0, 0, 0)
  let vertexIndex = 0

  // 保存交叉路口中点的顶点坐标和三角面
  const vertices = []
  const indices = []

  const samplePointIndex: Array<number> = []
  samplePoints.forEach((points) => {
    // 保存所有贝塞尔边界采样点的顶点坐标
    points.forEach((p) => {
      vertices.push(p.x, p.y, p.z)
    })
    // 记录每一组贝塞尔边界采样点的起始索引和总数
    samplePointIndex.push(vertexIndex)
    vertexIndex += points.length

    // 用贝塞尔曲线采样点的二分点坐标计算交叉路口中心坐标
    const center = points[(points.length - 1) / 2]
    junctionCenter.add(new Vector3(center.x, center.y, center.z))
  })
  // 保存交叉路口中心顶点坐标
  junctionCenter.divideScalar(samplePoints.length)
  vertices.push(junctionCenter.x, junctionCenter.y, junctionCenter.z)

  // const _uvs = [] // 交叉路口的 uv 不知道怎么定义了，低优先级
  for (let i = 0; i < samplePointIndex.length; i++) {
    const currentIndex = samplePointIndex[i]
    const nextIndex = samplePointIndex[(i + 1) % samplePointIndex.length]

    const halfLength = Constant.junctionEdgeSegment / 2
    // 由于交叉路口边界划分为偶数段，即存在奇数个采样点点，中点索引可以直接使用
    const currentMiddleIndex = halfLength + currentIndex
    const nextMiddleIndex = halfLength + nextIndex

    let _index = 0
    while (_index < halfLength) {
      if (_index === 0) {
        // face0：跟交叉路口中点形成的三角面
        indices.push(vertexIndex)
        indices.push(nextMiddleIndex - _index)
        indices.push(currentMiddleIndex + _index)
      }
      // face1
      indices.push(currentMiddleIndex + _index)
      indices.push(nextMiddleIndex - _index)
      indices.push(currentMiddleIndex + _index + 1)

      // face2
      indices.push(nextMiddleIndex - _index)
      indices.push(nextMiddleIndex - _index - 1)
      indices.push(currentMiddleIndex + _index + 1)

      _index++
    }
  }

  const _vertices = new Float32Array(vertices)
  const _indices = new Float32Array(indices)

  return {
    vertices: _vertices,
    indices: _indices,
  }
}

/**
 * 生成道路1和道路2之间端点组成的交叉路口某一条边界线的采样点
 * @param refBoundary
 * @returns
 */
export function getBezierEdgeSamplePoints (
  refBoundary: Array<biz.IRefRoad>,
  enableClosePoint: boolean, // 是否允许相近的端点也作为路口边界的关键点
) {
  const samplePoints = []
  // 遍历交叉路口关联的边界道路
  for (let i = 0; i < refBoundary.length; i++) {
    const currentBoundary = refBoundary[i]
    // 如果当前边界道路已经是最后一个，则 next 就是第一个边界道路
    const nextBoundary = refBoundary[(i + 1) % refBoundary.length]
    const {
      isTail: currentIsTail,
      alongVec: currentAlongVec,
      leftPoint: currentLeftPoint,
      rightPoint: currentRightPoint,
    } = currentBoundary
    const {
      isTail: nextIsTail,
      alongVec: nextAlongVec,
      leftPoint: nextLeftPoint,
      rightPoint: nextRightPoint,
    } = nextBoundary

    // 需要根据当前道路和下一个道路的驶入驶出关系来控制怎么连接交叉路口的边界线
    // 受 refBoundary 排序的顺逆时针影响
    let firstPoint, secondPoint
    if (currentIsTail && nextIsTail) {
      firstPoint = currentLeftPoint
      secondPoint = nextRightPoint
    } else if (!currentIsTail && !nextIsTail) {
      firstPoint = currentRightPoint
      secondPoint = nextLeftPoint
    } else if (currentIsTail && !nextIsTail) {
      firstPoint = currentLeftPoint
      secondPoint = nextLeftPoint
    } else if (!currentIsTail && nextIsTail) {
      firstPoint = currentRightPoint
      secondPoint = nextRightPoint
    }

    if (!firstPoint || !secondPoint) continue

    // 如果两条道路待形成交叉路口边界线的端点相同，则忽略该边界采样点的计算
    // 可接受毫米级的误差
    if (
      !enableClosePoint &&
      compareTwoVector3InDeviation(
        firstPoint,
        secondPoint,
        Constant.mmDeviation,
      )
    ) {
      continue
    }

    // 通过两个端点和两个控制点生成一条贝塞尔曲线
    const { curve: bezierCurve } = getBezier3WithDirection({
      point1: firstPoint,
      direction1: currentAlongVec,
      direction2: nextAlongVec,
      point2: secondPoint,
    })

    // 对贝塞尔曲线进行采样
    const points = bezierCurve.getSpacedPoints(Constant.junctionEdgeSegment)
    samplePoints.push(points)
  }
  return samplePoints
}

/**
 * 对交叉路口的边界参考点进行顺（逆）时针的排序
 * @param refBoundary
 */
export function sortJunctionRefBoundary (refBoundary: Array<biz.IRefRoad>) {
  const pMap = new Map()
  // 从顶视角来看，连接道路端点的二维中点集合
  const edgeCenter: Array<Vector2> = []
  refBoundary.forEach((boundary) => {
    const { leftPoint, rightPoint } = boundary
    if (!leftPoint || !rightPoint) return
    // 道路边界线的二维中点
    const _center = new Vector2(
      (leftPoint.x + rightPoint.x) / 2,
      (leftPoint.z + rightPoint.z) / 2,
    )
    // 通过 uuid 作为 key 对关联的数据进行保存
    const uuid = genUuid()
    // 将 uuid 挂载到三维矢量的对象上
    _center.uuid = uuid

    pMap.set(uuid, boundary)
    // 形成的闭合区域中心点需要先递增加上所有边缘点
    edgeCenter.push(_center)
  })

  /**
   * 将所有的道路端点的中心点，跟对应沿道路方向的矢量，两两创建一条二维平面的贝塞尔曲线
   * 基于贝塞尔曲线的中点，计算的交叉路口中心点可能更接近实际的效果
   * 保证交叉路口绘制闭合区域绘制的稳定性和准确性
   */
  const bezierCenter = new Vector2(0, 0)
  // 平面贝塞尔曲线的数量
  let count = 0
  for (let i = 0; i < edgeCenter.length; i++) {
    // 获取第一个中心点的平面坐标和沿道路方向的平面方向
    const center1 = edgeCenter[i]
    const refBoundary1 = pMap.get(center1.uuid)
    const direction1 = new Vector2(
      refBoundary1.alongVec.x,
      refBoundary1.alongVec.z,
    )
    for (let j = i + 1; j < edgeCenter.length; j++) {
      // 获取第二个中心点的平面坐标和沿道路方向的平面方向
      const center2 = edgeCenter[j]
      const refBoundary2 = pMap.get(center2.uuid)
      const direction2 = new Vector2(
        refBoundary2.alongVec.x,
        refBoundary2.alongVec.z,
      )

      // 计算平面的贝塞尔曲线
      const bezierCurve = getBezier2WithDirection({
        point1: center1,
        direction1,
        direction2,
        point2: center2,
      })
      // 保存贝塞尔曲线的中点
      const _bezierCenter = bezierCurve.getPointAt(0.5)
      count++
      bezierCenter.add(_bezierCenter)
    }
  }
  // 所有贝塞尔曲线的中点的总和，除以对应的总数，得到这一系列贝塞尔曲线中点的重心
  bezierCenter.divideScalar(count)

  // 所有道路端点的中心点绝对位置，转换成相对于贝塞尔重心点的相对位置
  edgeCenter.forEach((p) => {
    p.sub(bezierCenter)
  })

  // 按照跟平面坐标系中 x 正轴形成的夹角大小进行排序，该 sort 算法形成的点为顺时针
  edgeCenter.sort((p1, p2) => p1.angle() - p2.angle())

  // 排序后的连接道路数据
  const sortedRefBoundary = edgeCenter.map(p => pMap.get(p.uuid))

  // 手动清除缓存
  pMap.clear()

  return sortedRefBoundary
}

/**
 * 将更新车道几何体属性的方法封装成 Promise
 * @param params
 */
export function getLaneGeoAttrPromise (params: {
  roadId: string
  sectionId: string
  laneId: string
  points1: Array<common.vec3>
  points2: Array<common.vec3>
}) {
  const { roadId, sectionId, laneId, points1, points2 } = params
  return new Promise((resolve) => {
    const { vertices, indices, uvs } = getGeometryAttrBySamplePoints(
      points1,
      points2,
    )
    resolve({
      roadId,
      sectionId,
      laneId,
      vertices,
      indices,
      uvs,
    })
  })
}
/**
 * 将更新实线车道边界线的几何体属性方法封装成 Promise
 * @param params
 */
export function getSolidBoundaryGeoAttrPromise (params: {
  width: number
  offset: number
  isSingle: boolean
  points: Array<common.vec3> | ArrayBuffer
  isFirst: boolean
  roadId: string
  sectionId: string
  boundaryId: string
}) {
  const {
    width,
    offset,
    isSingle,
    points,
    isFirst,
    roadId,
    sectionId,
    boundaryId,
  } = params
  return new Promise((resolve) => {
    const { vertices, indices } = handleSolidBoundaryData({
      width,
      offset,
      isSingle,
      points,
    })
    resolve({
      vertices,
      indices,
      isFirst,
      roadId,
      sectionId,
      boundaryId,
    })
  })
}
/**
 * 将更新虚线车道边界线的几何体属性方法封装成 Promise
 * @param params
 */
export function getDashedBoundaryGeoAttrPromise (params: {
  width: number
  offset: number
  isSingle: boolean
  points: Array<common.vec3> | ArrayBuffer
  unitLength: number
  gapLength: number
  isFirst: boolean
  roadId: string
  sectionId: string
  boundaryId: string
}) {
  const {
    width,
    isSingle,
    offset,
    points,
    unitLength,
    gapLength,
    isFirst,
    roadId,
    sectionId,
    boundaryId,
  } = params
  return new Promise((resolve) => {
    const { vertices, indices } = handleDashedBoundaryData({
      width,
      offset,
      isSingle,
      points,
      unitLength,
      gapLength,
    })
    resolve({
      vertices,
      indices,
      isFirst,
      roadId,
      sectionId,
      boundaryId,
    })
  })
}

// 处理实线的边界线属性
export function handleSolidBoundaryData (payload: {
  width: number
  offset: number
  isSingle: boolean
  points: Array<common.vec3> | ArrayBuffer
}) {
  const { width, offset, isSingle, points } = payload
  let vec3Points
  if (points instanceof ArrayBuffer) {
    const _points = Array.prototype.slice.call(new Float32Array(points))
    vec3Points = transformVec3ByFlatArray(_points)
  } else {
    vec3Points = transformVec3ByObject(points)
  }
  const boundaryCurve = new CatmullRomCurve3(vec3Points)
  boundaryCurve.curveType = CurveType
  if (CurveType === 'catmullrom') {
    boundaryCurve.tension = CatmullromTension
  }
  const boundarySegment = vec3Points.length - 1
  const { vertices, indices } = calcSolidBoundaryGeoAttr({
    width,
    isSingle,
    keyPath: boundaryCurve,
    offset,
    segment: boundarySegment,
  })
  const _vertices = new Float32Array(vertices)
  // 组装回传的数据
  return {
    vertices: _vertices,
    indices,
  }
}

// 处理虚线的边界线属性
export function handleDashedBoundaryData (payload: {
  width: number
  offset: number
  isSingle: boolean
  points: Array<common.vec3> | ArrayBuffer
  unitLength: number
  gapLength: number
}) {
  const { width, isSingle, offset, points, unitLength, gapLength } = payload
  let vec3Points
  if (points instanceof ArrayBuffer) {
    const _points = Array.prototype.slice.call(new Float32Array(points))
    vec3Points = transformVec3ByFlatArray(_points)
  } else {
    vec3Points = transformVec3ByObject(points)
  }
  const boundaryCurve = new CatmullRomCurve3(vec3Points)
  boundaryCurve.curveType = CurveType
  if (CurveType === 'catmullrom') {
    boundaryCurve.tension = CatmullromTension
  }
  const { vertices, indices } = calcDashedBoundaryGeoAttr({
    unitLength,
    gapLength,
    keyPath: boundaryCurve,
    width,
    isSingle,
    offset,
  })
  const _vertices = new Float32Array(vertices)
  // 组装回传的数据
  return {
    vertices: _vertices,
    indices,
  }
}

/**
 * 计算实线边界线的几何体属性
 * @param params
 */
export function calcSolidBoundaryGeoAttr (params: {
  width: number
  keyPath: biz.ICurve3
  offset: number
  isSingle: boolean
  segment: number
}) {
  const { width, keyPath, offset = 0.1, isSingle = true, segment } = params
  let space = Number((1 / segment).toFixed(8))
  if (space > 1 / segment) {
    space = space - 1 / 10 ** 8
  }
  const innerPoints = []
  const outerPoints = []
  for (let i = 0; i <= segment; i++) {
    const percent = i * space
    const refPoint = keyPath.getPointAt(percent)

    const tangent = keyPath.getTangentAt(percent)
    const vertical = getVerticalVector(tangent)
    if (isSingle) {
      // 如果是单条线，则忽略 offset 字段
      const innerPoint = refPoint.clone()
      innerPoint.addScaledVector(vertical, -(width / 2))
      innerPoints.push(innerPoint)

      const outerPoint = refPoint.clone()
      outerPoint.addScaledVector(vertical, width / 2)
      outerPoints.push(outerPoint)
    } else {
      // 双线，则需要利用 offset 做偏移处理
      const innerPoint = refPoint.clone()
      innerPoint.addScaledVector(vertical, offset)
      innerPoints.push(innerPoint)

      const outerPoint = refPoint.clone()
      const _offset = offset > 0 ? offset + width : offset - width
      outerPoint.addScaledVector(vertical, _offset)
      outerPoints.push(outerPoint)
    }
  }

  let vertices, indices
  if (offset > 0) {
    const geoAttr = getGeometryAttrBySamplePoints(innerPoints, outerPoints)
    vertices = geoAttr.vertices
    indices = geoAttr.indices
  } else {
    // 如果偏移量为负，需要调整创建几何体传入的数组顺序，保证生成的网格正面朝上
    const geoAttr = getGeometryAttrBySamplePoints(outerPoints, innerPoints)
    vertices = geoAttr.vertices
    indices = geoAttr.indices
  }

  return {
    vertices,
    indices,
  }
}

/**
 * 计算虚线边界线的几何体属性
 * @param params
 */
function calcDashedBoundaryGeoAttr (params: {
  unitLength: number
  gapLength: number
  width: number
  keyPath: biz.ICurve3
  isSingle: boolean
  offset: number
}) {
  const {
    unitLength,
    gapLength,
    width,
    keyPath,
    isSingle = false,
    offset = 0.1,
  } = params
  const curveLength = fixedPrecision(keyPath.getLength(), 0)
  // current percent
  let cp = 0
  // 每一组虚实间隔的局部临时计数
  let count = 0
  // 保留 4 位小数
  const unitSpace = fixedPrecision(unitLength / curveLength, 4)
  const gapSpace = fixedPrecision(gapLength / curveLength, 4)
  // 每一个虚线实体的单位长度取采样点的段数
  const unitSegment = Math.ceil(unitLength)
  const unitDeltaSpace = unitSpace / unitSegment

  const innerPoints = []
  const outerPoints = []
  // 每一段虚线实体的起始索引
  const unitStartIndex = []
  while (cp <= 1) {
    if (count === 0) {
      // 当前采样点数组的长度，等于即将插入的采样点在数组中的索引
      const index = innerPoints.length
      unitStartIndex.push(index)
    }

    // 参考线上的采样点
    const refPoint = keyPath.getPointAt(cp)

    // 获取采样点的切线向量
    const tangent = keyPath.getTangentAt(cp)
    // 计算切线向量在 zx 平面上的垂直向量
    const vertical = getVerticalVector(tangent)

    if (isSingle) {
      // 如果是单条线，则忽略 offset 字段
      const innerPoint = refPoint.clone()
      innerPoint.addScaledVector(vertical, -(width / 2))
      innerPoints.push(innerPoint)

      const outerPoint = refPoint.clone()
      outerPoint.addScaledVector(vertical, width / 2)
      outerPoints.push(outerPoint)
    } else {
      // 双线，则需要利用 offset 做偏移处理
      const innerPoint = refPoint.clone()
      innerPoint.addScaledVector(vertical, offset)
      innerPoints.push(innerPoint)

      const outerPoint = refPoint.clone()
      const _offset = offset > 0 ? offset + width : offset - width
      outerPoint.addScaledVector(vertical, _offset)
      outerPoints.push(outerPoint)
    }

    count++
    if (count > unitSegment) {
      cp += gapSpace
      count = 0
      continue
    } else {
      cp += unitDeltaSpace
    }
  }

  const vertices = []
  const indices = []
  for (let i = 0; i < innerPoints.length; i++) {
    const p1 = innerPoints[i]
    const p2 = outerPoints[i]

    if (offset > 0) {
      vertices.push(p1.x, p1.y, p1.z)
      vertices.push(p2.x, p2.y, p2.z)
    } else {
      // 如果偏移量为负，则需要调整组合几何体的顶点顺序，保证网格正面朝上
      vertices.push(p2.x, p2.y, p2.z)
      vertices.push(p1.x, p1.y, p1.z)
    }

    // 在采样点遍历结束前
    if (i < innerPoints.length - 1) {
      // 前一组虚线实体部分末尾采样点，与下一组虚线实体部分起始采样点，不形成三角面
      // 从而实现虚线的间隔部分
      if (unitStartIndex.includes(i + 1)) {
        continue
      }
      const a = i * 2
      const b = i * 2 + 1
      const c = (i + 1) * 2
      const d = (i + 1) * 2 + 1
      indices.push(a, b, d)
      indices.push(a, d, c)
    }
  }

  return {
    vertices,
    indices,
  }
}

/**
 * 通过两条边界的采样点，获取几何体属性
 * @param points1
 * @param points2
 * @returns
 */
export function getGeometryAttrBySamplePoints (
  points1: Array<common.vec3>,
  points2: Array<common.vec3>,
) {
  // 由于第三方地图部分车道边界线存在左右边界采样点数量不一致的情况，取车道数量的小值
  const minLength = Math.min(points1.length, points2.length)
  const vertices = []
  const indices = []
  const uvs = []
  const deltaU = 1 / (minLength - 1)
  const deltaV = 1

  /**
   * a ---- b
   * |      |
   * c ---- d
   * 车道几何体，由车道两侧边界线的相同数量的采样点组成
   * 每四个点可以组成一对三角面，按照三角面的顶点顺时针来排列组合，三角面的顶点为 abd 和 adc
   * 为了保证后续纹理能够应用到车道的网格中，需要手动指定每个顶点的 uv 坐标
   */
  // 添加顶点
  for (let i = 0; i < minLength; i++) {
    const p1 = points1[i]
    const p2 = points2[i]
    const u = deltaU * i
    const v1 = 0
    const v2 = deltaV
    // 顶点坐标
    vertices.push(p1.x, p1.y, p1.z)
    vertices.push(p2.x, p2.y, p2.z)
    // 每个顶点对应的 uv 坐标
    uvs.push(u, v1)
    uvs.push(u, v2)

    if (i < minLength - 1) {
      // 在多个三角形中重复使用顶点，即为索引三角形
      const a = i * 2
      const b = i * 2 + 1
      const c = (i + 1) * 2
      const d = (i + 1) * 2 + 1
      indices.push(a, b, d) // face1
      indices.push(a, d, c) // face2
    }
  }

  const _vertices = new Float32Array(vertices)
  const _uvs = new Float32Array(uvs)

  return {
    vertices: _vertices,
    uvs: _uvs,
    indices,
  }
}

interface ISTLocation {
  laneLinkId: string
  s: number
  t: number
  percent: number
  tangent: Vector3
  roadid: string
  closestPointOnRefLine: Vector3
}

// 获取路口中的绝对坐标，在所有 laneLink 中最优的参考线 st 局部坐标
export function getLocationInJunction (params: {
  laneLinks: Array<biz.ILaneLink>
  point: common.vec3
  originLaneLinkId?: string // 是否有指定的 laneLinkId
}) {
  const { laneLinks, point, originLaneLinkId = '' } = params
  const _point = new Vector3(point.x, point.y, point.z)

  // 要返回的 st 坐标内容
  let option: ISTLocation | null = null
  let originLaneLinkOption: ISTLocation | null = null

  for (const link of laneLinks) {
    const { samplePoints, id, roadid } = link
    // 转换成 Vector3 类型
    const _samplePoints = transformVec3ByObject(samplePoints)
    // 基于 laneLink 的采样点，创建样条曲线
    const linkCurve = new CatmullRomCurve3(_samplePoints)
    linkCurve.curveType = CurveType
    if (CurveType === 'catmullrom') {
      linkCurve.tension = CatmullromTension
    }

    // laneLink 曲线的长度
    const linkLength = linkCurve.getLength()

    // 计算绝对坐标到 laneLink 参考线上最近的交点
    const {
      point: closestPoint,
      tangent,
      percent,
    } = getClosestPointFromCurvePath({
      curvePath: linkCurve,
      point: _point,
    })

    // s 坐标
    const s = fixedPrecision(linkLength * percent)
    // t 坐标
    const t = getTValue({
      tangent,
      refLinePoint: closestPoint,
      targetPoint: _point,
    })

    // 当前参考线坐标系下的局部位置数据
    const currentOption = {
      laneLinkId: id,
      s,
      t,
      percent,
      tangent,
      roadid,
      closestPointOnRefLine: closestPoint,
    }

    // 记录指定的 laneLinkId 对应的 st 参考线数据
    if (originLaneLinkId === id) {
      originLaneLinkOption = currentOption
    }

    if (!option) {
      // 第一次遍历的时候直接赋值
      option = currentOption
      continue
    } else {
      // 跟后续 laneLink 计算出来的参考线坐标系下的数据作比较，取更合理的
      if (currentOption.percent > 0 && currentOption.percent < 1) {
        if (option.percent === 0 || option.percent === 1) {
          // 如果上一组保存的 st 数据，处于边缘的情况，则用当前的覆盖
          option = currentOption
          continue
        } else {
          // 如果上一组的数据也是有效的，取最近的
          if (Math.abs(currentOption.t) < Math.abs(option.t)) {
            option = currentOption
            continue
          }
        }
      } else {
        // 当前组的数据处于边缘
        if (option.percent > 0 && option.percent < 1) {
          // 如果上一组数据有效，则直接跳过
          continue
        } else {
          // 上一组和当前组的数据都处于边缘，取离得最近得
          if (Math.abs(currentOption.t) < Math.abs(option.t)) {
            option = currentOption
            continue
          }
        }
      }
    }
  }

  // 如果指定的 laneLink 参考线坐标存在
  if (originLaneLinkOption && option) {
    // 且指定的数据处于合理范围中，则沿用指定的
    if (originLaneLinkOption.percent > 0 && originLaneLinkOption.percent < 1) {
      option = originLaneLinkOption
    } else {
      // 如果指定的 laneLink 数据处于边缘
      if (option.percent === 0 || option.percent === 1) {
        // 计算出的数据也处于边缘，取最近的
        if (Math.abs(originLaneLinkOption.t) < Math.abs(option.t)) {
          option = originLaneLinkOption
        }
      }
    }
  }

  return option
}

// 计算停车位四个角的描边几何体顶点属性
export function getCornerVerticesAttr (params: {
  curvePathPoints: Array<common.vec3>
  elevationPathPoints?: Array<common.vec3>
  refLineLocation: biz.IRefLineLocation
  count: number // 重复的次数
  width: number // 停车位宽度
  length: number // 停车位长度
  margin: number // 相邻停车位几何中心点间距
  lineWidth: number // 线宽
  yaw: number // 停车位跟参考线的偏航角（角度制）
  innerAngle: number // 停车位内部的夹角（角度制），暂时理解成左下的夹角，在 [45, 135] 度范围中
  isDashed?: boolean // 是否是虚线
}) {
  // 基于 st 参考系来算，最后转换成世界坐标系下的坐标
  // 【注意】停车位中心点不会超出 [s, roadLength] 范围，但四个角的坐标可能会超出范围
  // 在超出范围时，需要将参考线首或尾沿切线方向延伸来算区域外的 st 坐标，以及世界坐标系坐标
  const {
    curvePathPoints,
    elevationPathPoints,
    refLineLocation,
    count,
    width,
    length,
    margin,
    yaw,
    innerAngle,
    lineWidth,
    isDashed = false,
  } = params
  const { s, t } = refLineLocation
  // 通过道路参考线，以及停车位的中心点坐标、尺寸，计算停车位4个角的顶点
  const curvePath = new CatmullRomCurve3(transformVec3ByObject(curvePathPoints))
  curvePath.curveType = CurveType
  if (CurveType === 'catmullrom') {
    curvePath.tension = CatmullromTension
  }
  // 高程的曲线
  let elevationPath: biz.ICurve3
  if (elevationPathPoints) {
    elevationPath = new CatmullRomCurve3(
      transformVec3ByObject(elevationPathPoints),
    )
    elevationPath.curveType = CurveType
    if (CurveType === 'catmullrom') {
      elevationPath.tension = CatmullromTension
    }
  }

  // 将角度制转换成弧度制
  const _innerAngle = MathUtils.degToRad(innerAngle)
  const _yaw = MathUtils.degToRad(yaw)
  // 由于停车位内部存在夹角，停车位的形状可能变成平行四边形【矩形是特殊的平行四边形】

  const _width = width * Math.sin(_innerAngle)
  const _shortLength = (length - width * Math.cos(_innerAngle)) / 2
  const _longLength = _shortLength + width * Math.cos(_innerAngle)

  // 基于第一个停车位几何中心点的 s 坐标作为基准
  let startS = s
  const geoAttrs: Array<biz.IGeoAttr> = []
  const cornerPoints: Array<Array<common.vec3>> = []
  const sCoordinates: Array<number> = []
  // st 坐标系下的角点
  const cornerSTPoints: Array<Array<common.vec2>> = []
  for (let i = 0; i < count; i++) {
    const topLeft = new Vector2(startS - _shortLength, t + _width / 2)
    const topRight = new Vector2(startS + _longLength, t + _width / 2)
    const bottomRight = new Vector2(startS + _shortLength, t - _width / 2)
    const bottomLeft = new Vector2(startS - _longLength, t - _width / 2)

    // 在 st 坐标系下，绕当前停车位几何中心，旋转 yaw 偏航角
    const center = new Vector2(startS, t)
    topLeft.rotateAround(center, _yaw)
    topRight.rotateAround(center, _yaw)
    bottomRight.rotateAround(center, _yaw)
    bottomLeft.rotateAround(center, _yaw)

    sCoordinates.push(startS)

    // 按照停车位的间距，调整下一个停车位
    startS += margin

    // st 坐标系下的角点【顺时针】
    const cornerSTPoint = [topLeft, topRight, bottomRight, bottomLeft]
    // 将 st 坐标转换成世界坐标系下的坐标
    const cornerPoint = cornerSTPoint.map((p) => {
      const locationRes = getLocationByST({
        curvePath,
        elevationPath,
        s: p.x,
        t: p.y,
        // 通过四个角的st坐标，计算世界坐标系下的实际坐标，可以超出参考线范围
        crossBorder: true,
      })
      return locationRes.targetPoint
    })

    let geoAttr
    if (isDashed) {
      // 如果是虚线的停车位
      geoAttr = getDashGeoAttrByCornerPoints({
        points: cornerPoint,
        lineWidth,
      })
    } else {
      // 通用的实线停车位
      geoAttr = getGeoAttrByCornerPoints({
        points: cornerPoint,
        lineWidth,
      })
    }

    geoAttrs.push(geoAttr)
    cornerPoints.push(cornerPoint)
    cornerSTPoints.push(cornerSTPoint)
  }

  // 返回一个停车位的几何体属性和四个角的坐标
  return {
    geoAttrs,
    cornerSTPoints,
    cornerPoints,
    sCoordinates,
  }
}

// 基于单个停车位的四个角坐标，计算对应的几何体属性
function getGeoAttrByCornerPoints (params: {
  points: Array<common.vec3> // points 的数量必须是 4 个！
  lineWidth: number
}) {
  const { points, lineWidth } = params
  const cornerCount = 4

  // 计算停车位的中心点
  const center = new Vector3()
  const _points = transformVec3ByObject(points)
  _points.forEach((p) => {
    center.add(new Vector3(p.x, p.y, p.z))
  })
  center.divideScalar(cornerCount)

  // 基于四个角的顶点坐标，计算带有宽度的描边定位顶点坐标
  const outlinePoints = []
  for (let i = 0; i < cornerCount; i++) {
    // 每一个基准点，都会基于前、后的基准点共同计算三角面的顶点
    const prePoint = _points[(i - 1 + cornerCount) % cornerCount]
    const curPoint = _points[i]
    const nextPoint = _points[(i + 1) % cornerCount]

    // 两条边的方向
    const preDirection = prePoint.clone().sub(curPoint).normalize()
    const nextDirection = nextPoint.clone().sub(curPoint).normalize()

    // 两条边形成夹角
    const angle = preDirection.angleTo(nextDirection) / 2
    const cornerLength = lineWidth / Math.sin(angle)

    // 两条边的角平分线方向
    const cornerDirection = preDirection.clone().applyAxisAngle(axisY, angle)

    // 一个角对应的内、外两个定位点
    const innerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, cornerLength / 2)
    const outerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, -cornerLength / 2)
    outlinePoints.push(innerPoint, outerPoint)
  }

  // 将停车位四个角内侧和外侧的顶点，按照三角面的组装顺序，拼接顶点
  const vertices = []
  const indices = []
  const vLength = outlinePoints.length
  for (let i = 0; i < vLength; i += 2) {
    // 顶点的索引
    const index1 = i
    const index2 = (i + 1 + vLength) % vLength
    const index3 = (i + 2 + vLength) % vLength
    const index4 = (i + 3 + vLength) % vLength

    const innerPoint = outlinePoints[index1]
    const outerPoint = outlinePoints[index2]

    // 顶点的坐标（顶点只添加一遍）
    vertices.push(innerPoint.x, innerPoint.y, innerPoint.z)
    vertices.push(outerPoint.x, outerPoint.y, outerPoint.z)

    // 通过定义三角面的顶点索引，重复利用顶点，即索引三角面
    indices.push(index2, index1, index3) // face1
    indices.push(index3, index4, index2) // face2
  }

  const _vertices = new Float32Array(vertices)

  return {
    vertices: _vertices,
    indices,
  }
}

// 基于单个停车位的四个角坐标，计算虚线停车位对应的几何体属性
function getDashGeoAttrByCornerPoints (params: {
  points: Array<common.vec3> // points 的数量必须是 4 个！
  lineWidth: number
}) {
  const { points, lineWidth } = params
  const cornerCount = 4

  // 计算停车位的中心点
  const center = new Vector3()
  const _points = transformVec3ByObject(points)
  _points.forEach((p) => {
    center.add(new Vector3(p.x, p.y, p.z))
  })
  center.divideScalar(cornerCount)

  // 基于四个角的顶点坐标，计算带有宽度的描边定位顶点坐标
  const outlinePoints: Array<Array<Vector3>> = [[], [], [], []]

  // 虚线间隔0.6米
  const span = 0.6
  // 三角面顶点索引的基准值
  const vertexBasicIndex: Array<number> = []
  for (let i = 0; i < cornerCount; i++) {
    // 每一个基准点，都会基于前、后的基准点共同计算三角面的顶点
    const prePoint = _points[(i - 1 + cornerCount) % cornerCount]
    const curPoint = _points[i]
    const nextPoint = _points[(i + 1) % cornerCount]

    // 两条边的方向
    const preDirection = prePoint.clone().sub(curPoint).normalize()
    const nextDirection = nextPoint.clone().sub(curPoint).normalize()

    // 两条边形成夹角
    const angle = preDirection.angleTo(nextDirection) / 2
    const cornerLength = lineWidth / Math.sin(angle)

    // 两条边的角平分线方向
    const cornerDirection = preDirection.clone().applyAxisAngle(axisY, angle)

    // 一个角对应的内、外两个定位点
    const innerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, cornerLength / 2)
    const outerPoint = curPoint
      .clone()
      .addScaledVector(cornerDirection, -cornerLength / 2)
    outlinePoints[i].push(innerPoint, outerPoint)

    // 基于当前定位点，和下一组定位点，计算当前边的长度
    // 保证两端的虚线实体都存在，即[实体,间隔,实体,...,实体,间隔,实体]，所以两端的实体可以更长一些
    const dist = fixedPrecision(nextPoint.distanceTo(curPoint))
    // 分段数，理应大于2
    const segment = Math.floor(dist / span)
    // 中间等分跨度数量，要么为 0，要么一定是奇数
    let middleSpanCount = 0
    if (segment % 2 === 1) {
      // 如果是奇数
      middleSpanCount = Math.max(0, segment - 2)
    } else {
      // 如果是偶数
      middleSpanCount = Math.max(0, segment - 3)
    }
    // 虚线实体的数量
    const entityCount = Math.max(0, (middleSpanCount - 1) / 2)

    // 左右两端实体的长度
    let sideSpan = (dist - middleSpanCount * span) / 2
    if (entityCount === 0) {
      // 如果中间没有虚线的实体部分，则限定两端的实体长度
      sideSpan = span
    }
    // 从当前定位点到下一个定位点方向的垂直向量
    const vertical = new Vector3(-nextDirection.z, 0, nextDirection.x)

    // 第一组实体边界顶点
    const firstBasicPoint = curPoint
      .clone()
      .addScaledVector(nextDirection, sideSpan)
    const firstInnerPoint = firstBasicPoint
      .clone()
      .addScaledVector(vertical, lineWidth / 2)
    const firstOuterPoint = firstBasicPoint
      .clone()
      .addScaledVector(vertical, -lineWidth / 2)
    outlinePoints[i].push(firstInnerPoint, firstOuterPoint)

    // 中间的实体个数，每一个实体对应4个顶点（2个三角面）
    let offset = sideSpan + span
    let count = 0
    while (count < entityCount * 2) {
      const basicPoint = curPoint.clone().addScaledVector(nextDirection, offset)
      const iPoint = basicPoint.clone().addScaledVector(vertical, lineWidth / 2)
      const oPoint = basicPoint
        .clone()
        .addScaledVector(vertical, -lineWidth / 2)
      outlinePoints[i].push(iPoint, oPoint)

      count++
      offset += span
    }

    // 最后一组实体边界顶点
    const lastBasicPoint = curPoint
      .clone()
      .addScaledVector(nextDirection, dist - sideSpan)
    const lastInnerPoint = lastBasicPoint
      .clone()
      .addScaledVector(vertical, lineWidth / 2)
    const lastOuterPoint = lastBasicPoint
      .clone()
      .addScaledVector(vertical, -lineWidth / 2)
    outlinePoints[i].push(lastInnerPoint, lastOuterPoint)

    if (i === 0) {
      vertexBasicIndex[i] = 0
    } else {
      // 顶点索引的基准值依赖上一组顶点的数据
      vertexBasicIndex[i] =
        vertexBasicIndex[i - 1] + outlinePoints[i - 1].length
    }
  }

  const vertices = []
  const indices = []

  for (let i = 0; i < cornerCount; i++) {
    const curOutlinePoints = outlinePoints[i]
    const basicIndex = vertexBasicIndex[i]
    for (let j = 0; j < curOutlinePoints.length; j += 4) {
      let p1, p2, p3, p4
      if (j + 4 > curOutlinePoints.length) {
        // 最后一组实体边界顶点，跟下一条边的2个角点，组成另一段实体
        p1 = curOutlinePoints[j]
        p2 = curOutlinePoints[j + 1]
      } else {
        // 2个角点，跟第一组实体边界顶点，组成第一段实体
        p1 = curOutlinePoints[j]
        p2 = curOutlinePoints[j + 1]
        p3 = curOutlinePoints[j + 2]
        p4 = curOutlinePoints[j + 3]
      }

      // 保存顶点坐标
      vertices.push(p1.x, p1.y, p1.z)
      vertices.push(p2.x, p2.y, p2.z)
      p3 && vertices.push(p3.x, p3.y, p3.z)
      p4 && vertices.push(p4.x, p4.y, p4.z)

      // 形成三角面的顶点索引
      const index1 = basicIndex + j
      const index2 = basicIndex + j + 1
      let index3 = basicIndex + j + 2
      let index4 = basicIndex + j + 3
      if (!p3 || !p4) {
        // 索引为下一组的角点
        const nextBasicIndex = (i + 1) % cornerCount
        index3 = vertexBasicIndex[nextBasicIndex]
        index4 = vertexBasicIndex[nextBasicIndex] + 1
      }

      indices.push(index1, index3, index2)
      indices.push(index4, index2, index3)
    }
  }

  // 主动释放内存
  outlinePoints.length = 0
  vertexBasicIndex.length = 0

  const _vertices = new Float32Array(vertices)

  return {
    vertices: _vertices,
    indices,
  }
}
