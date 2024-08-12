<template>
  <div class="module-chart">
    <el-table
      class="table"
      :data="list"
    >
      <el-table-column
        :label="$t('module.module')"
        prop="name"
        :formatter="formatModuleName"
        show-overflow-tooltip
      />
      <el-table-column
        width="60"
        :label="$t('module.category')"
        prop="groupName"
        :formatter="formatModuleGroupName"
        show-overflow-tooltip
      />
      <el-table-column
        width="80"
        :label="$t('module.isConnected')"
      >
        <template #default="scope">
          <span :class="{ success: scope.row.connected, error: !scope.row.connected }" />
        </template>
      </el-table-column>
      <el-table-column
        width="80"
        :label="$t('module.isInitialized')"
      >
        <template #default="scope">
          <span :class="{ success: scope.row.initiated, error: !scope.row.initiated }" />
        </template>
      </el-table-column>
      <el-table-column
        width="70"
        :label="$t('module.stepTime')"
        prop="timeCost"
      />
      <el-table-column
        width="60"
        label="FPS"
        prop="fps"
      />
      <template #empty>
        {{ $t('tips.noData') }}
      </template>
    </el-table>
  </div>
</template>

<script>
import { mapActions, mapGetters, mapState } from 'vuex'
import { multiEgosModuleNameReg } from '@/common/utils'

export default {
  name: 'ModuleChart',
  computed: {
    ...mapState('chart-bar', [
      'moduleStatus',
      'moduleTimeCost',
    ]),
    ...mapGetters('sim-modules', [
      'modulesActive',
    ]),
    list () {
      if (Object.keys(this.moduleStatus).length < 1) return []
      const list = []
      this.modulesActive.forEach(({ name, groupName }) => {
        const { initiated, connected } = this.moduleStatus[name] || {}
        const timeCost = this.moduleTimeCost[name]
        const fps = this.fps(timeCost)
        list.push({
          name,
          groupName,
          initiated,
          connected,
          timeCost,
          fps,
        })
      })
      return list
    },
  },
  created () {
    this.getModuleSetList()
  },
  methods: {
    ...mapActions('module-set', [
      'getModuleSetList',
    ]),
    fps (duration) {
      if (duration === undefined) {
        return ''
      } else {
        if (duration < 1) {
          return '>1000'
        } else {
          return Math.round(1000 / duration)
        }
      }
    },
    formatModuleName (row, column, value) {
      const result = value.match(multiEgosModuleNameReg)
      if (row.groupName !== 'global' && result?.length) {
        return result[2]
      }
      return value
    },
    formatModuleGroupName (row, column, value) {
      if (value === 'global') {
        return this.$t('module.global')
      }
      return value
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.module-chart {
  width: 452px;

  .success,
  .error {
    display: inline-block;
    width: 8px;
    height: 8px;
    border-radius: 50%;
  }

  .success {
    background-color: #67C23A;
  }

  .error {
    background-color: #F56C6C;
  }

  :deep(td), :deep(th) {
    .cell {
      color: @title-font-color;
    }
  }
}
</style>
