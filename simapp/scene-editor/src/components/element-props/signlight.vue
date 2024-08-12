<template>
  <el-form
    v-if="form"
    ref="form"
    class="property-form property-signlight"
    size="small"
    :model="form"
    :rules="rules"
    label-width="92px"
    @submit.prevent
  >
    <el-collapse v-model="activeCollapseNames">
      <el-collapse-item id="scenario-property-basic-info" :title="$t('scenario.basicInformation')" name="basicInfo">
        <el-form-item prop="startShift" :label="$t('scenario.longitudinalDistance')" class="param-cell">
          <InputNumber
            :model-value="form.startShift"
            unit="m"
            :precision="3"
            :min="0"
            :max="maxShift"
            :disabled="true"
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
            :disabled="true"
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
            :precision="3"
            :disabled="true"
            @update:model-value="modifyObject({ direction: $event })"
          />
        </el-form-item>
        <div class="property-form-item-separator" />
        <el-form-item :label="$t('scenario.startTime')" class="param-cell">
          <InputNumber
            :model-value="form.startTime"
            unit="s"
            :min="0"
            :max="9999"
            @update:model-value="modifyObject({ startTime: $event })"
          />
        </el-form-item>
        <el-form-item :label="$t('scenario.greenLightDuration')" class="param-cell">
          <InputNumber
            :model-value="form.greenDuration"
            unit="s"
            :min="0"
            :max="600"
            @update:model-value="modifyObject({ greenDuration: $event })"
          />
        </el-form-item>
        <el-form-item :label="$t('scenario.yellowLightDuration')" class="param-cell">
          <InputNumber
            :model-value="form.yellowDuration"
            unit="s"
            :min="0"
            :max="100"
            @update:model-value="modifyObject({ yellowDuration: $event })"
          />
        </el-form-item>
        <el-form-item :label="$t('scenario.redLightDuration')" class="param-cell">
          <InputNumber
            :model-value="form.redDuration"
            unit="s"
            :min="0"
            :max="600"
            @update:model-value="modifyObject({ redDuration: $event })"
          />
        </el-form-item>
        <el-form-item :label="$t('scenario.trafficLightCycle')" class="param-cell">
          {{ (+form.greenDuration) + (+form.yellowDuration) + (+form.redDuration) }} s
        </el-form-item>
        <el-form-item class="param-cell">
          <el-button @click="startMission('EventConfig')">
            事件设置
          </el-button>
        </el-form-item>
      </el-collapse-item>
    </el-collapse>
  </el-form>
</template>

<script>
import property from './property'

export default {
  name: 'PropertySignlight',
  mixins: [property],
}
</script>

<style scoped lang="less">
</style>
