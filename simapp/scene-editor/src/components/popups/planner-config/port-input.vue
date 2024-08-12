<template>
  <div class="param-cell multi-input">
    <label class="form-label">{{ $t(desc.name) }}</label>
    <div class="form-multi-input">
      <InputNumber
        class="param-input"
        v-bind="desc"
        :model-value="value1"
        @update:model-value="valueInput('value1', $event)"
      />
      <InputNumber
        class="param-input"
        v-bind="desc"
        :model-value="value2"
        allow-empty-value
        @update:model-value="valueInput('value2', $event)"
      />
    </div>
  </div>
</template>

<script>
import InputNumber from './input-number.vue'

export default {
  name: 'NumInputItem',
  components: { InputNumber },
  props: {
    desc: {
      type: Object,
      required: true,
    },
    value: {
      type: Array,
      required: true,
    },
  },
  data () {
    return {
      value1: '',
      value2: '',
    }
  },
  watch: {
    value () {
      this.handleVal()
    },
  },
  mounted () {
    this.handleVal()
  },
  methods: {
    handleVal () {
      if (this.value?.length) {
        this.value1 = this.value[0]
        this.value2 = this.value[1]
      }
    },
    valueInput (attr, value) {
      this[attr] = value
      this.$emit('input', [this.value1, this.value2])
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
    &.multi-input {
      display: block;

      .form-label {
        display: block;
        margin-bottom: 15px;
      }
    }
    .form-multi-input {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-column-gap: 10px;
    }
  }
</style>
