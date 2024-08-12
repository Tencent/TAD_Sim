<template>
  <label class="label-numeric-input">
    <label
      v-if="label"
      class="label"
    >{{ label }}</label>
    <SimNumericInput
      class="input"
      :value="value"
      v-bind="$attrs"
    />
  </label>
</template>

<script>
import SimNumericInput from './sim-numeric-input.vue'

export default {
  name: 'LabelNumericInput',
  components: {
    SimNumericInput,
  },
  model: {
    prop: 'value',
    event: 'change',
  },
  props: {
    value: {
      type: Number,
      default: 0,
    },
    label: {
      type: String,
      default: '',
    },
  },
}
</script>

<style scoped lang="less">
  .label-numeric-input {
    display: inline-flex;
    align-items: center;

    .label {
      flex: auto;
      text-align: left;
      color: #9d9d9d;
      font-size: 12px;
    }

    .input {
      flex: none;
    }
  }
</style>
