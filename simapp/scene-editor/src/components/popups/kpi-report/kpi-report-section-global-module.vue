<template>
  <div class="kpi-report-section-global-module" :class="{ isCompare }">
    <template v-for="(data, index) in list" :key="index">
      <div class="kpi-report-section-global-module-item">
        <div class="kpi-report-detail-main-section">
          <div class="kpi-report-detail-main-section-title">
            <span>{{ $t('module.listGlobalForReport') }}</span>
            <span v-if="isCompare">（{{ $t('job') }} {{ index + 1 }}）</span>
          </div>
          <div class="kpi-report-detail-main-section-body">
            <KpiReportModuleTable :data="data" is-global />
          </div>
        </div>
      </div>
    </template>
  </div>
</template>

<script>
import KpiReportModuleTable from './kpi-report-module-table.vue'

export default {
  components: {
    KpiReportModuleTable,
  },
  props: {
    first: {
      type: Object,
      default: () => {},
    },
    second: {
      type: Object,
      default: null,
    },
  },
  computed: {
    list () {
      return [this.first, this.second].filter(e => !!e)
    },
    isCompare () {
      return !!this.first && !!this.second
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/var";
@import "@/assets/less/mixins";

.kpi-report-section-global-module {
  .kpi-report-section-global-module-item {
    margin-bottom: 20px;
  }

  .kpi-report-detail-main-section-title {
    padding-bottom: 12px;
    margin-bottom: 12px;
    border-bottom: 1px solid @global-bd-color;
  }
}
</style>
