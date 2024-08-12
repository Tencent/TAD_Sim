<template>
  <div class="input-object">
    <el-input
      v-model="displayValue"
      :disabled="disabled"
      class="input-object-textarea"
      type="textarea"
      autosize
      @change="handleChange"
    />
  </div>
</template>

<script>
import _ from 'lodash'

export default {
  name: 'InputObject',
  props: {
    modelValue: {
      validator (value) {
        return _.isPlainObject(value) || _.isArray(value) || value === null
      },
      required: true,
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      displayValue: '',
    }
  },
  watch: {
    modelValue: {
      handler (value) {
        if (value) {
          const val = this.stringify(value)
          if (val !== this.displayValue) {
            this.displayValue = val
          }
        }
      },
      immediate: true,
    },
  },
  methods: {
    stringify (value) {
      let val = value
      try {
        val = JSON.stringify(val)
      } catch (e) {
        console.error(e)
      }
      return val
    },
    handleChange (value) {
      let val = value.trim()
      try {
        val = JSON.parse(val)
        this.$emit('update:model-value', val)
      } catch (e) {
        this.$emit('update:model-value', null)
      }
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .input-object {
    display: inline-block;
    width: 100%;
    vertical-align: top;

    .input-object-textarea {
      width: 100%;

      :deep(.el-textarea__inner) {
        word-break: break-all;
      }
    }
  }
</style>
