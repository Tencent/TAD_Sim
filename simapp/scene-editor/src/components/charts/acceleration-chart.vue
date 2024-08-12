<template>
  <div class="acceleration-chart">
    <SimChart
      ref="chart"
      :option="chartOption"
      :width="406"
      :height="210"
      :lazy="true"
    />
  </div>
</template>

<script>
import eventBus from '@/event-bus'
import { getGradingKPIList } from '@/stored/grading-kpi'
import SimChart from '@/components/sim-chart.vue'

export default {
  name: 'AccelerationChart',
  components: {
    SimChart,
  },
  data () {
    return {
      chartOption: {
        title: {
          text: 'Acceleration/Time',
        },
        tooltip: {
          trigger: 'axis',
          formatter ([{ data: value, axisValue }]) {
            const timeStamp = axisValue * 1000

            const hours = `${Math.floor(timeStamp / 3600000)}`.padStart(2, '0')
            const minutes = `${Math.floor((timeStamp % 3600000) / 60000)}`.padStart(2, '0')
            const seconds = `${Math.floor((timeStamp % 60000) / 1000)}`.padStart(2, '0')
            const milliseconds = `${Math.floor(timeStamp % 1000)}`.padStart(3, '0')

            return `Time stamp: ${hours}:${minutes}:${seconds}.${milliseconds}<br />Acceleration: ${(value || 0).toFixed(3)}`
          },
          axisPointer: {
            animation: false,
          },
        },
        xAxis: {
          type: 'category',
          splitLine: {
            show: false,
          },
          boundaryGap: false,
          data: [],
        },
        yAxis: {
          name: 'm/s²',
          type: 'value',
          boundaryGap: [0, '100%'],
          splitLine: {
            lineStyle: {
              color: '#333',
            },
          },
        },
        series: [{
          name: 'Acceleration',
          type: 'line',
          showSymbol: false,
          hoverAnimation: false,
          data: [],
        }],
      },
    }
  },
  created () {
    eventBus.$on('grading-kpi', this.renderChart)
  },
  mounted () {
    this.renderChart({ detail: getGradingKPIList() })
  },
  beforeUnmount () {
    eventBus.$off('grading-kpi', this.renderChart)
  },
  methods: {
    renderChart (event) {
      const val = event?.detail
      this.$refs.chart.setOption({
        xAxis: {
          data: val.map(item => `${item.timeStamp / 1000}`),
        },
        series: [{
          data: val.map(item => item.acceleration),
        }],
      })
    },
  },
}
</script>

<style scoped lang="less">
  .acceleration-chart {
    box-sizing: border-box;
    width: 452px;
    height: 210px;
  }
</style>
