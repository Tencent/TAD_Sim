<script setup lang="ts">
import { computed } from 'vue'
import MenuItem from '../common/menuItem.vue'
import { useJunctionInteractionStore } from '@/plugins/editJunction/store/interaction'
import { useLaneAttrInteractionStore } from '@/plugins/editLaneAttr/store/interaction'
import { useLaneNumberInteractionStore } from '@/plugins/editLaneNumber/store/interaction'
import { useLinkInteractionStore } from '@/plugins/editLink/store/interaction'
import { useObjectInteractionStore } from '@/plugins/editObject/store/interaction'
import { useRoadInteractionStore } from '@/plugins/editRoad/store/interaction'
import { useHistoryStore } from '@/stores/history'
import { usePluginStore } from '@/stores/plugin'

const historyStore = useHistoryStore()
const pluginStore = usePluginStore()
const roadInteractionStore = useRoadInteractionStore()
const junctionInteractionStore = useJunctionInteractionStore()
const laneNumberInteractionStore = useLaneNumberInteractionStore()
const objectInteractionStore = useObjectInteractionStore()
const linkInteractionStore = useLinkInteractionStore()
const laneAttrInteractionStore = useLaneAttrInteractionStore()

const undoDisabled = computed(() => {
  return !historyStore.enableUndo
})
const redoDisabled = computed(() => {
  return !historyStore.enableRedo
})
const currentPlugin = computed(() => {
  return pluginStore.currentPlugin
})
// 根据当前各个编辑模式，判断是否支持删除选中元素
const removeDisabled = computed(() => {
  let disabled = true
  switch (currentPlugin.value) {
    case 'editRoad':
      if (roadInteractionStore.enableRemove) {
        disabled = false
      }
      break
    case 'editJunction':
      if (junctionInteractionStore.enableRemove) {
        disabled = false
      }
      break
    case 'editLaneAttr':
      if (laneAttrInteractionStore.enableRemove) {
        disabled = false
      }
      break
    case 'editLaneNumber':
      if (laneNumberInteractionStore.enableRemove) {
        disabled = false
      }
      break
    case 'editObject':
      if (objectInteractionStore.currentObject) {
        disabled = false
      }
      break
    case 'editLink':
      if (linkInteractionStore.enableRemove) {
        disabled = false
      }
      break
    default:
      break
  }

  return disabled
})

function undo () {
  // 调用数据层面的撤销
  historyStore.undo()
}
function redo () {
  // 调用数据层面的重做
  historyStore.redo()
}
// 删除当前选中的元素
function deleteCurrentSelected () {
  // 需要判断不同编辑模式
  switch (currentPlugin.value) {
    case 'editRoad':
      roadInteractionStore && roadInteractionStore.removeCurrentSelected()
      break
    case 'editJunction':
      junctionInteractionStore &&
      junctionInteractionStore.removeCurrentSelectedJunction()
      break
    case 'editLaneNumber':
      laneNumberInteractionStore &&
      laneNumberInteractionStore.removeSelectedLane()
      break
    case 'editLaneAttr':
      laneAttrInteractionStore && laneAttrInteractionStore.removeSelectedLane()
      break
    case 'editObject':
      objectInteractionStore && objectInteractionStore.removeCurrentSelected()
      break
    case 'editLink':
      linkInteractionStore && linkInteractionStore.removeSelectedJunctionLink()
      break
    default:
      break
  }
}
</script>

<template>
  <div>
    <BaseAuth :perm="['action.mapEditor.undo.enable']">
      <MenuItem
        :disabled="undoDisabled"
        :title="$t('menu.undo')"
        hotkey="Ctrl+Z"
        @trigger="undo"
      />
    </BaseAuth>
    <BaseAuth :perm="['action.mapEditor.redo.enable']">
      <MenuItem
        :disabled="redoDisabled"
        :title="$t('menu.redo')"
        hotkey="Ctrl+Y"
        @trigger="redo"
      />
    </BaseAuth>
    <MenuItem
      :disabled="removeDisabled"
      :title="$t('menu.delete')"
      hotkey="Delete"
      @trigger="deleteCurrentSelected"
    />
  </div>
</template>

<style scoped lang="less"></style>
