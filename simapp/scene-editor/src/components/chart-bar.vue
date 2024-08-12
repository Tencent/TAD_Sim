<template>
  <div class="chart-bar">
    <template v-if="!useCustomChart">
      <!-- 速度图表 -->
      <el-tooltip effect="dark" :content="$t('scenario.speed')" placement="left">
        <span>
          <button
            class="chart-btn"
            :class="{ active: speedChartVisible }"
            :disabled="status !== 'playing' || disabled"
            @click="toggleChartsVisible('speedChartVisible')"
          >
            <SpeedSvg />
          </button>
        </span>
      </el-tooltip>
      <!-- 加速度图表 -->
      <el-tooltip effect="dark" :content="$t('acceleration')" placement="left">
        <span>
          <button
            class="chart-btn"
            :class="{ active: accelerationChartVisible }"
            :disabled="status !== 'playing' || disabled"
            @click="toggleChartsVisible('accelerationChartVisible')"
          >
            <AccelerationSvg />
          </button>
        </span>
      </el-tooltip>
      <!-- 碰撞次数图表 -->
      <el-tooltip effect="dark" :content="$t('collision')" placement="left">
        <span>
          <button
            class="chart-btn"
            :class="{ active: collisionChartVisible }"
            :disabled="status !== 'playing' || disabled"
            @click="toggleChartsVisible('collisionChartVisible')"
          >
            <CollisionSvg />
          </button>
        </span>
      </el-tooltip>
    </template>
    <template v-if="useCustomChart">
      <!-- 实时数据图表 -->
      <el-tooltip effect="dark" :content="$t('player.realTimeData')" placement="left">
        <span>
          <button
            class="chart-btn chart-btn-module"
            :class="{ active: customChartVisible }"
            :disabled="status !== 'playing' || disabled"
            @click="toggleChartsVisible('customChartVisible')"
          >
            <RealTimeDataSvg />
          </button>
        </span>
      </el-tooltip>
    </template>
    <!-- 模块列表 -->
    <el-tooltip effect="dark" :content="$t('module.module')" placement="left">
      <span>
        <button
          class="chart-btn chart-btn-module"
          :class="{ active: moduleChartVisible }"
          :disabled="status !== 'playing'"
          @click="toggleChartsVisible('moduleChartVisible')"
        >
          <el-icon class="icon"><Operation /></el-icon>
        </button>
      </span>
    </el-tooltip>
    <!-- 显隐设置 -->
    <el-tooltip effect="dark" :content="$t('operation.viewConfig')" placement="left">
      <span>
        <button
          class="chart-btn chart-btn-module"
          :class="{ active: viewConfigVisible && presentScenario }"
          :disabled="status !== 'playing' || !presentScenario"
          @click="toggleChartsVisible('viewConfigVisible')"
        >
          <i class="el-icon-s-tools" />
        </button>
      </span>
    </el-tooltip>
    <div class="bottom-btns">
      <!-- 车辆仪表盘 -->
      <el-tooltip effect="dark" :content="$t('carDashboard')" placement="left">
        <span>
          <button
            class="chart-btn chart-btn-module"
            :class="{ active: dashboardVisible }"
            :disabled="status !== 'playing'"
            @click="toggleChartsVisible('dashboardVisible')"
          >
            <DashboardSvg />
          </button>
        </span>
      </el-tooltip>
      <!-- V2X预警信息 -->
      <BaseAuth :perm="['router.player.v2x.show']">
        <el-tooltip effect="dark" :content="$t('v2xWarning')" placement="left">
          <span>
            <button
              class="chart-btn chart-btn-module"
              :class="{ active: v2xWarningVisible }"
              :disabled="status !== 'playing'"
              @click="toggleChartsVisible('v2xWarningVisible')"
            >
              <V2xWarningSvg />
            </button>
          </span>
        </el-tooltip>
      </BaseAuth>
    </div>
    <div class="chart-containers">
      <SpeedChart
        v-if="speedChartVisible"
        class="chart-pop speed-chart"
      />
      <AccelerationChart
        v-if="accelerationChartVisible"
        class="chart-pop acceleration-chart"
      />
      <CollisionChart
        v-if="collisionChartVisible"
        class="chart-pop collision-chart"
      />
      <CustomChartWrap
        v-if="customChartVisible"
        class="chart-pop custom-chart-wrap"
      />
      <ModuleChart
        v-if="moduleChartVisible"
        class="chart-pop module-chart"
      />
      <DashboardChart
        v-if="dashboardVisible"
        class="chart-pop dashboard-chart"
      />
      <V2xWarningInfoChart
        v-if="v2xWarningVisible"
        class="chart-pop v2x-warning-info-chart"
        v-bind="$attrs"
      />
      <ViewConfig
        v-if="viewConfigVisible && presentScenario"
        class="chart-pop module-chart"
        v-bind="$attrs"
      />
    </div>
  </div>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import SpeedChart from './charts/speed-chart.vue'
