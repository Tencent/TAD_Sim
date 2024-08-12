<template>
  <div v-if="first" class="kpi-report-chart-unit-item">
    <div class="kpi-report-chart-info-wrap">
      <!-- 指标名称 -->
      <div class="kpi-report-chart-info-title">
        <slot name="title">
          <span>{{ index + 1 }}.</span>
          <span>{{ first.info.name }}</span>
        </slot>
      </div>
      <div class="kpi-report-chart-info-list">
        <!-- 指标定义 -->
        <div class="kpi-report-chart-info-item">
          <div class="label">
            {{ $t('kpi.definition') }}
          </div>
          <div class="value">
            <span>{{ first.info.desc }}</span>
          </div>
        </div>
        <!-- 评测阈值 -->
        <div v-if="getThresholdInfo(first.info)" class="kpi-report-chart-info-item">
          <div class="label">
            {{ $t('kpi.thresholds') }}
          </div>
          <div class="value">
            <span>{{ getThresholdInfo(first.info) }}</span>
          </div>
        </div>
        <!-- 评测通过条件 -->
        <div class="kpi-report-chart-info-item">
          <div class="label">
            {{ $t('kpi.passCondition') }}
          </div>
          <div class="value">
            <span>{{ getPassCondition(first.info) }}</span>
          </div>
        </div>
        <!-- 场景结束条件 -->
        <div class="kpi-report-chart-info-item">
          <div class="label">
            {{ $t('kpi.finishCondition') }}
          </div>
          <div class="value">
            <span>{{ getFinishCondition(first.info) }}</span>
          </div>
        </div>
        <!-- 实际值/阈值 -->
        <div v-if="getPassConditionValue(first.info) !== 'NaN'" class="kpi-report-chart-info-item">
          <div class="label">
            {{ $t('kpi.detectedCount') }}
          </div>
          <div class="value">
            <span>{{ first.info.detectedCount || 0 }}</span>
            <span>/</span>
            <span>{{ getPassConditionValue(first.info) }}</span>
            <span> {{ $t('kpi.totalCountUnit') }}</span>
          </div>
        </div>
        <!-- 评测结果 -->
        <div class="kpi-report-chart-info-item">
          <div class="label">
            {{ $t('indicator.evaluationResult') }}
          </div>
          <div class="value">
            <JobReportResultState type="plain" :value="first" :result-map="resultMap" />
          </div>
        </div>
        <!-- 分数结果 -->
        <div
          v-if="first.info.result.score !== undefined && first.info.result.score >= 0"
          class="kpi-report-chart-info-item"
        >
          <div class="label">
            {{ $t('indicator.scoreResult') }}
          </div>
          <div class="value">
            <JobReportResultScore type="plain" :value="first" />
          </div>
        </div>
      </div>
    </div>
    <div class="kpi-report-chart-chart-wrap">
      <template v-for="(step, i) of first.steps || []" :key="`first-pair-${i}-`">
        <template v-for="(attach, j) of step.attach || []" :key="`first-pair-${i}-${j}`">
          <table
            v-if="attach.pairData && attach.pairData.length"
            class="kpi-report-detail-pair-data"
          >
            <tbody>
              <tr v-for="pair of attach.pairData || []" :key="pair.key" class="kpi-report-detail-pair-data-item">
                <td class="kpi-report-detail-pair-data-label">
                  {{ decodeCN(pair.key) }}:
                </td>
                <td class="kpi-report-detail-pair-data-value">
                  {{ decodeCN(pair.value) }}
                </td>
              </tr>
            </tbody>
          </table>

          <template v-if="reportVersion === '2.1'">
            <KpiChartNew
              v-for="(xy, chartIndex) of attach.xyplot || []"
              :key="`first-chart-${i}-${j}-${chartIndex}`"
              :thresh="first.info.thresh"
              :data="xy"
            />
          </template>
          <template v-else>
            <KpiChart
              v-for="(xy, chartIndex) of attach.xyplot || []"
              :key="`first-chart-${i}-${j}-${chartIndex}`"
              :thresh="first.info.thresh"
              :data="xy"
            />
          </template>

          <template v-for="(sheet, sheetIndex) of attach.sheetData || []" :key="`first-sheet-${i}-${j}-${sheetIndex}`">
            <el-table
              v-show="getSheetData(sheet.sheetData).columns.length"
              :data="getSheetData(sheet.sheetData).data"
            >
              <el-table-column
                v-for="column of getSheetData(sheet.sheetData).columns"
                :key="column.prop"
                :prop="column.prop"
                :label="column.label"
              />
            </el-table>
          </template>
        </template>
      </template>
    </div>
  </div>
