<template>
  <div class="camera-advanced-params">
    <el-form :rules="cameraRules" :model="data">
      <NumInputItem
        :value="data.Frequency"
        :desc="cameraAdvancedInfo.Frequency"
        @input="changeAdvancedInfo({ paramName: 'Frequency', value: $event })"
      />
      <NumInputItem
        :value="data.Res_Horizontal"
        :desc="cameraAdvancedInfo.Res_Horizontal"
        @input="changeAdvancedInfo({ paramName: 'Res_Horizontal', value: $event })"
      />
      <NumInputItem
        :value="data.Res_Vertical"
        :desc="cameraAdvancedInfo.Res_Vertical"
        @input="changeAdvancedInfo({ paramName: 'Res_Vertical', value: $event })"
      />
      <template v-if="!isSemanticOrDepth">
        <NumInputItem
          :value="data.Exquisite"
          :desc="cameraAdvancedInfo.Exquisite"
          @input="changeAdvancedInfo({ paramName: 'Exquisite', value: $event })"
        />
        <div v-if="showDisplayMode" class="param-cell">
          <label class="form-label">{{ $t(cameraAdvancedInfo.DisplayMode.name) }}</label>
          <el-radio-group
            :model-value="data.DisplayMode"
            @change="changeAdvancedInfo({ paramName: 'DisplayMode', value: $event })"
          >
            <el-radio :label="0" :value="0">
              {{ $t('sensor.color') }}
            </el-radio>
            <el-radio :label="1" :value="1">
              {{ $t('sensor.grayscale') }}
            </el-radio>
          </el-radio-group>
        </div>
        <NumInputItem
          v-if="showBlurIntensity"
          :value="data.Blur_Intensity"
          :desc="cameraAdvancedInfo.Blur_Intensity"
          @input="changeAdvancedInfo({ paramName: 'Blur_Intensity', value: $event })"
        />
        <NumInputItem
          v-if="showMotionBlurAmount"
          :value="data.MotionBlur_Amount"
          :desc="cameraAdvancedInfo.MotionBlur_Amount"
          @input="changeAdvancedInfo({ paramName: 'MotionBlur_Amount', value: $event })"
        />
        <NumInputItem
          :value="data.LensFlares"
          :desc="cameraAdvancedInfo.LensFlares"
          @input="changeAdvancedInfo({ paramName: 'LensFlares', value: $event })"
        />
        <NumInputItem
          v-if="showNoiseIntensity"
          :value="data.Noise_Intensity"
          :desc="cameraAdvancedInfo.Noise_Intensity"
          @input="changeAdvancedInfo({ paramName: 'Noise_Intensity', value: $event })"
        />
        <NumInputItem
          v-if="showVignetteIntensity"
          :value="data.Vignette_Intensity"
          :desc="cameraAdvancedInfo.Vignette_Intensity"
          @input="changeAdvancedInfo({ paramName: 'Vignette_Intensity', value: $event })"
        />
        <NumInputItem
          :value="data.Bloom"
          :desc="cameraAdvancedInfo.Bloom"
          @input="changeAdvancedInfo({ paramName: 'Bloom', value: $event })"
        />
        <div class="param-cell">
          <label class="form-label">{{ $t(cameraAdvancedInfo.ExposureMode.name) }}</label>
          <el-radio-group
            :model-value="data.ExposureMode"
            @change="changeAdvancedInfo({ paramName: 'ExposureMode', value: $event })"
          >
            <el-radio :label="0" :value="0">
              {{ $t('sensor.automatic') }}
            </el-radio>
            <el-radio :label="1" :value="1">
              {{ $t('sensor.manual') }}
            </el-radio>
          </el-radio-group>
        </div>
        <template v-if="data.ExposureMode === 1">
          <NumInputItem
            :value="data.ShutterSpeed"
            :desc="cameraAdvancedInfo.ShutterSpeed"
            @input="changeAdvancedInfo({ paramName: 'ShutterSpeed', value: $event })"
          />
          <NumInputItem
            :value="data.ISO"
            :desc="cameraAdvancedInfo.ISO"
            @input="changeAdvancedInfo({ paramName: 'ISO', value: $event })"
          />
          <NumInputItem
            :value="data.Aperture"
            :desc="cameraAdvancedInfo.Aperture"
            @input="changeAdvancedInfo({ paramName: 'Aperture', value: $event })"
          />
        </template>
        <NumInputItem
          :value="data.Compensation"
          :desc="cameraAdvancedInfo.Compensation"
          @input="changeAdvancedInfo({ paramName: 'Compensation', value: $event })"
        />
        <NumInputItem
          :value="data.ColorTemperature"
          :desc="cameraAdvancedInfo.ColorTemperature"
          @input="changeAdvancedInfo({ paramName: 'ColorTemperature', value: $event })"
        />
        <NumInputItem
          :value="data.WhiteHint"
          :desc="cameraAdvancedInfo.WhiteHint"
          @input="changeAdvancedInfo({ paramName: 'WhiteHint', value: $event })"
        />
        <NumInputItem
          :value="data.Transmittance"
          :desc="cameraAdvancedInfo.Transmittance"
          @input="changeAdvancedInfo({ paramName: 'Transmittance', value: $event })"
        />
      </template>
      <div v-if="showIntrinsicParamType" class="param-cell param-multi-line">
        <label class="form-label">{{ $t(cameraAdvancedInfo.InsideParamGroup.name) }}</label>
        <el-radio-group
          :model-value="data.IntrinsicParamType"
          @change="changeAdvancedInfo({ paramName: 'IntrinsicParamType', value: $event })"
        >
          <el-radio :label="0" :value="0">
            {{ $t('sensor.Matrix') }}
          </el-radio>
          <el-radio :label="1" :value="1">
            FOV
          </el-radio>
          <el-radio :label="2" :value="2">
            {{ $t('sensor.Photoreceptor') }}
          </el-radio>
        </el-radio-group>
      </div>
      <div v-if="data.IntrinsicParamType === 0 || currentSensor.type === 'Fisheye'" class="multi-input-wrap">
        <div class="param-cell multi-input">
          <label class="form-label">{{ $t(cameraAdvancedInfo.Intrinsic_Matrix.name) }}</label>
          <div class="form-multi-input">
            <div v-for="(prop, index) of IntrinsicMatrixProps" :key="index" class="input-cell">
              <el-form-item :prop="`Intrinsic_Matrix.${index}`" class="form-multi-input-item">
                <InputNumber
                  :style="{ width: '56px' }"
                  :model-value="data.Intrinsic_Matrix[index]"
                  :placeholder="prop.name"
                  :readonly="prop.readonly"
                  @update:model-value="changeMultiValueParam('Intrinsic_Matrix', index, $event)"
                />
              </el-form-item>
            </div>
          </div>
        </div>
        <div class="param-cell multi-input">
          <label class="form-label">{{ $t(cameraAdvancedInfo.Distortion_Parameters.name) }}</label>
          <div v-if="currentSensor.type === 'Fisheye'" class="form-multi-input">
            <div
              v-for="(p, index) of FisheyeDistortionParameterNames"
              :key="p"
              class="input-cell"
            >
              <InputNumber
                :style="{ width: '56px' }"
                :model-value="data.Distortion_Parameters[index]"
                @update:model-value="changeMultiValueParam('Distortion_Parameters', index, $event)"
              />
              <span>{{ p }}</span>
            </div>
          </div>
          <div v-else class="form-multi-input">
            <div
              v-for="(p, index) of DistortionParameterNames"
              :key="p"
              class="input-cell"
            >
              <InputNumber
                :style="{ width: '56px' }"
                :model-value="data.Distortion_Parameters[index]"
                @update:model-value="changeMultiValueParam('Distortion_Parameters', index, $event)"
              />
              <span>{{ p }}</span>
            </div>
          </div>
        </div>
      </div>
      <div v-if="data.IntrinsicParamType === 1" class="multi-input-wrap">
        <NumInputItem
          :value="data.FOV_Horizontal"
          :desc="cameraAdvancedInfo.FOV_Horizontal"
          @input="changeAdvancedInfo({ paramName: 'FOV_Horizontal', value: $event })"
        />
        <NumInputItem
          :value="data.FOV_Vertical"
          :desc="cameraAdvancedInfo.FOV_Vertical"
          @input="changeAdvancedInfo({ paramName: 'FOV_Vertical', value: $event })"
        />
      </div>
      <div v-if="data.IntrinsicParamType === 2" class="multi-input-wrap">
        <NumInputItem
          :value="data.CCD_Width"
          :desc="cameraAdvancedInfo.CCD_Width"
          @input="changeAdvancedInfo({ paramName: 'CCD_Width', value: $event })"
        />
        <NumInputItem
          :value="data.CCD_Height"
          :desc="cameraAdvancedInfo.CCD_Height"
          @input="changeAdvancedInfo({ paramName: 'CCD_Height', value: $event })"
        />
        <NumInputItem
          :value="data.Focal_Length"
          :desc="cameraAdvancedInfo.Focal_Length"
          @input="changeAdvancedInfo({ paramName: 'Focal_Length', value: $event })"
        />
      </div>
    </el-form>
  </div>
