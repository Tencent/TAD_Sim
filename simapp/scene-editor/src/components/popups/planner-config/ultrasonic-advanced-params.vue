<template>
  <div class="ultrasonic-advanced-params">
    <NumInputItem
      :value="data.Frequency"
      :desc="ultrasonicAdvancedInfo.Frequency"
      @input="changeAdvancedInfo({ paramName: 'Frequency', value: $event })"
    />
    <NumInputItem
      :value="data.FOV_Horizontal"
      :desc="ultrasonicAdvancedInfo.FOV_Horizontal"
      @input="changeAdvancedInfo({ paramName: 'FOV_Horizontal', value: $event })"
    />
    <NumInputItem
      :value="data.FOV_Vertical"
      :desc="ultrasonicAdvancedInfo.FOV_Vertical"
      @input="changeAdvancedInfo({ paramName: 'FOV_Vertical', value: $event })"
    />
    <NumInputItem
      :value="data.dBmin"
      :desc="ultrasonicAdvancedInfo.dBmin"
      @input="changeAdvancedInfo({ paramName: 'dBmin', value: $event })"
    />
    <NumInputItem
      :value="data.Radius"
      :desc="ultrasonicAdvancedInfo.Radius"
      @input="changeAdvancedInfo({ paramName: 'Radius', value: $event })"
    />
    <NumInputItem
      :value="data.NoiseFactor"
      :desc="ultrasonicAdvancedInfo.NoiseFactor"
      @input="changeAdvancedInfo({ paramName: 'NoiseFactor', value: $event })"
    />
    <NumInputItem
      :value="data.NoiseStd"
      :desc="ultrasonicAdvancedInfo.NoiseStd"
      @input="changeAdvancedInfo({ paramName: 'NoiseStd', value: $event })"
    />
    <div class="param-cell">
      <label class="form-label">{{ $t(ultrasonicAdvancedInfo.AttachmentType.name) }}</label>
      <el-select
        :model-value="data.AttachmentType"
        @change="changeAdvancedInfo({ paramName: 'AttachmentType', value: $event })"
      >
        <el-option
          v-for="item of AttachmentTypeOptions"
          :key="item.value"
          :label="item.label"
          :value="item.value"
        />
      </el-select>
    </div>
    <template v-if="data.AttachmentType !== 'None'">
      <NumInputItem
        :value="data.AttachmentRange"
        :desc="ultrasonicAdvancedInfo.AttachmentRange"
        @input="changeAdvancedInfo({ paramName: 'AttachmentRange', value: $event })"
      />
    </template>
    <NumInputItem
      :value="data.Distance"
      :desc="ultrasonicAdvancedInfo.Distance"
      @input="changeAdvancedInfo({ paramName: 'Distance', value: $event })"
    />
    <div class="param-cell">
      <label class="form-label">{{ $t(ultrasonicAdvancedInfo.IndirectDistance.name) }}</label>
      <el-radio-group
        :model-value="data.IndirectDistance"
        @change="changeAdvancedInfo({ paramName: 'IndirectDistance', value: $event })"
      >
        <el-radio :label="0" :value="0">
          {{ $t('sensor.no') }}
        </el-radio>
        <el-radio :label="1" :value="1">
          {{ $t('sensor.yes') }}
        </el-radio>
      </el-radio-group>
    </div>
    <template v-if="data.IndirectDistance === 1">
      <NumInputItem
        :value="data.PulseMoment"
        :desc="ultrasonicAdvancedInfo.PulseMoment"
        @input="changeAdvancedInfo({ paramName: 'PulseMoment', value: $event })"
      />
      <NumInputItem
        :value="data.PulsePeriod"
        :desc="ultrasonicAdvancedInfo.PulsePeriod"
        @input="changeAdvancedInfo({ paramName: 'PulsePeriod', value: $event })"
      />
      <div class="param-cell">
        <label class="form-label">{{ $t(ultrasonicAdvancedInfo.PollTurn.name) }}</label>
        <el-input
          :model-value="data.PollTurn"
          style="width: 80px"
          maxlength="20"
          @input="changeAdvancedInfo({ paramName: 'PollTurn', value: $event })"
        />
      </div>
    </template>
  </div>
</template>

<script>
import NumInputItem from './num-input-item.vue'
import {
  AttachmentTypeOptions,
  LidarFrequencyOptions,
  ModelOptions,
  ultrasonicAdvancedInfo,
} from '@/store/modules/sensor/constants'

export default {
  name: 'UltrasonicAdvancedParams',
  components: {
    NumInputItem,
  },
  props: {
    currentSensor: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      AttachmentTypeOptions,
      ModelOptions,
      LidarFrequencyOptions,
      ultrasonicAdvancedInfo,
    }
  },
  computed: {
    data () {
      return this.currentSensor.param
    },
  },
  methods: {
    changeAdvancedInfo (params) {
      this.$emit('change-advanced-info', params)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .ultrasonic-advanced-params {
    padding: 0 22px;
  }

  .param-cell {
    display: flex;
    justify-content: space-between;
    padding: 10px 0;
    align-items: center;

    .el-radio {
      margin-right: 16px;

      &:last-child {
        margin-right: 0;
      }

      :deep(.el-radio__label) {
        padding-left: 5px;
      }
    }

    .param-input {
      box-sizing: border-box;
      width: 80px;
      height: 24px;

      :deep(.el-input__inner) {
        padding: 0 6px;
        line-height: 22px;
        height: 24px;
        border-radius: 2px;
      }

      &.el-input--suffix {
        :deep(.el-input__inner) {
          padding-right: 25px;
        }
      }
    }

    .el-select {
      width: 80px;

      :deep(.el-input--suffix) {
        .el-input__inner {
          height: 24px;
          line-height: 22px;
          border-radius: 2px;
          padding-left: 6px;
          color: @global-font-color;
          background-color: @darker-bg;
        }

        .el-input__icon {
          line-height: 24px;
        }
      }
    }

    &.multi-input {
      display: block;

      .form-label {
        display: block;
        margin-bottom: 15px;
      }
    }

    .form-multi-input {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-column-gap: 10px;

      .input-cell {
        display: flex;
        flex-direction: column;
        align-items: center;
        margin-bottom: 10px;

        :deep(.el-input__inner) {
          width: 60px;
          height: 24px;
          line-height: 22px;
          border-radius: 2px;
        }
      }
    }
  }
</style>