</template>

<script>
import KpiChart from '@/components/kpi-chart.vue'
import KpiChartNew from '@/components/kpi-chart-new.vue'
import dict from '@/common/dict'
import JobReportResultState from '@/components/popups/kpi-report/kpi-report-result-state.vue'
import JobReportResultScore from '@/components/popups/kpi-report/kpi-report-result-score.vue'
import { convertKpiSheetData } from '@/common/utils'

// \uXXXX 中文转义，莫名用 decodeURI 不行
function decodeCN (s) {
  if (typeof s !== 'string') return s
  return unescape(s.replace(/\\(u[0-9a-fA-F]{4})/g, '%$1'))
}

export default {
  name: 'KpiReportChartUnitItem',
  components: {
    KpiChart,
    KpiChartNew,
    JobReportResultState,
    JobReportResultScore,
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
    report: {
      type: Object,
      default: () => ({}),
    },
  },
  data () {
    return {
      resultMap: dict.kpiResultMap,
    }
  },
  computed: {
    reportVersion () {
      return this.report?.info?.reportVersion || ''
    },
  },
  methods: {
    decodeCN,
    getSheetData (sheet) {
      return convertKpiSheetData(sheet)
    },
    getThresholdInfo (info) {
      const additionals = info?.additionals || []
      const item = additionals.find(e => e.key === 'thresholdInfo')
      if (!item) return '-'
      return decodeCN(item.value)
    },
    getPassCondition (info) {
      const additionals = info?.additionals || []
      const item = additionals.find(e => e.key === 'passCondition')
      if (!item) return '-'
      return decodeCN(item.value)
    },
    getFinishCondition (info) {
      if (!info?.requestStop) return this.$t('indicator.unrelated') // 无关
      const isPass = info?.result?.state === 'PASS'
      if (isPass) return this.$t('indicator.becauseThisKpi', { n: this.$t('indicator.PASS') }) // 因当前指标评测通过，场景结束
      return this.$t('indicator.becauseThisKpi', { n: this.$t('indicator.FAIL') }) // 因当前指标评测未通过，场景结束
    },
    getPassConditionValue (info) {
      return info?.gradingKpi?.passCondition?.value || 'NaN'
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .kpi-report-chart-unit-item {
    display: flex;
    align-items: flex-start;
    gap: 20px;

    .kpi-report-chart-info-wrap {
      flex: 0 0 400px;

      .kpi-report-chart-info-title {
        margin-bottom: 20px;
        color: #B8B8B8;
      }

      .kpi-report-chart-info-item {
        display: flex;
        align-items: flex-start;
        margin-bottom: 10px;
        padding-left: 8px;
        line-height: 20px;

        & > .label {
          flex: 0 0 auto;
          margin-right: 10px;
          color: #878787;
          min-width: 110px;
        }
        & > .value {
          flex: 1 1 auto;
          overflow: hidden;
          color: #B8B8B8;
        }
      }
    }

    .kpi-report-chart-chart-wrap {
      flex: 1 1 auto;
      overflow: hidden;
    }

    .kpi-report-detail-pair-data {
      margin-bottom: 10px;
      border: none;

      .kpi-report-detail-pair-data-item {
        line-height: 24px;
        font-size: 12px;
        border: none;

        td {
          border: none;
        }

        .kpi-report-detail-pair-data-label {
          color: @disabled-color;
          padding-left: 20px;
          padding-right: 10px;
          text-align: right;
        }

        .kpi-report-detail-pair-data-value {
          color: @global-font-color;
        }
      }
    }
  }
</style>
