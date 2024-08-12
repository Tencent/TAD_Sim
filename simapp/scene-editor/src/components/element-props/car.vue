<template>
  <el-form
    v-if="form"
    ref="form"
    class="property-form property-car"
    :model="form"
    :rules="rules"
    label-width="98px"
    @submit.prevent
  >
    <el-collapse v-model="activeCollapseNames">
      <el-collapse-item id="scenario-property-basic-info" :title="$t('scenario.basicInformation')" name="basicInfo">
        <!-- 车道id -->
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
        <!-- 纵向距离 -->
        <el-form-item prop="startShift" class="param-cell" :label="$t('scenario.longitudinalDistance')">
          <InputNumber
            :model-value="form.startShift"
            unit="m"
            :precision="3"
            :min="0"
            :max="maxShift"
            @update:model-value="modifyObject({ startShift: $event })"
          />
        </el-form-item>
        <!-- 横向偏移 -->
        <el-form-item prop="startOffset" class="param-cell" :label="$t('scenario.lateralOffset')">
          <InputNumber
            :model-value="form.startOffset"
            unit="m"
            :precision="3"
            :min="-maxOffset"
            :max="maxOffset"
            @update:model-value="modifyObject({ startOffset: $event })"
          />
        </el-form-item>
        <!-- 初始高度 -->
        <el-form-item prop="z" :label="$t('scenario.startAlt')" class="param-cell">
          <InputNumber
            :model-value="form.startAlt"
            unit="m"
            :precision="3"
            disabled
          />
        </el-form-item>

        <div class="property-form-item-separator" role="separator" />
        <!-- 速度 -->
        <el-form-item class="param-cell multi-input" :label="$t('scenario.speed')">
          <numeric-range-slider-input
            v-model.lazy="velocityRange"
            unit="m/s"
            :min="0"
            :max="100"
            :step="0.01"
            :sub-labels="[$t('scenario.initial'), $t('scenario.maximum')]"
            :input-width="80"
          />
        </el-form-item>
        <!-- AI控制 -->
        <el-form-item class="param-cell multi-input">
          <el-radio
            id="car-behavior-type-ai"
            v-model="behaviorType"
            label="1"
            :style="{ marginBottom: '8px' }"
          >
            {{ $t('scenario.aiControl') }}
          </el-radio>
          <br>
          <el-radio
            id="car-behavior-type-custom"
            v-model="behaviorType"
            label="2"
          >
            {{ $t('operation.custom') }}
          </el-radio>
        </el-form-item>
        <el-form-item v-if="behaviorType === '1'" class="param-cell" :label="$t('scenario.aiModel')">
          <el-select
            id="object-property-ai-select"
            :model-value="form.aiModel"
            @change="modifyObject({ aiModel: $event })"
          >
            <el-option
              v-for="item in aiModels"
              :id="`object-property-ai-option-${item.id}`"
              :key="item.id"
              :label="$t(item.label)"
              :value="item.id"
            />
          </el-select>
        </el-form-item>
        <el-form-item v-if="behaviorType === '1'" class="param-cell multi-input">
          <NumericSliderInput
            id="traffic-flow-radical-degree-slider"
            :model-value="aggress"
            :label="$t('scenario.radicalDegree')"
            :max="1"
            :step="0.01"
            style="width: 100%"
            @update:model-value="modifyObjectParams({ aggress: $event })"
          />
        </el-form-item>
        <el-form-item v-if="behaviorType === '2'" class="param-cell">
          <template #label>
            <el-checkbox v-model="trajectoryEnabled">
              {{ $t('scenario.trajectoryFollowing') }}
            </el-checkbox>
          </template>
          <el-button
            id="car-behavior-type-setting"
            :disabled="behaviorType === '1' || trajectoryEnabled"
            class="solid-button"
            @click="carTriggerConfig"
          >
            {{ $t('scenario.eventSettings') }}
          </el-button>
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
        <el-form-item
          v-if="behaviorType === '2' && !trajectoryEnabled"
          class="param-cell"
          :label="$t('scenario.referenceVehicle')"
        >
          <el-select
            id="object-property-follow-select"
            :model-value="form.follow"
            @change="modifyObjectParams({ follow: $event })"
            @visible-change="$event && setOptionFollowIDList()"
          >
            <el-option
              v-for="item in optionFollowIDList"
              :id="`object-property-follow-option-${item.id}`"
              :key="item.id"
              :label="$t(item.name)"
              :value="item.id"
            />
          </el-select>
        </el-form-item>
        <el-form-item prop="snap" class="param-cell">
          <div class="snap-div" :title="trajectoryEnabled ? '轨迹跟踪模式下无法吸附至车道中心线' : ''">
            <el-checkbox :model-value="snap" :disabled="trajectoryEnabled" @update:model-value="setSnap($event)">
              {{ $t('scenario.attachToLaneCenterline') }}
            </el-checkbox>
          </div>
        </el-form-item>
      </el-collapse-item>
      <BaseAuth :perm="['action.car.props.v2x.obuStatus.modify.enable']">
        <el-collapse-item id="scenario-property-basic-v2x" :title="$t('scenario.v2xConfig')" name="v2xConfig">
          <el-form-item class="param-cell" :label="$t('scenario.v2xStatus')">
            <el-switch
              id="car-obu-enabled"
              v-model="form.obuStatus"
              active-value="1"
              inactive-value="0"
            >
              {{ $t('scenario.aiControl') }}
            </el-switch>
          </el-form-item>

          <BaseAuth :perm="['action.car.props.v2x.sensorGroup.modify.enable']">
            <el-form-item v-show="form.obuStatus === '1'" class="param-cell" :label="$t('scenario.obuDevice')">
              <el-select
                id="object-property-obu-select"
                clearable
                :model-value="form.sensorGroup"
                @change="modifyObjectParams({ sensorGroup: $event })"
              >
                <el-option
                  v-for="obu in obuBoundingSensorGroups"
                  :id="`object-property-lane-option-${obu.groupName}`"
                  :key="obu.groupName"
                  :label="getObuName(obu.group[0].idx)"
                  :value="obu.groupName"
                />
              </el-select>
            </el-form-item>
          </BaseAuth>
        </el-collapse-item>
      </BaseAuth>
    </el-collapse>
    <div class="track-points-wrap">
      <el-form-item class="param-cell multi-input">
        <div class="set-track-points-btns">
          <el-button
            size="small"
            class="solid-button"
            @click="setEnd({ type: 'car', id: form.id })"
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
              <div :id="`car-track-node-${scope.$index}`" @click="handleFocusTrackItem(scope.$index)">
                <div class="position">
                  {{ (+scope.row.lon).toFixed(14) }}
                </div>
                <div class="position">
                  {{ (+scope.row.lat).toFixed(14) }}
                </div>
                <div class="position">
                  {{ (+scope.row.alt).toFixed(14) }}
                </div>
              </div>
            </template>
          </el-table-column>
          <el-table-column v-if="trajectoryEnabled" width="80">
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
              <div class="position">
                <el-select
                  :model-value="scope.row.gear"
                  :placeholder="$t('scenario.gear')"
                  :style="{ width: '72px' }"
                  @change="modifyTrack(scope.$index, scope.row.velocity, $event)"
                >
                  <el-option
                    v-for="item in gears"
                    :key="item.value"
                    :label="item.label"
                    :value="item.value"
                  />
                </el-select>
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
                  <el-icon
                    class="el-icon-delete"
                    :title="$t('operation.delete')"
                  >
                    <Delete />
                  </el-icon>
                </el-button>
              </div>
            </template>
          </el-table-column>
        </el-table>
      </el-form-item>
      <div class="property-form-item-separator" role="separator" />
      <el-form-item :label="$t('scenario.size')" class="param-cell display-properties">
        <sim-label>
          {{ $t('scenario.length') }}: {{
            toFixed(form.carLength || form.boundingBox.dimensions.length, 3)
          }}m
        </sim-label>
        <sim-label>
          {{ $t('scenario.width') }}: {{ toFixed(form.carWidth || form.boundingBox.dimensions.width, 3) }}m
        </sim-label>
        <sim-label>
          {{ $t('scenario.height') }}: {{
            toFixed(form.carHeight || form.boundingBox.dimensions.height, 3)
          }}m
        </sim-label>
      </el-form-item>
    </div>
  </el-form>
