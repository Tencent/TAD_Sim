import { defineStore } from 'pinia'
import { registryStore } from '@/utils/tools'

interface IState {
  roadId: string
  sectionId: string
  boundaryId: string
  // 每次更新车道宽度，都更新时间戳，保证撤销和重做能够 diff 到差异执行 applyState 方法
  timestamp: number
}

const storeName = 'boundaryInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    sectionId: '',
    boundaryId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useBoundaryInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const { roadId, sectionId, boundaryId } = lastState
      this.roadId = roadId
      this.sectionId = sectionId
      this.boundaryId = boundaryId
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    // 车道线级别
    selectBoundary (boundaryId: string) {
      if (this.boundaryId === boundaryId) return false
      this.boundaryId = boundaryId
      return true
    },
    unselectBoundary () {
      const lastBoundaryId = this.boundaryId
      this.boundaryId = ''
      return lastBoundaryId
    },
    // section 级别
    selectSection (params: { roadId: string, sectionId: string }) {
      const { roadId, sectionId } = params
      if (this.roadId === roadId && this.sectionId === sectionId) return null
      const lastRoadId = this.roadId
      const lastSectionId = this.sectionId
      this.roadId = roadId
      this.sectionId = sectionId
      return {
        roadId: lastRoadId,
        sectionId: lastSectionId,
      }
    },
    unselectSection () {
      const lastRoadId = this.roadId
      const lastSectionId = this.sectionId
      this.roadId = ''
      this.sectionId = ''
      return {
        roadId: lastRoadId,
        sectionId: lastSectionId,
      }
    },
    unselectAll () {
      this.unselectSection()
      this.unselectBoundary()
    },
  },
})
