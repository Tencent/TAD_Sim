<!-- v-if 渲染 el-form-item 组件时，设置的规则可能会不生效，解决方法：为 el-form-item 添加 key -->
<template>
  <div class="item-1-body event-behavior-config">
    <el-form ref="form" :model="data">
      <el-form-item :label="`行为${index + 1}`" prop="actiontype" :rules="rules.actiontype">
        <el-select :model-value="data.actiontype" @change="onActionTypeChange($event)">
          <el-option
            v-for="behavior in behaviorTypeFilteredList"
            :key="behavior.value"
            :label="$t(behavior.label)"
            :value="behavior.value"
          />
        </el-select>
      </el-form-item>
      <template v-if="data.actiontype === 'velocity'">
        <el-form-item :label="$t('scenario.speedVariation')" prop="actionvalue" :rules="rules.speedValue">
          <InputNumber
            :model-value="data.actionvalue"
            :precision="2"
            :min="-66.68"
            :max="66.68"
            unit="m/s"
            :allow-empty-value="true"
            @update:model-value="updateData({ actionvalue: $event })"
          />
        </el-form-item>
      </template>
      <template v-if="data.actiontype === 'acc'">
        <el-form-item :label="$t('scenario.accVariation')" prop="actionvalue" :rules="rules.accValue">
          <InputNumber
            :model-value="data.actionvalue"
            :precision="2"
            unit="m/s²"
            :allow-empty-value="true"
            @update:model-value="updateData({ actionvalue: $event })"
          />
        </el-form-item>
        <el-form-item :label="$t('scenario.endCondition')">
          <el-select
            :model-value="data.endconditiontype"
            :style="{
              width: data.endconditiontype === 'none' ? '100%' : '70px',
              marginRight: data.endconditiontype === 'none' ? '0' : '5px',
            }"
            @change="updateData({ endconditiontype: $event })"
          >
            <el-option
              v-for="endCondition in endConditionList"
              :key="endCondition.value"
              :label="$t(endCondition.label)"
              :value="endCondition.value"
            />
          </el-select>
          <InputNumber
            v-show="data.endconditiontype !== 'none'"
            :model-value="data.endconditionvalue"
            :precision="2"
            :min="data.endconditiontype === 'time' ? 0 : -33.4 "
            :max="data.endconditiontype === 'time' ? undefined : 33.4"
            :unit="data.endconditiontype === 'time' ? 's' : 'm/s'"
            style="width:40%"
            @update:model-value="updateData({ endconditionvalue: $event })"
          />
        </el-form-item>
      </template>
      <template v-if="data.actiontype === 'merge'">
        <el-form-item :label="$t('scenario.posOffset')" prop="subtype" :rules="rules.mergeSubType">
          <el-select :model-value="data.subtype" @change="updateData({ subtype: $event })">
            <el-option
              v-for="offset in offsetList"
              :key="offset.value"
              :label="$t(offset.label)"
              :value="offset.value"
            />
          </el-select>
        </el-form-item>
        <!--    变道时间    -->
        <el-form-item :label="$t('scenario.laneChangingTime')" prop="actionvalue" :rules="rules.mergeValue">
          <InputNumber
            :model-value="data.actionvalue"
            :min="0"
            :precision="2"
            unit="s"
            :allow-empty-value="true"
            @update:model-value="updateData({ actionvalue: $event })"
          />
        </el-form-item>
        <!--    道内偏移    -->
        <el-form-item
          v-if="data.subtype.includes('in_lane')"
          :label="$t('scenario.insideLaneOffset')"
          prop="offset"
          :rules="rules.mergeLaneOffset"
        >
          <InputNumber
            :model-value="data.offset"
            :precision="2"
            :min="0"
            :max="2.5"
            unit="m"
            :allow-empty-value="true"
            :disabled="!['left_in_lane', 'right_in_lane'].includes(data.subtype)"
            @update:model-value="updateData({ offset: $event })"
          />
        </el-form-item>
      </template>
      <template v-if="data.actiontype === 'lateralDistance'">
        <el-form-item :label="$t('object')" prop="multi.target_element" :rules="rules.targetElement">
          <el-select :model-value="data.multi.target_element" @change="updateMulti({ target_element: $event })">
            <el-option
              v-for="target in targetElementList"
              :key="target.value"
              :label="target.label"
              :value="target.value"
            />
          </el-select>
        </el-form-item>
        <el-form-item :label="$t('scenario.distanceVariation')" prop="actionvalue" :rules="rules.distanceVariation">
          <InputNumber
            :model-value="data.actionvalue"
            :precision="2"
            :min="-100"
            :max="100"
            unit="m"
            allow-empty-value
            @update:model-value="updateData({ actionvalue: $event })"
          />
        </el-form-item>
        <el-form-item :label="$t('scenario.continues')" prop="multi.continuous" :rules="rules.continuous">
          <el-switch :model-value="data.multi.continuous" @update:model-value="updateMulti({ continuous: $event })" />
        </el-form-item>
        <el-form-item prop="multi.maxAcc" :rules="rules.maxAcc">
          <template #label>
            {{ $t('scenario.maxAcceleration') }}
            <el-tooltip
              style="margin-left: 5px"
              effect="dark"
              :content="$t('tips.theMeaningOfMinus1')"
              placement="top"
            >
              <el-icon class="el-icon-info">
                <warning />
              </el-icon>
            </el-tooltip>
          </template>
          <InputNumber
            :model-value="data.multi.maxAcc"
            :precision="2"
            :min="-1"
            :max="100"
            :external-rule="checkIfToMinus1"
            unit="m/s²"
            @update:model-value="updateMulti({ maxAcc: $event })"
          />
        </el-form-item>
        <el-form-item prop="multi.maxDec" :rules="rules.maxDec">
          <template #label>
            {{ $t('scenario.minAcceleration') }}
            <el-tooltip
              style="margin-left: 5px"
              effect="dark"
              :content="$t('tips.theMeaningOfMinus1')"
              placement="top"
            >
              <el-icon class="el-icon-info">
                <warning />
              </el-icon>
            </el-tooltip>
          </template>
          <InputNumber
            :model-value="data.multi.maxDec"
            :precision="2"
            :min="-1"
            :max="100"
            :external-rule="checkIfToMinus1"
            unit="m/s²"
            @update:model-value="updateMulti({ maxDec: $event })"
          />
        </el-form-item>
        <el-form-item prop="multi.maxSpeed" :rules="rules.maxSpeed">
          <template #label>
            {{ $t('scenario.maxVelocity') }}
            <el-tooltip
              style="margin-left: 5px"
              effect="dark"
              :content="$t('tips.theMeaningOfMinus1')"
              placement="top"
            >
              <el-icon class="el-icon-info">
                <warning />
              </el-icon>
            </el-tooltip>
          </template>
          <InputNumber
            :model-value="data.multi.maxSpeed"
            :precision="2"
            :min="-1"
            :max="100"
            :external-rule="checkIfToMinus1"
            unit="m/s"
            @update:model-value="updateMulti({ maxSpeed: $event })"
          />
        </el-form-item>
      </template>
      <template v-if="data.actiontype === 'activate'">
        <el-form-item
          label="横向控制"
        >
          <el-switch v-model="latControl" />
        </el-form-item>
        <el-form-item
          label="纵向控制"
        >
          <el-switch v-model="longControl" />
        </el-form-item>
      </template>
      <template v-if="data.actiontype === 'assign'">
        <el-form-item label="控制器" prop="actionvalue" :rules="rules.assignValue">
          <el-select :model-value="data.actionvalue" @change="updateData({ actionvalue: $event })">
            <el-option
              v-for="scheme in mainCarModuleSetList"
              :key="scheme.id"
              :label="scheme.name"
              :value="scheme.name"
            />
          </el-select>
        </el-form-item>
        <el-form-item label="期望速度" prop="multi.set_speed">
          <InputNumber
            :model-value="data.multi.set_speed"
            :precision="2"
            :min="0"
            :max="100"
            unit="m/s"
            allow-empty-value
            @update:model-value="updateMulti({ set_speed: $event })"
          />
        </el-form-item>
        <el-form-item label="跟车时距" prop="multi.set_timegap">
          <InputNumber
            :model-value="data.multi.set_timegap"
            :precision="2"
            :min="0"
            :max="20"
            unit="s"
            allow-empty-value
            @update:model-value="updateMulti({ set_timegap: $event })"
          />
        </el-form-item>
        <el-form-item label="重启">
          <el-switch :model-value="data.multi.resume_sw" @change="updateMulti({ resume_sw: $event })" />
        </el-form-item>
        <el-form-item label="取消">
          <el-switch :model-value="data.multi.cancel_sw" @change="updateMulti({ cancel_sw: $event })" />
        </el-form-item>
        <el-form-item label="速度增加">
          <el-switch :model-value="data.multi.speed_inc_sw" @change="updateMulti({ speed_inc_sw: $event })" />
        </el-form-item>
        <el-form-item label="速度减少">
          <el-switch :model-value="data.multi.speed_dec_sw" @change="updateMulti({ speed_dec_sw: $event })" />
        </el-form-item>
      </template>
      <template v-if="data.actiontype === 'override'">
        <el-form-item class="multi-item">
          <div class="first-item">
            <el-checkbox
              :model-value="orThrottle.checked"
              @change="updateOverrideMulti({ throttle: $event })"
            >
              油门
            </el-checkbox>
          </div>
          <InputNumber
            :model-value="orThrottle.value"
            :precision="2"
            :min="0"
            :max="100"
            unit="%"
            @update:model-value="updateOverrideMulti({ throttle: $event })"
          />
        </el-form-item>
        <el-form-item class="multi-item">
          <div class="first-item">
            <el-checkbox
              :model-value="orBrake.checked"
              @change="updateOverrideMulti({ brake: $event })"
            >
              制动
            </el-checkbox>
          </div>
          <InputNumber
            :model-value="orBrake.value"
            :precision="2"
            :min="0"
            :max="100"
            unit="%"
            @update:model-value="updateOverrideMulti({ brake: $event })"
          />
        </el-form-item>
        <el-form-item class="multi-item">
          <div class="first-item">
            <el-checkbox
              :model-value="orSteeringWheel.checked"
              @change="updateOverrideMulti({ steering_wheel: $event })"
            >
              转向
            </el-checkbox>
          </div>
          <InputNumber
            :model-value="orSteeringWheel.value"
            :precision="2"
            :min="-900"
            :max="900"
            unit="rad"
            @update:model-value="updateOverrideMulti({ steering_wheel: $event })"
          />
        </el-form-item>
        <el-form-item class="multi-item">
          <div class="first-item">
            <el-checkbox
              :model-value="orParkingBrake.checked"
              @change="updateOverrideMulti({ parking_brake: $event })"
            >
              驻车制动
            </el-checkbox>
          </div>
          <el-select
            :model-value="orParkingBrake.value"
            @change="updateOverrideMulti({ parking_brake: $event })"
          >
            <el-option
              v-for="pb of parkingBrakeList"
              :key="pb.value"
              :value="Number(pb.value)"
              :label="pb.label"
            />
          </el-select>
        </el-form-item>
        <el-form-item class="multi-item">
          <div class="first-item">
            <el-checkbox
              :model-value="orClutch.checked"
              @change="updateOverrideMulti({ clutch: $event })"
            >
              离合器
            </el-checkbox>
          </div>
          <InputNumber
            :model-value="orClutch.value"
            :precision="2"
            :min="0"
            :max="100"
            unit="%"
            @update:model-value="updateOverrideMulti({ clutch: $event })"
          />
        </el-form-item>
        <el-form-item class="multi-item">
          <div class="first-item">
            <el-checkbox
              :model-value="orGear.checked"
              @change="updateOverrideMulti({ gear: $event })"
            >
              档位
            </el-checkbox>
          </div>
          <el-select
            :model-value="orGear.value"
            @change="updateOverrideMulti({ gear: $event })"
          >
            <el-option
              v-for="gear of gearList"
              :key="gear.id"
              :value="gear.id"
              :label="gear.name"
            />
          </el-select>
        </el-form-item>
      </template>
      <template v-if="data.actiontype === 'command'">
        <el-form-item label="指令类型">
          <el-select v-model="commandSubtype" @change="onCommandSubtypeChange">
            <el-option
              v-for="type in commandTypeList"
              :key="type.value"
              :label="$t(type.label)"
              :value="type.value"
            />
          </el-select>
        </el-form-item>
        <template v-if="data.subtype === 'lane_change'">
          <el-form-item label="发送指令">
            <el-switch
              :model-value="data.actionvalue"
              active-value="true"
              inactive-value="false"
              @change="updateData({ actionvalue: $event })"
            />
          </el-form-item>
        </template>
        <template v-if="data.subtype === 'overtaking'">
          <el-form-item label="发送指令">
            <el-switch
              :model-value="data.actionvalue"
              active-value="true"
              inactive-value="false"
              @change="updateData({ actionvalue: $event })"
            />
          </el-form-item>
        </template>
        <template v-if="commandSubtype.startsWith('lateral_')">
          <el-form-item :label="$t('scenario.endCondition')">
            <el-select
              v-model="commandValue"
              :style="{ width: '50px', marginRight: '5px' }"
            >
              <el-option :label="$t('left')" value="0" />
              <el-option :label="$t('right')" value="1" />
            </el-select>
            <InputNumber
              :model-value="data.actionvalue"
              :precision="2"
              :min="0"
              :max="100"
              :unit="commandSubtype.startsWith('lateral_speed') ? 'm/s' : 'm/s²'"
              @update:model-value="updateData({ actionvalue: $event })"
            />
          </el-form-item>
        </template>
        <template v-if="commandSubtype === 'userdefine'">
          <el-form-item key="customCommand" label="自定义指令" prop="actionvalue" :rules="rules.commandValue">
            <el-input
              :model-value="data.actionvalue"
              @input="updateData({ actionvalue: $event })"
            />
          </el-form-item>
        </template>
      </template>
      <template v-if="data.actiontype === 'status'">
        <el-form-item label="状态类型">
          <el-select :model-value="data.subtype" @change="onStatusSubtypeChange">
            <el-option
              v-for="status in statusTypeList"
              :key="status.value"
              :label="$t(status.label)"
              :value="status.value"
            />
          </el-select>
        </el-form-item>
        <el-form-item v-if="data.subtype !== 'userdefine'" label="状态">
          <el-switch
            v-if="boolTypeStatusList.includes(data.subtype)"
            :model-value="data.actionvalue"
            active-value="true"
            inactive-value="false"
            @change="updateData({ actionvalue: $event })"
          />
          <el-select v-else :model-value="data.actionvalue" @change="updateData({ actionvalue: $event })">
            <template v-if="data.subtype === 'beam'">
              <el-option
                v-for="status of beamStatusList"
                :key="status.value"
                :value="status.value"
                :label="status.label"
              />
            </template>
            <template v-if="data.subtype === 'parkingbrake'">
              <el-option
                v-for="status of parkingBrakeList"
                :key="status.value"
                :value="status.value"
                :label="status.label"
              />
            </template>
            <template v-if="data.subtype === 'wiper'">
              <el-option
                v-for="status of wiperStatusList"
                :key="status.value"
                :value="status.value"
                :label="status.label"
              />
            </template>
            <template v-if="data.subtype === 'gear'">
              <el-option v-for="status of gearList" :key="status.id" :value="String(status.id)" :label="status.name" />
            </template>
            <template v-if="data.subtype === 'key'">
              <el-option
                v-for="status of keyStatusList"
                :key="status.value"
                :value="status.value"
                :label="status.label"
              />
            </template>
          </el-select>
        </el-form-item>
        <el-form-item
          v-if="data.subtype === 'userdefine'"
          key="customStatus"
          label="状态"
          prop="actionvalue"
          :rules="rules.statusValue"
        >
          <el-input
            :model-value="data.actionvalue"
            @input="updateData({ actionvalue: $event })"
          />
        </el-form-item>
      </template>
      <el-icon class="el-icon-delete" @click="$emit('delete')">
        <delete />
      </el-icon>
    </el-form>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapGetters, mapState } from 'vuex'
