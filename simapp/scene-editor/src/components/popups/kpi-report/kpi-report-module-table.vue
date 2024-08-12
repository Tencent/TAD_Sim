<template>
  <el-table :data="list">
    <el-table-column
      prop="算法名称"
      :label="$t('indicator.algorithmName')"
      :show-overflow-tooltip="!printing"
    />
    <el-table-column
      prop="算法分类"
      :label="$t('indicator.algorithmType')"
      :formatter="getModuleCategory"
    />
    <el-table-column
      prop="平均单步耗时_s_"
      :label="$t('indicator.averageStepTime')"
    />
    <el-table-column
      prop="算法步长_s_"
      :label="$t('indicator.cycleTime')"
    />
    <el-table-column
      prop="平均单步耗时占比___"
      :label="$t('indicator.averageStepTimeRatio')"
    />
    <el-table-column
      prop="期望平均单步耗时_s_"
      :label="$t('indicator.averageStepTimeReference')"
    />
    <el-table-column
      prop="算法文件"
      :label="$t('indicator.algorithmFile')"
      :formatter="getModuleFileName"
      :show-overflow-tooltip="!printing"
    />
  </el-table>
</template>

<script>
import _ from 'lodash'
import { convertKpiSheetData, defaultEgoId } from '@/common/utils'
import dict from '@/common/dict'
import eventBus from '@/event-bus'

export default {
  props: {
    data: {
      type: Object,
      default: () => ({}),
    },
    egoId: {
      type: String,
      default: defaultEgoId,
    },
    isGlobal: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      printing: false,
    }
  },
  computed: {
    list () {
      const version = _.has(this.data, 'meta') ? '1.0' : '2.0'
      let data = []
      if (version === '1.0') {
        const sheetData = _.get(this.data, 'report.info.moduleCfg.sheetData', [])
        const res = convertKpiSheetData(sheetData)
        data = res.data
      } else {
        const moduleCfg = _.get(this.data, 'report.info.moduleCfg', [])
        const module = moduleCfg.find(e => (this.isGlobal ? e.name === 'global' : e.name === this.egoId)) || {}
        const sheetData = module.sheetData || []
        const res = convertKpiSheetData(sheetData)
        data = res.data
      }
      const shownList = data.filter((e) => {
        const moduleName = e['算法名称'] || ''
        const isEgo = moduleName.slice(0, 3) === 'Ego'
        return this.isGlobal ? !isEgo : isEgo
      })
      return shownList
    },
  },
  mounted () {
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
    getModuleCategory (row, column, cellValue) {
      const moduleCategoryList = [...dict.moduleCategoryList, ...dict.moduleGlobalCategoryList]
      const cate = moduleCategoryList.find(c => c.id === +cellValue)
      if (cate) return this.$t(cate.name)
      return '--'
    },
    getModuleFileName (row, column, cellValue) {
      const arr = cellValue.split(/\\|\//)
      if (arr && arr.length) return arr.pop()
      return '--'
    },
  },
}
</script>
