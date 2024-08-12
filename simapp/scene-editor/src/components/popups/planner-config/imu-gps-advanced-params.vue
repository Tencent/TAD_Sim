<template>
  <div class="imu-gps-advanced-params">
    <div class="param-cell multi-input">
      <label class="form-label">{{ $t(IMUAdvancedInfo.Quaternion.name) }}</label>
      <div class="form-multi-input">
        <div
          v-for="(p, index) of QuaternionNames"
          :key="p"
          class="input-cell"
        >
          <InputNumber
            :model-value="data.Quaternion[index]"
            @update:model-value="changeMultiValueParam('Quaternion', index, $event)"
          />
          <span>{{ p }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import InputNumber from '@/components/input-number.vue'
import { IMUAdvancedInfo, QuaternionNames } from '@/store/modules/sensor/constants'

export default {
  name: 'ImuGpsAdvancedParams',
  components: {
    InputNumber,
  },
  props: {
    currentSensor: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      QuaternionNames,
      IMUAdvancedInfo,
    }
  },
  computed: {
    data () {
      return this.currentSensor.param
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
.imu-gps-advanced-params {
  padding: 0 22px;
}

.param-cell {
  display: flex;
  justify-content: space-between;
  padding: 10px 0;
  align-items: center;

  &.multi-input {
    display: block;

    .form-label {
      display: block;
      margin-bottom: 15px;
    }
  }

  .form-multi-input {
    display: grid;
    grid-template-columns: 1fr 1fr;
    grid-column-gap: 10px;

    .input-cell {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin-bottom: 10px;

      :deep(.el-input__inner) {
        width: 100%;
        height: 24px;
        line-height: 22px;
        border-radius: 2px;
      }
    }
  }
}
</style>
