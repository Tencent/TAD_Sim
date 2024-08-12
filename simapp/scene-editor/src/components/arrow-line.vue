<template>
  <i
    class="arrow-line"
    :class="{
      disabled,
      'arrow-down': direction === 'down',
      'arrow-up': direction === 'up',
      'arrow-left': direction === 'left',
      'arrow-right': direction === 'right',
    }"
  >
    <arrow-svg />
  </i>
</template>

<script>
export default {
  name: 'ArrowLine',
  props: {
    direction: {
      type: String,
      default: 'down',
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .arrow-line {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    transition: all .16s linear;
    color: @title-font-color;
    cursor: pointer;
    transform-origin: 50% 50%;

    &.arrow-up {
      transform: rotate(180deg);
    }

    &.arrow-left {
      transform: rotate(-90deg);
    }

    &.arrow-right {
      transform: rotate(90deg);
    }
  }
</style>
