<template>
  <el-dialog
    :model-value="visible"
    :header="dialogTitle"
    append-to-body
    top="16%"
    @close="handleClose"
  >
    <div v-if="visible" class="kpi-labels-setting-dialog">
      <el-form
        ref="form"
        :model="form"
        :rules="rules"
        class="property-form property-kpi-labels"
        label-position="left"
        label-width="110px"
      >
        <!-- 标签中文名称 -->
        <el-form-item prop="name" :label="$t('indicator.labelNameCN')">
          <el-input v-model="form.name" :maxlength="32" />
        </el-form-item>
        <!-- 标签英文名称 -->
        <el-form-item prop="id" :label="$t('indicator.labelNameEN')">
          <template #label>
            <span style="margin-right:4px">{{ $t('indicator.labelNameEN') }}</span>
            <el-tooltip :content="$t('indicator.labelNameENTip')" placement="bottom" style="align-self: center;">
              <el-icon class="el-icon-warning-outline">
                <warning />
              </el-icon>
            </el-tooltip>
          </template>
          <el-input v-model="form.id" :maxlength="32" :disabled="action === 'modify'" />
        </el-form-item>
      </el-form>
    </div>
    <template #footer>
      <el-button @click="handleClose">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button @click="handleConfirm">
        {{ $t('operation.ok') }}
      </el-button>
    </template>
  </el-dialog>
</template>

<script>
import { calcStrLength } from '@/common/utils'

export default {
  name: 'KpiLabelsSettingDialog',
  data () {
    return {
      visible: false,
      action: 'create', // create/modify
      originList: [], // { id, name }
      form: {},
      rules: [],
      callbacks: undefined,
    }
  },
  computed: {
    dialogTitle () {
      const actionTxt = this.action === 'create' ? this.$t('operation.create') : this.$t('menu.edit')
      return actionTxt + this.$t('kpi.labels')
    },
  },
  methods: {
    // 外放函数
    openDialog (props) {
      const {
        action, // create/modify
        list = [], // { id, name }
        data = {}, // { id, name }
        success,
        fail,
        complete,
      } = props

      this.action = action
      this.originList = list

      if (action === 'create') {
        this.rules = this.getRules()
        this.form = {}
      } else {
        this.rules = this.getRules()
        this.form = { ...data }
      }

      this.visible = true
      this.callbacks = { success, fail, complete }
    },
    // 关闭弹窗
    handleClose () {
      const { success, complete } = this.callbacks

      this.visible = false
      this.form = {}

      success && success({ confirm: false, data: undefined })
      complete && complete({ confirm: false, data: undefined })
    },
    // 触发确认
    handleConfirm () {
      const $form = this.$refs.form
      if (!$form) return

      $form.validate(async (valid) => {
        if (!valid) return

        this.confirm()
      })
    },
    // 正式确认
    confirm () {
      const { success, complete } = this.callbacks
      const data = { ...this.form }

      this.visible = false

      success && success({ confirm: true, data })
      complete && complete({ confirm: true, data })
    },
    getRules () {
      return {
        id: [
          { validator: this.validateNameEN, trigger: 'blur' },
        ],
        name: [
          { validator: this.validateNameCN, trigger: 'blur' },
        ],
      }
    },
    // 校验 - 标签中文名称
    validateNameCN (rule, value, callback) {
      if (!value || !value.trim()) {
        const error = new Error('请输入内容，支持中文、数字、符号，不超过32个字符')
        callback(error)
        return
      }
      if (value === '未分类') {
        const error = new Error('该名称已存在')
        callback(error)
        return
      }
      if (calcStrLength(value) > 32) {
        const error = new Error('输入不符合规则')
        callback(error)
        return
      }
      if (/[a-z]/i.test(value)) {
        const error = new Error('输入不符合规则')
        callback(error)
        return
      }
      const hasSameName = this.originList.find(e => e.name === value)
      if (hasSameName) {
        const error = new Error('名称重复，请重新输入')
        callback(error)
        return
      }
      callback()
    },
    // 校验 - 标签英文名称
    validateNameEN (rule, value, callback) {
      if (this.action === 'modify') {
        callback()
        return
      }
      if (!value || !value.trim()) {
        const error = new Error('请输入内容，支持英文，不超过32个字符')
        callback(error)
        return
      }
      if (calcStrLength(value) > 32) {
        const error = new Error('输入不符合规则')
        callback(error)
        return
      }
      if (!/^[a-z]+$/i.test(value)) {
        const error = new Error('输入不符合规则')
        callback(error)
        return
      }
      const hasSameName = this.originList.find(e => e.id === value)
      if (hasSameName) {
        const error = new Error('名称重复，请重新输入')
        callback(error)
        return
      }
      callback()
    },
  },
}
</script>

<style lang="less" scoped>
@import "@/assets/less/mixins";

.kpi-labels-setting-dialog {
  width: 500px;
  padding: 12px 24px;
}
</style>
