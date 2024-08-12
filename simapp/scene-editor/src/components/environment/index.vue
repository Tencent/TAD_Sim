<template>
  <div v-loading="loading" class="environment-config">
    <el-tabs v-model="currentTab" type="border-card" :before-leave="updateCurrentTab">
      <el-tab-pane
        name="scenarioEvnGroup"
        :label="$t('sensor.sceneConf')"
      >
        <EnvironmentGroup ref="scenarioEvnGroup" type="scenarioEvnGroup" />
      </el-tab-pane>
      <el-tab-pane
        name="globalEnvGroup"
        :label="$t('sensor.globalConf')"
      >
        <EnvironmentGroup ref="globalEnvGroup" type="globalEnvGroup" />
      </el-tab-pane>
    </el-tabs>
    <div class="btns">
      <div>
        <el-button size="small" @click="resetAll">
          {{ $t('operation.resetConfig') }}
        </el-button>
        <div class="global-mode-switch">
          <sim-label>{{ $t('sensor.globalMode') }}</sim-label>
          <el-switch
            :model-value="globalMode"
            @change="updateState({ globalMode: $event })"
          />
        </div>
      </div>
      <div>
        <el-button size="small" @click="onCancel">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button size="small" @click="saveEnv">
          {{ $t('operation.saveConf') }}
        </el-button>
      </div>
    </div>
  </div>
</template>

<script>
import { mapActions, mapMutations, mapState } from 'vuex'
import EnvironmentGroup from './environment-group.vue'
import eventBus from '@/event-bus'

export default {
  name: 'EnvironmentConfig',
  components: {
    // NumericSliderInput,
    EnvironmentGroup,
  },
  data () {
    return {
      loading: false,
      currentTab: 'scenarioEvnGroup',
    }
  },
  computed: {
    ...mapState('environment', [
      'globalMode',
    ]),
  },
  async created () {
    await this.resetAll()
  },
  methods: {
    ...mapMutations('environment', [
      'updateState',
    ]),
    ...mapActions('environment', [
      'getConfig',
      'saveConfig',
    ]),
    async resetAll () {
      this.loading = true
      try {
        await this.getConfig()
        eventBus.$emit('environment/reset')
      } catch (e) {
        this.$errorBox(e.message)
      }
      this.loading = false
    },
    async onCancel () {
      await this.resetAll()
      this.$emit('close')
    },
    async saveEnv () {
      try {
        await this.validate()
      } catch (e) {
        return e
      }

      this.loading = true
      try {
        await this.saveConfig()
      } catch (e) {
        this.$errorBox(e.message)
      }
      this.loading = false
      this.$emit('close')
    },
    async beforeClose () {
      await this.resetAll()
    },
    validate (...args) {
      return this.$refs[this.currentTab].$refs.envForm.$refs.form.validate(...args)
    },
    updateCurrentTab () {
      return this.validate()
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .environment-config {
    width: 580px;

    :deep(.el-tabs__header) {
      margin-bottom: 0;
      border-bottom-color: @darker-bg;
    }

    :deep(.el-tabs__nav) {
      display: flex;
      float: none;

      .el-tabs__item {
        flex: 1;
        background-color: @dark-bg;
        max-width: 33%;

        &.is-active {
          background-color: @hover-bg;
        }
      }
    }

    .el-tabs--border-card {
      box-shadow: none;
      border-right: none;
      border-left: none;
      border-bottom: none;
    }

    :deep(.el-tabs__content) {
      padding: 0;
    }

    .global-mode-switch {
      display: inline-block;
      margin-left: 30px;

      :deep(.sim-label) {
        margin-right: 10px;
      }
    }

    .btns {
      height: 48px;
      padding: 0 20px;
      display: flex;
      justify-content: space-between;
      align-items: center;

      .el-button {
        margin-left: 10px;
      }
    }
  }
</style>
