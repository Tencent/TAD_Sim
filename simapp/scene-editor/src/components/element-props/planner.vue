<template>
  <el-form
    v-if="form"
    ref="form"
    class="property-form property-planner"
    :model="form"
    :rules="rules"
    label-width="88px"
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

        <el-form-item prop="startOffset" :label="$t('scenario.lateralOffset')" class="param-cell">
          <InputNumber
            :model-value="form.startOffset"
            unit="m"
            :precision="3"
            :min="-maxOffset"
            :max="maxOffset"
            @update:model-value="modifyObject({ startOffset: $event })"
          />
        </el-form-item>

        <el-form-item prop="startAlt" :label="$t('scenario.height')" class="param-cell">
          <InputNumber
            :model-value="form.startAlt"
            unit="m"
            :precision="3"
            disabled
          />
        </el-form-item>

        <el-form-item prop="angle" :label="$t('scenario.angle')" class="param-cell">
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
        <div class="property-form-item-separator" role="separator" />

        <el-form-item prop="startVelocity" :label="$t('scenario.initVelocity')" class="param-cell">
          <InputNumber
            :model-value="form.startVelocity"
            unit="m/s"
            :min="0"
            :max="100"
            :precision="2"
            @update:model-value="modifyStartVelocity($event)"
          />
        </el-form-item>
        <el-form-item prop="maxVelocity" :label="$t('scenario.maxVelocity')" class="param-cell">
          <InputNumber
            :model-value="form.maxVelocity"
            unit="m/s"
            :min="0"
            :max="100"
            :precision="2"
            @update:model-value="modifyObjectParams({ maxVelocity: $event })"
          />
        </el-form-item>
        <el-form-item prop="acceleration_max" :label="$t('scenario.maxAcceleration')" class="param-cell">
          <InputNumber
            :model-value="form.acceleration_max"
            unit="m/s²"
            :min="0"
            :max="100"
            :precision="2"
            @update:model-value="modifyObjectParams({ acceleration_max: $event })"
          />
        </el-form-item>
        <el-form-item prop="deceleration_max" :label="$t('scenario.minAcceleration')" class="param-cell">
          <InputNumber
            :model-value="form.deceleration_max"
            unit="m/s²"
            :min="0"
            :max="100"
            :precision="2"
            @update:model-value="modifyObjectParams({ deceleration_max: $event })"
          />
        </el-form-item>
      </el-collapse-item>
      <!-- 主车选择 -->
      <el-collapse-item title="车辆类型配置" name="name">
        <div class="planner-selector-wrap">
          <PlannerSelector />
          <el-icon class="planner-config-trigger el-icon-s-tools" @click.stop="toConfigPlanner">
            <Tools />
          </el-icon>
        </div>
      </el-collapse-item>
      <!-- 主车选择 end -->
      <el-collapse-item title="控制器初始状态" name="controlInit">
        <el-form-item class="param-cell" label="横向控制">
          <el-switch
            :model-value="form.control_lateral"
            @change="modifyObjectParams({ control_lateral: $event })"
          />
        </el-form-item>
        <el-form-item class="param-cell" label="纵向控制">
          <el-switch
            :model-value="form.control_longitudinal"
            @change="modifyObjectParams({ control_longitudinal: $event })"
          />
        </el-form-item>
      </el-collapse-item>
      <el-collapse-item id="scenario-property-track" title="行为" name="track">
        <el-form-item class="param-cell multi-input tj-evt">
          <el-checkbox v-model="trajectoryEnabled">
            {{ $t('scenario.trajectoryFollowing') }}
          </el-checkbox>
          <el-button
            class="solid-button"
            @click="plannerEventConfig"
          >
            事件设置
          </el-button>
        </el-form-item>
        <el-form-item
          prop="sampleInterval"
          :label="$t('scenario.samplingInterval')"
          class="param-cell"
        >
          <InputNumber
            :model-value="form.sampleInterval"
            unit="m"
            :precision="3"
            :min="minSampleInterval"
            @update:model-value="modifyObjectParams({ sampleInterval: $event })"
          />
        </el-form-item>
        <el-form-item prop="snap" class="param-cell">
          <div class="snap-div" :title="trajectoryEnabled ? '轨迹跟踪模式下无法吸附至车道中心线' : ''">
            <el-checkbox :model-value="snap" :disabled="trajectoryEnabled" @update:model-value="setSnap($event)">
              {{ $t('scenario.attachToLaneCenterline') }}
            </el-checkbox>
          </div>
        </el-form-item>
        <div class="set-track-points-btns">
          <el-button
            size="small"
            class="solid-button"
            @click="setEnd({ type: 'planner', id: form.id })"
            @keydown.enter.prevent
          >
            <locate-svg />
            {{ $t('operation.setTrackPoints') }}
          </el-button>
          <el-icon class="el-icon-delete" :title="$t('operation.deleteAll')" @click="handleDeleteTrackItem(-1)">
            <Delete />
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
              <div :id="`planner-track-node-${scope.$index}`" @click="handleFocusTrackItem(scope.$index)">
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
          <el-table-column
            width="60"
            :label="$t('operation.operation')"
          >
            <template #default="scope">
              <div class="button-group">
                <el-button link @click="setTrackPoint(scope.$index)">
                  <el-icon class="el-icon-setting">
                    <Setting />
                  </el-icon>
                </el-button>
                <el-button
                  v-if="scope.$index > 0"
                  :id="`track-item-delete-${scope.$index}`"
                  link
                  @click="handleDeleteTrackItem(scope.$index)"
                >
                  <el-icon class="el-icon-delete" :title="$t('operation.delete')">
                    <Delete />
                  </el-icon>
                </el-button>
              </div>
            </template>
          </el-table-column>
        </el-table>
        <TrackPointSetting
          :visible="trackPointSettingVisible"
          :data="(form.endPosArr || [])[currentTrackPointIndex]"
          :additional-data="{ maxVelocity: +form.maxVelocity, currentTrackPointIndex }"
          @change="modifyTrack(currentTrackPointIndex, $event)"
          @close="trackPointSettingVisible = false"
        />
      </el-collapse-item>
    </el-collapse>
  </el-form>