import InputNumber from '@/components/input-number.vue'
import dict from '@/common/dict'
import filters from '@/filters'

const { $itemName } = filters

const behaviorTypeList = [
  // { value: 'velocity', label: 'triggerEvent.behaviorType.speed' },
  // { value: 'acc', label: 'triggerEvent.behaviorType.acceleration' },
  // { value: 'merge', label: 'triggerEvent.behaviorType.posOffset' },
  { value: 'activate', label: 'triggerEvent.behaviorType.activateController' },
  // todo: 调度暂时没好，先屏蔽
  // { value: 'assign', label: 'triggerEvent.behaviorType.assignController' },
  { value: 'override', label: 'triggerEvent.behaviorType.takeOverController' },
  { value: 'command', label: 'triggerEvent.behaviorType.userInstructions' },
  { value: 'status', label: 'triggerEvent.behaviorType.sendStatus' },
]

const carBehaviorTypeList = [
  { value: 'velocity', label: 'triggerEvent.behaviorType.speed' },
  { value: 'acc', label: 'triggerEvent.behaviorType.acceleration' },
  { value: 'merge', label: 'triggerEvent.behaviorType.posOffset' },
  { value: 'lateralDistance', label: 'triggerEvent.behaviorType.lateralDistance' },
  // { value: 'status', label: 'triggerEvent.behaviorType.sendStatus' },
]

