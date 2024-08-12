<!-- suppress NonAsciiCharacters -->
<template>
  <div class="modal-browser-window" :class="{ printing }">
    <div class="modal-browser-window-header">
      <div class="modal-browser-window-title">
        {{ $t('indicator.detailPopTitle') }}
      </div>
      <div class="modal-browser-window-close" @click="handleClose">
        <el-icon class="el-icon-close">
          <close />
        </el-icon>
      </div>
    </div>

    <div v-loading="loading" class="modal-browser-window-body kpi-report-detail">
      <div class="kpi-report-detail-nav">
        <KpiReportMenu :first="first" :second="second" />
        <div class="nav-bottom-tools">
          <el-button link class="btn" @click="handleExport('pdf')">
            {{ $t('indicator.exportPDF') }}
          </el-button>
          <el-button link class="btn" @click="handleExport('excel')">
            {{ $t('indicator.openExcelFolder') }}
          </el-button>
        </div>
      </div>
      <div v-if="first" class="kpi-report-detail-main">
        <section class="kpi-report-detail-basic">
          <div id="kpi-report-detail-basic" class="hash-target" />
          <KpiReportSectionBase :first="first" :second="second" />
        </section>
        <section class="kpi-report-global-module-list">
          <div id="kpi-report-global-module-list" class="hash-target" />
          <Suspense>
            <template #fallback>
              loading...
            </template>
            <KpiReportSectionGlobalModule :first="first" :second="second" />
          </Suspense>
        </section>
        <section class="kpi-report-ego-data-list">
          <div id="kpi-report-ego-data-list" class="hash-target" />
          <Suspense>
            <template #fallback>
              loading...
            </template>
            <KpiReportSectionEgoKpiList :first="first" :second="second" />
          </Suspense>
        </section>
      </div>
    </div>
  </div>
</template>

<script>
import { cloneDeep, get, isArray, isNumber } from 'lodash-es'
import { defineAsyncComponent } from 'vue'
import { mapActions, mapState } from 'vuex'
import KpiReportMenu from './kpi-report/kpi-report-menu.vue'
import KpiReportSectionBase from './kpi-report/kpi-report-section-base.vue'
import { errorHandler } from '@/common/errorHandler'
import { delay, getReportFileSuffix } from '@/common/utils'
import eventBus from '@/event-bus'

const KpiReportSectionGlobalModule = defineAsyncComponent(() => import('./kpi-report/kpi-report-section-global-module.vue'))
const KpiReportSectionEgoKpiList = defineAsyncComponent(() => import('./kpi-report/kpi-report-section-ego-kpi-list.vue'))

const { electron } = window
const { dialog, ModalBrowserWindow, crossBrowserWindowMessage, path } = electron

