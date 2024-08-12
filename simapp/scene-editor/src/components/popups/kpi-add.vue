<template>
  <el-dialog
    :model-value="true"
    :header="$t('indicator.addKPI')"
    append-to-body
    :close-on-click-modal="false"
    @close="$emit('close')"
  >
    <KpiList
      class="kpi-add"
      :selected="selected"
      @close="$emit('close')"
      @confirm="handleConfirm"
    />
  </el-dialog>
</template>

<script>
import KpiList from './kpi-list.vue'

export default {
  name: 'KpiAdd',
  components: {
    KpiList,
  },
  props: {
    selected: {
      type: Array,
      default () {
        return []
      },
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

  .kpi-add {
    width: 600px;
    height: auto;
    padding: 23px;
  }
</style>
