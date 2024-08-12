<template>
  <div>
    <SimChart
      v-for="(option, index) in options"
      v-show="showChart"
      :key="index"
      :option="option"
      :width="width"
      :height="height"
    />
  </div>
</template>

<script>
import _ from 'lodash'
import SimChart from '@/components/sim-chart.vue'

const safeColor = '#00F9FE'
const redColor = '#FA3424'

const gridSample = {
  top: 40,
  left: 15,
  bottom: 30,
  containLabel: true,
}

const seriesSample = {
  name: '评测数值',
  type: 'line',
  smooth: true,
  showSymbol: false,
  hoverAnimation: false,
  sampling: 'average',
  data: [],
}
const seriesThresholdSample = {
  name: '阈值上下限',
  type: 'line',
  step: 'start',
  showSymbol: false,
  hoverAnimation: false,
  sampling: 'average',
  lineStyle: {
    type: 'dashed',
    width: 1,
  },
  data: [],
}

const xAxisSample = {
  type: 'value',
  splitLine: {
    show: false,
  },
  axisLine: {
    onZero: false,
    lineStyle: {
      width: 2,
      color: '#999',
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
    show: false,
  },
  axisLine: {
    onZero: false,
    lineStyle: {
      width: 2,
      color: '#999',
    },
  },
  nameTextStyle: {
    align: 'left',
  },
}

const visualMapSample = {
  show: false,
  seriesIndex: 0,
  dimension: 0,
  pieces: [],
}

const legendSample = {
  left: 'auto',
  right: '0',
  align: 'right',
  orient: 'vertical',
  textStyle: {
    color: '#999',
  },
  data: [],
}

const legendLineSample = {
  name: '',
  icon: 'path://M0,10L20,10L20,11L0,11z',
}

const legendDashedSample = {
  name: '',
  icon: 'path://M0,10L9,10L9,11L0,11M12,10L20,10L20,11L12,11z',
}

const dataZoomSample = {
  realtime: true,
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
}

const chartOption = {
  color: [safeColor, '#3EAEF6', '#AA5BFD'],
  backgroundColor: '#111111',
  grid: gridSample,
  legend: legendSample,
  tooltip: {
    trigger: 'axis',
    formatter (dataArray) {
      const yText = dataArray.map(data => `${data.marker}${data.seriesName}: ${data.data[1]}`).join('<br>')
      const xText = `${dataArray[0]?.axisValue}${dataArray[0].data[2]}`
      return `${yText}<br>${xText}`
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
      orient: 'horizontal',
      bottom: 15,
      height: 12,
      ...dataZoomSample,
    },
    {
      type: 'slider',
      orient: 'vertical',
      right: 15,
      width: 12,
      ...dataZoomSample,
    },
    {
      type: 'inside',
    },
  ],
  xAxis: xAxisSample,
  yAxis: yAxisSample,
  series: seriesSample,
}

export default {
  name: 'KpiChartNew',
  components: {
    SimChart,
  },
  props: {
    data: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      width: 540,
      height: 240,
    }
  },
  computed: {
    options () {
      if (!this.data) return [chartOption]
      const { data } = this
      const { xAxis, yAxis: yAxisArray } = data

      // 多阈值且超出阈值显示红色，代码示例见下方链接
      // https://echarts.apache.org/examples/zh/editor.html?c=line-smooth&code=PYBwLglsB2AEC8sDeAoWsAeBBDEDOAXMmurGAJ4gCmRA5AMYCGYVA5sAE7m0A0J6eEABsIYADIRoNYqVJ4AFsADuRAGaMheKv1gBfPrIAmzRkQDaABh6wAjNYBM1gMzWALNYCs1gGwBdA6QARsAArtDGXADijCBqGlok-iTkOPhEqLIU1HQAbhohVLSJAUJsVOHpOqWqYHSMIWDAvDocEKzytbC0Fs2yGm3QdK3tYL2knBDlnbQ5VByQTEJj6MZgprBmOuhIsNCMALbStIAOpoA8CrywEPQwdCDM8gQA9I8AslY2FmL275_fPDY2MTvGwAL1oegCsgyslIe0OdEAUHKABTTABBp50hMKuNy6dzAD2eb3-nwAnESxKSAUD_jYXjZHB8vj9GdSxHTqWCthDOdCYbsDkdANByqPRnPQWMGOPuT1eTIpJJZwNp9N-TL-lLZAI5vN0Ol8xRIOXwIQ0Lxi5h0PLkihUsHUmioGIEc0meAAkuEqBgiI5OSBJvQqIQNqLkLB9owvbAABweKywa5CThDKiGcFJXnbMMRogfCxxhNJrqsDhUcppx2kHb7SQ5vMWAB0ebs8eAiY4ydTek5etk6crnIUyji9orWlagfdhk9RBcvv9gfNGdD4cjUbz1gL7a6Jc7fd5VeztjrG9bhdoxdL0HLIarNaPecbFmbm47ae7Oj3Mhhg5tdq0o-dCcPUjdw5yoAMg02JcD0jOl1xbNs6AvMsuWgsM7zghsmxPRDtxTN9eR7UhP0tARrWHf8B0At1gKILwwIgxcMxgog13zU8t1oHdrzQlda3gl8ixLFDPyhdCJVzLCnxws9uK7QiPxIfwSDHF0mPQZ5YGRNELU5VZ1jMSTrCM-840w4zvjM49TOU3k4SObSRV5LIjhEKRllIRhoAgcNIGxP8HQHFhYi6cpUwrQTaAAYicABRLBYoAMW8Dz0DcqgAGUKFKSolxcuhjAUfDOR1XsMU0oUdKhPSTHMGM43q6w2Ka-DmujVq81smF7LoSqnJhfKunS1LYC8nzmCgCUAtHYK6DCkbIqijwABEPAAIUS5aRvSrLyByr9nMoI5CvkYrtQ_crHlgM5AAh_wAAdMASGM0UAMCVACclQAuOUALE1AFbrQAYf8AFDk0UAD7dABS9L6_sAWSNAF9NQA87UABudPrRQAQt0AHIzACcgwBZRMAO39AEh_3TeX0uq_iPJxHFgAB2WMutkHqujOEbBtoYaKzG3zJoo7QyvxmFCY2aAQiEIRrH5wXrEpszc2p2F-ToemK0Z5nOVZib_PiTniIxUjYF5swRaF3YBf1vXjNzCziTMpwnAsKX0Fpk5-syI66EV3llb8qa1Yu7mjFqvnDeF_2DdFoOjcDmxLbjbxOorO25c5BXJEKFnvLZ1X7UU9A9V0ABuIA

      const options = (yAxisArray || []).map((yAxis) => {
        const xAxisName = `${xAxis.name}${xAxis.unit !== 'N/A' ? `(${xAxis.unit})` : ''}`
        const yAxisName = `${yAxis.name}${yAxis.unit !== 'N/A' ? `(${yAxis.unit})` : ''}`
        const normalLegend = yAxis.name
        const { thresholdUpper, thresholdLower } = yAxis

        let xAxisData = xAxis.axisData.slice(0)
        let yAxisData = yAxis.axisData.slice(0)

        if (xAxisData.length < 2) {
          xAxisData = Array.from({ length: 2 }).fill(xAxisData[0])
        }

        if (yAxisData.length < 2) {
          yAxisData = Array.from({ length: xAxisData.length }).fill(yAxisData[0])
        }

        const legendData = [{ ...legendLineSample, name: normalLegend }] // 线条解释
        const thresholdOptions = [] // 阈值上下限线条
        let visualMap // 红蓝区分

        // 阈值上限
        const upperData = this.getThresholdData(thresholdUpper, xAxisData)
        if (upperData) {
          const { desc, legend, data } = upperData
          legendData.push({
            ...legendDashedSample,
            name: legend,
          })
          thresholdOptions.push({
            ...seriesThresholdSample,
            name: desc,
            data,
          })
        }

        // 阈值下限
        const lowerData = this.getThresholdData(thresholdLower, xAxisData)
        if (lowerData) {
          const { desc, legend, data } = lowerData
          legendData.push({
            ...legendDashedSample,
            name: legend,
          })
          thresholdOptions.push({
            ...seriesThresholdSample,
            name: desc,
            data,
          })
        }

        // 红蓝区分
        if (upperData || lowerData) {
          const safes = yAxisData.map((y, index) => {
            const x = xAxisData[index]
            let isSafe = true
            if (upperData) {
              const threshold = upperData.data[index][1]
              const over = this.justifyValueItem(y, threshold, upperData)
              if (over) isSafe = false
            }
            if (lowerData) {
              const threshold = lowerData.data[index][1]
              const over = this.justifyValueItem(y, threshold, lowerData)
              if (over) isSafe = false
            }
            return [isSafe, x, y]
          })
          const pieces = safes.reduce((re, [isSafe, x]) => {
            const lastPiece = re[re.length - 1]
            const color = isSafe ? safeColor : redColor
            if (!lastPiece) {
              return [{ max: x, color }]
            }
            if (lastPiece.color === color) {
              lastPiece.max = x
              return re
            }
            return [...re, { min: lastPiece.max, max: x, color }]
          }, [])
          if (!('min' in pieces[pieces.length - 1])) {
            pieces[pieces.length - 1].min = 0
          }
          if (pieces.length > 2) {
            delete pieces[pieces.length - 1].max
          }
          visualMap = [{
            ...visualMapSample,
            pieces,
          }]
        }

        const xAxisUnit = xAxis.unit && xAxis.unit !== 'N/A' ? xAxis.unit : ''
        const seriesData = yAxisData.map((y, i) => [xAxisData[i], y, xAxisUnit])

        // const allYAxisData = [...yAxisData, ..._.flattenDeep(thresholdOptions.map(e => e.data.map(d => d[1])))]

        const getMinMax = (arr) => {
          const isBool = arr.every(e => e === 0 || e === 1)
          const min = isBool ? 0 : _.min(arr)
          const max = isBool ? 1 : _.max(arr)
          if (min === max) return { min: undefined, max: undefined }
          return { min, max }
        }
        const { min: xMin, max: xMax } = getMinMax(xAxisData)
        // const { min: yMin, max: yMax } = getMinMax(allYAxisData)

        const option = _.merge({}, chartOption, {
          legend: {
            data: legendData,
          },
          xAxis: {
            name: xAxisName,
            min: xMin,
            max: xMax,
          },
          yAxis: {
            name: yAxisName,
            // min: yMin,
            // max: yMax,
          },
          visualMap,
          series: [
            {
              ...seriesSample,
              name: normalLegend,
              data: seriesData,
            },
            ...thresholdOptions,
          ],
        })

        // console.log(yAxisName, option);
        return option
      })

      // console.log(data.name, options)
      return options
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
  methods: {
    getThresholdData (thresholdInfo, xAxisData) {
      if (!thresholdInfo || !thresholdInfo.value.length) return undefined

      if (this.isOverSafeNumber(thresholdInfo.value[0])) return undefined

      const { value, unit, desc } = thresholdInfo
      const thresholdName = `${desc}${unit !== 'N/A' ? `(${unit})` : ''}`
      const thresholdLegend = desc

      // 阈值若仅一个值，表示为横轴直线需补全长度与横轴一致
      let thresholdData = value.slice(0)
      if (value.length === 1) {
        thresholdData = Array.from({ length: xAxisData.length }).fill(value[0])
      }

      // 若阈值某一段没有值，则用上一段的值补全
      thresholdData = thresholdData.map((item, index) => {
        if (this.isOverSafeNumber(item)) {
          const lastItem = thresholdData[index - 1]
          return lastItem
        }
        return item
      })

      thresholdData = thresholdData.map((y, i) => [xAxisData[i], y])

      return {
        ...thresholdInfo,
        legend: thresholdLegend,
        label: thresholdName,
        data: thresholdData,
      }
    },
    // 判断阈值不通过
    justifyValueItem (value, threshold, thresholdData) {
      if (value === undefined) return false
      const { space, type } = thresholdData
      if (space === 'INTERVAL_SPACE_UP') {
        if (type === 'INTERVAL_TYPE_OPEN') return value > threshold
        return value >= threshold
      } else {
        if (type === 'INTERVAL_TYPE_OPEN') return value < threshold
        return value <= threshold
      }
    },
    isOverSafeNumber (value) {
      return value <= -2147483647 || value >= 2147483647
    },
  },
}
</script>

<style scoped>

</style>
