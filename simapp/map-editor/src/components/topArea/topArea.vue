<script setup lang="ts">
import Logo from './logo.vue'
import Menu from './menu.vue'
import WindowMenu from './windowMenu.vue'
import { globalConfig } from '@/utils/preset'

// 只有是单机版环境，才会展示 electron 窗口控制相关的界面
const { isElectron, isCloud } = globalConfig
const enableDragWindow = isElectron
</script>

<template>
  <div class="top-area" :class="{ drag: enableDragWindow }">
    <Logo class="logo" />
    <Menu class="edit-menu" />
    <div class="empty-placeholder" />
    <WindowMenu v-if="isElectron && !isCloud" class="window-menu" />
  </div>
</template>

<style scoped lang="less">
.top-area {
  position: relative;
  width: 100%;
  height: 100%;
  display: flex;
  background-color: var(--main-dark-color);
  &.drag {
    -webkit-app-region: drag;
  }

  .empty-placeholder {
    flex-grow: 1;
  }
  .logo {
    -webkit-app-region: no-drag;
  }
  .edit-menu {
    -webkit-app-region: no-drag;
  }
  .window-menu {
    -webkit-app-region: no-drag;
  }
}
</style>
