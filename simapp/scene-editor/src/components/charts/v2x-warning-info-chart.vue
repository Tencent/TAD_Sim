<template>
  <TheWindow>
    <template #header>
      <h3 class="v2x-title">
        {{ $t('v2xWarning') }}
      </h3>
    </template>
    <template #body>
      <div class="v2x-body">
        <div>
          <img src="@/assets/images/chart-bar-btns/content-imgs/warning.png" alt="">
        </div>
        <div class="v2x-info">
          <div class="title">
            {{ info.title }}
          </div>
          <div class="content">
            {{ info.content }}
          </div>
        </div>
        <div class="v2x-danger-distance">
          <FromDistance />
          {{ $t('dangerTargetDistance') }}：{{ info.danger_distance }}m
        </div>
        <div class="v2x-arrows">
          <div :class="{ 'arrow-active': isTurnStraight }">
            <ArrowStraight
              type="arrow-straight"
              class="svg-icon"
              :style="isTurnStraight ? handleStyle(info.lightstate) : {}"
            />
            <span v-if="isTurnStraight" class="arrow-value" :style="handleStyle(info.lightstate, true)">{{
              info.endtime
            }}</span>
          </div>
          <div :class="{ 'arrow-active': isTurnRight }">
            <ArrowRight
              type="arrow-right"
              class="svg-icon"
              :style="isTurnRight ? handleStyle(info.lightstate) : {}"
            />
            <span v-if="isTurnRight" class="arrow-value" :style="handleStyle(info.lightstate, true)">{{
              info.endtime
            }}</span>
          </div>
          <div :class="{ 'arrow-active': isTurnLeft }">
            <ArrowLeft
              type="arrow-left"
              class="svg-icon"
              :style="isTurnLeft ? handleStyle(info.lightstate) : {}"
            />
            <span v-if="isTurnLeft" class="arrow-value" :style="handleStyle(info.lightstate, true)">{{
              info.endtime
            }}</span>
          </div>
          <div :class="{ 'arrow-active': isTurn }">
            <ArrowTurn
              type="arrow-trun"
              class="svg-icon"
              :style="isTurn ? handleStyle(info.lightstate) : {}"
            />
            <span v-if="isTurn" class="arrow-value" :style="handleStyle(info.lightstate, true)">{{
              info.endtime
            }}</span>
          </div>
          <div :class="{ 'arrow-active': isTurnCircle }">
            <CircleLight
              type="arrow-circle"
              class="svg-icon"
              :style="isTurnCircle ? handleStyle(info.lightstate) : {}"
            />
            <span
              v-if="isTurnCircle || isTurnNone"
              class="arrow-value"
              :style="handleStyle(info.lightstate, true)"
            >{{ info.endtime }}</span>
          </div>
          <!--     适配宽度用     -->
          <div style="visibility:hidden">
            <CircleLight
              type="arrow-circle"
              class="svg-icon"
            />
            <span class="arrow-value">{{ 122 }}</span>
          </div>
        </div>
      </div>
    </template>
    <template #footer>
      <div class="v2x-footer">
        <div class="item">
          <span class="title">{{ $t('warningType') }}</span>
          <span class="content">{{ info.type }}</span>
        </div>
        <div class="item">
          <span class="title">{{ $t('scenario.signlight') }}</span>
          <span class="content">{{ info.turn.value }}: {{ info.turn.content }}</span>
        </div>
        <div class="item">
          <span class="title">{{ $t('riskLevel') }}</span>
          <span class="content">{{ info.level }}</span>
        </div>
        <div class="item">
          <span class="title">{{ $t('scenario.sighlightStatus') }}</span>
          <span class="content">{{ info.lightstate.value }}: {{ info.lightstate.content }}</span>
        </div>
      </div>
    </template>
  </TheWindow>
</template>

<script>
import { mapState } from 'vuex'
import TheWindow from '@/components/charts/the-window.vue'
import ArrowStraight from '@/assets/images/chart-bar-btns/content-imgs/arrow-straight.svg'
import ArrowRight from '@/assets/images/chart-bar-btns/content-imgs/arrow-right.svg'
import ArrowLeft from '@/assets/images/chart-bar-btns/content-imgs/arrow-left.svg'
import ArrowTurn from '@/assets/images/chart-bar-btns/content-imgs/arrow-turn.svg'
import CircleLight from '@/assets/images/chart-bar-btns/content-imgs/circle-light.svg'
import FromDistance from '@/assets/images/chart-bar-btns/content-imgs/frontDistance.svg'
import eventBus from '@/event-bus'

