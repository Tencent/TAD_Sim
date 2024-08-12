<template>
  <div class="kpi-report-chart-unit">
    <template v-if="first">
      <KpiReportChartUnitItem
        :report="report"
        :first="first"
        :index="index"
      >
        <template #title>
          <span>{{ index + 1 }}.</span>
          <span>{{ first.info.name }}</span>
          <template v-if="isCompare && second">
            <span>（{{ $t('indicator.jobResultTitle', { i: '1' }) }}）</span>
          </template>
        </template>
      </KpiReportChartUnitItem>
    </template>
    <template v-if="second">
      <KpiReportChartUnitItem
        :report="report"
        :first="second"
        :index="index"
      >
        <template #title>
          <span>{{ index + 1 }}.</span>
          <span>{{ second.info.name }}</span>
          <template v-if="isCompare && second">
            <span>（{{ $t('indicator.jobResultTitle', { i: '2' }) }}）</span>
          </template>
        </template>
      </KpiReportChartUnitItem>
    </template>
  </div>
</template>

<script>
import KpiReportChartUnitItem from './kpi-report-chart-unit-item.vue'

export default {
  name: 'KpiReportChartUnitNew',
  components: {
    KpiReportChartUnitItem,
  },
  props: {
    index: {
      type: [String, Number],
      default: '',
    },
    first: {
      type: Object,
      default: null,
    },
    second: {
      type: Object,
      default: null,
    },
    isCompare: {
      type: Boolean,
      default: false,
    },
    report: {
      type: Object,
      default: () => ({}),
    },
  },
  data () {
    return {
    }
  },
  methods: {
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";
</style>
