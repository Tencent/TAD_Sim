<template>
  <div class="numeric-slider-input">
    <div class="input-row">
      <sim-label v-if="label" class="input-title">
        {{ label }}
      </sim-label>
      <!--    <sim-numeric-input
            class="input-inner"
            :value="modelValue"
            :min="min"
            :max="max"
            :unit="unit"
            @change="handleChange"
          /> -->
      <InputNumber
        class="input-number"
        :model-value="modelValue"
        :min="min"
        :max="max"
        :unit="unit"
        @update:model-value="handleChange"
      />
    </div>
    <el-slider
      class="slider-row"
      :model-value="modelValue"
      :min="min"
      :max="max"
      :step="step"
      :show-tooltip="false"
      @input="handleInput"
    />
    <div>
      <span class="below-title" style="float: left">{{ $t('Conservative') }}</span>
      <span class="below-title" style="float: right">{{ $t('Radical') }}</span>
    </div>
  </div>
</template>

<script>
import InputNumber from '@/components/input-number.vue'

export default {
  name: 'NumericSliderInput',
  components: {
    InputNumber,
  },
  props: {
    modelValue: {
      type: Number,
      default: 0,
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
  },
  methods: {
    handleChange (val) {
      if (val !== this.modelValue) {
        this.$emit('update:model-value', val)
      }
    },
    handleInput (val) {
      if (val !== this.modelValue) {
        this.$emit('update:model-value', val)
      }
    },
  },
}
</script>

<style scoped lang="less">
.numeric-slider-input {
  display: inline-block;
  width: auto;

  .input-row {
    display: flex;
    align-items: center;

    .input-title {
      flex: auto;
      text-align: left;
    }

    .input-inner {
      flex: none;
    }
  }

  .slider-row {
    width: auto;
  }

  .below-title {
    font-size: 0.8em;
  }
}
</style>
