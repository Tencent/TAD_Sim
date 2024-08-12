<template>
  <div class="protobuf-log-player">
    <el-form
      ref="form"
      label-width="100px"
      size="small"
      :model="form"
      :rules="rules"
      @submit.prevent
    >
      <el-form-item :label="$t('module.aisle')" prop="channels">
        <CheckboxMultiple v-model="form.channels" :options="channelOptions" :disabled="false" @change="handleChange" />
      </el-form-item>
      <el-form-item :label="$t('module.playbackFilePath')" prop="log-file">
        <InputFileBrowser
          v-model="form['log-file']"
          :title="$t('tips.selectPlaybackFile')"
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
  name: 'ProtobufLogPlayer',
  mixins: [mixin],
  data () {
    return {
      form: {
        'channels': [],
        'log-file': '',
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

  .protobuf-log-player {
    display: inline-block;
    width: 100%;
    vertical-align: top;
  }
</style>
