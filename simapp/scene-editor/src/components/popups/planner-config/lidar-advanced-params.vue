<template>
  <div class="lidar-advanced-params">
    <div class="param-cell">
      <label class="form-label">{{ $t(lidarAdvancedInfo.Model.name) }}</label>
      <el-select
        :model-value="data.Model"
        style="width: 100px;"
        @change="changeAdvancedInfo({ paramName: 'Model', value: $event })"
      >
        <el-option
          v-for="item of ModelOptions"
          :key="item.value"
          :label="item.label"
          :value="item.value"
        />
      </el-select>
    </div>
    <NumInputItem
      v-if="data.Model === 'User'"
      :value="data.Frequency"
      :desc="lidarAdvancedInfo.Frequency"
      @input="changeAdvancedInfo({ paramName: 'Frequency', value: $event })"
    />
    <div v-else class="param-cell">
      <label class="form-label">{{ $t(lidarAdvancedInfo.Frequency.name) }}</label>
      <el-select
        :model-value="data.Frequency"
        style="width: 100px;"
        @change="changeAdvancedInfo({ paramName: 'Frequency', value: $event })"
      >
        <el-option
          v-for="item of LidarFrequencyOptions"
          :key="item.value"
          :label="item.label"
          :value="item.value"
        />
      </el-select>
    </div>
    <!-- <div class="param-cell">
      <label class="form-label">{{ $t(lidarAdvancedInfo.DrawPoint.name) }}</label>
      <el-radio-group
        :model-value="data.DrawPoint"
        @change="changeAdvancedInfo({paramName: 'DrawPoint', value: $event})"
      >
        <el-radio :label="true" :value="true">{{ $t('show') }}</el-radio>
        <el-radio :label="false" :value="false">{{ $t('hide') }}</el-radio>
      </el-radio-group>
    </div> -->
    <div class="param-cell">
      <label class="form-label">{{ $t(lidarAdvancedInfo.IP.name) }}</label>
      <el-input
        :model-value="data.IP"
        style="width: 80px"
        @input="changeAdvancedInfo({ paramName: 'IP', value: $event })"
      />
    </div>
    <PortInput
      :value="data.Port"
      :desc="lidarAdvancedInfo.Port"
      @input="changeAdvancedInfo({ paramName: 'Port', value: $event })"
    />
    <NumInputItem
      :value="data.Attenuation"
      :desc="lidarAdvancedInfo.Attenuation"
      @input="changeAdvancedInfo({ paramName: 'Attenuation', value: $event })"
    />
    <div class="param-cell">
      <label class="form-label">{{ $t('sensor.AngleDefinition') }}</label>
      <InputFileBrowser
        :model-value="data.AngleDefinition"
        :title="$t('tips.selectPlaybackFile')"
        :properties="['openFile']"
        @change="changeAdvancedInfo({ paramName: 'AngleDefinition', value: $event })"
      />
    </div>
    <NumInputItem
      :value="data.ExtinctionCoe"
      :desc="lidarAdvancedInfo.ExtinctionCoe"
      @input="changeAdvancedInfo({ paramName: 'ExtinctionCoe', value: $event })"
    />
    <template v-if="data.Model === 'User'">
      <NumInputItem
        :value="data.uChannels"
        :desc="lidarAdvancedInfo.uChannels"
        @input="changeAdvancedInfo({ paramName: 'uChannels', value: $event })"
      />
      <NumInputItem
        :value="data.uRange"
        :desc="lidarAdvancedInfo.uRange"
        @input="changeAdvancedInfo({ paramName: 'uRange', value: $event })"
      />
      <div class="param-cell">
        <label class="form-label">{{ $t(lidarAdvancedInfo.ReflectionType.name) }}</label>
        <el-select
          :model-value="data.ReflectionType"
          style="width: 100px;"
          @change="changeAdvancedInfo({ paramName: 'ReflectionType', value: $event })"
        >
          <el-option
            v-for="item of ReflectionTypeOption"
            :key="item.value"
            :label="$t(item.label)"
            :value="item.value"
          />
        </el-select>
      </div>
      <NumInputItem
        :value="data.uHorizontalResolution"
        :desc="lidarAdvancedInfo.uHorizontalResolution"
        @input="changeAdvancedInfo({ paramName: 'uHorizontalResolution', value: $event })"
      />
      <NumInputItem
        :value="data.uUpperFov"
        :desc="lidarAdvancedInfo.uUpperFov"
        @input="changeAdvancedInfo({ paramName: 'uUpperFov', value: $event })"
      />
      <NumInputItem
        :value="data.uLowerFov"
        :desc="lidarAdvancedInfo.uLowerFov"
        @input="changeAdvancedInfo({ paramName: 'uLowerFov', value: $event })"
      />
      <NumInputItem
        :value="data.FovStart"
        :desc="lidarAdvancedInfo.FovStart"
        @input="changeAdvancedInfo({ paramName: 'FovStart', value: $event })"
      />
      <NumInputItem
        :value="data.FovEnd"
        :desc="lidarAdvancedInfo.FovEnd"
        @input="changeAdvancedInfo({ paramName: 'FovEnd', value: $event })"
      />
    </template>
  </div>
</template>

<script>
import NumInputItem from './num-input-item.vue'
import PortInput from './port-input.vue'
import { LidarFrequencyOptions, ModelOptions, ReflectionTypeOption, lidarAdvancedInfo } from '@/store/modules/sensor/constants'
import InputFileBrowser from '@/components/input-file-browser.vue'

export default {
  name: 'LidarAdvancedParams',
  components: {
    InputFileBrowser,
    NumInputItem,
    PortInput,
  },
  props: {
    currentSensor: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      ModelOptions,
      LidarFrequencyOptions,
      lidarAdvancedInfo,
      ReflectionTypeOption,
      fileList: [],
    }
  },
  computed: {
    data () {
      return this.currentSensor.param
    },
  },
  methods: {
    fileRemove () {
      this.fileList = []
      this.changeAdvancedInfo({ paramName: 'AngleDefinition', value: '' })
    },
    fileChange (file) {
      this.fileList = [file]
      this.readFile(file.raw, (txt) => {
        this.changeAdvancedInfo({ paramName: 'AngleDefinition', value: txt })
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

  .lidar-advanced-params {
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
