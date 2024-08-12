<template>
  <span
    class="sim-label"
    :class="`sim-label--${type}`"
    v-bind="$attrs"
  >
    <span class="label">
      <slot />
    </span>
  </span>
</template>

<script>
/**
 * @module label
 * @desc 标签
 */
export default {
  name: 'SimLabel',
  props: {
    type: {
      type: String,
      default: 'default',
      validator (val) {
        return [
          'default',
          'primary',
          'success',
          'warning',
          'danger',
          'info',
          'text',
        ].includes(val)
      },
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .sim-label {
    font-size: 12px;
    display: inline-flex;
    align-items: center;

    .label {
      flex: auto;
      overflow: hidden;
      text-overflow: ellipsis;
      white-space: nowrap;
    }

    &--default {
      color: @global-font-color;
    }

    &--primary {
      color: @active-font-color;
    }

    &--success {
      color: @global-success-color;
    }

    &--warning {
      color: @global-warning-color;
    }

    &--danger {
      color: @global-danger-color;
    }

    &--info {
      color: @global-info-color;
    }
  }
</style>
