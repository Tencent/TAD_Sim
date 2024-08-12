<template>
  <div class="speed-chart">
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
  name: 'SpeedChart',
  components: {
    SimChart,
  },
  data () {
    return {
      chartOption: {
        title: {
          text: 'Speed/Time',
        },
        tooltip: {
          trigger: 'axis',
          formatter ([{ data: value, axisValue }]) {
            const timeStamp = axisValue * 1000

            const hours = `${Math.floor(timeStamp / 3600000)}`.padStart(2, '0')
            const minutes = `${Math.floor((timeStamp % 3600000) / 60000)}`.padStart(2, '0')
            const seconds = `${Math.floor((timeStamp % 60000) / 1000)}`.padStart(2, '0')
            const milliseconds = `${Math.floor(timeStamp % 1000)}`.padStart(3, '0')

            return `Time stamp: ${hours}:${minutes}:${seconds}.${milliseconds}<br />Speed: ${(value || 0).toFixed(3)}`
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
          data: [],
        },
        yAxis: {
          name: 'm/s',
          type: 'value',
          boundaryGap: [0, '100%'],
          splitLine: {
            lineStyle: {
              color: '#333',
            },
          },
        },
        series: [{
          name: 'Speed',
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
    renderChart (payload) {
      const val = payload?.detail || []
      this.$refs.chart.setOption({
        xAxis: {
          data: val.map(item => `${item.timeStamp / 1000}`),
        },
        series: [{
          data: val.map(item => item.speed),
        }],
      })
    },
  },
}
</script>

<style scoped>

</style>
