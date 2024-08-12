<template>
  <div class="grading">
    <el-form
      ref="form"
      label-width="80px"
      size="small"
      :model="form"
      :rules="rules"
      @submit.prevent
    >
      <el-form-item :label="$t('module.aisle')" prop="channels">
        <CheckboxMultiple v-model="form.channels" :options="channelOptions" :disabled="false" @change="handleChange" />
      </el-form-item>
      <el-form-item :label="$t('module.postProcessingScript')" prop="post-script">
        <InputFileBrowser
          v-model="form['post-script']"
          :title="$t('tips.selectPostScript')"
          :properties="['openFile']"
          :disabled="false"
          @change="handleChange"
        />
      </el-form-item>
    </el-form>
  </div>
</template>

<script>
import mixin from './mixin'

export default {
  name: 'ModuleSettingGrading',
  mixins: [mixin],
  data () {
    return {
      form: {
        'channels': [],
        'post-script': '',
      },
    }
  },
  watch: {
    modelValue: {
      handler (value) {
        if (value) {
          this.form = {
            ...value,
          }
          if (this.form.channels) {
            this.form.channels = this.form.channels.split('||')
          } else {
            this.form.channels = []
          }
        }
      },
      immediate: true,
    },
  },
  methods: {
    handleChange () {
      const form = {
        ...this.form,
      }
      form.channels = form.channels.join('||')
      this.$emit('update:model-value', form)
      this.$emit('change', form)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .grading {
    display: inline-block;
    width: 100%;
    vertical-align: top;
  }
</style>
