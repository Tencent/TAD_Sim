<script setup lang="ts">
// @ts-nocheck
import { computed } from 'vue'
import SimButton from '../common/button.vue'
import QuerySvg from './icon/quemark.svg?component'
import { useHelperStore } from '@/stores/helper'

// TODO 退出时前置弹出，需要控制该属性
const props = defineProps({
  isQuit: {
    type: Boolean,
    default: false,
  },
})

const helperStore = useHelperStore()

// 地图未保存弹窗，由于没有固定的触发交互，判断当前弹窗可见时，禁用键盘热键
const visible = computed(() => {
  if (helperStore.mapNotSaveDialogShow) {
    // 弹窗展示后，禁用热键
    helperStore.closeShortcutKeys()
  }
  return helperStore.mapNotSaveDialogShow
})

// 关闭弹窗
function handleClose () {
  helperStore.toggleDialogStatus('mapNotSave', false)
  helperStore.openShortcutKeys()
}

// 强制舍弃当前场景
function ForceDiscardScene () {
  if (props.isQuit) {
    // TODO 如果是退出编辑器，地图未保存
  } else {
    // 执行确认的回调
    if (helperStore.mapNotSavaConfirmCallback) {
      helperStore.mapNotSavaConfirmCallback()
      helperStore.setMapNotSaveConfirmCallback(null)
    }
    // 清空结束后退出弹窗
    handleClose()
  }
}
</script>

<template>
  <el-dialog
    v-model="visible"
    width="260px"
    class="tip-dialog"
    :close-on-click-modal="false"
    :show-close="false"
    :destroy-on-close="true"
    :close-on-press-escape="false"
    align-center
  >
    <div class="title">
      <el-icon size="24px">
        <QuerySvg />
      </el-icon>
      <span>{{ $t('desc.tips.tips') }}</span>
    </div>
    <div class="tips">
      {{ $t('actions.tips.mapNotSaveQuitWillLose') }}
    </div>
    <div class="button-part">
      <SimButton @click="handleClose">
        {{ $t('desc.tips.cancel') }}
      </SimButton>
      <SimButton class="primary" @click="ForceDiscardScene">
        {{
          $t('desc.tips.confirm')
        }}
      </SimButton>
    </div>
  </el-dialog>
</template>

<style lang="less"></style>
