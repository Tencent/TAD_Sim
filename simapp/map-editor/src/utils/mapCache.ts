// 尽可能少地将嵌套对象添加到 store 中存储，减少 vue 源码内部的监听和 trigger 影响性能
// 手动来管理地图核心元素的状态
const roadMap: Map<string, biz.IRoad> = new Map()
const junctionMap: Map<string, biz.IJunction> = new Map()
const laneLinkMap: Map<string, biz.ILaneLink> = new Map()
const objectMap: Map<string, biz.IObject> = new Map()

// road 元素
export function getRoad (roadId: string): biz.IRoad | undefined {
  if (!roadMap.has(roadId)) return undefined
  return roadMap.get(roadId)
}

export function setRoad (roadId: string, road: biz.IRoad) {
  roadMap.set(roadId, road)
}
export function removeRoad (roadId: string | Array<string>) {
  if (!Array.isArray(roadId)) {
    roadId = [roadId]
  }
  for (const id of roadId) {
    roadMap.has(id) && roadMap.delete(id)
  }
}
// junction 元素
export function getJunction (junctionId: string): biz.IJunction | undefined {
  if (!junctionMap.has(junctionId)) return undefined
  return junctionMap.get(junctionId)
}
export function setJunction (junctionId: string, junction: biz.IJunction) {
  junctionMap.set(junctionId, junction)
}
export function removeJunction (junctionId: string | Array<string>) {
  if (!Array.isArray(junctionId)) {
    junctionId = [junctionId]
  }
  for (const id of junctionId) {
    junctionMap.has(id) && junctionMap.delete(id)
  }
}
export function removeJunctionLink (
  params:
    | { junctionId: string, laneLinkId: string }
    | Array<{ junctionId: string, laneLinkId: string }>,
) {
  if (!Array.isArray(params)) {
    params = [params]
  }
  for (const { junctionId, laneLinkId } of params) {
    const junction = junctionMap.get(junctionId)
    if (junction) {
      const index = junction.laneLinks.findIndex(
        (laneLink: biz.ILaneLink) => laneLink.id === laneLinkId,
      )
      if (index > -1) {
        junction.laneLinks.splice(index, 1)
      }
    }
  }
}
export function addJunctionLink (params: {
  junctionId: string
  laneLink: biz.ILaneLink
}) {
  const { junctionId, laneLink } = params

  // 往 junctionMap 存入 laneLink 数据
  const junction = junctionMap.get(junctionId)
  if (junction) {
    junction.laneLinks.push(laneLink)
    return laneLink
  }
}

// laneLink 元素
export function getLaneLink (laneLinkId: string): biz.ILaneLink | undefined {
  if (!laneLinkMap.has(laneLinkId)) return undefined
  return laneLinkMap.get(laneLinkId)
}
export function setLaneLink (laneLinkId: string, laneLink: biz.ILaneLink) {
  laneLinkMap.set(laneLinkId, laneLink)
}
export function removeLaneLink (laneLinkId: string | Array<string>) {
  if (!Array.isArray(laneLinkId)) {
    laneLinkId = [laneLinkId]
  }
  for (const id of laneLinkId) {
    laneLinkMap.has(id) && laneLinkMap.delete(id)
  }
}
// object 元素
export function getObject (objectId: string): biz.IObject | undefined {
  if (!objectMap.has(objectId)) return undefined
  return objectMap.get(objectId)
}
export function setObject (objectId: string, object: biz.IObject) {
  objectMap.set(objectId, object)
}
export function removeObject (objectId: string | Array<string>) {
  if (!Array.isArray(objectId)) {
    objectId = [objectId]
  }
  for (const id of objectId) {
    objectMap.has(id) && objectMap.delete(id)
  }
}
// 重置
export function clearRoad () {
  roadMap.clear()
}
export function clearJunction () {
  junctionMap.clear()
}
export function clearLaneLink () {
  laneLinkMap.clear()
}
export function clearObject () {
  objectMap.clear()
}

export function clearAllCache () {
  clearRoad()
  clearJunction()
  clearLaneLink()
  clearObject()
}
