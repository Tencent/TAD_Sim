// 操作日志
import { defineStore } from 'pinia'
import { diff } from 'deep-object-diff'
import { ElMessage } from 'element-plus'
import { useJunctionStore } from './junction'
import { useRoadStore } from './road'
import { useObjectStore } from './object'
import { usePluginStore } from './plugin'
import { genUuid } from '@/utils/guid'
import { findElementById, getInitValueByName } from '@/utils/tools'
import { useEditRoadStore } from '@/plugins/editRoad/store/index'
import { useRoadInteractionStore } from '@/plugins/editRoad/store/interaction'
import { useJunctionInteractionStore } from '@/plugins/editJunction/store/interaction'
import { useLaneWidthInteractionStore } from '@/plugins/editLaneWidth/store/interaction'
import { useLaneNumberInteractionStore } from '@/plugins/editLaneNumber/store/interaction'
import { useLaneAttrInteractionStore } from '@/plugins/editLaneAttr/store/interaction'
import { useBoundaryInteractionStore } from '@/plugins/editBoundary/store/interaction'
import { useObjectInteractionStore } from '@/plugins/editObject/store/interaction'
import { useElevationInteractionStore } from '@/plugins/editElevation/store/interaction'
import { useSignalControlInteractionStore } from '@/plugins/editSignalControl/store/interaction'
import { getJunction, getObject, getRoad } from '@/utils/mapCache'
import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import { useCircleRoadInteraction } from '@/plugins/editCircleRoad/store/interaction'
import { useCrgInteractionStore } from '@/plugins/editCrg/store/interaction'
import i18n from '@/locales'

const MAX_STORAGE_COUNT = 40 // 最多可以存储多少条 historyState 的 data。超出的 data 为 空，且不可撤销。
let CANT_UNDO_REASON = '' // 不可撤销的原因，使用 ElMessage 进行提示

export interface IHistoryState {
  id: string
  name: string
  timestamp: number
  data: string
  diff: boolean
  roadId: Array<string>
  junctionId: Array<string>
  objectId: Array<string>
  exceedStorageLimit: boolean // 是否超出了 MAX_STORAGE_COUNT
}

interface IState {
  duringRevert: boolean // 是否处于撤销回退的状态
  records: Array<IHistoryState>
  currentIndex: number
  lastIndex: number
}

interface IAllState {
  roadState?: any
  junctionState?: any
  objectState?: any
  pluginState?: any
  editRoadState?: any
  roadInteractionState?: any
  junctionInteractionState?: any
  laneWidthInteractionState?: any
  laneNumberInteractionState?: any
  laneAttrInteractionState?: any
  boundaryInteractionState?: any
  objectInteractionState?: any
  elevationInteractionState?: any
  signalControlInteractionState?: any
  linkInteractionState?: any
  circleRoadInteractionState?: any
  crgInteractionState?: any
}

function getAllKeyInitState (): IAllState {
  const pluginStore = usePluginStore()
  const roadInteractionStore = useRoadInteractionStore()
  const junctionInteractionStore = useJunctionInteractionStore()
  const laneWidthInteractionStore = useLaneWidthInteractionStore()
  const laneNumberInteractionStore = useLaneNumberInteractionStore()
  const laneAttrInteractionStore = useLaneAttrInteractionStore()
  const boundaryInteractionStore = useBoundaryInteractionStore()
  const objectInteractionStore = useObjectInteractionStore()
  const elevationInteractionStore = useElevationInteractionStore()
  const signalControlInteractionStore = useSignalControlInteractionStore()
  const linkInteractionStore = useLinkInteractionStore()
  const circleRoadInteractionStore = useCircleRoadInteraction()
  const crgInteractionStore = useCrgInteractionStore()

  const pluginState = getInitValueByName(pluginStore.$id)
  const roadInteractionState = getInitValueByName(roadInteractionStore.$id)
  const junctionInteractionState = getInitValueByName(
    junctionInteractionStore.$id,
  )
  const laneWidthInteractionState = getInitValueByName(
    laneWidthInteractionStore.$id,
  )
  const laneNumberInteractionState = getInitValueByName(
    laneNumberInteractionStore.$id,
  )
  const laneAttrInteractionState = getInitValueByName(
    laneAttrInteractionStore.$id,
  )
  const boundaryInteractionState = getInitValueByName(
    boundaryInteractionStore.$id,
  )
  const objectInteractionState = getInitValueByName(objectInteractionStore.$id)
  const elevationInteractionState = getInitValueByName(
    elevationInteractionStore.$id,
  )
  const signalControlInteractionState = getInitValueByName(
    signalControlInteractionStore.$id,
  )
  const linkInteractionState = getInitValueByName(linkInteractionStore.$id)
  const circleRoadInteractionState = getInitValueByName(
    circleRoadInteractionStore.$id,
  )
  const crgInteractionState = getInitValueByName(crgInteractionStore.$id)

  let editRoadState = {}
  if (useEditRoadStore) {
    const editRoadStore = useEditRoadStore()
    if (editRoadStore) {
      editRoadState = getInitValueByName(editRoadStore.$id)
    }
  }

  return {
    roadState: {
      ids: [],
      elements: [],
    },
    junctionState: {
      ids: [],
      elements: [],
    },
    objectState: {
      ids: [],
      elements: [],
    },
    pluginState,
    editRoadState,
    roadInteractionState,
    junctionInteractionState,
    laneWidthInteractionState,
    laneNumberInteractionState,
    laneAttrInteractionState,
    boundaryInteractionState,
    objectInteractionState,
    elevationInteractionState,
    signalControlInteractionState,
    linkInteractionState,
    circleRoadInteractionState,
    crgInteractionState,
  }
}

