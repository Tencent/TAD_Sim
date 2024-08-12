<template>
  <el-form
    v-if="presentScenario"
    ref="form"
    class="property-form property-scenario"
    :model="presentScenario"
    :rules="rules"
    label-width="68px"
    @submit.prevent
  >
    <el-collapse v-model="activeCollapseNames">
      <el-collapse-item id="scenario-property-basic-info" :title="$t('scenario.basicInformation')" name="basicInfo">
        <el-form-item prop="sceneName" class="param-cell" :label="$t('scenario.name')">
          <span class="scenario-name">{{ sceneName + nameExt }}</span>
        </el-form-item>
        <el-form-item prop="mapName" class="param-cell" :label="$t('scenario.map')">
          <span class="map-name">{{ mapName }}</span>
        </el-form-item>
        <el-form-item
          prop="info"
          class="param-cell"
          style="padding-bottom: 25px"
          :label="$t('scenario.workingCondition')"
        >
          <el-input
            v-model="presentScenario.info"
            type="textarea"
            maxlength="200"
            show-word-limit
            @change="onInfoChange"
          />
        </el-form-item>
        <template v-if="generateInfo && generateInfo.parameters">
          <el-form-item v-if="generateInfo" class="param-cell" :label="$t('scenario.genSource')">
            <p class="generate-info generate-source" :title="generateInfo.originFile">
              {{ generateInfo.originFile }}
            </p>
          </el-form-item>
          <el-form-item v-if="generateInfo" class="param-cell" :label="$t('scenario.genStrategy')">
            <p class="generate-info" v-html="generateParameters" />
          </el-form-item>
        </template>
      </el-collapse-item>
      <!-- 场景设置 -->
      <BaseAuth :perm="['router.signlightControl.show']">
        <el-collapse-item
          id="scenario-property-scenario-setting"
          :title="$t('scenario.scenarioSetting')"
          name="scenarioSetting"
        >
          <!-- 信控配置 -->
          <el-form-item
            prop="signal"
            class="param-cell"
            style="padding-bottom: 25px"
            :class="{ disabled: !planList.length }"
            :label="$t('scenario.signalControlSetting')"
          >
            <BaseAuth :perm="['action.signlightControl.plan.switch.enable', 'disabled']">
              <template #default="{ disabled }">
                <el-select
                  id="signal-control-list-select"
                  v-model="currentPlanId"
                  class="signal-control-list-select"
                  style="width: 120px; margin-right: 8px"
                  :disabled="disabled || !planList.length"
                  @change="handleSignalControlPlanChange"
                >
                  <el-option
                    v-for="item in planList"
                    :id="`signal-control-list-option-${item.name}`"
                    :key="item.id"
                    :label="$t(item.name)"
                    :value="item.id"
                    :style="{ maxWidth: '300px' }"
                    :title="item.name"
                  />
                </el-select>
              </template>
            </BaseAuth>
            <el-icon class="el-icon-setting open-signal-control" @click="handleOpenSignalControl">
              <Setting />
            </el-icon>
          </el-form-item>
        </el-collapse-item>
      </BaseAuth>
      <!-- 指标设置 -->
      <el-collapse-item id="scenario-property-kpi-group" :title="$t('indicator.settingGroup')" name="kpiGroupSetting">
        <el-select
          id="kpi-group-list-select"
          v-model="currentKpiGroupFilePath"
          style="padding-bottom: 10px"
          class="kpi-group-list-select"
          @change="handleCurrentKpiGroupFilePathChange"
        >
          <el-option
            v-for="item in computedKpiGroupList"
            :id="`kpi-group-list-option-${item.name}`"
            :key="item.id"
            :label="$t(item.name)"
            :value="item.filePath"
            :style="{ maxWidth: '300px' }"
            :title="item.name"
          />
        </el-select>
        <el-icon class="el-icon-setting open-kpi-group-management" @click="handleOpenKpiGroupManagement">
          <Setting />
        </el-icon>
      </el-collapse-item>
    </el-collapse>
  </el-form>
</template>

<script>
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import _ from 'lodash'
import { validateScenarioName } from '@/common/utils'
import { editor, player } from '@/api/interface'
import { renameScenario } from '@/api/scenario'
import dict from '@/common/dict'

const { distanceModeOptionList } = dict

const distanceModes = {}
distanceModeOptionList.forEach((option) => {
  distanceModes[option.id] = option.name
})

