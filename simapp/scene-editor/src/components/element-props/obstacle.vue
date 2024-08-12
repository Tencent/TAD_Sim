<template>
  <el-form
    v-if="form"
    ref="form"
    class="property-form property-obstacle"
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
        <el-form-item prop="transfer" :label="$t('scenario.longitudinalDistance')" class="param-cell">
          <InputNumber
            :model-value="form.transfer"
            unit="m"
            :precision="3"
            :min="0"
            :max="maxShift"
            @update:model-value="modifyObject({ transfer: $event })"
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

        <el-form-item v-if="false" prop="direction" :label="$t('scenario.angle')" class="param-cell">
          <InputNumber
            :model-value="form.direction"
            unit="°"
            :min="0"
            :max="360"
            :precision="1"
            @update:model-value="modifyObject({ direction: $event })"
          />
        </el-form-item>
        <el-form-item prop="start_angle" :label="$t('scenario.startAngle')" class="param-cell">
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
        <div class="property-form-item-separator" />
        <el-form-item :label="$t('scenario.size')" class="param-cell display-properties">
          <sim-label>{{ $t('scenario.length') }}: {{ toFixed(form.length, 3) }}m</sim-label>
          <sim-label>{{ $t('scenario.width') }}: {{ toFixed(form.width, 3) }}m</sim-label>
          <sim-label>{{ $t('scenario.height') }}: {{ toFixed(form.height, 3) }}m</sim-label>
        </el-form-item>
      </el-collapse-item>
    </el-collapse>
  </el-form>
</template>

<script>
import property from './property'

export default {
  name: 'PropertyObstacle',
  mixins: [property],
  data () {
    return {}
  },
  methods: {},
}
</script>
