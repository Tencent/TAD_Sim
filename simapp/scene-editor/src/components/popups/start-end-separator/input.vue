<template>
  <div class="start-end-separator-input">
    <InputNumber
      :model-value="value.start"
      :precision="2"
      :min="min"
      :max="max"
      :allow-empty-value="true"
    />
    -
    <InputNumber
      :model-value="value.end"
      :precision="2"
      :min="min"
      :max="max"
      :allow-empty-value="true"
    />
    <span class="separator-icon">
      <el-tooltip effect="dark" :content="$t('interval')" placement="bottom">
        <separator-svg />
      </el-tooltip>
    </span>
    <InputNumber
      :model-value="value.sep"
      :precision="2"
      :min="0.01"
      :max="abs(value.end - value.start)"
      :allow-empty-value="true"
    />
  </div>
</template>

<script>
import InputNumber from '@/components/input-number.vue'

export default {
  name: 'StartEndSeparatorInput',
  components: {
    InputNumber,
  },
  props: {
    value: {
      type: Object,
      required: true,
    },
    min: {
      type: Number,
      default: Number.NEGATIVE_INFINITY,
    },
    max: {
      type: Number,
      default: Number.POSITIVE_INFINITY,
    },
  },
  methods: {
    abs (x) {
      return Math.abs(x)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .start-end-separator-input {
    display: inline-block;

    .separator-icon {
      position: relative;
      top: 2px;
      margin: 0 1px;
    }
  }
</style>
