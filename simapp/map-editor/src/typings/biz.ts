declare namespace biz {
  import type { Curve, Vector3 } from 'three'

  // 根据新增的插件拓展
  export type IPluginName =
    | 'editRoad'
    | 'editJunction'
    | 'editObject'
    | 'editLaneAttr'
    | 'editBoundary'
    | 'editLaneWidth'
    | 'editLaneNumber'
    | 'editLink'
    | 'editSection'
    | 'editElevation'
    | 'editSignalControl'
    | 'editCircleRoad'
    | 'editCrg'

  export type ICurve3 = Curve<Vector3>
  export type ILaneLinkRoadType = 'start' | 'end'
  export type IOrientation = '+' | '-' | 'none'
  export type ICameraType = 'P' | 'O'
  export type IControlType = 'line' | 'arc' | 'bezier' | 'catmullrom'
  export type IMapType = 'sqlite' | 'xodr 1.4' | 'xodr 1.5'
  // OpenCRG 相关的属性可选字段
  export type IOpenCrgOrientation = 'same' | 'opposite'
  export type IOpenCrgMode = 'attached' | 'attached0' | 'genuine' | 'global' // 对应的顺序：相对附着，绝对附着，真实，全局
  export type IOpenCrgPurpose = 'elevation' | 'friction'
  // 当前支持的车道类型
  export type ILaneType =
    | 'Driving'
    | 'Stop'
    | 'Shoulder'
    | 'Biking'
    | 'Sidewalk'
    | 'Border'
    | 'Restricted'
    | 'Parking'
    | 'Bidirectional'
    | 'ConnectingRamp'
    | 'Curb'
    | 'Entry'
    | 'Exit'
    | 'Median'
    | 'Offramp'
    | 'Onramp'
    | 'Rail'
    | 'RoadWorks'
    | 'Tram'
    | 'None'
  // 在支持双向车道后，需要区分正向车道和反向车道
  export type ILaneDirection = 'forward' | 'reverse'

  export interface ICmdOption {
    cmd: 'add' | 'update' | 'delete'
    type: 'road' | 'lanelink' | 'object'
    id: string
  }

  // 通用的元素属性类型
  // TODO 后续需要根据实际的地图数据做调整
  // 从地图文件中加载的道路数据，用 ICommonXXX 表示前后端通信透传的数据格式
  export interface ICommonElement {
    id: string
  }

  export interface ICommonOpenCRG {
    file: string // 包含 crg 数据的文件名称
    orientation: IOpenCrgOrientation // 相对于父级road元素的方向
    mode: IOpenCrgMode // 道路表面数据的附加模式
    purpose: IOpenCrgPurpose // 数据的物理用途
    sOffset: string // 中心线与道路参考线之间的s偏移
    tOffset: string // 中心线与道路参考线之间的t偏移
    zOffset: string // 中心线与道路参考线之间的z偏移
    zScale: string // 道路表面描述的 z 比例因子
    hOffset: string // 中心线与道路参考线之间的偏向角
  }

  // 高程控制点的结构
  export interface IElevationPoints {
    s: number // 控制点在道路参考线 s 轴上的坐标
    h: number // 控制点的高度
    slope: string // 坡度，控制点切线方向跟 s 轴的夹角（弧度制）
  }

  export interface ICommonRoad extends ICommonElement {
    length: number // 目前为道路中心线长度
    type: number
    samplePoints: Array<common.vec3>
    sections: Array<ICommonSection>
    // 控制点的数据和类型
    controlPoints?: Array<ICommonControlPoint>
    controlType?: IControlType
    opencrgs?: Array<ICommonOpenCRG>
    elevation?: Array<IElevationPoints>
  }
  export interface ICommonSection {
    id: string
    length: number // 通过 startPercent 和 endPercent 对 road.length 进行截断
    start: number // section 在当前道路中心线采样点的开始位置索引
    end: number // section 在当前道路中心线采样点的结束位置索引
    startPercent: number
    endPercent: number
    lanes: Array<ICommonLane>
    boundarys: Array<ICommonLaneBoundary>
  }

  export interface ICommonLaneBoundary {
    id: string // 车道边界线的 id
    mark: number // 车道线类型，通过映射关系表来维护
    samplePoints: Array<common.vec3> // 车道线的采样点
  }

  export interface ICommonLane {
    id: string // 车道 id
    type: number // 车道类型通过 int 类型的映射表进行维护
    lbid: string // 左边界线 id
    rbid: string // 右边界线 id
    speedlimit: number // 限速
    samplePoints: Array<common.vec3> // 车道中心线的采样点
    friction: number // 摩擦力系数
    sOffset: number // 材质纵向距离
  }

  // 交叉路口连接线的前后端透传数据
  export interface ICommonLaneLink extends ICommonElement {
    fid: string // 来路 laneId
    frid: string // 来路 roadId
    fsid: string // 来路 sectionId
    ftype: ILaneLinkRoadType // 来路的道路起始还是结束
    tid: string // 去路 laneId
    trid: string // 去路 roadId
    tsid: string // 去路 sectionId
    ttype: ILaneLinkRoadType // 去路的道路起始还是结束
    junctionid: string
    length: number // 路口连接线的长度
    roadid: string // 为了保证后端增量保存 xodr 文件有效，需要给 lanelink 提供唯一的 roadid
    samplePoints: Array<common.vec3>
    controlPoints?: Array<ICommonControlPoint>
    controlType?: IControlType
  }

  // 前后端透传的 junction 数据，junction 不会嵌套 laneLink
  export interface ICommonJunction extends ICommonElement {}

  // 前后端透传的控制点数据，主要用于曲率优化
  export interface ICommonControlPoint {
    x: number
    y: number
    z: number
    hdg: string
  }

  // 物体自定义字段结构
  export interface IUserData {
    code: string
    value: any // 由于需要在 userdata 中针对不同的物体透传定制数据，不一定都是字符串
  }
  // 停车位描边属性
  export interface IOutline {
    id: string
    s: string
    t: string
  }
  // 停车位边线样式
  export interface IMarking {
    cornerreferenceid: string
    width: string
    color: string
  }

  // 重复属性的类型
  export interface IRepeatProperty {
    s: string
    length: string
    distance: string
    tStart: string
    tEnd: string
    widthStart: string
    widthEnd: string
    lengthStart: string
    lengthEnd: string
    heightStart: string
    heightEnd: string
    zOffsetStart: string
    zOffsetEnd: string
  }
  // object 的类型
  export interface ICommonObject extends ICommonElement {
    type: string
    subtype: string
    name: string
    roadid: string
    lanelinkid: string
    s: string
    t: string
    zOffset: string
    validLength: string
    orientation: string
    radius: string
    length: string
    width: string
    height: string
    hdg: string
    pitch: string
    roll: string
    userdata?: Array<IUserData> // 自定义用户数据
    repeat?: IRepeatProperty // 重复属性
    outlines?: Array<IOutline> // 停车位描边角点
    markings?: Array<IMarking> // 停车位边线样式
  }

  export interface ICommonMapElements {
    header?: {
      version: string
    }
    roads: Array<ICommonRoad>
    // junctions: Array<ICommonJunction>
    lanelinks: Array<ICommonLaneLink>
    objects: Array<ICommonObject>
  }

  export interface ISaveCommand {
    command: common.diffType
    type: common.elementType
    id: string
    data?: ICommonJunction | ICommonRoad | ICommonObject
  }

  // ------------------------------------------------------------------------

  // 前端侧元素属性类型
  export interface IGeoAttr {
    vertices: Array<number> | Float32Array
    indices: Array<number>
    uvs?: Array<number>
  }
  export interface ILaneBoundary extends ICommonLaneBoundary {
    // 在数据层就尽可能算好，避免在渲染层同步计算
    firstLineAttr: IGeoAttr | null // 车道线样式第一条线的几何体属性
    secondLineAttr?: IGeoAttr | null // 车道线样式第二条线的几何体属性（不一定存在）
    isForward: boolean // 由于支持双向车道，需要判断当前边界线是否是正向车道的边界线
  }

  export interface ILane extends ICommonLane {
    enabled: boolean // 当前连接线是否生效
    isTransition: boolean // 是否属于过渡车道
    isExtends?: boolean // 是否是拓展出来的效果，只有在 isTransition 为 true 时才有作用
    normalWidth: number // 车道绝大部分平均的宽度
    geoAttr: IGeoAttr | null
  }

  export interface ISection {
    id: string // sectionId 用字符串维护
    lanes: Array<ILane>
    // 表示 section 占当前道路的起始和结束百分比
    pStart: number
    pEnd: number
    length: number
    boundarys: Array<ILaneBoundary>
  }

  export interface IRoad {
    id: string
    sections: Array<ISection>
    length: number // 当前道路的长度
    keyPath: ICurve3
    elevationPath?: ICurve3
    linkJunction: Array<string> // 跟当前道路相连的交叉路口
    type: number // 道路类型，应该有映射关系对应
    verticalAtHead: Vector3 // 车道中心线起始位置的垂直向量，指向正向车道一侧
    verticalAtTail: Vector3 // 车道中心线结束位置的垂直向量，指向正向车道一侧
    isCircleRoad?: boolean // 是否属于环形道路的部分
    circleOption?: biz.ICircleOption
    crgConfig?: Array<IOpenCrg> // 当前道路关联的 openCRG 配置文件
    roadType?: 'default' | 'tunnel' // 道路的类型。由于隧道作为物体的形式进行存储，在道路的数据结构中需要新增类型描述道路
  }

  // 基于道路拓展隧道的类型
  export interface ITunnel extends IRoad {
    roadType: 'tunnel' // 固定 roadType 属性为隧道
    tunnelS: number // 隧道的起始点 s 坐标
    tunnelLength: number // 隧道的长度
  }

  // 由于前端侧将 laneLink 嵌套在 junction 层级下会方便管理，需要区别于透传后端的数据结构
  export interface ILaneLink {
    fid: string // 来路 laneId
    frid: string // 来路 roadId
    fsid: string // 来路 sectionId
    ftype: ILaneLinkRoadType // 来路的道路起始还是结束
    tid: string // 去路 laneId
    trid: string // 去路 roadId
    tsid: string // 去路 sectionId
    ttype: ILaneLinkRoadType // 去路的道路起始还是结束
    id: string // laneLinkId
    length: number // 当前 laneLink 路径的长度
    samplePoints: Array<common.vec3> // 采样点
    roadid: string // 为了适配后端增量保存 xodr 逻辑，lanelink 中存一个唯一的 roadid
    enabled: boolean // 表示当前道路是否受其他元素联动影响（比如路面标识、标志牌控制等）link 关联不生效
    controlPoints: Array<ICommonControlPoint> // lanelink 的贝塞尔曲线的控制点
    controlType?: IControlType // 控制点的类型，默认都是 bezier
  }

  // roadId_percent 的组合。percent 为 0 表示为道路起点；percent 为 1 表示为道路终点
  // 由于需要支持正反向车道的双向道路，roadId_percent字段不足以支持
  // 暂定用 roadId_percent_forward 和 roadId_percent_reverse 字段来表示是正向车道侧还是反向车道侧
  export type ILinkRoad = string

  export interface IJunction {
    id: string
    laneLinks: Array<ILaneLink>
    linkRoads: Array<ILinkRoad>
    // 形成路口的相关道路边界数据
    refRoads: Array<IRefRoad>
    geoAttr: IGeoAttr | null
  }

  // 前端侧划分的物体大类
  export type MainType =
    | 'pole'
    | 'roadSign'
    | 'signalBoard'
    | 'trafficLight'
    | 'other'
    | 'parkingSpace' // 停车位作为 roadSign 的特殊情况来处理
    | 'sensor'
    | 'customModel' // 自定义导入模型
    | ''

  // 前端侧物体字段的定义
  export interface IObject {
    id: string
    mainType: MainType // 大得物体类型
    name: string // 当前类型中具体的某一种
    showName: string // 展示的名称（中文 or 英文）
    type: string
    subtype: string
    // 关联的道路级信息
    roadId: string
    sectionId: string
    laneId: string
    // 关联的交叉路口级信息
    junctionId: string
    laneLinkId: string
    // 空间信息
    position: common.vec3
    scale: common.vec3
    // 世界坐标系下的角度
    yaw: number // 前端侧展示角度制
    angle: number // 前端侧展示角度制

    // st 坐标系坐标
    s: number
    t: number

    // 部分情况需要手动控制物体看向的坐标
    lookAtPoint: common.vec3 | null
    // 距离参考线最近的交点信息
    closestPoint: common.vec3 | null
    closestPointTangent: common.vec3 | null
    closestPointPercent: number
    // 如果物体处于道路上，在道路参考线上投影点的坡度法向量
    projectNormal?: common.vec3 | null

    // 物体的尺寸
    radius: number
    width: number
    height: number
    length: number
  }

  export interface IPole extends IObject {}
  export interface ISignalBoard extends IObject {
    poleId: string // 信号标志牌放置的父级杆 id
    onVerticalPole: boolean // 是否放置在竖直杆上
  }
  export interface IRoadSign extends IObject {
    size: Array<number> // 图片纹理的 width 和 height
    // 模型原始的尺寸，主要用于倍数调整
    basicWidth: number
    basicLength: number
    basicHeight: number
  }
  // 基于顶点坐标创建的特殊路面标线（需要跟随道路参考线调整网格顶点位置）
  export interface ICustomRoadSign extends IObject {
    geoAttrs: Array<IGeoAttr> // 可能存在多组几何体属性
  }

  // 路口导向线
  export interface IIntersectionGuideLine extends ICustomRoadSign {
    curvePoints: Array<common.vec3>
    // 驶入路口的道路（道路的尾部）
    frid: string
    // 驶出路口的道路（道路的头部）
    trid: string
  }

  export interface ITrafficLight extends IObject {
    poleId: string
    onVerticalPole: boolean
    controlJunctionId: string
    controlRoadId: string // 一个物理灯只能绑定一条道路
  }

  export interface ISensor extends IObject {
    poleId: string
    onVerticalPole: boolean
    deviceParams: any // 传感器设备参数（rsu，摄像头，激光雷达，毫米波雷达），每一种的参数都不一样
  }
  export interface IOther extends IObject {
    isPlaneModel: boolean // 是否是平面模型
    size?: Array<number> // 图片纹理的 width 和 height，只有 isPlaneModel 为 true 才需要用到
    // 模型原始的尺寸，主要用于倍数调整
    basicWidth: number
    basicLength: number
    basicHeight: number
    span?: number // 天桥的跨度
  }

  // 自定义三维模型的数据结构
  export interface ICustomModel extends IObject {
    // 模型原始的尺寸，主要用于倍数调整
    basicWidth: number
    basicLength: number
    basicHeight: number
  }

  // 停车位单独提供数据结构
  export interface IParkingSpace extends IObject {
    width: number
    length: number
    margin: number // 相邻的两个停车位的间隔
    innerAngle: number
    lineWidth: number // 停车位线宽
    color: common.colorType // 线的颜色
    count: number // 重复的数量
    // 以下数组属性，按照索引顺序对应不同的停车位
    geoAttrs: Array<IGeoAttr> // 每一个停车位的几何体属性，可直接用于绘制
    cornerPoints: Array<Array<common.vec3>> // 世界坐标系下，停车位的4个角的坐标
    sCoordinates: Array<number> // 每一个停车位几何中点的 s 坐标
    cornerSTPoints: Array<Array<common.vec2>> // st 坐标系下，停车位的4个角点坐标，x -> s, y -> t
  }

  export interface IOpenCrg {
    showName: string // 展示的 crg 名称fi
    file: string // 包含 crg 数据的文件名称
    id?: string // 【云端版】crg 文件 id
    version?: string // 【云端版】crg 文件的版本
    // sStart: string // 应用crg的s起始坐标
    // sEnd: string // 应用crg的s结束坐标
    orientation: IOpenCrgOrientation // 相对于父级road元素的方向
    mode: IOpenCrgMode // 道路表面数据的附加模式
    purpose: IOpenCrgPurpose // 数据的物理用途
    sOffset: string // 中心线与道路参考线之间的s偏移
    tOffset: string // 中心线与道路参考线之间的t偏移
    zOffset: string // 中心线与道路参考线之间的z偏移
    zScale: string // 道路表面描述的 z 比例因子
    hOffset: string // 中心线与道路参考线之间的偏向角
  }

  // 跟参考线坐标系相关的计算后属性
  export interface IRefLineLocation {
    s: number
    t: number
    percent: number
    tangent: Vector3
    closestPointOnRefLine: Vector3
    closestPointInLane?: Vector3
    normal: Vector3 // 目标点在道路参考线投影点的坡度法向量。默认为竖直朝上
  }

  export interface IRefRoad {
    roadId: string
    isTail: boolean
    direction: ILaneDirection // 正向或反向车道
    alongVec: Vector3 // curve 曲线某一侧的端点切线方向的向量在 xz 平面的分量
    leftPoint: Vector3 | null
    rightPoint: Vector3 | null
  }

  // 计算路口中的车道连接线的车道基础信息
  export interface ILaneLinkInfo {
    flag: string
    isTail: boolean
    laneEndPoint: common.vec3 // 车道中心线上的某个采样端点（首或尾）
    tangent: common.vec3
    direction: ILaneDirection
  }

  export interface IControlPoint {
    id: string
    roadId: Array<string> // 存成数组形式，适用于一组控制点控制两条单向道路情况
    points: Array<IRefPoint>
  }

  export interface IRefPoint {
    id: string
    parentId: string
    x: number
    y: number
    z: number
  }

  // 环形道路的配置参数
  export interface ICircleOption {
    radius: number
    startAngle: number
    endAngle: number
    center: common.vec3
    points?: Array<common.vec3>
    isClockwise?: boolean // 新建的环形道路，无论是正向还是反向，都是逆时针绘制的；从地图文件中加载的反向道路，圆弧的绘制需要标识是顺时针的
  }

  // 常规前端部分类型
  export interface IFile {
    id: string | number
    preset: string
    name: string // 文件名，默认带后缀
    size: string
    createTime: string
  }
}
