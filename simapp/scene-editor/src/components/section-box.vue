<template>
  <div class="section-box">
    <div class="section-title">
      <slot name="title" />
    </div>
    <div class="section-body">
      <slot />
    </div>
  </div>
</template>

<script>
export default {
  name: 'SectionBox',
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .section-box {
    display: flex;
    flex-direction: column;
    min-height: 0;

    .section-title {
      display: flex;
      align-items: center;
      flex: 0 0 28px;
      padding: 0 20px 0 22px;
      color: @title-font-color;
      background-color: @darker-bg;
      font-size: 12px;
      line-height: 28px;
      user-select: none;
    }

    .section-body {
      flex: 1;
      background-color: @dark-bg;
      min-height: 0;
    }
  }
</style>
