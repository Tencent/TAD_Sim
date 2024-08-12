<template>
  <div class="sensor-params">
    <el-collapse v-model="activeTabs">
      <el-collapse-item name="base" :title="$t('scenario.basicInformation')" class="sensor-base">
        <div class="sensor-title">
          <h6 class="sensor-id">
            ID: {{ baseInfo.ID }}
          </h6>
          <el-select
            v-if="sensorInstallSlotVisible"
            :model-value="baseInfo.InstallSlot"
            @change="changeBaseInfo({ paramName: 'InstallSlot', value: $event })"
          >
            <el-option
              v-for="o of installSlotOptions"
              :key="o.key"
              :value="o.value"
              :label="o.label"
            />
          </el-select>
        </div>
        <div class="sensor-base-info">
          <h6 class="form-label">
            {{ $t('sensor.position') }}(cm)
          </h6>
          <h6 class="form-label">
            {{ $t('sensor.rotation') }}(°)
          </h6>
          <h6 v-if="gpuAndInstanceVisible" class="form-label">
            {{ $t('sensor.GPUSetting') }}
          </h6>
          <h6 v-if="gpuAndInstanceVisible" class="form-label">
            {{ $t('sensor.instanceID') }}
          </h6>
          <!-- <h6 class="form-label">{{ $t('sensor.algorithmModelId') }}</h6> -->
          <div class="base-info-cell">
            <InputNumber
              class="base-info-input"
              :model-value="baseInfo.LocationX"
              :min="-2000"
              :max="2000"
              @update:model-value="changeBaseInfo({ paramName: 'LocationX', value: $event })"
            />
            <span class="base-info-prop-name">X</span>
          </div>
          <div class="base-info-cell">
            <InputNumber
              class="base-info-input"
              :model-value="baseInfo.LocationY"
              :min="-2000"
              :max="2000"
              @update:model-value="changeBaseInfo({ paramName: 'LocationY', value: $event })"
            />
            <span class="base-info-prop-name">Y</span>
          </div>
          <div class="base-info-cell">
            <InputNumber
              class="base-info-input"
              :model-value="baseInfo.LocationZ"
              :min="0"
              :max="2000"
              @update:model-value="changeBaseInfo({ paramName: 'LocationZ', value: $event })"
            />
            <span class="base-info-prop-name">Z</span>
          </div>
          <div class="base-info-cell">
            <InputNumber
              class="base-info-input"
              :model-value="baseInfo.RotationX"
              :min="-180"
              :max="180"
              @update:model-value="changeBaseInfo({ paramName: 'RotationX', value: $event })"
            />
            <span class="base-info-prop-name">X</span>
          </div>
          <div class="base-info-cell">
            <InputNumber
              class="base-info-input"
              :model-value="baseInfo.RotationY"
              :min="-180"
              :max="180"
              @update:model-value="changeBaseInfo({ paramName: 'RotationY', value: $event })"
            />
            <span class="base-info-prop-name">Y</span>
          </div>
          <div class="base-info-cell">
            <InputNumber
              class="base-info-input"
              :model-value="baseInfo.RotationZ"
              :min="-180"
              :max="180"
              @update:model-value="changeBaseInfo({ paramName: 'RotationZ', value: $event })"
            />
            <span class="base-info-prop-name">Z</span>
          </div>
          <div v-if="gpuAndInstanceVisible" class="base-info-line">
            <el-select v-model="gpu">
              <el-option
                v-for="device of deviceOptions"
                :key="device.value"
                :value="device.value"
                :label="device.label"
              />
            </el-select>
          </div>
          <div v-if="gpuAndInstanceVisible" class="base-info-cell">
            <el-input
              v-model="instance"
              maxlength="10"
              @change="setValue($event, 'change')"
            />
          </div>
          <!-- <div class="base-info-line">
            <InputNumber :model-value="baseInfo.Device" />
          </div> -->
        </div>
      </el-collapse-item>
      <el-collapse-item name="advance" :title="$t(advancePropsTitle)">
        <component
          :is="sensorParamComponents[currentSensor.type]"
          :current-sensor="currentSensor"
          @change-advanced-info="changeAdvancedInfo"
        />
      </el-collapse-item>
    </el-collapse>
  </div>
</template>

<script>
import { mapGetters, mapMutations, mapState } from 'vuex'
import CameraAdvancedParams from './camera-advanced-params.vue'
import LidarAdvancedParams from './lidar-advanced-params.vue'
import RadarAdvancedParams from './radar-advanced-params.vue'
import TruthAdvanceParams from './truth-advanced-params.vue'
import ImuGpsAdvanceParams from './imu-gps-advanced-params.vue'
import UltrasonicAdvancedParams from './ultrasonic-advanced-params.vue'
import ObuAdvancedParams from './obu-advanced-params.vue'
import { deviceOptions } from '@/store/modules/sensor/constants'
import InputNumber from '@/components/input-number.vue'

