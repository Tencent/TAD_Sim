import CoordinateTransform from '../libs/coordinate_transform.cpp.js'

class CoordConverter {
  constructor () {
    this.ct = new CoordinateTransform()
    this.refLon = 0.0
    this.refLat = 0.0
    this.refAlt = 0.0
    this.refSet = false

    this.useENU = true // enu flag
  }

  // refSet () {
  //   return this.refSet
  // }

  setRef (x, y, z) {
    this.refLon = x
    this.refLat = y
    this.refAlt = z
    this.refSet = true
  }

  lonlat2xyz (lon, lat, alt) {
    lon = +lon
    lat = +lat
    alt = +alt

    if (this.refSet === false) {
      this.setRef(lon, lat, alt)
    }

    const tmp = this.ct.ll_to_meter(lon, lat, alt)
    const tmp2 = this.ct.dx_to_zx(tmp[0], tmp[1], tmp[2], this.refLon, this.refLat, this.refAlt)

    if (this.useENU) {
      return this.seu2enu(tmp2)
    }
    return tmp2
  }

  xyz2lonlat (x, y, z) {
    if (this.useENU) { // x,y,z is in enu space
      [x, y, z] = this.enu2seu([x, y, z])
    }
    const tmp = this.ct.zx_to_dx((-1 * x), y, z, this.refLon, this.refLat, this.refAlt)
    const tmp2 = this.ct.meter_to_ll(tmp[0], tmp[1], tmp[2])

    return tmp2
  }

  seu2enu (seu) {
    const [seuX, seuY, seuZ] = seu
    const enuX = seuY
    const enuY = -seuX
    const enuZ = seuZ
    return [enuX, enuY, enuZ]
  }

  enu2seu (enu) {
    const [enuX, enuY, enuZ] = enu
    const seuX = -enuY
    const seuY = enuX
    const seuZ = enuZ
    return [seuX, seuY, seuZ]
  }

  /**
   * get sea height relative to origin
   */
  // getOriginHeight () {
  //   const pos = this.lonlat2xyz(this.refLon, this.refLat, 0)
  //   return pos ? pos[2] : 0
  // }
}

export default CoordConverter
