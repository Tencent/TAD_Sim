/**
 * 测量类
 */
class Measurement {
  constructor () {
    this.id = 0
    this.posArr = []
  }

  /**
   * 复制一个测量类数据
   * @param other
   */
  copy (other) {
    this.id = other.id
    this.posArr = other.posArr.map(p => ({ ...p }))
    this.curve = other.curve
  }

  /**
   * 更新测量点坐标
   * @param ct
   */
  updatePointsFromCurve (ct) {
    this.posArr.splice(0)

    if (!this.curve) {
      return
    }

    this.curve.Points().forEach((value) => {
      const tmp = ct.xyz2lonlat(value.x, value.y, value.z)
      const p = {}
      p.lon = tmp[0].toFixed(8)
      p.lat = tmp[1].toFixed(8)
      p.alt = tmp[2].toFixed(3)
      this.posArr.push(p)
    }, this)
  }
}

export default Measurement
