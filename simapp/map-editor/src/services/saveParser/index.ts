// save 阶段数据处理过程
import { cloneDeep, pick } from 'lodash'
import { MathUtils, Vector2, Vector3 } from 'three'
import { pickUpObjectData } from './object'
import { useJunctionStore } from '@/stores/junction'
import { useRoadStore } from '@/stores/road'
import {
  MapVersion,
  getMatchingSegment,
  getParallelCurveSamplePoints,
} from '@/utils/business'
import {
  addCatmullromVirtualPoint,
  axisY,
  fixedPrecision,
  getClosestPointFromCurvePath,
  getPointInfoOnCurve,
  getValidRadInRange,
  getWorldAngle,
  halfPI,
  sortById,
  transformCoordinateToCommon,
  transformOneCoordinateToCommon,
} from '@/utils/common3d'
import { getJunction, getRoad } from '@/utils/mapCache'
import { useEditRoadStore } from '@/plugins/editRoad/store'
import { useFileStore } from '@/stores/file'

// 提取地图要保存元素的数据
export async function pickUpMapElementData (): Promise<biz.ICommonMapElements> {
  const roads = pickUpRoadData()
  const { laneLinks } = pickUpJunctionData()
  const objects = await pickUpObjectData()

  const allMapData = {
    roads,
    lanelinks: laneLinks,
    objects,
  }

  return allMapData
}

