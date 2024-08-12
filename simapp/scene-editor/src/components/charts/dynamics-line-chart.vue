<template>
  <div class="dynamics-line-chart">
    <SimChart
      :option="chartOption"
      :width="width"
      :height="height"
    />
  </div>
</template>

<script>
import SimChart from '@/components/sim-chart.vue'

export default {
  name: 'DynamicsLineChart',
  components: {
    SimChart,
  },
  props: {
    xAxis: {
      type: Object,
      required: true,
    },
    yAxis: {
      type: Object,
      required: true,
    },
    width: {
      type: Number,
      default: 190,
    },
  },
  data () {
    return {
      chartOption: {
        tooltip: {
          trigger: 'axis',
          confine: true,
          formatter (item) {
            const [data] = item
            return `${data.axisId.replace('\u00000', '')}: ${data.data[0]}<br />${data.seriesName}: ${data.data[1]}`
          },
        },
        grid: {
          top: '10px',
          left: '10px',
          right: '10px',
          bottom: '10px',
          containLabel: true,
        },
        xAxis: {
          axisTick: {
            show: false,
          },
          splitLine: {
            show: true,
            lineStyle: {
              color: '#303030',
              width: 0.5,
            },
          },
          axisLine: {
            show: true,
            lineStyle: {
              color: '#303030',
              width: 1.5,
            },
          },
          axisLabel: {
            color: '#DCDCDC',
          },
        },
        yAxis: {
          axisTick: {
            show: false,
          },
          splitLine: {
            show: true,
            lineStyle: {
              color: '#303030',
              width: 0.5,
            },
          },
          axisLine: {
            show: true,
            lineStyle: {
              color: '#303030',
              width: 1.5,
            },
          },
          axisLabel: {
            color: '#DCDCDC',
          },
        },
        series: [{
          type: 'line',
          showSymbol: false,
          data: [],
          lineStyle: {
            color: {
              type: 'linear',
              x: 0,
              y: 0,
              x2: 1,
              y2: 0,
              colorStops: [{
                offset: 0,
                color: '#43EAFF',
              }, {
                offset: 1,
                color: '#4543FF',
              }],
            },
          },
        }],
      },
    }
  },
  computed: {
    height () {
      return this.width * 190 / 200
    },
  },
  watch: {
    xAxis: {
      handler () {
        this.initData()
      },
      deep: true,
    },
    yAxis: {
      handler () {
        this.initData()
      },
      deep: true,
    },
  },
  created () {
    this.initData()
  },
  methods: {
    initData () {
      const { data: xData } = this.xAxis
      const { data: yData } = this.yAxis
      const data = []
      for (let i = 0; i < xData.length; i++) {
        data.push([xData[i], yData[i]])
      }
      this.chartOption.xAxis.name = this.xAxis.dispName + this.getUnit(this.xAxis)
      this.chartOption.yAxis.name = this.yAxis.dispName + this.getUnit(this.yAxis)
      this.chartOption.series[0].name = this.yAxis.dispName + this.getUnit(this.yAxis)
      this.chartOption.series[0].data = data
    },
    getUnit (axis) {
      const { unit } = axis
      if (unit && unit !== 'null' && unit !== '[]') {
        return `(${unit})`
      }
      return ''
    },
  },
}
</script>
