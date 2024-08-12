<template>
  <label
    class="sim-numeric-input"
    :class="{ active: isFocus }"
  >
    <input
      v-bind="$attrs"
      ref="input"
      type="text"
      :value="value"
      class="inner"
      v-on="inputListeners"
    >
    <span v-if="unit" class="unit">{{ unit }}</span>
  </label>
</template>

<script>
/**
 * @module numeric-input
 * @desc 数字输入框
 * @vue-prop {number} value=0 - 输入框的数字值
 * @vue-prop {number} [min=0] - 输入框限制的最小值
 * @vue-prop {number} [max=Number.MAX_VALUE] - 输入框限制的最大值
 * @vue-prop {string} [unit] - 数字单位
 * @vue-data {number} isFocus - 获取焦点标志
 * @vue-computed {object} inputListeners - 键值对事件监听对象
 * @vue-event {number} change - 仅在输入框失去焦点或用户按下回车时触发
 * @vue-event {number} input - 在 Input 值改变时触发
 */
export default {
  name: 'SimNumericInput',
  model: {
    prop: 'value',
    event: 'change',
  },
  props: {
    unit: {
      type: String,
      default: '',
    },
    min: {
      type: Number,
      default: Number.MIN_SAFE_INTEGER,
    },
    max: {
      type: Number,
      default: Number.MAX_SAFE_INTEGER,
    },
    disabled: {
      type: Boolean,
      default: false,
    },
    readonly: {
      type: Boolean,
      default: false,
    },
    value: {
      type: Number,
      default: 0,
    },
  },
  data () {
    return {
      isFocus: false,
      val: this.value,
    }
  },
  computed: {
    inputListeners () {
      const vm = this
      return Object.assign({}, this.$attrs, {
        input (event) {
          event.stopPropagation()
          const value = Number.parseFloat(event.target.value)
          vm.$emit('input', value)
        },
        change (event) {
          event.stopPropagation()
          let value = Number.parseFloat(event.target.value)
          if (Number.isNaN(value)) {
            value = vm.val
            event.target.value = value
          }
          value = Math.min(Math.max(value, vm.min), vm.max)
          vm.$emit('change', value)
        },
        focus () {
          vm.isFocus = true
          // 缓存修改之前的数据
          vm.val = vm.value
        },
        blur () {
          vm.isFocus = false
        },
      })
    },
  },
  methods: {
    /**
     * 获取焦点
     * @returns {void}
     */
    focus () {
      this.$refs.input.focus()
    },
    /**
     * 失去焦点
     * @returns {void}
     */
    blur () {
      this.$refs.input.blur()
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.sim-numeric-input {
  box-sizing: border-box;
  border-width: 1px;
  border-style: solid;
  border-radius: 1px;
  display: inline-flex;
  padding: 2px 4px;
  line-height: 1;
  font-size: 12px;
  width: 60px;
  height: 24px;
  transition: background-color 300ms linear, border-color 300ms linear, color 300ms linear;

  border-color: @title-bd;
  background-color: @dark-bg;
  color: @disabled-color;

  &:hover,
  &.active {
    color: @active-font-color;
    border-color: @active-font-color;

    .inner {
      color: @active-font-color;
    }
  }

  .inner {
    border: none;
    outline: none;
    background: none;
    color: @global-font-color;
    flex: auto;
    overflow: hidden;
    font-size: 12px;

    &::-webkit-input-placeholder {
      color: @darker-font-color;
    }
  }

  .unit {
    color: @darker-font-color;
    user-select: none;
    margin-left: 4px;
    flex: none;
    align-self: center;
  }
}
</style>
