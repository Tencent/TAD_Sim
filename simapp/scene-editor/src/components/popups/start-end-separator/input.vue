<template>
  <div class="start-end-separator-input">
    <InputNumber
      v-model="value.start"
      :precision="2"
      :min="min"
      :max="max"
      :allow-empty-value="true"
    />
    -
    <InputNumber
      v-model="value.end"
      :precision="2"
      :min="min"
      :max="max"
      :allow-empty-value="true"
    />
    <span class="separator-icon">
      <el-tooltip effect="dark" :content="$t('interval')" placement="bottom">
        <separator-svg tabindex="-1" />
      </el-tooltip>
    </span>
    <InputNumber
      v-model="value.sep"
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
    value: { // v-model:value 双向绑定
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
