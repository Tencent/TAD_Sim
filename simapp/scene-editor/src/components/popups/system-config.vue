<template>
  <div class="system-config">
    <el-tabs v-model="activeTab">
      <el-tab-pane :label="$t('menu.simSettings')" name="simulation">
        <el-form
          ref="form"
          :model="form.simulation"
          :rules="rules"
          label-width="120px"
          @submit.prevent
        >
          <section class="part-simulation">
            <h6 v-if="false" class="section-title">
              {{ $t('menu.simSettings') }}
            </h6>
            <div class="part-simulation-content">
              <ul class="config-list">
                <li class="config-item">
                  <el-form-item :label="$t('system.playRateControl')" prop="playControlRate">
                    <InputNumber
                      id="play-control-rate"
                      v-model="form.simulation.playControlRate"
                      unit="%"
                      :min="0"
                      :max="10000"
                      name="simulation.playControlRate"
                    />
                  </el-form-item>
                </li>
                <li class="config-item">
                  <el-form-item :label="$t('system.sceneMaxDuration')" prop="scenarioTimeLimit">
                    <el-select
                      id="scenarioTimeLimitType"
                      v-model="scenarioTimeLimitType"
                      class="short-input"
                      name="scenarioTimeLimitType"
                      @change="onTimeLimitTypeChanged"
                    >
                      <el-option
                        v-for="item in optionScenarioTimeLimitTypeList"
                        :id="`scenarioTimeLimitType${item.id}`"
                        :key="item.id"
                        :label="$t(item.name)"
                        :value="item.id"
                      />
                    </el-select>
                    <InputNumber
                      v-if="scenarioTimeLimitType !== 0"
                      id="scenarioTimeLimit"
                      v-model="form.simulation.scenarioTimeLimit"
                      unit="s"
                      :min="0"
                      :max="999999999"
                      name="simulation.scenarioTimeLimit"
                    />
                  </el-form-item>
                </li>
                <li class="config-item">
                  <el-form-item :label="$t('system.scheduleMode')" prop="coordinationMode">
                    <el-select
                      id="coordinationMode"
                      v-model="form.simulation.coordinationMode"
                      name="coordinationMode"
                    >
                      <el-option
                        v-for="item in optionCoordinationModeList"
                        :id="`coordinationMode${item.id}`"
                        :key="item.id"
                        :label="$t(item.name)"
                        :value="item.id"
                      />
                    </el-select>
                  </el-form-item>
                </li>
                <li class="config-item">
                  <el-form-item
                    :label="$t('system.EgoInitPosMessage')"
                    :title="$t('system.EgoInitPosMessageTip')"
                    prop="addingInitialLocationMsg"
                  >
                    <el-checkbox
                      id="addingInitialLocationMsg"
                      v-model="form.simulation.addingInitialLocationMsg"
                      name="addingInitialLocationMsg"
                    />
                  </el-form-item>
                </li>
                <li class="config-item">
                  <el-form-item
                    :label="$t('system.autoReset')"
                    :title="$t('system.autoResetTip')"
                    prop="autoReset"
                  >
                    <el-checkbox id="autoReset" v-model="form.simulation.autoReset" name="autoReset" />
                  </el-form-item>
                </li>
                <li class="config-item">
                  <el-form-item
                    :label="$t('system.overwriteUserLog')"
                    :title="$t('tips.restartToEffect')"
                    prop="overrideUserLog"
                  >
                    <el-checkbox
                      id="overrideUserLog"
                      v-model="form.simulation.overrideUserLog"
                      name="overrideUserLog"
                    />
                  </el-form-item>
                </li>
                <li class="config-item block">
                  <el-form-item
                    :label="$t('system.evaluationScriptPath')" prop="gradingFeedbackProcess"
                    label-width="160px"
                  >
                    <InputFileBrowser
                      id="gradingFeedbackProcess"
                      v-model="form.simulation.gradingFeedbackProcess"
                      :title="$t('tips.selectExecutableItem')"
                      :properties="['openFile']"
                      input-readonly
                    />
                  </el-form-item>
                </li>
              </ul>
            </div>
          </section>
          <section class="part-button-panel">
            <el-button id="system-config-reset" class="dialog-ok" @click="handleRestore">
              {{
                $t('operation.reset')
              }}
            </el-button>
            <el-button
              id="system-config-reset-warning"
              class="dialog-ok"
              :title="$t('system.resetWarningTip')"
              @click="resetAllWarning"
            >
              {{ $t('system.resetWarning') }}
            </el-button>
          </section>
        </el-form>
      </el-tab-pane>
      <el-tab-pane :label="$t('menu.editorConfig')" name="editing">
        <el-form
          :model="form.editor"
          label-width="120px"
          @submit.prevent
        >
          <div class="part-editing">
            <el-form-item prop="autoFollowRoadDirection" label="初始角度">
              <el-tooltip effect="dark" content="场景元素位置变化时是否自动对齐车道方向，该配置为全局配置">
                <el-checkbox
                  v-model="form.editor.autoFollowRoadDirection"
                >
                  位置变化时自动对齐车道方向
                </el-checkbox>
              </el-tooltip>
            </el-form-item>
          </div>
        </el-form>
      </el-tab-pane>
    </el-tabs>
    <section class="part-button-group">
      <el-button id="system-config-cancel" class="dialog-cancel" style="margin:0 15px;" @click="$emit('close')">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button id="system-config-confirm" class="dialog-ok" @click="handleSave">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { isEqual } from 'lodash'
