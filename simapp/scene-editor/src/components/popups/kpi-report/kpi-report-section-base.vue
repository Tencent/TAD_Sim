<template>
  <div class="kpi-report-section-base" :class="{ isCompare }">
    <template v-for="(data, index) in list" :key="index">
      <div class="kpi-report-section-base-item">
        <template v-if="isCompare">
          <div class="kpi-report-job-index-title">
            <span>{{ $t('job') }} {{ index + 1 }} </span>
          </div>
        </template>
        <div class="base-info">
          <div class="kpi-report-detail-main-section">
            <div class="kpi-report-detail-main-section-title">
              <span>{{ $t('scenario.basicInformation') }}</span>
            </div>
            <div class="kpi-report-detail-main-section-body">
              <KpiReportBaseInfo :value="data" />
            </div>
          </div>
        </div>
        <div class="extra-info">
          <div class="kpi-report-detail-main-section">
            <div class="kpi-report-detail-main-section-title">
              <span>{{ $t('scenario.sceneRunningResults') }}</span>
            </div>
            <div class="kpi-report-detail-main-section-body kpi-result-wrap">
              <div class="kpi-result-item">
                <div class="kpi-result-item-label">
                  {{ $t('scenario.overallResults') }}
                </div>
                <div v-if="first" class="kpi-result-item-value">
                  <KpiReportResultState type="plain" :value="data.report" :result-map="resultMap" />
                </div>
              </div>
            </div>
          </div>
          <div class="kpi-report-detail-main-section">
            <div class="kpi-report-detail-main-section-title">
              <span>{{ $t('scenario.sceneStatistics') }}</span>
            </div>
            <div class="kpi-report-detail-main-section-body kpi-result-chart-wrap">
              <KpiReportStatisticsChart :value="data" />
            </div>
          </div>
        </div>
      </div>
    </template>
  </div>
</template>

<script>
import KpiReportBaseInfo from '@/components/popups/kpi-report/kpi-report-base-info.vue'
import KpiReportResultState from '@/components/popups/kpi-report/kpi-report-result-state.vue'
import KpiReportStatisticsChart from '@/components/popups/kpi-report/kpi-report-statistics-chart.vue'
import dict from '@/common/dict'

export default {
  components: {
    KpiReportBaseInfo,
    KpiReportResultState,
    KpiReportStatisticsChart,
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
  data () {
    return {
      resultMap: dict.kpiResultMap,
    }
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

.kpi-report-section-base {
  &:not(.isCompare) {
    .kpi-report-section-base-item {
      display: flex;
      gap: 20px;

      & > .base-info,
      & > .extra-info {
        flex-grow: 1;
        overflow: hidden;
      }

      & > .base-info {
        flex-basis: 55%;
      }

      & > .extra-info {
        flex-basis: 45%;
      }
    }
  }

  &.isCompare {
    display: flex;
    gap: 12px;

    .kpi-report-section-base-item {
      flex-basis: 50%;
      overflow: hidden;
    }
  }

  .kpi-report-job-index-title {
    font-size: 16px;
    margin: -.2em 0 12px;
  }

  .kpi-report-detail-main-section {
    margin-bottom: 20px;
  }

  .kpi-report-detail-main-section-title {
    padding-bottom: 12px;
    margin-bottom: 12px;
    border-bottom: 1px solid @global-bd-color;
  }

  .kpi-result-item {
    display: flex;
    align-items: center;
    gap: 12px;

    .kpi-result-item-label {
      color: @global-info-color;
    }
  }
}
</style>
