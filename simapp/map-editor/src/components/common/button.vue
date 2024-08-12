<script setup lang="ts"></script>

<template>
  <el-button class="sim-button" v-bind="$attrs">
    <slot />
  </el-button>
</template>

<style scoped lang="less">
.sim-button {
  font-size: 12px;
  cursor: pointer;
  padding: 4px 12px;
  color: #9b9b9b;
  background-color: var(--component-area-color);
  border: 1px solid var(--property-border-color);
  border-radius: 0;
  height: 24px;
  line-height: 24px;
  &:hover {
    color: var(--active-color);
    background-color: var(--main-area-color);
    border-color: var(--active-color);
  }
  &:active,
  &:focus {
    color: #9b9b9b;
    background-color: var(--component-area-color);
    border-color: var(--property-border-color);
  }
  &:focus-visible {
    outline: none;
  }

  &.primary {
    color: #242424;
    background-color: var(--active-color);
    border-color: var(--active-color);

    &:hover {
      background-color: #67b7c4;
      border-color: #67b7c4;
    }
    &:active {
      color: #242424;
      background-color: #67b7c4;
      border-color: #67b7c4;
    }
  }
}
</style>
