import { defineStore } from 'pinia'
import { registryStore } from '@/utils/tools'

interface IState {
  roadId: string
  sectionId: string
  laneId: string
}

const storeName = 'sectionInteraction'
function createInitValue (): IState {
  return {
    roadId: '',
    sectionId: '',
    laneId: '',
  }
}

registryStore(storeName, createInitValue)

export const useSectionInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  actions: {
    applyState (lastState: IState, diffState: IState) {},
    // 车道相关
    selectLane (roadId: string, sectionId: string, laneId: string) {
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
    unselectLane () {
      this.roadId = ''
      this.sectionId = ''
      this.laneId = ''
    },
    // 道路相关
    selectRoad (roadId: string) {
      if (this.roadId === roadId) return
      this.roadId = roadId
      // 只选中道路，就清空对应的 sectionId 和 laneId
      this.sectionId = ''
      this.laneId = ''
    },
  },
})