function getAllKeyState (params?: {
  roads?: Array<biz.IRoad>
  junctions?: Array<biz.IJunction>
  objects?: Array<biz.IObject>
}): IAllState {
  const { roads = [], junctions = [], objects = [] } = params || {}

  const roadStore = useRoadStore()
  const junctionStore = useJunctionStore()
  const objectStore = useObjectStore()
  const pluginStore = usePluginStore()
  const roadInteractionStore = useRoadInteractionStore()
  const junctionInteractionStore = useJunctionInteractionStore()
  const laneWidthInteractionStore = useLaneWidthInteractionStore()
  const laneNumberInteractionStore = useLaneNumberInteractionStore()
  const laneAttrInteractionStore = useLaneAttrInteractionStore()
  const boundaryInteractionStore = useBoundaryInteractionStore()
  const objectInteractionStore = useObjectInteractionStore()
  const elevationInteractionStore = useElevationInteractionStore()
  const signalControlInteractionStore = useSignalControlInteractionStore()
  const linkInteractionStore = useLinkInteractionStore()
  const circleRoadInteractionStore = useCircleRoadInteraction()
  const crgInteractionStore = useCrgInteractionStore()

  const roadState = roadStore.$state
  const junctionState = junctionStore.$state
  const objectState = objectStore.$state

  // 通过 id 获取原始对象数据
  const _roads: Array<biz.ICommonElement> = []
  const _junctions: Array<biz.ICommonElement> = []
  const _objects: Array<biz.ICommonElement> = []
  for (const roadId of roadState.ids) {
    const road = getRoad(roadId)
    if (road) {
      // 先从 mapCache 的缓存中找，如果找到最好
      _roads.push(road)
    } else {
      // 如果是被删除的元素，从 mapCache 中已经找不到了，从调用 save 方法时透传的已删除的元素数组中找
      const element = findElementById(roads, roadId)
      if (element) {
        _roads.push(element)
      }
    }
  }
  for (const junctionId of junctionState.ids) {
    const junction = getJunction(junctionId)
    if (junction) {
      _junctions.push(junction)
    } else {
      const element = findElementById(junctions, junctionId)
      if (element) {
        _junctions.push(element)
      }
    }
  }
  for (const objectId of objectState.ids) {
    const object = getObject(objectId)
    if (object) {
      _objects.push(object)
    } else {
      const element = findElementById(objects, objectId)
      if (element) {
        _objects.push(element)
      }
    }
  }

  const pluginState = pluginStore.$state
  const roadInteractionState = roadInteractionStore.$state
  const junctionInteractionState = junctionInteractionStore.$state
  const laneWidthInteractionState = laneWidthInteractionStore.$state
  const laneNumberInteractionState = laneNumberInteractionStore.$state
  const laneAttrInteractionState = laneAttrInteractionStore.$state
  const boundaryInteractionState = boundaryInteractionStore.$state
  const objectInteractionState = objectInteractionStore.$state
  const elevationInteractionState = elevationInteractionStore.$state
  const signalControlInteractionState = signalControlInteractionStore.$state
  const linkInteractionState = linkInteractionStore.$state
  const circleRoadInteractionState = circleRoadInteractionStore.$state
  const crgInteractionState = crgInteractionStore.$state

  let editRoadState = {}
  if (useEditRoadStore) {
    const editRoadStore = useEditRoadStore()
    if (editRoadStore) {
      editRoadState = editRoadStore.$state
    }
  }

  return {
    roadState: {
      ids: roadState.ids,
      elements: _roads,
    },
    junctionState: {
      ids: junctionState.ids,
      elements: _junctions,
    },
    objectState: {
      ids: objectState.ids,
      elements: _objects,
    },
    pluginState,
    editRoadState,
    roadInteractionState,
    junctionInteractionState,
    laneWidthInteractionState,
    laneNumberInteractionState,
    laneAttrInteractionState,
    boundaryInteractionState,
    objectInteractionState,
    elevationInteractionState,
    signalControlInteractionState,
    linkInteractionState,
    circleRoadInteractionState,
    crgInteractionState,
  }
}