const machineBehaviorTypeList = [
  { value: 'velocity', label: 'triggerEvent.behaviorType.speed' },
  { value: 'acc', label: 'triggerEvent.behaviorType.acceleration' },
  // { value: 'lateralDistance', label: 'triggerEvent.behaviorType.lateralDistance' },
  // { value: 'merge', label: 'triggerEvent.behaviorType.posOffset' },
  // { value: 'status', label: 'triggerEvent.behaviorType.sendStatus' },
]

const endConditionList = [
  { value: 'none', label: 'none' },
  { value: 'time', label: 'time' },
  { value: 'velocity', label: 'velocity' },
]

const offsetList = [
  // { value: 'none', label: 'None' },
  { value: 'left', label: 'scenario.moveToTheLeftLane' },
  { value: 'right', label: 'scenario.moveToTheRightLane' },
  { value: 'left_in_lane', label: 'scenario.moveToTheLeftOfTheLane' },
  { value: 'right_in_lane', label: 'scenario.moveToTheRightOfTheLane' },
]

const commandTypeList = [
  { value: 'lane_change_left', label: '向左变道' },
  { value: 'lane_change_right', label: '向右变道' },
  { value: 'lane_change_any', label: '任意方向变道' },
  { value: 'overtaking_left', label: '向左超车' },
  { value: 'overtaking_right', label: '向右超车' },
  { value: 'overtaking_any', label: '任意方向超车' },
  { value: 'pull_over', label: '靠边停车' },
  { value: 'emergency_stop', label: '紧急停车' },
  { value: 'lateral_speed', label: '横向速度' },
  { value: 'lateral_accel', label: '横向加速度' },
  { value: 'userdefine', label: '自定义' },
]

