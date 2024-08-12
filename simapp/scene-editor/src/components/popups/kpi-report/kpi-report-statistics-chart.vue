<template>
  <div v-if="value">
    <SimChart
      :option="option"
      :width="width"
      :height="height"
    />
  </div>
</template>

<script>
import _ from 'lodash'
import SimChart from '@/components/sim-chart.vue'

export default {
  components: {
    SimChart,
  },
  props: {
    value: {
      type: Object,
      default: () => {},
    },
  },
  data () {
    return {
      width: 350,
      height: 130,
    }
  },
  computed: {
    chartOption () {
      return {
        tooltip: {
          trigger: 'item',
          textStyle: {
            color: '#878787',
            fontSize: 12,
          },
          padding: [4, 10],
          borderColor: '#4F6787',
          backgroundColor: 'rgba(45, 45, 45, 0.75)',
          formatter: '{b}占比: <span style="color:white">{d}%</span>',
          transitionDuration: 0,
        },
        color: ['#1F7485', '#F73222', '#D89614'],
        legend: {
          left: '140',
          bottom: '0',
          padding: [12, 15],
          orient: 'vertical',
          itemHeight: 6,
          itemWidth: 6,
          icon: 'circle',
          backgroundColor: '#111111',
          textStyle: {
            color: '#878787',
            fontSize: 10,
            lineHeight: 8,
          },
        },
        series: [
          {
            type: 'pie',
            radius: ['60%', '100%'],
            center: ['65', '65'],
            legendHoverLink: true,
            hoverAnimation: false,
            label: {
              show: false,
            },
            data: [
              {
                value: 0,
                name: this.$t('indicator.passed'),
                label: {
                  show: true,
                  position: 'center',
                  fontSize: '12',
                  formatter: `${this.$t('indicator.passRate')}:\n{d}%`,
                },
              },
              { value: 0, name: this.$t('indicator.failed') },
              { value: 0, name: 'N/A' },
            ],
          },
        ],
      }
    },
    option () {
      const summary = _.get(this.value, 'report.summary', {})
      const {
        casePassSize = 0,
        caseFailSize = 0,
        caseSize = 0,
      } = summary
      const option = _.merge({}, this.chartOption, {
        series: [
          {
            data: [
              { value: casePassSize },
              { value: caseFailSize },
              { value: caseSize - casePassSize - caseFailSize },
            ],
          },
        ],
      })

      return option
    },
  },
}
</script>

<style scoped>

</style>
