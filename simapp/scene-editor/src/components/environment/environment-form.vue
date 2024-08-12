<template>
  <el-form
    ref="form"
    inline
    label-width="72px"
    class="environment-form"
    :model="data"
    :rules="rules"
  >
    <div class="time-stamp">
      <h6>{{ $t('scenario.timeStamp') }}</h6>
      <el-form-item prop="TimeStamp">
        <InputNumber
          :disabled="index === 0"
          :min="0"
          :max="999999999"
          :precision="0"
          :model-value="data.TimeStamp"
          @update:model-value="$emit('change', { TimeStamp: $event })"
        />
      </el-form-item>
    </div>
    <div class="weather">
      <h6>{{ $t('scenario.weather') }}</h6>
      <!-- 能见度 -->
      <el-form-item :label="$t('scenario.visibility')">
        <InputNumber
          :min="0"
          :max="30"
          :precision="3"
          unit="km"
          :model-value="data.Weather.VisualRange"
          @update:model-value="$emit('change', { Weather: { VisualRange: $event } })"
        />
      </el-form-item>
      <!-- 风速 -->
      <el-form-item :label="$t('scenario.windSpeed')">
        <InputNumber
          :min="0"
          :max="17"
          :precision="3"
          unit="m/s"
          :model-value="data.Weather.WindSpeed"
          @update:model-value="$emit('change', { Weather: { WindSpeed: $event } })"
        />
      </el-form-item>
      <!-- 云密度 -->
      <el-form-item :label="$t('scenario.cloudDensity')">
        <el-select
          :model-value="data.Weather.CloudState"
          @change="$emit('change', { Weather: { CloudState: $event } })"
        >
          <el-option
            v-for="opt of CloudStateOptions"
            :key="opt.value"
            :value="opt.value"
            :label="$t(opt.label)"
          />
        </el-select>
      </el-form-item>
      <!--      <el-form-item :label="$t('scenario.rainfall')"> -->
      <!--        <InputNumber -->
      <!--          :min="0" -->
      <!--          :max="250" -->
      <!--          :precision="3" -->
      <!--          unit="mm" -->
      <!--          :model-value="data.Weather.RainFall" -->
      <!--          @update:model-value="$emit('change', {Weather: {RainFall: $event}})" -->
      <!--        /> -->
      <!--      </el-form-item> -->
      <!--      <el-form-item :label="$t('scenario.snowfall')"> -->
      <!--        <InputNumber -->
      <!--          :min="0" -->
      <!--          :max="10" -->
      <!--          :precision="3" -->
      <!--          unit="mm" -->
      <!--          :model-value="data.Weather.SnowFall" -->
      <!--          @update:model-value="$emit('change', {Weather: {SnowFall: $event}})" -->
      <!--        /> -->
      <!--      </el-form-item> -->
      <!-- 降水 -->
      <el-form-item :label="$t('scenario.precipitation')">
        <el-select
          :model-value="data.Weather.PrecipitationType"
          @change="$emit('change', { Weather: { PrecipitationType: $event } })"
        >
          <el-option value="dry" :label="$t('scenario.dry')" />
          <el-option value="rain" :label="$t('scenario.rainfall')" />
          <el-option value="snow" :label="$t('scenario.snowfall')" />
        </el-select>
      </el-form-item>
      <el-form-item
        v-show="data.Weather.PrecipitationType !== 'dry'"
        :label="precipitationIntensityTitle"
      >
        <InputNumber
          :min="0"
          :max="1"
          :precision="3"
          :model-value="data.Weather.PrecipitationIntensity"
          @update:model-value="$emit('change', { Weather: { PrecipitationIntensity: $event } })"
        />
      </el-form-item>
      <!-- 温度暂时不设置 -->
      <el-form-item v-if="false" :label="$t('scenario.temperature')">
        <InputNumber
          :min="-100"
          :max="100"
          :precision="2"
          unit="°C"
          :model-value="data.Weather.Temperature"
          @update:model-value="$emit('change', { Weather: { Temperature: $event } })"
        />
      </el-form-item>
    </div>
    <div class="date-time">
      <h6>{{ $t('date') }}</h6>
      <el-form-item>
        <el-date-picker
          v-model="date"
          :disabled="index > 0"
          :clearable="false"
          :style="{ width: '128px' }"
          value-format="YYYY-MM-DD"
        />
      </el-form-item>
      <h6>{{ $t('time') }}</h6>
      <el-form-item>
        <el-time-picker
          v-model="time"
          :disabled="index > 0"
          :clearable="false"
          :style="{ width: '128px' }"
          value-format="HH:mm:ss"
          popper-class="self-time-picker"
        />
      </el-form-item>
    </div>
  </el-form>