export default {
  name: 'PropertyScenario',
  data () {
    return {
      mapName: '',
      rules: {},
      activeCollapseNames: ['basicInfo', 'scenarioSetting', 'kpiGroupSetting'],
      currentKpiGroupFilePath: '',
      sceneName: '',
      nameExt: '',
      currentPlanId: '',
      planList: [],
    }
  },
  computed: {
    ...mapState('scenario', [
      'generateInfo',
      'scenarioList',
    ]),
    ...mapGetters('scenario', [
      'presentScenario',
      'isPlaying',
    ]),
    ...mapState('kpi-group', [
      'kpiGroupList',
    ]),
    ...mapState('signalControl', [
      'signlightPlanUpdateFlag',
    ]),
    generateParameters () {
      const { parameters = '{}' } = this.generateInfo || {}
      let params
      try {
        params = JSON.parse(parameters)
      } catch (e) {
        params = {}
      }
      const result = []

      if (_.get(params, 'ego.velocity.start', 0) || _.get(params, 'ego.velocity.end', 0)) {
        result.push(`${this.$t('scenario.initSpeedEgo')}: ${_.get(params, 'ego.velocity.start', 0) || 0}m/s-${_.get(params, 'ego.velocity.end', 0) || 0}m/s；`)
        result.push(`${this.$t('scenario.speedIntervalEgo')}: ${_.get(params, 'ego.velocity.sep', 0) || 0}m/s；`)
      }

      if (params.type === 'all') {
        result.push(this.$t('scenario.vehicleTrigger', { t: 'Overall' }))
      } else {
        result.push(this.$t('scenario.vehicleTrigger', { t: `car${_.get(params, 'vehicle.id', '')}` }))
      }

      if (_.get(params, 'vehicle.velocity.start', 0) || _.get(params, 'vehicle.velocity.end', 0)) {
        result.push(`${this.$t('scenario.initSpeedVehicle')}: ${_.get(params, 'vehicle.velocity.start', 0) || 0}-${_.get(params, 'vehicle.velocity.end', 0) || 0}m/s，`)
        result.push(`${this.$t('scenario.speedIntervalVehicles')}: ${_.get(params, 'vehicle.velocity.sep', 0) || 0}m/s；`)
      }

      if (_.get(params, 'vehicle.offset.start', 0) || _.get(params, 'vehicle.offset.end', 0)) {
        result.push(`${this.$t('scenario.lateralOffsetRangeVehicles')}: ${_.get(params, 'vehicle.offset.start', 0) || 0}-${_.get(params, 'vehicle.offset.end', 0) || 0}m，`)
        result.push(`${this.$t('scenario.lateralOffsetIntervalVehicles')}: ${_.get(params, 'vehicle.offset.sep', 0) || 0}m；`)
      }

      if (_.get(params, 'vehicle.pos.start', 0) || _.get(params, 'vehicle.pos.end', 0)) {
        result.push(`${this.$t('scenario.longOffsetRangeVehicles')}: ${_.get(params, 'vehicle.pos.start', 0) || 0}-${_.get(params, 'vehicle.pos.end', 0) || 0}m，`)
        result.push(`${this.$t('scenario.longOffsetIntervalVehicles')}: ${_.get(params, 'vehicle.pos.sep', 0) || 0}m；`)
      }

      _.forEach(_.get(params, 'vehicle.trigger.timeList', []), (timeTrigger) => {
        const strArr = [
          this.$t('scenario.TimeTriggering'),
        ]
        if (_.get(timeTrigger, 'time.start', 0) || _.get(timeTrigger, 'time.end', 0)) {
          strArr.push(`${this.$t('scenario.timeTriggerValue')}: ${_.get(timeTrigger, 'time.start', 0) || 0}-${_.get(timeTrigger, 'time.end', 0) || 0}s`)
          strArr.push(`${this.$t('scenario.timeTriggerInterval')}: ${_.get(timeTrigger, 'time.sep', 0) || 0}s`)
        }

        if (_.get(timeTrigger, 'velocity.start', 0) || _.get(timeTrigger, 'velocity.end', 0)) {
          strArr.push(`${this.$t('scenario.speedRange')}: ${_.get(timeTrigger, 'velocity.start', 0) || 0}-${_.get(timeTrigger, 'velocity.end', 0) || 0}m/s`)
          strArr.push(`${this.$t('scenario.SpeedInterval')}: ${_.get(timeTrigger, 'velocity.sep', 0) || 0}m/s`)
        }

        if (_.get(timeTrigger, 'acceleration.start', 0) || _.get(timeTrigger, 'acceleration.end', 0)) {
          strArr.push(`${this.$t('scenario.accRange')}: ${_.get(timeTrigger, 'acceleration.start', 0) || 0}-${_.get(timeTrigger, 'acceleration.end', 0) || 0}m/s²`)
          strArr.push(`${this.$t('scenario.accInterval')}: ${_.get(timeTrigger, 'acceleration.sep', 0) || 0}m/s²`)
        }

        if (_.get(timeTrigger, 'offsets.start', 0) || _.get(timeTrigger, 'offsets.end', 0)) {
          strArr.push(`${this.$t('scenario.lateralOffsetRange')}: ${_.get(timeTrigger, 'offsets.start', 0) || 0}-${_.get(timeTrigger, 'offsets.end', 0) || 0}m`)
          strArr.push(`${this.$t('scenario.lateralOffsetInterval')}: ${_.get(timeTrigger, 'offsets.sep', 0) || 0}m`)
        }

        if (_.get(timeTrigger, 'durations.start', 0) || _.get(timeTrigger, 'durations.end', 0)) {
          strArr.push(`${this.$t('scenario.offsetCompletionTimeRange')}: ${_.get(timeTrigger, 'durations.start', 0) || 0}-${_.get(timeTrigger, 'durations.end', 0) || 0}s`)
          strArr.push(`${this.$t('scenario.offsetCompletionInterval')}: ${_.get(timeTrigger, 'durations.sep', 0) || 0}s`)
        }

        result.push(`${strArr.join('，')}；`)
      })

      _.forEach(_.get(params, 'vehicle.trigger.conditionList', []), (conditionTrigger) => {
        const strArr = []

        if (conditionTrigger.type.value === 'timeRelative') {
          strArr.push('TTC')
        } else if (conditionTrigger.type.value === 'distanceRelative') {
          strArr.push(this.$t('scenario.offsetCompletionInterval'))
        }

        strArr.push(`${this.$t('scenario.distanceType')}: ${distanceModes[conditionTrigger.distancemode.value]}`)

        strArr.push(`${this.$t('scenario.triggerConditionValueRange')}: ${_.get(conditionTrigger, 'value.start', 0) || 0}-${_.get(conditionTrigger, 'value.start', 0) || 0}`)
        strArr.push(`${this.$t('scenario.triggerConditionValueInterval')}: ${_.get(conditionTrigger, 'value.sep', 0) || 0}`)
        strArr.push(`${this.$t('scenario.conditionalTriggerTimes')}: ${distanceModes[conditionTrigger.times.value] || 0}`)
        strArr.push(`${this.$t('scenario.posOffset')}: ${dict.triggerDirectionOptionList[conditionTrigger.direction.value]}`)

        if (_.get(conditionTrigger, 'velocity.start', 0) || _.get(conditionTrigger, 'velocity.end', 0)) {
          strArr.push(`${this.$t('scenario.speedRange')}: ${_.get(conditionTrigger, 'velocity.start', 0) || 0}-${_.get(conditionTrigger, 'velocity.end', 0) || 0}m/s`)
          strArr.push(`${this.$t('scenario.SpeedInterval')}: ${_.get(conditionTrigger, 'velocity.sep', 0) || 0}m/s`)
        }

        if (_.get(conditionTrigger, 'acceleration.start', 0) || _.get(conditionTrigger, 'acceleration.end', 0)) {
          strArr.push(`${this.$t('scenario.accRange')}: ${_.get(conditionTrigger, 'acceleration.start', 0) || 0}-${_.get(conditionTrigger, 'acceleration.end', 0) || 0}m/s²`)
          strArr.push(`${this.$t('scenario.accInterval')}: ${_.get(conditionTrigger, 'acceleration.sep', 0) || 0}m/s²`)
        }

        if (_.get(conditionTrigger, 'offsets.start', 0) || _.get(conditionTrigger, 'offsets.end', 0)) {
          strArr.push(`${this.$t('scenario.offsetRange')}: ${_.get(conditionTrigger, 'offsets.start', 0) || 0}-${_.get(conditionTrigger, 'offsets.end', 0) || 0}m`)
          strArr.push(`${this.$t('scenario.OffsetInterval')}: ${_.get(conditionTrigger, 'offsets.sep', 0) || 0}m`)
        }

        if (_.get(conditionTrigger, 'durations.start', 0) || _.get(conditionTrigger, 'durations.end', 0)) {
          strArr.push(`${this.$t('scenario.offsetCompletionTimeRange')}: ${_.get(conditionTrigger, 'durations.start', 0) || 0}-${_.get(conditionTrigger, 'durations.end', 0) || 0}s`)
          strArr.push(`${this.$t('scenario.offsetCompletionInterval')}: ${_.get(conditionTrigger, 'durations.sep', 0) || 0}s`)
        }

        result.push(`${strArr.join('，')}；`)
      })

      return result.join('<br />')
    },
    computedKpiGroupList () {
      return [
        { id: -1, name: 'indicator.defaultGroup', filePath: '' },
        ...this.kpiGroupList,
      ]
    },

    fullName () {
      let fullName = this.presentScenario.filename
      if (!fullName) {
        fullName = `${this.presentScenario.name}.sim`
      }
      return fullName
    },
  },
  watch: {
    presentScenario (newValue) {
      if (newValue) {
        this.updateData()
      }
    },
    signlightPlanUpdateFlag () {
      this.updatePlanList()
    },
  },
  created () {
    this.getKpiGroupList()
  },
  mounted () {
    this.updateData()
    this.updatePlanList()
  },
  methods: {
    ...mapMutations('scenario', {
      updateScenario: 'updateState',
      updatePresentScenario: 'updatePresentScenario',
    }),
    ...mapMutations('mission', [
      'startMission',
    ]),
    ...mapActions('kpi-group', [
      'getKpiGroupList',
    ]),
    ...mapActions('scenario', [
      'getScenarioList',
    ]),
    ...mapActions('signalControl', [
      'resetSignalControl',
      'applySignlightPlans',
    ]),
    updateData () {
      let mapInfo
      if (this.isPlaying) {
        mapInfo = player.scenario.getMapInfo()
      } else {
        mapInfo = editor.scenario.getMapInfo()
      }
      this.mapName = _.get(mapInfo, 'mapName')
      this.currentKpiGroupFilePath = editor.scenario.getCurrentKPIConfig()

      this.updateSceneName()
      this.updateNameExt()
    },

    // 刷新信控配置列表
    updatePlanList () {
      if (!editor?.scenario?.getSignlightPlans) return

      const signlightPlans = editor.scenario.getSignlightPlans()
      const { activePlan, signlights: allSignlights } = signlightPlans

      console.log('signlightPlans:', signlightPlans)
      const group = _.groupBy(allSignlights, 'plan')
      this.planList = Object.keys(group).map((id) => {
        const name = id === '0' ? '默认信控配置' : `信控配置 ${id}`
        return { id, name }
      })

      if (this.planList.length > 0) {
        this.currentPlanId = activePlan
      }
    },

    updateSceneName () {
      const fullName = this.fullName || ''
      const index = fullName.lastIndexOf('.')
      this.sceneName = fullName.slice(0, index)
    },

    updateNameExt () {
      const fullName = this.fullName || ''
      const index = fullName.lastIndexOf('.')
      this.nameExt = fullName.slice(index)
    },

    handleCurrentKpiGroupFilePathChange (value) {
      this.updateScenario({
        isDirty: true,
      })
      editor.scenario.setCurrentKPIConfig(value)
    },
    handleOpenSignalControl () {
      if (this.planList.length < 1) {
        this.$message.warning(this.$t('tips.noSignlightForPlan'))
        return
      }
      this.resetSignalControl()
      this.startMission('SignlightConfig')
    },
    async handleSignalControlPlanChange (planId) {
      await this.applySignlightPlans(planId)
      const tips = this.$t('operation.apply') + this.$t('tips.success') // tip: 应用成功
      this.$message.success(tips)
      this.updateScenario({ isDirty: true })
    },
    handleOpenKpiGroupManagement () {
      this.startMission('KpiManagement')
    },

    onInfoChange (value) {
      editor.scenario.setSceneDesc(value)
      this.updateScenario({
        isDirty: true,
      })
    },

    async onSceneNameChange (value) {
      if (!this.validateName(value)) {
        this.updateSceneName()
        return
      }
      console.log(value)
      const oldName = this.fullName
      const newName = value + this.nameExt
      const id = this.presentScenario.id

      const payload = {
        oldName,
        newName,
        id,
      }
      this.updateScenario({
        loading: true,
      })
      const result = await renameScenario(payload)
      if (result.code === 0) {
        await this.getScenarioList()
        this.updatePresentScenario({
          id,
          name: value,
        })
      } else {
        this.updateSceneName()
        this.$errorBox(result.message)
      }
      this.updateScenario({
        loading: false,
      })
    },

    validateName (value) {
      if (validateScenarioName(value)) {
        const matched = _.find(this.scenarioList, { name: value })
        if (matched) {
          this.$errorBox(this.$t('tips.sameSceneNameExists'))
          return false
        }
      } else {
        this.$errorBox(this.$t('tips.namePrefixErr', { name: this.$t('scenario.name') }))
        return false
      }
      return true
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .property-scenario {
    // .scenario-name {
    //   width: 100px;
    // }
    .scenario-name,
    .map-name {
      font-size: 12px;
      word-break: break-all;
    }
  }

  .generate-info {
    font-size: 12px;
    line-height: 24px;
    color: #c2c2c2;
    padding: 0;
    margin: 0;
  }

  .generate-source {
    .text-overflow;
  }

  .open-signal-control {
    cursor: pointer;
  }

  .kpi-group-list-select {
    width: 150px;
  }

  .open-kpi-group-management {
    margin-left: 10px;
    font-size: 16px;
    cursor: pointer;
  }
</style>

<style lang="less">
  .param-cell {
    .el-textarea {
      .el-input__count {
        background: rgba(0, 0, 0, 0);
        font-size: 8px;
        bottom: -20px;
        right: 0;
      }
    }
  }
</style>
