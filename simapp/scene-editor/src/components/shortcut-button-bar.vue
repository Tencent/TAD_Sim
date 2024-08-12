<template>
  <div class="shortcut-button-bar">
    <div class="commonly-used-buttons">
      <button
        id="shortcut-open"
        class="button-item"
        :disabled="!isModifying"
        :title="$t('menu.open')"
        @click="open"
      >
        <span class="button-icon">
          <open-svg style="font-size: 24px;" />
        </span>
        <span class="button-text">{{ $t('menu.open') }}</span>
      </button>
      <button
        id="shortcut-create"
        class="button-item"
        :disabled="!isModifying"
        :title="$t('menu.new')"
        @click="add"
      >
        <span class="button-icon">
          <new-svg />
        </span>
        <span class="button-text">{{ $t('menu.new') }}</span>
      </button>
      <button
        id="shortcut-save"
        class="button-item"
        :disabled="!isModifying || !isDirty"
        :title="$t('menu.save')"
        @click="handleSave"
      >
        <span class="button-icon">
          <save-svg />
        </span>
        <span class="button-text">{{ $t('menu.save') }}</span>
      </button>
      <div class="button-split" />
      <button
        id="shortcut-planner-config"
        class="button-item"
        :disabled="!isModifying"
        :title="$t('menu.plannerConfig')"
        @click="plannerConfig"
      >
        <span class="button-icon">
          <planner-config-svg />
        </span>
        <span class="button-text">{{ $t('menu.plannerConfig') }}</span>
      </button>
      <button
        v-show="false"
        id="shortcut-traffic"
        class="button-item"
        :disabled="!isModifying || !presentScenario"
        :title="$t('menu.trafficFlow')"
        @click="trafficConfig"
      >
        <span class="button-icon">
          <traffic-svg />
        </span>
        <span class="button-text">{{ $t('menu.trafficFlow') }}</span>
      </button>
      <!-- 导入交通灯 -->
      <BaseAuth :perm="['router.importTrafficLight.show']">
        <BaseAuth :perm="['router.importTrafficLight.disabled', 'disabled']">
          <template #default="{ disabled }">
            <button
              id="shortcut-signlight"
              class="button-item"
              :disabled="disabled || (!isModifying || !presentScenario)"
              :title="$t('menu.importTrafficLights')"
              @click="importScenario"
            >
              <span class="button-icon">
                <i class="el-icon" style="font-size:22px;"><Bottom /></i>
              </span>
              <span class="button-text">{{ $t('menu.importTrafficLights') }}</span>
            </button>
          </template>
        </BaseAuth>
      </BaseAuth>
      <!-- 信控配置 -->
      <BaseAuth :perm="['router.signlightControl.show']">
        <button
          id="shortcut-signlight"
          class="button-item"
          :disabled="!isModifying || !presentScenario"
          :title="$t('menu.signalControlConfiguration')"
          @click="signlight"
        >
          <span class="button-icon">
            <SignlightSvg />
          </span>
          <span class="button-text">{{ $t('menu.signalControlConfiguration') }}</span>
        </button>
      </BaseAuth>
      <!-- 状态机 -->
      <BaseAuth :perm="['router.stateMachine.show']">
        <button
          id="shortcut-finite-state-machine"
          class="button-item"
          :disabled="!isModifying || !presentScenario"
          :title="$t('menu.stateMachine')"
          @click="finiteStateMachine"
        >
          <span class="button-icon">
            <watch-over-svg />
          </span>
          <span class="button-text">{{ $t('menu.stateMachine') }}</span>
        </button>
      </BaseAuth>
      <!-- 传感器 -->
      <BaseAuth :perm="['router.sensors.show']">
        <!-- 该版本旧传感器有问题，先不显示 -->
        <!-- <button
          id="shortcut-sensor"
          class="button-item"
          :disabled="!isModifying || !presentScenario"
          :title="sensorGlobalMode ? $t('tips.globalSensorConf') : $t('menu.sensor')"
          @click="showSensor"
        >
          <span class="button-icon">
            <sensor-svg />
            <span v-if="sensorGlobalMode" class="global-mark">G</span>
          </span>
          <span class="button-text">{{ $t('menu.sensor') }}</span>
        </button> -->
      </BaseAuth>
      <button
        id="shortcut-environment"
        class="button-item"
        :disabled="!isModifying || !presentScenario"
        :title="environmentGlobalMode ? $t('tips.globalEnvironmentConf') : $t('menu.environment')"
        @click="showEnvironment"
      >
        <span class="button-icon">
          <environment-svg />
          <span v-if="environmentGlobalMode" class="global-mark">G</span>
        </span>
        <span class="button-text">{{ $t('menu.environment') }}</span>
      </button>
      <BaseAuth :perm="['router.sceneGeneration.show']">
        <button
          id="shortcut-scenario-generation"
          class="button-item"
          :disabled="isDirty || !isModifying || !presentScenario"
          :title="$t('menu.sceneGeneration')"
          @click="scenarioGeneration"
        >
          <span class="button-icon">
            <scenario-generator-svg />
          </span>
          <span class="button-text">{{ $t('menu.sceneGeneration') }}</span>
        </button>
      </BaseAuth>
      <div class="button-split" />
      <button
        id="shortcut-ruler-tool"
        class="button-item"
        :disabled="!isModifying || !presentScenario"
        :title="$t('menu.measure')"
        @click="distanceMeasurement"
        @keydown.enter.prevent
      >
        <span class="button-icon">
          <ruler-svg />
        </span>
        <span class="button-text">{{ $t('menu.measure') }}</span>
      </button>
      <div v-if="plannerList.length > 1" class="button-wrap">
        <el-dropdown trigger="click" placement="bottom-start" @command="backToPlanner($event)">
          <button
            id="shortcut-back-to-planner"
            class="button-item"
            :title="$t('menu.switchPlanner')"
          >
            <span class="button-icon">
              <back-to-main-svg />
              <triangle-down />
            </span>
            <span class="button-text">{{ $t('menu.switchPlanner') }}</span>
          </button>
          <template #dropdown>
            <el-dropdown-menu class="perspective-dropdown-menu">
              <el-dropdown-item
                v-for="p of plannerList"
                :id="`shortcut-back-to-planner-${p.id}`"
                :key="p.key"
                :command="p.id"
                :title="$itemName(p)"
              >
                <component :is="p.icon" class="perspective-dropdown-menu-item-icon" />
                <span class="perspective-dropdown-item-text">{{ $itemName(p) }}</span>
              </el-dropdown-item>
            </el-dropdown-menu>
          </template>
        </el-dropdown>
      </div>
      <div v-else-if="isPlaying" class="button-wrap">
        <el-dropdown trigger="click" placement="bottom-start" @command="backToPlanner($event)">
          <button
            id="shortcut-back-to-planner"
            class="button-item"
            :title="$t('menu.switchPlanner')"
          >
            <span class="button-icon">
              <back-to-main-svg />
              <triangle-down />
            </span>
            <span class="button-text">{{ $t('menu.switchPlanner') }}</span>
          </button>
          <template #dropdown>
            <el-dropdown-menu class="perspective-dropdown-menu">
              <el-dropdown-item
                v-for="p of plannerList"
                :id="`shortcut-back-to-planner-${p.id}`"
                :key="p.key"
                :command="p.id"
                :title="$itemName(p)"
              >
                <component :is="p.icon" class="perspective-dropdown-menu-item-icon" />
                <span class="perspective-dropdown-item-text">{{ $itemName(p) }}</span>
              </el-dropdown-item>
            </el-dropdown-menu>
          </template>
        </el-dropdown>
      </div>
      <button
        v-else
        id="shortcut-back-to-planner"
        class="button-item"
        :disabled="plannerList.length < 1"
        :title="$t('menu.backToPlanner')"
        @click="backToPlanner()"
      >
        <span class="button-icon">
          <back-to-main-svg />
        </span>
        <span class="button-text">{{ $t('menu.backToPlanner') }}</span>
      </button>
      <div class="button-wrap">
        <el-dropdown trigger="click" placement="bottom-start" @command="switchPerspective1">
          <button
            id="shortcut-perspective"
            class="button-item"
            :disabled="false"
            :title="$t(currentPerspectiveItem.label)"
          >
            <span class="button-icon">
              <component :is="currentPerspectiveItem.icon" />
              <triangle-down />
            </span>
            <span class="button-text">{{ $t(currentPerspectiveItem.label) }}</span>
          </button>
          <template #dropdown>
            <el-dropdown-menu class="perspective-dropdown-menu">
              <el-dropdown-item
                v-for="p of validPerspectives"
                :id="`shortcut-perspective-${p.key}`"
                :key="p.key"
                :command="p.key"
                :disabled="!currentCameraSupported.includes(p.key)"
                :title="$t(p.label)"
              >
                <component :is="p.icon" class="perspective-dropdown-menu-item-icon" />
                <span class="perspective-dropdown-item-text">{{ $t(p.label) }}</span>
              </el-dropdown-item>
            </el-dropdown-menu>
          </template>
        </el-dropdown>
      </div>
      <div class="zoom-button" :class="{ disabled: !viewControlEnabled }">
        <div class="button-icon">
          <button id="shortcut-zoom-in" class="zoom-icon" :disabled="!viewControlEnabled" @mouseup="zoomIn">
            <resize-narrow-svg />
          </button>
          <button id="shortcut-zoom-out" class="zoom-icon" :disabled="!viewControlEnabled" @mouseup="zoomOut">
            <resize-zoom-svg />
          </button>
        </div>
        <span class="button-text">{{ factorStr }}</span>
      </div>
    </div>
    <div class="player-editor-buttons">
      <BaseAuth :perm="['router.mapEditor.show']">
        <button
          id="shortcut-map-edit"
          class="button-item"
          :disabled="batchPlaying"
          :title="$t('menu.mapEditing')"
          @click="openMapEditor"
        >
          <span class="button-icon">
            <map-editor-svg />
          </span>
          <span class="button-text">{{ $t('menu.mapEditing') }}</span>
        </button>
      </BaseAuth>
      <button
        id="shortcut-play"
        class="button-item"
        :class="{ 'button-active': isPlaying }"
        :disabled="batchPlaying"
        :title="$t('menu.play')"
        @click="switchStatus('playing')"
      >
        <span class="button-icon">
          <play-svg />
        </span>
        <span class="button-text">{{ $t('menu.play') }}</span>
      </button>
      <button
        id="shortcut-scenario-edit"
        class="button-item"
        :class="{ 'button-active': isModifying }"
        :disabled="batchPlaying"
        :title="$t('menu.sceneEditing')"
        @click="switchStatus('modifying')"
      >
        <span class="button-icon">
          <scenario-editor-svg />
        </span>
        <span class="button-text">{{ $t('menu.sceneEditing') }}</span>
      </button>
      <div class="button-split" />
    </div>
  </div>