import AccelerationChart from './charts/acceleration-chart.vue'
import CollisionChart from './charts/collision-chart.vue'
import ModuleChart from './charts/module-chart.vue'
import DashboardChart from './charts/dashboard-chart.vue'
import v2xWarningInfoChart from './charts/v2x-warning-info-chart.vue'
import CustomChartWrap from './charts/custom-chart-wrap.vue'
import ViewConfig from './charts/view-config.vue'
import dict from '@/common/dict'

import AccelerationSvg from '@/assets/images/chart-bar-btns/a.svg'
import CollisionSvg from '@/assets/images/chart-bar-btns/c.svg'
import SpeedSvg from '@/assets/images/chart-bar-btns/s.svg'
import V2xWarningSvg from '@/assets/images/chart-bar-btns/v2x-warning.svg'
import DashboardSvg from '@/assets/images/chart-bar-btns/dashboard.svg'
import RealTimeDataSvg from '@/assets/images/chart-bar-btns/real-time-data.svg'

export default {
  name: 'ChartBar',
  components: {
    SpeedChart,
    AccelerationChart,
    CollisionChart,
    ModuleChart,
    DashboardChart,
    V2xWarningInfoChart: v2xWarningInfoChart,
    CustomChartWrap,
    AccelerationSvg,
    CollisionSvg,
    SpeedSvg,
    V2xWarningSvg,
    DashboardSvg,
    RealTimeDataSvg,
    ViewConfig,
  },
  computed: {
    ...mapState('chart-bar', [
      'useCustomChart',
      'speedChartVisible',
      'accelerationChartVisible',
      'collisionChartVisible',
      'moduleChartVisible',
      'dashboardVisible',
      'v2xWarningVisible',
      'customChartVisible',
      'viewConfigVisible',
    ]),
    ...mapState('scenario', [
      'status',
    ]),
    ...mapGetters('scenario', [
      'presentScenario',
      'isPlaying',
    ]),
    ...mapGetters('sim-modules', [
      'modulesActive',
    ]),
    disabled () {
      // 当模块名称为Grading或分类为评测（其中预设模块无分类），则可显示速度等报表
      const gradingCategoryId = dict.moduleCategoryList.find(e => e.name === 'module.Evaluating')?.id
      return !this.modulesActive.some(({ name, category }) => /Grading_?/.test(name) && (!category || category === gradingCategoryId))
    },
  },
  watch: {
    disabled (value) {
      if (value) {
        if (this.speedChartVisible) {
          this.toggleChartsVisible('speedChartVisible')
        }
        if (this.accelerationChartVisible) {
          this.toggleChartsVisible('accelerationChartVisible')
        }
        if (this.collisionChartVisible) {
          this.toggleChartsVisible('collisionChartVisible')
        }
        if (this.customChartVisible) {
          this.toggleChartsVisible('customChartVisible')
        }
      }
    },
  },
  created () {
    this.getModuleSetList()
  },
  methods: {
    ...mapMutations('chart-bar', [
      'toggleChartsVisible',
    ]),
    ...mapActions('module-set', [
      'getModuleSetList',
    ]),
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .chart-bar {
    flex: 0 0 30px;
    margin-right: 1px;
    background-color: @dark-bg;
    position: relative;
    z-index: 3;
    display: flex;
    flex-direction: column;
    align-items: center;
  }

  .chart-btn {
    .reset-button;
    margin-top: 10px;

    &:hover:not(:disabled),
    &.active {
      color: @active-font-color;
    }
    &:not(.active) {
      filter: grayscale(0.9);
    }

    &.chart-btn-module {
      font-size: 22px;
    }
  }

  .chart-containers {
    position: absolute;
    z-index: 2;
    top: 24px;
    right: 31px;
    display: flex;
    flex-direction: column;
    align-items: flex-end;
    max-height: 80vh;
    overflow-x: hidden;
    overflow-y: auto;
  }

  .chart-pop {
    flex-shrink: 0;

    &:not(:first-child) {
      margin-top: 10px;
    }
    background-color: rgba(0, 0, 0, 0.50);
    box-shadow: -2px 3px 4px 0 rgba(0, 0, 0, 0.30);

    :deep(.el-table) {
      background-color: transparent;

      tr {
        background-color: transparent;

        th, td {
          background-color: transparent;
        }
      }
    }
  }

  .bottom-btns {
    position: absolute;
    bottom: 0;
    text-align: center;
  }
</style>
