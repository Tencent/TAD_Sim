<template>
  <div class="log2world-setting">
    <el-form v-model="form" label-width="150px" :disabled="playingStatus === 'playing' || isPaused">
      <el-form-item class="switch-form" label="主车Log2World">
        <el-switch
          v-model="egoSwitch"
          class="planner-switch"
          :disabled="playingStatus === 'playing' || isPaused"
          active-color="#16d1f3"
          inactive-color="#3F3F3F"
        />
      </el-form-item>
      <el-form-item label="交通流Log2World">
        <el-switch
          v-model="trafficSwitch"
          class="planner-switch"
          :disabled="playingStatus === 'playing' || isPaused"
          active-color="#16d1f3"
          inactive-color="#3F3F3F"
        />
      </el-form-item>
      <el-form-item label="Log2World切换时机">
        <el-radio-group v-model="switchType">
          <el-tooltip effect="dark" content="本场景没有事件" placement="top" :disabled="!!eventsLength">
            <el-radio style="padding-top: 0px;" :label="2" :value="2" :disabled="!eventsLength">
              <span style="vertical-align: middle">第一个事件点前&nbsp;</span>
              <InputNumber
                :disabled="switchType === 1"
                style="vertical-align: middle"
                :model-value="secondUnit(switchTime)"
                unit="S"
                :precision="3"
                :min="0"
                :max="getSwitchMaxTime()"
                @update:model-value="updateState({ switchTime: $event * 1000 })"
              />
            </el-radio>
          </el-tooltip>
          <el-radio style="padding-top: 15px;" :label="1" :value="1">
            <span style="vertical-align: middle">场景第&nbsp;</span>
            <InputNumber
              :disabled="switchType === 2"
              style="vertical-align: middle"
              :model-value="secondUnit(switchTimeScene)"
              unit="S"
              :precision="3"
              :min="0"
              :max="100000"
              @update:model-value="updateState({ switchTimeScene: $event * 1000 })"
            />
            <el-icon class="el-icon-alarm-clock time-icon" @click="handleSetTime">
              <alarm-clock />
            </el-icon>
          </el-radio>
        </el-radio-group>
      </el-form-item>

      <section class="button-group">
        <el-button class="dialog-cancel" style="margin:0 15px;" @click="$emit('close')">
          取消
        </el-button>
        <el-button class="dialog-ok" @click="handleSave">
          确认
        </el-button>
      </section>
    </el-form>
  </div>
</template>

<script>
import { mapActions, mapMutations, mapState } from 'vuex'
import { ElMessage } from 'element-plus'
import InputNumber from '@/components/input-number.vue'
import { setLog2WorldConfig } from '@/api/log2world'

export default {
  components: {
    InputNumber,
  },
  data () {
    return {}
  },
  computed: {
    ...mapState('log2world', [
      'form',
      'events',
    ]),
    ...mapState('scenario', [
      'currentScenario',
      'playingStatus',
      'isPaused',
      'timestamp',
    ]),

    eventsLength () {
      return this.events?.events.length || 0
    },

    egoSwitch: {
      get () {
        return this.form.egoSwitch
      },
      set (val) {
        this.updateState({
          egoSwitch: val,
        })
      },
    },
    trafficSwitch: {
      get () {
        return this.form.trafficSwitch
      },
      set (val) {
        this.updateState({
          trafficSwitch: val,
        })
      },
    },
    switchTime: {
      get () {
        return this.form.switchTime
      },
      set (val) {
        this.updateState({
          switchTime: val,
        })
      },
    },
    switchType: {
      get () {
        return this.form.switchType
      },
      set (val) {
        this.updateState({
          switchType: val,
        })
      },
    },
    switchTimeScene: {
      get () {
        return this.form.switchTimeScene
      },
      set (val) {
        this.updateState({
          switchTimeScene: val,
        })
      },
    },
  },
  created () {
    if (this.currentScenario) {
      this.getLog2WorldConfig({
        scenePath: this.currentScenario.path,
      })
    }
  },

  methods: {
    ...mapMutations('log2world', [
      'updateState',
    ]),
    ...mapActions('log2world', [
      'getLog2WorldConfig',
    ]),
    ...mapActions('scenario', [
      'setup',
    ]),
    async handleSave () {
      if (!this.currentScenario) {
        return
      }
      const { egoSwitch, trafficSwitch, switchTime, switchType } = this.form
      const payload = {
        scenarioPath: this.currentScenario.path,
        params: {
          egoSwitch,
          trafficSwitch,
          switchTime: switchType === 2 ? switchTime : this.switchTimeScene,
          switchType,
        },
      }
      try {
        const { code, msg } = await setLog2WorldConfig(payload)
        if (code === 100) {
          if (this.currentScenario) {
            // 通知交通流切换状态
            await this.setup(this.currentScenario)
            await this.getLog2WorldConfig({
              scenePath: this.currentScenario.path,
            })
          }
          ElMessage.success('Log2World设置成功')
          this.$emit('close')
        } else {
          ElMessage.error(msg)
        }
      } catch (error) {
        ElMessage.error(error.message)
      }
    },

    handleSetTime () {
      this.updateState({ switchTimeScene: Math.max(this.timestamp, 0) })
    },
    getSwitchMaxTime () {
      const events = this.events?.events || []
      if (events.length) {
        return events[0].ts / 1000
      }
      return 100000
    },
    secondUnit (val) {
      return val / 1000
    },
  },
}
</script>

<style scoped lang='less'>
@import "@/assets/less/mixins";

.log2world-setting {
  width: 450px;
  height: auto;
  padding: 20px;

  .time {
    width: 60px
  }

  .el-form .el-form-item :deep(.el-form-item__label) {
    color: @title-font-color;
  }

  .el-radio-group .el-radio {
    display: block;
  }

  :deep(.planner-switch) {
    .el-switch__core::after {
      background-color: white
    }
  }

  .time-icon {
    color: white;
    margin-left: 7px;
    vertical-align: middle;

    &:hover {
      color: #16d1f3
    }
  }

  .dialog-ok, .dialog-cancel {
    color: @global-font-color;
  }

  .button-group {
    text-align: right;
    border-top: 1px solid @darker-bg;
    margin-top: 15px;
    padding-top: 15px;
  }

}
</style>
