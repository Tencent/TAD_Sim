import { cloneDeep } from 'lodash-es'
import {
  BufferAttribute,
  BufferGeometry,
  CatmullRomCurve3,
  Group,
  Line,
  LineBasicMaterial,
  Mesh,
  MeshBasicMaterial,
  SphereGeometry,
  Vector3,
} from 'three'
import { Float32BufferAttribute } from 'three/src/core/BufferAttribute'
import { RenderOrder } from '../common/Constant'
import { MarkerTrack } from './MarkerTrack'

/**
 * 轨迹线类
 */
export class EditorCurve {
  constructor (simuScene, bWitAttr, boxSize, mat) {
    this.m_lineType = EditorCurve.StraightLine
    this.m_simuScene = simuScene
    this.m_group = new Group()
    this.m_lineGeom = new BufferGeometry()
    this.m_linePosition = new Float32Array(2 * 3)
    this.m_points = []
    this.m_boxes = []
    this.m_withAttr = !!bWitAttr
    this.m_mat = EditorCurve.PathMat
    if (mat) {
      this.m_mat = mat
    }

    if (boxSize) {
      this.m_boxSize = boxSize
    } else {
      this.m_boxSize = 0.5
    }

    if (this.m_lineType === EditorCurve.CurveLine) {
      this.m_controlLine = new CatmullRomCurve3(this.m_points)
      this.m_lineSamplePosition = new Float32Array(2 * 3)
      this.m_samplePointNumber = 50
    } else {
      this.m_controlLine = null
    }

    // initialize two points
    this.m_points.push(new Vector3(0, 0, 0))
    this.m_points.push(new Vector3(0, 0, 0))

    let box = this.createBox(0, 0, 0)
    this.m_boxes.push(box)
    this.m_group.add(box)
    box = this.createBox(0, 0, 0)
    this.m_boxes.push(box)
    this.m_group.add(box)

    // initialize center line
    this.m_linePosition = new Float32Array(2 * 3)
    for (let i = 0; i < 6; ++i) {
      this.m_linePosition[i] = 0
    }

    this.m_lineGeom.setAttribute('position', new BufferAttribute(this.m_linePosition, 3))
    this.m_lineGeom.setDrawRange(0, 2)
    const lineMat = this.m_mat
    this.m_centerLine = new Line(this.m_lineGeom, lineMat)
    this.m_centerLine.renderOrder = RenderOrder.ROAD_SIGN + 10
    this.m_group.add(this.m_centerLine)

    this.m_simuScene.scene.add(this.m_group)

    // logic data
    this.m_attributes = []
    if (this.m_withAttr) {
      this.m_attributes.push({ velocity: 6, gear: 'drive' })
      this.m_attributes.push({ velocity: 6, gear: 'drive' })
    }
  }

  dispose () {
    this.m_simuScene.scene.remove(this.m_group)
    if (this.track) {
      this.track.dispose()
      this.track = null
    }
  }

  SimuScene () {
    return this.m_simuScene
  }

  Boxes () {
    return this.m_boxes
  }

  BoxSize () {
    return this.m_boxSize
  }

  WithAttr () {
    return this.m_withAttr
  }

  FindBoxIdx (uuid) {
    return this.m_boxes.findIndex((value, idx, arr) => uuid === value.uuid)
  }

  /**
   * 创建一个点球，用于显示轨迹点
   * @param x
   * @param y
   * @param z
   * @return {Mesh}
   */
  createBox (x, y, z) {
    const boxGeom = new SphereGeometry(this.m_boxSize, 8, 4)
    const box = new Mesh(boxGeom, EditorCurve.pointMat)
    box.position.set(x, y, z)
    box.rotateX(1.57)
    box.renderOrder = RenderOrder.OBJECTS + 1
    return box
  }

  /**
   * 轨迹线
   * @return {Line}
   */
  centerLine () {
    return this.m_centerLine
  }

  Points () {
    return this.m_points
  }

