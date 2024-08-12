<script setup lang="ts">
// @ts-nocheck
import { ref, watch } from 'vue'
import SimButton from '../common/button.vue'
import QuerySvg from './icon/quemark.svg?component'

const props = defineProps({
  modelValue: {
    type: Boolean,
    default: false,
  },
})
const emit = defineEmits(['cancel', 'confirm'])
const visible = ref(props.modelValue)
watch(
  () => props.modelValue,
  (newVal) => {
    visible.value = newVal
  },
  {
    immediate: true,
  },
)
// 关闭弹窗
function cancel () {
  emit('cancel')
}
// 确认
function confirm () {
  emit('confirm', true)
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
    append-to-body
    align-center
  >
    <div class="title">
      <el-icon size="24px">
        <QuerySvg />
      </el-icon>
      <span>{{ $t('desc.tips.tips') }}</span>
    </div>
    <div class="tips">
      {{ $t('actions.file.confirmToDeleteMap') }}
    </div>
    <div class="button-part">
      <SimButton @click="cancel">
        {{ $t('desc.tips.cancel') }}
      </SimButton>
      <SimButton class="primary" @click="confirm">
        {{
          $t('desc.tips.confirm')
        }}
      </SimButton>
    </div>
  </el-dialog>
</template>

<style lang="less"></style>
