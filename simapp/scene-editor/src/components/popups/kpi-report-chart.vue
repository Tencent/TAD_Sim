<template>
  <div class="kpi-report-chart">
    <div v-if="firstSeriesData || secondSeriesData" class="kpi-report-chart-title">
      {{ index }}
    </div>
    <div v-if="firstSeriesData" class="kpi-report-chart-item">
      <div class="kpi-report-chart-title">
        <div class="kpi-report-chart-title-left">
          <template v-if="second">
            {{ $t('scenario.scenario') }} 1
          </template>
          {{ name }} <template v-if="values[0] !== undefined">
            {{ $kpiUnit(values[0], unit, name) }}
          </template>
        </div>
        <div v-if="passes.length > 0" class="kpi-report-chart-title-right">
          <span :class="{ red: passes[0] === false }">
            {{ $kpiPass(passes[0]) }}
          </span>
        </div>
      </div>
      <div v-if="threshold" class="kpi-report-chart-title">
        {{ threshold.text }}{{ $t('indicator.threshold') }} {{ getThresholdValidValue(first) }} {{ threshold.unit }}
      </div>
      <div class="kpi-report-chart-content">
        <SimChart
          ref="kpi-report-chart-01"
          :option="chart.option"
          :width="chart.width"
          :height="chart.height"
          :lazy="true"
        />
        <div class="kpi-report-chart-mask" />
      </div>
    </div>
    <div v-if="secondSeriesData" class="kpi-report-chart-item">
      <div class="kpi-report-chart-title">
        <div class="kpi-report-chart-title-left">
          {{ $t('scenario.scenario') }} 2
          {{ name }} <template v-if="values[1] !== undefined">
            {{ $kpiUnit(values[1], unit, name) }}
          </template>
        </div>
        <div v-if="passes.length > 0" class="kpi-report-chart-title-right">
          <span :class="{ red: passes[1] === false }">
            {{ $kpiPass(passes[1]) }}
          </span>
        </div>
      </div>
      <div v-if="threshold" class="kpi-report-chart-title">
        {{ threshold.text }}{{ $t('indicator.threshold') }} {{ getThresholdValidValue(second) }} {{ threshold.unit }}
      </div>
      <div class="kpi-report-chart-content">
        <SimChart
          ref="kpi-report-chart-02"
          :option="chart.option"
          :width="chart.width"
          :height="chart.height"
          :lazy="true"
        />
        <div class="kpi-report-chart-mask" />
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import SimChart from '@/components/sim-chart.vue'

