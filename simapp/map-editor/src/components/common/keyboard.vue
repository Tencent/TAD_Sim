<script setup lang="ts">
// 只执行逻辑，不渲染内容
import Mousetrap from 'mousetrap'
import { computed, onMounted, onUnmounted } from 'vue'
import { useHistoryStore } from '@/stores/history'
import { useHelperStore } from '@/stores/helper'
import { usePluginStore } from '@/stores/plugin'
import { useRoadInteractionStore } from '@/plugins/editRoad/store/interaction'
import { useJunctionInteractionStore } from '@/plugins/editJunction/store/interaction'
import { useLaneWidthInteractionStore } from '@/plugins/editLaneWidth/store/interaction'
import { useLaneNumberInteractionStore } from '@/plugins/editLaneNumber/store/interaction'
import { useLaneAttrInteractionStore } from '@/plugins/editLaneAttr/store/interaction'
import { useBoundaryInteractionStore } from '@/plugins/editBoundary/store/interaction'
import { useSignalControlInteractionStore } from '@/plugins/editSignalControl/store/interaction'
import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import { setSelected } from '@/main3d/controls/dragControls'
import root3d from '@/main3d'
import pluginManager from '@/plugins'
import { useFileStore } from '@/stores/file'
import { useObjectInteractionStore } from '@/plugins/editObject/store/interaction'
import type EditObjectHelper from '@/plugins/editObject/main3d'
import { useCircleRoadInteraction } from '@/plugins/editCircleRoad/store/interaction'
import { useCrgInteractionStore } from '@/plugins/editCrg/store/interaction'
import { warningMessage } from '@/utils/common'
import i18n from '@/locales'
import { useElevationInteractionStore } from '@/plugins/editElevation/store/interaction'
import root2d from '@/main2d/index'
import selectionBox from '@/main3d/selection/index'
import { getPermission } from '@/utils/permission'

defineOptions({
  name: 'SimKeyboard',
})
const enableCreate = getPermission('action.mapEditor.create.enable')
const enableOpen = getPermission('action.mapEditor.open.enable')
const enableSave = getPermission('action.mapEditor.save.enable')
const enableSaveAs = getPermission('action.mapEditor.saveAs.enable')

const fileStore = useFileStore()
const historyStore = useHistoryStore()
const helperStore = useHelperStore()
// 【TODO】 每个插件订阅全局的事件，再去执行各插件自定义的操作
// 其实插件模块、操作记录模块、文件模块都有判断并调用插件中交互状态的数据，并没有很好地解耦，后续可以研究
const roadInteractionStore = useRoadInteractionStore()
const junctionInteractionStore = useJunctionInteractionStore()
const laneWidthInteractionStore = useLaneWidthInteractionStore()
const laneNumberInteractionStore = useLaneNumberInteractionStore()
const laneAttrInteractionStore = useLaneAttrInteractionStore()
const boundaryInteractionStore = useBoundaryInteractionStore()
const objectInteractionStore = useObjectInteractionStore()
const signalControlInteractionStore = useSignalControlInteractionStore()
const linkInteractionStore = useLinkInteractionStore()
const circleRoadInteractionStore = useCircleRoadInteraction()
const crgInteractionStore = useCrgInteractionStore()
const elevationInteractionStore = useElevationInteractionStore()
const pluginStore = usePluginStore()

const shortcutKeysEnabled = computed(() => {
  return helperStore.shortcutKeysEnabled
})
const currentPlugin = computed(() => {
  return pluginStore.currentPlugin
})
const hasDiff = computed(() => {
  return historyStore.getDiffRecord().length > 0
})

/**
 * 开启对键盘快捷键的事件监听
 */
