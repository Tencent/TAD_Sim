<template>
  <div class="kpi-report-section-ego-detail-list" :class="{ isCompare, useAnimation }">
    <el-collapse v-model="collapseActive">
      <template v-for="collapse in collapseList" :key="collapse.id">
        <el-collapse-item :name="collapse.id">
          <template #title>
            <div class="compare-header">
              <div :id="`${collapse.id}`" class="hash-target" />
              <span class="title">{{ collapse.title }}</span>
              <div v-if="isCompare" class="tools">
                <el-button
                  v-if="collapse.id !== defaultCollapseId"
                  link
                  @click="handleDeleteCustom($event, collapse.id)"
                >
                  <el-icon><delete /></el-icon>
                </el-button>
                <el-button link @click="handleAddCustom">
                  <el-icon><plus /></el-icon>
                </el-button>
              </div>
            </div>
          </template>

          <div class="kpi-report-section-ego-detail-item">
            <!-- 主车列表 -->
            <section
              v-if="isCompare"
              class="kpi-report-detail-main-section ego-select-list"
            >
              <div class="kpi-report-compare-wrap">
                <template v-for="(data, col) in list" :key="col">
                  <div class="kpi-report-compare-item">
                    <div class="ego-list-wrap">
                      <div class="ego-list-title">
                        <span>{{ $t('job') }} {{ col + 1 }} {{ $t('scenario.ego') }}</span>
                      </div>
                      <div class="ego-list">
                        <template v-for="ego in getEgoList(data)" :key="ego.egoId">
                          <div
                            v-if="chosenEgos[collapse.id]"
                            class="ego-list-item"
                            :class="{ active: chosenEgos[collapse.id][col] === ego.egoId }"
                            @click="chooseEgo(collapse.id, col, ego)"
                          >
                            {{ ego.egoId }}
                          </div>
                        </template>
                      </div>
                    </div>
                  </div>
                </template>
              </div>
            </section>
            <!-- 主车评测信息 -->
            <section class="kpi-report-detail-main-section">
              <div :id="`${collapse.id}_ego_info`" class="hash-target" />
              <div class="kpi-report-detail-main-section-title">
                <span>{{ $t('indicator.kpiReportEgoDetailList') }}</span>
              </div>
              <div class="kpi-report-compare-wrap">
                <template v-for="(data, col) in list" :key="col">
                  <div
                    v-if="chosenEgos[collapse.id]"
                    class="kpi-report-compare-item"
                  >
                    <div class="kpi-report-detail-main-section-body">
                      <KpiReportEgoInfo :data="data" :ego-id="chosenEgos[collapse.id][col]" />
                    </div>
                  </div>
                </template>
              </div>
            </section>
            <!-- 配置列表 -->
            <section class="kpi-report-detail-main-section no-page-break">
              <div :id="`${collapse.id}_module`" class="hash-target" />
              <div class="kpi-report-detail-main-section-title">
                <span>{{ $t('module.listForReport') }}</span>
              </div>
              <div class="kpi-report-compare-wrap">
                <template v-for="(data, col) in list" :key="col">
                  <div
                    v-if="chosenEgos[collapse.id]"
                    class="kpi-report-compare-item"
                  >
                    <div class="kpi-report-detail-main-section-body">
                      <KpiReportModuleTable
                        :data="data"
                        :ego-id="chosenEgos[collapse.id][col]"
                      />
                    </div>
                  </div>
                </template>
              </div>
            </section>
            <!-- 指标列表 -->
            <section class="kpi-report-detail-main-section no-page-break">
              <div :id="`${collapse.id}_kpi_table`" class="hash-target" />
              <div class="kpi-report-detail-main-section-title">
                <span>{{ $t('indicator.list') }}</span>
              </div>
              <div class="kpi-report-compare-wrap">
                <template v-for="(data, col) in list" :key="col">
                  <div
                    v-if="chosenEgos[collapse.id]"
                    class="kpi-report-compare-item"
                  >
                    <div class="kpi-report-detail-main-section-body">
                      <KpiReportKpiTable
                        :data="data"
                        :ego-id="chosenEgos[collapse.id][col]"
                        @filter-change="(...args) => handleKpiTableFilter(collapse.id, ...args)"
                      />
                    </div>
                  </div>
                </template>
              </div>
            </section>
            <!-- 指标详情 -->
            <section class="kpi-report-detail-main-section">
              <div :id="`${collapse.id}_kpi_detail`" class="hash-target" />
              <div class="kpi-report-detail-main-section-title">
                <span>{{ $t('indicator.detail') }}</span>
              </div>
              <template v-for="(_, count) in getMaxCasesCount(collapse.id)" :key="count">
                <div class="kpi-report-compare-wrap no-page-break">
                  <template v-for="(data, col) in list" :key="col">
                    <div v-if="chosenEgos[collapse.id]" class="kpi-report-compare-item">
                      <div class="kpi-report-detail-main-section-body">
                        <KpiReportKpiDetailItem
                          :data="getCasesList(data, collapse.id, col)[count]"
                          :report="data.report"
                          :index="count"
                        />
                      </div>
                    </div>
                  </template>
                </div>
              </template>
            </section>
            <!-- 元数据 -->
            <section class="kpi-report-detail-main-section">
              <div :id="`${collapse.id}_meta_data`" class="hash-target" />
              <div class="kpi-report-detail-main-section-title">
                <span>{{ $t('indicator.metadata') }}</span>
              </div>
              <template v-for="(_, count) in getMaxMetaCount(collapse.id)" :key="count">
                <div class="kpi-report-compare-wrap no-page-break">
                  <template v-for="(data, col) in list" :key="col">
                    <div v-if="chosenEgos[collapse.id]" class="kpi-report-compare-item">
                      <div class="kpi-report-detail-main-section-body">
                        <KpiReportKpiMetaItem
                          :meta="getMetaList(data, collapse.id, col)[count]"
                          :index="count"
                        />
                      </div>
                    </div>
                  </template>
                </div>
              </template>
            </section>
          </div>
        </el-collapse-item>
      </template>
    </el-collapse>
  </div>
