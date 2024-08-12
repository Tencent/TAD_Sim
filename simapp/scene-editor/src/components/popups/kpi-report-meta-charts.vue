<template>
  <div class="kpi-report-meta-charts">
    <div
      v-for="(item, index) in metaList"
      :key="index"
      class="kpi-report-meta-list-item"
    >
      <div
        v-for="(meta, i) in item"
        :key="i"
        class="kpi-report-meta-item"
      >
        <div class="kpi-report-meta-info">
          <div class="kpi-report-meta-name">
            <span>{{ index + 1 }}. {{ meta.name }}</span>
            <span v-if="isCompare && item.length >= 2">
              （{{ $t('indicator.jobResultTitle', { i }) }}）
            </span>
          </div>
        </div>
        <div class="kpi-report-meta-chart">
          <KpiChartNew :data="meta" />
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import KpiChartNew from '@/components/kpi-chart-new.vue'

export default {
  name: 'KpiReportMetaCharts',
  components: {
    KpiChartNew,
  },
  props: {
    first: {
      type: Object,
      default () {
        return null
      },
    },
    second: {
      type: Object,
      default () {
        return null
      },
    },
    isCompare: {
      type: Boolean,
      default: false,
    },
  },
  computed: {
    metaList () {
      const { first, second, isCompare } = this
      if (!isCompare && first) {
        return _.get(first, 'meta', []).map(meta => [meta])
      }
      if (first && second) {
        const firstMeta = _.get(first, 'meta', [])
        const secondMeta = _.get(second, 'meta', [])
        const topics = _.uniq(firstMeta.concat(secondMeta).map(meta => meta.name))
        return topics.map((topic) => {
          return [
            firstMeta.find(meta => meta.name === topic),
            secondMeta.find(meta => meta.name === topic),
          ].filter(e => !!e)
        })
      }
      return []
    },
  },
  watch: {
    metaList () {
      console.log('= metaList =', this.metaList)
    },
  },
  mounted () {
    console.log('= metaList =', this.metaList)
  },
}
</script>

<style lang="less" scoped>
@import "@/assets/less/mixins";

.kpi-report-meta-charts {
  .kpi-report-meta-list-item {
    margin-bottom: 22px;

    &:last-child {
      margin-bottom: 0;
    }
  }

  .kpi-report-meta-item {
    margin-bottom: 14px;

    &:last-child {
      margin-bottom: 0;
    }
  }

  .kpi-report-meta-name {
    color: #B8B8B8;
    margin-bottom: 14px;
  }
}
</style>
