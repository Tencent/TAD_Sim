<script setup lang="ts">
// @ts-nocheck
import SelectSvg from './viewIcon/map-select.svg?component'
import MoveSvg from './viewIcon/map-move.svg?component'
import RotateSvg from './viewIcon/map-rotate.svg?component'
</script>

<template>
  <div class="view-interaction-container">
    <span class="icon-container left">
      <SelectSvg class="icon" />
    </span>
    <span class="icon-container center">
      <MoveSvg class="icon" />
    </span>
    <span class="icon-container right">
      <RotateSvg class="icon" />
    </span>
  </div>
</template>

<style scoped lang="less">
.view-interaction-container {
  position: absolute;
  top: 32px;
  right: 12px;

  display: flex;

  .icon-container {
    width: 28px;
    height: 24px;
    display: flex;
    align-items: center;
    justify-content: center;
    background-color: var(--main-area-color);
    cursor: pointer;

    .icon {
      width: 12px;
      height: 12px;
    }
  }

  --border-radius: 4px;

  .left {
    border-top-left-radius: var(--border-radius);
    border-bottom-left-radius: var(--border-radius);
  }
  .right {
    border-top-right-radius: var(--border-radius);
    border-bottom-right-radius: var(--border-radius);
  }
  .center {
  }
}
</style>