</template>

<script>
import { mapActions } from 'vuex'
import { cloneDeep, isEqual, pick } from 'lodash-es'
import property from './property'
import { editor } from '@/api/interface'
import TrackPointSetting from '@/components/element-props/track-point-setting.vue'
import PlannerSelector from '@/components/element-props/planner-selector.vue'

export default {
  name: 'PropertyPlanner',
  components: { TrackPointSetting, PlannerSelector },
  mixins: [property],
  data () {
    return {
      activeCollapseNames: ['basicInfo', 'name', 'controlInit', 'track'],
      minSampleInterval: 0.01,
      enableSnap: true,
      trackPointSettingVisible: false,
      currentTrackPointIndex: 0,
    }
  },
  computed: {
    velocityRange: {
      get () {
        return [+this.form.startVelocity, +this.form.maxVelocity]
      },
      set (value) {
        const [velocity, maxVelocity] = value
        const endPosArr = this.form.endPosArr.map((pos, index) => {
          if (index === 0) {
            return {
              ...pos,
              velocity,
            }
          }
          return pos
        })
        this.modifyObjectParams({
          endPosArr,
          startVelocity: velocity,
          maxVelocity,
        })
      },
    },
    trajectoryEnabled: {
      get () {
        return this.form.trajectory_enabled
      },

      set (value) {
        const endPosArr = this.form.endPosArr.slice()
        if (value) {
          endPosArr.forEach((p) => {
            p.frontwheel = null
            p.accs = null
            p.heading = null
          })
          // 同步主车的初始速度
          endPosArr[0].velocity = this.form.startVelocity
        } else {
          endPosArr.forEach((p) => {
            delete p.frontwheel
            delete p.accs
            delete p.heading
          })
        }
        this.modifyObjectParams({
          endPosArr,
          trajectory_enabled: value,
        })
      },
    },
  },
  created () {
    this.getModuleSetList()
  },
  mounted () {
    editor.addEventListener('pathNodeSelected', this.pathNodeSelected)
  },
  beforeUnmount () {
    editor.removeEventListener('pathNodeSelected', this.pathNodeSelected)
  },
  methods: {
    ...mapActions('scenario', [
      'setEnd',
      'selectPathEnd',
      'removePathPoint',
    ]),
    ...mapActions('module-set', ['getModuleSetList']),
    pathNodeSelected (payload) {
      if (!payload) {
        this.$refs.table.setCurrentRow()
        return
      }
      const { type, index } = payload
      const el = document.getElementById(`planner-track-node-${index}`)
      if (type === 'planner') {
        this.$refs.table.setCurrentRow(this.form.endPosArr[index])
        if (el) {
          el.scrollIntoView()
        }
      }
    },
    modifyStartVelocity (velocity) {
      velocity = Math.min(this.form.maxVelocity, velocity, 100)
      const endPosArr = this.form.endPosArr.slice()
      endPosArr[0].velocity = velocity
      this.modifyObjectParams({
        startVelocity: velocity,
        endPosArr,
      })
    },
    async modifyTrack (idx, { velocity, gear, accs, heading, frontwheel, lon, lat, alt }) {
      const endPosArr = cloneDeep(this.form.endPosArr)
      endPosArr[idx].velocity = velocity
      endPosArr[idx].gear = gear
      endPosArr[idx].accs = accs
      endPosArr[idx].heading = heading
      endPosArr[idx].frontwheel = frontwheel

      endPosArr[idx].lon = lon
      endPosArr[idx].lat = lat
      endPosArr[idx].alt = alt

      const len = endPosArr.length

      if (idx === 0) {
        // 是否重新定位，如果重新定位，则可能更新初始角度
        const isRelocated = !isEqual(pick(endPosArr[0], ['lon', 'lat']), { lon, lat })
        if (isRelocated) {
          // 重新定位了
          const params = {}
          const { alt: startAlt, dist, offset, lid } = await window.simuScene.hadmap.getNearbyLaneInfo(lon, lat, alt)
          params.laneId = lid
          params.startAlt = startAlt
          params.startShift = dist
          params.startOffset = offset
          params.startVelocity = velocity
          this.modifyObject(params)
        } else {
          this.modifyObjectParams({
            startVelocity: velocity,
          })
        }
        if (len > 1) {
          // 如果第二个点的档位和当前点档位不一致，则下一个点的速度置为 0
          if (endPosArr[idx].gear !== endPosArr[idx + 1].gear) {
            endPosArr[idx + 1].velocity = 0
          }
          const isRelocated = !isEqual(pick(endPosArr[idx], ['lon', 'lat']), { lon, lat })
          if (isRelocated) {
            // 编辑最后一个点时要更新主车终点信息
            this.modifyObjectParams({
              endLon: lon,
              endLat: lat,
              endAlt: alt,
            })
          }
        }
      } else {
        // 最后一个点
        if (idx === len - 1) {
          // 和前一个点比较更改当前点
          if (endPosArr[idx].gear !== endPosArr[idx - 1].gear) {
            endPosArr[idx].velocity = 0
          }
        } else {
          // 和前一个点比较更改当前点
          if (endPosArr[idx].gear !== endPosArr[idx - 1].gear) {
            endPosArr[idx].velocity = 0
          }

          // 和后一个点比较更改后一个点
          if (endPosArr[idx].gear !== endPosArr[idx + 1].gear) {
            endPosArr[idx + 1].velocity = 0
          }
        }
      }

      window.simuScene.dropPathEnd('planner', this.form.id, idx, [+lon, +lat, +alt])

      await this.modifyObjectParams({
        endPosArr,
      })
      this.$nextTick(() => {
        this.$refs.table.setCurrentRow(this.form.endPosArr[idx])
      })
    },
    handleFocusTrackItem (index) {
      this.selectPathEnd({
        type: 'planner',
        id: this.form.id,
        index,
      })
    },
    handleDeleteTrackItem (index) {
      this.removePathPoint({
        type: 'planner',
        id: this.form.id,
        index,
        endPosArr: this.form.endPosArr,
      })
    },
    setTrackPoint (index) {
      this.currentTrackPointIndex = index
      this.trackPointSettingVisible = true
      this.selectPathEnd({
        type: 'planner',
        id: this.form.id,
        index,
      })
    },
    plannerEventConfig () {
      this.startMission('EventConfig')
    },
    toConfigPlanner () {
      this.startMission('PlannerConfig')
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.property-planner {
  .solid-button {
    margin-bottom: 10px;
  }

  .position {
    line-height: 1.5;
  }

  .button-group {
    .el-button {
      margin: 0 5px 0 0;
      padding: 0;

      &:last-child {
        margin: 0;
      }
    }
  }

  :deep(.el-table__body tr.current-row > td) {
    color: #16d1f3;
    background-color: transparent;
  }

  .tj-evt {
    :deep(.el-form-item__content) {
      flex: 1;
      display: flex !important;
      align-items: center;
      justify-content: space-between;

      .el-button {
        margin-bottom: 0;
      }

      &::before, &::after {
        display: none;
      }
    }
  }

  .set-track-points-btns {
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .param-cell .snap-div {
    display: flex;
    justify-content: flex-end;
  }

  .planner-selector-wrap {
    display: flex;
    justify-content: flex-start;
    align-items: center;
    padding: 10px 0 20px;

    .planner-config-trigger {
      margin-left: 15px;
      font-size: 14px;
      color: #B9B9B9;
      cursor: pointer;
    }
  }
}
</style>