// 提取道路 road 级别的数据
function pickUpRoadData (): Array<biz.ICommonRoad> {
  const roadStore = useRoadStore()
  const editRoadStore = useEditRoadStore()
  const fileStore = useFileStore()
  const { mapVersion } = fileStore

  const _roads: Array<biz.ICommonRoad> = []
  for (const roadId of roadStore.ids) {
    const road = getRoad(roadId)
    if (!road) continue
    const { keyPath, sections, length: roadLength } = road

    // TODO 解析高程控制点

    const _road: biz.ICommonRoad = {
      id: roadId,
      length: roadLength,
      type: 0,
      samplePoints: [],
      sections: [],
    }

    // 根据 section 的划分，获取道路中心线的采样点
    const roadSamplePoints: Array<common.vec3> = []
    // 道路中心线采样点的索引偏移
    let index: number = -1
    const _sections: Array<biz.ICommonSection> = []
    // 对 section 先按照 id 顺序依次排序
    sections.sort(sortById)

    for (const section of sections) {
      const _lanes: Array<biz.ICommonLane> = []
      const _boundarys: Array<biz.ICommonLaneBoundary> = []
      const {
        id: sectionId,
        pStart,
        pEnd,
        lanes,
        boundarys,
        length: sectionLength,
      } = section
      // 获取当前 section 的采样间隔
      const segment = getMatchingSegment(section)
      const { refPoints: roadInSectionSamplePoints } =
        getParallelCurveSamplePoints({
          keyPath,
          offset: 0,
          pStart,
          pEnd,
          segment,
        })
      // 对道路中心线采样点做坐标转换
      roadSamplePoints.push(
        ...transformCoordinateToCommon(roadInSectionSamplePoints),
      )

      // 索引的手动指定
      const startIndex = index + 1
      const endIndex = index + segment + 1
      index = endIndex

      // 处理 section - lanes 数据
      section.lanes.sort(sortById)
      for (const lane of lanes) {
        const { id: laneId, samplePoints } = lane
        // 对车道中心线采样点做坐标转换
        const _samplePoints = transformCoordinateToCommon(samplePoints)
        // 将采样点坐标进行转换
        const _lane: biz.ICommonLane = {
          id: laneId,
          samplePoints: _samplePoints,
          ...pick(lane, [
            'lbid',
            'rbid',
            'speedlimit',
            'type',
            'friction',
            'sOffset',
          ]),
        }
        _lanes.push(_lane)
      }
      // 处理 section - boundarys 数据
      for (const boundary of boundarys) {
        const { id: boundaryId, samplePoints } = boundary
        // 对车道边界线采样点做坐标转换
        const _samplePoints = transformCoordinateToCommon(samplePoints)
        const _boundary: biz.ICommonLaneBoundary = {
          id: boundaryId,
          samplePoints: _samplePoints,
          ...pick(boundary, ['mark']),
        }
        _boundarys.push(_boundary)
      }

      const _section: biz.ICommonSection = {
        id: sectionId,
        length: sectionLength,
        startPercent: pStart,
        endPercent: pEnd,
        start: startIndex,
        end: endIndex,
        lanes: [],
        boundarys: [],
      }

      _section.lanes = _lanes
      _section.boundarys = _boundarys

      _sections.push(_section)
    }

    // 第三方地图和旧版编辑器生产的地图，不保存控制点，其他的地图都需要保存控制点
    if (mapVersion !== MapVersion.v1 && mapVersion !== MapVersion.third) {
      // 计算道路的控制点透传给后端用于做曲率优化
      const controlPoints: Array<biz.ICommonControlPoint> = []
      let controlType: biz.IControlType = 'catmullrom'
      if (road.isCircleRoad && road.circleOption) {
        // 如果是圆弧道路（环形道路模式下创建的道路），取首尾两个端点作为控制点即可
        const { center, isClockwise = false } = road.circleOption

        // 是否属于双向道路的反向道路
        const isReverseRoad = false

        // 首尾端点
        const pHead = keyPath.getPointAt(0)
        const _pHead = transformOneCoordinateToCommon(pHead)
        const pTail = keyPath.getPointAt(1)
        const _pTail = transformOneCoordinateToCommon(pTail)

        // 圆弧计算首和尾的切线方向，需要借助圆心坐标来计算
        // 获取环形相关的属性，判断是否是顺时针创建的
        const directionHead = new Vector3(
          pHead.x - center.x,
          pHead.y - center.y,
          pHead.z - center.z,
        ).normalize()
        const directionTail = new Vector3(
          pTail.x - center.x,
          pTail.y - center.y,
          pTail.z - center.z,
        ).normalize()

        let hdgHead = ''
        let hdgTail = ''
        if (isClockwise || isReverseRoad) {
          // 顺时针绘制，或者是反向道路
          hdgHead = String(
            getWorldAngle(directionHead.applyAxisAngle(axisY, -halfPI)),
          )
          hdgTail = String(
            getWorldAngle(directionTail.applyAxisAngle(axisY, -halfPI)),
          )
        } else {
          // 常规的逆时针绘制
          hdgHead = String(
            getWorldAngle(directionHead.applyAxisAngle(axisY, halfPI)),
          )
          hdgTail = String(
            getWorldAngle(directionTail.applyAxisAngle(axisY, halfPI)),
          )
        }

        // 保存圆弧道路的控制点
        controlPoints.push(
          {
            ..._pHead,
            hdg: hdgHead,
          },
          {
            ..._pTail,
            hdg: hdgTail,
          },
        )
        controlType = 'arc'
      } else {
        // 常规的曲线道路
        // 如果只有2个控制点，其实就是 line 类型
        if (keyPath.points.length === 2) {
          controlPoints.push(
            getPointInfoOnCurve(keyPath, 0),
            getPointInfoOnCurve(keyPath, 1),
          )
          controlType = 'line'
        } else {
          // 单独添加第一个控制点
          controlPoints.push(getPointInfoOnCurve(keyPath, 0))
          // 道路的总长度
          const roadLength = keyPath.getLength()
          // 按照 0.1m 的采样间隔来求中间的控制点的占比
          const segment = roadLength * 10
          // 控制点总个数
          const { length: pointNumber } = keyPath.points
          for (let i = 1; i < pointNumber - 1; i++) {
            // 循环遍历控制点，第二个至倒数第二个不知道控制点占整条曲线路径的百分比
            const p = keyPath.points[i]
            const { point: closestPoint, tangent } =
              getClosestPointFromCurvePath({
                curvePath: keyPath,
                point: {
                  x: p.x,
                  y: p.y,
                  z: p.z,
                },
                segment,
              })
            const hdg = String(getWorldAngle(tangent))
            const _p = transformOneCoordinateToCommon(closestPoint)
            controlPoints.push({
              x: _p.x,
              y: _p.y,
              z: _p.z,
              hdg,
            })
          }
          // 单独添加最后一个控制点
          controlPoints.push(getPointInfoOnCurve(keyPath, 1))

          // 对于 catmullrom 类型的曲线控制点，需要在首和尾添加2个辅助的虚拟控制点
          addCatmullromVirtualPoint(controlPoints)
        }
      }

      // 将控制点的数据绑定到 road 的数据结构中
      _road.controlPoints = controlPoints
      _road.controlType = controlType
    }

    // 第三方地图和旧版编辑器生产的地图，不保存高程控制点，其他地图需要保存高程控制点

    if (road.elevationPath) {
      const points = cloneDeep(road.elevationPath.points)
      if (points.length > 1) {
        const elevation = []

        // 2个以上控制点
        for (const p of points) {
          // 当前控制点占道路参考线的比例
          const percent = Math.min(Math.max(p.x / road.length, 0), 1)

          // 获取当前控制点的切线夹角
          const tangent = road.elevationPath.getTangentAt(percent)
          const directionVec2 = new Vector2(tangent.x, tangent.y)
          const angle = getValidRadInRange(directionVec2.angle())

          // 获取每个一控制点的数据
          elevation.push({
            s: p.x,
            h: p.y,
            slope: String(angle),
          })
        }
        _road.elevation = elevation

        if (points.length === 2) {
          // 如果只有2个控制点
          const pHead = points[0]
          const pTail = points[1]
          // 且高程为 0
          if (fixedPrecision(pHead.y) === 0 && fixedPrecision(pTail.y) === 0) {
            // 则不透传高程数据
            delete _road.elevation
          }
        }
      }
    }

    // opencrgs 相关的数据保存
    if (road.crgConfig && road.crgConfig.length > 0) {
      const crgs: Array<biz.ICommonOpenCRG> = road.crgConfig.map((option) => {
        const {
          file,
          orientation,
          mode,
          purpose,
          sOffset,
          tOffset,
          zOffset,
          zScale,
          hOffset,
        } = option
        // 将偏航角的角度制转换成弧度制
        const _hOffset = String(
          // 弧度制透传时保存3位小数
          fixedPrecision(MathUtils.degToRad(Number(hOffset)), 3),
        )
        return {
          file,
          orientation,
          mode,
          purpose,
          sOffset: String(fixedPrecision(Number(sOffset), 1)),
          tOffset: String(fixedPrecision(Number(tOffset), 1)),
          zOffset: String(fixedPrecision(Number(zOffset), 1)),
          zScale: String(fixedPrecision(Number(zScale), 1)),
          hOffset: _hOffset,
        }
      })
      _road.opencrgs = crgs
    }

    _road.samplePoints = roadSamplePoints
    _road.sections = _sections
    _roads.push(_road)
  }

  return _roads
}

