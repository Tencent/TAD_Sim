<template>
  <div class="numeric-range-slider-input">
    <div class="top-label">
      <sim-label v-if="label">
        {{ label }}
      </sim-label>
    </div>
    <div>
      <el-slider
        v-model="values"
        range
        :min="min"
        :max="max"
        :step="step"
        :show-tooltip="false"
        :disabled="disabled"
        @change="handleChange"
      />
    </div>
    <div class="numeric-group">
      <el-input
        v-model="values[0]"
        :min="min"
        :max="max"
        class="short-input"
        :style="{ width: `${inputWidth}px` }"
        :disabled="disabled"
        @change="handleChange"
      >
        <template #suffix>
          <span class="input-unit">{{ unit }}</span>
        </template>
      </el-input>
      <el-input
        v-model="values[1]"
        :min="min"
        :max="max"
        class="short-input"
        :style="{ width: `${inputWidth}px` }"
        :disabled="disabled"
        @change="handleChange"
      >
        <template #suffix>
          <span class="input-unit">{{ unit }}</span>
        </template>
      </el-input>
    </div>
    <div v-if="subLabels.length" class="numeric-labels">
      <sim-label :style="{ width: `${inputWidth}px` }">
        {{ subLabels[0] }}
      </sim-label>
      <sim-label :style="{ width: `${inputWidth}px` }">
        {{ subLabels[1] }}
      </sim-label>
    </div>
  </div>
</template>

<script>
function compareNumbers (a, b) {
  return a - b
}

export default {
  name: 'NumericRangeSliderInput',
  props: {
    modelValue: {
      type: Array,
      default: () => [0, 50],
      validator (value) {
        if (value.length !== 2) {
          return false
        }
        return value[0] <= value[1]
      },
    },
    label: {
      type: String,
      default: '',
    },
    unit: {
      type: String,
      default: '',
    },
    min: {
      type: Number,
      default: 0,
    },
    max: {
      type: Number,
      default: 100,
    },
    step: {
      type: Number,
      default: 1,
    },
    subLabels: {
      type: Array,
      default: () => [],
    },
    inputWidth: {
      type: Number,
      default: 60,
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      values: [0, 50],
    }
  },
  computed: {
    minValue: {
      get () {
        return Math.min(this.modelValue[0], this.modelValue[1])
      },
      set (value) {
        this.$emit('update:model-value', [value, this.maxValue])
      },
    },
    maxValue: {
      get () {
        return Math.max(this.modelValue[0], this.modelValue[1])
      },
      set (value) {
        this.$emit('update:model-value', [this.minValue, value])
      },
    },
  },
  watch: {
    modelValue (newValue) {
      this.values = newValue
    },
  },
  created () {
    this.values = this.modelValue
  },
  methods: {
    handleChange () {
      const [value0, value1] = this.values
      const val0 = Math.max(Math.min(Number.parseFloat(value0) || 0, this.max), this.min)
      const val1 = Math.max(Math.min(Number.parseFloat(value1) || 0, this.max), this.min)
      this.$emit('update:model-value', [val0, val1].sort(compareNumbers))
      this.$emit('change', [val0, val1].sort(compareNumbers))
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .numeric-range-slider-input {

    .top-label {
      text-align: left;
    }

    .numeric-group {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 5px;

      .input-unit {
        color: @disabled-color;
      }

      .short-input {
        width: 60px;
      }
    }

    .numeric-labels {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 5px;

      .sim-label {
        width: 60px;
        text-align: center;
      }
    }
  }
</style>
