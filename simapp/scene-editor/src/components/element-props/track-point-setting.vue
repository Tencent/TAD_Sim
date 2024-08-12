<template>
  <el-dialog
    :model-value="visible"
    :close-on-click-modal="false"
    :header="$t('operation.setTrackPoints')"
  >
    <el-form
      ref="form"
      inline
      class="track-point-setting-form"
      label-width="90px"
      :rules="rules"
      :model="item"
    >
      <h6 class="form-sub-title">
        {{ $t("operation.positionSet") }}
      </h6>
      <div class="form-line">
        <el-form-item :label="$t('scenario.longitude')" prop="lat" :show-message="false">
          <InputNumber
            v-model="item.lon"
            :precision="7"
          />
        </el-form-item>
        <el-form-item :label="$t('scenario.altitude')">
          <InputNumber
            v-model="item.alt"
            :precision="7"
            disabled
          />
        </el-form-item>
      </div>
      <div class="form-line">
        <el-form-item :label="$t('scenario.latitude')" prop="lat">
          <InputNumber
            v-model="item.lat"
            :precision="7"
          />
          <template #error="{ error }">
            <span class="lat-err-message">{{ error }}</span>
          </template>
        </el-form-item>
      </div>
      <template v-if="plannerTrajectoryEnabled">
        <h6 class="form-sub-title">
          {{ $t('operation.statusSet') }}
        </h6>
        <div class="form-line">
          <el-form-item :label="$t('scenario.speed')">
            <InputNumber
              v-model="item.velocity"
              unit="m/s"
              :max="additionalData.currentTrackPointIndex ? 100 : additionalData.maxVelocity"
            />
          </el-form-item>
          <el-form-item :label="$t('acceleration')">
            <InputNumber
              v-model="item.accs"
              unit="m/s²"
              allow-empty-value
            />
          </el-form-item>
        </div>
        <div class="form-line">
          <el-form-item :label="$t('scenario.heading')">
            <InputNumber
              v-model="item.heading"
              unit="rad"
              allow-empty-value
            />
          </el-form-item>
          <el-form-item :label="$t('scenario.frontWheelAngle')">
            <InputNumber
              v-model="item.frontwheel"
              unit="rad"
              allow-empty-value
            />
          </el-form-item>
        </div>
        <div class="form-line">
          <el-form-item :label="$t('scenario.gear')">
            <el-select
              v-model="item.gear"
            >
              <el-option
                v-for="gear of gearList"
                :key="gear.id"
                :value="gear.id"
                :label="gear.name"
              />
            </el-select>
          </el-form-item>
        </div>
      </template>
      <div class="form-buttons">
        <el-button @click="$emit('close')">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button @click="saveHandle">
          {{ $t('operation.ok') }}
        </el-button>
      </div>
    </el-form>
  </el-dialog>
</template>

<script>
import { mapGetters } from 'vuex'
import InputNumber from '@/components/input-number.vue'

const gearList = [
  {
    id: 'drive',
    name: 'D',
  },
  {
    id: 'park',
    name: 'P',
  },
  {
    id: 'reverse',
    name: 'R',
  },
  {
    id: 'neutral',
    name: 'N',
  },
  {
    id: 'manual1',
    name: 'M1',
  },
  {
    id: 'manual2',
    name: 'M2',
  },
  {
    id: 'manual3',
    name: 'M3',
  },
  {
    id: 'manual4',
    name: 'M4',
  },
  {
    id: 'manual5',
    name: 'M5',
  },
  {
    id: 'manual6',
    name: 'M6',
  },
  {
    id: 'manual7',
    name: 'M7',
  },
]

export default {
  name: 'TrackPointSetting',
  components: { InputNumber },
  props: {
    visible: {
      type: Boolean,
      required: true,
    },
    data: {
      type: Object,
      required: true,
    },
    additionalData: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      item: {
        velocity: '',
        accs: '',
        heading: '',
        frontwheel: '',
        gear: '',
        lon: '',
        lat: '',
        alt: '',
      },
      rules: {
        lon: [
          { required: true, message: this.$t('tips.inputLongitude') },
        ],
        lat: [
          { required: true, message: this.$t('tips.inputLatitude') },
          { validator: this.validateLonLat, trigger: 'manual' },
        ],
      },
    }
  },
  computed: {
    ...mapGetters('scenario', {
      nodeTree: 'tree',
      selectedObject: 'selectedObject',
    }),
    gearList () {
      return gearList
    },
    plannerTrajectoryEnabled () {
      const { selectedObject } = this
      if (!selectedObject) {
        return false
      }
      const { type, trajectory_enabled } = selectedObject
      return type === 'planner' && trajectory_enabled
    },
  },
  watch: {
    visible (val) {
      if (val) {
        this.$nextTick(() => {
          Object.assign(this.item, {
            velocity: '',
            accs: '',
            heading: '',
            frontwheel: '',
            gear: '',
            lon: '',
            lat: '',
            alt: '',
          }, this.data)
        })
        if (this.$refs.form) {
          this.$refs.form.clearValidate()
        }
      }
    },
  },
  created () {
    Object.assign(this.item, {
      velocity: '',
      accs: '',
      heading: '',
      frontwheel: '',
      gear: '',
    }, this.data)
  },
  methods: {
    async validateLonLat (rule, value, callback) {
      if (this.plannerTrajectoryEnabled) {
        return callback()
      }
      const { lon, lat } = this.item
      const { err, alt, offset } = await window.simuScene.hadmap.getNearbyLaneInfo(lon, lat)
      if (err !== 0) {
        callback(new Error(this.$t('tips.placeWithoutRoadInfo')))
      } else if (Math.abs(offset) > 2.5) {
        callback(new Error(this.$t('tips.outOfRoadCenter', { n: '2.5' })))
      } else {
        this.item.alt = (+alt).toFixed(7)
        callback()
      }
    },
    async saveHandle () {
      try {
        await this.$refs.form.validate()
        if (this.item.frontwheel === '') {
          this.item.frontwheel = null
        }
        if (this.item.accs === '') {
          this.item.accs = null
        }
        if (this.item.heading === '') {
          this.item.heading = null
        }
        this.$emit('change', { ...this.item })
        this.$emit('close')
      } catch (e) {
      }
    },
  },
}
</script>

<style scoped lang="less">
.track-point-setting-form {
  padding: 20px 60px 20px 20px;

  .el-form-item {
    margin-right: 30px;
  }

  .form-buttons {
    text-align: right;
  }

  .form-sub-title {
    color: #c2c2c2;
    font-size: 12px;
    font-weight: normal;
    margin-bottom: 5px;
  }

  .lat-err-message {
    position: absolute;
    z-index: 2;
    top: 26px;
    width: 250px;
    left: 0;
    font-size: 12px;
    color: #F56C6C;
  }
}
</style>