// 提取交叉路口和车道连接线级别的数据
function pickUpJunctionData () {
  const junctionStore = useJunctionStore()

  const _junctions: Array<biz.ICommonJunction> = []
  const _laneLinks: Array<biz.ICommonLaneLink> = []

  for (const junctionId of junctionStore.ids) {
    const junction = getJunction(junctionId)
    if (!junction) continue
    const { laneLinks } = junction

    // 对于交叉路口级别的数据，暂时没有过多的数据需要透传，目前只穿 junctionId
    _junctions.push({
      id: junctionId,
    })

    // 对于 laneLink 数据需要铺平传入数组中
    for (const laneLink of laneLinks) {
      const { samplePoints, enabled, controlPoints, controlType } = laneLink
      // 如果有效性为 false，则不会保存
      if (!enabled) continue

      // 将车道连接线的采样点进行坐标转换
      const _samplePoints = transformCoordinateToCommon(samplePoints)
      const _laneLink: biz.ICommonLaneLink = {
        samplePoints: _samplePoints,
        ...pick(laneLink, [
          'id',
          'fid',
          'frid',
          'fsid',
          'ftype',
          'tid',
          'trid',
          'tsid',
          'ttype',
          'length',
          'roadid',
        ]),
        junctionid: junctionId,
      }
      // laneLink 贝塞尔曲线的控制点信息
      if (controlPoints && controlPoints.length > 0) {
        // laneLink 透传控制点
        _laneLink.controlPoints = controlPoints
        _laneLink.controlType = controlType
      }

      _laneLinks.push(_laneLink)
    }
  }

  return {
    junctions: _junctions,
    laneLinks: _laneLinks,
  }
}