export default {
  name: 'KpiReportChart',
  components: {
    SimChart,
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
    index: {
      type: [String, Number],
      default: '',
    },
    name: {
      type: String,
      required: true,
    },
    values: {
      type: Array,
      default () {
        return []
      },
    },
    passes: {
      type: Array,
      default () {
        return []
      },
    },
    unit: {
      type: Object,
      default () {
        return {}
      },
    },
    threshold: {
      type: Object,
      default () {
        return null
      },
    },
    x: {
      type: String,
      required: true,
    },
    series: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      chart: {
        width: 600,
        height: 160,
        option: {
          grid: {
            top: 30,
            bottom: 20,
            left: '5%',
            right: '5%',
            containLabel: true,
          },
          tooltip: {
            trigger: 'axis',
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
            type: 'value',
            boundaryGap: [0, '100%'],
            splitLine: {
              lineStyle: {
                color: '#333',
              },
            },
          },
          series: {
            type: 'line',
            showSymbol: false,
            hoverAnimation: false,
            data: [],
          },
        },
      },
    }
  },
  computed: {
    seriesData () {
      const object = {}
      if (this.first) {
        const series01 = _.get(this.first, this.series.key)
        if (this.isArrayEvery(series01, null)) {
          object['01'] = null
        } else {
          object['01'] = series01
        }
      }
      if (this.second) {
        const series02 = _.get(this.second, this.series.key)
        if (this.isArrayEvery(series02, null)) {
          object['02'] = null
        } else {
          object['02'] = series02
        }
      }
      return object
    },
    firstSeriesData () {
      return this.seriesData['01'] || null
    },
    secondSeriesData () {
      return this.seriesData['02'] || null
    },
  },
  watch: {
    first () {
      this.renderChart()
    },
    second () {
      this.renderChart()
    },
  },
  created () {
    this.renderChart()
  },
  methods: {
    isArrayEvery (data, value) {
      if (data && data.length) {
        return _.every(data, (item) => {
          return item === value
        })
      } else {
        return true
      }
    },
    getThresholdValue (data) {
      if (this.threshold) {
        const item = _.get(data, this.threshold.name)
        if (item) {
          return item[0]
        }
      }
      return undefined
    },
    getThresholdValidValue (data) {
      const value = this.getThresholdValue(data)
      return value !== undefined ? value.toFixed(3) : ''
    },
    renderChart () {
      this.$nextTick(() => {
        this.renderChartItem(this.first, '01')
        this.renderChartItem(this.second, '02')
      })
    },
    renderChartItem (data, id) {
      if (data) {
        const seriesData = this.seriesData[id]
        if (seriesData) {
          const { name = '', unit = '' } = this.series
          const xAxisData = _.get(data, this.x)
          const option = {
            xAxis: {
              axisLabel: {
                showMinLabel: true,
                showMaxLabel: true,
                interval: Math.round(xAxisData.length / 14),
              },
              data: xAxisData,
            },
            tooltip: {
              formatter ([{ data, axisValue }]) {
                const text = `${axisValue} s <br/> ${name}`
                let value = data
                if (value === true || value === false) {
                  value = value ? this.$t('yes') : this.$t('no')
                  return `${text} ${value}`
                } else {
                  if (_.isNumber(value)) {
                    return `${text} ${value.toFixed(3)} ${unit}`
                  } else {
                    return `${axisValue} s`
                  }
                }
              },
            },
            series: {
              type: 'line',
              data: seriesData,
            },
          }
          if (this.threshold) {
            const thresholdValue = this.getThresholdValue(data)
            if (thresholdValue !== undefined) {
              option.series.markLine = {
                silent: true,
                symbol: false,
                data: [{
                  yAxis: thresholdValue,
                }],
              }
              const { comparison } = this.threshold
              const min = _.min(seriesData)
              const max = _.max(seriesData)
              const comparisonObject = {}
              if (comparison === 'lt') {
                comparisonObject.gte = min
                comparisonObject.lte = thresholdValue
              } else if (comparison === 'gt') {
                comparisonObject.gte = thresholdValue
                comparisonObject.lte = Math.ceil(max)
              }
              option.visualMap = {
                top: 0,
                right: 40,
                pieces: [{
                  ...comparisonObject,
                  color: '#FA3424',
                }],
                outOfRange: {
                  color: '#1B889C',
                },
              }
            }
          } else {
            option.series.lineStyle = {
              color: '#1B889C',
            }
          }
          if (seriesData[0] === true || seriesData[0] === false) {
            option.yAxis = {
              min: 0,
              max: 1,
              interval: 1,
              axisLabel: {
                formatter (value) {
                  return value ? this.$t('yes') : this.$t('no')
                },
              },
            }
          }
          const el = this.$refs[`kpi-report-chart-${id}`]
          if (el) {
            el.setOption(option)
          }
        } else {
          console.log(`no series data ${this.series.key}`)
        }
      }
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .kpi-report-chart {
    .kpi-report-chart-item {
      margin-top: 5px;
    }

    .kpi-report-chart-title {
      display: flex;
      justify-content: space-between;
      color: #B8B8B8;

      .red {
        color: #fa3424;
      }
    }

    .kpi-report-chart-content {
      position: relative;
    }

    .kpi-report-chart-mask {
      position: absolute;
      right: 0;
      top: 0;
      width: 150px;
      height: 24px;
      background-color: #191919;
    }
  }
</style>
