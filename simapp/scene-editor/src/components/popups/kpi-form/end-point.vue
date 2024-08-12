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
      <el-form-item :label="$t(thresholdName)" prop="parameters.Radius">
        {{ $t('indicator.radiusRangeOfEndpointScope') }}
        <InputNumber
          id="kpi-form-parameters-radius"
          v-model="form.parameters.Radius"
          :disabled="disabled"
          unit="m"
          :min="0"
          :max="7"
        />
        <br>
        <el-checkbox id="kpi-form-parameters-need-parking" v-model="needParking" :disabled="disabled">
          {{ $t('indicator.arrivalSpeed', { n: 0 }) }}
        </el-checkbox>
      </el-form-item>
      <el-form-item :label="$t(passConditionName)">
        {{ $t('indicator.output') }} true
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
          <span>{{ $t('indicator.output') }} true</span>
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
  name: 'EndPoint',
  mixins: [mixin],
  data () {
    return {
      needParking: false,
    }
  },
  watch: {
    needParking (value) {
      let val = ''
      if (value) {
        val = '1'
      } else {
        val = '0'
      }
      this.form.parameters.NeedParking = val
    },
  },
  created () {
    const { NeedParking } = this.form.parameters
    if (NeedParking === '1') {
      this.needParking = true
    } else if (NeedParking === '0') {
      this.needParking = false
    }
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";
  @import "@/assets/less/kpi-form";
</style>