const statusTypeList = [
  { value: 'emergency_brake', label: '紧急制动' },
  { value: 'vehicle_lost_control', label: '车辆失控' },
  { value: 'hands_on_steeringwheel', label: '双手握持方向盘' },
  { value: 'eyes_on_road', label: '视线在道路' },
  { value: 'lidar', label: '激光雷达' },
  { value: 'radar', label: '毫米波雷达' },
  { value: 'ultrasonic', label: '超声波雷达' },
  { value: 'camera', label: '摄像头' },
  { value: 'gnss', label: 'GNSS' },
  { value: 'imu', label: 'IMU' },
  { value: 'localization', label: '定位' },
  { value: 'beam', label: '行车灯' },
  { value: 'brake_light', label: '制动灯' },
  { value: 'hazard_light', label: '危险报警灯' },
  { value: 'left_turn_light', label: '左转灯' },
  { value: 'right_turn_light', label: '右转灯' },
  { value: 'driver_seatbelt', label: '主驾安全带' },
  { value: 'passenger_seatbelt', label: '副驾安全带' },
  { value: 'driver_door', label: '主驾车门' },
  { value: 'passenger_door', label: '副驾车门' },
  { value: 'hood', label: '前机箱盖' },
  { value: 'trunk', label: '后备箱盖' },
  { value: 'parkingbrake', label: '驻车制动' },
  { value: 'wiper', label: '雨刮器' },
  { value: 'gear', label: '档位' },
  { value: 'key', label: '车辆点火' },
  { value: 'userdefine', label: '自定义' },
]