const advancePropsTitles = {
  Camera: 'sensor.camera_param',
  Fisheye: 'sensor.fisheye_param',
  Semantic: 'sensor.semantic_param',
  Depth: 'sensor.depth_param',
  TraditionalLidar: 'sensor.lidar_param',
  Radar: 'sensor.radar_param',
  Truth: 'sensor.truth_param',
  IMU: 'sensor.imu_param',
  GPS: 'sensor.gps_param',
  Ultrasonic: 'sensor.ultrasonic_param',
  OBU: 'sensor.obu_param',
}

export default {
  name: 'SensorParams',
  components: {
    InputNumber,
    CameraAdvancedParams,
    LidarAdvancedParams,
    RadarAdvancedParams,
    TruthAdvanceParams,
    ImuGpsAdvanceParams,
    UltrasonicAdvancedParams,
    ObuAdvancedParams,
  },
  data () {
    return {
      activeTabs: ['base', 'advance'],
      deviceOptions,
      sensorParamComponents: {
        Camera: 'CameraAdvancedParams',
        Fisheye: 'CameraAdvancedParams',
        Semantic: 'CameraAdvancedParams',
        Depth: 'CameraAdvancedParams',
        TraditionalLidar: 'LidarAdvancedParams',
        Radar: 'RadarAdvancedParams',
        Truth: 'TruthAdvanceParams',
        IMU: 'ImuGpsAdvanceParams',
        GPS: 'ImuGpsAdvanceParams',
        Ultrasonic: 'UltrasonicAdvancedParams',
        OBU: 'ObuAdvancedParams',
      },
    }
  },
  computed: {
    ...mapGetters('planners', [
      'currentSensor',
    ]),
    ...mapState('planners', [
      'currentPlanner',
    ]),
    baseInfo () {
      return this.currentSensor
    },
    advancePropsTitle () {
      return advancePropsTitles[this.currentSensor.type] || ''
    },
    gpu: {
      get () {
        return this.baseInfo.Device.split('.')[0]
      },
      set (value) {
        this.changeBaseInfo({ paramName: 'Device', value: `${value}.${this.instance}` })
      },
    },
    instance: {
      get () {
        return this.baseInfo.Device.split('.')[1] || ''
      },
      set (value) {
        this.changeBaseInfo({ paramName: 'Device', value: `${this.gpu}.${value}` })
      },
    },
    gpuAndInstanceVisible () {
      return this.currentSensor.type !== 'IMU' && this.currentSensor.type !== 'GPS'
    },
    sensorInstallSlotVisible () {
      return this.currentPlanner.catalogSubCategory === 'combination'
    },
    installSlotOptions () {
      const catalogParams = this.currentPlanner.catalogParams.slice()
      if (catalogParams.length > 1 && this.currentPlanner.catalogSubCategory === 'combination') {
        catalogParams.shift()
      }
      return catalogParams.map((p, index) => {
        return {
          key: `C${index}`,
          value: `C${index}`,
          label: index === 0 ? this.$t('scenario.tractor') : `${this.$t('scenario.trailer')}${index}`,
        }
      })
    },
  },
  methods: {
    ...mapMutations('planners', [
      'changeBaseInfo',
      'changeAdvancedInfo',
    ]),
    setValue ($event) {
      const val = $event.trim().replace(/\W/g, '') || '0'
      this.changeBaseInfo({ paramName: 'Device', value: `${this.gpu}.${val}` })
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .sensor-params {
    box-sizing: border-box;

    .el-collapse {
      border-bottom: none;
      border-top: none;

      :deep(.el-collapse-item:last-child) {
        margin-bottom: -1px;
      }
    }

    :deep(.el-collapse-item__header) {
      height: 28px;
      line-height: 28px;
      padding: 0 0 0 20px;
    }

    .sensor-base {
      :deep(.el-collapse-item__content) {
        padding: 15px 22px;
      }

      .sensor-title {
        display: flex;
        justify-content: space-between;

        .sensor-id {
          font-size: 12px;
          font-weight: normal;
          margin-bottom: 10px;
        }
      }
    }

    .sensor-base-info {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-column-gap: 12px;

      .form-label {
        font-size: 12px;
        font-weight: normal;
        margin-bottom: 10px;
        grid-column-start: 1;
        grid-column-end: 4;

        &:nth-child(1) {
          grid-row: 1;
        }

        &:nth-child(2) {
          grid-row: 3;
        }

        &:nth-child(3) {
          grid-row: 5;
        }

        &:nth-child(4) {
          grid-row: 7;
        }

        &:nth-child(5) {
          grid-row: 9;
        }
      }

      .base-info-cell {
        display: flex;
        flex-direction: column;
        align-items: center;
        margin-bottom: 10px;
      }

      .base-info-line {
        grid-column-start: 1;
        grid-column-end: 4;
        margin-bottom: 10px;
      }

      .base-info-input {
        width: auto;
      }
    }
  }
</style>
