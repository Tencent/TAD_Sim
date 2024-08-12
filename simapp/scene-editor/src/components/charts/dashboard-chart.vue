<template>
  <TheWindow>
    <template #header>
      <h3 class="dashboard-title">
        {{ $t('carDashboard') }}
      </h3>
    </template>
    <template #body>
      <div class="dashboard-body">
        <div class="charts">
          <div class="speed-chart chart">
            <DashBoardChart title="Speed" unit="km / h" :progress="info.speed" />
          </div>
          <div class="mid chart">
            <div class="turn-corner">
              <LeftTurnLightActive v-if="info.leftTurnSigLampSts" class="turn-arrow" />
              <LeftTurnLight v-else class="turn-arrow" />
              <span class="turning-angle">{{ info.carAngle }}°</span>
              <RightTurnLightActive v-if="info.rightTurnSigLampSts" class="turn-arrow" />
              <RightTurnLight v-else class="turn-arrow" />
            </div>
            <div class="heading-angle">
              <HeadingAngle class="heading-angle-chart" :style="styleObject" />
              <img :src="car" alt="" class="heading-car">
            </div>
            <div class="gear">
              <Curve />
              <div class="gear-container">
                <span
                  v-for="(gear, key) in gearMap"
                  :key="gear"
                  :class="{ 'gear-active': String(info.geadMode) === key }"
                >
                  {{ gear }}
                </span>
              </div>
            </div>
          </div>
          <div class="acc-chart chart">
            <DashBoardChart
              title="Accel"
              unit="m / s²"
              :progress="info.acceleration"
              start-in-middle
              :max="40"
            />
          </div>
        </div>
      </div>
      <div class="bottom">
        <div class="left">
          <div class="front-distance">
            <FrontDistance />
            <span class="front-distance-content">{{ $t('frontDistance') }}: {{ info.distHeadway }} m</span>
          </div>
          <div class="indicator-light">
            <!--     远光       -->
            <MainBeamActive v-if="info.highBeam" class="light-item" />
            <MainBeam v-else class="light-item" />
            <!--     近光       -->
            <DippedBeamActive v-if="info.lowBeam" class="light-item" />
            <DippedBeam v-else class="light-item" />
            <!--     前雾       -->
            <FrontFogLightActive v-if="info.frontFogLamp" class="light-item" />
            <FrontFogLight v-else class="light-item" />
            <!--     后雾       -->
            <RearFogLightActive v-if="info.rearFogLamp" class="light-item" />
            <RearFogLight v-else class="light-item" />
            <!--     示廓       -->
            <WidthLampActive v-if="info.positionLamp" class="light-item" />
            <WidthLamp v-else class="light-item" />
            <!--     双闪       -->
            <DangerousWarningLightActive v-if="info.hazardLight" class="light-item" />
            <DangerousWarningLight v-else class="light-item" />
          </div>
        </div>
        <div class="right">
          <div class="steering-wheel">
            <SteeringWheel />
            <span class="steer-angle">{{ info.steeringWheelAngle }}°</span>
          </div>
          <div class="progress-container">
            <div class="progress-item">
              <span class="progress-label">{{ $t('brake') }}</span>
              <BaseProgress class="progress" :percentage="info.brakePedalPos" />
            </div>
            <div class="progress-item">
              <span class="progress-label">{{ $t('gasPedal') }}</span>
              <BaseProgress class="progress" :percentage="info.accpedalPosition" />
            </div>
          </div>
        </div>
      </div>
    </template>
  </TheWindow>
</template>

<script>
import TheWindow from '@/components/charts/the-window.vue'
import BaseProgress from '@/components/charts/base-progress.vue'
import DashBoardChart from '@/components/charts/base-dashboard.vue'

