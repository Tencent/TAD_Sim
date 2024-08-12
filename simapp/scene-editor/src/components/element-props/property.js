import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { cloneDeep, set } from 'lodash-es'
import InputNumber from '@/components/input-number.vue'
import { editor } from '@/api/interface'
import NumericRangeSliderInput from '@/components/numeric-range-slider-input.vue'

/*
* 例如，以下字符串将匹配该正则表达式：
*  (10, 20)
*  [5, 15]
* */
const rangeReg = /(\(|\[)([^,)\]]*),\s?(.*)(\)|\])/

/**
 * 一个包含多个表单输入组件的复合组件，用于处理场景中的对象属性。
 * @module ObjectProperties
 */
export default {
  components: {
    InputNumber,
    NumericRangeSliderInput,
  },
  data () {
    return {
      rules: {},
      activeCollapseNames: ['basicInfo'],
      silent: false,
      clonedForm: null, // 记录form为null之前的值
      snap: editor.getSnap(),
    }
  },
  computed: {
    ...mapGetters('scenario', {
      form: 'selectedObject',
    }),
    ...mapGetters('element-props', [
      'optionLaneIDList',
      'maxShift',
      'maxOffset',
    ]),
    ...mapState('system', {
      editorConfig: 'editor',
    }),
    autoFollowRoadDirection: {
      get () {
        return this.editorConfig.autoFollowRoadDirection
      },
      set (value) {
        const payload = { ...this.editorConfig, autoFollowRoadDirection: value }
        this.setEditConfig(payload)
      },
    },
  },
  watch: {
    form: {
      handler (val) {
        // clonedForm记录form为null之前的值
        if (val) this.clonedForm = cloneDeep(this.form)
      },
      deep: true,
      immediate: true,
    },
  },
  methods: {
    ...mapMutations('scenario', [
      'updateObject',
    ]),
    ...mapMutations('mission', [
      'startMission',
    ]),
    ...mapActions('system', ['setEditConfig']),
    async modifyObject (param) {
      const data = this.form ? { ...this.form } : { ...this.clonedForm }
      Object.keys(param).forEach((key) => {
        set(data, key, param[key])
      })

      const payload = { type: this.form ? this.form.type : this.clonedForm.type, data }
      delete payload.sensors
      try {
        // 数据经过视图处理，可能会被修改
        const finalData = await editor.object.update(payload)
        this.updateObject({ ...payload, data: finalData })
      } catch (e) {
        console.error(e)
      }
    },
    modifyObjectParams (param) {
      const data = {
        id: this.form ? this.form.id : this.clonedForm.id,
      }
      Object.keys(param).forEach((key) => {
        set(data, key, param[key])
      })

      const type = this.form ? this.form.type : this.clonedForm.type
      const payload = {}

      // 交通车等保留type
      // 主车的type不一样
      if (type === 'planner') {
        if (param.subType === 'combination') {
          data.subType = 'combination'
        } else {
          data.subType = 'car'
        }
      }

      delete payload.sensors
      try {
        // 场景的type相当于subType
        editor.object.modifyObjectParams({ type, data })
        this.updateObject({ type, data })
      } catch (e) {
        console.error(e)
      }
    },

    /**
     * 设置对象的角度。
     * @param {number} angle - 对象的角度值。
     */
    setObjectAngle (angle) {
      const data = this.form ? { ...this.form } : { ...this.clonedForm }
      const payload = {
        type: data.type,
        id: data.id,
        angle,
      }
      try {
        editor.object.setAngle(payload)
        this.updateObject({ type: payload.type, data: { id: payload.id, start_angle: angle } })
      } catch (e) {
        console.log(e)
      }
    },

    /**
     * 设置是否紧贴道路中线
     * @param {number} val - 值。
     * @returns {number} - 返回设置后的值。
     */
    setSnap (val) {
      editor.setSnap(val)
      this.snap = editor.getSnap()
      return this.snap
    },
    toFixed (value, precision) {
      if (Number.isNaN(value)) {
        return `${value}`.substr(0, precision)
      } else {
        return (+value).toFixed(precision)
      }
    },

    /**
     * 从字符串中解析范围的最小值或最大值。
     * @param {string} value - 包含范围的字符串。
     * @param {string} flag - 指定返回最小值还是最大值，可以是 'min' 或 'max'。
     * @returns {number} - 返回解析得到的最小值或最大值。
     */
    range (value, flag) {
      const [, , _min, _max] = rangeReg.exec(value)
      const min = +_min
      const max = +_max
      if (flag === 'min') return min
      if (flag === 'max') return max
    },
  },
}
