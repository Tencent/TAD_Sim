import {
  BufferAttribute,
  BufferGeometry,
  CatmullRomCurve3,
  Line,
  LineDashedMaterial,
} from 'three'
import Utility from './Utility'
import { MessageBox } from './MessageBox'
import i18n from '@/locales'

/**
 * @class MarkerSpline
 * @description 轨迹跟踪的轨迹线
 */
export class MarkerSpline {
  constructor (scene) {
    this.m_attrs = []
    this.m_points = []
    // fixme: 这个线型有诸多问题，需要优化
    this.m_controlLine = new CatmullRomCurve3(this.m_points, false, 'centripetal', 0.5)

    this.m_scene = scene

    this.m_linePosition = new Float32Array(2 * 3)
    for (let i = 0; i < 6; ++i) {
      this.m_linePosition[i] = 0
    }

    this.m_geometry = new BufferGeometry()
    this.m_geometry.setAttribute('position', new BufferAttribute(this.m_linePosition, 3))
    this.m_geometry.setDrawRange(0, 2)
    this.m_centerLine = new Line(this.m_geometry, MarkerSpline.brokenMat)
  }

  set visible (value) {
    this.m_centerLine.visible = !!value
  }

  get visible () {
    return this.m_centerLine.visible
  }

  dispose () {
    this.reset()
    this.m_geometry.dispose()
    this.m_geometry = null
    this.m_controlLine = null
  }

  reset () {
    if (this.m_attrs.length > 0) {
      this.m_attrs.splice(0, this.m_attrs.length)
    }
    if (this.m_points.length > 0) {
      this.m_points.splice(0, this.m_points.length)
    }
    if (this.m_centerLine != null) {
      this.m_scene.remove(this.m_centerLine)
    }
  }

  setAttributes (attrs) {
    if (this.m_attrs.length > 0) {
      this.m_attrs.splice(0, this.m_attrs.length)
    }

    attrs.forEach((value) => {
      const attr = {}
      attr.velocity = value.velocity
      attr.gear = value.gear
      this.m_attrs.push(attr)
    }, this)
  }

  check (posarr, minA, maxA) {
    const ret = { result: true, message: i18n.t('tips.success') }
    // 1, 计算每两个点之间的距离及总距离
    const distances = []
    let sum = 0
    for (let i = 1; i < this.m_points.length; ++i) {
      const d = this.m_points[i].distanceTo(this.m_points[i - 1])
      distances.push(d)
      sum += d
    }

    // 总行程大于0
    if (sum > 0) {
      // 2, ------计算每个关键点的比值及索引
      const factors = []
      let accumF = 0
      factors.push(accumF)
      distances.forEach((value) => {
        const f = value / sum
        accumF = f + accumF
        factors.push(accumF)
      })
      // 3, -----计算加速度
      // 获得曲线长度
      const curveLength = this.m_controlLine.getLength()
      for (let i = 1; i < factors.length; ++i) {
        const shift = curveLength * (factors[i] - factors[i - 1])
        const v0 = posarr[i - 1].velocity
        const v1 = posarr[i].velocity
        // 两端点速度均为0
        if (Utility.equalToZero(v0) && Utility.equalToZero(v1)) {
          ret.result = false
          ret.message = i18n.t('tips.thereAreSpeed', { s: '0' })
          return
        } else {
          const deltaT = shift / (0.5 * v0 + 0.5 * v1)
          if (deltaT < 0) {
            ret.result = false
            ret.message = i18n.t('tips.tooSlow')
          } else if (Utility.equalToZero(deltaT)) {
          } else if (deltaT > 0) {
            const a = (v1 - v0) / deltaT
            if (a < minA || a > maxA) {
              ret.result = false
              ret.messge = i18n.t('tips.accCannotMet', { n: i })
            }
          }
        }
      }
    }

    return ret
  }

