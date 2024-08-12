import { cloneDeep } from 'lodash'
import { defineStore } from 'pinia'
import { Vector3 } from 'three'
import { useObjectStore } from '../object'
import { getObjectInJunction } from '../object/common'
import { useRoadStore } from '../road'
import { getRoadEndPoints } from '../road/road'
import { getLaneLinkUniqueFlag } from './link'
import { registryStore } from '@/utils/tools'
import {
  addJunctionLink,
  getJunction,
  getRoad,
  removeJunction,
  removeJunctionLink,
  setJunction,
} from '@/utils/mapCache'
import { genJunctionId, genLaneLinkId, genRoadId } from '@/utils/guid'
import {
  addCatmullromVirtualPoint,
  fixedPrecision,
  getBezier3WithDirection,
  getEndPointAlongVec3,
  getWorldAngle,
  transformArrayObjectToArray,
  transformOneCoordinateToCommon,
} from '@/utils/common3d'
import pool from '@/services/worker'
import { Constant } from '@/utils/business'

interface IState {
  ids: Array<string>
  elements: Array<biz.IJunction>
}

interface IConnectLinkRoad {
  junctionId: string
  linkRoad: Array<biz.ILinkRoad> | biz.ILinkRoad
}

const storeName = 'junction'
function createInitValue (): IState {
  return {
    ids: [],
    elements: [],
  }
}

registryStore(storeName, createInitValue)