</template>

<script>
import InputNumber from '@/components/input-number.vue'
import eventBus from '@/event-bus'

const CloudStateOptions = [
  {
    label: 'scenario.cloudStateMap.free',
    value: 'free',
  },
  {
    label: 'scenario.cloudStateMap.cloudy',
    value: 'cloudy',
  },
  {
    label: 'scenario.cloudStateMap.overcast',
    value: 'overcast',
  },
  {
    label: 'scenario.cloudStateMap.rainy',
    value: 'rainy',
  },
]

export default {
  name: 'EnvironmentForm',
  components: {
    InputNumber,
  },
  props: {
    data: {
      type: Object,
      required: true,
    },
    timeStamps: {
      type: Array,
      default: () => [],
    },
    index: {
      type: Number,
      required: true,
    },
  },
  data () {
    return {
      rules: {
        TimeStamp: [
          { required: true, trigger: 'blur' },
          {
            validator: (rules, value, callback) => {
              const arr = this.timeStamps.filter(t => t === value)
              if (arr.length >= 2) {
                callback(new Error(this.$t('tips.timestampExisted', { t: value })))
              } else {
                callback()
              }
            },
            trigger: 'blur',
          },
        ],
      },
    }
  },
  computed: {
    date: {
      get () {
        const { data } = this
        if (!data.Date) {
          return null
        }
        let { Year, Month, Day } = data.Date
        Month = `${Month}`.padStart(2, '0')
        Day = `${Day}`.padStart(2, '0')
        return `${Year}-${Month}-${Day}`
      },
      set (val) {
        if (!val) {
          // this.$emit('change', { Date: null })
        } else {
          const [, Year, Month, Day] = /(\d{4})-(\d{2})-(\d{2})/.exec(val)
          this.$emit('change', { Date: { Year: +Year, Month: +Month, Day: +Day } })
        }
      },
    },
    time: {
      get () {
        const { data } = this
        if (!data.Time) {
          return null
        }
        let { Hour, Minute, Second, Millisecond } = data.Time
        Hour = `${Hour}`.padStart(2, '0')
        Minute = `${Minute}`.padStart(2, '0')
        Second = `${Second}`.padStart(2, '0')
        Millisecond = `${Millisecond}`.padStart(3, '0')
        return `${Hour}:${Minute}:${Second}.${Millisecond}`
      },
      set (val) {
        if (!val) {
          // this.$emit('change', { Time: null })
        } else {
          const [, Hour, Minute, Second] = /(\d{2}):(\d{2}):(\d{2})/.exec(val)
          const Millisecond = 0
          this.$emit('change', { Time: { Hour: +Hour, Minute: +Minute, Second: +Second, Millisecond } })
        }
      },
    },
    CloudStateOptions: () => CloudStateOptions,
    precipitationIntensityTitle () {
      switch (this.data.Weather.PrecipitationType) {
        case 'rain':
          return this.$t('scenario.rainIntensity')
        case 'snow':
          return this.$t('scenario.snowIntensity')
        default:
          return ''
      }
    },
  },
  created () {
    eventBus.$on('environment/reset', this.reset)
  },
  beforeUnmount () {
    eventBus.$off('environment/reset', this.reset)
  },
  methods: {
    reset () {
      this.$refs.form.clearValidate()
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.environment-form {
  display: grid;
  grid-template-columns: 4fr 3fr;
  grid-template-areas: 't t' 'w d';
  padding: 10px 20px;

  h6 {
    font-size: 13px;
    padding: 8px 0;
    margin-bottom: 8px;
    font-weight: normal;
    color: @global-font-color;
  }

  .time-stamp {
    grid-area: t;

    :deep(.el-form-item) {
      width: 100%;

      .el-form-item__content {
        width: 100%;
      }
    }
  }

  .weather {
    :deep(.input-number) {
      width: 100px;
    }
  }

}
</style>

<style lang="less">
.self-time-picker {
    .el-time-spinner__item {
      color: #c2c2c2 !important;
    }
    .is-active {
      color: #dcdcdc !important;
    }
  }
</style>
