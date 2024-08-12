import { Root } from 'protobufjs'

// 获取 Electron 提供的 proto 文件描述
const { electron } = window
const protoRoot = new Root()
Root.fromJSON(electron.modules.getProtoDescription(), protoRoot)
console.log(protoRoot)

// 定义消息类型及其对应的 topic 列表
const typeMap = {
  Location: [
    'LOCATION',
    'LOCATION_REPLAY',
    'LOCATION_TRAILER',
    'LOCATION_TRAILER_REPLAY',
  ],
  Trajectory: [
    'TRAJECTORY',
    'TRAJECTORY_1',
    'TRAJECTORY_2',
    'TRAJECTORY_3',
    'TRAJECTORY_4',
    'TRAJECTORY_5',
    'TRAJECTORY_REPLAY',
  ],
  Grading: [
    'GRADING',
  ],
  Control: [
    'CONTROL',
  ],
  Control_V2: [
    'CONTROL_V2',
  ],
  Traffic: [
    'TRAFFIC',
    'TRAFFIC_REPLAY',
  ],
  VehicleState: [
    'VEHICLE_STATE',
  ],
  // 没人用了，先屏蔽
  // TrajectoryFollow: [
  //   'TRAJECTORY_FOLLOW',
  // ],
  // V2xEarlyWarning: [
  //   'V2XEARLYWARNING',
  // ],
}

// 创建 topic 到解码器的映射
const topicMap = new Map()

Object.entries(typeMap).forEach(([key, value]) => {
  const decoder = protoRoot.lookupType(key)
  value.forEach((topic) => {
    topicMap.set(topic, decoder)
  })
})

// 提取 ego_id 的正则表达式
const egoReg = /(Ego_(\d{3})\/)?(\w+)/

/**
 * 根据 topic 获取对应的解码器
 * @param {string} topic - 要获取解码器的主题
 * @return {protobufjs.Type} 解码器
 */
function getDecoder (topic) {
  let realTopic = topic
  const matchResult = egoReg.exec(topic)
  if (matchResult?.length) {
    const [,,, t] = matchResult
    realTopic = t
  }
  return topicMap.get(realTopic)
}

const { nested: { sim_msg: simMsg } } = protoRoot

// 基本类型列表
const basicTypes = [
  'double',
  'int64',
  'uint64',
  'int32',
  'uint32',
  'float',
  'bool',
  'string',
  'bytes',
]

/**
 * 递归查找嵌套类型定义
 * @param {string} type - 类型名称
 * @param {protobufjs.Type} typeDefine - 类型定义
 * @return {protobufjs.Type} 找到的类型定义
 */
function findType (type, typeDefine) {
  const arr = type.split('.')
  if (arr.length > 1) {
    let subType = typeDefine
    for (const i of arr) {
      subType = findType(i, subType)
    }
    return subType
  }
  let subType = typeDefine[type]
  let c = typeDefine

  while (!subType && c.parent) {
    subType = c.parent[type] || (c.parent.nested && c.parent.nested[type])
    c = c.parent
  }

  return subType
}

function mapOptions (type) {
  if (type.fields) {
    return Object.entries(type.fields).map(([key, value]) => {
      const label = key

      if (basicTypes.includes(value.type)) {
        return {
          label,
          value: key,
          type: value.type,
        }
      }

      const subType = findType(value.type, type)
      if (subType && subType.fields) {
        return {
          label,
          value: key,
          type: value.type,
          children: mapOptions(subType),
        }
      }

      return undefined // 特殊的类型不可选择
    }).filter(e => !!e)
  }
  return []
}

// 获取 pb 列表，格式为 [{ label, value, type, children }]
function getTopicProto (field) {
  if (!simMsg[field]) return []
  return mapOptions(simMsg[field])
}

export {
  topicMap,
  getDecoder,
  getTopicProto,
}