export const useJunctionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    length: state => state.ids.length,
    getJunctionById: () => {
      return (junctionId: string) => getJunction(junctionId)
    },
    hasLinkRoad: () => {
      return (junctionId: string, linkRoad: string) => {
        const junction = getJunction(junctionId)
        if (!junction) return false

        return junction.linkRoads.includes(linkRoad)
      }
    },
    /**
     * 通过 linkRoad 来寻找是否有跟它绑定的路口（可能存在多个路口绑定同一个 linkRoad 的情况），返回第一个路口
     * @param state
     */
    getJunctionByLinkRoad: (state) => {
      return (linkRoad: string) => {
        for (const junctionId of state.ids) {
          const junction = getJunction(junctionId)
          if (!junction) continue
          const hasLinkRoad = junction.linkRoads.includes(linkRoad)
          if (hasLinkRoad) {
            return junction
          }
        }
        return null
      }
    },
  },
  actions: {
    /**
     * 将地图文件中解析的交叉路口状态同步到 store 中
     * @param newState
     */
    applyMapFileState (newState: IState) {
      return new Promise((resolve) => {
        const { ids } = newState
        this.ids = ids
        resolve(true)
      })
    },
    applyState (params: {
      last: IState
      diff: IState
      lastDiffIds: Array<string>
    }) {
      const { last: lastState, lastDiffIds } = params
      const { ids: lastIds, elements } = lastState
      elements &&
      elements.forEach((junction) => {
        if (junction.geoAttr) {
          junction.geoAttr.vertices = transformArrayObjectToArray(
            junction.geoAttr.vertices,
          )
          junction.geoAttr.indices = transformArrayObjectToArray(
            junction.geoAttr.indices,
          )
        }

        // 将上一个状态的 junction 对象保存到缓存中
        setJunction(junction.id, junction)
      })

      // 如果存在 diffIds，但上一个状态中id不存在，说明是上一个状态没有这个元素
      for (const diffId of lastDiffIds) {
        if (!lastState.ids.includes(diffId)) {
          removeJunction(diffId)
        }
      }
      // 将上一个状态，作为当前的状态
      this.ids = lastIds
    },
    /**
     * 更新当前交叉路口的数据（主要是闭合区域边界线采样点 + laneLink 的数据）
     * @param junction
     * @param isPureUpdate 是否是单纯地更新车道结构
     */
    async updateJunction (params: {
      junction: biz.IJunction
      isPureUpdate?: boolean
      isInit?: boolean
    }) {
      const { junction, isPureUpdate = false, isInit = false } = params
      const roadStore = useRoadStore()

      // 更新交叉路口自身形成的闭合区域边缘线采样点
      const { linkRoads } = junction
      const refRoads: Array<biz.IRefRoad> = []
      const refLaneInfo: Array<biz.ILaneLinkInfo> = []

      // 如果形成路口的连接道路为 2 条，则允许相近的端点作为路口区域边界关键点
      const enableClosePoint = linkRoads.length === 2

      for (const linkRoad of linkRoads) {
        const [roadId, percent, direction] = linkRoad.split('_')
        const road = roadStore.getRoadById(roadId)
        if (!road) continue
        const isTail = percent === '1'

        // 处理计算 laneLink 需要的数据
        const roadType = isTail ? 'end' : 'start'
        const { sections, keyPath } = road
        const targetSection = isTail ?
          sections[sections.length - 1] :
          sections[0]
        const { id: sectionId } = targetSection
        for (const lane of targetSection.lanes) {
          const { id: laneId, samplePoints } = lane
          const currentLaneDirection =
            Number(laneId) < 0 ? 'forward' : 'reverse'
          // 如果车道反向不一致，则跳过
          if (currentLaneDirection !== direction) continue
          const flag = getLaneLinkUniqueFlag(
            roadId,
            sectionId,
            laneId,
            roadType,
            direction,
          )
          // 车道在路口处的中心线采样端点
          const laneEndPoint = isTail ?
            samplePoints[samplePoints.length - 1] :
            samplePoints[0]
          // 车道在路口处的切线方向
          const defaultTangent = keyPath.getTangentAt(isTail ? 1 : 0)
          const tangent = new Vector3(0, 0, 0)
          tangent.x = isTail ? defaultTangent.x : -defaultTangent.x
          tangent.z = isTail ? defaultTangent.z : -defaultTangent.z
          tangent.normalize()

          refLaneInfo.push({
            flag,
            isTail,
            laneEndPoint,
            tangent,
            direction,
          })
        }

        // 处理路口几何体需要的数据
        // 计算当前道路在首/尾的左右车道边界线上的端点
        const { left: leftPoint, right: rightPoint } = getRoadEndPoints({
          road,
          isTail,
          direction, // 区分正反向车道
        })
        // 获取连接的道路端点侧数据
        refRoads.push({
          roadId,
          isTail,
          direction,
          alongVec: getEndPointAlongVec3(road.keyPath, isTail),
          leftPoint,
          rightPoint,
        })
      }

      // 将重新计算好的关联道路的边界数据缓存到源数据中
      // 方便后续有需要使用这部分数据的地方省去重新计算的逻辑
      junction.refRoads = refRoads

      // 封装成异步方法，跟计算 laneLink 同时执行
      const updateJunctionGeoAttr = async () => {
        // 如果交叉路口连接的道路数量小于 2，则无法形成一个路口的闭合区域
        if (junction.linkRoads.length < 2) {
          junction.geoAttr = null
          return
        }
        // // 通过 worker 线程来计算路口几何体的采样点
        const payload = {
          refBoundary: refRoads,
          enableClosePoint,
          isMultipleRoad: junction.linkRoads.length > 2,
        }
        const geoAttr = await pool.exec('getJunctionGeoAttr', [payload])
        // 用计算好的采样点结果更新当前交叉路口的边缘采样点数据
        if (!geoAttr) return
        const { vertices, indices } = geoAttr as biz.IGeoAttr
        junction.geoAttr = {
          vertices,
          indices,
        }
      }

      // 封装成异步方法，调用 worker 线程
      const updateLaneLinks = async () => {
        // 如果是加载地图文件的初始化，则不需要重新计算 lanelink 的数据
        if (isInit) return

        const payload = {
          laneInfos: refLaneInfo,
          originLaneLinks: junction.laneLinks,
        }
        const newLaneLinks = await pool.exec('getJunctionLaneLinks', [payload])
        newLaneLinks.forEach((laneLink: biz.ILaneLink) => {
          if (!laneLink.id) {
            // 在 webWorker 中计算出来的有效新增的 lanelink，需要赋值新的 lanelinkid，以及使用新的 roadid
            laneLink.id = genLaneLinkId()
            laneLink.roadid = genRoadId()
          }
        })
        junction.laneLinks = newLaneLinks
      }

      await Promise.all([updateJunctionGeoAttr(), updateLaneLinks()])

      // // 路口中的特殊路面标线，联动更新
      // const objectStore = useObjectStore()
      // const objectsInJunction = getObjectInJunction({
      //   junctionId: junction.id,
      //   ids: objectStore.ids,
      // })
      // for (const _object of objectsInJunction) {
      //   const { mainType, id: objectId } = _object
      //   if (mainType === 'roadSign') {
      //     objectStore.updateRoadSignByJunction(objectId)
      //   }
      // }

      // 将是否是单纯更新车道结构作为返回值，在渲染 dispatcher 逻辑中判断
      return isPureUpdate
    },
    /**
     * 将道路连接到交叉路口
     * @param params
     * @returns
     */
    async connectLinkRoad (params: IConnectLinkRoad) {
      let { junctionId = '', linkRoad } = params
      // 转换成数组统一处理
      if (!Array.isArray(linkRoad)) {
        linkRoad = [linkRoad]
      }
      const roadStore = useRoadStore()
      let junction
      // 是否需要新建一个 junction
      let isNew = false
      if (!junctionId) {
        // 新增一个 junction
        junctionId = genJunctionId()
        junction = {
          id: junctionId,
          laneLinks: [],
          linkRoads: [],
          refRoads: [],
          geoAttr: null,
        } as biz.IJunction
        isNew = true
      } else {
        // 基于已有的junction，添加连接的道路
        junction = getJunction(junctionId)
        if (!junction) return false
      }

      // 交叉路口 road 层面的数据
      for (const _linkRoad of linkRoad) {
        const [roadId] = _linkRoad.split('_')
        const road = getRoad(roadId)
        if (!road) continue
        // 在 junction 数据结构中存储连接道路的信息，也需要把连接状态同步到 road 的数据结构中
        roadStore.addRoadLinkJunction(roadId, junctionId)

        if (!junction.linkRoads.includes(_linkRoad)) {
          // 只添加不重复的关联道路数据
          junction.linkRoads.push(_linkRoad)
        }
      }

      // 更新交叉路口自身的结构
      await this.updateJunction({ junction })

      if (isNew) {
        this.ids.push(junction.id)
        setJunction(junction.id, junction)
      }

      // 将路口的 id 返回，用于在新建路口时调用该方法的逻辑获取新的路口 id
      return junction.id
    },

    /**
     * 将道路从交叉路口中断开
     */
    async disconnectLinkRoad (params: IConnectLinkRoad) {
      const { junctionId } = params
      let { linkRoad: removeLinkRoad } = params
      if (!junctionId) return false
      const junction = getJunction(junctionId)
      if (!junction) return false

      const roadStore = useRoadStore()

      // 将待删除的车道连接关系转换成数组统一处理
      if (!Array.isArray(removeLinkRoad)) {
        removeLinkRoad = [removeLinkRoad]
      }

      // 将待删除的连接道路，从道路和交叉路口各自的数据结构中删除
      for (const _removeLinkRoad of removeLinkRoad) {
        const [roadId] = _removeLinkRoad.split('_')
        const road = getRoad(roadId)
        if (!road) continue

        // 从 road 的数据结构中，将要断开的交叉路口数据删除
        roadStore.removeRoadLinkJunction(roadId, junctionId)

        // 获取待删除的连接道路数据在交叉路口数据中保存的索引
        const index = junction.linkRoads.findIndex(l => l === _removeLinkRoad)
        if (index > -1) {
          junction.linkRoads.splice(index, 1)
        }
      }

      if (junction.linkRoads.length > 1) {
        // 如果还有剩余的连接道路，则更新交叉路口自身的结构
        await this.updateJunction({ junction })
        return {
          junction,
        }
      } else {
        // 如果已经将连接的道路全部清空，则将当前交叉路口删除
        const removedJunctionRes = this.removeJunction(junctionId, true)
        return removedJunctionRes
      }
    },
    /**
     * 手动删除一个交叉路口
     * @param junctionId
     * @param callByRemoveLinkRoad true 表示清空道路连接关系跟随删除交叉路口，false 表示为主动先删除道路，再连带删除对应道路连接关系
     * @returns
     */
    removeJunction (junctionId: string, callByRemoveLinkRoad: boolean = false) {
      const junction = getJunction(junctionId)
      if (!junction) return false

      const removedJunction = cloneDeep(junction)
      // 如果不是由取消连接车道导致的 junction 没有任何关联，则属于主动删除，需要删除所有的道路连接关系
      if (!callByRemoveLinkRoad) {
        const roadStore = useRoadStore()
        const { linkRoads } = junction
        for (const linkRoad of linkRoads) {
          const [roadId] = linkRoad.split('_')
          const road = getRoad(roadId)
          if (!road) continue

          roadStore.removeRoadLinkJunction(roadId, junctionId)
          const index = linkRoads.findIndex(l => l === linkRoad)
          if (index > -1) {
            junction.linkRoads.splice(index, 1)
          }
        }
      }

      // 将交叉路口从数组中删除
      removeJunction(junctionId)
      const junctionIdIndex = this.ids.findIndex(id => id === junctionId)
      if (junctionIdIndex > -1) {
        this.ids.splice(junctionIdIndex, 1)
      }

      // 删除跟当前路口绑定的物体
      const objectStore = useObjectStore()
      const objectsInJunction = getObjectInJunction({
        junctionId,
        ids: objectStore.ids,
      })
      for (const _object of objectsInJunction) {
        const { mainType, id: objectId } = _object
        switch (mainType) {
          case 'pole':
            // 删除杆时，会联动删除杆上的物体（后续对于标志牌和信号灯的删除，其实没必要单独执行）
            objectStore.removePole(objectId, false)
            break
          case 'roadSign':
            objectStore.removeRoadSign(objectId, false)
            break
          case 'other':
            objectStore.removeOther(objectId, false)
            break
          case 'signalBoard':
            objectStore.removeSignalBoard(objectId, false)
            break
          case 'trafficLight':
            objectStore.removeTrafficLight(objectId, false)
            break
          default:
            break
        }
      }
      // 删除物体的 id 集合
      const objectId = objectsInJunction.map(objectData => objectData.id)

      return {
        junction: removedJunction,
        objectId,
        objects: objectsInJunction,
      }
    },
    /**
     * 删除一条道路连接线
     * @param junctionId : string
     * @param laneLinkId : string
     * @returns
     */
    removeJunctionLink (junctionId: string, laneLinkId: string) {
      const removedJunctionLink = { junctionId, laneLinkId }
      removeJunctionLink(removedJunctionLink)
    },
    /**
     * 为指定的 junction 添加一条 link
     * @param params:{
     *     junctionId:string 指定某条 junction 的 id
     *     laneLinkBaseInfo:Omit<
     *         biz.ILaneLink,
     *         'id' | 'samplePoints' | 'length' | 'enabled'
     *       >
     * }
     * @returns false | biz.ILaneLink 添加成功返回一条完整的 laneLink 信息，添加失败返回 false
     */
    addJunctionLink (params: {
      junctionId: string
      laneLinkBaseInfo: Omit<
        biz.ILaneLink,
        'id' | 'samplePoints' | 'length' | 'enabled'
      >
    }) {
      const { junctionId, laneLinkBaseInfo } = params
      const { fid, frid, fsid, ftype, tid, trid, tsid, ttype } =
        laneLinkBaseInfo

      const junction = getJunction(junctionId)
      if (!junction) return false

      const roadStore = useRoadStore()

      // 生成新的 laneLinkId
      const laneLinkId = genLaneLinkId()

      // 获取对应的道路端点和切线方向数据
      const fromRoad = getRoad(frid)
      const toRoad = getRoad(trid)
      if (!fromRoad || !toRoad) return false
      const queryFromLaneRes = roadStore.getLaneById(frid, fsid, fid)
      const queryToLaneRes = roadStore.getLaneById(trid, tsid, tid)
      if (!queryFromLaneRes || !queryToLaneRes) return false
      const { lane: fromLane } = queryFromLaneRes
      const { lane: toLane } = queryToLaneRes

      // 来路
      const isFromForward = Number(fid) < 0
      let fromTangent = new Vector3()
      let fromPoint = new Vector3()
      // from 道路只存在以下两种情况
      if (isFromForward && ftype === 'end') {
        fromTangent = fromRoad.keyPath.getTangentAt(1)
        fromTangent.y = 0
        fromTangent.normalize()
        fromPoint = fromLane.samplePoints[fromLane.samplePoints.length - 1]
      } else if (!isFromForward && ftype === 'start') {
        fromTangent = fromRoad.keyPath.getTangentAt(0)
        fromTangent.y = 0
        fromTangent.x = -fromTangent.x
        fromTangent.z = -fromTangent.z
        fromTangent.normalize()
        fromPoint = fromLane.samplePoints[0]
      }

      // 去路
      const isToForward = Number(tid) < 0
      let toTangent = new Vector3()
      let toPoint = new Vector3()
      // to 道路只存在以下两种情况
      if (isToForward && ttype === 'start') {
        toTangent = toRoad.keyPath.getTangentAt(0)
        toTangent.y = 0
        toTangent.x = -toTangent.x
        toTangent.z = -toTangent.z
        toTangent.normalize()
        toPoint = toLane.samplePoints[0]
      } else if (!isToForward && ttype === 'end') {
        toTangent = toRoad.keyPath.getTangentAt(1)
        toTangent.y = 0
        toTangent.normalize()
        toPoint = toLane.samplePoints[toLane.samplePoints.length - 1]
      }

      // 计算新增 lanelink 的贝塞尔曲线
      const { curve: laneLinkCurve } = getBezier3WithDirection({
        point1: fromPoint,
        direction1: fromTangent,
        direction2: toTangent,
        point2: toPoint,
      })

      // 车道连接线的长度
      const length = fixedPrecision(laneLinkCurve.getLength())
      // 车道连接线的采样点
      const samplePoints = laneLinkCurve.getSpacedPoints(
        Constant.laneLinkSegment,
      )

      const newLaneLink: biz.ILaneLink = {
        ...laneLinkBaseInfo,
        id: laneLinkId,
        length,
        samplePoints,
        roadid: genRoadId(),
        enabled: true,
        controlPoints: [],
      }

      // 由于后端咱们还没弄好 bezier 曲线的控制点解析，暂时用 catmullrom 类型控制点
      newLaneLink.controlType = 'catmullrom'
      // 获取 catmullrom 类型曲线的采样比例
      const percent = [0, 0.15, 0.5, 0.85, 1]
      const controlPoints: Array<biz.ICommonControlPoint> = []
      percent.forEach((_percent) => {
        const point = laneLinkCurve.getPointAt(_percent)
        const tangent = laneLinkCurve.getTangentAt(_percent)
        // 获取切线方向的世界坐标航向角
        const hdg = getWorldAngle(tangent)
        // 将控制点转换成平面坐标下的坐标
        const _point = transformOneCoordinateToCommon(point)
        controlPoints.push({
          ..._point,
          hdg: String(hdg),
        })
      })

      // 补充 catmullrom 线形道路的首尾2个虚拟控制点
      addCatmullromVirtualPoint(controlPoints)
      newLaneLink.controlPoints = controlPoints

      addJunctionLink({
        junctionId,
        laneLink: newLaneLink,
      })

      return newLaneLink
    },
  },
})