</template>

<script>
import NumInputItem from './num-input-item.vue'
import InputNumber from '@/components/input-number.vue'
import {
  DistortionParameterNames,
  FisheyeDistortionParameterNames,
  IntrinsicMatrixProps,
  NoiseIntensityOptions,
  cameraAdvancedInfo,
} from '@/store/modules/sensor/constants'

export default {
  name: 'CameraAdvancedParams',
  components: {
    NumInputItem,
    InputNumber,
  },
  props: {
    currentSensor: {
      type: Object,
      required: true,
    },
  },
  data () {
    const intrinsicMatrixRules = {}
    const intrinsicMatrixRule = {
      required: true,
      trigger: 'blur',
    }
    for (let i = 0; i < 10; i++) {
      intrinsicMatrixRules[`Intrinsic_Matrix.${i}`] = intrinsicMatrixRule
    }

    return {
      cameraAdvancedInfo,
      NoiseIntensityOptions,
      DistortionParameterNames,
      FisheyeDistortionParameterNames,
      IntrinsicMatrixProps,
      cameraRules: {
        ...intrinsicMatrixRules,
      },
    }
  },
  computed: {
    data () {
      return this.currentSensor.param
    },
    showDisplayMode () {
      const { type } = this.currentSensor
      return type !== 'Semantic' && type !== 'Depth'
    },
    showBlurIntensity () {
      return this.currentSensor.type !== 'Depth'
    },
    showMotionBlurAmount () {
      return this.currentSensor.type !== 'Depth'
    },
    showVignetteIntensity () {
      return this.currentSensor.type !== 'Depth'
    },
    showNoiseIntensity () {
      return this.currentSensor.type !== 'Depth'
    },
    showIntrinsicParamType () {
      return this.currentSensor.type !== 'Fisheye'
    },
    isSemanticOrDepth () {
      return ['Semantic', 'Depth'].includes(this.currentSensor.type)
    },
  },
  methods: {
    changeMultiValueParam (name, index, value) {
      const paramValue = this.data[name].slice()
      paramValue[index] = value
      this.changeAdvancedInfo({ paramName: name, value: paramValue })
    },
    changeAdvancedInfo (params) {
      this.$emit('change-advanced-info', params)
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.camera-advanced-params {
  padding: 0 20px;
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

.multi-input-wrap {
  height: 370px;

  .multi-input .input-cell {
    margin-bottom: 0;
  }
}

.form-multi-input-item {
  margin-bottom: 0;

  :deep(.el-form-item__error) {
    display: none;
  }
}
</style>