  generateTrackPointByInterval (sampleInterval, converter, lengthOffset, timeOffset, delatToStart) {
    if (sampleInterval < 0.009) {
      const msg = i18n.t('tips.samplingSmall')
      MessageBox.promptEditorUIMessage('warn', msg)
      return
    }

    // 1, 计算每两个点之间的距离及总距离
    const distances = []
    let sum = 0
    for (let i = 1; i < this.m_points.length; ++i) {
      const d = this.m_points[i].distanceTo(this.m_points[i - 1])
      distances.push(d)
      sum += d
    }

    // 总行程大于0
    if (sum > 0) {
      // 获得曲线长度
      const curveLength = this.m_controlLine.getLength()
      // 采样间隔比值
      const t = sampleInterval / curveLength
      // 离散点数量
      const count = Math.ceil(curveLength / sampleInterval)

      // 2, ------计算每个关键点的比值及索引
      const factors = []
      const keyIndex = []
      let accumF = 0
      factors.push(0)
      keyIndex.push(0)
      distances.forEach((value) => {
        const f = value / sum
        accumF = f + accumF
        factors.push(accumF)

        const idx = Math.ceil(accumF / t)
        keyIndex.push(idx)
      })

      const ps = []
      // 3, -----获取每个点的位置并计算每个点的切线值
      // 前count个
      for (let i = 0; i < count; ++i) {
        const p = this.m_controlLine.getPoint(t * i)
        const dir = this.m_controlLine.getTangent(t * i)
        if (Utility.equalToZero(dir.x)) {
          p.theta = 0
          console.warn('control path theta calculate eror!')
        } else {
          p.theta = Math.atan2(dir.y, dir.x)
        }
        ps.push(p)
      }
      // 第count+1个
      ps.push(this.m_controlLine.getPoint(1))
      const dir = this.m_controlLine.getTangent(1)
      let theta = 0
      if (!Utility.equalToZero(dir.x)) {
        theta = Math.atan2(dir.y, dir.x)
      } else {
        console.error('control path theta calculate eror!')
      }
      ps[count].theta = theta

      // 4, ------计算速度和挡位
      for (let i = 1; i < keyIndex.length; ++i) {
        const start = keyIndex[i - 1]
        const end = keyIndex[i]
        const segCount = end - start
        let deltaVelocity = (this.m_attrs[i].velocity - this.m_attrs[i - 1].velocity)
        deltaVelocity = deltaVelocity / segCount
        const curGear = this.m_attrs[i].gear
        for (let j = start, m = 0; j <= end; j++, m++) {
          ps[j].velocity = this.m_attrs[i - 1].velocity + m * deltaVelocity
          ps[j].gear = curGear
        }
      }

      // 5， -------计算总时间和总长度
      ps[0].t = timeOffset
      ps[0].totalLen = lengthOffset
      for (let i = 1; i < ps.length; ++i) {
        const deltaD = ps[i].distanceTo(ps[i - 1])
        let deltaT = 0
        // 速度为0
        if (Utility.equalToZero(ps[i - 1].velocity)) {
          // 速度为0而位移不为0
          if (!Utility.equalToZero(deltaD)) {
            console.error('velocity is 0 but shift is not 0')
          }
        } else {
          deltaT = deltaD / ps[i - 1].velocity
        }

        ps[i].totalLen = ps[i - 1].totalLen + deltaD
        ps[i].t = ps[i - 1].t + deltaT
      }

      // 6, --------将位置坐标转换为经纬度并计算加速度
      for (let i = 0; i < ps.length; ++i) {
        const tmp = converter.xyz2lonlat(ps[i].x, ps[i].y, ps[i].z)
        ps[i].x = tmp[0]
        ps[i].y = tmp[1]
        ps[i].z = tmp[2]
        if (i < ps.length - 1) {
          const deltaT = ps[i + 1].t - ps[i].t
          if (!Utility.equalToZero(deltaT)) {
            const deltaV = ps[i + 1].velocity - ps[i].velocity
            ps[i].a = deltaV / deltaT
          } else {
            ps[i].a = 0
            console.error('control path acceleration calculate eror!')
          }
        }
      }
      ps[ps.length - 1].a = ps[ps.length - 2].a

      // 7, -----整体进行时间偏移
      if (delatToStart > 0) {
        const first = Object.assign({}, ps[0])
        for (let i = 0; i < ps.length; ++i) {
          ps[i].t += delatToStart
        }
        ps.unshift(first)
      }

      return ps
    }

    return []
  }

  fromPoints (points) {
    this.reset()

    const that = this
    points.forEach((value, idx, arr) => {
      that.m_points.push(value.clone())
    })

    if (this.m_points.length > 1) {
      // display line
      this.m_controlLine.updateArcLengths()
      const length = this.m_controlLine.getLength()
      const samplePointNumber = Math.max(length / 0.5, 2)
      const ps = this.m_controlLine.getPoints(samplePointNumber)
      let position = this.m_linePosition

      if (position.length !== (ps.length * 3)) {
        this.m_linePosition = new Float32Array(ps.length * 3)
        position = this.m_linePosition
      }

      for (let i = 0; i < ps.length; ++i) {
        position[3 * i] = ps[i].x
        position[3 * i + 1] = ps[i].y
        position[3 * i + 2] = ps[i].z
      }

      const bg = this.m_centerLine.geometry
      bg.setAttribute('position', new BufferAttribute(this.m_linePosition, 3))
      bg.attributes.position.needsUpdate = true
      this.m_geometry.setDrawRange(0, ps.length)
      this.m_centerLine.geometry.computeBoundingSphere()
      this.m_scene.add(this.m_centerLine)
    }
  }
}

MarkerSpline.brokenMat = new LineDashedMaterial({
  color: 0xFF0000,
  dashSize: 4,
  gapSize: 2,
})