const getReportFileName = filePath => filePath.split(/\\|\//).pop().replace('.dict', '')

export default {
  name: 'KpiReportGeneric',
  components: {
    KpiReportMenu,
    KpiReportSectionBase,
    KpiReportSectionGlobalModule,
    KpiReportSectionEgoKpiList,
  },
  provide () {
    return {
      autoPrint: !!this.$route.query.autoPrint,
    }
  },
  data () {
    return {
      loading: false,
      isCompare: false,
      list: [],
      printing: false,
      ids: null,
      comparedIds: null,
      autoPrint: null,
      winId: null,
    }
  },
  computed: {
    ...mapState('kpi', [
      'kpiLabelsList',
    ]),
    first () {
      return this.list[0]
    },
    second () {
      return this.list[1]
    },
  },
  async created () {
    const { comparedIds, autoPrint, ids, winId, missionSessionId } = this.$route.query
    this.ids = ids
    this.comparedIds = comparedIds
    this.autoPrint = autoPrint
    this.winId = winId
    this.missionSessionId = missionSessionId
    if (comparedIds && comparedIds.length > 1) {
      this.isCompare = true
    }
  },
  async mounted () {
    await this.refreshKpiLabelsList()
    this.getAllCatalogs()
    if (!this.ids?.length && this.missionSessionId) {
      const list = await new Promise((resolve, reject) => {
        // 向报告管理页面请求批量数据
        crossBrowserWindowMessage.once('kpi-report-print-send-list', resolve)
        crossBrowserWindowMessage.emit('kpi-report-print-get-list', this.missionSessionId)
        setTimeout(() => {
          reject(new Error('Timeout'))
        }, 5000)
      })
      const len = list.length
      let stop = false
      crossBrowserWindowMessage.once('kpi-report-print-cancel', () => {
        stop = true
      })
      for (let i = 0; i < len; i += 1) {
        const id = list[i]
        if (stop) break
        try {
          this.ids = id
          await this.initData()
          // 延迟一下等页面渲染
          await delay(200)
          await this.handleExport('pdf')
          const percentage = i === len - 1 ? 100 : 100 / len * (i + 1)
          crossBrowserWindowMessage.emit('kpi-report-print-process', percentage)
        } catch (e) {
          console.log(e)
        }
      }
      crossBrowserWindowMessage.emit('kpi-report-print-finish')
    } else {
      await this.initData()
    }
  },
  methods: {
    ...mapActions('kpi-report', [
      'getKpiReport',
    ]),
    ...mapActions('kpi', [
      'refreshKpiLabelsList',
    ]),
    ...mapActions('planners', [
      'getAllCatalogs',
    ]),
    async initData () {
      const { ids, comparedIds } = this
      const list = []

      this.loading = true
      const data1 = await this.getAndMergeKpiReport(ids)
      list.push(data1)
      if (comparedIds && comparedIds.length) {
        const data2 = await this.getAndMergeKpiReport(comparedIds)
        list.push(data2)
      }
      this.list = list
      this.loading = false
    },
    async getAndMergeKpiReport (ids) {
      let result = null
      if (isArray(ids)) {
        ids.sort((id1, id2) => {
          const suffix1 = getReportFileSuffix(id1)
          const suffix2 = getReportFileSuffix(id2)
          if (!suffix1) return -1
          if (!suffix2) return 1
          return 0
        })
      } else {
        ids = [ids]
      }

      for (const id of ids) {
        try {
          const data = await this.getKpiReport(id)
          data.id = getReportFileName(id)
          console.log('==getKpiReport', cloneDeep(data))
          if (!result) {
            result = data
          } else {
            // 合并 summary
            const summary = get(result, 'report.summary')
            const newSummary = get(data, 'report.summary')
            if (summary && newSummary) {
              Object.keys(summary).forEach((key) => {
                if (isNumber(summary[key]) && isNumber(newSummary[key])) {
                  summary[key] += newSummary[key]
                }
              })
            }

            // 合并指标详情
            const cases = get(result, 'report.cases')
            const newCases = get(data, 'report.cases')
            if (cases && newCases) {
              const repeatNames = {}
              newCases.forEach((kase) => {
                if (cases.findIndex(c => c.info.name === kase.info.name) > -1) {
                  if (kase.info.name in repeatNames) {
                    repeatNames[kase.info.name] += 1
                  } else {
                    repeatNames[kase.info.name] = 1
                  }
                  kase.info.name += `(${repeatNames[kase.info.name]})`
                }
                cases.push(kase)
              })
            }

            // 合并结果
            const rst = get(result, 'report.info.result')
            const newRst = get(data, 'report.info.result')
            if (rst && newRst) {
              // 两个都通过才算通过
              rst.state = (rst.state === 'PASS' && newRst.state === 'PASS') ? 'PASS' : 'FAIL'
              rst.reason += `\n${newRst.reason}`
            }
          }
        } catch (e) {
          await errorHandler(e)
        }
      }
      return result
    },
    async handleExport (type) {
      if (this.first) {
        const endSecond = this.first?.report.info.date.endSecond
        if (!endSecond) return
        const { autoPrint, ids, winId } = this
        let id
        if (isArray(ids)) {
          ids.sort((id1, id2) => {
            const suffix1 = getReportFileSuffix(id1)
            const suffix2 = getReportFileSuffix(id2)
            if (!suffix1) return -1
            if (!suffix2) return 1
            return 0
          })
          id = ids[0]
        } else {
          id = ids
        }
        const date = new Date(endSecond * 1000)
        if (type === 'pdf') {
          let filePath
          if (autoPrint) {
            const fileName = id.split(path.sep).pop()
            filePath = `${autoPrint}${path.sep}${fileName}.pdf`
          } else {
            const dateStr = `${date.getFullYear()}_${date.getMonth() + 1}_${date.getDate()}_${date.getHours()}_${date.getMinutes()}_${date.getSeconds()}`;
            ({ filePath } = await dialog.showSaveDialog({
              title: this.$t('indicator.exportReportTitle'),
              defaultPath: `${dateStr}.${type}`,
            }))
          }
          if (filePath && filePath.length) {
            eventBus.$emit('kpi-report-export-click')
            window.scrollTo(0, 0)
            this.printing = true
            return new Promise((resolve, reject) => {
              this.$nextTick(async () => {
                const printOptions = {
                  marginsType: 1,
                  pageSize: 'A4',
                  scaleFactor: 67,
                }
                try {
                  if (winId) {
                    await ModalBrowserWindow.writeToPDFById(+winId, filePath, printOptions)
                  } else {
                    await ModalBrowserWindow.writeToPDF(filePath, printOptions)
                  }
                  this.$message.success(this.$t('tips.exportSuccess'))
                  resolve()
                } catch (e) {
                  this.$message.error(this.$t('tips.exportFail'))
                  reject(e)
                } finally {
                  this.printing = false
                  eventBus.$emit('kpi-report-export-finish')
                }
              })
            })
          }
        } else if (type === 'excel') {
          const result = /(\d{4}_\d{1,2}_\d{1,2})_\d{1,2}_\d{1,2}_\d{1,2}(\.dict|\.pblog\.json)$/.exec(id)
          if (result && result.length) {
            electron.editor.openKpiReportPath('pblog')
          } else {
            this.$message.error(this.$t('tips.notFoundExcel'))
          }
        }
      }
    },
    handleClose () {
      ModalBrowserWindow.closeWindow()
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/var";
@import "@/assets/less/mixins";
@import "@/assets/less/modal-browser-window";

.modal-browser-window {
  &.printing {
    pointer-events: none;
    .modal-browser-window-header {
      pointer-events: auto;
    }
  }
}

.kpi-report-detail {
  display: flex;

  :deep(&.modal-browser-window-body) {
    font-size: inherit;
    color: @global-font-color;
    background-color: @global-body-color;
  }

  .kpi-report-detail-nav {
    flex-shrink: 0;
    position: sticky;
    top: 28px;
    width: 10em;
    height: calc(100vh - 28px);
    border-right: 1px solid @global-bg-color;
    display: flex;
    flex-direction: column;

    & > :deep(.kpi-report-menu-wrap) {
      flex-grow: 1;
      overflow-y: auto;
      overflow-x: hidden;
    }

    .nav-bottom-tools {
      flex-shrink: 0;
      display: flex;
      flex-direction: column;
      gap: 1px;
      box-sizing: border-box;

      & > .btn {
        display: flex;
        height: 33px;
        align-items: center;
        --el-button-text-color: var(--el-text-color-secondary);
        background-color: var(--el-button-bg-color);

        & > :deep(span) {
          justify-content: center;
        }
      }

      :deep(.el-button+.el-button) {
        margin-left: 0;
      }
    }
  }

  .kpi-report-detail-main {
    flex-grow: 1;
    overflow: hidden;
    padding: 12px;
    box-sizing: border-box;
  }

  :deep(.hash-target) {
    position: relative;
    top: -36px;
  }
}
</style>
