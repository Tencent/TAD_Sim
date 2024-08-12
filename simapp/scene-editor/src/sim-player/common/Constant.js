/**
 * 场景编辑状态Map
 * @type {{
 * StatePickMeasurementEnd: number,
 * StatePickEgoCarEnd: number,
 * StateCount: number,
 * StatePickVehicleEnd: number,
 * StateEdit: number,
 * isEditingCurve(*): *,
 * StatePickPedestrianEnd: number
 * }}
 */
export const SceneState = {
  StateEdit: 0,
  StatePickEgoCarEnd: 1,
  StatePickVehicleEnd: 2,
  StatePickPedestrianEnd: 3,
  StatePickMeasurementEnd: 4,
  StateCount: 5,
  isEditingCurve (state) {
    return (
      state === this.StatePickEgoCarEnd ||
      state === this.StatePickVehicleEnd ||
      state === this.StatePickPedestrianEnd ||
      state === this.StatePickMeasurementEnd
    )
  },
}

/**
 * 地图元素类型
 * @type {{ROAD: string, BOUNDARY: string, INVALID: string, OBJECT: string, LANELINK: string, LANE: string}}
 */
export const MapElementType = {
  INVALID: 'invalid',
  LANE: 'lane',
  LANELINK: 'lanelink',
  ROAD: 'road',
  OBJECT: 'object',
  BOUNDARY: 'boundary',
}

/**
 * 不同场景元素的最大横向偏移
 * @type {{Pedestrian: number, Other: number}}
 */
export const OffsetConst = {
  Pedestrian: 5,
  Other: 2.5,
}

/**
 * 事件触发条件Map
 * @type {{TimeAbsolute: string, TimeRelative: string, DistanceRelative: string, DistanceAbsolute: string}}
 */
export const TriggerCondition = {
  TimeAbsolute: 'timeAbsolute',
  TimeRelative: 'timeRelative',
  DistanceAbsolute: 'distanceAbsolute',
  DistanceRelative: 'distanceRelative',
}

/**
 * 加速度结束条件类型Map
 * @type {{Time: string, None: string, Velocity: string}}
 */
export const AccelerationTerminationType = {
  None: '',
  Time: 'time',
  Velocity: 'velocity',
}

/**
 * 距离类型，欧式or车道
 * @type {{Euclidean: string, LaneProjection: string, None: string}}
 */
export const DistanceMode = {
  None: '',
  Euclidean: 'euclideandistance',
  LaneProjection: 'laneprojection',
}

/**
 * 运行环境，单机or云端
 * @type {{Cloud: number, Local: number}}
 */
export const RunMode = {
  Local: 0,
  Cloud: 1,
}

/**
 * 是否忽略道路高程
 * @type {{No: number, Yes: number}}
 */
export const IgnoreHeight = {
  No: 0,
  Yes: 1,
}

/**
 * 当前操作状态Map
 * @type {{PICK: number, ROTATE: number, PAN: number}}
 */
export const OperationMode = {
  PICK: 1,
  PAN: 2,
  ROTATE: 3,
}

/**
 * renderOrder方案各个元素的order Map
 * @type {{
 * MAPFILTH: number, ROAD_SIGN: number,
 * GRID: number, LIGHT: number, BOUNDARY: number,
 * OBJECTS: number, POLE: number, LANE: number
 * }}
 */
export const RenderOrder = {
  GRID: 0,
  LANE: 100,
  BOUNDARY: 150,
  ROAD_SIGN: 200,
  MAPFILTH: 250,
  OBJECTS: 300, // 模型
  POLE: 400,
  LIGHT: 500,
}

export const remappingReg = /(Ego_(\d{3})\/)?(\w+)?/
