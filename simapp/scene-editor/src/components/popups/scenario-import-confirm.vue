<template>
  <el-dialog
    class="scenario-import-confirm"
    :model-value="true"
    append-to-body
    :close-on-click-modal="false"
    :show-close="true"
    @close="handleConfirm(2)"
  >
    <div class="scenario-import-confirm-wrap">
      <div class="scenario-import-confirm-title">
        <el-icon class="el-icon-question">
          <warning />
        </el-icon> {{ $t('tips.tips') }}
      </div>

      <div class="scenario-import-confirm-content">
        {{ $t('tips.fileNameExists', { name, text }) }}
      </div>

      <section class="scenario-import-confirm-btns">
        <div>
          <el-checkbox v-show="showApplyAll" @change="changed">
            {{ $t('operation.applyAll') }}
          </el-checkbox>
        </div>
        <div>
          <el-button
            class="dialog-cancel"
            @click="handleConfirm(1)"
          >
            {{ $t('operation.keepBoth') }}
          </el-button>
          <el-button
            class="dialog-cancel"
            @click="handleConfirm(2)"
          >
            {{ $t('operation.cancel') }}
          </el-button>
          <el-button
            type="primary"
            class="dialog-ok"
            :disabled="disableReplace"
            @click="handleConfirm(3)"
          >
            {{ $t('operation.replace') }}
          </el-button>
        </div>
      </section>
    </div>
  </el-dialog>
</template>

<script>
export default {
  name: 'ScenarioImportConfirm',
  props: {
    name: {
      type: String,
      default: '-testname',
    },
    windowType: {
      type: Number,
      default: 1, // 1-scene; 2-map
    },
    showApplyAll: {
      type: Boolean,
      default: false,
    },
    disableReplace: {
      type: Boolean,
      default: false,
    },
  },
  computed: {
    text () {
      const text = ['', 'scenario', 'map']
      return this.$t(`scenario.${text[this.windowType]}`)
    },
  },
  methods: {
    handleConfirm (type) {
      this.$emit('confirm', {
        type,
      })
    },

    changed (value) {
      this.$emit('apply-all', {
        value,
      })
    },
  },
}
</script>

<style lang="less" scoped>
@import '@/assets/less/mixins';

.scenario-import-confirm {
  :deep(.el-dialog .el-dialog__header) {
    padding: 0 15px;
    height: 12px;
    line-height: 12px;
    background-color: #242424;
  }

  .scenario-import-confirm-wrap {
    width: 320px;
    padding-bottom: 15px;
    background-color: #242424;
    border-radius: 2px;
    box-shadow: 0 4px 4px 0 rgba(0, 0, 0, 0.14);

    .scenario-import-confirm-title {
      height: 20px;
      padding: 15px 15px 10px;
      font-size: 18px;
      text-align: center;
      color: #c2c2c2;

      i {
        margin-right: 10px;
        color: #ec8215;
        font-size: 24px;
        line-height: 1;
        font-weight: 400;
        vertical-align: bottom;
      }
    }

    .scenario-import-confirm-content {
      padding: 10px 15px;
      color: #606266;
      font-size: 14px;
      line-height: 24px;
    }

    .scenario-import-confirm-btns {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 5px 15px 0;
    }
  }
}
</style>