const lightStateMap = {
  0: { value: 0, content: 'Unavailable', zh: '未知状态', state: 'default' },
  1: { value: 1, content: 'Dark', zh: '信号灯未工作', state: 'default' },
  2: { value: 2, content: 'stop-Then-Proceed', zh: '红闪', state: 'red-flash' },
  3: { value: 3, content: 'stop-And-Remain', zh: '红灯状态', state: 'red' },
  4: { value: 4, content: 'pre-Movement', zh: '绿灯待行状态（红末闪烁）', state: 'red-flash' },
  5: { value: 5, content: 'permissive-Movement-Allowed', zh: '绿灯状态', state: 'green' },
  6: { value: 6, content: 'protected-Movement-Allowed', zh: '受保护相位绿灯（箭头灯）', state: 'green' },
  7: { value: 7, content: 'intersection-clearance', zh: '黄灯状态', state: 'yellow' },
  8: { value: 8, content: 'caution-Conflicting-Traffic', zh: '黄闪', state: 'yellow-flash' },
}

const turnMap = {
  // 默认为圆形灯
  0: { value: 0, content: 'unknownStatus', state: 'circle' },
  1: { value: 1, content: 'maneuverStraightAllowed', state: 'straight' },
  2: { value: 2, content: 'maneuverLeftAllowed', state: 'left' },
  3: { value: 3, content: 'maneuverRightAllowed', state: 'right' },
  4: { value: 4, content: 'maneuverUTurnAllowed', state: 'turn' },
  5: { value: 5, content: 'maneuverLeftTurnOnRedAllowed', state: 'left' },
  6: { value: 6, content: 'maneuverRightTurnOnRedAllowed', state: 'right' },
  7: { value: 7, content: 'maneuverLaneChangeAllowed', state: 'none' },
  8: { value: 8, content: 'maneuverNoStoppingAllowed', state: 'none' },
  9: { value: 9, content: 'yieldAllwaysRequired', state: 'none' },
  10: { value: 10, content: 'goWithHalt', state: 'none' },
  11: { value: 11, content: 'caution', state: 'none' },
  12: { value: 12, content: 'reserved1', state: 'none' },
}

const originInfo = {
  type: '', // 信息类型
  level: '', // 风险等级
  title: '', // 预警标题
  content: '', // 预警内容
  lightstate: '', // 信号灯颜色
  turn: '', // 信号灯类型
  danger_distance: '', // 危险目标距离
  endtime: '', // 倒计时
  timestamp: '',
  age: '', // type、level、title、content 存活时间
}

export default {
  name: 'V2xWarningInfoChart',
  components: { TheWindow, ArrowStraight, ArrowRight, ArrowLeft, ArrowTurn, CircleLight, FromDistance },
  data () {
    return {
      info: {
        ...originInfo,
      },
      unwatch: null,
    }
  },
  computed: {
    ...mapState('scenario', ['timestamp']),
    isTurnStraight () {
      return this.info.turn.state === 'straight'
    },
    isTurnLeft () {
      return this.info.turn.state === 'left'
    },
    isTurnRight () {
      return this.info.turn.state === 'right'
    },
    isTurnCircle () {
      return this.info.turn.state === 'circle'
    },
    isTurn () {
      return this.info.turn.state === 'turn'
    },
    isTurnNone () {
      return this.info.turn.state === 'none'
    },
  },
  mounted () {
    eventBus.$on('v2x-warning-info', this.handleInfo)
  },
  beforeUnmount () {
    Object.assign(this.info, originInfo)
    if (this.unwatch) {
      this.unwatch()
      this.unwatch = null
    }
    eventBus.$off('v2x-warning-info', this.handleInfo)
  },
  methods: {
    handleInfo (payload) {
      const message = payload?.detail || null
      if (message === 'reset') {
        // clearInfo
        Object.assign(this.info, originInfo)
        if (this.unwatch) {
          this.unwatch()
          this.unwatch = null
        }
        return
      }

      const { age, turn, lightstate, endtime } = message
      Object.assign(
        this.info,
        {
          ...message,
          age: [0, '0'].includes(age) ? 3000 : age * 1000, // age 为 0 表示默认值: 3s
          turn: turnMap[turn] || { value: turn, content: '-', state: 'default' },
          lightstate: lightStateMap[lightstate] || { value: lightstate, content: '-', state: 'default' },
        },
      )

      // 处理 age 、endTime
      const lastTimestamp = this.timestamp
      if (this.unwatch) {
        this.unwatch()
        this.unwatch = null
      }

      const originEndtime = endtime
      let handleEndTimeFlag = false
      let handleAgeFlag = false

      this.unwatch = this.$watch('timestamp', (curTimestamp) => {
        // 处理 endTime
        if (this.info.endtime || this.info.endtime === 0) {
          this.info.endtime = originEndtime - Math.floor((curTimestamp - lastTimestamp) / 1000)
          if (this.info.endtime < 0) {
            this.info.endtime = ''
            this.info.lightstate = ''
            this.info.turn = ''
            handleEndTimeFlag = true
          }
        }

        // 处理 age
        if ((curTimestamp - lastTimestamp) > this.info.age) {
          this.info.type = ''
          this.info.level = ''
          this.info.title = ''
          this.info.content = ''
          this.info.danger_distance = ''
          handleAgeFlag = true
        }

        if (handleEndTimeFlag && handleAgeFlag) {
          this.unwatch()
          this.unwatch = null
        }
      })
    },
    handleStyle (lightState, isText = false) {
      if (!lightState) {
        return isText ? { color: '#606266', fontWeight: 700 } : {}
      }

      const { state } = lightState
      if (state === 'default') {
        return isText ? { color: '#606266', fontWeight: 700 } : {}
      }

      const arr = state.split('-')
      if (isText) {
        return {
          color: arr[0],
          fontWeight: 700,
        }
      }
      if (arr.includes('flash')) {
        // 闪光
        return {
          animation: isText ? '' : `${arr[0]}Dazzling 2s linear infinite`,
        }
      } else {
        // 非闪光
        return {
          color: arr[0],
        }
      }
    },
  },
}
</script>