import LeftTurnLightActive from '@/assets/images/chart-bar-btns/content-imgs/leftTurnLightActive.svg'
import LeftTurnLight from '@/assets/images/chart-bar-btns/content-imgs/leftTurnLight.svg'
import RightTurnLightActive from '@/assets/images/chart-bar-btns/content-imgs/rightTurnLightActive.svg'
import RightTurnLight from '@/assets/images/chart-bar-btns/content-imgs/rightTurnLight.svg'
import HeadingAngle from '@/assets/images/chart-bar-btns/content-imgs/headingAngle.svg'
import car from '@/assets/images/chart-bar-btns/content-imgs/car.png'
import Curve from '@/assets/images/chart-bar-btns/content-imgs/curve.svg'
import FrontDistance from '@/assets/images/chart-bar-btns/content-imgs/frontDistance.svg'
import MainBeamActive from '@/assets/images/chart-bar-btns/content-imgs/mainBeamActive.svg'
import MainBeam from '@/assets/images/chart-bar-btns/content-imgs/mainBeam.svg'
import DippedBeamActive from '@/assets/images/chart-bar-btns/content-imgs/dippedBeamActive.svg'
import DippedBeam from '@/assets/images/chart-bar-btns/content-imgs/dippedBeam.svg'
import FrontFogLightActive from '@/assets/images/chart-bar-btns/content-imgs/frontFogLightActive.svg'
import FrontFogLight from '@/assets/images/chart-bar-btns/content-imgs/frontFogLight.svg'
import RearFogLightActive from '@/assets/images/chart-bar-btns/content-imgs/rearFogLightActive.svg'
import RearFogLight from '@/assets/images/chart-bar-btns/content-imgs/rearFogLight.svg'
import WidthLampActive from '@/assets/images/chart-bar-btns/content-imgs/widthLampActive.svg'
import WidthLamp from '@/assets/images/chart-bar-btns/content-imgs/widthLamp.svg'
import DangerousWarningLightActive from '@/assets/images/chart-bar-btns/content-imgs/dangerousWarningLightActive.svg'
import DangerousWarningLight from '@/assets/images/chart-bar-btns/content-imgs/dangerousWarningLight.svg'
import SteeringWheel from '@/assets/images/chart-bar-btns/content-imgs/steeringWheel.svg'
import eventBus from '@/event-bus'

const originInfo = {
  acceleration: '', // 加速度
  accpedalPosition: '', // 油门板位置
  brakePedalPos: '', // 刹车板位置
  distHeadway: 'N/A', // 距离前车距离
  geadMode: '', // 挡位
  leftTurnSigLampSts: '', // 左转指示灯
  rightTurnSigLampSts: '', // 右转指示灯
  carAngle: 'N/A', // 车辆转向角度
  speed: '', // 速度
  steeringWheelAngle: 'N/A', // 方向盘转向角度
  frontFogLamp: '', // 前雾灯
  rearFogLamp: '', // 后雾灯
  hazardLight: '', // 双闪灯
  highBeam: '', // 远光灯
  lowBeam: '', // 近光灯
  positionLamp: '', // 示廓灯
}

