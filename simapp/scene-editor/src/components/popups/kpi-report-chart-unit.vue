<template>
  <div class="kpi-report-chart-unit">
    <template v-if="first">
      <div class="kpi-report-detail-kpi-chart-head">
        <div class="kpi-report-detail-kpi-chart-title">
          <span class="kpi-report-chart-title-index">{{ index + 1 }}.</span>
          <div>
            {{ first.info.name }}
            &nbsp;&nbsp;-&nbsp;&nbsp;
            <span class="kpi-report-detail-kpi-chart-sub-title">{{ first.info.desc }}</span>
            <template v-if="typeof first.info.thresh === 'number' && !isCompare">
              <br>{{ $t('indicator.threshold') }}: {{ first.info.thresh }}
            </template>
          </div>
        </div>
        <div v-if="!isCompare" class="kpi-report-detail-kpi-chart-result">
          {{ $t('result') }}&nbsp;&nbsp;
          <el-tag
            :class="{
              green: first.info.result.state === 'PASS',
              red: first.info.result.state === 'FAIL',
              yellow: ['SKIPPED', 'UN_DEFINED'].includes(first.info.result.state),
            }"
          >
            {{ $t(resultMap[first.info.result.state]) }}
          </el-tag>
        </div>
      </div>
      <div v-if="isCompare" class="kpi-report-detail-kpi-chart-head">
        <div class="kpi-report-detail-kpi-chart-conclusion">
          {{ $t('indicator.jobResultTitle', { i: '1' }) }}
          <span class="kpi-report-detail-kpi-chart-reason">{{ first.info.result.reason }}</span>
          <template v-if="typeof first.info.thresh === 'number'">
            <br>{{ $t('indicator.threshold') }}: {{ first.info.thresh }}
          </template>
        </div>
        <div class="kpi-report-detail-kpi-chart-result">
          {{ $t('indicator.jobResultTitle', { i: '1' }) }}&nbsp;&nbsp;
          <el-tag
            :class="{
              green: first.info.result.state === 'PASS',
              red: first.info.result.state === 'FAIL',
              yellow: ['SKIPPED', 'UN_DEFINED'].includes(first.info.result.state),
            }"
          >
            {{ $t(resultMap[first.info.result.state]) }}
          </el-tag>
        </div>
      </div>
      <template v-for="(step, i) of first.steps || []" :key="`first-pair-${i}-`">
        <template v-for="(attach, j) of step.attach || []" :key="`first-pair-${i}-${j}`">
          <table
            v-if="attach.pairData && attach.pairData.length"
            class="kpi-report-detail-pair-data"
          >
            <tbody>
              <tr v-for="pair of attach.pairData || []" :key="pair.key" class="kpi-report-detail-pair-data-item">
                <td class="kpi-report-detail-pair-data-label">
                  {{ pair.key }}:
                </td>
                <td class="kpi-report-detail-pair-data-value">
                  {{ pair.value }}
                </td>
              </tr>
            </tbody>
          </table>

          <KpiChart
            v-for="(xy, chartIndex) of attach.xyplot || []"
            :key="`first-chart-${i}-${j}-${chartIndex}`"
            :thresh="first.info.thresh"
            :data="xy"
          />

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
    </template>
    <template v-if="second">
      <br>
      <div class="kpi-report-detail-kpi-chart-head">
        <div v-if="!first" class="kpi-report-detail-kpi-chart-title">
          <span class="kpi-report-chart-title-index">{{ index + 1 }}.</span>
          <div>
            {{ second.info.name }}
            &nbsp;&nbsp;-&nbsp;&nbsp;
            <span class="kpi-report-detail-kpi-chart-sub-title">{{ second.info.desc }}</span>
            <template v-if="typeof second.info.thresh === 'number'">
              <br>{{ $t('indicator.threshold') }}: {{ second.info.thresh }}
            </template>
          </div>
        </div>
        <div v-else class="kpi-report-detail-kpi-chart-conclusion">
          {{ $t('indicator.jobResultTitle', { i: '2' }) }}
          <span class="kpi-report-detail-kpi-chart-reason">{{ second.info.result.reason }}</span>
          <template v-if="typeof second.info.thresh === 'number'">
            <br>{{ $t('indicator.threshold') }}: {{ second.info.thresh }}
          </template>
        </div>
        <div class="kpi-report-detail-kpi-chart-result">
          {{ $t('indicator.jobResultTitle', { i: '2' }) }}&nbsp;&nbsp;
          <el-tag
            :class="{
              green: second.info.result.state === 'PASS',
              red: second.info.result.state === 'FAIL',
              yellow: ['SKIPPED', 'UN_DEFINED'].includes(second.info.result.state),
            }"
          >
            {{ $t(resultMap[second.info.result.state]) }}
          </el-tag>
        </div>
      </div>

      <template v-for="(step, i) of second.steps || []" :key="`second-pair-${i}-`">
        <template v-for="(attach, j) of step.attach || []" :key="`second-pair-${i}-${j}`">
          <table
            v-if="attach.pairData && attach.pairData.length"
            class="kpi-report-detail-pair-data"
          >
            <tbody>
              <tr v-for="pair of attach.pairData || []" :key="pair.key" class="kpi-report-detail-pair-data-item">
                <td class="kpi-report-detail-pair-data-label">
                  {{ pair.key }}:
                </td>
                <td class="kpi-report-detail-pair-data-value">
                  {{ pair.value }}
                </td>
              </tr>
            </tbody>
          </table>

          <KpiChart
            v-for="(xy, chartIndex) of attach.xyplot || []"
            :key="`second-chart-${i}-${j}-${chartIndex}`"
            :thresh="second.info.thresh"
            :data="xy"
          />

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
    </template>
  </div>
</template>

<script>
import KpiChart from '@/components/kpi-chart.vue'
import dict from '@/common/dict'
import { convertKpiSheetData } from '@/common/utils'

export default {
  name: 'KpiReportChartUnit',
  components: {
    KpiChart,
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
  },
  data () {
    return {
      resultMap: dict.kpiResultMap,
    }
  },
  methods: {
    getSheetData (sheet) {
      return convertKpiSheetData(sheet)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  @green-bg: rgba(0, 250, 255, 0.2);
  @green-color: #00faff;

  @red-bg: rgba(247, 50, 34, 0.2);
  @red-color: #F73222;

  @yellow-bg: rgba(216, 150, 20, 0.2);
  @yellow-color: #D89614;

  .kpi-report-chart-unit {
    .el-tag {
      height: 24px;
      line-height: 24px;
      border: none;

      &.green {
        background-color: @green-bg;
        color: @green-color;
      }

      &.red {
        background-color: @red-bg;
        color: @red-color;
      }

      &.yellow {
        background-color: @yellow-bg;
        color: @yellow-color;
      }
    }

    .kpi-report-detail-kpi-chart-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 10px;

      .kpi-report-detail-kpi-chart-title {
        display: flex;
        align-items: flex-start;
        font-size: 12px;
        color: @title-font-color;
        font-weight: normal;
        .kpi-report-detail-kpi-chart-sub-title {
          color: @disabled-color;
        }
      }

      .kpi-report-chart-title-index {
        margin-right: 10px;
      }

      .kpi-report-detail-kpi-chart-conclusion {
        color: @disabled-color;
        margin-left: 14px;

        .kpi-report-detail-kpi-chart-reason {
          color: @global-font-color;
          margin-left: 15px;
        }
      }

      .kpi-report-detail-kpi-chart-result {
        line-height: 24px;
      }
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
