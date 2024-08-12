<template>
  <div class="the-window">
    <div class="the-window-head">
      <div class="head-title">
        <slot name="header" />
      </div>
    </div>
    <div class="the-window-body">
      <slot name="body" />
    </div>
    <div class="the-window-footer">
      <slot name="footer" />
    </div>
  </div>
</template>

<script>
export default {
  name: 'TheWindow',
}
</script>

<style scoped lang="less">
.the-window {
  display: flex;
  flex-direction: column;
  border-radius: 10px;
  z-index: 1;
  width: 452px;
  overflow: hidden;
  box-shadow: 0 0 3px rgba(17, 21, 27, 0.75);

  .the-window-head {
    display: flex;
    justify-content: space-between;
    align-items: center;
    height: 40px;
    padding: 0 15px;
    background-color: rgba(45, 60, 81, 1);

    .head-title {
      flex: 1;
      height: 36px;
      line-height: 36px;
    }
  }

  .the-window-body {
    flex: 1;
    display: flex;
    flex-direction: column;
    min-height: 1px;
    background-color: rgba(17, 21, 27, 0.75);
  }

  .the-window-footer {
    background-color: rgba(45, 60, 81, 0.4);
    border-radius: 0 0 10px 10px;
  }
}
</style>
