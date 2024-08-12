<template>
  <el-dialog
    class="scenario-export-confirm"
    :model-value="true"
    :header="$t('tips.selectExportScenarioFormat')"
    append-to-body
    :close-on-click-modal="false"
    :show-close="true"
    @close="$emit('close')"
  >
    <div class="scenario-export-confirm-wrap">
      <el-form
        ref="form"
        :model="form"
        :rules="rules"
        label-width="100px"
        @submit.prevent
      >
        <el-form-item :label="$t('scenario.exportSceneFormat')" prop="ext">
          <el-select v-model="form.ext" :placeholder="$t('tips.pleaseSelect')" class="export-scenario-select">
            <el-option :label="$t('scenario.currentFormat')" :value="-1" />
            <el-option v-for="ext in exts" :key="ext" :label="`.${ext}`" :value="ext" />
          </el-select>
        </el-form-item>
      </el-form>

      <section class="button-group">
        <el-button class="dialog-cancel" @click="$emit('close')">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button class="dialog-ok" type="primary" @click="handleOK">
          {{ $t('operation.ok') }}
        </el-button>
      </section>
    </div>
  </el-dialog>
</template>

<script>
export default {
  name: 'ScenarioExportConfirm',
  props: {
    exts: {
      type: Array,
      required: true,
    },
  },
  data () {
    return {
      form: {
        ext: -1,
      },
      rules: {
      },
    }
  },
  methods: {
    async handleOK () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return
      }
      this.$emit('confirm', this.form.ext)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .scenario-export-confirm {
    .scenario-export-confirm-wrap {
      width: 188px;
      padding: 20px 22px;
    }

    .button-group {
      margin-top: 30px;
      text-align: right;
    }
  }
</style>
