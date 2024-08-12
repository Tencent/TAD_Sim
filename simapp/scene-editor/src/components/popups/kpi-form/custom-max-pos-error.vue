<template>
  <div class="kpi-form">
    <el-form
      ref="form"
      label-width="140px"
      size="small"
      :model="form"
      :rules="rules"
      @submit.prevent
    >
      <el-form-item :label="$t('indicator.name')" prop="name">
        <el-input id="kpi-form-name" :model-value="name" :disabled="true" />
      </el-form-item>
      <el-form-item :label="$t('definition')" prop="definition">
        <el-input
          id="kpi-form-definition"
          :model-value="definition"
          type="textarea"
          :rows="2"
          :disabled="true"
        />
      </el-form-item>
      <el-form-item :label="$t('calculation')" prop="calculation">
        <el-input
          id="kpi-form-calculation"
          :model-value="calculation"
          type="textarea"
          :rows="2"
          :disabled="true"
        />
      </el-form-item>
      <el-form-item :label="$t(thresholdName)" prop="parameters.Threshold">
        {{ $t('indicator.rangeOfLateralOffset') }}
        <InputNumber
          id="kpi-form-parameters-threshold"
          v-model="form.parameters.Threshold"
          :disabled="disabled"
          unit="cm"
          :min="1"
          :max="700"
        />
      </el-form-item>
      <el-form-item :label="$t(passConditionName)" prop="passCondition">
        <el-radio-group id="kpi-form-pass-condition-mode" v-model="passConditionMode" :disabled="disabled">
          <el-radio :label="1" :value="1">
            {{ $t('indicator.times') }}
          </el-radio>
          <el-radio :label="0" :value="0">
            {{ $t('none') }}
          </el-radio>
        </el-radio-group>
        <div v-if="passConditionMode === 1" class="kpi-form-pass-condition">
          <span>&lt; {{ $t('indicator.times') }}</span>
          <InputNumber
            id="kpi-form-pass-condition"
            v-model="form.passCondition"
            :disabled="disabled"
            :unit="$t('indicator.timesUnit')"
            :precision="0"
            :min="1"
            :max="100"
          />
        </div>
      </el-form-item>
      <el-form-item :label="$t(finishConditionName)" prop="finishCondition">
        <el-radio-group id="kpi-form-finish-condition-mode" v-model="finishConditionMode" :disabled="disabled">
          <el-radio :label="1" :value="1">
            {{ $t('yes') }}
          </el-radio>
          <el-radio :label="0" :value="0">
            {{ $t('no') }}
          </el-radio>
        </el-radio-group>
        <div v-if="finishConditionMode === 1" class="kpi-form-finish-condition">
          <span>&ge; {{ $t('indicator.times') }}</span>
          <InputNumber
            id="kpi-form-finish-condition"
            v-model="form.finishCondition"
            :disabled="disabled"
            :unit="$t('indicator.timesUnit')"
            :precision="0"
            :min="1"
            :max="100"
          />
        </div>
      </el-form-item>
    </el-form>
    <section v-if="!disabled" class="kpi-form-buttons">
      <el-button id="kpi-form-cancel" class="dialog-cancel" @click="handleCancel">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button id="kpi-form-confirm" class="dialog-cancel" @click="handleConfirm">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import mixin from './mixin'

export default {
  name: 'CustomMaxPosError',
  mixins: [mixin],
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";
  @import "@/assets/less/kpi-form";
</style>
