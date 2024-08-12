<template>
  <div v-if="form && form.name" class="kpi-form-new">
    <el-form
      ref="form"
      label-width="100px"
      size="small"
      :model="form"
      :rules="rules"
      @submit.prevent
    >
      <!-- 指标名称 -->
      <el-form-item :label="$t('indicator.name')">
        <span>{{ $kpiAlias(form) }}</span>
      </el-form-item>
      <!-- 指标定义 -->
      <el-form-item :label="$t('kpi.definition')">
        <span>{{ form.parameters.definition }}</span>
      </el-form-item>
      <!-- 指标分类 -->
      <el-form-item :label="$t('kpi.labels')" prop="category.labels">
        <div class="kpi-labels">
          <template v-if="!labelsEditing">
            <el-tag
              v-for="tag in getCategory(form)"
              :key="tag"
              disable-transitions
            >
              {{ tag }}
            </el-tag>
            <el-button
              v-if="disabled"
              link
              icon="edit"
              @click="handleStartLabelsSetting"
            />
          </template>
          <template v-else>
            <el-select
              v-model="labelsTmpData"
              multiple
              class="kpi-labels-select"
            >
              <el-option
                v-for="op in kpiLabelsList"
                :key="op.labelKey"
                :value="op.labelKey"
                :label="op.labelDescription"
              />
            </el-select>
          </template>
        </div>
      </el-form-item>
      <!-- 评测阈值 -->
      <el-form-item
        v-if="form.parameters.thresholds && form.parameters.thresholds.length"
        :label="$t('kpi.thresholds')"
        prop="parameters.thresholds"
      >
        <div class="kpi-thresholds-list">
          <div
            v-for="(threshold, index) in form.parameters.thresholds"
            :key="threshold.thresholdID"
            class="kpi-thresholds-item"
          >
            <div class="label">
              {{ threshold.thresholdName }}
            </div>
            <div class="content">
              <template v-if="threshold.thresholdType === 'ThresholdType_DOUBLE'">
                <InputNumber
                  v-model="threshold.threshold"
                  :unit="threshold.thresholdUnit !== 'N/A' ? threshold.thresholdUnit : ''"
                  :disabled="disabled"
                />
              </template>
              <template v-else-if="threshold.thresholdType === 'ThresholdType_STRING'">
                <el-form-item
                  :prop="`threshold${index}`"
                  :rules="[{ index, validator: thresholdRequired, trigger: 'blur' }]"
                >
                  <el-input v-model="threshold.threshold" :disabled="disabled" size="medium" />
                </el-form-item>
                <!-- <span>{{ threshold.threshold }}</span> -->
              </template>
              <template v-else-if="threshold.thresholdType === 'ThresholdType_BOOL'">
                <el-radio-group v-model="threshold.threshold" :disabled="disabled">
                  <template v-if="threshold.threshold === 'true' || threshold.threshold === 'false'">
                    <el-radio label="true" value="true">
                      {{ $t('yes') }}
                    </el-radio>
                    <el-radio label="false" value="false">
                      {{ $t('no') }}
                    </el-radio>
                  </template>
                  <template v-else>
                    <el-radio label="1" value="1">
                      {{ $t('yes') }}
                    </el-radio>
                    <el-radio label="0" value="0">
                      {{ $t('no') }}
                    </el-radio>
                  </template>
                </el-radio-group>
              </template>
            </div>
          </div>
        </div>
      </el-form-item>
      <!-- 评测通过条件 -->
      <el-form-item v-if="form.passCondition" :label="$t('kpi.passCondition')">
        <el-radio-group
          id="kpi-form-pass-condition-mode"
          v-model="passConditionMode"
          class="condition-mode-group"
          style="display: block;"
          :disabled="disabled"
        >
          <el-radio class="condition-item" :label="true" :value="true">
            <div class="condition-total-count">
              <span class="label">{{ $t('kpi.totalCount') }}</span>
              <span class="relation">{{ form.passCondition.relation }}</span>
              <InputNumber
                v-model="form.passCondition.value"
                :min="1"
                :precision="0"
                :unit="$t('kpi.totalCountUnit')"
                :disabled="disabled"
              />
            </div>
          </el-radio>
          <el-radio class="condition-item" :label="false" :value="false">
            <span>{{ $t('kpi.defaultPassCondition') }}</span>
          </el-radio>
        </el-radio-group>
      </el-form-item>
      <!-- 场景结束条件 -->
      <el-form-item v-if="form.finishCondition" :label="$t('kpi.finishCondition')">
        <el-radio-group
          id="kpi-form-finish-condition-mode"
          v-model="finishConditionMode"
          class="condition-mode-group"
          style="display: block;"
          :disabled="disabled"
        >
          <el-radio class="condition-item" :label="true" :value="true">
            <div class="condition-total-count">
              <span class="label">{{ $t('kpi.totalCount') }}</span>
              <span class="relation">{{ form.finishCondition.relation }}</span>
              <InputNumber
                v-model="form.finishCondition.value"
                :min="1"
                :precision="0"
                :unit="$t('kpi.totalCountUnit')"
                :disabled="disabled"
              />
            </div>
          </el-radio>
          <el-radio class="condition-item" :label="false" :value="false">
            <span>{{ $t('kpi.defaultFinishCondition') }}</span>
          </el-radio>
        </el-radio-group>
      </el-form-item>
      <!-- 指标评分规则 -->
      <el-form-item
        v-if="form.parameters && !hideScoreSetting"
        :label="$t('kpi.scoreSetting')"
      >
        <template #label>
          <span>{{ $t('kpi.scoreSetting') }}</span>
          <el-tooltip placement="bottom" :content="$t('kpi.scoreSettingTip')">
            <el-icon class="el-icon-warning-outline" style="margin-left:4px">
              <warning />
            </el-icon>
          </el-tooltip>
        </template>
        <div v-if="scoreEditing">
          <el-button
            link
            icon="remove"
            class="kpi-score-setting-switch"
            :disabled="disabled"
            @click="handleCloseScoreSetting"
          >
            {{ $t('kpi.scoreSettingClose') }}
          </el-button>
          <div class="score-setting-wrap">
            <KpiScoreSetting
              ref="scoreSetting"
              :value="form.parameters.scoreMap1d"
              :disabled="disabled"
            />
          </div>
        </div>
        <div v-else>
          <el-button
            link
            icon="circle-plus"
            class="kpi-score-setting-switch"
            :disabled="disabled"
            @click="handleStartScoreSetting"
          >
            {{ $t('kpi.scoreSettingStart') }}
          </el-button>
        </div>
      </el-form-item>
    </el-form>
    <section v-if="!disabled" class="kpi-form-buttons">
      <el-button id="kpi-form-cancel" class="dialog-cancel" @click="handleCancel">
        {{
          $t('operation.cancel')
        }}
      </el-button>
      <el-button id="kpi-form-confirm" class="dialog-cancel" @click="handleConfirm">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
    <section v-if="disabled && labelsEditing" class="kpi-form-buttons-for-label">
      <el-button id="kpi-form-cancel" class="dialog-cancel" @click="handleCancelLabels">
        {{
          $t('operation.cancel')
        }}
      </el-button>
      <el-button id="kpi-form-confirm" class="dialog-cancel" @click="handleConfirmLabels">
        {{
          $t('operation.ok')
        }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { cloneDeep, isPlainObject } from 'lodash-es'
import { mapState } from 'vuex'
import KpiScoreSetting from './kpi-score-setting.vue'
import { toPlainObject } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'
import KpiMixin from '@/components/popups/kpi-form-new/mixin'
import service from '@/api'

export default {
  name: 'KpiFormNew',
  components: {
    InputNumber,
    KpiScoreSetting,
  },
  mixins: [
    KpiMixin,
  ],
  props: {
    value: {
      validator (value) {
        return isPlainObject(value) || value === null
      },
      required: true,
    },
    disabled: {
      type: Boolean,
      default: true,
    },
    hideScoreSetting: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      form: toPlainObject(this.value),
      rules: {},
      passConditionMode: false,
      finishConditionMode: false,
      labelsEditing: false,
      labelsTmpData: [],
      scoreEditing: false,
    }
  },
  computed: {
    ...mapState('kpi', [
      'kpiLabelsList',
    ]),
  },
  watch: {
    value (newVal) {
      if (Object.keys(newVal).length) this.initFormData(newVal)
    },
  },
  mounted () {
    if (Object.keys(this.value).length) this.initFormData(this.value)
  },
  methods: {
    initFormData (propForm) {
      const form = toPlainObject(propForm)

      // 条件若为0，则勾选无关
      if (form.passCondition.value >= 1) {
        this.passConditionMode = true
      } else {
        form.passCondition.value = 1
        this.passConditionMode = false
      }
      if (form.finishCondition.value >= 1) {
        this.finishConditionMode = true
      } else {
        form.finishCondition.value = 1
        this.finishConditionMode = false
      }

      if (form.parameters?.scoreMap1d?.u?.length) {
        this.scoreEditing = true
      }

      this.labelsEditing = false
      this.labelsTmpData = []
      if (form.category?.labels?.length) {
        form.category.labels = form.category.labels.filter(id => this.kpiLabelsList.some(item => item.labelKey === id))
      }

      this.form = form
      console.log('kpi form init:', form)
    },
    handleStartScoreSetting () {
      if (this.disabled) return
      this.scoreEditing = true
    },
    handleCloseScoreSetting () {
      if (this.disabled) return
      this.scoreEditing = false
    },
    handleCancel () {
      this.$emit('close')
    },
    async handleConfirm () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }

      const form = cloneDeep(this.form)
      if (form.parameters.thresholds) {
        form.parameters.thresholds.forEach((item) => {
          item.threshold = `${item.threshold}`.trim()
        })
      }
      if (!this.passConditionMode) {
        form.passCondition.value = 0
      }
      if (!this.finishConditionMode) {
        form.finishCondition.value = 0
      }

      if (this.scoreEditing && !this.hideScoreSetting) {
        this.$refs.scoreSetting.validate((valid) => {
          if (!valid) return

          const scoreMap1d = this.$refs.scoreSetting.getResult()
          form.parameters.scoreMap1d = scoreMap1d

          this.$emit('confirm', form)
        })
        return
      }

      if (!this.scoreEditing) {
        delete form.parameters.scoreMap1d
      }

      this.$emit('confirm', form)
    },
    // 指标分类开启编辑
    handleStartLabelsSetting () {
      this.labelsEditing = true
      this.labelsTmpData = this.form.category.labels
    },
    // 指标分类编辑取消
    handleCancelLabels () {
      this.labelsEditing = false
      this.labelsTmpData = []
    },
    // 指标分类编辑确认
    async handleConfirmLabels () {
      try {
        const form = cloneDeep(this.form)
        form.category.labels = this.labelsTmpData
        const params = {
          name: form.name,
          labels: this.labelsTmpData,
        }
        await service.setGradingLabel(params)
        this.$message.success(`${this.$t('operation.modify')}${this.$t('tips.success')}`)
        this.handleCancelLabels()
        this.$emit('change', form)
      } catch (err) {
        console.log(err)
        this.$message.error(err.message)
      }
    },
    // 评测阈值有部分必填
    thresholdRequired (rule, _, callback) {
      const item = this.form.config.parameters.thresholds[rule.index]
      if (item.thresholdType === 'ThresholdType_STRING') {
        if (!item.threshold.trim()) {
          const error = new Error(this.$t('messages.valueRequired'))
          return callback(error)
        }
      }
      callback()
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";
@import "@/assets/less/kpi-form";

:deep(.el-button.kpi-score-setting-switch) {
  padding: 0.4em 0;
}

.kpi-labels-select {
  width: 100%;
  height: auto;
  display: block;

  :deep(.el-tag) {
    padding-right: 10px;
    --el-tag-text-color: #fff;
    --el-tag-bg-color: #333;
    --el-tag-border-color: transparent;
    --el-tag-hover-color: var(--el-tag-bg-color);
  }

  :deep(.el-tag) {
    .el-tag__close {
      right: -5px;
    }

    &:hover {
      .el-tag__close {
        background-color: @list-item-hover-bg;
        color: @active-font-color;
      }
    }
  }
}

.kpi-form-buttons-for-label {
  text-align: right;
}
</style>
