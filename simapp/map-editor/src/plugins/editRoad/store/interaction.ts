import { defineStore } from 'pinia'
import { useEditRoadStore } from './index'
import { registryStore } from '@/utils/tools'
import { useRoadStore } from '@/stores/road'

interface IState {
  roadId: string
  sectionId: string
  laneId: string
  // 由于道路参考线控制点涉及较多的操作，将对应的选中状态聚合到核心交互模块中
  refPoint: null | biz.IRefPoint
  refLineCpId: string // 当前选中的道路参考线对应的控制点集 id
  moreRefPoints: Array<biz.IRefPoint> // 框选的多个控制点【框选是一个单独的选中，只有在选中道路前提下才能框选】
  timestamp: number
}

const storeName = 'roadInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    sectionId: '',
    laneId: '',
    refPoint: null,
    refLineCpId: '',
    moreRefPoints: [],
    timestamp: 0,
  }
}

// 本地缓存的可交互元素的标识，方便通过变量 this[`unselect${flag}`] 调用自身 action
// Road 在 RefXXX 之前，能够在取消 road 选中状态时，还能拿到选中的 RefPoint 或 RefLine 的数据（如果有的话）
const cacheElementFlags = ['Road', 'RefPoint', 'RefLine']

registryStore(storeName, createInitValue)

