<script setup lang="ts">
import { computed } from 'vue'
import MenuItem from '../common/menuItem.vue'
import { useFileStore } from '@/stores/file'
import { globalConfig } from '@/utils/preset'
import { useHistoryStore } from '@/stores/history'
import { warningMessage } from '@/utils/common'
import i18n from '@/locales'

const fileStore = useFileStore()
const historyStore = useHistoryStore()
const { isElectron } = globalConfig
// 根据操作记录，来判断当前是否有diff数据，即是否能保存
const hasDiff = computed(() => {
  return historyStore.getDiffRecord().length > 0
})

function handleCreateNew () {
  // 新建场景
  fileStore.createNew()
}
async function handleOpenMap () {
  fileStore.openMapList()
}
// 保存地图
async function handleSaveMap () {
  // 如果没有有效的操作记录，则不会触发
  if (!hasDiff.value) return
  // 如果处于loading状态，不能重复执行保存操作
  if (fileStore.isLoading) return
  // 不允许直接保存预设地图
  if (fileStore.isPreset) {
    warningMessage({
      content: i18n.global.t('desc.tips.cannotSavePresetMapFileDirectly'),
    })
    return
  }

  fileStore.saveMap()
}
// 另存为新地图
function handleSaveAsNewMap () {
  // 另存为需要传 true 参数
  fileStore.saveMap(true)
}
function quit () {
  // 退出
  fileStore.quit()
}
</script>

<template>
  <div>
    <BaseAuth :perm="['action.mapEditor.create.enable']">
      <MenuItem
        :title="$t('menu.new')"
        hotkey="Ctrl+N"
        @trigger="handleCreateNew"
      />
    </BaseAuth>
    <BaseAuth :perm="['action.mapEditor.open.enable']">
      <MenuItem
        :title="$t('menu.open')"
        hotkey="Ctrl+O"
        @trigger="handleOpenMap"
      />
    </BaseAuth>
    <BaseAuth :perm="['action.mapEditor.save.enable']">
      <MenuItem
        :title="$t('menu.save')"
        :disabled="!hasDiff"
        hotkey="Ctrl+S"
        @trigger="handleSaveMap"
      />
    </BaseAuth>
    <BaseAuth :perm="['action.mapEditor.saveAs.enable']">
      <MenuItem
        :title="$t('menu.saveAs')"
        hotkey="Ctrl+Shift+S"
        @trigger="handleSaveAsNewMap"
      />
    </BaseAuth>
    <!-- 只在单机版环境，才会在该菜单栏中提供退出按钮 -->
    <MenuItem
      v-if="isElectron"
      :title="$t('menu.quit')"
      hotkey="Ctrl+Q"
      @trigger="quit"
    />
  </div>
</template>

<style scoped lang="less"></style>
