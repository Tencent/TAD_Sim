<script setup lang="ts">
</script>

<template>
  <div class="add-model-button">
    <div class="model-image">
      <el-icon>
        <Plus />
      </el-icon>
    </div>
    <div class="model-item-title">
      {{ $t('operation.importModel') }}
    </div>
  </div>
</template>

<style scoped lang="less">
@import "@/assets/less/mixins";

.add-model-button {
  cursor: pointer;

  &:hover {
    .model-image {
      border-color: @active-font-color;
      color: @active-font-color;
    }

    .model-item-title {
      color: @active-font-color;
    }
  }

  .model-image {
    display: flex;
    justify-content: center;
    align-items: center;
    border: 1px dashed #383C43;
    border-radius: 6px;
    width: 80px;
    height: 80px;
  }

  .model-item-title {
    .text-overflow;
    padding-top: 10px;
    font-size: 12px;
    line-height: 1.5em;
    color: @title-font-color;
    text-align: center;
  }
}
</style>