const boolTypeCommandList = ['lane_change_left', 'lane_change_right', 'lane_change_any', 'overtaking_left', 'overtaking_right', 'overtaking_any', 'overtaking']

const boolTypeStatusList = [
  'emergency_brake',
  'vehicle_lost_control',
  'hands_on_steeringwheel',
  'eyes_on_road',
  'lidar',
  'radar',
  'ultrasonic',
  'camera',
  'gnss',
  'imu',
  'localization',
  'brake_light',
  'hazard_light',
  'left_turn_light',
  'right_turn_light',
  'driver_seatbelt',
  'passenger_seatbelt',
  'driver_door',
  'passenger_door',
  'hood',
  'trunk',
]

const beamStatusList = [
  { value: '0', label: '关闭' },
  { value: '1', label: '自动' },
  { value: '2', label: '近光灯' },
  { value: '3', label: '远光灯' },
]

const wiperStatusList = [
  { value: '0', label: 'off' },
  { value: '1', label: 'low' },
  { value: '2', label: 'med' },
  { value: '3', label: 'high' },
]

const keyStatusList = [
  { value: '0', label: 'off' },
  { value: '1', label: 'ign' },
  { value: '2', label: 'acc' },
  { value: '3', label: 'on' },
]

const gearList = dict.gearList

const parkingBrakeList = [
  { value: '0', label: '已释放' },
  { value: '1', label: '已拉起' },
  { value: '2', label: '拉起中' },
  { value: '3', label: '释放中' },
]

