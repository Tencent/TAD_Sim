<template>
  <SectionBox class="debug-bar">
    <template #title>
      <div class="debug-title">
        <span class="debug-title-text">{{ $t('Debug') }}</span>
        <ArrowLine id="debug-toggle" :direction="direction" @click="toggleCollapse" />
      </div>
    </template>
    <div v-if="!collapse" ref="el" class="debug-wrap" :style="{ height: `${height}px` }">
      <ul id="debug-tabs" class="debug-tabs">
        <div class="debug-tab-items">
          <li
            id="debug-tab-log"
            class="debug-tab-item"
            :class="{ active: currentTab === 0 }"
            @click="updateState({ currentTab: 0 })"
          >
            {{ $t('Log') }}
          </li>
          <li
            id="debug-tab-message"
            class="debug-tab-item"
            :class="{ active: currentTab === 1 }"
            @click="updateState({ currentTab: 1 })"
          >
            {{ $t('Message') }}
          </li>
        </div>
        <div class="debug-tab-icons">
          <el-icon id="debug-clear" class="icon" @click="clear">
            <Delete />
          </el-icon>
          <el-icon id="debug-open-folder" class="icon" @click="showItemInFolder">
            <Folder />
          </el-icon>
        </div>
      </ul>
      <div class="debug-content">
        <LogMessage
          v-if="currentTab === 0"
        />
        <ProtoMessage
          v-if="currentTab === 1"
          ref="protoMessage"
        />
      </div>
    </div>
    <div
      ref="resizeLayer"
      class="debug-bar-resize-layer"
      @pointerdown="startResize"
      @pointermove="resizing"
      @pointerup="stopResize"
    />
  </SectionBox>
</template>

<script>
import { mapGetters, mapMutations, mapState } from 'vuex'
import ArrowLine from './arrow-line.vue'
import SectionBox from './section-box.vue'
import LogMessage from './log-message.vue'
import ProtoMessage from './proto-message.vue'
import eventBus from '@/event-bus'

const { electron } = window

export default {
  name: 'DebugBar',
  components: {
    SectionBox,
    ArrowLine,
    LogMessage,
    ProtoMessage,
  },
  data () {
    return {
      pointerId: -1,
      isMoving: false,
      height: 0,
      collapse: true,
      startPageY: 0,
      startHeight: 0,
      max: 500,
      min: 0,
    }
  },
  computed: {
    ...mapState('debug-bar', [
      'currentTab',
    ]),
    ...mapGetters('debug-bar', [
      'currentLogFileName',
    ]),
    direction () {
      return this.collapse ? 'up' : 'down'
    },
  },
  watch: {
    height (val) {
      this.collapse = !val
    },
  },
  mounted () {
    this.height = Number(localStorage.getItem('debug-bar-height')) || 0
  },
  methods: {
    ...mapMutations('debug-bar', [
      'updateState',
    ]),
    clear () {
      if (this.currentTab === 0) {
        eventBus.$emit('log-message', null)
        electron.userLog.clearMessage()
      } else if (this.currentTab === 1) {
        eventBus.$emit('proto-message', null)
        this.$refs.protoMessage.clear()
      }
    },
    showItemInFolder () {
      if (this.currentLogFileName) {
        electron.userLog.showItemInFolder(this.currentLogFileName)
      } else {
        electron.userLog.openUserLogFolder()
      }
    },
    toggleCollapse () {
      if (this.collapse) {
        this.height = 200
      } else {
        this.height = 0
      }

      localStorage.setItem('debug-bar-height', this.height)
    },
    startResize ($event) {
      let layer
      ({ pageY: this.startPageY, target: layer, pointerId: this.pointerId } = $event)
      this.isMoving = true
      layer.setPointerCapture(this.pointerId)

      this.startHeight = this.height
    },
    resizing ($event) {
      if (this.isMoving) {
        const distance = this.startPageY - $event.pageY
        this.height = Math.max(Math.min(this.startHeight + distance, this.max), this.min)
      }
    },
    stopResize ($event) {
      $event.target.releasePointerCapture(this.pointerId)
      this.isMoving = false
      localStorage.setItem('debug-bar-height', this.height)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.debug-bar {
  display: flex;
  flex-direction: column;
  cursor: default;
  position: relative;
  overflow: hidden;

  .debug-title {
    height: 28px;
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .debug-wrap {
    height: 0;
    display: flex;
    flex-direction: column;
    padding: 0 30px;
    background-color: @dark-bg;
    color: @title-font-color;
    font-size: 12px;

    .debug-tabs {
      display: flex;
      justify-content: space-between;
      height: 24px;
      padding-top: 4px;
      line-height: 24px;
      border-bottom: 2px solid @list-bd;

      .debug-tab-items {
        .debug-tab-item {
          display: inline-block;
          height: 24px;
          margin-right: 30px;
          cursor: pointer;
          color: @title-font-color;
          border-bottom: 2px solid transparent;
          transition: color, border-bottom-color .16s linear;

          &.active {
            cursor: default;
            color: @active-font-color;
            border-bottom-color: @active-font-color;
          }
        }
      }

      .debug-tab-icons {
        font-size: 13px;

        i {
          margin-left: 10px;
          cursor: pointer;
        }
      }
    }

    .debug-content {
      flex: 1;
      overflow: auto;
      padding: 10px 0;
    }
  }

  .debug-bar-resize-layer {
    height: 3px;
    width: 100%;
    position: absolute;
    z-index: 1000;
    top: 0;
    left: 0;
    cursor: row-resize;
  }
}
</style>
