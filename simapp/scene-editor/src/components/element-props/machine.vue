<template>
  <el-form
    v-if="form"
    ref="form"
    class="property-form property-dynamic-obstacle"
    size="small"
    :model="form"
    :rules="rules"
    label-width="98px"
    @submit.prevent
  >
    <el-collapse v-model="activeCollapseNames">
      <el-collapse-item id="scenario-property-basic-info" :title="$t('scenario.basicInformation')" name="basicInfo">
        <el-form-item prop="laneId" class="param-cell" :label="$t('scenario.onLane')">
          <el-select
            id="object-property-lane-select"
            :model-value="form.laneId"
            @change="modifyObject({ laneId: $event })"
          >
            <el-option
              v-for="laneId in optionLaneIDList"
              :id="`object-property-lane-option-${laneId}`"
              :key="laneId"
              :label="laneId"
              :value="laneId"
            />
          </el-select>
        </el-form-item>
        <el-form-item prop="startShift" :label="$t('scenario.longitudinalDistance')" class="param-cell">
          <InputNumber
            :model-value="form.startShift"
            unit="m"
            :precision="3"
            :min="0"
            :max="maxShift"
            @update:model-value="modifyObject({ startShift: $event })"
          />
        </el-form-item>
        <el-form-item prop="offset" :label="$t('scenario.lateralOffset')" class="param-cell">
          <InputNumber
            :model-value="form.offset"
            unit="m"
            :precision="3"
            :min="-maxOffset"
            :max="maxOffset"
            @update:model-value="modifyObject({ offset: $event })"
          />
        </el-form-item>

        <el-form-item prop="z" :label="$t('scenario.startAlt')" class="param-cell">
          <InputNumber
            :model-value="form.startAlt"
            unit="m"
            :precision="3"
            disabled
          />
        </el-form-item>
      </el-collapse-item>
      <el-collapse-item id="scenario-property-motion" :title="$t('scenario.motionControl')" name="motion">
        <el-form-item class="param-cell multi-input" :label="$t('scenario.speed')">
          <numeric-range-slider-input
            v-model.lazy="velocityRange"
            unit="m/s"
            :min="0"
            :max="10.00"
            :step="0.01"
            :sub-labels="[$t('scenario.initial'), $t('scenario.maximum')]"
            :input-width="80"
          />
        </el-form-item>
        <el-form-item class="param-cell multi-input">
          <el-radio id="car-behavior-type-custom" v-model="behaviorType" label="2">
            {{ $t('operation.custom') }}
          </el-radio>
          <el-button
            id="car-behavior-type-setting"
            :disabled="behaviorType === '1'"
            class="solid-button"
            @click="machineTriggerConditionConfig"
          >
            {{ $t('scenario.triggerSettings') }}
          </el-button>
        </el-form-item>
        <el-form-item v-if="behaviorType === '2'" class="param-cell multi-input">
          <el-checkbox v-model="trajectoryEnabled" disabled>
            {{ $t('scenario.trajectoryFollowing') }}
          </el-checkbox>
        </el-form-item>
        <el-form-item v-if="false" prop="angle" :label="$t('scenario.angle')" class="param-cell">
          <InputNumber
            :model-value="angle"
            unit="°"
            :min="0"
            :max="360"
            :precision="3"
            @update:model-value="modifyObject({ angle: $event / 180 * Math.PI })"
          />
        </el-form-item>
        <el-form-item v-if="trajectoryEnabled" prop="start_angle" :label="$t('scenario.startAngle')" class="param-cell">
          <InputNumber
            :model-value="form.start_angle"
            unit="°"
            :min="0"
            :max="360"
            :precision="1"
            @update:model-value="setObjectAngle($event)"
          />
          <div class="checkbox-wrap">
            <el-checkbox v-model="autoFollowRoadDirection">
              自动对齐车道方向
            </el-checkbox>
            <el-tooltip effect="dark" content="场景元素位置变化时是否自动对齐车道方向，该配置为全局配置">
              <i class="el-icon-warning-outline checkbox-tip-icon" />
            </el-tooltip>
          </div>
        </el-form-item>
      </el-collapse-item>
      <el-collapse-item id="scenario-property-track" :title="$t('scenario.locusPoint')" name="track">
        <el-form-item class="param-cell multi-input">
          <div class="set-track-points-btns">
            <el-button
              size="small"
              class="solid-button"
              @click="setEnd({ type: 'machine', id: form.id })"
              @keydown.enter.prevent
            >
              <locate-svg />
              {{ $t('operation.setTrackPoints') }}
            </el-button>
            <el-icon class="el-icon-delete" :title="$t('operation.deleteAll')" @click="handleDeleteTrackItem(-1)">
              <delete />
            </el-icon>
          </div>
          <el-table
            ref="table"
            class="table"
            highlight-current-row
            :data="form.endPosArr || []"
          >
            <el-table-column
              :label="`${$t('scenario.longitude')}/${$t('scenario.latitude')}/${$t('scenario.altitude')}`"
            >
              <template #default="scope">
                <div :id="`machine-track-node-${scope.$index}`" @click="handleFocusTrackItem(scope.$index)">
                  <div class="position">
                    {{ (+scope.row.lon).toFixed(7) }}
                  </div>
                  <div class="position">
                    {{ (+scope.row.lat).toFixed(7) }}
                  </div>
                  <div class="position">
                    {{ (+scope.row.alt).toFixed(7) }}
                  </div>
                </div>
              </template>
            </el-table-column>
            <el-table-column v-if="false" width="80">
              <template #default="scope">
                <div class="position">
                  <InputNumber
                    :model-value="scope.row.velocity"
                    style="vertical-align: middle;"
                    unit="m/s"
                    :precision="2"
                    :min="0"
                    :max="+form.maxVelocity"
                    :style="{ width: '72px', marginBottom: '5px' }"
                    @update:model-value="modifyTrack(scope.$index, $event, scope.row.gear)"
                  />
                </div>
              </template>
            </el-table-column>
            <el-table-column
              width="40"
              :label="$t('operation.operation')"
            >
              <template #default="scope">
                <div class="button-group">
                  <el-button
                    v-if="scope.$index > 0"
                    :id="`track-item-delete-${scope.$index}`"
                    link
                    @click="handleDeleteTrackItem(scope.$index)"
                  >
                    <el-icon class="el-icon-delete" :title="$t('operation.delete')">
                      <delete />
                    </el-icon>
                  </el-button>
                </div>
              </template>
            </el-table-column>
          </el-table>
        </el-form-item>
      </el-collapse-item>
    </el-collapse>
  </el-form>
