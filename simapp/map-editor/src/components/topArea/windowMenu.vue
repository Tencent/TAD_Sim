<script setup lang="ts">
// @ts-nocheck
import { get } from 'lodash'
import { onMounted, onUnmounted, ref } from 'vue'
import MinWindowSvg from './windowMenuIcon/minwindow.svg?component'
import MaxWindowSvg from './windowMenuIcon/maxwindow.svg?component'
import RestoreWindowSvg from './windowMenuIcon/restore.svg?component'
import CloseWindowSvg from './windowMenuIcon/closewindow.svg?component'
import { useFileStore } from '@/stores/file'

// 获取 electron 环境对象
const fileStore = useFileStore()
const focusedBrowserWindow = get(window, 'electron.focusedBrowserWindow')
const crossBrowserWindowMessage = get(
  window,
  'electron.crossBrowserWindowMessage',
)
const isMaximized = ref(false)

onMounted(() => {
  // 监听跨窗口派发的事件
  if (crossBrowserWindowMessage) {
    crossBrowserWindowMessage.on('map-editor-close', quit)
  }
})
onUnmounted(() => {
  // 销毁组件时取消监听
  if (crossBrowserWindowMessage) {
    crossBrowserWindowMessage.off('map-editor-close', quit)
  }
})

// 缩小窗口
function minimum () {
  if (!focusedBrowserWindow) return
  focusedBrowserWindow.minimize()
}
// 切换全屏
async function toggleMaximum () {
  if (!focusedBrowserWindow) return
  const _isMaximized = await focusedBrowserWindow.isMaximized()
  if (_isMaximized) {
    focusedBrowserWindow.unmaximize()
    isMaximized.value = false
  } else {
    focusedBrowserWindow.maximize()
    isMaximized.value = true
  }
}
// 退出应用
function quit () {
  fileStore.quit()
}
</script>

<template>
  <ul class="window-btn-ul">
    <li class="window-btn-li" @click="minimum">
      <MinWindowSvg />
    </li>
    <li class="window-btn-li" @click="toggleMaximum">
      <RestoreWindowSvg v-if="isMaximized" />
      <MaxWindowSvg v-else />
    </li>
    <li class="window-btn-li" @click="quit">
      <CloseWindowSvg />
    </li>
  </ul>
</template>

<style scoped lang="less">
.window-btn-ul {
  display: flex;
  align-items: center;

  .window-btn-li {
    cursor: pointer;
    width: 40px;
    height: 40px;
    display: flex;
    align-items: center;
    justify-content: center;
    &:hover {
      background-color: var(--property-button-active-bg);
    }
  }
}
</style>
