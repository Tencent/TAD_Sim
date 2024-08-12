import { defineStore } from 'pinia'
import { CatmullRomCurve3, Vector3 } from 'three'
import {
  CatmullromTension,
  CurveType,
  compareTwoVector3InDeviation,
  getClosestPointFromCurvePath,
  transformVec3ByObject,
} from '@/utils/common3d'
import { genRoadId, genUuid } from '@/utils/guid'
import { getRoad } from '@/utils/mapCache'
import { registryStore } from '@/utils/tools'

interface IState {
  controlPoints: Array<biz.IControlPoint>
}

export interface IAddControlPointRes {
  controlPoint: biz.IControlPoint
  refPoint: biz.IRefPoint
}

const storeName = 'editRoad'
function createInitValue () {
  return {
    controlPoints: [],
  }
}
registryStore(storeName, createInitValue)

export const useEditRoadStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    length: state => state.controlPoints.length,
    /**
     * 判断是否属于首尾两侧的端点
     * @param state
     * @returns
     */
    isEndRefPoint: (state) => {
      return (cpId: string, refPointId: string) => {
        const cp = state.controlPoints.find(cp => cp.id === cpId)
        if (!cp) {
          return {
            isHead: false,
            isTail: false,
          }
        }
        let targetIndex = -1
        cp.points.forEach((rp, index) => {
          if (targetIndex !== -1) return
          if (rp.id === refPointId) {
            targetIndex = index
          }
        })

        return {
          isHead: targetIndex === 0,
          isTail: targetIndex === cp.points.length - 1,
        }
      }
    },
    getControlPointById: (state) => {
      return (id: string) => state.controlPoints.find(cp => cp.id === id)
    },
    /**
     * 通过 refPointId 找到对应的控制点
     * @param state
     * @returns
     */
    getControlPointByRefPointId: (state) => {
      return (refPointId: string) => {
        for (const cp of state.controlPoints) {
          for (const refPoint of cp.points) {
            if (refPoint.id === refPointId) {
              return cp
            }
          }
        }
        return null
      }
    },
    /**
     * 通过 roadId 找到对应的控制点
     * @param roadId
     */
    getControlPointByRoadId: (state) => {
      return (roadId: string) => {
        for (const cp of state.controlPoints) {
          if (cp.roadId.includes(roadId)) {
            return cp
          }
        }

        return null
      }
    },
    getRefPointById: (state) => {
      return (cpId: string, refPointId: string) => {
        const controlPoint = state.controlPoints.find(cp => cp.id === cpId)
        if (!controlPoint) return null
        const refPoint = controlPoint.points.find(rp => rp.id === refPointId)
        return refPoint
      }
    },
    getAdjacentRefPointsById: (state) => {
      return (cpId: string, refPointId: string) => {
        const controlPoint = state.controlPoints.find(cp => cp.id === cpId)
        if (!controlPoint) return []
        const index = controlPoint.points.findIndex(
          rp => rp.id === refPointId,
        )
        return [controlPoint.points[index - 1], controlPoint.points[index + 1]]
      }
    },
  },
  actions: {
    applyMapFileState (newState: IState) {
      return new Promise((resolve) => {
        const { controlPoints } = newState
        this.controlPoints = controlPoints
        resolve(true)
      })
    },
    applyState (lastState: IState, diffState: IState) {
      const { controlPoints } = lastState
      this.controlPoints = controlPoints
    },
    /**
     * 删除道路所有的参考线控制点（配合删除道路一起使用）
     * @param roadId
     */
    removeAllControlPoint (roadId: string) {
      let targetCp: biz.IControlPoint | null = null
      let targetIndex = ''
      for (const index in this.controlPoints) {
        const cp = this.controlPoints[index]
        if (cp.roadId.includes(roadId)) {
          targetCp = cp
          targetIndex = index
          break
        }
      }

      if (!targetCp) return ''
      // 如果一组控制点控制了2条单向道路，则只需要将 roadId 删除即可，参考线和控制点网格保留
      if (targetCp.roadId.length > 1) {
        const roadIdIndex = targetCp.roadId.findIndex(id => id === roadId)
        targetCp.roadId[roadIdIndex] = ''
        // 不需要触发辅助元素的销毁和重绘
        return ''
      } else {
        // 将当前控制点集合的数据从数组中删除
        this.controlPoints.splice(Number(targetIndex), 1)
        // 返回删除的控制点集 id，方便渲染层销毁实体元素
        return targetCp.id
      }
    },
    /**
     * 移动整条道路，对应的参考线控制点联动调整
     * @param params
     */
    moveAllControlPoint (params: {
      controlPointId: string
      offset: common.vec3
    }) {
      const { controlPointId, offset } = params
      const currentCp = this.getControlPointById(controlPointId)
      if (!currentCp) return null

      currentCp.points.forEach((p) => {
        p.x = p.x + offset.x
        p.z = p.z + offset.z
      })

      // 获取当前控制点管理的正向和反向道路，如果对应的道路是环形道路，则需要更新环形道路的圆心 center 属性
      const [forwardRoadId] = currentCp.roadId
      const forwardRoad = getRoad(forwardRoadId)
      // 正向道路
      if (forwardRoad && forwardRoad.isCircleRoad && forwardRoad.circleOption) {
        const { center } = forwardRoad.circleOption
        const newCenter = {
          x: center.x + offset.x,
          y: center.y,
          z: center.z + offset.z,
        }
        forwardRoad.circleOption.center = newCenter
      }

      return currentCp
    },
    /**
     * 新增控制点，根据 cpId 存在与否来判断是创建一组新的控制点，还是基于原来的某组控制点新增某一个控制点
     * @param params
     * @returns
     */
    addControlPoint (params: {
      vec3: common.vec3
      cpId?: string
      isTail?: boolean // 是否将新的点插入数组尾部
      onSuccess?: Function // 新增成功后的回调函数
    }) {
      const { vec3, cpId = '', isTail = true, onSuccess = () => {} } = params
      if (cpId) {
        // 如果有对应的 controlPoint id，则直接将控制点插入
        const currentCp = this.getControlPointById(cpId)
        if (!currentCp) return null

        // 判断新增的点，是否与之前的参考线控制点过于接近重合，如果几乎重合就不支持新增
        let isTooClose = false
        currentCp.points.forEach((p) => {
          if (isTooClose) return null
          // 50cm 的误差，可根据实际情况调整
          if (compareTwoVector3InDeviation(p, vec3, 0.5)) {
            isTooClose = true
          }
        })
        if (isTooClose) return null

        const refPoint = {
          ...vec3,
          id: genUuid(),
          parentId: cpId,
        }
        if (isTail) {
          currentCp.points.push(refPoint)
        } else {
          currentCp.points.unshift(refPoint)
        }
        onSuccess &&
        onSuccess({
          controlPoint: currentCp,
          refPoint,
        })

        return {
          // 如果是已创建的道路，则返回当前新增的点对应的百分比，用于控制点道路中 section 的比例和长度
          percent: isTail ? 1 : 0,
        }
      } else {
        // 如果没有 controlPoint id，则视为新创建一条道路，根据当前配置的是否支持单向道路选项来新增 road
        // 再向其中新增控制点，如果是双向车道，则共用控制点
        const newCpId = genUuid()
        const roadId: Array<string> = []

        roadId.push(genRoadId())

        const refPoint = {
          ...vec3,
          id: genUuid(),
          parentId: newCpId,
        }
        const newControlPoint: biz.IControlPoint = {
          id: newCpId,
          roadId,
          points: [refPoint],
        }
        // 新建一组控制点的数据结构
        this.controlPoints.push(newControlPoint)

        onSuccess &&
        onSuccess({
          controlPoint: newControlPoint,
          refPoint,
        })

        return true
      }
    },
    /**
     * 在道路参考线中，插入补间的控制点
     * @param params
     */
    addTweenControlPoint (params: {
      point: common.vec3 // 射线跟采样点网格的交互点
      cpId: string
      onSuccess?: Function
    }) {
      const { point, cpId, onSuccess = () => {} } = params
      const currentCp = this.getControlPointById(cpId)
      if (!currentCp) return null

      // 基于控制点创建一条新的样条曲线
      const _points = currentCp.points.map(p => new Vector3(p.x, p.y, p.z))
      const keyPath = new CatmullRomCurve3(_points)
      keyPath.curveType = CurveType
      if (CurveType === 'catmullrom') {
        keyPath.tension = CatmullromTension
      }

      // 获取距离交互点最近的一个样条曲线中的采样点
      const { point: targetPoint, percent: targetPercent } =
        getClosestPointFromCurvePath({
          curvePath: keyPath,
          point,
        })

      // 遍历当前所有控制点，拿插入点索引跟已存在的控制点索引做对比，判断插入点在数组中的索引
      let basicIndex = -1
      currentCp.points.forEach((p, index) => {
        if (basicIndex > -1) return
        const { percent } = getClosestPointFromCurvePath({
          curvePath: keyPath,
          point: p,
        })
        if (percent > targetPercent) {
          basicIndex = index
        }
      })

      if (basicIndex === -1) return null

      // 判断新增的点，是否与之前的参考线控制点过于接近重合，如果几乎重合就不支持新增
      let isTooClose = false
      currentCp.points.forEach((p) => {
        if (isTooClose) return
        // 50cm 的误差，可根据实际情况调整
        if (compareTwoVector3InDeviation(p, targetPoint, 0.5)) {
          isTooClose = true
        }
      })
      if (isTooClose) return null

      const refPoint = {
        ...targetPoint,
        id: genUuid(),
        parentId: cpId,
      }

      currentCp.points.splice(basicIndex, 0, refPoint)

      onSuccess &&
      onSuccess({
        controlPoint: currentCp,
        refPoint,
      })

      return {
        // 将插入的补间控制点的 percent 返回
        percent: targetPercent,
      }
    },
    /**
     * 删除某一组控制点中的某一个控制点
     * @param pointId
     * @param cpId
     * @returns
     */
    removeControlPoint (pointId: string, cpId: string) {
      const currentCp = this.getControlPointById(cpId)
      if (!currentCp) return null
      const refPointIndex = currentCp.points.findIndex(p => p.id === pointId)

      if (refPointIndex < 0) return null
      const removePoint = currentCp.points[refPointIndex]

      let closePointRes
      if (currentCp.points.length > 1) {
        // 先获取当前移动的点占整个参考线的百分比
        const _points = transformVec3ByObject(currentCp.points)
        const curvePath = new CatmullRomCurve3(_points)
        curvePath.curveType = CurveType
        if (CurveType === 'catmullrom') {
          curvePath.tension = CatmullromTension
        }
        closePointRes = getClosestPointFromCurvePath({
          curvePath,
          point: removePoint,
        })
      }

      currentCp.points.splice(refPointIndex, 1)

      // 对于只剩 0 和 1 个控制点的情况做特殊处理
      const [forwardRoadId] = currentCp.roadId
      if (currentCp.points.length === 1) {
        return {
          cpId,
          pointNumber: 1,
          forwardRoadId,
        }
      } else if (currentCp.points.length === 0) {
        // 如果所有的控制点都删除，则将当前点集对象也删除
        const cpIndex = this.controlPoints.findIndex(p => p.id === cpId)
        const [forwardRoadId] = currentCp.roadId
        this.controlPoints.splice(cpIndex, 1)
        return {
          cpId,
          pointNumber: 0,
          forwardRoadId,
        }
      }

      if (closePointRes) {
        return {
          cpId,
          // 传入 -1 认为不需要考虑当前还剩多少控制点
          pointNumber: -1,
          forwardRoadId,
          // 如果删除控制点后，对应的道路结构还存在，则返回当前删除控制点在原路径中所占的比例
          percent: closePointRes.percent,
        }
      } else {
        return {
          cpId,
          // 传入 -1 认为不需要考虑当前还剩多少控制点
          pointNumber: -1,
          forwardRoadId,
        }
      }
    },
    // 移动多个控制点
    moveMoreControlPoints (params: {
      cpId: string
      points: Array<biz.IRefPoint>
    }) {
      const { cpId, points } = params
      if (!cpId) return
      const currentCp = this.getControlPointById(cpId)
      if (!currentCp) return
      // 先通过映射表缓存
      const map: Map<string, biz.IRefPoint> = new Map()
      for (const p of points) {
        map.set(p.id, p)
      }

      currentCp.points.forEach((p) => {
        const res = map.get(p.id)
        if (!res) return

        // 更新每一个选中的控制点坐标
        p.x = res.x
        p.y = res.y
        p.z = res.z
      })

      // 手动清空
      map.clear()
    },
    /**
     * 移动某一组控制点中某一个控制点的位置
     * @param pointId
     * @param cpId
     * @param vec3
     * @returns
     */
    moveControlPoint (pointId: string, cpId: string, vec3: common.vec3) {
      if (!cpId) return
      const currentCp = this.getControlPointById(cpId)
      if (!currentCp) return
      const point = currentCp.points.find(p => p.id === pointId)
      if (!point) return

      let closePointRes
      if (currentCp.points.length > 1) {
        // 先获取当前移动的点占整个参考线的百分比
        const _points = transformVec3ByObject(currentCp.points)
        const curvePath = new CatmullRomCurve3(_points)
        curvePath.curveType = CurveType
        if (CurveType === 'catmullrom') {
          curvePath.tension = CatmullromTension
        }
        closePointRes = getClosestPointFromCurvePath({
          curvePath,
          point,
        })
      }

      point.x = vec3.x
      point.y = vec3.y
      point.z = vec3.z

      if (closePointRes) {
        // 将当前操作控制点占参考线路径的百分比返回
        return {
          percent: closePointRes.percent,
        }
      }
    },
  },
})
