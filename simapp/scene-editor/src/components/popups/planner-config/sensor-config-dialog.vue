<template>
  <el-dialog
    :model-value="visible"
    append-to-body
    :close-on-click-modal="false"
    :close-on-press-escape="false"
    :header="title"
    @close="visible = false"
  >
    <el-form
      ref="form"
      class="sensor-config-dialog"
      :model="item"
      :rules="rules"
    >
      <el-form-item class="sensor-name" :label="$t('name')" prop="name">
        <el-input
          v-model="item.name"
          style="width: 120px"
        />
      </el-form-item>
      <component
        :is="sensorParamComponents[item.type]"
        :current-sensor="item"
        @change-advanced-info="changeAdvancedInfo"
      />
    </el-form>
    <template #footer>
      <div class="sensor-config-dialog-btns">
        <el-button @click="cancel">
          取消
        </el-button>
        <el-button @click="save">
          保存
        </el-button>
      </div>
    </template>
  </el-dialog>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import CameraAdvancedParams from './camera-advanced-params.vue'
import LidarAdvancedParams from './lidar-advanced-params.vue'
import RadarAdvancedParams from './radar-advanced-params.vue'
import TruthAdvanceParams from './truth-advanced-params.vue'
import ImuGpsAdvanceParams from './imu-gps-advanced-params.vue'
import UltrasonicAdvancedParams from './ultrasonic-advanced-params.vue'
import ObuAdvancedParams from './obu-advanced-params.vue'
import { deviceOptions, nameMap } from '@/store/modules/sensor/constants'
import { changeSensorParam } from '@/store/modules/sensor/factory'

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
  name: 'SensorConfigDialog',
  components: {
    CameraAdvancedParams,
    LidarAdvancedParams,
    RadarAdvancedParams,
    TruthAdvanceParams,
    ImuGpsAdvanceParams,
    UltrasonicAdvancedParams,
    ObuAdvancedParams,
  },
  props: {},
  data () {
    return {
      visible: false,
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
      item: {},
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterName') },
          {
            validator: (rule, name, callback) => {
              if (!name) {
                return callback(new Error('请填写传感器配置名称'))
              }
              if (name.length > 20) {
                return callback(new Error('传感器配置名称不能超过20个字符'))
              }
              const isRepeated = this.sensors.some((s) => {
                if (s.idx === this.item.idx) {
                  return false
                }
                return s.name === name
              })
              if (isRepeated) {
                return callback(new Error('这个配置名称已经存在了'))
              }
              return callback()
            },
            trigger: 'blur',
          },
        ],
      },
    }
  },
  computed: {
    ...mapState('planners', ['sensors']),
    advancePropsTitle () {
      return advancePropsTitles[this.item.type] || ''
    },
    title () {
      let name = ''
      if (this.item?.type) {
        name = this.$t(nameMap[this.item.type])
      }
      return `${name}设备配置`
    },
  },
  methods: {
    ...mapActions('planners', ['saveSensor']),
    startEdit (idx) {
      const sensor = this.sensors.find(s => s.idx === idx)
      this.item = _.cloneDeep(sensor)
      this.visible = true
    },
    changeAdvancedInfo (payload) {
      const { type, param } = this.item
      // 重新计算数据
      const newParam = changeSensorParam(type, param, payload)
      this.item.param = { ...param, ...newParam }
    },
    cancel () {
      this.visible = false
      this.item = {}
    },
    async save () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return
      }
      await this.saveSensor(this.item)
      this.$message.success(this.$t('tips.saveSuccess'))
      this.cancel()
    },
  },
}
</script>

<style scoped lang="less">
.sensor-config-dialog {
  width: 240px;
  height: 500px;
  padding-top: 10px;
  overflow: auto;

  .sensor-config-dialog-btns {
    flex: 0 0 34px;
    padding: 0 10px;
    display: flex;
    justify-content: flex-end;
    align-items: center;
  }

  .param-cell {
    display: flex;
    justify-content: space-between;
    padding: 10px 0;
    align-items: center;

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
  }

  .sensor-name {
    display: flex;
    justify-content: space-between;
    padding: 0 20px;

    :deep(.el-form-item__label) {
      font-size: 14px;
      color: #606266;
    }

    &::before {
      display: none;
    }

    &::after {
      display: none;
    }
  }
}
</style>
