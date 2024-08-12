<template>
  <div id="home" v-loading="loading">
    <TitleBar />
    <WindowButtons />
    <ShortcutButtonBar />
    <div id="main">
      <LeftBar />
      <div class="player-editor">
        <StatusBar v-if="isModifying || isPlaying" />
        <SimStage class="player-editor-content" />
        <PlayerControls v-if="isPlaying" />
        <DebugBar v-if="isPlaying" />
      </div>
      <ChartBar v-if="isPlaying" />
      <div v-if="isModifying" class="right-bar">
        <ScenarioElements />
        <ElementProps />
      </div>
    </div>
    <SimDialog />
    <el-dialog
      top="40vh"
      :header="progressTitle"
      :model-value="progressVisible"
      width="400px"
      append-to-body
      :show-close="false"
      :close-on-press-escape="false"
      :close-on-click-modal="false"
    >
      <div class="progress-wrapper">
        <el-progress :percentage="progressPercentage" />
      </div>
    </el-dialog>
    <ProgressBar />
  </div>
</template>

<script>
import { mapGetters, mapState } from 'vuex'
import TitleBar from '@/components/title-bar.vue'
import WindowButtons from '@/components/window-buttons.vue'
import ShortcutButtonBar from '@/components/shortcut-button-bar.vue'
import LeftBar from '@/components/left-bar.vue'
import StatusBar from '@/components/status-bar.vue'
import PlayerControls from '@/components/player-controls.vue'
import SimStage from '@/components/stage.vue'
import ChartBar from '@/components/chart-bar.vue'
import DebugBar from '@/components/debug-bar.vue'
import ScenarioElements from '@/components/scenario-elements.vue'
import ElementProps from '@/components/element-props/index.vue'
import SimDialog from '@/components/popups/dialog.vue'
import ProgressBar from '@/components/progress-bar.vue'

const { electron: { focusedBrowserWindow } } = window
export default {
  name: 'SimHome',
  components: {
    TitleBar,
    WindowButtons,
    ShortcutButtonBar,
    LeftBar,
    StatusBar,
    PlayerControls,
    SimStage,
    ChartBar,
    DebugBar,
    ScenarioElements,
    ElementProps,
    SimDialog,
    ProgressBar,
  },
  computed: {
    ...mapGetters('scenario', [
      'isModifying',
      'isPlaying',
    ]),
    ...mapGetters(['loading']),
    ...mapState('mission', [
      'progressVisible',
      'progressPercentage',
      'progressTitle',
    ]),
  },
  mounted () {
    window.onresize = async () => {
      const isMaximized = await focusedBrowserWindow.isMaximized()
      this.$store.commit('title-bar/updateState', { isMaximized })
    }
  },
}
</script>

<style lang="less">
  @import "@/assets/less/mixins";

  #home {
    height: 100%;
    display: flex;
    flex-direction: column;
  }

  #main {
    flex: 1;
    display: flex;
    align-items: stretch;
    background-color: black;
    overflow: hidden;

    .player-editor {
      flex: 1;
      margin-right: 1px;
      background-color: @dark-bg;
      display: flex;
      flex-direction: column;
      position: relative;
      overflow: hidden;

      .player-editor-content {
        flex: 1;
      }
    }

    .right-bar {
      width: 260px;
      display: flex;
      flex-direction: column;
      overflow: hidden;
      min-height: 1px;
    }
  }

  .progress-wrapper {
    padding: 20px 8px 20px 20px;
  }
</style>
