<template>
  <div>
    <div class="current-time">
      {{ currentTimeFormatter(currentTime) }}
    </div>
    <!--  开始播放后才显示进度条；当前场景为 log2world 任务时，则场景运行时不显示进度条及场景总时间  -->
    <MonitorProgress v-if="isLogSim && !isLog2World && timestamp > 0" :events="events" :percentage="progress" />
    <div ref="player-controls" class="player-controls">
      <div class="player-controls-buttons">
        <template v-if="playingMode === 'list' && !shouldPlayAfterSetup">
          <el-tooltip effect="dark" :hide-after="1000" :content="$t('player.startBatchPlay')" placement="top">
            <span>
              <button
                id="player-controls-play-list"
                class="player-control-button"
                :disabled="filteredPlaylist.length === 0"
                @click="startBatchPlay"
              >
                <player-controls-play-svg />
              </button>
            </span>
          </el-tooltip>
        </template>
        <template v-if="playingMode === 'single' || (playingMode === 'list' && shouldPlayAfterSetup)">
          <el-tooltip
            v-if="buttonObject.play.vif"
            effect="dark"
            :hide-after="1000"
            :content="$t('player.play')"
            placement="top"
          >
            <span>
              <button
                id="player-controls-play"
                :disabled="!buttonObject.play.enabled"
                class="player-control-button"
                @click="play"
              >
                <player-controls-play-svg />
              </button>
            </span>
          </el-tooltip>
        </template>
        <template v-if="playingMode === 'single' || (playingMode === 'list' && shouldPlayAfterSetup)">
          <el-tooltip
            v-if="buttonObject.pause.vif"
            effect="dark"
            :hide-after="1000"
            :content="$t('player.pause')"
            placement="top"
          >
            <span>
              <button id="player-controls-pause" class="player-control-button" @click="() => pause()">
                <player-controls-pause-svg />
              </button>
            </span>
          </el-tooltip>
        </template>
        <template v-if="playingMode === 'single' || (playingMode === 'list' && shouldPlayAfterSetup)">
          <el-tooltip effect="dark" :hide-after="1000" :content="$t('player.nextFrame')" placement="top">
            <span>
              <button
                id="player-controls-step"
                :disabled="!buttonObject.stop.enabled"
                class="player-control-button"
                @click="step"
              >
                <player-controls-next-frame-svg />
              </button>
            </span>
          </el-tooltip>
        </template>
        <el-tooltip
          v-if="playingMode === 'single'"
          effect="dark"
          :hide-after="1000"
          :content="$t('player.stop')"
          placement="top"
        >
          <span>
            <button
              id="player-controls-stop"
              :disabled="!buttonObject.stop.enabled"
              class="player-control-button"
              @click="stop"
            >
              <player-controls-stop-svg />
            </button>
          </span>
        </el-tooltip>
        <template v-if="playingMode === 'list' && shouldPlayAfterSetup">
          <el-tooltip effect="dark" :hide-after="1000" :content="$t('tips.endCurrentScene')" placement="top">
            <span>
              <button
                id="player-controls-stop-list-current"
                :disabled="!buttonObject.stop.enabled"
                class="player-control-button"
                @click="stop('auto')"
              >
                <player-controls-stop-svg />
              </button>
            </span>
          </el-tooltip>
        </template>
        <span v-if="timestamp >= 0" class="player-control-text">{{ secondsUnderTotal }}</span>
      </div>
      <div v-if="playingMode === 'list'" class="player-controls-batch-play" />
      <div class="player-controls-panel">
        <span v-if="isLogSim" style="color:#ffffff; margin-right: 15px">
          <!-- todo: 翻译 -->
          <el-button @click="log2worldClick">切换设置</el-button>
        </span>
        <div v-if="playingMode === 'list'" class="player-controls-loop-times">
          <div v-if="shouldPlayAfterSetup">
            <button
              id="player-controls-stop-list-all"
              :disabled="!buttonObject.stop.enabled && false"
              class="player-controls-stop-list-all"
              @click="stopBatchPlay"
            >
              {{ $t('player.EndBatchPlay') }}
            </button>
          </div>
          <div v-else>
            <span class="player-controls-loop-times-label">{{ $t('player.ListCycleIndex') }}</span>
            <InputNumber
              class="player-controls-loop-times-input-number"
              :model-value="playingTimes"
              :unit="$t('indicator.timesUnit')"
              :precision="0"
              :min="1"
              :max="10000"
              @update:model-value="updateState({ playingTimes: $event, times: 1 })"
            />
          </div>
        </div>
        <el-tooltip
          v-if="(
            (playingMode === 'single' && playingStatus !== 'playing')
            || (playingMode === 'list' && !shouldPlayAfterSetup)
          )"
          effect="dark"
          :hide-after="1000"
          :content="$t(`player.${playingMode}PlayingMode`)"
          placement="top"
          class="player-controls-play-mode-tooltip"
        >
          <span>
            <button :id="`player-controls-play-mode-${playingMode}`" class="player-control-button" @click="switchMode">
              <player-controls-play-mode-single-svg v-if="playingMode === 'single'" />
              <player-controls-play-mode-list-svg v-else-if="playingMode === 'list'" />
            </button>
          </span>
        </el-tooltip>
      </div>
    </div>
  </div>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import MonitorProgress from './MonitorProgress.vue'
import InputNumber from '@/components/input-number.vue'

