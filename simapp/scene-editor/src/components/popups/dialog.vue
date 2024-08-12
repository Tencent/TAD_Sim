<template>
  <el-dialog
    ref="dialog"
    :model-value="visible"
    :header="$t(title)"
    append-to-body
    :close-on-click-modal="false"
    :class="customClass"
    :before-close="close"
    @closed="closed"
  >
    <component
      :is="body"
      ref="content"
      v-bind="missionProps"
      @close="close"
    />
  </el-dialog>
</template>

<script>
import { mapMutations, mapState } from 'vuex'
import { invoke } from 'lodash-es'

import ScenarioManagement from '@/components/popups/scenario-management.vue'
import MapManagement from '@/components/popups/map-management.vue'
import CreateNewScenario from '@/components/popups/create-new-scene.vue'
import ModuleManagement from '@/components/popups/module-management.vue'
import ModuleGlobalManagement from '@/components/popups/module-global-management.vue'
import TrafficFlowConfig from '@/components/popups/traffic-flow-config.vue'
import KpiManagement from '@/components/popups/kpi-management.vue'
import KpiReportManagement from '@/components/popups/kpi-report-management.vue'
import FiniteStateMachine from '@/components/popups/finite-state-machine.vue'
import ScenarioGeneration from '@/components/popups/scenario-generation.vue'
import KpiIndicatorSetting from '@/components/popups/kpi-indicator-setting.vue'
import ImportFromTapd from '@/components/popups/import-from-tapd.vue'
import SaveAs from '@/components/popups/save-as.vue'
import SystemConfig from '@/components/popups/system-config.vue'
import AboutTadsim from '@/components/popups/about-tadsim.vue'
import PedestrianOrAnimalTriggerConfig from '@/components/popups/pedestrian-or-animal-trigger-condition-config.vue'
import EventConfig from '@/components/popups/event-config.vue'
import Environment from '@/components/environment/index.vue'
import PlannerConfig from '@/components/popups/planner-config/index.vue'
import Log2WorldSetting from '@/components/popups/log2world-setting.vue'
import SignlightConfig from '@/components/popups/signlight-config.vue'
import AddModel from '@/components/popups/model-manage/add-model.vue'

export default {
  name: 'SimDialog',
  components: {
    ScenarioManagement,
    MapManagement,
    CreateNewScenario,
    ModuleManagement,
    ModuleGlobalManagement,
    TrafficFlowConfig,
    KpiManagement,
    KpiReportManagement,
    FiniteStateMachine,
    ScenarioGeneration,
    KpiIndicatorSetting,
    ImportFromTapd,
    SaveAs,
    SystemConfig,
    AboutTadsim,
    PedestrianOrAnimalTriggerConfig,
    Environment,
    EventConfig,
    PlannerConfig,
    Log2WorldSetting,
    SignlightConfig,
    AddModel,
  },
  data () {
    return {
      bodyComponents: {
        ScenarioManagement: 'ScenarioManagement',
        MapManagement: 'MapManagement',
        CreateNewScenario: 'CreateNewScenario',
        ModuleManagement: 'ModuleManagement',
        ModuleGlobalManagement: 'ModuleGlobalManagement',
        TrafficFlowConfig: 'TrafficFlowConfig',
        KpiManagement: 'KpiManagement',
        KpiReportManagement: 'KpiReportManagement',
        FiniteStateMachine: 'FiniteStateMachine',
        ScenarioGeneration: 'ScenarioGeneration',
        KpiIndicatorSetting: 'KpiIndicatorSetting',
        ImportFromTapd: 'ImportFromTapd',
        SaveAs: 'SaveAs',
        SystemConfig: 'SystemConfig',
        AboutTadsim: 'AboutTadsim',
        PedestrianOrAnimalTriggerConfig: 'PedestrianOrAnimalTriggerConfig',
        Environment: 'Environment',
        EventConfig: 'EventConfig',
        PlannerConfig: 'PlannerConfig',
        Log2WorldSetting: 'Log2WorldSetting',
        SignlightConfig: 'SignlightConfig',
        AddModel: 'AddModel',
      },
      titles: {
        ScenarioManagement: 'menu.scenarioManagement',
        MapManagement: 'menu.mapManagement',
        CreateNewScenario: 'scenario.new',
        ModuleManagement: 'menu.moduleManagement',
        ModuleGlobalManagement: 'menu.moduleGlobalManagement',
        TrafficFlowConfig: 'menu.traffic',
        KpiManagement: 'menu.indicator',
        KpiReportManagement: 'menu.evaluation',
        FiniteStateMachine: 'menu.stateMachine',
        ScenarioGeneration: 'menu.sceneGeneration',
        KpiIndicatorSetting: 'menu.KPISetting',
        ImportFromTapd: 'TAPD import',
        SaveAs: 'menu.saveAs',
        SystemConfig: 'menu.system',
        AboutTadsim: 'menu.about',
        PedestrianOrAnimalTriggerConfig: 'menu.triggerConditionConfiguration',
        Environment: 'menu.environment',
        EventConfig: 'menu.eventConfig',
        PlannerConfig: 'menu.plannerConfig',
        // todo: 翻译
        Log2WorldSetting: '切换设置',
        SignlightConfig: 'menu.signalControlConfiguration',
        AddModel: 'operation.importModel',
      },
      body: null,
      title: '',
      customClass: '',
    }
  },
  computed: {
    ...mapState('mission', [
      'currentMission',
      'missionProps',
    ]),
    visible () {
      return !!this.currentMission
    },
  },
  watch: {
    currentMission (val) {
      if (val) {
        this.body = this.bodyComponents[val]
        this.title = this.titles[val]
        this.customClass = val
      }
    },
  },
  methods: {
    ...mapMutations('mission', [
      'updateState',
    ]),
    async close (done) {
      // 关闭时执行子组件某些方法，方便子组件处理关闭事件
      const result = await invoke(this.$refs.content, 'beforeClose', true)

      if (result !== false) {
        this.updateState({
          currentMission: null,
        })
        done && done()
        // this.body = null
        // this.title = ''
      }
    },
    closed () {
      this.body = null
      this.title = ''
    },
    closeFromChildren () {
      // todo: this.$refs.dialog.handleClose()
    },
  },
}
</script>

<style lang="less">
@import "@/assets/less/mixins";

.AboutTadsim {
  .el-dialog__header {
    background-color: @dark-bg;
  }
}

.el-dialog.SignlightConfig {
  width: 100%;
  margin-top: 0 !important;
  height: 100%;
  display: flex;
  flex-direction: column;

  & > .el-dialog__body {
    flex-grow: 1;
    overflow: hidden;
  }
}
</style>
