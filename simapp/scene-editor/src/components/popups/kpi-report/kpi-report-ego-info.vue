<template>
  <section class="kpi-report-ego-info">
    <div class="kpi-report-ego-info-list">
      <template v-for="item in list" :key="item['名称']">
        <div class="kpi-report-info-item">
          <div class="kpi-report-info-label">
            {{ item['名称'] }}
          </div>
          <div class="kpi-report-info-value">
            {{ item['值'] }}
          </div>
        </div>
      </template>
    </div>
  </section>
</template>

<script>
import _ from 'lodash'
import { mapState } from 'vuex'
import { convertKpiSheetData, defaultEgoId, getModelDisplayName } from '@/common/utils'

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
  },
  computed: {
    ...mapState('planners', [
      'plannerList',
    ]),
    list () {
      const version = _.has(this.data, 'meta') ? '1.0' : '2.0'
      let data = []
      if (version === '1.0') {
        data = [
          { 名称: '主车ID', 值: this.egoId },
          { 名称: '主车模型', 值: '' },
          { 名称: '主车控制器名称', 值: '' },
        ]
      } else {
        const moduleCfg = _.get(this.data, 'report.info.egoCfg', [])
        const module = moduleCfg.find(e => (e.name === this.egoId)) || {}
        const sheetData = module.sheetData || []
        const res = convertKpiSheetData(sheetData)
        data = res.data
        const nameItem = data.find(e => e['名称'] === '主车模型')
        if (nameItem && nameItem['值']) {
          nameItem['值'] = this.getEgoCnName(nameItem['值'])
        }
      }
      return data
    },
  },
  methods: {
    getEgoCnName (plannerType) {
      const match = this.plannerList.find(e => e.variable === plannerType)
      if (!match) return ''
      return getModelDisplayName(match)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.kpi-report-info-item {
  display: flex;

  &:not(:last-child) {
    margin-bottom: 10px;
  }

  .kpi-report-info-label {
    color: @global-info-color;
    width: 100px;
    padding-right: 15px;
  }

  .kpi-report-info-value {
    flex: 1;
    color: @global-font-color;
    .text-overflow();
  }
}
</style>
