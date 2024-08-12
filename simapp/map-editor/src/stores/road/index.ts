import { defineStore } from 'pinia'
import { CatmullRomCurve3, Vector3 } from 'three'
import { cloneDeep, get } from 'lodash'
import { useJunctionStore } from '../junction'
import { useFileStore } from '../file'
import { createRoadData, updateRoadAllGeoAttr } from './road'
import { getBoundaryById, updateLaneBoundaryInRoad } from './boundary'
import {
  addLaneInSection,
  getInnerTransitionLaneId,
  getTransitionLaneLinkSection,
  judgeOtherTransitionLaneInSection,
  judgeTargetLaneBlockIndex,
  removeLaneInSection,
  updateLaneCenterPoints,
  updateLaneWidthInSection,
} from './lane'
import {
  divideSectionOnly,
  divideSectionWithNarrowTween,
  divideSectionWithWidenTween,
} from './section'
import { useConfig3dStore } from '@/stores/config3d'
import { Constant, LaneFriction, LaneTypeEnum } from '@/utils/business'
import {
  CatmullromTension,
  CurveType,
  compareTwoNumberInDeviation,
  fixedPrecision,
  getCurveEndPointVerticalVec3,
  sortByAbsId,
  transformArrayObjectToArray,
  transformVec3,
  transformVec3ByArray,
  transformVec3ByObject,
} from '@/utils/common3d'
import { registryStore } from '@/utils/tools'
import { getRoad, removeRoad, setRoad } from '@/utils/mapCache'
import { genRoadId, genUuid } from '@/utils/guid'
import { useEditRoadStore } from '@/plugins/editRoad/store'
import { warningMessage } from '@/utils/common'
import i18n from '@/locales'

interface IState {
  ids: Array<string>
  elements: Array<biz.IRoad>
}

export interface IQueryLane {
  roadId: string
  sectionId: string
  laneId: string
}
export interface IUpdateLaneWidth extends IQueryLane {
  width: number
}

export interface IUpdateSection extends IQueryLane {
  tweenSamplePoints: Array<Vector3>
  // 左右车道边界线截断点的"索引/总长度"形式
  rightIndex: string
  leftIndex: string
}

const storeName = 'road'
function createInitValue (): IState {
  return {
    ids: [],
    elements: [],
  }
}

registryStore(storeName, createInitValue)