</template>

<script>
import { mapActions, mapGetters, mapState } from 'vuex'
import { cloneDeep } from 'lodash-es'
import property from './property'
import { editor } from '@/api/interface'
import NumericSliderInput from '@/components/numeric-slider-input.vue'
import filters from '@/filters'

export default {
  name: 'PropertyCar',
  components: {
    NumericSliderInput,
  },
  mixins: [property],
  data () {
    return {
      optionFollowIDList: [],
      aiModels: [
        {
          label: 'scenario.highway',
          id: 'TrafficVehicleDefault',
        },
        {
          label: 'scenario.urbanArea',
          id: 'TrafficVehicleArterial',
        },
      ],
      gears: [{
        value: 'drive',
        label: 'D',
      }, {
        value: 'reverse',
        label: 'R',
      }],
      enableSnap: true,
      activeCollapseNames: ['basicInfo', 'v2xConfig'],
    }
  },
  computed: {
    ...mapState('scenario', ['objects']),
    ...mapGetters('planners', [
      'obuBoundingSensorGroups',
    ]),
    ...mapState('planners', [
      'sensors',
    ]),
    velocityRange: {
      get () {
        return [+this.form.startVelocity, +this.form.maxVelocity]
      },
      set (value) {
        const [startVelocity, maxVelocity] = value
        const { endPosArr } = this.form
        if (endPosArr?.length > 0) {
          endPosArr.forEach((pos) => {
            pos.velocity = Math.min(startVelocity, maxVelocity)
          })
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
        if (behavior === 'TrafficVehicle' || behavior === 'TrafficVehicleArterial') {
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

        if (value !== '2') {
          // 如果不是 '自定义'，并且如果其他 object 的跟车为该 car，需要重置其他 object 的跟车。（只有自定义的car 能设置跟车）
          this.objects.forEach((object) => {
            if (object.follow && object.type === 'car') {
              const matchRes = object.follow.match(/\d+/g)

              if (matchRes && matchRes.length > 0) {
                const followId = Number(matchRes[0])

                if (followId === Number(this.form.id)) {
                  object.follow = ''
                }
              }
            }
          })
        }
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
        // 取消轨迹跟踪需要重置初始角度，故这里走modifyObject
        this.modifyObject({
          behavior,
        })
      },
    },

    aggress () {
      const aggress = this.form.aggress === undefined ? 0.5 : this.form.aggress
      return +aggress
    },
  },
  watch: {
    'form.id': {
      handler () {
        this.setOptionFollowIDList()
      },
      immediate: true,
    },
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
    pathNodeSelected (payload) {
      if (!payload) {
        this.$refs.table.setCurrentRow()
        return
      }
      const { type, index } = payload
      const el = document.getElementById(`car-track-node-${index}`)
      if (type === 'vehicle') {
        this.$refs.table.setCurrentRow(this.form.endPosArr[index])
        if (el) {
          el.scrollIntoView()
        }
      }
    },
    carTriggerConfig () {
      this.startMission('EventConfig')
    },
    setOptionFollowIDList () {
      if (!this.form) return
      // 跟车只能选择主车，这里只会返回主车
      const vehicles = editor.scenario.getCanBeFollowedVehicleIDs(this.form.id)
      const optionFollowIDList = [
        { id: '', name: 'none' },
      ]
      vehicles.forEach(({ id, type }) => {
        const name = filters.$itemName({ id, type })
        const group = filters.$followName({ id, type }) // ego_1 、 v_1

        optionFollowIDList.push({
          id: `${group}`,
          name,
        })
      })
      this.optionFollowIDList = optionFollowIDList
    },
    handleFocusTrackItem (index) {
      this.selectPathEnd({
        type: 'car',
        id: this.form.id,
        index,
      })
    },
    handleDeleteTrackItem (index) {
      this.removePathPoint({
        type: 'car',
        id: this.form.id,
        index,
        endPosArr: this.form.endPosArr,
      })
    },
    modifyTrack (idx, velocity, gear) {
      const endPosArr = cloneDeep(this.form.endPosArr)
      endPosArr[idx].velocity = velocity
      endPosArr[idx].gear = gear

      let maxVelocity = this.form.maxVelocity
      maxVelocity = Math.max(maxVelocity, velocity)

      if (idx === 0) {
        this.modifyObjectParams({
          startVelocity: velocity,
          maxVelocity,
          endPosArr,
        })
      } else {
        this.modifyObjectParams({
          maxVelocity,
          endPosArr,
        })
      }
    },
    getObuName (idx) {
      const obu = this.sensors.find(s => s.idx === idx)
      return obu?.name || ''
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.property-car {
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

  .track-points-wrap {
    padding: 20px;
  }
}
</style>