</template>

<script>
import { get, groupBy } from 'lodash-es'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { editor } from '@/api/interface'

import TopPerspectiveSvg from '@/assets/images/shortcut-btns/perspective/top.svg'
// import RightPerspectiveSvg from '@/assets/images/shortcut-btns/perspective/right.svg'
// import LeftPerspectiveSvg from '@/assets/images/shortcut-btns/perspective/left.svg'
import PerspectivePerspectiveSvg from '@/assets/images/shortcut-btns/perspective/perspective.svg'
import DriverPerspectiveSvg from '@/assets/images/shortcut-btns/perspective/driver.svg'
import FollowCarPerspectiveSvg from '@/assets/images/shortcut-btns/perspective/followCar.svg'

const { electron } = window

export default {
  name: 'ShortcutButtonBar',
  components: {
    TopPerspectiveSvg,
    PerspectivePerspectiveSvg,
    DriverPerspectiveSvg,
    FollowCarPerspectiveSvg,
  },
  data () {
    return {
      perspectives: [
        {
          label: 'views.top',
          key: 'top',
          icon: 'TopPerspectiveSvg',
        },
        {
          label: 'views.driver',
          key: 'fps',
          icon: 'DriverPerspectiveSvg',
        },
        {
          label: 'views.car',
          key: 'third',
          icon: 'FollowCarPerspectiveSvg',
        },
        {
          label: 'views.free',
          key: 'debugging',
          icon: 'PerspectivePerspectiveSvg',
        },
      ],

      perspectivesModify: [
        {
          label: 'views.top',
          key: 'top',
          icon: 'TopPerspectiveSvg',
        },
        {
          label: 'views.perspective',
          key: 'debugging',
          icon: 'PerspectivePerspectiveSvg',
        },
      ],

      currentPlanId: '',
      planList: [],
    }
  },
  computed: {
    ...mapState('scenario', [
      'isDirty',
      'currentPerspective',
      'currentPerspectivePlaying',
      'supportedPerspectives',
      'factor',
    ]),
    ...mapState('environment', {
      environmentGlobalMode: 'globalMode',
    }),
    ...mapGetters('scenario', [
      'isModifying',
      'isPlaying',
      'tree',
      'presentScenario',
      'batchPlaying',
      'currentCameraSupported',
    ]),
    ...mapState('signalControl', [
      'signlightPlanUpdateFlag',
    ]),
    viewControlEnabled () {
      return this.isModifying || this.isPlaying
    },

    currentPerspectiveValue () {
      return this.isModifying ? this.currentPerspective : this.currentPerspectivePlaying
    },

    currentPerspectiveItem () {
      return this.validPerspectives.find(p => p.key === this.currentPerspectiveValue)
    },
    plannerList () {
      return get(this, 'tree.planner') || []
    },
    factorStr () {
      return `${(100 / this.factor).toFixed(1)}%`
    },

    validPerspectives () {
      if (this.isModifying) {
        return this.perspectivesModify
      } else {
        return this.perspectives
      }
    },
  },
  watch: {
    signlightPlanUpdateFlag () {
      this.updatePlanList()
    },
  },
  created () {
    this.getGlobalConfig()
  },
  methods: {
    ...mapMutations('scenario', [
      'updateState',
    ]),
    ...mapActions('scenario', [
      'saveScenario',
      'importScenario',
      'switchPerspective',
      'backToPlanner',
      'setEnd',
      'switchStatus',
      'zoomIn',
      'zoomOut',
    ]),
    ...mapMutations('mission', [
      'startMission',
    ]),
    ...mapActions('mission', [
      'openCreateNewScenario',
    ]),
    ...mapActions('environment', [
      'getGlobalConfig',
    ]),
    ...mapActions('signalControl', [
      'resetSignalControl',
    ]),
    openMapEditor () {
      electron.mapEditor.open()
    },
    showEnvironment () {
      this.startMission('Environment')
    },
    open () {
      this.startMission('ScenarioManagement')
    },
    add () {
      this.openCreateNewScenario()
    },
    trafficConfig () {
      this.startMission('TrafficFlowConfig')
    },
    plannerConfig () {
      this.startMission('PlannerConfig')
    },
    signlight () {
      if (this.planList.length < 1) {
        this.$message.warning(this.$t('tips.noSignlightForPlan'))
        return
      }
      this.resetSignalControl()
      this.startMission('SignlightConfig')
    },
    finiteStateMachine () {
      this.startMission('FiniteStateMachine')
    },
    scenarioGeneration () {
      this.startMission('ScenarioGeneration')
    },
    distanceMeasurement () {
      editor.scenario.setDistanceMeasurement()
    },
    // 预设场景保存，走另存为的逻辑
    handleSave () {
      if (!this.presentScenario) return
      if (this.presentScenario.preset === '0') {
        this.saveScenario()
      } else {
        // 预设地图另存为
        this.startMission('SaveAs')
      }
    },
    // 刷新信控配置列表
    updatePlanList () {
      if (!editor?.scenario?.getSignlightPlans) return

      const signlightPlans = editor.scenario.getSignlightPlans()
      const { activePlan, signlights: allSignlights } = signlightPlans

      const group = groupBy(allSignlights, 'plan')
      this.planList = Object.keys(group).map((id) => {
        const name = id === '0' ? '默认信控配置' : `信控配置 ${id}`
        return { id, name }
      })

      if (this.planList.length > 0) {
        this.currentPlanId = activePlan
      }
    },
    switchPerspective1 (type) {
      // 这里直接绑定到ElDropdown的command事件里会报错，暂不知道为何
      this.switchPerspective(type)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.shortcut-button-bar {
  flex: 0 0 60px;
  margin-bottom: 1px;
  background-color: @dark-bg;
  color: @title-font-color;
  display: flex;
  justify-content: space-between;
  user-select: none;
}

.commonly-used-buttons,
.player-editor-buttons {
  height: 60px;
  padding: 0 20px;
  display: flex;
  align-items: center;
}

.button-wrap {
  .reset-button;
  width: 52px;
  height: 60px;
  display: flex;
  flex-direction: column;
  justify-content: flex-start;
  align-items: center;
}

.button-item {
  .button-wrap;

  &:hover:not(:disabled),
  &.button-active:not(:disabled) {
    color: @active-font-color;

    path {
      fill: @active-font-color;
    }
  }
}

.button-icon {
  height: 38px;
  width: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  position: relative;

  .el-icon-bottom {
    font-size: 24px;
  }
}

.button-text {
  .text-overflow;
  max-width: 100%;
  text-align: center;
  min-width: 52px;
  line-height: 18px;
  font-size: 12px;
}

.zoom-button {
  .button-wrap;
  cursor: default;

  &:hover:not(.disabled) {
    background-color: transparent;
    color: inherit;
  }

  &.disabled {
    color: @disabled-color;
  }

  .zoom-icon {
    display: flex;
    align-items: center;
    cursor: pointer;
    padding: 0;
    margin: 0;
    background-color: @dark-bg;
    border: none;
    color: @title-font-color;
    outline: none;

    &:hover:not(:disabled) {
      color: @active-font-color;
    }

    &:disabled {
      cursor: not-allowed;
      color: @disabled-color;
    }
  }
}

.commonly-used-buttons,
.player-editor-buttons {
  gap: 10px;
}

.button-split {
  margin: 0 10px;
  position: relative;

  &::after {
    content: '';
    width: 0;
    height: 50px;
    border-left: 1px solid #000;
    position: absolute;
    z-index: 2;
    top: 0;
    right: 0;
    transform: translateY(-50%);
  }
}

.player-editor-buttons {
  flex-direction: row-reverse;
}

.perspective-dropdown-menu {
  width: auto;
  max-height: 200px;
  overflow: auto;

  .el-dropdown-menu__item {
    color: @title-font-color;
    height: 24px;
    line-height: 24px;
    font-size: 12px;
    padding: 0 10px;
    display: flex;
    align-items: center;

    .perspective-dropdown-menu-item-icon {
      font-size: 24px;
      width: 24px;
      margin-right: 10px;
    }

    .perspective-dropdown-item-text {
      .text-overflow;
      flex: 1;
    }

    &.is-disabled {
      color: @disabled-color;
      cursor: not-allowed;
    }
  }
}

.global-mark {
  display: block;
  position: absolute;
  z-index: 2;
  top: 4px;
  left: 21px;
  font-size: 12px;
  text-shadow: 1px 1px @dark-bg,
  2px 2px @dark-bg,
    -1px -1px @dark-bg,
    -2px -2px @dark-bg,
  0 1px @dark-bg,
  0 2px @dark-bg,
  1px 0 @dark-bg,
  2px 0 @dark-bg;
}
</style>