  Attributes () {
    return this.m_attributes
  }

  /**
   * 添加一个点
   * @param point
   * @param updateModel
   * @param attr
   */
  addPoint (point, updateModel, attr) {
    this.insertPoint(this.m_points.length, point, updateModel, attr)
  }

  /**
   * 复制最后一点，并合并参数的属性添加到末尾
   * @param attr
   */
  addOnePoint (attr) {
    const point = this.m_points[this.m_points.length - 1].clone()
    this.addPoint(point, true, attr)
  }

  /**
   * 获取采样点索引
   * @param idx
   * @return {number|*}
   */
  getIndexFromSampleIdx (idx) {
    if (this.m_lineType === EditorCurve.StraightLine) {
      return idx
    } else {
      if (this.m_samplePointNumber < this.m_points.length) {
        return idx
      } else {
        const delta = this.m_samplePointNumber / this.m_points.length
        return Math.floor(idx / delta)
      }
    }
  }

  /**
   * 在指定位置插入一个点
   * @param idx
   * @param point
   * @param updateModel
   * @param attr
   */
  insertPoint (idx, point, updateModel, attr) {
    const p = new Vector3()
    p.copy(point)
    this.m_points.splice(idx, 0, p)

    const box = this.createBox(p.x, p.y, p.z)
    this.m_group.add(box)
    this.m_boxes.splice(idx, 0, box)
    if (this.WithAttr()) {
      if (!attr) {
        // 没有传递attr但这条线又包含时，自动根据临近点生成一个
        const attrs = this.Attributes()
        const sourceAttr = attrs[idx - 1] || attrs[idx]
        attr = cloneDeep(sourceAttr)
      }
      if (attr) {
        this.insertAttribute(idx, attr)
      }
    }
    if (updateModel) {
      this.updateModels()
    }
  }

  /**
   * 插入一个属性
   * @param idx
   * @param attr
   */
  insertAttribute (idx, attr) {
    this.m_attributes.splice(idx, 0, attr)
  }

  /**
   * 删除一个点
   * @param idx
   */
  removePoint (idx) {
    if (idx >= this.m_points.length || idx < 0) {
      console.error(`removePoint idx: ${idx} is error!`)
      return
    }

    try {
      // const uuid = this.m_boxes[idx].uuid
      this.m_group.remove(this.m_boxes[idx])
      this.m_points.splice(idx, 1)
      this.m_boxes.splice(idx, 1)

      if (this.m_attributes.length > 0) {
        this.removeAttribute(idx)
      }

      this.updateModels()
      // todo: 什么破玩意儿
      // this.m_simuScene.sceneParser.removeVehicleEndModelByUUID(uuid)
    } catch (error) {
      console.warn(error)
    }
  }

  /**
   * 删除一个属性
   * @param idx
   */
  removeAttribute (idx) {
    if (idx >= this.m_attributes.length || idx < 0) {
      console.error(`removeAttribute idx: ${idx} is error!`)
      return
    }

    this.m_attributes.splice(idx, 1)
  }

  /**
   * 修改一个点
   * @param idx
   * @param vec
   * @param updateModel
   * @param attr
   */
  modifyPoint (idx, vec, updateModel, attr) {
    if (idx >= this.m_points.length || idx < 0) {
      console.error(`modifyPoint idx: ${idx} is error!`)
      return
    }

    this.m_points[idx].copy(vec)
    this.m_boxes[idx].position.copy(vec)

    if (updateModel) {
      this.updateModels()
    }

    if (attr !== undefined) {
      this.modifyAttribute(idx, attr)
    }
  }

  /**
   * 修改一个属性
   * @param idx
   * @param attr
   */
  modifyAttribute (idx, attr) {
    if (idx >= this.m_attributes.length || idx < 0) {
      console.error(`modifyAttribute idx: ${idx} is error!`)
      return
    }

    this.m_attributes[idx] = attr
  }

