<template>
  <div class="kpi-report-base-info">
    <div class="kpi-report-detail-basic-info">
      <div class="kpi-report-info-item">
        <div class="kpi-report-info-label">
          {{ $t('scenario.scenarioName') }}
        </div>
        <div class="kpi-report-info-value" :title="value && value.report.info.sceneName || '--'">
          {{ value && value.report.info.sceneName || '--' }}
        </div>
      </div>
      <div class="kpi-report-info-item">
        <div class="kpi-report-info-label">
          {{ $t('scenario.scenarioDesc') }}
        </div>
        <div class="kpi-report-info-value">
          {{ value && value.report.info.sceneDesc || '--' }}
        </div>
      </div>
      <div class="kpi-report-info-item">
        <div class="kpi-report-info-label">
          {{ $t('scenario.savedVersionOfScene') }}
        </div>
        <div class="kpi-report-info-value">
          {{ data_version(value) }}
        </div>
      </div>
      <div class="kpi-report-info-item">
        <div class="kpi-report-info-label">
          {{ $t('scenario.completionTimeOfRunning') }}
        </div>
        <div class="kpi-report-info-value">
          {{ endDate(value) }}
        </div>
      </div>
      <div class="kpi-report-info-item">
        <div class="kpi-report-info-label">
          {{ $t('scenario.totalRunningTime') }}
        </div>
        <div class="kpi-report-info-value">
          {{
            value && `${value.report.info.date.durationSecond} s` || '--'
          }}
        </div>
      </div>
      <div class="kpi-report-info-item">
        <div class="kpi-report-info-label">
          {{ $t('scenario.totalDistance') }}
        </div>
        <div class="kpi-report-info-value">
          {{ mileage(value) }}
        </div>
      </div>
    </div>
    <div v-if="value" class="kpi-report-detail-basic-thumbnail">
      <KpiReportSceneImage :id="value.id" />
    </div>
  </div>
</template>

<script>
import { get } from 'lodash-es'
import dayjs from 'dayjs'
import KpiReportSceneImage from '@/components/popups/kpi-report/kpi-report-scene-image.vue'

const timeFilter = second => dayjs(second).format('YYYY-MM-DD HH:mm:ss')

export default {
  components: {
    KpiReportSceneImage,
  },
  props: {
    value: {
      type: Object,
      default: () => {
      },
    },
  },
  data () {
    return {}
  },
  methods: {
    endDate (item) {
      const endSecond = get(item, 'report.info.date.endSecond')
      if (endSecond) return timeFilter(endSecond * 1000)
      return '--'
    },
    mileage (item) {
      const mileage = get(item, 'report.info.mileage')
      if (typeof mileage === 'number') return `${mileage.toFixed(3)}km`
      return '--'
    },
    data_version (item) {
      const additional = get(item, 'report.info.additionals', [])
      const tmp = additional.find(({ key }) => key === 'date_version')
      if (tmp && tmp.value) return timeFilter(+tmp.value)
      return '--'
    },
  },
}
</script>

<style lang="less" scoped>
@import "@/assets/less/mixins";

.kpi-report-base-info {
  display: flex;

  .kpi-report-detail-basic-info {
    flex-grow: 1;
    overflow: hidden;
  }

  .kpi-report-info-item {
    display: flex;
    margin-bottom: 10px;

    .kpi-report-info-label {
      color: @global-info-color;
      width: 100px;
      padding-right: 15px;
      text-align: right;
    }

    .kpi-report-info-value {
      .text-overflow();
      flex: 1;
      color: @global-font-color;
    }
  }

  .kpi-report-detail-basic-thumbnail {
    flex: 0 0 200px;
    margin-left: 10px;

    .header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      color: @global-info-color;

      .label {
        padding: 12px 0;
      }
    }

    .preview-wrap {
      position: relative;
      width: 200px;
      height: 112px;
      overflow: hidden;
      // cursor: pointer;

      &.placeholder {
        & > img {
          margin-left: -1px;
        }
      }

      & > img {
        width: 100%;
        height: 100%;
        display: block;
        // object-fit: cover;
        border-radius: 8px;
      }

      &.placeholder > img {
        opacity: 0.5;
      }

      .tips-wrap {
        position: absolute;
        top: 0;
        left: 0;
        width: 100%;
        height: 100%;
        background: rgba(0, 0, 0, 0.2);
        color: #888;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
      }
    }
  }
}
</style>
