<template>
  <div v-if="meta.name" class="kpi-report-kpi-meta-item">
    <div class="kpi-report-meta-name">
      <span>{{ index + 1 }}. {{ meta.name }}</span>
    </div>
    <KpiChartNew :data="meta" />
  </div>
</template>

<script>
import KpiChartNew from '@/components/kpi-chart-new.vue'

export default {
  components: {
    KpiChartNew,
  },
  props: {
    meta: {
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

<style scoped lang="less">
.kpi-report-kpi-meta-item {
  .kpi-report-meta-name {
    margin-bottom: 10px;
  }
}
</style>
