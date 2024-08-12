<template>
  <div class="input-key-value">
    <el-input
      v-model="displayValue"
      :disabled="disabled"
      class="input-key-value-textarea"
      type="textarea"
      autosize
      @change="handleChange"
    />
  </div>
</template>

<script>
import _ from 'lodash'

export default {
  name: 'InputKeyValue',
  props: {
    modelValue: {
      validator (value) {
        return _.isPlainObject(value) || value === null
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
        const val = this.stringify(value)
        if (val !== this.displayValue) {
          this.displayValue = val
        }
      },
      immediate: true,
    },
  },
  methods: {
    stringify (object) {
      const arr = []
      _.forOwn(object, (val, key) => {
        arr.push(`${key}=${val}`)
      })
      return arr.join('\n')
    },
    parse (str) {
      const object = {}
      if (str) {
        const arr = str.split('\n')
        _.forEach(arr, (value) => {
          if (value) {
            const kv = value.split('=')
            const [key, ...val] = kv
            object[key] = val.join('=')
          }
        })
      }
      return object
    },
    handleChange (value) {
      const val = this.parse(value)
      this.$emit('update:model-value', val)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .input-key-value {
    display: inline-block;
    width: 100%;
    vertical-align: top;

    .input-key-value-textarea {
      width: 100%;

      :deep(.el-textarea__inner) {
        word-break: break-all;
      }
    }
  }
</style>
