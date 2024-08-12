import { Vector3 } from 'three'
import { EditorCurve } from './EditorCurve'
import Vehicle from './Vehicle'

// 路径数据类
class RouteData {
  constructor () {
    this.id = 0
    this.type = 0
    this.remarks = ''

    this.startLon = 0.0
    this.startLat = 0.0
    this.midLon = -999.0
    this.midLat = -999.0
    this.endLon = -999.0
    this.endLat = -999.0

    this.roadId = 0
    this.sectionId = 0
    this.laneId = 0
    this.startRPM = undefined
    this.midRPM = undefined
    this.endRPM = undefined
    this.mids = undefined

    this.endPosArr = [] // 路径上所有的点

    this.startAlt = 0
    this.endAlt = 0

    this.controlPath = null //

    this.host = null // 关联的主体
  }

  /**
   * 拷贝除了模型以外的数据
   * @param other
   */
  copyNoModel (other) {
    this.id = other.id
    this.type = other.type
    this.remarks = other.remarks

    this.startLon = other.startLon
    this.startLat = other.startLat
    this.realStartLon = other.realStartLon
    this.realStartLat = other.realStartLat
    this.realStartAlt = other.realStartAlt
    this.midLon = other.midLon
    this.midLat = other.midLat
    this.endLon = other.endLon
    this.endLat = other.endLat

    this.roadId = other.roadId
    this.sectionId = other.sectionId
    this.laneId = other.laneId
    this.mids = other.mids

    this.endPosArr = other.endPosArr.map(value => ({ ...value }))
  }

  /**
   * 拷贝除了ID以外的数据
   * @param other
   * @param ct
   */
  copyNoID (other, ct) {
    const id = this.id
    this.copyNoModel(other)
    this.id = id

    if (other.curve) {
      if (!this.curve) {
        this.curve = new EditorCurve(other.curve.SimuScene(), other.curve.WithAttr(), other.curve.BoxSize())
      }

      this.updateCurveFromSelf(ct)

      this.curve.copyAttributes(other.curve)
    } else {
      if (this.curve) {
        this.curve.dispose()
        this.curve = null
      }
    }
  }

  /**
   * 设置模型可见性
   * @param show
   */
  setVisible (show) {
    this.startRPM?.setVisible(show)
    this.midRPM?.setVisible(show)
    this.endRPM?.setVisible(show)
  }

  /**
   * 更新模型位置
   * @param ct
   */
  updateRouteModel (ct) {
    if (ct) {
      if (this.startRPM) {
        const tmp = ct.lonlat2xyz(this.startLon, this.startLat, 0)
        this.startRPM.setPos(tmp[0], tmp[1], 0)
      }
      if (this.midRPM) {
        const tmp = ct.lonlat2xyz(this.midLon, this.midLat, 0)
        this.midRPM.setPos(tmp[0], tmp[1], 0)
      }
      if (this.endRPM) {
        const tmp = ct.lonlat2xyz(this.endLon, this.endLat, 0)
        this.endRPM.setPos(tmp[0], tmp[1], 0)
      }
    }
  }

  /**
   * 修改路径上的点
   * @param idx
   * @param point
   * @param bUpdateCoord
   * @param ct
   */
  modifyOnePoint (idx, point, bUpdateCoord, ct) {
    if (!this.curve) {
      return
    }

    this.curve.modifyPoint(idx, point, true)
    if (bUpdateCoord) {
      if (idx === this.curve.Points().length) {
        const tmp = ct.xyz2lonlat(point.x, point.y, 0)
        this.endLon = tmp[0].toFixed(8)
        this.endLat = tmp[1].toFixed(8)
      }
    }
  }

  /**
   * 重新采样并更新模型
   * @param ct
   */
  updateCurveFromSelf (ct) {
    if (!this.curve) {
      return
    }

    const len = this.endPosArr.length

    let tmp
    if (this.realStartLon) {
      tmp = ct.lonlat2xyz(+this.realStartLon, +this.realStartLat, +this.realStartAlt || 0)
    } else {
      tmp = ct.lonlat2xyz(+this.endPosArr[0].lon, +this.endPosArr[0].lat, +this.endPosArr[0].alt)
    }
    this.curve.modifyPoint(0, new Vector3(tmp[0], tmp[1], tmp[2]), false)

    const boxLen = this.curve.Boxes().length
    if (boxLen < len) {
      for (let i = 1; i < boxLen; ++i) {
        tmp = ct.lonlat2xyz(this.endPosArr[i].lon, this.endPosArr[i].lat, this.endPosArr[i].alt)
        this.curve.modifyPoint(i, new Vector3(tmp[0], tmp[1], tmp[2]), false)
      }
      for (let i = boxLen; i < len; ++i) {
        tmp = ct.lonlat2xyz(this.endPosArr[i].lon, this.endPosArr[i].lat, this.endPosArr[i].alt)
        this.curve.addPoint(new Vector3(tmp[0], tmp[1], tmp[2]), false)
      }
    } else {
      for (let i = 1; i < len; ++i) {
        tmp = ct.lonlat2xyz(this.endPosArr[i].lon, this.endPosArr[i].lat, this.endPosArr[i].alt)
        this.curve.modifyPoint(i, new Vector3(tmp[0], tmp[1], tmp[2]), false)
      }
      for (let i = len; i < boxLen; ++i) {
        const l = this.curve.Boxes().length
        this.curve.removePoint(l - 1)
      }
    }

    this.curve.updateModels()
  }

