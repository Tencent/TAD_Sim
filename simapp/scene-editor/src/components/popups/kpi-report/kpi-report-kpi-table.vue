<template>
  <div v-if="list && list.length" class="job-report-kpi-table">
    <el-table ref="table" :data="shownList">
      <el-table-column
        prop="指标名称"
        :label="$t('indicator.name')"
        :show-overflow-tooltip="!printing"
      />
      <el-table-column
        prop="评测结果"
      >
        <template #header>
          <TreeFilters
            :label="$t('indicator.evaluationResult')"
            :data="resultDict"
            :with-root-node="false"
            @filter-change="handleFilterChange('评测结果', $event)"
          />
        </template>
        <template #default="{ row }">
          <KpiReportResultState
            type="plain"
            :value="{ info: { result: { state: row['评测结果'] } } }"
            :result-map="resultMap"
          />
        </template>
      </el-table-column>
      <el-table-column
        prop="分数"
        label="分数结果"
      >
        <template #default="{ row }">
          <span class="score">
            <KpiReportResultScore type="plain" :value="{ info: { result: { score: row['分数'] } } }" />
          </span>
        </template>
      </el-table-column>
      <el-table-column
        prop="实际值_通过条件阈值"
        :label="$t('kpi.detectedCount')"
      />
      <el-table-column
        prop="评测阈值"
        :label="$t('kpi.thresholds')"
        :show-overflow-tooltip="!printing"
      />
      <el-table-column
        prop="分类标签"
        :label="$t('kpi.labels')"
        :show-overflow-tooltip="!printing"
      >
        <template #header>
          <TreeFilters
            :label="$t('kpi.labels')"
            :data="categoryDict"
            :with-root-node="false"
            @filter-change="handleFilterChange('分类标签', $event)"
          />
        </template>
        <template #default="{ row }">
          <div class="kpi-labels">
            <el-tag
              v-for="tag in row['分类标签']"
              :key="tag"
              disable-transitions
            >
              {{ getLabelsCN(tag) }}
            </el-tag>
          </div>
        </template>
      </el-table-column>
    </el-table>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapState } from 'vuex'
import KpiReportResultState from './kpi-report-result-state.vue'
import KpiReportResultScore from './kpi-report-result-score.vue'
import TreeFilters from '@/components/tree-filters.vue'
import KpiMixin from '@/components/popups/kpi-form-new/mixin'
import dict from '@/common/dict'
import { calcStrLength, defaultEgoId } from '@/common/utils'
import eventBus from '@/event-bus'

const { kpiResultMap } = dict

const defaultLabelsWidth = 100

export default {
  components: {
    TreeFilters,
    KpiReportResultState,
    KpiReportResultScore,
  },
  mixins: [
    KpiMixin,
  ],
  props: {
    data: {
      type: Object,
      default: () => ({}),
    },
    egoId: {
      type: String,
      default: defaultEgoId,
    },
  },
  emits: ['filter-change'],
  data () {
    return {
      filters: {},
      resultMap: kpiResultMap,
      labelsWidth: defaultLabelsWidth,
      printing: false,
    }
  },
  computed: {
    ...mapState([
      'kpiCategoryOptions',
    ]),
    resultDict () {
      return [
        {
          label: '通过',
          id: 'PASS',
        },
        {
          label: '不通过',
          id: 'FAIL',
        },
      ]
    },
    list () {
      const version = _.has(this.data, 'meta') ? '1.0' : '2.0'
      let data = []
      if (version === '1.0') {
        const cases = _.get(this.data, 'report.cases', [])
        const res = cases.map(this.getCaseInfo)
        data = res
      } else {
        const egoCases = _.get(this.data, 'report.egoCases', [])
        const tmp = egoCases.find(e => e.egoId === this.egoId)
        const cases = tmp?.cases || []
        const res = cases.map(this.getCaseInfo)
        data = res
      }
      return data
    },
    shownList () {
      let data = this.list || []
      Object.keys(this.filters).forEach((key) => {
        const filterValue = this.filters[key] || []
        if (filterValue.length) {
          if (key === '分类标签') {
            data = data.filter((item) => {
              const value = item[key].join(';')
              const match = filterValue.some(label => value.includes(label))
              return match
            })
          } else {
            data = data.filter(item => filterValue.includes(item[key]))
          }
        }
      })
      return data
    },
    categoryDict () {
      return this.kpiLabelsList.map(({ labelKey, labelDescription }) => ({ id: labelKey, label: labelDescription }))
    },
  },
  watch: {
    shownList (list) {
      this.refreshTableWidth(list)
    },
  },
  mounted () {
    this.refreshTableWidth(this.shownList)
    eventBus.$on('kpi-report-export-click', this.handleMainExport)
    eventBus.$on('kpi-report-export-finish', this.handleMainExportFinish)
  },
  unmounted () {
    eventBus.$off('kpi-report-export-click', this.handleMainExport)
    eventBus.$off('kpi-report-export-finish', this.handleMainExportFinish)
  },
  methods: {
    handleMainExport () {
      this.printing = true
    },
    handleMainExportFinish () {
      this.printing = false
    },
    handleFilterChange (key, value) {
      const ids = value.map(e => e.id)
      this.filters[key] = ids
      this.$emit('filter-change', key, ids)
    },
    refreshTableWidth (list) {
      const allLabels = list.map(e => e['分类标签'])
      const getLabelWidth = label => calcStrLength(this.getLabelsCN(label)) / 2 * 12 + 28
      const getWidth = labels => labels.reduce((re, e) => (re + getLabelWidth(e)), 0)
      const allLabelsWidth = allLabels.map(e => getWidth(e))
      this.labelsWidth = _.max([...allLabelsWidth, defaultLabelsWidth])
    },
    getLabelsCN (label) {
      const tmp = this.categoryDict.find(item => item.id === label)
      return tmp ? tmp.label : label
    },
    getCaseInfo (item) {
      if (!item?.info) return undefined
      const name = item.info.name || ''
      const state = item.info.result.state || 'UN_DEFINED'
      const score = item.info.result.score || 0
      const detectedCount = item.info.detectedCount || 0
      const passCount = item.info.gradingKpi.passCondition.value || 0
      const category = item.info.gradingKpi.category.labels || []
      const thresholds = this.getKpiThresholdTxt({ config: item.info.gradingKpi })
      return {
        指标名称: name,
        评测结果: state,
        分数: score,
        实际值_通过条件阈值: `${detectedCount}/${passCount} 次`,
        评测阈值: thresholds,
        分类标签: category,
      }
    },
    getKpiThresholdTxt (info) {
      const thresholdOptions = info.config?.parameters?.thresholds || []
      const thresholds = thresholdOptions.map(item => `${item.thresholdName}:${item.threshold}${item.thresholdUnit && item.thresholdUnit !== 'N/A' ? item.thresholdUnit : ''}`)
      return thresholds.join('; ') || '-'
    },
  },
}
</script>

<style lang="less" scoped>
@import "@/assets/less/mixins";

@green-bg: rgba(0, 250, 255, 0.2);
@green-color: #00faff;

.job-report-kpi-table {
  /deep/ .tree-filters-label {
    font-size: inherit;
    color: inherit;
  }

  .kpi-labels {
    display: flex;

    .el-tag {
      flex-shrink: 0;
      height: 24px;
      line-height: 24px;
      border: none;
      background-color: @green-bg;
      color: @green-color;
      margin-right: 4px;
    }
  }
}
</style>
