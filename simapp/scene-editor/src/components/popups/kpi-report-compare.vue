<template>
  <el-dialog
    :model-value="true"
    :header="$t('indicator.comparisonReports')"
    append-to-body
    :close-on-click-modal="false"
    class="kpi-report-compare-dialog"
    @close="$emit('close')"
  >
    <div v-loading="loading" class="kpi-report-compare">
      <section class="part-details">
        <div class="kpi-report-compare-item">
          {{ $t('indicator.comparison') }} 1
          <el-select
            id="kpi-report-select-01"
            v-model="selected[0]"
          >
            <el-option
              v-for="item in list01"
              :key="item.id"
              :label="item.label"
              :value="item.id"
            />
          </el-select>
        </div>
        <div class="kpi-report-compare-item">
          {{ $t('indicator.comparison') }} 2
          <el-select
            id="kpi-report-select-02"
            v-model="selected[1]"
          >
            <el-option
              v-for="item in list02"
              :key="item.id"
              :label="item.label"
              :value="item.id"
            />
          </el-select>
        </div>
      </section>

      <section class="part-buttons">
        <el-button id="kpi-report-compare-cancel" class="dialog-cancel" @click="$emit('close')">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button id="kpi-report-compare-confirm" class="dialog-cancel" @click="handleConfirm">
          {{ $t('operation.contrast') }}
        </el-button>
      </section>
    </div>
  </el-dialog>
</template>

<script>
import { mapActions, mapState } from 'vuex'

export default {
  name: 'KpiReportCompare',
  data () {
    return {
      selected: ['', ''],
    }
  },
  computed: {
    ...mapState('kpi-report', [
      'loading',
      'kpiReportList',
    ]),
    reallyKpiReport () {
      return this.kpiReportList.map((item) => {
        const id = `${item.name} ${item.time}`
        return { ...item, id, label: id }
      })
    },
    list01 () {
      return this.reallyKpiReport.filter((item) => {
        return item.paths !== this.selected[1]
      })
    },
    list02 () {
      return this.reallyKpiReport.filter((item) => {
        return item.paths !== this.selected[0]
      })
    },
  },
  async created () {
    // 没必要，而且刷新了表格里的选中情况
    // await this.getKpiReportList()
  },
  methods: {
    ...mapActions('kpi-report', [
      'getKpiReportList',
    ]),
    handleConfirm () {
      const [first, second] = this.selected
      if (first && second) {
        const firstItem = this.reallyKpiReport.find(item => item.id === first)
        const secondItem = this.reallyKpiReport.find(item => item.id === second)
        const selection = [firstItem.paths, secondItem.paths]
        // TODO: 原始数据 paths 是字符串，多主车后为数组，路由传递的 id 需改变
        this.$emit('confirm', selection)
      } else {
        this.$message({
          type: 'error',
          message: this.$t('tips.selectTwoEvaluation'),
        })
      }
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .kpi-report-compare-dialog {
    margin-top: 120px;

    .kpi-report-compare {
      width: 360px;
      height: auto;
      padding: 23px;

      .kpi-report-compare-item {
        margin: 30px 0;
        font-size: 13px;

        :deep(.el-select) {
          width: 300px;
          margin-left: 10px;
        }
      }

      .part-buttons {
        margin-top: 10px;
        height: 40px;
        line-height: 40px;
        text-align: right;
      }
    }
  }
</style>
