<script setup lang="ts">
import StructureTree from './structureTree.vue'
import Property from './property.vue'
</script>

<template>
  <div class="right-area">
    <StructureTree />
    <Property />
  </div>
</template>

<style scoped lang="less">
.right-area {
  height: 100%;
  display: flex;
  flex-direction: column;
  position: relative;
}
</style>
