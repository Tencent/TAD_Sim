import { MarkerSpline } from './MarkerSpline'
import i18n from '@/locales'

/**
 * 轨迹线类
 */
export class MarkerTrack {
  constructor (scene) {
    this.scene = scene
    this.splines = []
    this.enabled = false
  }

  dispose () {
    this.splines.forEach((value) => {
      value.dispose()
    })

    this.splines.splice(0, this.splines.length)
  }

  setEnabled (enabled) {
    this.enabled = enabled
  }

  set visible (value) {
    this.splines.forEach((item) => {
      item.visible = value
    })
  }

  /**
   * 根据提供的点重新计算轨迹线
   * @param points
   * @param attrs
   */
  fromPoints (points, attrs) {
    if (!this.enabled) {
      return
    }

    if (points.length < 1 || attrs.length < 1) {
      this.splines.forEach((value) => {
        value.dispose()
      })
      this.splines.splice(0)
      return
    }

    const lines = []
    const linesAttrs = []
    const line = []
    const lineAttr = []
    lines.push(line)
    linesAttrs.push(lineAttr)

    for (let i = 0; i < attrs.length; ++i) {
      line.push(points[i])
      lineAttr.push(attrs[i])
    }

    if (this.splines.length < lines.length) {
      for (let i = this.splines.length; i < lines.length; ++i) {
        const centerLine = new MarkerSpline(this.scene)
        this.splines.push(centerLine)
      }
    } else if (this.splines.length > lines.length) {
      for (let i = lines.length; i < this.splines.length; ++i) {
        this.splines[i].dispose()
      }
      const count = this.splines.length - lines.length
      this.splines.splice(lines.length, count)
    }

    for (let i = 0; i < lines.length; ++i) {
      const centerLine = this.splines[i]
      centerLine.fromPoints(lines[i])
      centerLine.setAttributes(linesAttrs[i])
    }
  }

  /**
   * 检查轨迹点
   * @param posarr
   * @param minA
   * @param maxA
   * @return {{result: boolean, message: TranslateResult}}
   */
  check (posarr, minA, maxA) {
    if (!this.enabled) {
      return
    }

    const rets = []
    this.splines.forEach((value, idx) => {
      const ret = value.check(posarr, minA, maxA)
      rets.push(ret)
    })

    const ret = { result: true, message: i18n.t('tips.success') }
    let msg = ''
    rets.forEach((value, idx) => {
      if (value.result == false) {
        msg += `${i18n.t('tips.afterTimesOfShift', { n: idx })}, ${value.message}`
        ret.result = false
      }
    })

    if (ret.result === false) {
      ret.message = msg
    }

    return ret
  }

  /**
   * 根据采样间隔生成轨迹点
   * @param sampleInterval
   * @param ct
   * @return {*[]}
   */
  generateTrackPointByInterval (sampleInterval, ct) {
    if (!this.enabled) {
      return
    }

    const NODELAY = 0
    const DELAYTOSTART = 2

    const pss = []
    let lengthOffset = 0
    let timeOffset = 0
    let delayToStart = DELAYTOSTART
    this.splines.forEach((value, idx) => {
      if (idx === 0) {
        delayToStart = NODELAY
      } else {
        delayToStart = DELAYTOSTART
      }

      const ps = value.generateTrackPointByInterval(sampleInterval, ct, lengthOffset, timeOffset, delayToStart)
      if (ps.length > 0) {
        pss.push(ps)
        lengthOffset = ps[ps.length - 1].totalLen
        timeOffset = ps[ps.length - 1].t
      }
    })

    return [].concat(...pss)
  }
}
