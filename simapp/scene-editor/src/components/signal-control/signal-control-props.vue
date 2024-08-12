<template>
  <SectionBox class="signal-control-props">
    <template #title>
      {{ $t('scenario.attribute') }}
    </template>
    <div class="signal-control-props-body">
      <h3 class="element-title">
        <span>{{ $t('scenario.crossingControlSetting') }}</span>
      </h3>
      <el-form
        ref="form"
        class="el-form-auto-width crossing-setting-form"
        :model="form"
        :rules="rules"
        :disabled="disabled"
        label-width="68px"
        @submit.prevent
      >
        <!-- 周期时长 -->
        <el-form-item
          prop="cycleTime"
          class="param-cell cycleTime"
          :label="$t('scenario.cycleTime')"
        >
          <InputNumber
            v-model="form.cycleTime"
            :min="3"
            :max="600"
            unit="s"
            @blur="changeValue('cycleTime', $event)"
          />
        </el-form-item>
        <!-- 状态 启用/禁用 -->
        <el-form-item
          prop="mapName"
          class="param-cell crossingStatus"
          :label="$t('scenario.crossingStatus')"
        >
          <el-switch
            v-model="form.crossingStatus"
            class="custom-switch"
            @change="changeValue('crossingStatus', $event)"
          />
        </el-form-item>
      </el-form>
      <h3 class="element-title">
        <span>{{ $t('scenario.signlightSetting') }}</span>
        <SignalControlPhaseActions :disabled="disabled" @command="handleCommand" />
      </h3>
      <div class="signal-control-props-wrap">
        <SignalControlPhaseList :disabled="disabled" />
      </div>
    </div>
  </SectionBox>
</template>

<script>
import { mapActions, mapGetters } from 'vuex'
import _ from 'lodash'
import SignalControlPhaseActions from './common/signal-control-phase-actions.vue'
import SignalControlPhaseList from './common/signal-control-phase-list.vue'
import SectionBox from '@/components/section-box.vue'
import InputNumber from '@/components/input-number.vue'
import { getPermission } from '@/utils/permission'

export default {
  components: {
    SectionBox,
    InputNumber,
    SignalControlPhaseActions,
    SignalControlPhaseList,
  },
  data () {
    return {
      form: {},
      rules: {},
    }
  },
  computed: {
    ...mapGetters('signalControl', [
      'currentJunction',
      'isSystemPlan',
    ]),
    disabled () {
      if (this.isSystemPlan) return true
      const canModify = getPermission('action.signlightControl.phase.settings.enable')
      if (!canModify) return true
      return false
    },
  },
  watch: {
    currentJunction (val) {
      this.form = _.cloneDeep(val)
    },
  },
  mounted () {
    this.form = _.cloneDeep(this.currentJunction)
  },
  methods: {
    ...mapActions('signalControl', [
      'changeJunctionValue',
      'updatePhaseList',
      'addPhase',
      'deletePhase',
      'addLightState',
      'deleteLightState',
    ]),
    changeValue (key) {
      const id = this.currentJunction.id
      const value = this.form[key]
      this.changeJunctionValue({ id, key, value })
    },
    handleCommand (command, args) {
      switch (command) {
        case 'addPhase': {
          this.addPhase()
          break
        }
        case 'delPhase': {
          this.deletePhase()
          break
        }
        // case 'move': {
        //   break
        // }
        case 'add': {
          this.addLightState(args)
          break
        }
        case 'delColor': {
          this.deleteLightState()
          break
        }
        default:
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.signal-control-props {
  flex: 1;
  min-width: 100px;
}

.signal-control-props-body {
  height: 100%;
  color: @title-font-color;
  flex: 1;
  overflow-x: hidden;
  overflow-y: auto;
  -webkit-overflow-scrolling: touch;
}

.signal-control-props-wrap {
  :deep(.el-collapse-item__header) {
    background-color: @hover-bg;
  }
}

.element-title {
  height: 32px;
  line-height: 32px;
  padding: 0 20px;
  font-weight: normal;
  font-size: 12px;
  color: #b9b9b9;
  border-bottom: 1px solid @border-color;
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.no-property {
  text-align: center;
  font-size: 12px;
  color: @disabled-color;
  line-height: 3em;
  user-select: none;
}

.crossing-setting-form {
  display: flex;
  flex-wrap: wrap;
  padding: 10px 0 10px 20px;

  & > .el-form-item {
    flex: 1 0 50%;
    box-sizing: border-box;
    padding-right: 20px;
    margin-bottom: 10px;
  }
}

.el-form.el-form-auto-width {
  :deep(.el-form-item__label) {
    width: auto !important;
  }
  :deep(.el-form-item__content) {
    margin-left: 10px !important;
  }
}

:deep(.el-switch.custom-switch) {
  .el-switch__core {
    background-color: #5e5e5e;
    color: #ffffff;
    border-color: transparent;

    &::after {
      background-color: #ffffff;
    }

    &::before {
      content: "禁用";
      font-size: 10px;
      position: absolute;
      top: -0.1em;
      right: 0.1em;
      transform: scale(0.8);
    }
  }

  &.is-checked .el-switch__core {
    background-color: #0491aa;
  }

  &.is-checked .el-switch__core::before {
    content: "启用";
    left: 0.1em;
    right: auto;
  }
}
</style>
