<template>
  <div class="obu-advanced-params">
    <NumInputItem
      :value="data.FrequencyBSM"
      :desc="OBUAdvancedInfo.FrequencyBSM"
      @input="changeAdvancedInfo({ paramName: 'FrequencyBSM', value: $event })"
    />
    <NumInputItem
      :value="data.DistanceOpen"
      :desc="OBUAdvancedInfo.DistanceOpen"
      @input="changeAdvancedInfo({ paramName: 'DistanceOpen', value: $event })"
    />
    <NumInputItem
      :value="data.DistanceCity"
      :desc="OBUAdvancedInfo.DistanceCity"
      @input="changeAdvancedInfo({ paramName: 'DistanceCity', value: $event })"
    />
    <div class="param-cell">
      <label>{{ $t(OBUAdvancedInfo.Band.name) }}</label>
      <el-select
        :style="{ width: '115px' }"
        :model-value="data.Band"
        @change="changeAdvancedInfo({ paramName: 'Band', value: $event })"
      >
        <el-option
          v-for="item of OBUBandOptions"
          :key="item.value"
          :label="$t(item.label)"
          :value="item.value"
        />
      </el-select>
    </div>
    <NumInputItem
      :value="data.Mbps"
      :desc="OBUAdvancedInfo.Mbps"
      @input="changeAdvancedInfo({ paramName: 'Mbps', value: $event })"
    />
    <NumInputItem
      :value="data.SystemDelay"
      :desc="OBUAdvancedInfo.SystemDelay"
      @input="changeAdvancedInfo({ paramName: 'SystemDelay', value: $event })"
    />
    <NumInputItem
      :value="data.CommuDelay"
      :desc="OBUAdvancedInfo.CommuDelay"
      @input="changeAdvancedInfo({ paramName: 'CommuDelay', value: $event })"
    />
    <div class="param-cell">
      <label class="form-label">{{ $t(OBUAdvancedInfo.TriggerImmediately.name) }}</label>
      <el-radio-group
        :model-value="data.TriggerImmediately"
        @change="changeAdvancedInfo({ paramName: 'TriggerImmediately', value: $event })"
      >
        <el-radio label="False" value="False">
          {{ $t('no') }}
        </el-radio>
        <el-radio label="True" value="True">
          {{ $t('yes') }}
        </el-radio>
      </el-radio-group>
    </div>
    <NumInputItem
      :value="data.PosAccuracy"
      :desc="OBUAdvancedInfo.PosAccuracy"
      @input="changeAdvancedInfo({ paramName: 'PosAccuracy', value: $event })"
    />
    <div class="param-cell">
      <label class="form-label">{{ $t(OBUAdvancedInfo.NoTeam.name) }}</label>
      <el-radio-group
        :model-value="data.NoTeam"
        @change="changeAdvancedInfo({ paramName: 'NoTeam', value: $event })"
      >
        <el-radio label="False" value="False">
          {{ $t('no') }}
        </el-radio>
        <el-radio label="True" value="True">
          {{ $t('yes') }}
        </el-radio>
      </el-radio-group>
    </div>
    <div class="param-cell">
      <label class="form-label">{{ $t(OBUAdvancedInfo.DisableRSU.name) }}</label>
      <el-radio-group
        :model-value="data.DisableRSU"
        @change="changeAdvancedInfo({ paramName: 'DisableRSU', value: $event })"
      >
        <el-radio label="False" value="False">
          {{ $t('no') }}
        </el-radio>
        <el-radio label="True" value="True">
          {{ $t('yes') }}
        </el-radio>
      </el-radio-group>
    </div>
    <div class="param-cell">
      <label class="form-label">{{ $t('sensor.PreBSM') }}</label>
      <InputFileBrowser
        :model-value="data.PreBSM"
        :title="$t('tips.selectPlaybackFile')"
        :properties="['openFile']"
        @change="changeAdvancedInfo({ paramName: 'PreBSM', value: $event })"
      />
    </div>
  </div>
</template>

<script>
import NumInputItem from './num-input-item.vue'
import {
  OBUAdvancedInfo,
  OBUBandOptions,
  v2xDelayTypeOptions,
} from '@/store/modules/sensor/constants'
import InputFileBrowser from '@/components/input-file-browser.vue'

export default {
  name: 'ObuAdvancedParams',
  components: {
    InputFileBrowser,
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
      v2xDelayTypeOptions,
      OBUAdvancedInfo,
      OBUBandOptions,
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

.obu-advanced-params {
  padding: 0 22px;
}

.param-cell {
  display: flex;
  justify-content: space-between;
  padding: 10px 0;
  align-items: center;

  .el-select {
    width: 80px;

    :deep(.el-input--suffix) {
      .el-input__inner {
        height: 24px;
        line-height: 22px;
        border-radius: 2px;
        padding-left: 6px;
      }

      .el-input__icon {
        line-height: 24px;
      }
    }
  }

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
      padding: 0 6px;
    }
  }

  .form-range-input {
    width: 100px;

    .input-cell {
      flex-direction: row;
      justify-content: space-between;
    }

    :deep(.el-input__prefix),
    :deep(.el-input__suffix) {
      color: @disabled-color;
    }
  }
}
</style>