export default {
  name: 'EventBehaviorConfig',
  components: {
    InputNumber,
  }, // 不声明 emits 可能会导致某些第三方代码手动发送事件从而报错
  props: {
    index: {
      type: Number,
      default: 0,
    },
    data: {
      type: Object,
      required: true,
    },
  },
  emits: ['delete', 'change'],
  data () {
    return {
      endConditionList,
      offsetList,
      commandTypeList,
      statusTypeList,
      gearList,
      parkingBrakeList,
      boolTypeCommandList,
      boolTypeStatusList,
      beamStatusList,
      wiperStatusList,
      keyStatusList,
      rules: {
        actiontype: [
          {
            required: true,
            message: '请选择事件行为。',
          },
        ],
        speedValue: [
          {
            required: true,
            message: '请输入速度变化。',
            trigger: 'blur',
          },
        ],
        accValue: [
          {
            required: true,
            message: '请输入加速度变化。',
            trigger: 'blur',
          },
        ],
        mergeSubType: [
          {
            required: true,
            message: '请选择偏移类型。',
          },
        ],
        mergeValue: [
          {
            required: true,
            message: '请输入变道时间。',
            trigger: 'blur',
          },
        ],
        mergeLaneOffset: [
          {
            required: true,
            message: '请输入道内偏移。',
            trigger: 'blur',
          },
        ],
        assignValue: [
          {
            validator (rule, value, callback) {
              if (value === 'none') {
                callback(new Error('请选择控制器。'))
              } else {
                callback()
              }
            },
          },
        ],
        commandValue: [
          {
            required: true,
            message: '请输入自定义指令',
          },
        ],
        statusValue: [
          {
            required: true,
            message: '请输入自定义状态',
          },
        ],
        targetElement: [
          {
            required: true,
            message: '请选择对象',
          },
        ],
        distanceVariation: [
          {
            required: true,
            message: '请输入距离变化',
            trigger: 'blur',
          },
        ],
        continuous: [
          {
            required: true,
            message: '请选择是否连续',
          },
        ],
        maxAcc: [
          {
            required: true,
            message: '请输入最大加速度',
            trigger: 'blur',
          },
        ],
        maxDec: [
          {
            required: true,
            message: '请输入最大减速度',
            trigger: 'blur',
          },
        ],
        maxSpeed: [
          {
            required: true,
            message: '请输入最大速度',
            trigger: 'blur',
          },
        ],
      },
    }
  },
  computed: {
    ...mapState('scenario', ['objects']),
    ...mapGetters('scenario', [
      'selectedObject',
    ]),
    ...mapGetters('module-set', ['mainCarModuleSetList']),
    latControl: {
      get () {
        switch (this.data.subtype) {
          case 'autopilot':
          case 'lateral':
            return true
          default:
            return false
        }
      },
      set (val) {
        const { longControl } = this
        let subtype
        switch (true) {
          case val && longControl:
            subtype = 'autopilot'
            break
          case val && !longControl:
            subtype = 'lateral'
            break
          case !val && longControl:
            subtype = 'longitudinal'
            break
          case !val && !longControl:
            subtype = 'off'
            break
          default:
            subtype = ''
        }
        this.updateData({ subtype })
      },
    },
    longControl: {
      get () {
        switch (this.data.subtype) {
          case 'autopilot':
          case 'longitudinal':
            return true
          default:
            return false
        }
      },
      set (val) {
        const { latControl } = this
        let subtype
        switch (true) {
          case val && latControl:
            subtype = 'autopilot'
            break
          case val && !latControl:
            subtype = 'longitudinal'
            break
          case !val && latControl:
            subtype = 'lateral'
            break
          case !val && !latControl:
            subtype = 'off'
            break
          default:
            subtype = ''
        }
        this.updateData({ subtype })
      },
    },
    commandSubtype: {
      get () {
        const {
          offset,
          subtype,
          actiontype,
        } = this.data
        if (actiontype !== 'command') return null
        if (subtype === 'lane_change') {
          switch (offset) {
            case '0':
              return 'lane_change_left'
            case '1':
              return 'lane_change_right'
            case '2':
              return 'lane_change_any'
            default:
              return null
          }
        }
        if (subtype === 'overtaking') {
          switch (offset) {
            case '0':
              return 'overtaking_left'
            case '1':
              return 'overtaking_right'
            case '2':
              return 'overtaking_any'
            default:
              return null
          }
        }
        if (['lateral_speed_to_left', 'lateral_speed_to_right'].includes(subtype)) {
          return 'lateral_speed'
        }
        if (['lateral_accel_to_left', 'lateral_accel_to_right'].includes(subtype)) {
          return 'lateral_accel'
        }
        return subtype
      },
      set (value) {
        const { actionvalue: checked } = this.data
        let subtype = value
        let actionvalue = ''
        let offset = ''
        switch (value) {
          case 'lane_change_left':
            subtype = 'lane_change'
            actionvalue = checked
            offset = '0'
            break
          case 'lane_change_right':
            subtype = 'lane_change'
            actionvalue = checked
            offset = '1'
            break
          case 'lane_change_any':
            subtype = 'lane_change'
            actionvalue = checked
            offset = '2'
            break
          case 'overtaking_left':
            subtype = 'overtaking'
            actionvalue = checked
            offset = '0'
            break
          case 'overtaking_right':
            subtype = 'overtaking'
            actionvalue = checked
            offset = '1'
            break
          case 'overtaking_any':
            subtype = 'overtaking'
            actionvalue = checked
            offset = '2'
            break
          case 'lateral_speed':
            subtype = 'lateral_speed_to_left'
            actionvalue = '0'
            break
          case 'lateral_accel':
            subtype = 'lateral_accel_to_left'
            actionvalue = '0'
            break
          case 'customize':
            subtype = ''
            actionvalue = ''
            break
          case 'pull_over':
          case 'emergency_stop':
          default:
        }
        this.updateData({
          actionvalue,
          subtype,
          offset,
        })
      },
    },
    commandValue: {
      get () {
        const {
          subtype,
        } = this.data
        if (['lateral_speed_to_left', 'lateral_accel_to_left'].includes(subtype)) {
          return '0'
        }
        if (['lateral_speed_to_right', 'lateral_accel_to_right'].includes(subtype)) {
          return '1'
        }
        return null
      },
      set (value) {
        const {
          actiontype,
          subtype,
        } = this.data
        if (actiontype !== 'command') return
        let newSubType
        if (subtype.startsWith('lateral_speed_to')) {
          switch (value) {
            case '0':
              newSubType = 'lateral_speed_to_left'
              break
            case '1':
              newSubType = 'lateral_speed_to_right'
              break
          }
        }
        if (subtype.startsWith('lateral_accel_to')) {
          switch (value) {
            case '0':
              newSubType = 'lateral_accel_to_left'
              break
            case '1':
              newSubType = 'lateral_accel_to_right'
              break
          }
        }
        this.updateData({
          subtype: newSubType,
        })
      },
    },
    statusSubType: {
      get () {
        const {
          actiontype,
          subtype,
        } = this.data
        if (actiontype !== 'status') return null
        return subtype
      },
      set (value) {
        this.updateData({ subtype: value })
      },
    },
    orThrottle () {
      const throttle = _.get(this, 'data.multi.throttle', 'false;0')
      let [checked, value] = throttle.split(';')
      checked = checked === 'true'
      value = Number(value)
      return { checked, value }
    },
    orBrake () {
      const brake = _.get(this, 'data.multi.brake', 'false;0')
      let [checked, value] = brake.split(';')
      checked = checked === 'true'
      value = Number(value)
      return { checked, value }
    },
    orSteeringWheel () {
      const steeringWheel = _.get(this, 'data.multi.steering_wheel', 'false;0')
      let [checked, value] = steeringWheel.split(';')
      checked = checked === 'true'
      value = Number(value)
      return { checked, value }
    },
    orParkingBrake () {
      const parkingBrake = _.get(this, 'data.multi.parking_brake', 'false;0')
      let [checked, value] = parkingBrake.split(';')
      checked = checked === 'true'
      value = Number(value)
      return { checked, value }
    },
    orClutch () {
      const clutch = _.get(this, 'data.multi.clutch', 'false;0')
      let [checked, value] = clutch.split(';')
      checked = checked === 'true'
      value = Number(value)
      return { checked, value }
    },
    orGear () {
      const gear = _.get(this, 'data.multi.gear', 'false;0')
      let [checked, value] = gear.split(';')
      checked = checked === 'true'
      value = Number(value)
      return { checked, value }
    },
    isTrafficCar () {
      return _.get(this, 'selectedObject.type') === 'car'
    },
    isMachine () {
      return _.get(this, 'selectedObject.type') === 'machine'
    },
    behaviorTypeFilteredList () {
      if (this.isTrafficCar) {
        return carBehaviorTypeList
      } else if (this.isMachine) {
        return machineBehaviorTypeList
      }
      return behaviorTypeList
    },
    targetElementList () {
      return this.objects.filter((o) => {
        if (o.id === this.selectedObject.id && o.type === this.selectedObject.type) {
          return false
        }
        return !['signlight', 'obstacle'].includes(o.type)
      })
        .map((o) => {
          return {
            value: o.type === 'planner' ? `ego_${o.id}` : `${o.type === 'car' ? 'v' : 'p'}_${o.id}`,
            label: $itemName(o),
          }
        })
    },
  },
  methods: {
    updateData (obj) {
      this.$emit('change', { ...this.data, ...obj })
    },
    updateMulti (obj) {
      this.$emit('change', { ...this.data, multi: { ...this.data.multi, ...obj } })
    },
    updateOverrideMulti (obj) {
      const { multi = {} } = this.data
      const prop = {}
      Object.entries(obj).forEach(([key, value]) => {
        let bool = false
        let num = 0
        if (multi[key]) {
          [bool, num] = multi[key].split(';')
        }
        if (_.isBoolean(value)) {
          bool = `${value}`
        } else if (_.isNumber(value)) {
          num = `${value}`
        }
        prop[key] = `${bool};${num}`
      })
      this.updateMulti(prop)
    },
    computeMulti (multiSum) {
      if (multiSum === 'false') {
        this.updateMulti({ isValid: false })
      } else {
        this.updateMulti({ isValid: true })
      }
    },
    onActionTypeChange (type) {
      this.$refs.form.clearValidate()
      const data = {
        actionid: this.data.actionid,
        actiontype: type,
        actionvalue: '',
        subtype: '',
        multi: {
          isValid: false,
        },
        offset: '',
      }
      switch (type) {
        case 'velocity':
          data.actionvalue = ''
          data.multi = {
            isValid: false,
          }
          break
        case 'acc':
          data.actionvalue = ''
          data.endconditiontype = 'none'
          data.endconditionvalue = ''
          break
        case 'merge':
          data.subtype = ''
          data.actionvalue = ''
          data.offset = ''
          break
        case 'activate':
          data.subtype = 'off'
          break
        case 'assign':
          data.actionvalue = 'none'
          data.multi = {
            isValid: true,
            set_speed: '',
            set_timegap: '',
            resume_sw: false,
            cancel_sw: false,
            speed_inc_sw: false,
            speed_dec_sw: false,
          }
          break
        case 'override':
          data.actionvalue = 'none'
          data.multi = {
            isValid: true,
            throttle: 'false;0',
            brake: 'false;0',
            steering_wheel: 'false;0',
            parking_brake: 'false;0',
            clutch: 'false;0',
            gear: 'false;0',
          }
          break
        case 'command':
          data.actionvalue = 'true'
          data.offset = '0'
          data.subtype = 'lane_change'
          break
        case 'status':
          data.subtype = 'emergency_brake'
          data.actionvalue = 'true'
          break
        case 'lateralDistance':
          data.actionvalue = ''
          data.subtype = 'none'
          data.offset = '0'
          data.endconditiontype = 'none'
          data.endconditionvalue = '0.0'
          data.multi.target_element = ''
          data.multi.maxAcc = '-1'
          data.multi.maxDec = '-1'
          data.multi.maxSpeed = '-1'
          data.multi.continuous = true
          break
        default:
      }
      this.$emit('change', data)
    },
    onCommandSubtypeChange () {
      this.$refs.form.clearValidate()
    },
    onStatusSubtypeChange (subtype) {
      const obj = { subtype }
      if (this.boolTypeStatusList.includes(subtype)) {
        if (!['true', 'false'].includes(obj.actionvalue)) {
          obj.actionvalue = 'true'
        }
      } else if (subtype === 'userdefine') {
        obj.actionvalue = ''
      } else {
        obj.actionvalue = '0'
      }
      this.updateData(obj)
      this.$refs.form.clearValidate()
    },
    async validate () {
      return this.$refs.form.validate()
    },
    checkIfToMinus1 (val) {
      val = Number(val)
      return val > -1 && val < 0 ? -1 : val
    },
  },
}
</script>

<style scoped lang="less">
  .event-behavior-config.item-1-body {
    border-bottom: 1px solid #313131;
    padding-right: 20px;
    position: relative;
    z-index: 1;

    .el-icon-delete {
      position: absolute;
      z-index: 2;
      right: 0;
      top: 5px;
      cursor: pointer;
    }

    .multi-item {
      :deep(.el-form-item__content) {
        flex: 1;
      }

      .first-item {
        width: 151px;
      }

      .el-select {
        flex: 1;
        width: auto;
      }
    }
  }
</style>