import { mapActions, mapState } from 'vuex'
import { toPlainObject } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'
import InputFileBrowser from '@/components/input-file-browser.vue'
import { messageBoxConfirm } from '@/plugins/message-box'

const optionScenarioTimeLimitTypeList = [
  { id: 0, name: 'system.unlimited' },
  { id: 1, name: 'system.customize' },
]
const optionCoordinationModeList = [
  { id: 1, name: 'system.synchronous' },
  { id: 2, name: 'system.asynchronous' },
  { id: 3, name: 'system.byPriority' },
]

// function positiveValidator (rule, value, callback) {
//   if (value > 0) {
//     callback()
//   } else {
//     callback(new Error(this.$t('tips.enterPositiveNum')))
//   }
// }

function nonnegativeValidator (rule, value, callback) {
  if (value >= 0) {
    callback()
  } else {
    callback(new Error(this.$t('tips.enterNonNegativeNum')))
  }
}

export default {
  name: 'SystemConfig',
  components: {
    InputNumber,
    InputFileBrowser,
  },
  props: {
    cancel: {
      type: Function,
      default () {
      },
    },
  },
  data () {
    return {
      activeTab: 'simulation',
      form: {
        simulation: {
          playControlRate: 100,
          scenarioTimeLimit: 60,
          coordinationMode: 1,
          addingInitialLocationMsg: true,
          autoReset: true,
          overrideUserLog: true,
          gradingFeedbackProcess: '',
        },
        editor: {
          autoFollowRoadDirection: true,
        },
      },
      rules: {
        playControlRate: [
          { required: true, message: this.$t('tips.enterPlaybackRateControl'), trigger: 'blur' },
          { type: 'number', message: this.$t('tips.enterNum'), trigger: 'blur' },
          { validator: nonnegativeValidator, trigger: 'blur' },
        ],
        scenarioTimeLimit: [
          { required: true, message: this.$t('tips.enterMaximumDuration'), trigger: 'blur' },
          { type: 'number', message: this.$t('tips.enterNum'), trigger: 'blur' },
          { validator: this.validTimeLimit, trigger: 'blur' },
        ],
        coordinationMode: [
          { required: true, message: this.$t('tips.enterSchedulingMode'), trigger: 'blur' },
          { type: 'number', message: this.$t('tips.enterNum'), trigger: 'blur' },
        ],
      },
      // originalSimulationForm: null,
      scenarioTimeLimitType: 0,
      optionScenarioTimeLimitTypeList,
      optionCoordinationModeList,
    }
  },
  computed: {
    ...mapState('system', [
      'simulation',
      'editor',
    ]),
  },
  watch: {
    // simulation: {
    //   handler (value) {
    //     const simulation = {
    //       ...value,
    //     }
    //     if (simulation.scenarioTimeLimit === 0) {
    //       simulation.scenarioTimeLimit = 60
    //       this.scenarioTimeLimitType = 0
    //     } else {
    //       this.scenarioTimeLimitType = 1
    //     }
    //     Object.assign(this.form.simulation, simulation)
    //     this.originalSimulationForm = toPlainObject(this.form.simulation)
    //   },
    //   immediate: true,
    // },
  },
  async created () {
    await this.getConfig()
    const {
      editor,
      simulation,
    } = this

    Object.assign(this.form.editor, editor)
    Object.assign(this.form.simulation, simulation)
    if (simulation.scenarioTimeLimit === 0) {
      this.scenarioTimeLimitType = 0
    } else {
      this.scenarioTimeLimitType = 1
    }
  },
  methods: {
    ...mapActions('system', [
      'getConfig',
      'saveConfig',
      'restoreDefaultConfig',
    ]),
    ...mapActions('title-bar', [
      'resetAllWarning',
    ]),
    onTimeLimitTypeChanged (type) {
      // console.log(type, '修改了时间限制类型')
      if (type === 0) {
        this.form.simulation.scenarioTimeLimit = 0
      } else {
        this.form.simulation.scenarioTimeLimit = 60
      }
    },
    async promptFormChanged () {
      if (!isEqual(this.simulation, toPlainObject(this.form.simulation))) {
        try {
          await messageBoxConfirm(this.$t('tips.whetherToSaveConfigChange'), {
            closeOnClickModal: false,
            closeOnPressEscape: false,
          })
          return this.handleSave()
        } catch (e) {
          return true
        }
      }
      return true
    },
    beforeClose () {
      return this.promptFormChanged()
    },
    async handleRestore () {
      try {
        await this.$confirmBox(this.$t('tips.whetherRestoreDefaultConfig'))
        this.restoreDefaultConfig()
      } catch (e) {
        // pass
      }
    },
    async handleSave () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      try {
        const form = toPlainObject(this.form)
        // if (this.scenarioTimeLimitType === 0) {
        //   form.simulation.scenarioTimeLimit = 0
        // }
        await this.saveConfig(form)

        this.$emit('close')
        return true
      } catch (e) {
        this.$errorBox(e.message)
        return false
      }
    },
    validTimeLimit (rule, value, callback) {
      if (this.scenarioTimeLimitType === 0) {
        if (value !== 0) {
          return callback(new Error(this.$t('tips.enterPositiveNum')))
        }
      } else if (this.scenarioTimeLimitType === 1) {
        if (value <= 0) {
          return callback(new Error(this.$t('tips.enterNonNegativeNum')))
        }
      }
      callback()
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.system-config {
  width: 580px;
  height: auto;
  padding: 20px;

  .part-simulation {
    padding-top: 15px;

    .part-simulation-content {
      padding: 10px 5px;

      .config-list {
        list-style: none;

        &:after {
          content: '';
          display: block;
          clear: both;
        }

        .config-item {
          float: left;
          width: 50%;
          height: 44px;
        }

        .config-item.block {
          width: 100%;
        }
      }
    }
  }

  .part-button-panel {
    text-align: left;
  }

  .part-button-group {
    text-align: right;
    margin-top: 25px;
    padding-top: 15px;
    border-top: 1px solid @darker-bg;
  }

  .part-editing {
    padding-top: 15px;
  }

  .section-title {
    border-bottom: solid 1px #333;
    padding: 5px 0;
    margin-bottom: 8px;
    font-size: 12px;
    color: @title-font-color;
    font-weight: normal;
  }

  .marginLeft {
    margin-left: 10px;
  }

  .el-form .el-form-item :deep(.el-form-item__label) {
    color: @title-font-color;
  }

  .short-input {
    width: 80px;
  }

  .input-unit {
    color: @disabled-color;
  }

  .dialog-ok, .dialog-cancel {
    color: @global-font-color;
  }
}
</style>