function addKeyboardListener () {
  Mousetrap.stopCallback = function () {
    return false
  }

  // TODO 如果对事件的绑定，影响了 html 标签默认的行为，需要做一定的处理

  // 撤销
  Mousetrap.bind('ctrl+z', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return
    historyStore.undo()
  })
  // 重做
  Mousetrap.bind('ctrl+y', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return
    historyStore.redo()
  })
  // 删除
  Mousetrap.bind('del', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return

    if (currentPlugin.value === 'editRoad') {
      roadInteractionStore && roadInteractionStore.removeCurrentSelected()
    } else if (currentPlugin.value === 'editJunction') {
      junctionInteractionStore &&
      junctionInteractionStore.removeCurrentSelectedJunction()
    } else if (currentPlugin.value === 'editLaneNumber') {
      // 通过快捷键删除车道
      laneNumberInteractionStore &&
      laneNumberInteractionStore.removeSelectedLane()
    } else if (currentPlugin.value === 'editLaneAttr') {
      // 通过快捷键删除车道
      laneAttrInteractionStore && laneAttrInteractionStore.removeSelectedLane()
    } else if (currentPlugin.value === 'editObject') {
      // 通过快捷键删除物体
      objectInteractionStore && objectInteractionStore.removeCurrentSelected()
    } else if (
      currentPlugin.value === 'editLink' &&
      getPermission('action.mapEditor.laneLink.delete.enable')
    ) {
      // 通过快捷键删除路口道路连接线
      linkInteractionStore && linkInteractionStore.removeSelectedJunctionLink()
    } else if (currentPlugin.value === 'editElevation') {
      // 如果在高程编辑模式，则删除删除选中的高程控制点
      root2d.removeControlPoint()
    }
  })
  // 删除（同上）
  Mousetrap.bind('backspace', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return

    if (currentPlugin.value === 'editRoad') {
      roadInteractionStore && roadInteractionStore.removeCurrentSelected()
    } else if (currentPlugin.value === 'editJunction') {
      junctionInteractionStore &&
      junctionInteractionStore.removeCurrentSelectedJunction()
    } else if (currentPlugin.value === 'editLaneNumber') {
      // 通过快捷键删除车道
      laneNumberInteractionStore &&
      laneNumberInteractionStore.removeSelectedLane()
    } else if (currentPlugin.value === 'editLaneAttr') {
      // 通过快捷键删除车道
      laneAttrInteractionStore && laneAttrInteractionStore.removeSelectedLane()
    } else if (currentPlugin.value === 'editObject') {
      // 通过快捷键删除物体
      objectInteractionStore && objectInteractionStore.removeCurrentSelected()
    } else if (
      currentPlugin.value === 'editLink' &&
      getPermission('action.mapEditor.laneLink.delete.enable')
    ) {
      // 通过快捷键删除路口道路连接线
      linkInteractionStore && linkInteractionStore.removeSelectedJunctionLink()
    } else if (currentPlugin.value === 'editElevation') {
      // 如果在高程编辑模式，则删除删除选中的高程控制点
      root2d.removeControlPoint()
    }
  })
  // 如果有选中的元素，取消选中
  Mousetrap.bind('esc', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return

    setSelected(null)
    root3d.setViewControlsEnabled(true)

    if (currentPlugin.value === 'editRoad') {
      roadInteractionStore && roadInteractionStore.unselectExceptSomeone()
      // 道路编辑模式存在预览放置元素，在按下 esc 键时需要销毁
      const plugin = pluginManager.getPlugin(currentPlugin.value)
      if (plugin) {
        plugin.editHelper.clear()
      }
      // 如果是拖拽框选多个控制点的过程中，按下 esc 键，需要取消选中
      selectionBox.onSelectEnd()
    } else if (currentPlugin.value === 'editJunction') {
      junctionInteractionStore && junctionInteractionStore.unselectJunction()
    } else if (currentPlugin.value === 'editLaneWidth') {
      laneWidthInteractionStore && laneWidthInteractionStore.unselectAll()
      // 车道宽度模式存在预览放置元素，在按下 esc 键时需要销毁
      const plugin = pluginManager.getPlugin(currentPlugin.value)
      if (plugin) {
        plugin.editHelper.clear()
      }
    } else if (currentPlugin.value === 'editLaneNumber') {
      laneNumberInteractionStore && laneNumberInteractionStore.unselectAll()
    } else if (currentPlugin.value === 'editLaneAttr') {
      laneAttrInteractionStore && laneAttrInteractionStore.unselectAll()
    } else if (currentPlugin.value === 'editBoundary') {
      boundaryInteractionStore && boundaryInteractionStore.unselectAll()
    } else if (currentPlugin.value === 'editObject') {
      objectInteractionStore && objectInteractionStore.unselectExceptSomeone()
      // 物体编辑模式存在预览放置元素，在按下 esc 键时需要销毁
      const plugin = pluginManager.getPlugin(currentPlugin.value)
      if (plugin) {
        // 清除拖拽过程中的辅助交互元素
        ;(plugin.editHelper as EditObjectHelper).clearVirtualElements()
      }
    } else if (currentPlugin.value === 'editSignalControl') {
      signalControlInteractionStore &&
      signalControlInteractionStore.unselectTrafficLight()
    } else if (currentPlugin.value === 'editLink') {
      linkInteractionStore && linkInteractionStore.unSelectAll()
    } else if (currentPlugin.value === 'editCircleRoad') {
      circleRoadInteractionStore && circleRoadInteractionStore.unselectRoad()
    } else if (currentPlugin.value === 'editCrg') {
      crgInteractionStore && crgInteractionStore.unselectRoad()
    } else if (currentPlugin.value === 'editElevation') {
      elevationInteractionStore && elevationInteractionStore.unselectRoad()
    }

    root3d.core.render()
  })
  // 新建场景
  Mousetrap.bind('ctrl+n', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return
    if (!enableCreate) return
    fileStore.createNew()
  })
  // 打开地图
  Mousetrap.bind('ctrl+o', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return
    if (!enableOpen) return
    fileStore.openMapList()
  })
  // 保存
  Mousetrap.bind('ctrl+s', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return
    if (!enableSave) return
    // 没有有效的操作记录，也不会保存
    if (!hasDiff.value) return
    // 处于 loading 状态，不能重复保存
    if (fileStore.isLoading) return
    // 不允许直接保存预设地图
    if (fileStore.isPreset) {
      warningMessage({
        content: i18n.global.t('desc.tips.cannotSavePresetMapFileDirectly'),
      })
      return
    }
    fileStore.saveMap()
  })
  // 另存为
  Mousetrap.bind('ctrl+shift+s', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return
    if (!enableSaveAs) return
    fileStore.saveMap(true)
  })

  // 退出（可能跟mac的退出应用快捷键保持一致？）
  Mousetrap.bind('ctrl+q', (event) => {
    event.preventDefault()
    if (!shortcutKeysEnabled.value) return undefined
    // TODO 网页版就不需要这个功能，需要做一定的判断
  })
}

/**
 * 移除对键盘快捷键的自定义事件监听
 */
function removeKeyboardListener () {
  Mousetrap.reset()
}

onMounted(() => {
  addKeyboardListener()
})
onUnmounted(() => {
  removeKeyboardListener()
})
</script>

<template>
  <slot />
</template>
