import _ from 'lodash'
import InputNumber from '@/components/input-number.vue'
import InputFileBrowser from '@/components/input-file-browser.vue'
import CheckboxMultiple from '@/components/checkbox-multiple.vue'

/**
 * 一个包含多个表单输入组件的复合组件。
 * @module FormComponent
 */

export default {
  components: {
    InputNumber,
    InputFileBrowser,
    CheckboxMultiple,
  },
  props: {
    /**
     * 表单数据对象，可以是 null 或一个普通对象。
     * @type {object | null}
     */
    modelValue: {
      validator (value) {
        return _.isPlainObject(value) || value === null
      },
      required: true,
    },
  },
  data () {
    return {
      form: null,
      rules: {},
      /**
       * 可用的选项列表。
       * @type {Array<string>}
       */
      channelOptions: [
        'CONTROL',
        'CONTROL_V2',
        'GPS_SIM',
        'GRADING',
        'IMU_SIM',
        'LOCATION',
        'LOCATION_TRAILER',
        'TRAFFIC',
        'TRAJECTORY',
        'TRAJECTORYFOLLOW',
        'TXSIM_SENSOR_DATA',
        'VEHICLE_STATE',
      ],
    }
  },
}
