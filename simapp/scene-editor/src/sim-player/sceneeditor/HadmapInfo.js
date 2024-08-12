/**
 * 地图信息类，id，名称，参考经纬度，参考海拔，地图等级等
 */
class HadmapInfo {
  constructor () {
    this.mapId = -1
    this.mapName = ''
    this.mapRefLon = 0.0
    this.mapRefLat = 0.0
    this.mapRefAlt = 0.0
    this.unrealLevelIndex = 0
  }
}

export default HadmapInfo
