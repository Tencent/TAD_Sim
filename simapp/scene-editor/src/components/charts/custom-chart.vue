<template>
  <div class="custom-chart">
    <SimChart
      ref="chart"
      :option="chartOption"
      :width="406"
      :height="180"
    />
  </div>
</template>

<script>
import _ from 'lodash'
import { graphic } from 'echarts/lib/echarts'
import SimChart from '@/components/sim-chart.vue'

const chartStyle = {
  color: 'props.color',
  title: {
    padding: [4, 10, 10],
  },
  grid: {
    top: '15%',
    bottom: '18%',
    left: '12%',
    right: '3%',
  },
  tooltip: {
    trigger: 'axis',
    axisPointer: {
      animation: false,
    },
  },
  xAxis: {
    nameLocation: 'center',
    boundaryGap: false,
    nameTextStyle: {
      padding: [8, 0, 0],
    },
    axisLine: {
      lineStyle: {
        color: '#B9B9B9',
      },
    },
    axisTick: {
      lineStyle: {
        color: '#B9B9B9',
      },
    },
    splitLine: {
      show: false,
    },
  },
  yAxis: {
    nameLocation: 'center',
    nameTextStyle: {
      padding: [0, 0, 12],
    },
    axisLine: {
      lineStyle: {
        color: '#B9B9B9',
      },
    },
    axisTick: {
      lineStyle: {
        color: '#B9B9B9',
      },
    },
    boundaryGap: [0, '100%'],
    splitLine: {
      lineStyle: {
        color: '#333',
      },
    },
  },
}
const seriesStyle = {
  showSymbol: false,
  smooth: true,
  animation: false,
  areaStyle: {
    color: 'getAreaLinearGradient(props.color)',
  },
}
function getAreaLinearGradient (color) {
  return new graphic.LinearGradient(0, 0, 0, 1, [
    { offset: 0, color, opacity: 0.3 },
    { offset: 1, color: 'transparent', opacity: 0 },
  ])
}

export default {
  name: 'CustomChart',
  components: {
    SimChart,
  },
  props: {
    index: {
      type: Number,
      default: 0,
    },
    color: {
      type: String,
      default: 'red',
    },
    config: {
      type: Object,
      default: () => ({}),
    },
    data: {
      type: Object,
      default: () => ({ xAxis: [], yAxis: [] }),
    },
  },
  data () {
    return { }
  },
  computed: {
    chartOption () {
      const { index, color, config } = this
      const { field, chart } = config || {}
      const { title, unit, xAxisName, yAxisName } = chart || {}

      const option = _.merge({}, chartStyle, {
        color,
        title: {
          text: `${index}. ${title}`,
        },
        tooltip: {
          formatter (res) {
            const [{ data: value, axisValue }] = res
            const timeStamp = axisValue
            const dotHtml = `<span class="color" style="background-color:${color}"></span>`
            const valueHtml = `${Number((value || 0).toFixed(3))}`
            const unitHtml = unit !== 'N/A' ? `${unit}` : ''
            return `${timeStamp}s<br />${dotHtml} ${yAxisName} ${valueHtml}${unitHtml}`
          },
        },
        xAxis: {
          type: 'category',
          name: xAxisName,
        },
        yAxis: {
          type: 'value',
          name: yAxisName,
        },
        series: [{
          ...seriesStyle,
          name: field,
          type: 'line',
          areaStyle: {
            color: getAreaLinearGradient(color),
          },
        }],
      })

      // console.log(option)
      return option
    },
  },
  watch: {
    data (newData) {
      this.renderChart(newData)
    },
  },
  mounted () {
    this.renderChart(this.data)
  },
  methods: {
    renderChart (data) {
      const $chart = this.$refs.chart
      if (!$chart) return
      const { xAxis, yAxis } = data || {}
      $chart.setOption({
        xAxis: {
          data: xAxis || [],
        },
        series: [{
          data: yAxis || [],
        }],
      })
    },
  },
}
</script>

<style lang="less" scoped>
.custom-chart {
  :deep(.color) {
    display: inline-block;
    width: 6px;
    height: 6px;
    vertical-align: middle;
    border-radius: 50%;
  }
}
</style>
