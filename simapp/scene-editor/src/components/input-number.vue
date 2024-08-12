<template>
  <div
    class="input-number"
    :class="{ 'input-number-disabled': disabled, 'input-number-readonly': readonly }"
  >
    <el-input
      ref="input"
      :model-value="displayValue"
      :disabled="disabled"
      :readonly="readonly"
      @input="handleInput"
      @change="handleChange"
      @blur="$emit('blur', $event)"
    >
      <template v-if="unit" #suffix>
        {{ unit }}
      </template>
    </el-input>
  </div>
</template>

<script>
export default {
  name: 'InputNumber',
  props: {
    modelValue: {
      type: [
        Number,
        String,
      ],
      default: 0,
    },
    precision: {
      type: Number,
      validator (val) {
        return val >= 0 && val === Number.parseInt(val, 10)
      },
      default: undefined,
    },
    unit: {
      type: String,
      default: '',
    },
    min: {
      type: Number,
      default: Number.NEGATIVE_INFINITY,
    },
    max: {
      type: Number,
      default: Number.POSITIVE_INFINITY,
    },
    disabled: {
      type: Boolean,
      default: false,
    },
    allowEmptyValue: {
      type: Boolean,
      default: false,
    },
    readonly: {
      type: Boolean,
      default: false,
    },
    externalRule: {
      type: Function,
      default: null,
    },
  },
  data () {
    return {
      currentValue: 0,
      userInput: undefined,
    }
  },
  computed: {
    displayValue () {
      if (this.userInput !== undefined) {
        return this.userInput
      }

      let currentValue = this.currentValue
      const isAllowAndEmptyValue = this.allowEmptyValue && currentValue === ''

      if (!isAllowAndEmptyValue) {
        if (this.precision !== undefined) {
          currentValue = (+currentValue).toFixed(this.precision)
        }
      }

      return currentValue
    },
  },
  watch: {
    modelValue: {
      handler (value) {
        if (value !== this.currentValue) {
          this.currentValue = value
        }
      },
      immediate: true,
    },
    min (val) {
      if (this.currentValue < val) this.setCurrentValue(this.currentValue)
    },
    max (val) {
      if (this.currentValue > val) this.setCurrentValue(this.currentValue)
    },
  },
  methods: {
    toPrecision (num, precision) {
      const factor = 10 ** precision

      return Number.parseFloat(Math.round(num * factor) / factor)
    },
    handleInput (value) {
      this.userInput = value.trim()
    },
    handleChange (val) {
      const value = val.trim()
      this.setCurrentValue(value)
      // this.$nextTick(() => {
      //   if (this.value !== this.currentValue) {
      //     this.currentValue = this.modelValue
      //   }
      // })
    },
    setCurrentValue (value) {
      const oldVal = this.currentValue
      let newVal = value
      const isAllowAndEmptyValue = this.allowEmptyValue && newVal === ''

      if (!isAllowAndEmptyValue) {
        if (typeof newVal === 'string') {
          if (/(^[+-]?[1-9]\d*$)|(^[+-]?[1-9]\d*\.\d+$)|(^[+-]?0$)|(^[+-]?0\.\d+$)/.test(newVal)) {
            newVal = +newVal
          } else {
            newVal = oldVal
          }
        }

        if (newVal >= this.max) {
          newVal = this.max
        }
        if (newVal <= this.min) {
          newVal = this.min
        }

        if (this.externalRule) {
          newVal = this.externalRule(newVal)
        }

        if (this.precision !== undefined) {
          newVal = this.toPrecision(newVal, this.precision)
        }
      }

      this.userInput = undefined
      this.currentValue = newVal
      this.$emit('update:model-value', newVal)
    },
    focus () {
      this.$refs.input.focus()
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .input-number {
    box-sizing: content-box;
    display: inline-block;
    vertical-align: top;
    width: 80px;

    .el-input {
      display: inline-flex;
      justify-content: space-between;
      align-items: center;
      overflow: hidden;
    }

    :deep(.el-input__prefix),
    :deep(.el-input__suffix) {
      color: @disabled-color;
      position: static;
      user-select: none;
    }

    :deep(.el-input__prefix) {
      margin-right: 4px;
    }

    :deep(.el-input__suffix) {
      margin-left: 4px;
    }
  }

  .input-number-disabled,
  .input-number-readonly {
    cursor: not-allowed;
  }
</style>