export const useHistoryStore = defineStore('history', {
  state: (): IState => ({
    duringRevert: false,
    records: [],
    currentIndex: -1,
    lastIndex: -1,
  }),
  getters: {
    // 用于展示的操作记录
    recordsOnDisplay: (state) => {
      // 如果存在撤销重做的操作，则使用截取的记录
      if (state.records.length < 1) return []
      if (state.duringRevert) {
        if (state.lastIndex === -1) {
          return []
        } else {
          return state.records.slice(0, state.lastIndex + 1)
        }
      }

      return state.records
    },
    // 是否允许撤销
    enableUndo: (state) => {
      CANT_UNDO_REASON = ''

      // 没有任何记录，或只有一条默认的操作记录，不能撤回
      if (state.currentIndex === -1 || state.currentIndex === 0) return false
      // 如果有撤回操作，且撤回到最开始默认的一条操作记录，不能继续撤回
      if (state.lastIndex === 0) return false

      if (state.duringRevert) {
        // 处于回退状态，已经撤回到 -1，即没有记录展示了，就不能继续撤回了
        if (state.lastIndex === -1) {
          return false
        }
      } else {
        // 不处于回退状态，只要 currentIndex 大于 -1，即有操作记录，就能回退
        if (state.lastIndex === -1) {
          return true
        }
      }

      if (state.records[state.lastIndex - 1].exceedStorageLimit) {
        CANT_UNDO_REASON = i18n.global.t('actions.tips.exceedUndoLimit')
        return false
      }

      return state.lastIndex <= state.currentIndex
    },
    // 是否允许重做
    enableRedo: (state) => {
      if (!state.duringRevert) return false
      return state.lastIndex < state.currentIndex
    },
    // 当前的操作记录中是否有可用的 diff 操作
    getDiffRecord () {
      return () => {
        // 操作记录根据当前展示的记录来判定，而不是所有支持撤销和重做的源记录
        const filterRes = this.recordsOnDisplay.filter(record => record.diff)
        return filterRes
      }
    },
  },
  actions: {
    /**
     * 记录当前数据状态
     * @param actionName
     * @param diff 该条记录是否用于diff，默认为 true
     */
    save (params: {
      title: string
      diff?: boolean // 默认操作记录都属于 diff
      roadId?: string | Array<string>
      junctionId?: string | Array<string>
      objectId?: string | Array<string>
      roads?: Array<biz.IRoad>
      junctions?: Array<biz.IJunction>
      objects?: Array<biz.IObject>
    }) {
      const {
        title,
        diff = true,
        roads = [],
        junctions = [],
        objects = [],
      } = params
      let { roadId = [], junctionId = [], objectId = [] } = params
      // 支持 id 字符串和数组的传入，统一转换成数组处理
      if (typeof roadId === 'string') {
        roadId = [roadId]
      }
      if (typeof junctionId === 'string') {
        junctionId = [junctionId]
      }
      if (typeof objectId === 'string') {
        objectId = [objectId]
      }

      const currentData = getAllKeyState({
        roads,
        junctions,
        objects,
      })
      // 组装成一条完整的记录数据
      const record: IHistoryState = {
        id: genUuid(),
        name: title,
        timestamp: new Date().getTime(),
        data: JSON.stringify(currentData),
        diff,
        roadId,
        junctionId,
        objectId,
        exceedStorageLimit: false,
      }

      // 只有在执行了新的操作后，才会解除回退状态
      // 需要将 lastIndex 后面的记录删掉，lastIndex 重置为 -1
      if (this.duringRevert) {
        this.duringRevert = false
        this.records.splice(this.lastIndex + 1)
        this.currentIndex = this.lastIndex
        this.lastIndex = -1
      }

      this.records.push(record)
      this.currentIndex++

      // records 太多时，只记录最新的 MAX_STORAGE_COUNT 条记录中的 data，之前记录中的 data 置空
      const diffCount = this.records.length - MAX_STORAGE_COUNT
      if (diffCount > 0) {
        this.records[diffCount - 1].exceedStorageLimit = true
        this.records[diffCount - 1].data = ''
      }
    },
    // 撤销
    undo () {
      if (!this.enableUndo) {
        CANT_UNDO_REASON &&
        ElMessage({
          type: 'warning',
          message: CANT_UNDO_REASON,
          grouping: true,
        })
        return
      }

      // 在允许撤销的前提下，获取上一次操作记录的索引
      if (this.lastIndex === -1) {
        this.lastIndex = this.currentIndex - 1
      } else {
        this.lastIndex--
      }

      // 一旦回退，就会进入回退状态
      if (!this.duringRevert) {
        this.duringRevert = true
      }

      this.syncState(this.lastIndex, true)
    },
    // 重做
    redo () {
      if (!this.enableRedo) return

      // 已经重做到最新的记录了，无法继续重做
      if (this.lastIndex === this.currentIndex) return

      this.lastIndex++

      this.syncState(this.lastIndex, false)
    },
    /**
     * 同步到操作记录指定的索引状态
     * @param index
     */
    syncState (index: number, isUndo: boolean) {
      let lastState
      if (index === -1) {
        // 回退到没有数据的状态，则用所有状态的默认值来进行同步
        lastState = getAllKeyInitState()
      } else {
        const lastRecord = this.records[index]
        lastState = JSON.parse(lastRecord.data)
      }

      // 如果是撤销操作，地图元素（roadId、junctionId、objectId）的增量，存在 index + 1 索引对应的记录中
      // 如果是重做操作，相当于从撤销的状态，回到带有 diff 数据的那条记录中，diff 数据的索引即为 index
      const diffMapIndex = isUndo ? index + 1 : index
      const diffMapRecord = this.records[diffMapIndex]
      // TODO 后续再补充 objectId
      const { roadId, junctionId, objectId } = diffMapRecord

      // 由于要对当前状态的属性做调整处理，做一次深拷贝以防篡改状态
      const currentState = getAllKeyState()
      console.time('diff')
      const diffContent: IAllState = diff(lastState, currentState)
      console.timeEnd('diff')

      // 获取增量要更新元素的信息
      for (const stateName in diffContent) {
        switch (stateName) {
          case 'roadState': {
            const roadStore = useRoadStore()

            // 对于地图元素级别的状态，透传更丰富的内容
            roadStore.applyState({
              // 透传保留有 elements 属性的数据
              last: lastState[stateName],
              diff: diffContent[stateName],
              lastDiffIds: roadId,
            })
            break
          }
          case 'junctionState': {
            const junctionStore = useJunctionStore()
            junctionStore.applyState({
              last: lastState[stateName],
              diff: diffContent[stateName],
              lastDiffIds: junctionId,
            })
            break
          }
          case 'objectState': {
            const objectStore = useObjectStore()
            objectStore.applyState({
              last: lastState[stateName],
              diff: diffContent[stateName],
              lastDiffIds: objectId,
            })
            break
          }
          case 'editRoadState': {
            const editRoadStore = useEditRoadStore()
            editRoadStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'pluginState': {
            const pluginStore = usePluginStore()
            pluginStore.applyState(lastState[stateName], diffContent[stateName])
            break
          }
          case 'roadInteractionState': {
            const roadInteractionStore = useRoadInteractionStore()
            roadInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'junctionInteractionState': {
            const junctionInteractionStore = useJunctionInteractionStore()
            junctionInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'laneWidthInteractionState': {
            const laneWidthInteractionStore = useLaneWidthInteractionStore()
            laneWidthInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'laneNumberInteractionState': {
            const laneNumberInteractionStore = useLaneNumberInteractionStore()
            laneNumberInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'laneAttrInteractionState': {
            const laneAttrInteractionStore = useLaneAttrInteractionStore()
            laneAttrInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'boundaryInteractionState': {
            const boundaryInteractionStore = useBoundaryInteractionStore()
            boundaryInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'objectInteractionState': {
            const objectInteractionStore = useObjectInteractionStore()
            objectInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'elevationInteractionState': {
            const elevationInteractionStore = useElevationInteractionStore()
            elevationInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'signalControlInteractionState': {
            const signalControlInteractionStore =
              useSignalControlInteractionStore()
            signalControlInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'linkInteractionState': {
            const linkInteractionStore = useLinkInteractionStore()
            linkInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'circleRoadInteractionState': {
            const circleRoadInteractionStore = useCircleRoadInteraction()
            circleRoadInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          case 'crgInteractionState': {
            const crgInteractionStore = useCrgInteractionStore()
            crgInteractionStore.applyState(
              lastState[stateName],
              diffContent[stateName],
            )
            break
          }
          default:
            break
        }
      }
    },
  },
})
