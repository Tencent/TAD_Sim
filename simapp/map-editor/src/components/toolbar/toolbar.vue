<script setup lang="ts">
import FileOperation from './fileOperation.vue'
import PluginList from './pluginList.vue'
import ViewOperation from './viewOperation.vue'
</script>

<template>
  <div class="toolbar">
    <FileOperation />
    <span class="divider" />
    <PluginList />
    <span class="divider" />
    <ViewOperation />
  </div>
</template>

<style scoped lang="less">
.toolbar {
  display: flex;
  width: 100%;
  height: 100%;
  background-color: var(--main-area-color);
  font-size: 14px;
  .divider {
    display: block;
    width: 1px;
    background-color: var(--input-disabled-border-color);
    margin: 8px 12px;
  }
}
</style>
