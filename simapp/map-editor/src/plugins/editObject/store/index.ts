// object 编辑模式辅助元素相关的状态
import { defineStore } from 'pinia'
import { registryStore } from '@/utils/tools'

interface IState {}

const storeName = 'editObject'
function createInitValue (): IState {
  return {}
}

registryStore(storeName, createInitValue)

export const useEditSectionStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {},
  actions: {
    // 应用历史状态
    applyState (lastState: IState, diffState: IState) {},
  },
})
