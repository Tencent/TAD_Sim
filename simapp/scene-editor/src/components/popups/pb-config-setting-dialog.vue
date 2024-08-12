<template>
  <div>
    <el-dialog
      class="pb-config-setting-dialog"
      :model-value="visible"
      :header="dialogTitle"
      append-to-body
      top="5%"
      width="500px"
      :close-on-click-modal="false"
      @close="handleClose"
    >
      <el-form
        v-if="visible"
        ref="form"
        :model="form"
        :rules="rules"
        class="pb-config-setting-from form-label-align-left"
        label-width="80px"
      >
        <el-collapse v-model="activeCollapseNames">
          <el-collapse-item :title="$t('scenario.basicInformation')" name="basicInfo">
            <el-form-item prop="field" :label="$t('player.formField')">
              <el-input
                v-model="form.field"
                :maxlength="50"
                :placeholder="$t('tips.pleaseInput', { text: $t('player.formField') })"
              />
            </el-form-item>
            <el-form-item prop="topic" :label="$t('player.formTopic')">
              <el-select
                v-model="form.topic"
                :placeholder="$t('tips.pleaseInput', { text: $t('player.formTopic') })"
                @change="handleTopicChange"
              >
                <el-option
                  v-for="op in topicList"
                  :key="op.value"
                  :value="op.value"
                  :label="op.label"
                />
              </el-select>
            </el-form-item>
            <el-form-item prop="paths" :label="$t('player.formPaths')">
              <el-cascader
                v-model="form.paths"
                :options="pbConfigPathsOptions"
                :props="{ multiple: false }"
                filterable
                :placeholder="$t('tips.pleaseInput', { text: $t('player.formPaths') })"
              />
            </el-form-item>
            <el-form-item prop="description" :label="$t('player.formDescription')">
              <el-input
                v-model="form.description"
                type="textarea"
                :maxlength="200"
                :placeholder="$t('tips.pleaseInput', { text: $t('player.formDescription') })"
              />
            </el-form-item>
          </el-collapse-item>
          <el-collapse-item v-if="form.chart" :title="$t('player.chartInfo')" name="chartInfo">
            <el-form-item prop="chart.title" :label="$t('player.chartTitle')">
              <el-input
                v-model="form.chart.title"
                :maxlength="50"
                :placeholder="$t('tips.pleaseInput', { text: $t('player.chartTitle') })"
              />
            </el-form-item>
            <el-form-item prop="chart.unit" :label="$t('player.chartUnit')">
              <el-input
                v-model="form.chart.unit"
                :maxlength="50"
                :placeholder="$t('tips.pleaseInput', { text: $t('player.chartUnit') })"
              />
            </el-form-item>
            <el-form-item prop="chart.xAxisName" :label="$t('player.chartXAxis')">
              <el-input
                v-model="form.chart.xAxisName"
                :maxlength="50"
                :placeholder="$t('tips.pleaseInput', { text: $t('player.chartXAxis') })"
              />
            </el-form-item>
            <el-form-item prop="chart.yAxisName" :label="$t('player.chartYAxis')">
              <el-input
                v-model="form.chart.yAxisName"
                :maxlength="50"
                :placeholder="$t('tips.pleaseInput', { text: $t('player.chartYAxis') })"
              />
            </el-form-item>
          </el-collapse-item>
        </el-collapse>
      </el-form>
      <div class="form-buttons">
        <el-button v-if="action === 'modify'" @click="handleReset">
          {{ $t('player.formReset') }}
        </el-button>
        <el-button @click="handleClose">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button @click="handleConfirm">
          {{ $t('operation.ok') }}
        </el-button>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import { getTopicProto } from '@/common/proto-define'

const defaultForm = {
  field: '',
  topic: 'DebugMessage',
  paths: [],
  description: '',
  chart: {
    title: '',
    unit: '',
    xAxisName: '',
    yAxisName: '',
  },
}

export default {
  data () {
    return {
      visible: false,
      action: '', // create modify
      originData: undefined,
      list: [],
      form: {},
      callbacks: {},
      pbConfigPathsOptions: [],
      activeCollapseNames: ['basicInfo', 'chartInfo'],
      rules: [],
    }
  },
  computed: {
    ...mapState('pb-config', ['topicList']),
    dialogTitle () {
      if (this.action === 'create') return this.$t('player.addPbConfigItem')
      return this.$t('player.modifyPbConfigItem')
    },
  },
  methods: {
    ...mapActions('pb-config', [
      'refreshTopicList',
    ]),
    async openDialog (opts) {
      const { action, list, data, success, fail } = opts
      this.action = action
      this.list = list
      this.originData = data || defaultForm
      this.refreshTopicList()
      const form = action === 'modify' ? _.cloneDeep(data) : _.cloneDeep(defaultForm)
      form.paths = (form.paths || []).slice(1)
      form.chart.unit = form.chart.unit === 'N/A' ? '' : form.chart.unit
      this.rules = this.getFormRules()
      this.form = form
      this.pbConfigPathsOptions = await getTopicProto(this.form.topic || 'DebugMessage')
      this.callbacks = { success, fail }
      this.visible = true
    },
    async handleTopicChange (topic) {
      this.form.paths = []
      this.pbConfigPathsOptions = await getTopicProto(topic || 'DebugMessage')
    },
    handleReset () {
      const form = _.cloneDeep(this.originData)
      form.paths = (form.paths || []).slice(1)
      form.chart.unit = form.chart.unit === 'N/A' ? '' : form.chart.unit
      this.form = form
    },
    handleClose () {
      this.callbacks.success({ confirm: false, data: undefined })
      this.visible = false
    },
    async handleConfirm () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return
      }
      const form = _.cloneDeep(this.form)
      form.chart.unit = form.chart.unit ? form.chart.unit : 'N/A'
      form.paths = [form.topic, ...form.paths]
      this.callbacks.success({ confirm: true, data: form })
      this.visible = false
    },
    getFormRules () {
      return {
        'field': [
          { required: true, message: this.$t('tips.pleaseInput', { text: this.$t('player.formField') }) },
          {
            validator: (_, value, callback) => {
              if (value && this.action !== 'modify' && this.list?.length) {
                if (this.list.find(e => value === e.field)) {
                  callback(new Error('字段名称不可重复'))
                  return
                }
              }
              callback()
            },
          },
        ],
        'topic': [{ required: true, message: this.$t('tips.pleaseInput', { text: this.$t('player.formTopic') }) }],
        'paths': [{ required: true, message: this.$t('tips.pleaseInput', { text: this.$t('player.formPaths') }) }],
        'chart.title': [{ required: true, message: this.$t('tips.pleaseInput', { text: this.$t('player.chartTitle') }) }],
      }
    },
  },
}
</script>

<style lang="less" scoped>
.pb-config-setting-from {
  :deep(.el-cascader) {
    line-height: unset;
  }

  :deep(.el-collapse-item__content) {
    padding: 16px 20px 0;
  }

  :deep(.el-cascader__search-input) {
    background-color: transparent;

    &::placeholder {
      color: red;
      display: none;
    }
  }
}

:deep(.el-form.form-label-align-left) {
  .el-form-item__label {
    text-align: left;
  }
}

.form-buttons {
  display: flex;
  align-items: center;
  justify-content: flex-end;
  padding: 0 20px 16px;
}
</style>