  /**
   * 根据所有点重新计算路径
   * @param ct
   */
  updateRoutePointsFromCurve (ct) {
    this.endPosArr.splice(0)

    this.mids = undefined
    this.midLon = -999.0
    this.midLat = -999.0
    this.endLon = -999.0
    this.endLat = -999.0

    if (!this.curve) {
      return
    }

    this.curve.Points().forEach((value, index) => {
      const attr = {
        velocity: Vehicle.StartSpeed,
        gear: 'drive',
      }
      const curveAttr = this.curve.getAttributeByIndex(index)
      if (curveAttr) {
        Object.assign(attr, curveAttr)
      } else {
        if (this.host) {
          attr.velocity = this.host.startVelocity
        }
      }

      const tmp = ct.xyz2lonlat(value.x, value.y, value.z) // fix height
      const p = {}
      p.lon = tmp[0].toFixed(8)
      p.lat = tmp[1].toFixed(8)
      p.alt = tmp[2].toFixed(3)
      p.velocity = attr.velocity
      p.gear = attr.gear
      this.endPosArr.push(p)
    })

    const len = this.endPosArr.length

    if (len > 0) {
      this.realStartLon = this.endPosArr[0].lon
      this.realStartLat = this.endPosArr[0].lat
      this.realStartAlt = this.endPosArr[0].alt || 0
    }

    if (len > 1) {
      this.endLon = this.endPosArr[len - 1].lon
      this.endLat = this.endPosArr[len - 1].lat
      this.endAlt = this.endPosArr[len - 1].alt || 0
    }

    if (len > 2) {
      this.mids = `${this.endPosArr[1].lon},${this.endPosArr[1].lat},${this.endPosArr[1].alt || 0}` // add height info
      for (let i = 2; i < len - 1; ++i) {
        this.mids += (`;${this.endPosArr[i].lon},${this.endPosArr[i].lat},${this.endPosArr[i].alt || 0}`) // add height info
      }
    }
  }

  // 最先解析到 RouteData 里面, 然后保存到 EditorCurve 的 attributes 里面;
  addControlPathInfo (controlPath) {
    if (controlPath && this.endPosArr) {
      for (let i = 0; i < this.endPosArr.length; i++) {
        const { speed_m_s: speedMS = Vehicle.StartSpeed, gear = 'drive' } = controlPath[i]
        this.endPosArr[i].velocity = speedMS
        this.endPosArr[i].gear = gear
      }
    }
  }

  /**
   * 重新设置轨迹点
   * @param posArr
   */
  updateEndPositions (posArr) {
    try {
      if (posArr.length !== this.endPosArr.length) {
        console.warn('posArr length not equals', posArr, this.endPosArr)
      }

      for (let i = 0; i < this.endPosArr.length; i++) {
        Object.assign(this.endPosArr[i], posArr[i])
      }

      if (this.curve) {
        this.curve.updateAttrFromEndPositions(posArr)
      }
    } catch (error) {
      console.warn(error)
    }
  }

  // 最后保存时使用
  generateControlPath () {
    this.controlPath = this.endPosArr.map((item) => {
      const { velocity, gear, lon, lat, alt } = item
      return {
        speed_m_s: `${velocity}`,
        gear,
        lon,
        lat,
        alt,
      }
    })
    return this.controlPath
  }

  /**
   * 从后端数据中设置数据
   * @param data
   */
  setFromBackEndData (data) {
    this.id = data.id
    this.type = data.type
    if (data.info) {
      this.remarks = data.info
    }
    // start_end
    if (data.type === 'start_end') {
      // start
      const [startLon, startLat] = data.start.split(',')
      this.startLon = startLon
      this.startLat = startLat

      this.endPosArr.push({
        lon: startLon,
        lat: startLat,
        alt: 0,
      })

      // mid
      if (data.mid?.length) {
        const [lon, lat] = data.mid.split(',').map(item => Number.parseFloat(item))
        this.midLon = lon
        this.midLat = lat
      }

      // mids endPosArr
      if (data.mids?.length) {
        data.mids.split(';')
          .forEach((value) => {
            const [
              lon,
              lat,
              alt,
            ] = value.split(',').map(item => Number.parseFloat(item))
            this.endPosArr.push({
              lon,
              lat,
              alt: alt || 0, // add height info
            })
          })
      }

      if (data.end?.length) {
        const [endLon, endLat, endAlt] = data.end.split(',').map(item => Number.parseFloat(item)) // fix err
        this.endLon = endLon
        this.endLat = endLat
        this.endAlt = endAlt || 0 // 没有则为0
        this.endPosArr.push({
          lon: endLon,
          lat: endLat,
          alt: this.endAlt, // fix height
        })
      }

      // control path
      if (data.controlPath) {
        this.addControlPathInfo(data.controlPath)
      }

      // road id
    } else if (data.type === 'roadID') {
      this.roadId = data.roadID
      this.sectionId = data.sectionID
    } else {
      console.error('this type error!')
    }
  }
}

export default RouteData
