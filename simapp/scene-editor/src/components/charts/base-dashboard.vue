<template>
  <div :style="{ width: `${width}px`, height: `${height}px` }" class="chart-container">
    <span class="progress"> {{ (progress || progress === 0) ? progress.toFixed(2) : 'N/A' }} </span>
    <div class="title-info-container">
      <div class="title">
        {{ title }}
      </div>
      <div class="unit">
        {{ unit }}
      </div>
    </div>
    <svg
      :width="`${width}px`"
      :height="`${height}px`"
      xmlns="http://www.w3.org/2000/svg"
    >
      <defs>
        <linearGradient id="strokeGradient">
          <stop offset="0%" stop-color="#00F9FE" />
          <stop offset="100%" stop-color="#0295E7" />
        </linearGradient>
      </defs>

      <!--  刻度背景  -->
      <g opacity="0.6">
        <path
          :d="`M${startPoint.x} ${startPoint.y} A ${radius} ${radius}, 0, 1, 0, ${endPoint.x} ${endPoint.y}`"
          :stroke-width="innerDashWidth"
          stroke="#1A4347FF"
          stroke-linecap="butt"
          fill="transparent"
          :transform="transform"
        />
      </g>
      <!--  内圈  -->
      <path
        :d="`M${innerStartPoint.x} ${innerStartPoint.y}
        A ${innerCircleRadius} ${innerCircleRadius}, 0, 1, 0, ${innerEndPoint.x} ${innerEndPoint.y}`"
        :stroke-width="1"
        stroke="#98E1DE"
        stroke-linecap="butt"
        fill="transparent"
        :transform="transform"
      />
      <!--  刻度  -->
      <path :d="tickPath" stroke="#98E1DE" :transform="transform" />
      <!--  外圈  -->
      <path
        :d="`M${outStartPoint.x} ${outStartPoint.y}
        A ${outCircleRadius} ${outCircleRadius}, 0, 1, 0, ${outEndPoint.x} ${outEndPoint.y}`"
        :stroke-width="3"
        stroke="#343436FF"
        stroke-linecap="butt"
        fill="transparent"
        :transform="transform"
      />
      <!--  外圈高亮  -->
      <path
        :d="`
        M${outHighLightStartPoint.x} ${outHighLightStartPoint.y}
        A ${outCircleRadius} ${outCircleRadius}, 0, ${progressLargeArcFlag}, ${progressDirection},
        ${outHighLightEndPoint.x} ${outHighLightEndPoint.y}
        `"
        :stroke-width="3"
        stroke="#4E4E59FF"
        stroke-linecap="butt"
        fill="transparent"
        :transform="transform"
      />
      <!--  进度  -->
      <g opacity="0.6">
        <path
          :d="`M${progressStartPoint.x} ${progressStartPoint.y}
          A ${radius} ${radius}, 0, ${progressLargeArcFlag}, ${progressDirection},
          ${progressEndPoint.x} ${progressEndPoint.y}`"
          :stroke-width="innerDashWidth"
          stroke="url(#strokeGradient)"
          stroke-linecap="butt"
          fill="transparent"
          :transform="transform"
        />
      </g>
      <!--  进度跟随标  -->
      <polygon :points="trianglePoints" :transform="transform" stroke="white" fill="white" />
    </svg>
  </div>
</template>

<script>
export default {
  // 暂时默认为逆时针绘制
  name: 'BaseDashboard',
  props: {
    startAngle: {
      type: Number,
      default: 240,
    },
    endAngle: {
      type: Number,
      default: 300,
    },
    startInMiddle: {
      type: Boolean,
      default: false,
    },
    width: {
      type: Number,
      default: 170,
    },
    height: {
      type: Number,
      default: 170,
    },
    innerDashWidth: {
      type: Number,
      default: 13,
    },
    tickGap: {
      type: Number,
      default: 20,
    },
    progress: {
      type: [Number, String],
      default: 100,
    },
    max: {
      type: Number,
      default: 200,
    },
    title: {
      type: String,
      default: 'Speed',
    },
    unit: {
      type: String,
      default: 'km / h',
    },
  },
  computed: {
    radius () {
      return Math.min(this.width, this.height) / 2 - 20
    },
    transform () {
      // 将坐标轴变换到中心，y 轴反转
      return `translate(${this.width / 2}, ${this.height / 2})scale(1, -1)`
    },
    startPoint () {
      return this.calculateXy(this.startAngle, this.radius)
    },
    endPoint () {
      return this.calculateXy(this.endAngle, this.radius)
    },
    innerCircleRadius () {
      return this.radius - this.innerDashWidth / 2
    },
    innerStartPoint () {
      return this.calculateXy(this.startAngle, this.innerCircleRadius)
    },
    innerEndPoint () {
      return this.calculateXy(this.endAngle, this.innerCircleRadius)
    },
    outCircleRadius () {
      return this.radius + this.innerDashWidth / 2 + 6
    },
    outStartPoint () {
      return this.calculateXy(this.startAngle, this.outCircleRadius)
    },
    outEndPoint () {
      return this.calculateXy(this.endAngle, this.outCircleRadius)
    },
    outHighLightStartPoint () {
      return this.calculateXy(this.progressStartAngle, this.outCircleRadius)
    },
    outHighLightEndPoint () {
      return this.calculateXy(this.progressEndAngle, this.outCircleRadius)
    },
    tickPath () {
      // 暂时默认为逆时针绘制
      let path = ''
      for (let angle = this.startAngle; angle >= this.endAngle - 360; angle -= this.tickGap) {
        const Q = angle < 0 ? angle + 360 : angle
        const { x: x1, y: y1 } = this.calculateXy(Q, this.radius - this.innerDashWidth / 2)
        const tickLength = 10
        const { x: x2, y: y2 } = this.calculateXy(Q, this.radius + tickLength - this.innerDashWidth / 2)
        path += `M${x1} ${y1} L${x2} ${y2} `
      }
      return path
    },
    progressAngle () {
      // return this.startAngle - (this.progress / this.max) * (360 - this.endAngle + this.startAngle);
      if (this.startInMiddle) {
        return 90 - (this.progress / this.max) * (360 - this.endAngle + this.startAngle)
      }
      return this.startAngle - (this.progress / this.max) * (360 - this.endAngle + this.startAngle)
    },
    progressStartAngle () {
      if (this.startInMiddle) {
        return 90
      }
      return this.startAngle
    },
    progressStartPoint () {
      if (this.startInMiddle) {
        return this.calculateXy(this.progressStartAngle, this.radius)
      }
      return this.startPoint
    },
    progressEndAngle () {
      return this.progressAngle < 0 ? this.progressAngle + 360 : this.progressAngle
    },
    progressEndPoint () {
      return this.calculateXy(this.progressEndAngle, this.radius)
    },
    progressLargeArcFlag () {
      if (this.startInMiddle) {
        return 0
      }
      return this.startAngle - this.progressAngle > 180 ? 1 : 0
    },
    progressDirection () {
      return this.progress < 0 ? 1 : 0
    },
    outerRadiusHighLightEndPoint () {
      return this.calculateXy(this.progressAngle < 0 ?
        this.progressAngle + 360 :
        this.progressAngle, this.outCircleRadius)
    },
    trianglePoints () {
      const Q = this.progressAngle < 0 ? this.progressAngle + 360 : this.progressAngle
      const diff1 = 1
      const diff2 = 6
      const r = this.outCircleRadius + 3
      const { x: x1, y: y1 } = this.calculateXy(Q, r)
      const { x: x2, y: y2 } = this.calculateXy(Q + diff1, r + diff2)
      const { x: x3, y: y3 } = this.calculateXy(Q - diff1, r + diff2)
      return `${x1},${y1},${x2},${y2},${x3},${y3}`
    },
  },
  methods: {
    calculateXy (Q, r) {
      if (Q === 0 || Q === 360) {
        return { x: r, y: 0 }
      }
      if (Q === 90) {
        return { x: 0, y: r }
      }
      if (Q === 180) {
        return { x: -r, y: 0 }
      }
      if (Q === 270) {
        return { x: 0, y: -r }
      }

      let ySymbol
      let xSymbol
      if (Q < 90) {
        xSymbol = 1
        ySymbol = 1
      } else if (Q < 180) {
        ySymbol = 1
        xSymbol = -1
      } else if (Q < 270) {
        xSymbol = -1
        ySymbol = -1
      } else {
        xSymbol = 1
        ySymbol = -1
      }

      const tanQ = Math.tan((2 * Math.PI * Q) / 360) // 倾斜角度的正切值
      const y = ySymbol * Math.abs(Math.sqrt(1 / (tanQ * tanQ + 1)) * r * tanQ)
      const x = xSymbol * Math.abs(y / tanQ)
      return { x, y }
    },
  },
}
</script>

<style scoped lang='less'>
.chart-container {
  position: relative;
  color: #fff;
  .progress {
    font-size: 25px;
    position: absolute;
    top: 45%;
    left: 50%;
    transform: translate(-50%,-50%);
    text-shadow: 2px 1px 2px #41F9DE;
  }
  .title-info-container{
    position: absolute;
    bottom: 0;
    left: 50%;
    transform: translate(-50%,-50%);
    text-align: center;
    font-weight: bolder;
    .title {
      font-size: 18px;
      margin-bottom: 5px;
      color: #A3C0D5;
      font-family: PingFang SC,serif;
    }
    .unit {
      font-size: 12px;
      font-family: Arial Narrow,serif;
      font-weight: 700;
      color: #FFFFFF;
    }
  }
}
</style>
