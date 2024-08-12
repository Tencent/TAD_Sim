<template>
  <div ref="wrap" class="wrap" :style="{ width: `${width}px`, height: `${height}px` }" />
</template>

<script>
import 'echarts/lib/chart/bar'
import 'echarts/lib/chart/pie'
import 'echarts/lib/chart/line'
import 'echarts/lib/component/tooltip'
import 'echarts/lib/component/title'
import 'echarts/lib/component/legend'
import 'echarts/lib/component/markLine'
import 'echarts/lib/component/visualMap'
import 'echarts/lib/component/dataZoom'
import 'echarts-gl'
import echarts from 'echarts/lib/echarts'
import { throttle } from 'lodash-es'
import mixins from '@/assets/less/mixins.less?inline'

export default {
  name: 'SimChart',
  inject: {
    autoPrint: {
      from: 'autoPrint',
      default: false,
    },
  },
  props: {
    option: {
      type: Object,
      required: true,
    },
    width: {
      type: Number,
      default: 800,
    },
    height: {
      type: Number,
      default: 600,
    },
    // 设置此属性在option更新时需手动调用refreshChart方法，但可有效防止图表多次重渲染的抖动现象
    lazy: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      timer: null,
      throttleId: null,
    }
  },
  watch: {
    width () {
      this.throttleId()
    },
  },
  mounted () {
    this.chart = echarts.init(this.$refs.wrap)
    this.chart.setOption({
      animation: !this.autoPrint,
      title: {
        textStyle: {
          color: mixins['title-font-color'],
          fontSize: 12,
        },
      },
      textStyle: {
        color: mixins['title-font-color'],
      },
    })
    this.chart.setOption(this.option)
    this.throttleId = throttle(() => {
      this.chart.resize()
    }, 20, {
      leading: false,
      trailing: true,
    })
    if (!this.lazy) {
      this.unwatch = this.$watch('option', (newVal) => {
        this.chart.setOption(newVal)
      }, {
        deep: true,
      })
    }
  },
  beforeUnmount () {
    if (this.unwatch) {
      this.unwatch()
    }
    const canvasArr = this.$refs.wrap.getElementsByTagName('canvas')
    Array.prototype.forEach.call(canvasArr, (canvas) => {
      const webglCxt = canvas.getContext('webgl')
      if (webglCxt) {
        webglCxt.getExtension('WEBGL_lose_context').loseContext()
      }
    })
    this.chart.dispose()
    clearTimeout(this.timer)
  },
  methods: {
    refreshChart () {
      this.chart.setOption(this.option)
    },
    setOption (option) {
      this.chart.setOption(option)
    },
  },
}
</script>

<style scoped>

</style>
