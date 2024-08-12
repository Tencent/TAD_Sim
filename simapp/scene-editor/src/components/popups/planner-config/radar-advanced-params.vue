<template>
  <div class="radar-advanced-params">
    <NumInputItem
      :value="data.Frequency"
      :desc="radarAdvancedInfo.Frequency"
      @input="changeAdvancedInfo({ paramName: 'Frequency', value: $event })"
    />
    <div class="param-cell">
      <label class="form-label">{{ $t(radarAdvancedInfo.F0_GHz.name) }}</label>
      <el-select
        :model-value="data.F0_GHz"
        @change="changeAdvancedInfo({ paramName: 'F0_GHz', value: $event })"
      >
        <el-option
          v-for="item of radarF0Options"
          :key="item.value"
          :label="item.label"
          :value="item.value"
        />
      </el-select>
    </div>
    <NumInputItem
      :value="data.Pt_dBm"
      :desc="radarAdvancedInfo.Pt_dBm"
      @input="changeAdvancedInfo({ paramName: 'Pt_dBm', value: $event })"
    />
    <NumInputItem
      :value="data.Gt_dBi"
      :desc="radarAdvancedInfo.Gt_dBi"
      @input="changeAdvancedInfo({ paramName: 'Gt_dBi', value: $event })"
    />
    <NumInputItem
      :value="data.Gr_dBi"
      :desc="radarAdvancedInfo.Gr_dBi"
      @input="changeAdvancedInfo({ paramName: 'Gr_dBi', value: $event })"
    />
    <NumInputItem
      :value="data.Ts_K"
      :desc="radarAdvancedInfo.Ts_K"
      @input="changeAdvancedInfo({ paramName: 'Ts_K', value: $event })"
    />
    <NumInputItem
      :value="data.Fn_dB"
      :desc="radarAdvancedInfo.Fn_dB"
      @input="changeAdvancedInfo({ paramName: 'Fn_dB', value: $event })"
    />
    <NumInputItem
      :value="data.L0_dB"
      :desc="radarAdvancedInfo.L0_dB"
      @input="changeAdvancedInfo({ paramName: 'L0_dB', value: $event })"
    />
    <NumInputItem
      :value="data.SNR_min_dB"
      :desc="radarAdvancedInfo.SNR_min_dB"
      @input="changeAdvancedInfo({ paramName: 'SNR_min_dB', value: $event })"
    />
    <NumInputItem
      :value="data.delay"
      :desc="radarAdvancedInfo.delay"
      @input="changeAdvancedInfo({ paramName: 'delay', value: $event })"
    />
    <NumInputItem
      :value="data.vfov"
      :desc="radarAdvancedInfo.vfov"
      @input="changeAdvancedInfo({ paramName: 'vfov', value: $event })"
    />
    <NumInputItem
      :value="data.hfov"
      :desc="radarAdvancedInfo.hfov"
      @input="changeAdvancedInfo({ paramName: 'hfov', value: $event })"
    />
    <NumInputItem
      :value="data.hwidth"
      :desc="radarAdvancedInfo.hwidth"
      @input="changeAdvancedInfo({ paramName: 'hwidth', value: $event })"
    />
    <NumInputItem
      :value="data.vwidth"
      :desc="radarAdvancedInfo.vwidth"
      @input="changeAdvancedInfo({ paramName: 'vwidth', value: $event })"
    />
    <div class="param-cell">
      <label class="form-label">{{ $t('sensor.CustomFault') }}</label>
      <InputFileBrowser
        :model-value="data.PreBSM"
        :title="$t('tips.selectPlaybackFile')"
        :properties="['openFile']"
        @change="changeAdvancedInfo({ paramName: 'CustomFault', value: $event })"
      />
    </div>
  </div>
</template>

<script>
import NumInputItem from './num-input-item.vue'
import {
  LidarFrequencyOptions,
  ModelOptions,
  radarAdvancedInfo,
  radarF0Options,
  weathers,
} from '@/store/modules/sensor/constants'
import InputFileBrowser from '@/components/input-file-browser.vue'

export default {
  name: 'RadarAdvancedParams',
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
      radarAdvancedInfo,
      ModelOptions,
      LidarFrequencyOptions,
      weathers,
      radarF0Options,
      fileList: [],
    }
  },
  computed: {
    data () {
      return this.currentSensor.param
    },
  },
  methods: {
    selectFileHandler1 (file) {
      this.changeAdvancedInfo({ paramName: 'ANTENNA_ANGLE_path1', value: file.raw.path })
    },
    selectFileHandler2 (file) {
      this.changeAdvancedInfo({ paramName: 'ANTENNA_ANGLE_path2', value: file.raw.path })
    },
    fileRemove () {
      this.fileList = []
      this.changeAdvancedInfo({ paramName: 'CustomFault', value: '' })
    },
    fileChange (file) {
      this.fileList = [file]
      this.readFile(file.raw, (txt) => {
        this.changeAdvancedInfo({ paramName: 'CustomFault', value: txt })
      })
    },
    readFile (file, cb) {
      const reader = new FileReader()
      reader.onload = function (res) {
        cb && cb(res.target.result)
      }
      reader.readAsText(file)
    },
    changeAdvancedInfo (params) {
      this.$emit('change-advanced-info', params)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .radar-advanced-params {
    padding: 0 22px;
  }

  .param-cell {
    display: flex;
    justify-content: space-between;
    padding: 10px 0;
    align-items: center;

    .sim-radio {
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

    .form-file-name-input {
      width: 180px;
      height: 24px;

      :deep(.el-input__inner) {
        padding: 0 6px;
        line-height: 22px;
        height: 24px;
        border-radius: 2px;
      }
    }

    .sim-select {
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

    &.param-multi-line {
      display: block;

      .form-label {
        display: block;
        margin-bottom: 8px;
      }
    }
  }
</style>
