<template>
  <div v-loading="loading" class="kpi-report-management">
    <section class="part-search">
      <el-input
        id="kpi-report-name"
        :model-value="keyword"
        style="width: 200px;"
        prefix-icon="search"
        :placeholder="($t('operation.searchName'))"
        @input="updateState({ keyword: $event })"
      />
    </section>

    <section class="part-batch">
      <div />
      <div>
        <el-tooltip effect="dark" :content="$t('operation.export')" placement="bottom">
          <span
            id="kpi-reports-export"
            class="kpi-report-icon"
            :class="{ disabled: !selected.length || exporting }"
            @click="batchExportPDF(selected)"
          >
            <el-icon class="icon el-icon-upload2" style="font-size:12px;"><upload /></el-icon>
          </span>
        </el-tooltip>
        <!-- 暂时做不了导出Excel -->
        <el-dropdown v-if="false" trigger="click" placement="top-start" @command="batchExport">
          <span
            class="kpi-report-icon"
            :class="{ disabled: !selected.length || exporting }"
          >
            <el-icon class="icon el-icon-upload2" style="font-size:12px;"><upload /></el-icon>
          </span>
          <template #dropdown>
            <el-dropdown-menu>
              <el-dropdown-item command="pdf">
                {{ $t('indicator.exportPDF') }}
              </el-dropdown-item>
              <el-dropdown-item command="excel">
                {{ $t('indicator.exportExcel') }}
              </el-dropdown-item>
            </el-dropdown-menu>
          </template>
        </el-dropdown>
        <el-tooltip effect="dark" :content="$t('indicator.comparison')" placement="bottom">
          <span
            id="kpi-reports-compare"
            class="kpi-report-icon"
            :class="{ disabled: kpiReportList.length < 2 }"
            @click="kpiReportCompareVisible = true"
          >
            <compare-svg />
          </span>
        </el-tooltip>
        <el-tooltip effect="dark" :content="$t('operation.delete')" placement="bottom">
          <span
            id="kpi-reports-delete"
            class="kpi-report-icon"
            :class="{ disabled: !selected.length || exporting }"
            @click="handleDelete"
          >
            <el-icon class="icon el-icon-delete" style="font-size:12px;"><delete /></el-icon>
          </span>
        </el-tooltip>
      </div>
    </section>

    <section class="part-details">
      <el-table
        id="kpi-report-table"
        ref="table"
        size="small"
        :data="filteredKpiReportList"
        height="270"
        row-key="name"
        @selection-change="tableSelectionChange"
      >
        <el-table-column
          type="selection"
          label-class-name="select-all"
          width="36"
          :selectable="canSelect"
        />
        <el-table-column
          :label="$t('name')"
          prop="name"
          class-name="table-row"
          show-overflow-tooltip
        />
        <el-table-column
          :label="$t('time')"
          prop="time"
          class-name="table-row"
          show-overflow-tooltip
        />
        <el-table-column
          :label="$t('indicator.evaluationResult')"
          class-name="table-row"
          show-overflow-tooltip
        >
          <template #default="scope">
            <el-tag
              :class="{
                green: scope.row.pass,
                red: !scope.row.pass,
              }"
            >
              {{ $kpiPass(scope.row.pass) }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column
          :label="$t('operation.operation')"
          prop="id"
          width="70"
        >
          <template #default="scope">
            <el-button
              v-if="canSelect(scope.row)"
              class="check"
              link
              @click="handleView(scope.row.paths)"
            >
              {{ $t('operation.check') }}
            </el-button>
            <el-tooltip
              v-else
              content="报告生成中，请稍后"
            >
              <el-button
                class="disabled"
                link
              >
                {{ $t('operation.check') }}
              </el-button>
            </el-tooltip>
          </template>
        </el-table-column>
      </el-table>
    </section>

    <section class="part-buttons">
      <el-button
        id="kpi-report-close"
        class="dialog-cancel"
        @click="$emit('close')"
      >
        {{ $t('operation.close') }}
      </el-button>
    </section>

    <KpiReportCompare
      v-if="kpiReportCompareVisible"
      @close="kpiReportCompareVisible = false"
      @confirm="handleKpiReportCompareConfirm"
    />
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import KpiReportCompare from './kpi-report-compare.vue'
import { delay } from '@/common/utils'
import i18n from '@/locales'

const { electron: { ModalBrowserWindow, dialog, crossBrowserWindowMessage, path, kpi } } = window

export default {
  name: 'KpiReportManagement',
  components: {
    KpiReportCompare,
  },
  data () {
    return {
      selected: [],
      kpiReportCompareVisible: false,
    }
  },
  computed: {
    ...mapState('kpi-report', [
      'loading',
      'keyword',
      'kpiReportList',
      'exporting',
    ]),
    ...mapGetters('kpi-report', [
      'filteredKpiReportList',
    ]),
  },
  created () {
    this.startPolling()
  },
  beforeUnmount () {
    this.updateState({ keyword: '' })
    this.endPolling()
  },
  methods: {
    ...mapMutations('kpi-report', [
      'updateState',
    ]),
    ...mapActions('kpi-report', [
      'getKpiReportList',
      'destroyKpiReport',
      'startPolling',
      'endPolling',
    ]),
    showKpiReportDetailBrowserWindow (ids, comparedIds = []) {
      const search = new URLSearchParams()
      ids.forEach((id) => {
        search.append('ids', id)
      })
      if (comparedIds && comparedIds.length) {
        comparedIds.forEach((id) => {
          search.append('comparedIds', id)
        })
      }
      this.updateState({ loading: true })
      ModalBrowserWindow.createWindow(`${location.href}kpi-report-generic?${search}`, {
        width: comparedIds.length ? 1340 : 1190,
        height: 720,
      })
      setTimeout(() => {
        this.updateState({ loading: false })
      }, 1000 * 5)
    },
    tableSelectionChange (selection) {
      this.selected = selection
    },
    async handleDelete () {
      const ids = _.flatten(this.selected.map(item => item.paths))
      try {
        await this.$confirmBox(this.$t('tips.whetherDeleteCurrentOption'))
        await this.destroyKpiReport(ids)
      } catch (e) {
        // pass
      }
    },
    handleView: _.throttle(function (ids) {
      this.showKpiReportDetailBrowserWindow(ids)
    }, 1000, { trailing: false }),
    handleKpiReportCompareConfirm ([ids1, ids2]) {
      this.showKpiReportDetailBrowserWindow(ids1, ids2)
      this.$nextTick(() => {
        this.kpiReportCompareVisible = false
      })
    },
    batchExport (type) {
      switch (type) {
        case 'pdf':
          this.batchExportPDF(this.selected)
          break
        case 'excel':
          this.batchExportExcel(this.selected)
          break
        default:
      }
    },
    async batchExportPDF (selected) {
      if (!selected.length) return

      let stop = false
      try {
        const { canceled, filePaths } = await dialog.showOpenDialog({
          properties: ['openDirectory'],
        })
        if (canceled) return
        const loadingInstance = this.$progressMessage({
          message: this.$t('tips.exporting'),
          onClose: () => {
            stop = true
            crossBrowserWindowMessage.emit('kpi-report-print-cancel')
          },
        })
        const setPercentage = (percentage) => {
          loadingInstance.updateProgress(percentage)
        }
        // 处理进度事件
        crossBrowserWindowMessage.on('kpi-report-print-process', setPercentage)
        // 响应批量打印数据请求
        crossBrowserWindowMessage.once('kpi-report-print-get-list', () => {
          crossBrowserWindowMessage.emit('kpi-report-print-send-list', selected.map(s => s.paths))
        })

        this.updateState({ exporting: true })
        const search = new URLSearchParams()
        search.append('missionSessionId', `${Date.now()}`)
        search.append('autoPrint', filePaths[0])
        const winId = await ModalBrowserWindow.createWindow(null, {
          width: 1190,
          height: 720,
          show: false,
        }, false)
        search.append('winId', winId)
        const url = `${location.href}kpi-report-generic?${search}`
        await ModalBrowserWindow.loadURLById(winId, url)
        await new Promise((resolve) => {
          crossBrowserWindowMessage.once('kpi-report-print-finish', resolve)
        })
        crossBrowserWindowMessage.off('kpi-report-print-process', setPercentage)
        if (!stop) {
          // 点了取消则不延迟
          await delay(400)
        }
        ModalBrowserWindow.closeWindowById(winId)
        if (stop) {
          this.$message.warning(i18n.t('tips.cancelled'))
        } else {
          this.$message.success(i18n.t('tips.exportSuccess'))
        }
        loadingInstance.close()
        this.updateState({ exporting: false })
      } catch (e) {
        console.log(e)
      }
    },
    async batchExportExcel (selected) {
      const { canceled, filePaths } = await dialog.showOpenDialog({
        properties: ['openDirectory'],
      })
      if (canceled) return
      const reg = /^P|F\.(.*(\d{4}_\d{1,2}_\d{1,2})_\d{1,2}_\d{1,2}_\d{1,2})(\.dict|\.pblog\.json)$/
      const fileList = []
      selected.forEach((item) => {
        const logFileName = item.paths[0].split(path.sep).pop()
        const execResult = reg.exec(logFileName)
        if (execResult && execResult.length) {
          const [, sceneFileName, excelPath] = execResult
          fileList.push(`${excelPath}${path.sep}${sceneFileName}.pblog.xlsx`)
        }
      })
      kpi.copyExcel(fileList, filePaths[0])
    },
    canSelect (row) {
      const { paths } = row
      return paths.findIndex(item => item.endsWith('.tmp')) === -1
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

.kpi-report-management {
  width: 600px;
  height: auto;
  padding: 23px;

  .part-search {
    text-align: center;
    margin-top: 10px;
    margin-bottom: 23px;
  }

  .part-batch {
    height: 30px;
    line-height: 30px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-top: -13px;
    margin-bottom: 10px;

    .kpi-report-icon {
      color: @title-font-color;

      &:first-child {
        svg {
          transform: scale(1.2);
          color: @title-font-color;
        }
      }

      &.disabled {
        pointer-events: none;
        opacity: 0.5;
      }
    }

    .icon {
      color: @global-font-color;
      font-size: 16px;
      margin: 0 10px;
      cursor: pointer;

      &:hover {
        color: white;
      }
    }
  }

  .part-details {
    :deep(.check) {
      color: @active-font-color;
    }

    :deep(.disabled) {
      color: @disabled-color;
      cursor: not-allowed;
    }

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
  }

  .part-buttons {
    margin-top: 10px;
    height: 40px;
    line-height: 40px;
    text-align: right;
  }
}
</style>
