<template>
  <div class="input-file-browser">
    <el-input
      v-model="displayValue"
      :disabled="disabled"
      :readonly="inputReadonly"
      @change="handleChange"
      @click="handleInputClick"
    >
      <template #suffix>
        <el-button
          icon="folder-opened"
          :disabled="disabled"
          @click.stop="handleBrowseFile"
        />
        <el-button
          v-if="inputReadonly && displayValue"
          icon="folder"
          style="margin-left: 0;"
          @click.stop="displayValue = '';handleChange('')"
        />
      </template>
    </el-input>
  </div>
</template>

<script>
import { errorHandler } from '@/common/errorHandler'

const { electron: { dialog } } = window

export default {
  name: 'InputFileBrowser',
  props: {
    modelValue: {
      type: String,
      default: '',
    },
    title: {
      type: String,
      default: () => this.$t('tips.selectFile'),
    },
    properties: {
      type: Array,
      default () {
        return ['openFile']
      },
    },
    disabled: {
      type: Boolean,
      default: false,
    },
    inputReadonly: {
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
        this.displayValue = value
      },
      immediate: true,
    },
  },
  methods: {
    handleChange (value) {
      this.$emit('update:model-value', value)
      this.$emit('change', value)
    },
    async handleBrowseFile () {
      try {
        const { filePaths } = await dialog.showOpenDialog({
          title: this.title,
          properties: this.properties,
        })
        const [filePath] = filePaths
        if (filePath) {
          this.$emit('update:model-value', filePath)
          this.$emit('change', filePath)
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
    handleInputClick ($event) {
      if (this.inputReadonly) {
        $event.target.blur()
        this.handleBrowseFile()
      }
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .input-file-browser {
    display: inline-block;
    width: 100%;
    vertical-align: top;

    :deep(.el-input__suffix) {
      right: 1px;
    }

    :deep(.el-input__wrapper) {
      padding-right: 0;
    }
  }
</style>
