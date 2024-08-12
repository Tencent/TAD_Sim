import { defineStore } from 'pinia'
import { registryStore } from '@/utils/tools'

interface IState {
  junctionId: string
  hTPointFlag: string
  laneLinkId: string
  timestamp: number
}

const storeName = 'linkInteraction'

function createInitValue (): IState {
  return {
    junctionId: '',
    hTPointFlag: '',
    laneLinkId: '',
    timestamp: 0,
  }
}

registryStore(storeName, createInitValue)

export const useLinkInteractionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {
    currentJunctionId: ({ junctionId, timestamp }) => {
      // 无实际意义， 只为触发 timestamp getter 让 currentJunctionId 更新
      // eslint-disable-next-line no-unused-expressions
      timestamp
      return junctionId
    },
    currentHtTPointFlag: ({ hTPointFlag, timestamp }) => {
      // 无实际意义， 只为触发 timestamp getter 让 currentHtTPointFlag 更新
      // eslint-disable-next-line no-unused-expressions
      timestamp
      return hTPointFlag
    },
    currentLaneLinkId: ({ laneLinkId, timestamp }) => {
      // 无实际意义， 只为触发 timestamp getter 让 currentLaneLinkId 更新
      // eslint-disable-next-line no-unused-expressions
      timestamp
      return laneLinkId
    },
    // 当前是否允许执行删除操作
    enableRemove: (state) => {
      if (state.laneLinkId) return true
      return false
    },
  },
  actions: {
    // 应用操作记录中的状态
    applyState (lastState: IState, diffState: IState) {
      const { junctionId, hTPointFlag, laneLinkId } = lastState
      this.junctionId = junctionId

      // undo / redo 暂时不需要高亮 link 或是高亮辅助点，因此这里取消选中，将他们置空
      this.hTPointFlag = ''
      this.laneLinkId = ''
    },
    updateTimestamp () {
      this.timestamp = Date.now()
    },
    selectJunction (junctionId: string) {
      if (junctionId === this.junctionId) return false
      this.junctionId = junctionId
      return junctionId
    },
    unSelectJunction () {
      this.junctionId = ''
    },
    // -------- hTPoint 操作 --------
    selectHTPoint (hTPointFlag: string) {
      // 如果当前处于 laneLink 选中状态，则添加连接线
      if (this.laneLinkId) {
        this.unSelectJunctionLink()
      }

      // 如果当前没有处于点选状态，则进入点选状态
      if (!this.hTPointFlag) {
        this.hTPointFlag = hTPointFlag
        return true
      }

      if (hTPointFlag === this.hTPointFlag) {
        // 如果处于点选状态且选择了相同的点，则取消当前选择点
        this.unSelectHTPoint()
        return false
      } else {
        // 如果处于点选状态且选择了不同的点，则添加在俩点之间添加link
        this.addJunctionLink(hTPointFlag)
        return false
      }
    },
    unSelectHTPoint () {
      this.hTPointFlag = ''
    },
    // -------- laneLink 操作 --------
    selectJunctionLink (laneLinkId: string) {
      // 如果当前处理 point 选中状态， 则先取消选择
      if (this.hTPointFlag) {
        this.unSelectHTPoint()
      }

      if (laneLinkId === this.laneLinkId) {
        this.unSelectJunctionLink()
        return false
      }
      this.laneLinkId = laneLinkId
      return true
    },
    unSelectJunctionLink () {
      this.laneLinkId = ''
    },
    unSelectAllButJunction () {
      this.hTPointFlag && this.unSelectHTPoint()
      this.laneLinkId && this.unSelectJunctionLink()
    },
    removeSelectedJunctionLink () {
      if (!this.laneLinkId) return false
      return this.laneLinkId
    },
    addJunctionLink (pointFlagReadyToLink: string) {
      return { curSelectedPointFlag: this.hTPointFlag, pointFlagReadyToLink }
    },
    unSelectAll () {
      this.unSelectAllButJunction()
      this.unSelectJunction()
    },
  },
})
