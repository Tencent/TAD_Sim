<template>
  <div class="kpi-management">
    <el-tabs v-model="activeTab">
      <el-tab-pane :label="$t('indicator.groupConf')" name="group">
        <KpiGroup
          v-if="activeTab === 'group'"
          @close="$emit('close')"
        />
      </el-tab-pane>
      <el-tab-pane :label="$t('indicator.list')" name="kpi">
        <KpiSetting
          v-if="activeTab === 'kpi'"
          @close="$emit('close')"
        />
      </el-tab-pane>
      <el-tab-pane :label="$t('indicator.labels')" name="labels">
        <KpiLabelsPanel
          v-if="activeTab === 'labels'"
          @close="$emit('close')"
        />
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import { mapActions } from 'vuex'
import KpiGroup from './kpi-group.vue'
import KpiSetting from './kpi-setting.vue'
import KpiLabelsPanel from './kpi-labels-panel.vue'

export default {
  name: 'KpiManagement',
  components: {
    KpiGroup,
    KpiSetting,
    KpiLabelsPanel,
  },
  data () {
    return {
      activeTab: 'group',
    }
  },
  mounted () {
    this.refreshKpiLabelsList()
  },
  methods: {
    ...mapActions('kpi', [
      'refreshKpiLabelsList',
    ]),
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .kpi-management {
    width: 600px;
    height: auto;
    padding: 23px;
  }
</style>
