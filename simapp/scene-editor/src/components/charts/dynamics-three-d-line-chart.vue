<template>
  <div class="dynamics-3d-line-chart">
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
    zAxis: {
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
          confine: true,
        },
        visualMap: {
          show: false,
          dimension: 2,
          min: 0,
          max: 30,
          inRange: {
            color: ['#43EAFF', '#4543FF'],
          },
        },
        xAxis3D: {
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
            interval: 'auto',
          },
          type: 'value',
        },
        yAxis3D: {
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
            interval: 'auto',
          },
          type: 'value',
        },
        zAxis3D: {
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
            interval: 'auto',
          },
          type: 'value',
        },
        grid3D: {
          viewControl: {
            projection: 'orthographic',
          },
        },
        series: [{
          type: 'surface',
          data: [],
          lineStyle: {
            width: 4,
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
    zAxis: {
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
      const { data: zData } = this.zAxis
      const xLen = xData.length
      const yLen = yData.length
      let minX = Math.min(...xData)
      let maxX = Math.max(...xData)
      let minY = Math.min(...yData)
      let maxY = Math.max(...yData)
      let minZ = Math.min(...zData)
      let maxZ = Math.max(...zData)
      const data = []
      for (let j = 0; j < yLen; j++) {
        for (let i = 0; i < xLen; i++) {
          const z = i + xLen * j
          data.push([xData[i], yData[j], zData[z]])
        }
      }
      if (minX < 0) {
        minX *= 1.1
      } else {
        minX *= 0.9
      }
      if (maxX < 0) {
        maxX *= 0.9
      } else {
        maxX *= 1.1
      }
      if (minY < 0) {
        minY *= 1.1
      } else {
        minY *= 0.9
      }
      if (maxY < 0) {
        maxY *= 0.9
      } else {
        maxY *= 1.1
      }
      if (minZ < 0) {
        minZ *= 1.1
      } else {
        minZ *= 0.9
      }
      if (maxZ < 0) {
        maxZ *= 0.9
      } else {
        maxZ *= 1.1
      }
      this.chartOption.xAxis3D.min = minX.toFixed(3)
      this.chartOption.xAxis3D.max = maxX.toFixed(3)
      this.chartOption.yAxis3D.min = minY.toFixed(3)
      this.chartOption.yAxis3D.max = maxY.toFixed(3)
      this.chartOption.zAxis3D.min = minZ.toFixed(3)
      this.chartOption.zAxis3D.max = maxZ.toFixed(3)
      this.chartOption.series[0].data = data
      this.chartOption.series[0].dataShape = [yLen, xLen]
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