export default {
  name: 'DashboardChart',
  components: {
    DashBoardChart,
    TheWindow,
    LeftTurnLightActive,
    LeftTurnLight,
    RightTurnLightActive,
    RightTurnLight,
    HeadingAngle,
    Curve,
    FrontDistance,
    MainBeamActive,
    MainBeam,
    DippedBeamActive,
    DippedBeam,
    RearFogLightActive,
    RearFogLight,
    FrontFogLightActive,
    FrontFogLight,
    WidthLampActive,
    WidthLamp,
    DangerousWarningLightActive,
    DangerousWarningLight,
    SteeringWheel,
    BaseProgress,
  },
  data () {
    return {
      car,
      gearMap: Object.freeze({
        1: 'P',
        2: 'R',
        3: 'N',
        4: 'D',
      }),
      speed: 120,
      acc: 8,
      info: { ...originInfo },
    }
  },
  computed: {
    styleObject () {
      return {
        transform: `rotateX(45deg)scale(1.4)rotate(${Number((this.info.carAngle || 0)) - 90}deg)`,
      }
    },
  },
  mounted () {
    eventBus.$on('car-dashboard', this.handleInfo)
  },
  beforeUnmount () {
    Object.assign(this.info, originInfo)
    eventBus.$off('car-dashboard', this.handleInfo)
  },
  methods: {
    handleInfo (payload) {
      const info = payload?.detail || {}
      if (info === 'reset') {
        Object.assign(this.info, originInfo)
        return
      }

      let acceleration = info.acceleration ? +this.molding(info.acceleration) : 0
      if (info.acceleration?.x < 0) {
        acceleration *= -1
      }

      Object.assign(this.info, {
        ...info,
        carAngle: info.rpy ? (info.rpy.z * 180 / Math.PI).toFixed(2) : 'N/A', // 车辆转向角度
        steeringWheelAngle: (info.steeringWheelAngle || info.steeringWheelAngle === 0) ? info.steeringWheelAngle.toFixed(1) : 'N/A',
        acceleration,
        distHeadway: (info.distHeadway || info.distHeadway === 0) ? info.distHeadway.toFixed(2) : 'N/A',
        speed: info.speed ? this.molding(info.speed) * 3.6 : '',
      })
    },
    molding (item) {
      return Math.sqrt(Number(Object.values(item).reduce((acc, cur) => {
        return acc + Number(cur) ** 2
      }, 0)).toFixed(2))
    },
  },
}
</script>

<style scoped lang='less'>
@import "@/assets/less/mixins.less";

.the-window {
  width: 452px;
  height: 328px;
}

:deep(.the-window-body) {
  height: 100%;
  justify-content: center;
}

.dashboard-title {
  font-size: 12px;
  color: #fff;
}

.dashboard-body {
  display: flex;
  align-items: center;
  justify-content: center;

  .charts {
    display: flex;
    transform: scale(0.8, 0.8);

    .chart {
      flex: 1 1 33%;
      margin: auto;
      display: flex;
      justify-content: center;
    }

    .mid {
      flex-direction: column;
      align-items: center;

      .turn-corner {
        display: flex;
        align-items: center;
        .turning-angle {
          display: inline-block;
          margin: 10px 20px 15px 20px;
          transform: translateY(20px);
          color: #fff;
          font-size: 21px;
          text-shadow: 2px 1px 2px rgba(65, 249, 222, 0.47);
        }

        .turn-arrow {
          scale: 1.5;
        }
      }

      .heading-angle {
        position: relative;
        margin-top: 10px;

        .heading-angle-chart {
          transform-style: preserve-3d;
          perspective: 800px;
          //position: relative;
          transform-origin: center;
          transform: rotateX(45deg) scale(1.4);
        }

        .heading-car {
          position: absolute;
          left: 50%;
          top: 50%;
          transform: translate(-50%, -50%)
        }
      }

      .gear {
        color: gray;

        .gear-container {
          text-align: center;
        }

        .gear-active {
          color: @active-font-color;
        }
      }
    }

  }
}

.bottom {
  padding: 10px 20px 10px;
  display: flex;
  justify-content: space-between;

  .left {
    display: flex;
    flex-direction: column;
    justify-content: space-around;

    .front-distance {
      color: #fff;

      .front-distance-content {
        margin-left: 5px;
      }
    }

    .indicator-light {
      margin-top: 5px;

      .light-item {
        &:not(:first-child) {
          margin-left: 9px;
        }
      }
    }
  }

  .right {
    display: flex;
    color: #fff;

    .steering-wheel {
      position: relative;
      text-align: center;
      transform: scale(0.9);
      // todo: delete?
      img {
        width: 52px;
        height: 53px;
      }

      .steer-angle {
        position: absolute;
        left: 53%;
        top: 47%;
        transform: translate(-50%, -50%);
      }
    }

    .progress-container {
      display: flex;
      flex-direction: column;
      justify-content: space-around;
      margin-left: 10px;

      .progress-item {
        display: flex;
        align-items: center;

        .progress-label {
        }

        .progress {
          margin-left: 10px;
          width: 120px;
        }
      }
    }
  }

  color: #343436;
}
</style>
