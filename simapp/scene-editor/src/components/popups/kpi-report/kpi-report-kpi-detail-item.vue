<template>
  <div v-if="data && data.info" class="kpi-report-kpi-detail-item">
    <KpiReportChartUnitItem
      :report="report"
      :first="data"
      :index="index"
    >
      <template #title>
        <span>{{ index + 1 }}.</span>
        <span>{{ data.info.name }}</span>
      </template>
    </KpiReportChartUnitItem>
  </div>
</template>

<script>
import KpiReportChartUnitItem from '../kpi-report-chart-unit-item.vue'

export default {
  components: {
    KpiReportChartUnitItem,
  },
  props: {
    data: {
      type: Object,
      default: () => ({}),
    },
    report: {
      type: Object,
      default: () => ({}),
    },
    index: {
      type: Number,
      default: 0,
    },
  },
}
</script>