<style scoped lang='less'>
@import "@/assets/less/mixins.less";
.the-window {
  width: 452px;
}

.v2x-title {
  font-size: 14px;
  color: #fff;
}

.v2x-body {
  position: relative;
  display: flex;
  padding: 20px 30px 40px;
  align-items: center;
  margin: auto 0;
  min-height: 60px;

  .v2x-info {
    margin-left: 10px;
    flex: 1;

    .property {
      width: 300px;
      word-break: normal;
      display: block;
      white-space: pre-wrap;
      word-wrap: break-word;
    }

    .title {
      color: #97aac2;
      font-size: 14px;
      .property()
    }

    .content {
      color: #fff;
      font-size: 18px;
      text-shadow: 1px 1px 2px rgba(65, 249, 222, 0.47);
      .property()
    }
  }

  .v2x-danger-distance {
    position: absolute;
    left: 30px;
    bottom: 10px;
    color: white;
  }

  .v2x-arrows {
    position: absolute;
    top: 10px;
    right: 10px;

    .svg-icon {
      transform: scale(1.8) translateY(-1px);
    }

    .arrow-value {
      margin-left: 10px;
    }

    .svg-icon {
      color: #606266;
    }

    //
    //.arrow-active {
    //  .arrow-value {
    //    font-weight: 700;
    //    color: red;
    //  }
    //  .svg-icon {
    //    color: red;
    //  }
    //}

    .arrow-dazzling {
      .svg-icon {
        color: red;
        animation: redDazzling 2s linear infinite;
      }
    }

    //@keyframes dazzling { from { color: #606266; } to { color: red; }  }
  }
}

.v2x-footer {
  display: flex;
  justify-content: space-around;
  font-size: 12px;
  padding: 5px 0;
  flex-wrap: wrap;

  .item {
    line-height: 2em;

    &:nth-child(even) {
      flex-basis: 65%;
    }

    &:nth-child(odd) {
      flex-basis: 35%;
    }

    display: flex;

    .title {
      flex: 0 0 65px;
      color: #97aac2;
      text-align: right;
    }

    .content {
      flex-grow: 1;
      margin-left: 15px;
      color: #fff;
    }
  }
}
</style>

<style>
@keyframes redDazzling {
  0% {
    color: red;
  }
  50% {
    color: #606266;
  }
  100% {
    color: red
  }
}

@keyframes greenDazzling {
  0% {
    color: green;
  }
  50% {
    color: #606266;
  }
  100% {
    color: green
  }
}

@keyframes yellowDazzling {
  0% {
    color: yellow;
  }
  50% {
    color: #606266;
  }
  100% {
    color: yellow
  }
}
</style>
