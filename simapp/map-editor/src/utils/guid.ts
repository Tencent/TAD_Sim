import { v4 as uuidv4 } from 'uuid'

type cacheIdType = 'road' | 'junction' | 'object' | 'laneBoundary' | 'laneLink'
type InvalidIdType = 'bigInt' | 'repeat'
interface IIdLog {
  element: cacheIdType
  type: InvalidIdType
  id: string
}

let roadId = 0
let junctionId = 0
let objectId = 0
let laneBoundaryId = 0
let laneLinkId = 0

// 用于在解析地图元素阶段，判断是否有重复的元素 id，如果有则重新给其赋值
const roadIdSet: Set<string> = new Set()
const junctionIdSet: Set<string> = new Set()
const objectIdSet: Set<string> = new Set()
const laneBoundarySet: Set<string> = new Set()
const laneLinkSet: Set<string> = new Set()
// 不符合规范的 id 记录
const invalidIdLog: Array<IIdLog> = []

// 解析过程中缓存 id
export function cacheId (elementType: cacheIdType, id: string | number) {
  if (
    elementType !== 'road' &&
    elementType !== 'junction' &&
    elementType !== 'object' &&
    elementType !== 'laneBoundary' &&
    elementType !== 'laneLink'
  ) {
    return false
  }

  // 将 id 统一转换成字符串类型
  if (typeof id === 'number') {
    id = String(id)
  }

  // 如果是大数
  if (isBigIntId(id)) {
    invalidIdLog.push({
      element: elementType,
      type: 'bigInt',
      id,
    })
    return false
  }

  // 重复 id 记录
  let tempSet: Set<string>
  switch (elementType) {
    case 'road':
      tempSet = roadIdSet
      break
    case 'junction':
      tempSet = junctionIdSet
      break
    case 'object':
      tempSet = objectIdSet
      break
    case 'laneBoundary':
      tempSet = laneBoundarySet
      break
    case 'laneLink':
      tempSet = laneLinkSet
      break
  }
  if (tempSet.has(id)) {
    invalidIdLog.push({
      element: elementType,
      type: 'repeat',
      id,
    })
    return false
  }

  tempSet.add(id)
  return true
}

export function checkIdValid (type: cacheIdType, id: string | number) {
  // 第三方地图中 roadId 存在为 0 的情况
  if (!id && id !== 0) return false
  if (typeof id === 'number') {
    id = String(id)
  }
  if (type === 'road') {
    return roadIdSet.has(id)
  } else if (type === 'junction') {
    return junctionIdSet.has(id)
  } else if (type === 'object') {
    return objectIdSet.has(id)
  } else if (type === 'laneBoundary') {
    return laneBoundarySet.has(id)
  } else if (type === 'laneLink') {
    return laneLinkSet.has(id)
  }

  return false
}

// 生成递增的道路 id
export function genRoadId () {
  while (roadIdSet.has(String(roadId + 1))) {
    roadId++
  }
  roadId++
  const _roadId = String(roadId)
  roadIdSet.add(_roadId)
  return _roadId
}

// 生成递增的交叉路口 id
export function genJunctionId () {
  while (junctionIdSet.has(String(junctionId + 1))) {
    junctionId++
  }
  junctionId++
  const _junctionId = String(junctionId)
  junctionIdSet.add(_junctionId)
  return _junctionId
}

// 生成递增的物体 id
export function genObjectId () {
  while (objectIdSet.has(String(objectId + 1))) {
    objectId++
  }
  objectId++
  const _objectId = String(objectId)
  objectIdSet.add(_objectId)
  return _objectId
}

// 生成递增的车道边界线 id
export function genLaneBoundaryId () {
  while (laneBoundarySet.has(String(laneBoundaryId + 1))) {
    laneBoundaryId++
  }
  laneBoundaryId++
  const _laneBoundaryId = String(laneBoundaryId)
  laneBoundarySet.add(_laneBoundaryId)
  return _laneBoundaryId
}

// 生成递增的车道连接线 id
export function genLaneLinkId () {
  while (laneLinkSet.has(String(laneLinkId + 1))) {
    laneLinkId++
  }
  laneLinkId++
  const _laneLinkId = String(laneLinkId)
  laneLinkSet.add(_laneLinkId)
  return _laneLinkId
}

// 清除缓存
export function clearIdCache () {
  roadId = 0
  junctionId = 0
  objectId = 0
  laneBoundaryId = 0
  laneLinkId = 0

  roadIdSet.clear()
  junctionIdSet.clear()
  objectIdSet.clear()
  laneBoundarySet.clear()
  laneLinkSet.clear()

  invalidIdLog.length = 0
}

// 生成普通的 uuid，适用于交互控制点的 id
export function genUuid () {
  return uuidv4()
}

// 如果超过 id 超过 8 位数（千万），则可以视为 id 为大数，需要使用前端侧的 id 代替
export function isBigIntId (id: number | string) {
  if (String(id).length > 8) return true
  return false
}
