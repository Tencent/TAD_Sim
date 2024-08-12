<template>
  <SimChart
    v-show="showChart"
    :option="option"
    :width="width"
    :height="height"
  />
</template>

<script>
import _ from 'lodash'
import SimChart from '@/components/sim-chart.vue'
import { isZeroAndOne, timeAxisLabelFormatter } from '@/common/utils'

const gridHeight = 180

const yAxisGap = 36

const gridSample = {
  top: 10,
  left: 'center',
  height: gridHeight,
}

const seriesSample = {
  type: 'line',
  showSymbol: false,
  hoverAnimation: false,
  data: [],
}

const xAxisSample = {
  type: 'category',
  splitLine: {
    show: false,
  },
  axisLine: {
    onZero: false,
    lineStyle: {
      width: 2,
      color: '#898989',
    },
  },
  boundaryGap: false,
  nameGap: 10,
  data: [],
}

const yAxisSample = {
  type: 'value',
  boundaryGap: [0, '100%'],
  splitLine: {
    lineStyle: {
      color: '#333',
    },
  },
  axisLine: {
    onZero: false,
    lineStyle: {
      width: 2,
      color: '#898989',
    },
  },
}

const chartOption = {
  color: ['#00F9FE', '#3EAEF6', '#AA5BFD', '#3D99AA', '#5D5BFD'],
  grid: gridSample,
  tooltip: {
    trigger: 'axis',
    formatter (dataArray) {
      return dataArray.map(data => `${data.marker}${data.seriesName}: ${data.data}`).join('<br>')
    },
    axisPointer: {
      type: 'cross',
      animation: false,
      label: {
        backgroundColor: 'rgba(50, 50, 50, 0.7)',
        borderWidth: 0,
        shadowBlur: 0,
      },
      link: { xAxisIndex: 'all', yAxisIndex: 'all' },
    },
  },
  dataZoom: [
    {
      type: 'slider',
      show: true,
      realtime: true,
      // start: 0,
      // end: 100,
      top: 215,
      bottom: 5,
      labelFormatter: value => value / 100,
      backgroundColor: '#0f0f0f',
      borderColor: 'rgba(0,0,0,0)',
      fillerColor: 'rgba(255,255,255,.1)',
      handleStyle: {
        color: '#303030',
        borderColor: '#535353',
      },
      textStyle: {
        color: '#dfdfdf',
      },
      dataBackground: {
        lineStyle: {
          color: 'rgba(220,222,226,0.5)',
          width: 1,
          opacity: 1,
        },
      },
    },
  ],
  xAxis: xAxisSample,
  yAxis: yAxisSample,
  series: seriesSample,
}

export default {
  name: 'KpiChart',
  components: {
    SimChart,
  },
  props: {
    data: {
      type: Object,
      required: true,
    },
    thresh: {
      type: Number,
      default: null,
    },
  },
  data () {
    return {
      width: 540,
      height: 240,
    }
  },
  computed: {
    option () {
      if (!this.data) return chartOption
      const {
        thresh,
        data: {
          xAxis: {
            name: xAxisName,
            unit: xAxisUnit,
            axisData: axisDataX,
          },
          yAxis: yAxisArray,
        },
      } = this
      let markLine = null
      if (_.isNumber(thresh)) {
        markLine = {
          silent: true,
          symbol: false,
          lineStyle: {
            type: 'dotted',
          },
          data: [{
            yAxis: thresh,
          }],
        }
      }
      let min = 0
      let max = 0
      const yAxis = []
      const series = yAxis.map(({ axisData }) => {
        const _min = _.min(axisData)
        min = Math.min(min, _min)
        const _max = _.max(axisData)
        max = Math.max(max, _max)

        return {
          ...seriesSample,
          markLine,
          data: axisData,
        }
      })

      yAxisArray.forEach((yAxisItem, yAxisIndex) => {
        const {
          axisData: yAxisData,
          name: yName,
          unit: yUnit,
        } = yAxisItem
        let yNameStr = yName
        if (yUnit !== 'N/A') {
          yNameStr += `(${yUnit})`
        }
        const isBoolData = isZeroAndOne(yAxisData)

        const minMax = isBoolData ? { min: 0, max: 2, interval: 1 } : null

        yAxis.push(_.merge({}, yAxisSample, {
          name: yNameStr,
          type: 'value',
          position: 'left',
          offset: yAxisGap * yAxisIndex,
          axisLabel: {
            showMinLabel: false,
          },
          // nameLocation: 'start',
          // nameRotate: 60,
          // nameGap: 0,
          // nameTextStyle: {
          //   fontSize: 9,
          //   align: 'right',
          // },
        }, minMax))
        const _min = _.min(yAxisData)
        min = Math.min(min, _min)
        const _max = _.max(yAxisData)
        max = Math.max(max, _max)

        series.push(_.merge({}, seriesSample, {
          name: yNameStr,
          markLine,
          data: yAxisData,
          yAxisIndex,
        }))
      })

      let visualMap = null
      // const comparisonObject = {}

      const comparison = 'gt' // todo: 替换成传进来的数据
      if (markLine) {
        const pieces = []
        if (comparison === 'lt') {
          pieces.push({
            lt: thresh,
            gt: Math.min(min, thresh) - 1,
            color: '#FA3424',
          })
          pieces.push({
            gte: thresh,
            lt: Math.max(max, thresh) + 1,
            color: '#00F9FE',
          })
        } else if (comparison === 'gt') {
          pieces.push({
            gt: thresh,
            lte: Math.max(max, thresh) + 1,
            color: '#FA3424',
          })
          pieces.push({
            lte: thresh,
            gt: Math.min(min, thresh) - 1,
            color: '#00F9FE',
          })
        }
        visualMap = {
          show: false,
          pieces,
        }
      }

      return _.merge({}, chartOption, {
        visualMap,
        grid: {
          left: yAxis.length * yAxisGap,
        },
        xAxis: {
          name: `${xAxisName}(${xAxisUnit})`,
          data: axisDataX,
          axisLabel: {
            formatter: timeAxisLabelFormatter,
          },
        },
        yAxis,
        series,
      })
    },
    showChart () {
      const {
        data: {
          xAxis: {
            axisData: axisDataX,
          },
          yAxis: yAxisArray,
        },
      } = this
      return axisDataX && axisDataX.length && yAxisArray && yAxisArray.length
    },
  },
}
</script>

<style scoped>

</style>
