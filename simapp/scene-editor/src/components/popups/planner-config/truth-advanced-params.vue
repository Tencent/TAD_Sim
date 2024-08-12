<template>
  <div class="truth-advanced-params">
    <NumInputItem
      :value="data.vfov"
      :desc="truthAdvancedInfo.vfov"
      @input="changeAdvancedInfo({ paramName: 'vfov', value: $event })"
    />
    <NumInputItem
      :value="data.hfov"
      :desc="truthAdvancedInfo.hfov"
      @input="changeAdvancedInfo({ paramName: 'hfov', value: $event })"
    />
    <NumInputItem
      :value="data.drange"
      :desc="truthAdvancedInfo.drange"
      @input="changeAdvancedInfo({ paramName: 'drange', value: $event })"
    />
  </div>
</template>

<script>
import NumInputItem from './num-input-item.vue'
import { truthAdvancedInfo } from '@/store/modules/sensor/constants'

export default {
  name: 'TruthAdvancedParams',
  components: {
    NumInputItem,
  },
  props: {
    currentSensor: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      truthAdvancedInfo,
    }
  },
  computed: {
    data () {
      return this.currentSensor.param
    },
  },
  methods: {
    changeAdvancedInfo (params) {
      this.$emit('change-advanced-info', params)
    },
  },
}
</script>

<style scoped lang="less">
  .truth-advanced-params {
    padding: 0 22px;
  }

  .param-cell {
    display: flex;
    justify-content: space-between;
    padding: 10px 0;
    align-items: center;

    .el-radio {
      margin-right: 16px;

      &:last-child {
        margin-right: 0;
      }

      :deep(.el-radio__label) {
        padding-left: 5px;
      }
    }

    .param-input {
      box-sizing: border-box;
      width: 80px;
      height: 24px;

      :deep(.el-input__inner) {
        padding: 0 6px;
        line-height: 22px;
        height: 24px;
        border-radius: 2px;
      }

      &.el-input--suffix {
        :deep(.el-input__inner) {
          padding-right: 25px;
        }
      }
    }

    .el-select {
      width: 80px;

      :deep(.el-input--suffix) {
        .el-input__inner {
          height: 24px;
          line-height: 22px;
          border-radius: 2px;
          padding-left: 6px;
        }

        .el-input__icon {
          line-height: 24px;
        }
      }
    }

    &.multi-input {
      display: block;

      .form-label {
        display: block;
        margin-bottom: 15px;
      }
    }

    .form-multi-input {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-column-gap: 10px;

      .input-cell {
        display: flex;
        flex-direction: column;
        align-items: center;
        margin-bottom: 10px;

        :deep(.el-input__inner) {
          width: 60px;
          height: 24px;
          line-height: 22px;
          border-radius: 2px;
        }
      }
    }
  }
</style>