export const useRoadStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    length: state => state.ids.length,
    getRoadById: () => {
      return (roadId: string) => getRoad(roadId)
    },
    getSectionById: () => {
      return (roadId: string, sectionId: string) => {
        const road = getRoad(roadId)
        if (!road) return null
        const section = road.sections.find(
          section => section.id === sectionId,
        )
        if (!section) return null
        return {
          section,
          road,
        }
      }
    },
    getLaneById: () => {
      return (roadId: string, sectionId: string, laneId: string) => {
        const road = getRoad(roadId)
        if (!road) return null
        const section = road.sections.find(
          section => section.id === sectionId,
        )
        if (!section) return null
        const lane = section.lanes.find(lane => lane.id === laneId)
        if (!lane) return null
        return {
          lane,
          section,
          road,
        }
      }
    },
    getLaneBoundaryById: () => {
      return (roadId: string, sectionId: string, boundaryId: string) => {
        const road = getRoad(roadId)
        if (!road) return null
        const section = road.sections.find(
          section => section.id === sectionId,
        )
        if (!section) return null
        const boundary = section.boundarys.find(
          boundary => boundary.id === boundaryId,
        )
        if (!boundary) return null
        return {
          boundary,
          section,
          road,
        }
      }
    },
  },
  actions: {
    /**
     * 将地图文件中解析的道路状态同步到 store 中
     * @param newState
     */
    applyMapFileState (newState: IState) {
      return new Promise((resolve) => {
        const { ids } = newState
        this.ids = ids
        resolve(true)
      })
    },
    /**
     * 用历史的状态更新作为当前的状态
     * @param lastState
     * @param diffState diff 结果，用于定于 applyState 操作后的渲染重绘
     */
    applyState (params: {
      last: IState
      diff: IState
      lastDiffIds: Array<string>
    }) {
      const { last: lastState, lastDiffIds } = params
      const { ids: lastIds, elements } = lastState
      // 对于 store 中存储的 threejs 实例对象，重新赋值保证对应的实例方法能够正常调用
      elements &&
      elements.forEach((road) => {
        // 重新计算对应的 ketPath 属性
        const { keyPath, elevationPath } = road
        const _keyPath = new CatmullRomCurve3(
          transformVec3ByArray(keyPath.points),
        )
        _keyPath.curveType = CurveType
        if (CurveType === 'catmullrom') {
          _keyPath.tension = CatmullromTension
        }
        road.keyPath = _keyPath
        if (elevationPath) {
          // 高程曲线
          const _elevationPath = new CatmullRomCurve3(
            transformVec3ByArray(elevationPath.points),
          )
          _elevationPath.curveType = CurveType
          if (CurveType === 'catmullrom') {
            _elevationPath.tension = CatmullromTension
          }
          road.elevationPath = _elevationPath
        }
        // 对于存储的 Vector3 实例重新赋值
        road.verticalAtHead = transformVec3(road.verticalAtHead)
        road.verticalAtTail = transformVec3(road.verticalAtTail)

        // 重新给车道和车道边界线的 geoAttr 赋值
        for (const section of road.sections) {
          for (const lane of section.lanes) {
            if (!lane.geoAttr) continue
            lane.geoAttr.vertices = transformArrayObjectToArray(
              lane.geoAttr.vertices,
            )
            lane.geoAttr.indices = transformArrayObjectToArray(
              lane.geoAttr.indices,
            )
            lane.geoAttr.uvs = transformArrayObjectToArray(lane.geoAttr.uvs)
          }
          for (const boundary of section.boundarys) {
            if (boundary.firstLineAttr) {
              boundary.firstLineAttr.vertices = transformArrayObjectToArray(
                boundary.firstLineAttr.vertices,
              )
              boundary.firstLineAttr.indices = transformArrayObjectToArray(
                boundary.firstLineAttr.indices,
              )
            }
            if (boundary.secondLineAttr) {
              boundary.secondLineAttr.vertices = transformArrayObjectToArray(
                boundary.secondLineAttr.vertices,
              )
              boundary.secondLineAttr.indices = transformArrayObjectToArray(
                boundary.secondLineAttr.indices,
              )
            }
          }
        }
        // 将上一个状态的 road 对象保存到缓存中
        setRoad(road.id, road)
      })

      // 如果存在 diffIds，但上一个状态中id不存在，说明是上一个状态没有这个元素
      for (const diffId of lastDiffIds) {
        if (!lastState.ids.includes(diffId)) {
          removeRoad(diffId)
        }
      }

      // 将上一个状态，作为当前的状态
      this.ids = lastIds
    },
    async updateArcRoad (params: {
      roadId: Array<string>
      radius: number
      startAngle: number
      endAngle: number
      center: common.vec3
      points: Array<common.vec3>
    }) {
      const { roadId, radius, startAngle, endAngle, center, points } = params
      const [forwardRoadId] = roadId

      const updatePromises: Array<Promise<string>> = []

      // 需要给dispatchers逻辑中用于操作记录的有效道路 id
      const _roadIds: Array<string> = []

      // 对采样点进行调整
      if (points.length > 2) {
        // 最后的2个采样点，如果重合，需要删除掉一个重合的点
        const lastFirst = points[points.length - 1]
        const lastSecond = points[points.length - 2]

        if (
          fixedPrecision(lastFirst.x) === fixedPrecision(lastSecond.x) &&
          fixedPrecision(lastFirst.z) === fixedPrecision(lastSecond.z)
        ) {
          points.splice(points.length - 1, 1)
        }
      }

      // 更新 editRoadStore 中的道路参考线控制点的属性
      const editRoadStore = useEditRoadStore()
      const cp = editRoadStore.getControlPointByRoadId(forwardRoadId)
      if (cp) {
        const refPoints = points.map(p => ({
          x: p.x,
          y: p.y,
          z: p.z,
          id: genUuid(),
          parentId: cp.id,
        }))
        // 更新道路参考线对应的控制点数据
        cp.points = refPoints
      }

      // 正向道路
      if (forwardRoadId) {
        const _points = transformVec3ByObject(points)
        const forwardCurve = new CatmullRomCurve3(_points)
        forwardCurve.curveType = CurveType
        updatePromises.push(
          (() => {
            return new Promise((resolve) => {
              this.updateRoadKeyPath({
                roadId: forwardRoadId,
                keyPath: forwardCurve,
                isPure: true,
              }).then(() => {
                const forwardRoad = getRoad(forwardRoadId)
                if (forwardRoad) {
                  forwardRoad.circleOption = {
                    radius,
                    startAngle,
                    endAngle,
                    center,
                  }
                }
                _roadIds.push(forwardRoadId)
                resolve(forwardRoadId)
              })
            })
          })(),
        )
      }

      await Promise.all(updatePromises)

      return _roadIds
    },
    // 创建环形道路，不需要可视化的道路参考线控制点
    async createCircleRoad (options: Array<biz.ICircleOption>) {
      // 缓存创建的道路 id
      const roadIds: Array<string> = []
      const createPromises: Array<Promise<string>> = []
      const editRoadStore = useEditRoadStore()
      for (const option of options) {
        const { points, center, radius, startAngle, endAngle } = option
        if (!points) continue

        // 判断最后的2个顶点是否重合，删除掉重合的顶点
        if (points.length > 2) {
          // 最后的2个采样点
          const lastFirst = points[points.length - 1]
          const lastSecond = points[points.length - 2]
          if (
            fixedPrecision(lastFirst.x) === fixedPrecision(lastSecond.x) &&
            fixedPrecision(lastFirst.z) === fixedPrecision(lastSecond.z)
          ) {
            // 删除最后一个重合采样点
            points.splice(points.length - 1, 1)
          }
        }

        // 基于采样点，创建对应的道路参考线
        const _points = transformVec3ByObject(points)

        const forwardCurve = new CatmullRomCurve3(_points)
        forwardCurve.curveType = CurveType

        // 生成对应的正向和反向道路
        const forwardRoadId = genRoadId()
        roadIds.push(forwardRoadId)

        // 应该是需要调用 editRoadStore，通过道路参考线控制点来管理的
        const cpId = genUuid()
        const controlRoadId = [forwardRoadId]
        const refPoints = points.map(p => ({
          x: p.x,
          y: p.y,
          z: p.z,
          id: genUuid(),
          parentId: cpId,
        }))
        const controlPoint: biz.IControlPoint = {
          id: cpId,
          roadId: controlRoadId,
          points: refPoints,
        }
        // 直接将控制点添加到对应的状态数组中
        editRoadStore.controlPoints.push(controlPoint)

        createPromises.push(
          (() => {
            return new Promise((resolve) => {
              // 创建正向环形道路
              this.updateRoadKeyPath({
                roadId: forwardRoadId,
                keyPath: forwardCurve,
                isPure: true,
              }).then(() => {
                const forwardRoad = getRoad(forwardRoadId)
                if (forwardRoad) {
                  // 将道路更新为环形道路，挂载对应的属性
                  forwardRoad.isCircleRoad = true
                  forwardRoad.circleOption = {
                    radius,
                    center,
                    startAngle,
                    endAngle,
                  }
                }

                resolve(forwardRoadId)
              })
            })
          })(),
        )
      }

      await Promise.all(createPromises)

      return roadIds
    },
    /**
     * 更新道路的参考线，包含新增道路和更新道路
     * @param roadId
     * @param keyPath
     * @returns
     */
    async updateRoadKeyPath (params: {
      roadId: string
      keyPath: biz.ICurve3
      targetPercent?: number // 修改的控制点在原来的参考线路径中，所占的百分比
      isPure?: boolean // 是否是纯粹地更新道路结构
    }) {
      const { roadId, keyPath, targetPercent = undefined } = params
      if (!roadId) return
      const main3dStore = useConfig3dStore()
      let currentRoad = this.getRoadById(roadId)
      let isNew = false
      if (currentRoad) {
        // 重新计算车道中心线首尾的垂直向量
        const roadLength = fixedPrecision(keyPath.getLength())
        const [verticalAtHead, verticalAtTail] =
          getCurveEndPointVerticalVec3(keyPath)

        // 如果存在更新的比例
        if (targetPercent !== undefined) {
          // 将比例控制在 [0, 1] 区间
          const _percent = Math.max(0, Math.min(targetPercent, 1))

          // 找到目标百分比对应对应的 section。除了 targetSection，其余 section 的 length 属性保持不变，百分比会变化
          let targetSectionId = ''
          let targetSection: biz.ISection | null = null
          // 第一次遍历时，获取除了目标section外的其余section长度总和
          let otherSectionLength = 0
          for (const section of currentRoad.sections) {
            const { pStart, pEnd, length } = section
            if (_percent >= pStart && _percent <= pEnd) {
              targetSection = section
              targetSectionId = section.id
            } else {
              // 叠加其他section的长度
              otherSectionLength += length
            }
          }
          // 对于首尾 section 自身的比例精度不准确的情况，手动再判断一遍
          const delta = 0.05
          const headRes = compareTwoNumberInDeviation(0, _percent, delta)
          const tailRes = compareTwoNumberInDeviation(1, _percent, delta)
          if (headRes) {
            if (targetSectionId !== '0') {
              targetSectionId = '0'
              targetSection = currentRoad.sections[0]
              // 从叠加的section长度中将目标section剔除
              otherSectionLength -= targetSection.length
            }
          } else if (tailRes) {
            if (targetSectionId !== String(currentRoad.sections.length - 1)) {
              // sectionId 在每条道路中都是从 0 递增
              targetSectionId = String(currentRoad.sections.length - 1)
              targetSection =
                currentRoad.sections[currentRoad.sections.length - 1]
              // 从叠加的section长度中将目标section剔除
              otherSectionLength -= targetSection.length
            }
          }

          // 只有目标 section 存在，才会针对每个 section 的比例做调整
          if (targetSectionId && targetSection) {
            // 再次遍历道路中的 section
            let basicPercent = 0
            for (const section of currentRoad.sections) {
              const { id: sectionId, length: originLength } = section
              if (sectionId !== targetSectionId) {
                // 对于 targetSection 外的其余 section，保持长度不变，比例更新
                const percentSpan = originLength / roadLength
                section.pStart = basicPercent
                section.pEnd = basicPercent + percentSpan
                basicPercent += percentSpan
              } else {
                // 对于 targetSection，长度应该时自适应的，长度和比例都需要更新
                const targetLength = fixedPrecision(
                  roadLength - otherSectionLength,
                )
                section.length = targetLength
                const percentSpan = targetLength / roadLength
                section.pStart = basicPercent
                section.pEnd = basicPercent + percentSpan
                basicPercent += percentSpan
              }
            }
            // 对于最后一个 section 有可能 pEnd 叠加的结果不一定为 1，手动设置为 1，保证整个道路的 section 的占比从 0 到 1
            currentRoad.sections[currentRoad.sections.length - 1].pEnd = 1
          }
        } else {
          // 如果不存在 section 比例的更新，则基于原本的 section 占比更新 section 的长度
          currentRoad.sections.forEach((section) => {
            const { pStart, pEnd } = section
            section.length = fixedPrecision(roadLength * (pEnd - pStart))
          })
        }

        currentRoad.keyPath = keyPath
        currentRoad.length = roadLength
        currentRoad.verticalAtHead = verticalAtHead
        currentRoad.verticalAtTail = verticalAtTail

        // 调整已有的高程数据
        const { elevationPath } = currentRoad
        if (elevationPath && elevationPath.points.length > 1) {
          const points = cloneDeep(elevationPath.points)
          const lastRoadLength = points[points.length - 1].x

          // 更新每一个高程控制点的水平坐标（即x坐标）
          points.forEach((p) => {
            // 获取原有高程控制点在当前道路中的比例
            const percent = p.x / lastRoadLength
            p.x = fixedPrecision(roadLength * percent)
          })

          // 获取高程曲线的路径
          const newPath = new CatmullRomCurve3(transformVec3ByObject(points))
          newPath.curveType = CurveType
          if (CurveType === 'catmullrom') {
            newPath.tension = CatmullromTension
          }

          currentRoad.elevationPath = newPath
        }

        // 车道边界线采样点的数据更新
        updateLaneBoundaryInRoad({
          keyPath,
          road: currentRoad,
        })
      } else {
        // 如果没有找到对应的 road，则视为新增一条道路
        isNew = true
        const road = createRoadData({
          roadId,
          keyPath,
          laneNumber: main3dStore.defaultLaneNumber,
          laneWidth: main3dStore.defaultLaneWidth,
          roadType: main3dStore.roadType, // 默认创建的道路类型
        })
        currentRoad = road
      }

      // 更新当前道路中，车道和车道边界线的几何体属性
      await updateRoadAllGeoAttr(currentRoad)

      // 更新缓存的道路属性
      if (currentRoad) {
        setRoad(roadId, currentRoad)
      }
      // 如果是新增道路，则需要推入 id
      if (isNew) {
        this.ids.push(roadId)
      }
    },
    // 更新一条道路的高程控制点（需要在调用该方法之前就区分反向道路）
    async updateRoadElevation (
      roadOptions: Array<{
        roadId: string
        points: Array<common.vec3>
      }>,
      action?: 'update' | 'add' | 'remove', // 更新控制点的类型
    ) {
      if (!roadOptions || roadOptions.length < 1) return

      for (const option of roadOptions) {
        const { roadId, points } = option
        if (!roadId) continue
        const road = this.getRoadById(roadId)
        if (!road) continue

        // 用新的高程数据替换原有的高程
        const elevationPath = new CatmullRomCurve3(
          transformVec3ByObject(points),
        )
        elevationPath.curveType = CurveType
        road.elevationPath = elevationPath

        // 车道边界线采样点的数据更新
        updateLaneBoundaryInRoad({
          keyPath: road.keyPath,
          road,
        })

        // 更新当前道路中，车道和车道边界线的几何体属性
        await updateRoadAllGeoAttr(road)

        setRoad(roadId, road)
      }

      // 返回更新的道路 id
      return roadOptions.map(option => option.roadId)
    },
    /**
     * 删除整条单向道路
     * @param roadId
     * @returns
     */
    async removeRoad (roadId: string) {
      if (!roadId) return null
      const road = this.getRoadById(roadId)
      if (!road) return null

      const _road = cloneDeep(road)

      // 删除道路，提前将连接关系从已连接的 junction 中移除
      const junctionStore = useJunctionStore()
      // 拷贝当前路口关联的路口 id 集合
      const linkJunction = [...road.linkJunction]
      const removeJunctionPromises = []
      for (const junctionId of linkJunction) {
        removeJunctionPromises.push(
          (() => {
            return new Promise((resolve) => {
              junctionStore
                .disconnectLinkRoad({
                  junctionId,
                  // 将首尾的连接标识组装都删除
                  linkRoad: [
                    `${roadId}_1_forward`,
                    `${roadId}_1_reverse`,
                    `${roadId}_0_forward`,
                    `${roadId}_0_reverse`,
                  ],
                })
                .then((data) => {
                  resolve(data)
                })
            })
          })(),
        )
      }
      const removeJunctionRes = await Promise.all(removeJunctionPromises)

      // 将道路数据从数组中删除
      const roadIndex = this.ids.findIndex(id => id === roadId)
      this.ids.splice(roadIndex, 1)
      removeRoad(roadId)

      // 将删除的地图元素数据返回！
      return {
        road: _road,
        removeJunctionRes,
      }
    },
    /**
     * 道路添加关联的交叉路口
     * @param roadId
     * @param junctionId
     * @returns
     */
    addRoadLinkJunction (roadId: string, junctionId: string) {
      const road = this.getRoadById(roadId)
      if (!road) return
      if (!road.linkJunction.includes(junctionId)) {
        road.linkJunction.push(junctionId)
      }
    },
    /**
     * 删除道路关联的交叉路口
     * @param roadId
     * @param junctionId
     * @returns
     */
    removeRoadLinkJunction (roadId: string, junctionId: string) {
      const road = this.getRoadById(roadId)
      if (!road) return
      const index = road.linkJunction.findIndex(id => id === junctionId)
      if (index > -1) {
        road.linkJunction.splice(index, 1)
      }
    },
    /**
     * 更新车道宽度
     * @param params
     */
    async updateLaneWidth (params: IUpdateLaneWidth) {
      const { roadId, sectionId, laneId, width } = params
      const fileStore = useFileStore()
      // 当前是否有打开的地图文件
      const isOpenFile = !!fileStore.openedFile

      // 限制车道宽度
      const _width = Math.max(
        Constant.minLaneWidth,
        Math.min(Constant.maxLaneWidth, width),
      )

      // 判断指定的车道前后，是否存在截断的情况，只调整区间内的指定车道的宽度
      const queryResult = this.getLaneById(roadId, sectionId, laneId)
      if (!queryResult) return false
      // 如果车道是包含平滑过渡的车道，后续再支持
      const { road } = queryResult
      //  暂不支持在平滑过渡车道外侧的目标车道新增车道，特殊情况太多太复杂了
      const transitionLaneId = getInnerTransitionLaneId(road.sections)
      if (
        transitionLaneId &&
        Math.abs(Number(laneId)) >= Math.abs(Number(transitionLaneId))
      ) {
        console.log('暂不支持修改平滑过渡车道及外侧车道的宽度')
        return false
      }

      const { keyPath, elevationPath } = road
      // 先判断是否存在车道的截断
      const { head: headBlockIndex, tail: tailBlockIndex } =
        judgeTargetLaneBlockIndex(road, sectionId, laneId)

      for (let i = 0; i < road.sections.length; i++) {
        // 对于截断区域之外的 section 不处理
        if (headBlockIndex !== -1 && i <= headBlockIndex) continue
        if (tailBlockIndex !== -1 && i >= tailBlockIndex) continue
        const section = road.sections[i]
        // 更新 section 中的车道宽度
        updateLaneWidthInSection({
          section,
          laneId,
          keyPath,
          elevationPath,
          width: _width,
          isOpenFile,
        })

        // 更新完车道结构后，更新车道中心线的采样点
        updateLaneCenterPoints({
          lanes: section.lanes,
          boundarys: section.boundarys,
        })
      }

      // 更新当前道路中，车道和车道边界线的几何体属性
      await updateRoadAllGeoAttr(road)

      return true
    },
    /**
     * 新增车道（涉及大量采样点数据的更新计算，在调用前需要控制频率）
     * @param params
     * @returns
     */
    async addLane (params: IQueryLane) {
      const { roadId, sectionId, laneId } = params
      const queryResult = this.getLaneById(roadId, sectionId, laneId)
      if (!queryResult) return false
      const { road } = queryResult
      const { keyPath, elevationPath } = road

      const fileStore = useFileStore()
      // 当前是否有打开的地图文件
      const isOpenFile = !!fileStore.openedFile

      // 暂不支持在平滑过渡车道外侧的目标车道新增车道，特殊情况太多太复杂了
      const transitionLaneId = getInnerTransitionLaneId(road.sections)
      if (
        transitionLaneId &&
        Math.abs(Number(laneId)) >= Math.abs(Number(transitionLaneId))
      ) {
        console.log('暂不支持新增平滑过渡车道及外侧的车道')
        return false
      }

      // 先判断是否存在车道的截断
      const { head: headBlockIndex, tail: tailBlockIndex } =
        judgeTargetLaneBlockIndex(road, sectionId, laneId)

      for (let i = 0; i < road.sections.length; i++) {
        // 对于截断区域之外的 section 不处理
        if (headBlockIndex !== -1 && i <= headBlockIndex) continue
        if (tailBlockIndex !== -1 && i >= tailBlockIndex) continue
        const section = road.sections[i]
        addLaneInSection({
          laneId,
          section,
          keyPath,
          elevationPath,
          isOpenFile,
        })
        // 更新完车道结构后，更新车道中心线的采样点
        updateLaneCenterPoints({
          lanes: section.lanes,
          boundarys: section.boundarys,
        })
      }

      // 更新当前道路中，车道和车道边界线的几何体属性
      await updateRoadAllGeoAttr(road)

      return true
    },
    /**
     * 删除车道（涉及大量采样点数据的更新计算，在调用前需要控制频率）
     * @param params
     * @returns
     */
    async removeLane (params: IQueryLane) {
      const { roadId, sectionId, laneId } = params
      const queryResult = this.getLaneById(roadId, sectionId, laneId)
      if (!queryResult) return false
      const { road, section } = queryResult
      const { keyPath, elevationPath } = road

      // 不允许删除最后一条正向和反向的车道
      const isForward = Number(laneId) < 0
      const forwardLanes = section.lanes
        .filter(lane => Number(lane.id) < 0)
        .sort(sortByAbsId)
      const reverseLanes = section.lanes
        .filter(lane => Number(lane.id) > 0)
        .sort(sortByAbsId)

      if (isForward) {
        if (forwardLanes.length === 1 && laneId === forwardLanes[0].id) {
          // 如果section中只剩下最后一条正向lane，则不允许删除，且在前端侧提示
          warningMessage({
            content: i18n.global.t(
              'actions.tips.notSupportedDeleteOnlyOneForwardLaneInSection',
            ),
          })
          return false
        }
      } else {
        if (reverseLanes.length === 1 && laneId === reverseLanes[0].id) {
          // 如果section中只剩下最后一条反向lane，则不允许删除，且在前端侧提示
          warningMessage({
            content: i18n.global.t(
              'actions.tips.notSupportedDeleteOnlyOneReverseLaneInSection',
            ),
          })
          return false
        }
      }

      // 1、判断是否有平滑过渡车道
      const transitionLaneId = getInnerTransitionLaneId(road.sections)
      let headBlockIndex = -1
      let tailBlockIndex = -1
      let startSectionId = ''
      let endSectionId = ''
      if (!transitionLaneId) {
        // 如果没有，则直接判断是否有 block 的外形截断
        const blockRes = judgeTargetLaneBlockIndex(road, sectionId, laneId)
        headBlockIndex = blockRes.head
        tailBlockIndex = blockRes.tail
      } else {
        // 如果有，则判断是否存在平滑过渡的车道连通关系
        if (Math.abs(Number(laneId)) > Math.abs(Number(transitionLaneId))) {
          // 暂不支持删除平滑过渡车道外侧的车道（包括平滑过渡车道和常规车道）
          console.warn('暂不支持删除平滑过渡车道外侧的车道')
          return false
        } else if (laneId === transitionLaneId) {
          // 待删除的车道，跟平滑过渡车道 id 相同，说明处于同一排序位置中
          // 只能删除平滑过渡的车道以及跟它存在连通关系的相关车道
          const sectionRes = getTransitionLaneLinkSection({
            sections: road.sections,
            sectionId,
            laneId,
          })
          startSectionId = sectionRes.start
          endSectionId = sectionRes.end

          // 没有筛选出符合规定的 sectionId，返回 -1 说明存在删除平滑过渡车道外侧车道的情况
          if (startSectionId === '-1' || endSectionId === '-1') {
            console.log('暂不支持删除平滑过渡车道外侧的车道')
            return false
          }

          // 如果待删除的平滑过渡车道及其连通车道所在的 section 中，还包含其他平滑过渡车道，则视为复杂情况，也暂不支持删除
          const isOtherTransitionLaneExist = judgeOtherTransitionLaneInSection({
            sections: road.sections,
            startSectionId,
            endSectionId,
            laneId,
          })
          if (isOtherTransitionLaneExist) {
            console.warn('暂不支持删除存在多个平滑过渡车道堆叠情况的车道')
            return false
          }
        } else {
          // 暂不支持删除平滑过渡车道内侧唯一的一条常规车道，会导致平滑过渡车道没有正确的连通关系
          if (transitionLaneId === '-2' && laneId === '-1') {
            console.warn('暂不支持删除平滑过渡车道内侧唯一的常规车道')
            return false
          }
          // 删除平滑过渡车道内侧的常规车道，是可支持的，也需要做外形截断的判断
          const blockRes = judgeTargetLaneBlockIndex(road, sectionId, laneId)
          headBlockIndex = blockRes.head
          tailBlockIndex = blockRes.tail
        }
      }

      for (let i = 0; i < road.sections.length; i++) {
        const section = road.sections[i]

        // 判断是否有符合条件的可删除的平滑过渡车道 section
        if (!startSectionId || !endSectionId) {
          // 如果指定的 section 区间不存在，则先判断是否存在车道的截断
          if (headBlockIndex !== -1 && i <= headBlockIndex) continue
          if (tailBlockIndex !== -1 && i >= tailBlockIndex) continue
        } else {
          // 指定的 section 区间存在，则在跳过区间外的 section
          if (Number(section.id) < Number(startSectionId)) continue
          if (Number(section.id) > Number(endSectionId)) continue
        }

        // 删除 section 中指定的车道
        removeLaneInSection({
          laneId,
          section,
          keyPath,
          elevationPath,
        })

        // 更新完车道结构后，更新车道中心线的采样点
        updateLaneCenterPoints({
          lanes: section.lanes,
          boundarys: section.boundarys,
        })
      }

      // 更新当前道路中，车道和车道边界线的几何体属性
      await updateRoadAllGeoAttr(road)

      return true
    },

    /**
     * 只将 section 一分为二，不存在平滑过渡的补间边界
     * @param params
     */
    async addSectionOnly (params: IUpdateSection) {
      const { roadId, sectionId, laneId, leftIndex } = params
      const queryResult = this.getLaneById(roadId, sectionId, laneId)
      if (!queryResult) return
      const { road, section: targetSection } = queryResult
      const { keyPath } = road
      // 由于 leftIndex 和 rightIndex 一样，取一个就行
      const [_leftIndex, totalIndex] = leftIndex
        .split('/')
        .map(num => Number(num))
      const totalSegment = totalIndex - 1
      const { pStart, pEnd } = targetSection

      // 计算截断点在整个道路中心线的占比
      const delta = (pEnd - pStart) / totalSegment
      // percent 的数值需要调整精度
      const newPercent = fixedPrecision(
        pStart + delta * _leftIndex,
        Constant.offsetPointsSamplePrecision,
      )

      divideSectionOnly({
        road,
        section: targetSection,
        laneId,
        newPercent,
      })
    },
    /**
     * 将 section 划分成带有平滑过渡边界线效果的 section
     * @param params
     */
    async addSectionWithTween (params: IUpdateSection) {
      const {
        roadId,
        sectionId,
        laneId,
        leftIndex,
        rightIndex,
        tweenSamplePoints,
      } = params
      const queryResult = this.getLaneById(roadId, sectionId, laneId)
      if (!queryResult) return
      const { road, section: targetSection } = queryResult
      const [_leftIndex] = leftIndex.split('/').map(num => Number(num))
      const [_rightIndex] = rightIndex.split('/').map(num => Number(num))

      // isExtends = true，表示截断点左侧车道数减 1，存在车道数从 n-1 变成 n 的平滑过渡
      // isExtends = false，表示截断点右侧车道数减 1，存在车道数从 n 变成 n-1 的平滑过渡
      const isExtends = _leftIndex < _rightIndex

      if (isExtends) {
        divideSectionWithWidenTween({
          road,
          section: targetSection,
          laneId,
          leftIndex,
          rightIndex,
          tweenSamplePoints,
        })
      } else {
        divideSectionWithNarrowTween({
          road,
          section: targetSection,
          laneId,
          leftIndex,
          rightIndex,
          tweenSamplePoints,
        })
      }
    },
    /**
     * 更新车道的静态属性
     * @param params
     * @returns
     */
    updateLaneAttr (params: {
      roadId: string
      sectionId: string
      laneId: string
      attrName: 'speedlimit' | 'type' | 'friction' | 'sOffset'
      value: string | number
    }) {
      const { roadId, sectionId, laneId, attrName, value } = params
      const queryLaneRes = this.getLaneById(roadId, sectionId, laneId)
      if (!queryLaneRes) return
      const { section, lane } = queryLaneRes
      lane[attrName] = Number(value)
      // 从数据层控制材质纵向偏移量的最大值（不能超过section长度）
      if (attrName === 'sOffset') {
        if (Number(value) > section.length) {
          // 手动限制最大值
          lane[attrName] = section.length
        }
      }
      if (attrName === 'type') {
        // 如果是更新车道类型，需要自动切换对应的摩擦力系数，覆盖掉原有的值
        const laneTypeStr = LaneTypeEnum[Number(value)] as biz.ILaneType
        const friction = LaneFriction[laneTypeStr] || 0
        lane.friction = friction
      }
    },
    async updateLaneBoundaryMark (params: {
      roadId: string
      sectionId: string
      boundaryId: string
      mark: number
    }) {
      const { roadId, sectionId, boundaryId, mark } = params
      const querySectionRes = this.getSectionById(roadId, sectionId)
      if (!querySectionRes) return
      const { road, section } = querySectionRes
      const boundary = getBoundaryById(section.boundarys, boundaryId)
      if (!boundary) return
      // 新旧的 mark 一致直接返回
      if (boundary.mark === mark) return
      boundary.mark = mark

      // 只更新边界线的几何体结构，车道结构不动
      await updateRoadAllGeoAttr(road, {
        ignoreLane: true,
      })

      return true
    },
    // 计算当前地图元素的中心点
    getMapCenter () {
      const length = this.ids.length
      if (length < 1) return null

      let _x = 0
      let _y = 0
      let _z = 0
      for (const roadId of this.ids) {
        const road = getRoad(roadId)
        if (!road) continue
        const { points: keyPathPoints } = road.keyPath
        const firstPoint = keyPathPoints[0]
        const lastPoint = keyPathPoints[keyPathPoints.length - 1]

        // 道路首尾参考点的中点

        const centerX = (firstPoint.x + lastPoint.x) / 2
        const centerY = (firstPoint.y + lastPoint.y) / 2
        const centerZ = (firstPoint.z + lastPoint.z) / 2

        _x += centerX
        _y += centerY
        _z += centerZ
      }

      const center = new Vector3(_x / length, _y / length, _z / length)

      return center
    },
    // 在一条目标道路上，新增 openCrg 相关的配置
    addCrg (params: {
      roadId: string
      file: string // 关联的 crg 文件名称【单机版是绝对路径，云端版是 fileKey（包含唯一 uuid 和 crg 文件名称）】
      id?: string // 【云端版】crg 文件的 id
      version?: string // 【云端版】crg 文件的版本
      showName?: string // 【云端版】crg 名称
    }) {
      const { roadId, id, version, file } = params
      let { showName = '' } = params
      const road = getRoad(roadId)
      if (!road) return
      // 如果没 crg 配置，则初始化
      if (!road.crgConfig) {
        road.crgConfig = []
      }

      if (!showName) {
        // 如果 showName 没有透传，说明是单机版，则使用路径解析后的文件名称
        // 单机版涉及 windows 和 ubuntu，需要通过 electron 获取当前系统的路径分隔符
        const sep = get(window, 'electron.path.sep') || '\\'
        const fileSegment = file.split(sep)
        showName = fileSegment[fileSegment.length - 1]
      }

      let index = -1
      // 通过 file 属性找到对应 crg 选项进行更新
      for (let i = 0; i < road.crgConfig.length; i++) {
        if (index > -1) continue

        const crgOption = road.crgConfig[i]
        // 通过 file 和 showName 完全一致来确认唯一性
        if (file === crgOption.file && showName === crgOption.showName) {
          index = i
        }
      }
      // 如果当前已经配置过的 crg 数据中存在相同的数据，则不能重复添加
      if (index > -1) {
        // 提示不能重复添加
        warningMessage({
          content: i18n.global.t('actions.crg.cannotBindDuplicateCrgFile'),
        })
        return
      }

      const crgOption: biz.IOpenCrg = {
        file,
        showName,
        orientation: 'same', // 默认为同向
        mode: 'attached', // 默认为相对附着
        purpose: 'elevation', // 默认为高程
        sOffset: '0',
        tOffset: '0',
        zOffset: '0',
        zScale: '1',
        hOffset: '0',
      }

      // 如果透传了 crg 的 id 和版本，也需要保存
      if (id && version) {
        crgOption.id = id
        crgOption.version = version
      }

      road.crgConfig.push(crgOption)

      setRoad(roadId, road)

      return roadId
    },
    // 在一条目标道路上，更新 openCrg 配置
    updateCrg (params: {
      roadId: string
      file: string
      showName: string
      crgConfig: biz.IOpenCrg
    }) {
      const { roadId, file, showName, crgConfig } = params
      const road = getRoad(roadId)
      if (!road) return
      if (!road.crgConfig) return
      let index = -1
      // 通过 file 属性找到对应 crg 选项进行更新
      for (let i = 0; i < road.crgConfig.length; i++) {
        if (index > -1) continue
        const crgOption = road.crgConfig[i]
        // 通过 file 和 showName 完全一致来确认唯一性
        if (file === crgOption.file && showName === crgOption.showName) {
          index = i
        }
      }
      if (index === -1) return

      // 更新对应索引位置下的 crg 选项
      road.crgConfig[index] = crgConfig

      setRoad(roadId, road)

      return roadId
    },
    // 删除目标道路上的 openCrg 配置
    removeCrg (params: { roadId: string, file: string, showName: string }) {
      const { roadId, file, showName } = params
      const road = getRoad(roadId)
      if (!road) return
      if (!road.crgConfig) return
      let index = -1
      // 通过 file 属性找到对应 crg 选项进行更新
      for (let i = 0; i < road.crgConfig.length; i++) {
        if (index > -1) continue
        const crgOption = road.crgConfig[i]
        // 通过 file 和 showName 完全一致来确认唯一性
        if (file === crgOption.file && showName === crgOption.showName) {
          index = i
        }
      }
      if (index === -1) return

      // 删除数组中对应 crg 配置
      road.crgConfig.splice(index, 1)

      setRoad(roadId, road)
      return roadId
    },
    // 更新隧道的起始点或长度
    updateTunnelAttr (params: {
      roadId: string
      s: number
      length: number
      needRedraw?: boolean // 是否需要更新隧道元素
      saveRecord?: boolean // 是否需要记录日志
    }) {
      const { roadId, needRedraw = false, saveRecord = false } = params
      let { s, length } = params
      const road = getRoad(roadId)
      if (!road || road.roadType !== 'tunnel') return null
      const { length: roadLength } = road

      // 通过 editStore 判断是否有是双向道路，如果是，则需要更新另一条道路的隧道属性
      const editRoadStore = useEditRoadStore()
      const cp = editRoadStore.getControlPointByRoadId(roadId)
      if (!cp) return null

      // 将 s 坐标控制在 [0, roadLength] 区间
      s = Math.max(0, Math.min(roadLength, s))

      // 如果总和超过 roadLength，也需要调整隧道长度
      if (s + length > roadLength) {
        length = roadLength - s
      }
      // 如果计算的隧道长度属性小于 0，需要手动调整为 0
      if (length < 0) {
        length = 0
      }

      const [forwardRoadId] = cp.roadId
      const roadIds = []

      const fRoad = getRoad(forwardRoadId) as biz.ITunnel

      if (fRoad) {
        if (roadId === forwardRoadId) {
          // 如果当前道路是正向道路
          fRoad.tunnelS = fixedPrecision(s, 3)
          fRoad.tunnelLength = fixedPrecision(length, 3)
        } else {
          const s1 = Math.max(0, fRoad.length - s - length)
          fRoad.tunnelS = fixedPrecision(s1, 3)
          fRoad.tunnelLength = fixedPrecision(length, 3)
        }
        roadIds.push(forwardRoadId)
        setRoad(forwardRoadId, fRoad)
      }

      // 返回道路 id 数组
      return roadIds
    },
  },
})
