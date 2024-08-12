class MapFile {
  constructor () {
    this.lon = ''
    this.lat = ''
    this.alt = ''
    this.unrealLevelIndex = 0
    this.mapName = ''
  }

  /**
   * 设置数据
   * @param {string} [data.lon]
   * @param {string} [data.lat]
   * @param {string} [data.alt]
   * @param {number} [data.unrealLevelIndex]
   * @param {string} [data.mapName]
   */
  setData (data) {
    Object.assign(this, data)
  }
}

export default MapFile