</template>

<script>
import { mapActions, mapMutations } from 'vuex'
import _ from 'lodash'
import property from './property'
import { editor } from '@/api/interface'

export default {
  name: 'DynamicObstacle',
  mixins: [property],
  data () {
    return {
      activeCollapseNames: ['basicInfo', 'motion', 'track'],
    }
  },
  computed: {
    velocityRange: {
      get () {
        return [+this.form.startVelocity, +this.form.maxVelocity]
      },
      set (value) {
        const [startVelocity, maxVelocity] = value
        const { endPosArr } = this.form
        if (endPosArr?.length > 0) {
          endPosArr[0].velocity = Math.min(startVelocity, maxVelocity)
        }
        this.modifyObjectParams({
          startVelocity,
          maxVelocity,
          endPosArr,
        })
      },
    },
    behaviorType: {
      get () {
        const { behavior } = this.form
        if (behavior === 'TrafficVehicle') {
          return '1'
        } else if (behavior === 'UserDefine' || behavior === 'TrajectoryFollow') {
          return '2'
        }
        return null
      },
      set (value) {
        let behavior
        if (value === '1') {
          behavior = 'TrafficVehicle'
        } else if (value === '2') {
          behavior = 'UserDefine'
        }
        this.modifyObject({
          behavior,
        })
      },
    },
    trajectoryEnabled: {
      get () {
        return this.form.behavior === 'TrajectoryFollow'
      },

      set (value) {
        let behavior
        if (value) {
          behavior = 'TrajectoryFollow'
        } else {
          behavior = 'UserDefine'
        }
        this.modifyObject({
          behavior,
        })
      },
    },
    angle () {
      const radian = this.form.angle || 0
      return radian / Math.PI * 180
    },
  },
  mounted () {
    editor.addEventListener('pathNodeSelected', this.pathNodeSelected)
  },
  beforeUnmount () {
    editor.removeEventListener('pathNodeSelected', this.pathNodeSelected)
  },
  methods: {
    ...mapMutations('mission', [
      'startMission',
    ]),
    ...mapActions('scenario', [
      'setEnd',
      'selectPathEnd',
      'removePathPoint',
    ]),
    machineTriggerConditionConfig () {
      this.startMission('EventConfig')
    },
    pathNodeSelected (payload) {
      if (!payload) {
        this.$refs.table.setCurrentRow()
        return
      }
      const { type, index } = payload
      const el = document.getElementById(`machine-track-node-${index}`)
      if (type === 'machine') {
        this.$refs.table.setCurrentRow(this.form.endPosArr[index])
        if (el) {
          el.scrollIntoView()
        }
      }
    },
    handleFocusTrackItem (index) {
      this.selectPathEnd({
        type: 'machine',
        id: this.form.id,
        index,
      })
    },
    handleDeleteTrackItem (index) {
      this.removePathPoint({
        type: 'machine',
        id: this.form.id,
        index,
        endPosArr: this.form.endPosArr,
      })
    },
    modifyTrack (idx, velocity, gear) {
      console.log(`idx: ${idx}, velocity: ${velocity}, gear: ${gear}`)
      const endPosArr = _.cloneDeep(this.form.endPosArr)
      endPosArr[idx].velocity = velocity
      endPosArr[idx].gear = gear

      let maxVelocity = this.form.maxVelocity
      maxVelocity = Math.max(+maxVelocity, velocity)

      if (idx === 0) {
        this.modifyObjectParams({
          startVelocity: velocity,
          endPosArr,
          maxVelocity,
        })
      } else {
        this.modifyObjectParams({
          endPosArr,
          maxVelocity,
        })
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.property-dynamic-obstacle {
  .solid-button {
    margin-bottom: 10px;
  }

  .position {
    line-height: 1.5;
  }

  .button-group {
    .el-button {
      margin: 0;
      padding: 0;
    }
  }

  :deep(.el-table__body tr.current-row > td) {
    color: #16d1f3;
    background-color: transparent;
  }

  .set-track-points-btns {
    display: flex;
    align-items: center;
    justify-content: space-between;
  }
}
</style>