// 地图元素交互状态
export const useRoadInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    selectedRefPoint (state) {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp

      if (!state.refPoint) return null
      const { id, parentId } = state.refPoint
      const editRoadStore = useEditRoadStore()
      const refPoint = editRoadStore.getRefPointById(parentId, id)
      if (!refPoint) return null

      return refPoint
    },
    // selectedRefPoint 的相邻点
    selectedAdjacentRefPoints (state) {
      if (!state.refPoint) return []
      const { id, parentId } = state.refPoint
      const editRoadStore = useEditRoadStore()
      return editRoadStore.getAdjacentRefPointsById(parentId, id)
    },
    currentRoadId (state) {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      return state.roadId
    },
    // 当前的交互选中状态，是否允许执行删除操作
    enableRemove (state) {
      // eslint-disable-next-line no-unused-expressions
      state.timestamp
      if (state.roadId || state.refPoint) return true

      return false
    },
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const { roadId, sectionId, laneId, refPoint, refLineCpId } = lastState
      this.roadId = roadId
      this.sectionId = sectionId
      this.laneId = laneId
      this.refPoint = refPoint
      this.refLineCpId = refLineCpId
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 删除当前选中的交互元素（需要针对不同的元素类型，调用不同的方法）
    async removeCurrentSelected () {
      if (this.roadId) {
        await this.removeCurrentRoad()
      } else if (this.refPoint) {
        await this.removeCurrentControlPoint()
      }
    },
    // 删除当前选中的道路
    async removeCurrentRoad (id: string = '') {
      // 如果没有透传指定的道路 id，则删除当前选中的道路
      const removeRoadId = id || this.roadId
      if (!removeRoadId) return null
      const roadStore = useRoadStore()
      const editRoadStore = useEditRoadStore()
      const removeRoad = roadStore.getRoadById(removeRoadId)
      if (!removeRoad) return null

      // 先取消选中
      this.unselectRoad()
      // 再对辅助元素和交互状态做处理
      editRoadStore.removeAllControlPoint(removeRoadId)
      // 获取删除道路之前连通的 junctionId
      let linkJunctionId: Array<string> = []
      if (removeRoad) {
        linkJunctionId = [...removeRoad.linkJunction]
      }

      // 再对 road 实际元素做删除
      const removeRoadRes = await roadStore.removeRoad(removeRoadId)
      if (!removeRoadRes) return null
      const { road, removeJunctionRes } = removeRoadRes

      return {
        elementId: removeRoadId,
        roadId: [removeRoadId],
        linkJunctionId,
        road,
        removeJunctionRes,
      }
    },
    async removeCurrentControlPoint () {
      if (!this.refPoint) return null
      const { id, parentId } = this.refPoint
      // 删除当前选中的道路参考线控制点
      const editRoadStore = useEditRoadStore()
      // 获取当前参考线控制点集对应的 road 连通的 junctionId，需要联动更新
      const cp = editRoadStore.getControlPointById(parentId)
      if (!cp) return null
      // 先对交互状态做处理，取消辅助参考点的选中状态
      this.unselectRefPoint()

      // 通过 removeControlPoint 方法来触发操作记录的保存
      editRoadStore.removeControlPoint(id, parentId)

      if (cp.points.length > 1) {
        // 如果当前控制点集的参考点数量大于等于 2 个，默认选中正向道路的 roadId
        // 保证道路有选中状态，能够支持后续其他交互
        this.selectRoad(cp.roadId[0])
      }
    },
    // 道路参考线相关
    selectRefLine (cpId: string) {
      this.unselectExceptSomeone('RefLine')

      if (this.refLineCpId === cpId) return false

      this.refLineCpId = cpId
      return true
    },
    unselectRefLine () {
      if (this.refLineCpId) {
        // 如果存在已选中的控制线，则在取消道路选中时，返回对应的参考线控制点集的 id
        const tempId = this.refLineCpId
        this.refLineCpId = ''
        return tempId
      }

      this.refLineCpId = ''
      return ''
    },
    // 道路参考线控制点相关
    selectRefPoint (refPoint: biz.IRefPoint) {
      this.unselectExceptSomeone('RefPoint')

      if (this.refPoint && this.refPoint.id === refPoint.id) return false

      this.refPoint = refPoint
      return true
    },
    // 选中多个控制点【多选控制点，不会清空之前道路或参考线的选中状态】
    selectMoreRefPoints (points: Array<biz.IRefPoint>) {
      this.moreRefPoints = points
      return true
    },
    // 取消多个控制点的选中
    unselectMoreRefPoints () {
      if (this.moreRefPoints.length > 0) {
        this.moreRefPoints.length = 0
        return true
      }

      this.moreRefPoints.length = 0
      return false
    },
    unselectRefPoint () {
      // 单个控制点的取消选中
      if (this.refPoint) {
        // 如果存在已选中的控制点，则在取消道路选中时，返回对应的参考线控制点集的 id
        const tempId = this.refPoint.parentId
        this.refPoint = null
        return tempId
      }
      // 多个控制点的取消选中
      if (this.moreRefPoints.length > 0) {
        const tempId = this.moreRefPoints[0].parentId
        this.moreRefPoints.length = 0
        return tempId
      }

      this.refPoint = null
      this.moreRefPoints.length = 0
      return ''
    },
    // 车道相关
    selectLane (roadId: string, sectionId: string, laneId: string) {
      this.unselectExceptSomeone('Road')
      if (
        this.roadId === roadId &&
        this.sectionId === sectionId &&
        this.laneId === laneId
      ) {
        return false
      }
      this.roadId = roadId
      this.sectionId = sectionId
      this.laneId = laneId
      return true
    },
    // 道路相关
    selectRoad (roadId: string) {
      this.unselectExceptSomeone('Road')
      if (this.roadId === roadId) return
      this.roadId = roadId
      // 只选中道路，就清空对应的 sectionId 和 laneId
      this.sectionId = ''
      this.laneId = ''
    },
    // 清空道路元素的选中状态
    unselectRoad () {
      // 判断是否有当前选中的参考线控制点集
      if (this.roadId) {
        // 如果存在已选中的道路，则在取消选中时，返回对应的参考线控制点集的 id
        const editRoadStore = useEditRoadStore()
        if (editRoadStore) {
          const cpData = editRoadStore.getControlPointByRoadId(this.roadId)
          if (cpData) {
            this.roadId = ''
            this.sectionId = ''
            this.laneId = ''

            return cpData.id
          }
        }
      }

      this.roadId = ''
      this.sectionId = ''
      this.laneId = ''
      return ''
    },
    /**
     * 取消其他元素的选中状态，可以提供某一个元素除外
     * @param name
     */
    unselectExceptSomeone (name?: string) {
      cacheElementFlags.forEach((flag) => {
        if (name && name === flag) return
        const actionName = `unselect${flag}`
        // @ts-expect-error
        this[actionName]()
      })
    },
  },
})
