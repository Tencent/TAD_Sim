<template>
  <div class="param-cell">
    <label class="form-label">{{ $t(desc.name) }}</label>
    <InputNumber
      class="param-input"
      v-bind="desc"
      :model-value="value"
      @update:model-value="$attrs.onInput"
    />
  </div>
</template>

<script>
import InputNumber from './input-number.vue'

export default {
  name: 'NumInputItem',
  components: { InputNumber },
  inheritAttrs: false,
  props: {
    desc: {
      type: Object,
      required: true,
    },
    value: {
      type: [Number, String],
      required: true,
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .param-cell {
    display: flex;
    justify-content: space-between;
    padding: 10px 0;
    align-items: center;

    .param-input {
      width: 80px;
    }
  }
</style>
