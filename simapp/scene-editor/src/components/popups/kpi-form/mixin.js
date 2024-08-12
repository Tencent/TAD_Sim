import { isPlainObject } from 'lodash-es'
import dict from '@/common/dict'
import { toPlainObject } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'

/**
 * KPI 模块通用属性组件。
 * @module KPICommonAttributes
 */
export default {
  components: {
    InputNumber,
  },
  props: {
    /**
     * 表单数据对象，可以是 null 或一个普通对象。
     * @type {object | null}
     */
    value: {
      validator (value) {
        return isPlainObject(value) || value === null
      },
      required: true,
    },
    /**
     * 是否禁用组件。
     * @type {boolean}
     * @default true
     */
    disabled: {
      type: Boolean,
      default: true,
    },
  },
  data () {
    return {
      // form数据
      form: toPlainObject(this.value),
      // 校验规则
      rules: {},
      prevPassCondition: -1,
      prevFinishCondition: -1,
      passConditionMode: -1, // 0, 1
      finishConditionMode: -1, // 0, 1
    }
  },
  computed: {
    name () {
      if (this.form) {
        const { name } = this.form
        const object = dict.kpi[name]
        return object ? this.$t(object.name) : name
      }
    },
    definition () {
      if (this.form) {
        const object = dict.kpi[this.form.name]
        return object ? this.$t(object.definition) : ''
      }
    },
    calculation () {
      if (this.form) {
        const object = dict.kpi[this.form.name]
        return object ? this.$t(object.calculation) : ''
      }
    },
    thresholdName () {
      if (this.disabled) {
        return 'indicator.defaultThreshold'
      } else {
        return 'indicator.threshold'
      }
    },
    passConditionName () {
      if (this.disabled) {
        return 'indicator.defaultIndicatorPassCondition'
      } else {
        return 'indicator.indicatorPassConditions'
      }
    },
    finishConditionName () {
      if (this.disabled) {
        return 'indicator.defaultAsTheScenarioEndCondition'
      } else {
        return 'indicator.asAScenarioEndCondition'
      }
    },
  },
  watch: {
    form: {
      handler (value) {
        const { passCondition, finishCondition } = value
        if (passCondition) {
          this.prevPassCondition = passCondition
        }
        if (finishCondition) {
          this.prevFinishCondition = finishCondition
        }
      },
      deep: true,
    },
    passConditionMode (value) {
      if (value === 0) {
        this.form.passCondition = 0
      } else if (value === 1) {
        this.form.passCondition = this.prevPassCondition
      }
    },
    finishConditionMode (value) {
      if (value === 0) {
        this.form.finishCondition = 0
      } else if (value === 1) {
        this.form.finishCondition = this.prevFinishCondition
      }
    },
  },
  created () {
    if (this.form) {
      const { passCondition, finishCondition } = this.form
      if (passCondition !== undefined) {
        this.prevPassCondition = passCondition === 0 ? 1 : passCondition
        this.passConditionMode = passCondition === 0 ? 0 : 1
      }
      if (finishCondition !== undefined) {
        this.prevFinishCondition = finishCondition === 0 ? 1 : finishCondition
        this.finishConditionMode = finishCondition === 0 ? 0 : 1
      }
    }
  },
  methods: {
    /**
     * 取消操作，关闭组件。
     */
    handleCancel () {
      this.$emit('close')
    },
    /**
     * 确认操作，验证表单数据并发出确认事件。
     * @returns {Promise<boolean>} - 如果验证成功，返回 true，否则返回 false。
     */
    async handleConfirm () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      this.$emit('confirm', this.form)
    },
  },
}
