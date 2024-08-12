<script setup lang="ts">
import { onMounted, onUnmounted, ref } from 'vue'
import root3d from '@/main3d'
import ee, { CustomEventType } from '@/utils/event'
import { disposeMainProcess, initMainProcess } from '@/services/mainProcess'

// webgl 渲染容器的 dom 元素
const webglContainerRef = ref()

onMounted(() => {
  // 初始化 webgl 核心元素
  root3d.init({
    dom: webglContainerRef.value,
  })

  initMainProcess()

  document.addEventListener('pointerdown', onPointerdown)
  document.addEventListener('contextmenu', onContextMenu)
  webglContainerRef.value.addEventListener('pointermove', onPointerMove)
})

onUnmounted(() => {
  disposeMainProcess()

  document.removeEventListener('pointerdown', onPointerdown)
  document.removeEventListener('contextmenu', onContextMenu)
  webglContainerRef.value.removeEventListener('pointermove', onPointerMove)
})

function onPointerdown () {
  ee.emit(CustomEventType.window.pointerdown)
}

function onContextMenu () {
  ee.emit(CustomEventType.window.contextmenu)
}

function onPointerMove (event: PointerEvent) {
  ee.emit(CustomEventType.webglArea.pointermove, event)
}
</script>

<template>
  <div ref="webglContainerRef" v-my-drop class="webgl-container" />
</template>

<style scoped lang="less">
.webgl-container {
  position: relative;
  flex: 1;
  min-height: 0;
}
</style>
