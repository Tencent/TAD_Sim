<template>
  <el-dialog
    :model-value="true"
    :header="$t('indicator.modify')"
    append-to-body
    :close-on-click-modal="false"
    top="6vh"
    width="1000px"
    @close="$emit('close')"
  >
    <KpiFormNew
      v-if="currentFormComponent"
      class="kpi-update"
      :value="value"
      :disabled="false"
      @close="$emit('close')"
      @confirm="handleConfirm"
    />
  </el-dialog>
</template>

<script>
import KpiFormNew from './kpi-form-new/index.vue'

export default {
  name: 'KpiUpdate',
  components: {
    KpiFormNew,
  },
  props: {
    value: {
      type: Object,
      required: true,
    },
  },
  computed: {
    currentFormComponent () {
      if (this.value) {
        return this.value
      }
      return null
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

  .kpi-update {
    min-height: 360px;
    padding: 23px;

    :deep(&.kpi-form-new .el-form) {
      max-height: 560px;
      overflow-x: hidden;
      overflow-y: auto;
    }
  }
</style>
