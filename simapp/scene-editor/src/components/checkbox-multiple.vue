<template>
  <div class="checkbox-multiple">
    <span class="checkbox-multiple-toggle-group" @click="showGroup = !showGroup">
      <el-icon v-if="showGroup" class="el-icon-arrow-down"><arrow-down /></el-icon>
      <el-icon v-else class="el-icon-arrow-up"><arrow-up /></el-icon>
    </span>
    <el-checkbox
      v-model="checkAll"
      :indeterminate="isIndeterminate"
      :disabled="disabled"
      @change="handleCheckAllChange"
    >
      {{ $t('operation.selectAll') }}
    </el-checkbox>
    <el-checkbox-group v-if="showGroup" v-model="checked" :disabled="disabled" @change="handleCheckedChange">
      <el-checkbox v-for="option in options" :key="option" :label="option" /><br>
    </el-checkbox-group>
  </div>
</template>

<script>
export default {
  name: 'CheckboxMultiple',
  props: {
    modelValue: {
      type: Array,
      default () {
        return []
      },
    },
    options: {
      type: Array,
      default () {
        return []
      },
    },
    disabled: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      checkAll: false,
      checked: [],
      isIndeterminate: false,
      showGroup: false,
    }
  },
  watch: {
    modelValue: {
      handler (value) {
        this.checked = value
        this.setCheckAllAndIsIndeterminate(value)
      },
      immediate: true,
    },
  },
  methods: {
    setCheckAllAndIsIndeterminate (checked) {
      const checkedCount = checked.length
      this.checkAll = checkedCount === this.options.length
      this.isIndeterminate = checkedCount > 0 && checkedCount < this.options.length
    },
    handleCheckAllChange (value) {
      const checked = value ? this.options : []
      this.isIndeterminate = false
      this.$emit('update:model-value', checked)
      this.$emit('change', checked)
    },
    handleCheckedChange (value) {
      this.setCheckAllAndIsIndeterminate(value)
      this.$emit('update:model-value', value)
      this.$emit('change', value)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .checkbox-multiple {
    display: inline-block;
    width: 100%;
    vertical-align: top;

    .checkbox-multiple-toggle-group {
      margin-right: 10px;
      font-size: 12px;
      vertical-align: middle;
      cursor: pointer;
    }
  }
</style>
