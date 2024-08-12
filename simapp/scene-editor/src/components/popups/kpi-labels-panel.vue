<template>
  <div class="kpi-labels-panel">
    <section class="part-header">
      <el-button icon="plus" @click="handleCreate">
        {{ $t('indicator.createLabel') }}
      </el-button>
    </section>

    <section class="part-details">
      <el-table
        id="kpi-labels-list-table"
        ref="table"
        size="small"
        :data="shownList"
        height="300"
      >
        <el-table-column
          :label="$t('indicator.labelNameCN')"
          prop="labelDescription"
          show-overflow-tooltip
        />
        <el-table-column
          :label="$t('indicator.labelNameEN')"
          prop="labelKey"
          show-overflow-tooltip
        />
        <el-table-column :label="$t('operation.operation')" width="100">
          <template #default="{ row }">
            <el-button
              :disabled="!canModify(row)"
              link
              @click="handleModify(row)"
            >
              {{ $t('menu.edit') }}
            </el-button>
            <el-button
              :disabled="!canDelete(row)"
              link
              @click="handleDelete(row)"
            >
              {{ $t('operation.delete') }}
            </el-button>
          </template>
        </el-table-column>
      </el-table>
    </section>

    <KpiLabelsSettingDialog ref="KpiLabelsSettingDialog" />
  </div>
</template>

<script>
import { mapActions, mapState } from 'vuex'
import KpiLabelsSettingDialog from '@/components/popups/kpi-labels-setting-dialog.vue'
import service from '@/api'

export default {
  name: 'KpiLabelsPanel',
  components: {
    KpiLabelsSettingDialog,
  },
  data () {
    return {
      labelSettingDialog: {
        visible: false,
        data: {},
        action: 'create',
      },
    }
  },
  computed: {
    ...mapState('kpi', [
      'kpiLabelsList',
    ]),
    shownList () {
      return this.kpiLabelsList
    },
  },
  async mounted () {
    await this.refreshKpiLabelsList()
  },
  methods: {
    ...mapActions('kpi', [
      'refreshKpiLabelsList',
    ]),
    canModify (row) {
      return !row.system
    },
    canDelete (row) {
      return !row.system
    },
    async handleCreate () {
      const $dialog = this.$refs.KpiLabelsSettingDialog
      if (!$dialog) return
      $dialog.openDialog({
        action: 'create',
        list: this.kpiLabelsList.map(({ labelKey, labelDescription }) => ({ id: labelKey, name: labelDescription })),
        data: {},
        success: async (res) => {
          if (!res || !res.confirm) return
          try {
            const params = {
              labelKey: res.data.id,
              labelDescription: res.data.name,
            }
            await service.setGradingLabelDescription(params)
            this.$message.success(`${this.$t('operation.create')}${this.$t('tips.success')}`)
            await this.refreshKpiLabelsList()
          } catch (err) {
            console.log(err)
            this.$message.error(err.message)
          }
        },
      })
    },
    async handleModify (row) {
      const $dialog = this.$refs.KpiLabelsSettingDialog
      if (!$dialog) return
      $dialog.openDialog({
        action: 'modify',
        list: this.kpiLabelsList.map(({ labelKey, labelDescription }) => ({ id: labelKey, name: labelDescription })),
        data: {
          id: row.labelKey,
          name: row.labelDescription,
        },
        success: async (res) => {
          if (!res || !res.confirm) return
          try {
            const params = {
              labelKey: res.data.id,
              labelDescription: res.data.name,
            }
            await service.setGradingLabelDescription(params)
            this.$message.success(`${this.$t('menu.edit')}${this.$t('tips.success')}`)
            await this.refreshKpiLabelsList()
          } catch (err) {
            console.log(err)
            this.$message.error(err.message)
          }
        },
      })
    },
    async handleDelete (row) {
      try {
        await this.$confirmBox('确认要删除该标签吗')
      } catch (e) {
        return
      }
      console.log('delete', row)
      try {
        await service.delGradingLabelDescription({
          labelKey: row.labelKey,
        })
        this.$message.success(`${this.$t('operation.delete')}${this.$t('tips.success')}`)
        await this.refreshKpiLabelsList()
      } catch (err) {
        console.log(err)
        this.$message.error(err.message)
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.kpi-labels-panel {
  .part-header {
    padding: 20px 0;
  }

  .part-details {
    background-color: #111;

    :deep(.el-button.is-disabled) {
      background: none;
      border: none;
    }
  }
}
</style>