</template>

<script>
import _ from 'lodash'
import KpiReportEgoInfo from './kpi-report-ego-info.vue'
import KpiReportModuleTable from './kpi-report-module-table.vue'
import KpiReportKpiTable from './kpi-report-kpi-table.vue'
import KpiReportKpiDetailItem from './kpi-report-kpi-detail-item.vue'
import KpiReportKpiMetaItem from './kpi-report-kpi-meta-item.vue'
import eventBus from '@/event-bus'
import { defaultEgoId } from '@/common/utils'

export default {
  components: {
    KpiReportEgoInfo,
    KpiReportModuleTable,
    KpiReportKpiTable,
    KpiReportKpiDetailItem,
    KpiReportKpiMetaItem,
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
      collapseList: [],
      collapseActive: [],
      defaultCollapseId: -1,
      chosenEgos: {},
      kpiFilters: {},
      useAnimation: true,
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
  watch: {
    first () {
      this.initCollapseList()
    },
  },
  created () {
    this.initCollapseList()
    eventBus.$on('kpi-report-menu-click', this.handleLeftMenuClick)
    eventBus.$on('kpi-report-export-click', this.handleMainExport)
    eventBus.$on('kpi-report-export-finish', this.handleMainExportFinish)
  },
  unmounted () {
    eventBus.$off('kpi-report-menu-click', this.handleLeftMenuClick)
    eventBus.$off('kpi-report-export-click', this.handleMainExport)
    eventBus.$off('kpi-report-export-finish', this.handleMainExportFinish)
  },
  methods: {
    // 初始化数据
    initCollapseList () {
      const chosenEgos = {}
      if (this.isCompare) {
        this.collapseList = [{ id: '0', title: this.getCompareTitle(0) }]
        this.collapseList.forEach(({ id }) => (chosenEgos[id] = [defaultEgoId, defaultEgoId]))
      } else {
        const egoList = this.getEgoList(this.first)
        const collapseList = egoList.map(({ egoId }) => ({ id: egoId, title: egoId }))
        this.collapseList = collapseList
        this.collapseList.forEach(({ id }) => (chosenEgos[id] = [id, id]))
      }
      const { id: defaultId } = this.collapseList[0] || {}
      this.defaultCollapseId = String(defaultId)
      this.collapseActive = [this.defaultCollapseId]
      this.chosenEgos = chosenEgos
    },
    // 对比时新增板块
    handleAddCustom (e) {
      e.stopPropagation()
      if (!this.isCompare) return
      const id = String(this.collapseList.length)
      const title = this.getCompareTitle(id)
      this.collapseList = this.collapseList.concat({ id, title })
      this.chosenEgos = { ...this.chosenEgos, [id]: [defaultEgoId, defaultEgoId] }
      this.$message.success(`已新增${title}`)
    },
    // 对比时删除板块
    handleDeleteCustom (e, id) {
      e.stopPropagation()
      this.collapseList = this.collapseList.filter(e => e.id !== id)
    },
    // 切换主车
    chooseEgo (collapseId, col, ego) {
      const old = this.chosenEgos[collapseId]
      const newArr = old.slice()
      newArr[col] = ego.egoId
      this.chosenEgos[collapseId] = newArr
    },
    // 主菜单点击的回调
    handleLeftMenuClick (e) {
      const data = e.detail
      // 对比时id为0_module，非对比时id为Ego_001_module
      const getEgoId = id => this.isCompare ? id.split('_').shift() : id.split('_').slice(0, 2).join('_')
      const collapseId = getEgoId(data.id)
      if (!collapseId) return
      this.collapseActive = _.uniq(this.collapseActive.concat([collapseId]))
    },
    // 该板块中，指标列表进行筛选，指标详情也相应进行筛选
    handleKpiTableFilter (collapseId, key, value) {
      this.kpiFilters[collapseId] = { ...this.kpiFilters[collapseId], [key]: value }
    },
    // 外部点击导出时，所有板块展开
    handleMainExport () {
      this.useAnimation = false
      this.cacheCollapseActive = this.collapseActive.slice(0)
      this.collapseActive = this.collapseList.map(e => e.id)
    },
    // 外部导出完成时，恢复菜单项之前展开状态
    // 需先排序，从内向外依次触发，不然外层会关闭后又被打开
    handleMainExportFinish () {
      this.collapseActive = this.cacheCollapseActive || []
      setTimeout(() => {
        this.useAnimation = true
      }, 500)
    },
    // 对比评测的标题
    getCompareTitle (index) {
      return `${this.$t('indicator.comparisonTitle')} ${+index + 1}`
    },
    // 获取主车列表
    getEgoList (data) {
      const version = _.has(data, 'meta') ? '1.0' : '2.0'
      let egoList = version === '1.0' ? [{ egoId: defaultEgoId }] : _.get(data, 'report.egoCases', [])
      // 主车列表按 001 002 进行排序
      const getEgoIdNumber = egoId => +egoId.slice(4)
      egoList = egoList.sort((a, b) => getEgoIdNumber(a.egoId) - getEgoIdNumber(b.egoId))
      return egoList
    },
    // 获取该板块中，多任务中，指标列表最大数量
    // TODO: 以后应改为二维数组，且需加上排序
    getMaxCasesCount (collapseId) {
      const max = _.max(this.list.map((data, col) => this.getCasesList(data, collapseId, col).length))
      return max
    },
    // 获取指标详情列表
    getCasesList (data, collapseId, col) {
      const currentEgoId = this.chosenEgos[collapseId][col]
      const version = _.has(data, 'meta') ? '1.0' : '2.0'
      let list = []
      if (version === '1.0') {
        const cases = _.get(data, 'report.cases', [])
        list = cases
      } else {
        const egoCases = _.get(data, 'report.egoCases', [])
        const tmp = egoCases.find(e => e.egoId === currentEgoId)
        list = tmp ? tmp.cases : []
      }
      const shownList = list.filter((item) => {
        const state = item.info.result.state || 'UN_DEFINED'
        const category = item.info.gradingKpi.category.labels || []
        const filter = this.kpiFilters[collapseId] || {}
        if (filter['评测结果']?.length) {
          return filter['评测结果'].includes(state)
        }
        if (filter['指标分类']?.length) {
          return filter['指标分类'].some(label => category.includes(label))
        }
        return true
      })
      return shownList
    },
    // 获取该板块中，多任务中，元数据列表最大数量
    getMaxMetaCount (collapseId) {
      const max = _.max(this.list.map((data, col) => this.getMetaList(data, collapseId, col).length))
      return max
    },
    // 获取指标详情列表
    getMetaList (data, collapseId, col) {
      const currentEgoId = this.chosenEgos[collapseId][col]
      const version = _.has(data, 'meta') ? '1.0' : '2.0'
      if (version === '1.0') {
        const res = _.get(data, 'meta', [])
        return res
      } else {
        const egoMeta = _.get(data, 'egoMeta', [])
        const tmp = egoMeta.find(e => e.egoId === currentEgoId)
        return tmp ? tmp.meta : []
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/var";

.kpi-report-section-ego-detail-list {
  :deep(.el-collapse) {
    .el-collapse-item {
      &:not(:last-child) {
        margin-bottom: 20px;
      }
    }

    .el-collapse-item__wrap {
      box-shadow: inset 0 0 1px @global-bd-color;
    }

    .el-collapse-item__content {
      padding: 10px 10px;
      line-height: normal;
    }
  }

  .kpi-report-detail-main-section {
    &:not(:last-child) {
      margin-bottom: 20px;
    }
  }

  .kpi-report-detail-main-section-title {
    padding-bottom: 12px;
    margin-bottom: 12px;
    border-bottom: 1px solid @global-bd-color;
    line-height: normal;
  }

  .compare-header {
    flex-grow: 1;
    overflow: hidden;
    display: flex;

    & > .tools {
      margin-left: auto;
      padding-right: 20px;
    }
  }

  .kpi-report-compare-wrap {
    display: flex;
    gap: 12px;

    &:not(:last-child) {
      margin-bottom: 20px;
    }

    .kpi-report-compare-item {
      flex-basis: 50%;
      overflow: hidden;
    }
  }

  .ego-list-wrap {
    display: flex;
    gap: 12px;

    .ego-list-title {
      flex-shrink: 0;
      line-height: 24px;
    }

    .ego-list {
      flex-grow: 1;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .ego-list-item {
      padding: 4px 10px;
      background: @global-bg-color;
      max-width: 240px;
      cursor: pointer;

      &.active {
        color: @global-primary-color;
        background: @active-bg-color;
      }
    }
  }

  &.isCompare {
    :deep(.kpi-report-chart-unit-item) {
      display: block;
    }
  }
  &:not(.isCompare) {
    .kpi-report-compare-wrap {
      display: block;
    }
  }

  &:not(.useAnimation) {
    :deep(.el-collapse-item__wrap) {
      transition: none !important;
    }
  }
}
</style>
