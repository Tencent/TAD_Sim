<script setup lang="ts">
import { onMounted, onUnmounted, ref, watch } from 'vue'
import { useHelperStore } from '@/stores/helper'
import ee, { CustomEventType } from '@/utils/event'

const helperStore = useHelperStore()

const visible = ref(false)
const top = ref(0)
const left = ref(0)

watch(
  () => helperStore.contextMenuShow,
  (val) => {
    visible.value = val
  },
  {
    immediate: true,
    deep: true,
  },
)
watch(
  () => helperStore.contextMenuTop,
  (val) => {
    top.value = val
  },
  {
    immediate: true,
    deep: true,
  },
)
watch(
  () => helperStore.contextMenuLeft,
  (val) => {
    left.value = val
  },
  {
    immediate: true,
    deep: true,
  },
)

function clear () {
  if (!visible.value) return
  helperStore.hideContextMenu()
}

function removeElement () {
  if (helperStore.removeElementId) {
    ee.emit(CustomEventType.element.remove, helperStore.removeElementId)
  }
  clear()
}

onMounted(() => {
  ee.on(CustomEventType.window.pointerdown, clear)
  ee.on(CustomEventType.window.contextmenu, clear)
})

onUnmounted(() => {
  ee.off(CustomEventType.window.pointerdown, clear)
  ee.off(CustomEventType.window.contextmenu, clear)
})
</script>

<template>
  <div
    class="context-menu"
    :style="{
      display: visible ? 'block' : 'none',
      top: `${top}px`,
      left: `${left}px`,
    }"
  >
    <div class="item" @pointerdown.prevent.stop="removeElement">
      {{ $t('menu.delete') }}
    </div>
  </div>
</template>

<style scoped lang="less">
.context-menu {
  position: absolute;
  z-index: 1001;
  width: 80px;
  background-color: var(--component-area-color);
  padding: 4px 0;
  border: 1px solid var(--dialog-area-color);

  .item {
    height: 20px;
    padding: 0 4px;
    line-height: 20px;
    color: var(--text-color);
    cursor: pointer;
    &:hover {
      background-color: var(--dialog-area-color);
    }
  }
}
</style>
