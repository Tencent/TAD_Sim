<template>
  <div id="log-message" class="log-message">
    <ul class="log-message-tabs">
      <li
        v-for="m in modulesFocus"
        :key="m.name"
        class="log-message-tab-item"
        :class="{ active: currentName === m.name }"
        @click="updateState({ currentName: m.name })"
      >
        {{ m.name }}
      </li>
    </ul>
    <div ref="content" class="log-message-content">
      <pre v-if="currentMessage">{{ currentMessage }}</pre>
    </div>
  </div>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { getLogMessage } from '@/stored/log-message'
import eventBus from '@/event-bus'

export default {
  name: 'LogMessage',
  data () {
    return {
      logMessage: null,
    }
  },
  computed: {
    ...mapState('debug-bar', [
      'currentName',
    ]),
    ...mapGetters('debug-bar', [
      'currentLogFileName',
    ]),
    ...mapGetters('sim-modules', [
      'modulesActive',
    ]),
    ...mapState('scenario', ['followingPlannerId']),
    modulesFocus () {
      return this.modulesActive.filter((m) => {
        return m.groupName === `Ego_${`${this.followingPlannerId}`.padStart(3, '0')}` || m.groupName === 'global'
      })
    },
    currentMessage () {
      if (this.logMessage && this.currentLogFileName) {
        return this.logMessage[this.currentLogFileName]
      } else {
        return null
      }
    },
  },
  watch: {
    modulesFocus: {
      handler (val) {
        if (val.length && !this.modulesFocus.find(m => m.name === this.currentName)) {
          this.updateState({ currentName: val[0].name || '' })
        }
      },
      immediate: true,
    },
    currentName () {
      this.scrollToBottom()
    },
    currentMessage: {
      handler () {
        this.scrollToBottom()
      },
      immediate: true,
    },
  },
  created () {
    eventBus.$on('log-message', this.handleLogMessage)
    this.getModuleSetList()
  },
  mounted () {
    this.handleLogMessage({ detail: getLogMessage() })
  },
  beforeUnmount () {
    eventBus.$off('log-message', this.handleLogMessage)
  },
  methods: {
    ...mapMutations('debug-bar', [
      'updateState',
    ]),
    ...mapActions('module-set', [
      'getModuleSetList',
    ]),
    handleLogMessage (payload) {
      this.logMessage = payload?.detail || ''
    },
    scrollToBottom () {
      this.$nextTick(() => {
        const el = this.$refs.content
        if (el) {
          el.scrollTop = el.scrollHeight
        }
      })
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.log-message {
  height: 100%;
  display: flex;
  flex-direction: column;

  .log-message-tabs {
    padding-bottom: 10px;

    .log-message-tab-item {
      display: inline-block;
      margin-right: 20px;
      cursor: pointer;
      color: @title-font-color;

      &.active {
        cursor: default;
        color: @active-font-color;
      }
    }
  }

  .log-message-content {
    flex: 1;
    line-height: 1.25;
    overflow-y: auto;

    pre {
      margin: 0;
      padding: 0;
      word-break: break-all;
      white-space: pre-wrap;
    }
  }
}
</style>