export default {
  name: 'PlayerControls',
  components: {
    InputNumber,
    MonitorProgress,
  },
  data () {
    return {
      playingModeText: {
        single: 'player.singlePlayingMode',
        list: 'player.listPlayingMode',
      },
      now: Date.now(),
      intervalId: 0,
    }
  },
  computed: {
    ...mapState('scenario', [
      'currentScenario',
      'playingStatus',
      'playingMode',
      'playingTimes',
      'timestamp',
      'times',
      'shouldPlayAfterSetup',
      'startPlaylistScenarioId',
    ]),
    ...mapState('log2world', [
      'events',
      'committedForm',
    ]),
    ...mapState('playlist', [
      'sceneType',
    ]),
    ...mapGetters('playlist', [
      'filteredPlaylist',
    ]),
    buttonObject () {
      const { playingStatus } = this
      return {
        play: {
          vif: playingStatus === 'uninitialized' || playingStatus === 'ready',
          enabled: playingStatus === 'ready',
        },
        pause: {
          vif: playingStatus === 'playing',
        },
        step: {
          enabled: playingStatus === 'ready',
        },
        stop: {
          enabled: playingStatus === 'ready',
        },
      }
    },

    progress () {
      const value = this.timestamp / this.events.duration * 100
      return Math.max(0, Math.min(value, 100))
    },

    isLogSim () {
      const flag = this.currentScenario && this.currentScenario.traffictype === 'simrec'
      return !!flag
    },

    currentTime () {
      if (this.isLogSim) {
        return this.events.startTimestamp + this.timestamp
      } else {
        return this.now
      }
    },

    secondsUnderTotal () {
      const cur = (this.timestamp / 1000).toFixed(3)

      if (this.isLogSim && !this.isLog2World) {
        const total = (this.events.duration / 1000).toFixed(3)
        return `${cur}s/${total}s`
      } else {
        return `${cur}s`
      }
    },
    isLog2World () {
      const { egoSwitch, trafficSwitch } = this.committedForm
      return egoSwitch || trafficSwitch
    },
  },
  created () {
    this.intervalId = setInterval(() => {
      this.now = Date.now()
    }, 1000)
  },
  beforeUnmount () {
    if (this.intervalId) {
      clearInterval(this.intervalId)
    }
  },
  methods: {
    ...mapMutations('scenario', [
      'updateState',
    ]),
    ...mapActions('scenario', [
      'play',
      'pause',
      'step',
      'stop',
      'resetScenario',
    ]),
    ...mapMutations('mission', [
      'startMission',
    ]),
    startBatchPlay () {
      const matched = this.filteredPlaylist.find(item => item.id === this.startPlaylistScenarioId)
      let scenario = this.filteredPlaylist[0]
      if (matched) {
        scenario = matched
      } else {
        this.updateState({ startPlaylistScenarioId: scenario.id })
      }
      this.updateState({ shouldPlayAfterSetup: true, times: 1 })
      this.resetScenario(scenario)
    },
    async stopBatchPlay () {
      try {
        await this.$confirmBox(this.$t('tips.whetherEndBatchPlayback'))
        this.updateState({ startPlaylistScenarioId: this.currentScenario.id })
        this.updateState({ shouldPlayAfterSetup: false })
        this.pause(true)
      } catch (e) {
        // pass
      }
    },
    switchMode () {
      let playingMode = ''
      if (this.playingMode === 'single') {
        playingMode = 'list'
      } else if (this.playingMode === 'list') {
        playingMode = 'single'
      }
      this.updateState({ playingMode, playingTimes: 1, timestamp: -1, times: 1 })
      if (playingMode) {
        this.$message({
          type: 'success',
          message: this.$t('tips.switchedTo', { playingMode: this.$t(this.playingModeText[playingMode]) }),
        })
      }
    },
    log2worldClick () {
      this.startMission('Log2WorldSetting')
    },
    currentTimeFormatter (value) {
      if (value < 0) {
        return ' '
      }
      const date = new Date(value)
      let minutes = date.getMinutes()
      if (minutes < 10) {
        minutes = `0${minutes}`
      }
      let seconds = date.getSeconds()
      if (seconds < 10) {
        seconds = `0${seconds}`
      }
      return `${date.getFullYear()}年${date.getMonth() + 1}月${date.getDate()}日 ${date.getHours()}:${minutes}:${seconds}`
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .current-time {
    padding: 3px;
    color: @global-font-color;
    background-color: rgba(0, 0, 0, .5);
  }

  .player-controls {
    box-sizing: border-box;
    height: 34px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    background-color: rgba(0, 0, 0, .5);

    .player-control-button {
      .reset-button();
      height: 24px;
      width: 24px;
      margin-left: 20px;
      background-color: transparent;

      &:hover:not(:disabled) {
        color: @active-font-color;
      }
    }

    .player-controls-buttons {
      span {
        display: inline-block;
        vertical-align: middle;
      }
    }

    .player-control-text {
      margin-left: 20px;
      color: @global-font-color;
    }

    .player-controls-panel {
      margin-right: 20px;

      .player-controls-stop-list-all {
        color: @global-font-color;
        background-color: rgba(0, 0, 0, .5);
        border: 1px solid @global-font-color;
        cursor: pointer;

        &:disabled {
          color: @disabled-color;
          border-color: @disabled-color;
          cursor: not-allowed;
        }
      }

      .player-controls-loop-times {
        display: inline-block;
        color: @title-font-color;
        font-size: 12px;

        .player-controls-loop-times-label {
          margin-right: 10px;
        }

        .player-controls-loop-times-input-number {
          width: 70px;
          vertical-align: middle;
          :deep(.el-input__inner) {
            padding-right: 20px;
          }
        }

        :deep(.el-input.is-active .el-input__inner),
        :deep(.el-input__inner:focus),
        :deep(.el-input__inner:hover) {
          border-color: #404040;
        }
      }

      .player-controls-play-mode-tooltip {
        vertical-align: middle;
      }

      .player-control-button {
        margin-left: 10px;
      }
    }
  }
</style>
