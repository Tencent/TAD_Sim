<template>
  <el-dialog
    :model-value="true"
    :header="$t('module.add')"
    append-to-body
    :close-on-click-modal="false"
    @close="$emit('close')"
  >
    <ModuleList
      class="module-add"
      :selected="selectedModuleNames"
      :global="global"
      @close="$emit('close')"
      @confirm="handleConfirm"
    />
  </el-dialog>
</template>

<script>
import ModuleList from './module-list.vue'

export default {
  name: 'ModuleAdd',
  components: {
    ModuleList,
  },
  props: {
    selected: {
      type: Array,
      default () {
        return []
      },
    },
    // false主车算法，true全局模块
    global: {
      type: Boolean,
      default: false,
    },
  },
  computed: {
    selectedModuleNames () {
      return this.selected.map(s => s.name)
    },
  },
  methods: {
    handleConfirm (payload) {
      this.$emit('confirm', payload)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .module-add {
    width: 600px;
    height: auto;
    padding: 23px;
  }
</style>
