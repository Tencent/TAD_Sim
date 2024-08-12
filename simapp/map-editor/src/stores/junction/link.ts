// 跟交叉路口中车道连接线相关的计算逻辑

export interface ILinkRoadData {
  // 是否是道路的尾部，true 表示道路驶出侧，false 表示道路驶入侧
  isTail: boolean
  road: biz.IRoad
}

/**
 * 通过 laneLink 驶入/驶出车道的信息，获取唯一的标识
 * @param roadId
 * @param sectionId
 * @param laneId
 * @param type
 * @param direction // 区分正反向车道
 * @returns
 */
export function getLaneLinkUniqueFlag (
  roadId: string,
  sectionId: string,
  laneId: string,
  type: biz.ILaneLinkRoadType,
  direction: biz.ILaneDirection,
) {
  return `${roadId}_${sectionId}_${laneId}_${type}_${direction}`
}

/**
 * 通过 id 获取对应的 laneLink 数据
 * @param laneLinks
 * @param laneLinkId
 */
export function getLaneLinkById (
  laneLinks: Array<biz.ILaneLink>,
  laneLinkId: string,
) {
  const laneLink = laneLinks.find(l => l.id === laneLinkId)
  return laneLink
}
