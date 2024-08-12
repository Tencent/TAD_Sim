// 插件的状态管理
import { defineStore } from 'pinia'
import { useConfig3dStore } from './config3d'
import { registryStore } from '@/utils/tools'
import root3d from '@/main3d/index'
import { useHelperStore } from '@/stores/helper'

interface IState {
  prePlugin: '' | biz.IPluginName
  currentPlugin: '' | biz.IPluginName
}

const storeName = 'plugin'
function createInitValue (): IState {
  return {
    prePlugin: '',
    currentPlugin: '',
  }
}

registryStore(storeName, createInitValue)

export const usePluginStore = defineStore('plugin', {
  state: (): IState => {
    return createInitValue()
  },
  actions: {
    reset () {
      this.prePlugin = ''
      this.currentPlugin = ''
    },
    applyState (lastState: IState, diffState: IState) {
      const { prePlugin, currentPlugin } = lastState
      this.prePlugin = prePlugin
      this.currentPlugin = currentPlugin
    },
    activate (name: biz.IPluginName) {
      if (this.currentPlugin === name) return

      // 切换编辑模式，需要重置结构式的选中状态
      const helperStore = useHelperStore()
      helperStore.clearSelected()

      this.prePlugin = this.currentPlugin
      this.currentPlugin = name
      // 如果是切换到道路编辑模式
      if (name === 'editRoad') {
        const config3dStore = useConfig3dStore()
        // 且当前是透视视角，则需要强行切换为正交视角
        if (config3dStore.cameraType === 'P') {
          config3dStore.toggleCameraTypeOnly('O')
          root3d.updateCamera({ type: 'O' })
        }
      }
    },
  },
})