  /**
   * 重新采样更新模型
   */
  updateModels () {
    if (this.m_lineType === EditorCurve.CurveLine) {
      this.m_controlLine.points = this.m_points

      // display line
      const ps = this.m_controlLine.getPoints(this.m_samplePointNumber)
      let position = this.m_linePosition

      if (position.length !== (ps.length * 3)) {
        this.m_linePosition = new Float32Array(ps.length * 3)
        position = this.m_linePosition
      }

      for (let i = 0; i < ps.length; ++i) {
        position[3 * i] = ps[i].x
        position[3 * i + 1] = ps[i].y
        position[3 * i + 2] = ps[i].z + 0.2
      }

      const bg = this.m_centerLine.geometry
      bg.setAttribute('position', new Float32BufferAttribute(this.m_linePosition, 3))

      this.m_lineGeom.setDrawRange(0, ps.length)
      this.m_centerLine.geometry.computeBoundingSphere()
    } else {
      let position = this.m_linePosition

      if (position.length !== (this.m_points.length * 3)) {
        this.m_linePosition = new Float32Array(this.m_points.length * 3)
        position = this.m_linePosition
      }

      for (let i = 0; i < this.m_points.length; ++i) {
        position[3 * i] = this.m_points[i].x
        position[3 * i + 1] = this.m_points[i].y
        position[3 * i + 2] = this.m_points[i].z + 0.2
      }

      const bg = this.m_centerLine.geometry
      bg.setAttribute('position', new Float32BufferAttribute(this.m_linePosition, 3))

      this.m_lineGeom.setDrawRange(0, this.m_points.length)
      this.m_centerLine.geometry.computeBoundingSphere()
    }

    if (this.track) {
      this.track.fromPoints(this.Points(), this.Attributes())
    }
  }

  /**
   * 获取属性
   * @param index
   * @return {*}
   */
  getAttributeByIndex (index) {
    return this.m_attributes[index]
  }

  /**
   * 从轨迹点更新属性
   * @param posArr
   */
  updateAttrFromEndPositions (posArr) {
    try {
      if (posArr.length !== this.m_attributes.length) {
        console.warn('posArr length not equals ', posArr, this.m_attributes)
      }

      for (let i = 0; i < this.m_attributes.length; i++) {
        const { velocity, gear } = posArr[i]
        this.m_attributes[i].velocity = velocity
        this.m_attributes[i].gear = gear
      }
    } catch (error) {
      console.log(error)
    }
  }

  /**
   * 复制属性
   * @param other
   */
  copyAttributes (other) {
    try {
      this.m_attributes.splice(0)
      for (let i = 0; i < other.Attributes().length; i++) {
        const attr = Object.assign({}, other.Attributes()[i])
        this.m_attributes.push(attr)
      }
    } catch (error) {
      console.warn(error)
    }
  }

  /**
   * 设置轨迹是否显示
   * @param enabled
   */
  setTrackEnabled (enabled) {
    if (enabled) {
      if (!this.track) {
        this.track = new MarkerTrack(this.m_simuScene.scene)
      }
      this.track.setEnabled(true)
      this.track.fromPoints(this.Points(), this.Attributes())
      this.track.visible = true
      this.m_centerLine.visible = false
    } else {
      if (this.track) {
        this.track.setEnabled(false)
        this.track.visible = false
      }
      this.m_centerLine.visible = true
    }
  }
}

EditorCurve.MeasurementMat = new LineBasicMaterial({ color: 0xFA6400, linewidth: 5 })
EditorCurve.PathMat = new LineBasicMaterial({
  color: 0xFFFF00,
  linewidth: 5,
  polygonOffset: true,
  polygonOffsetUnits: 1,
  polygonOffsetFactor: 1,
})
EditorCurve.pointMat = new MeshBasicMaterial({
  color: 0xFFFF00,
})
EditorCurve.StraightLine = 0
EditorCurve.CurveLine = 1
