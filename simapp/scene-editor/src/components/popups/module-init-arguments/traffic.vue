<template>
  <div class="traffic">
    <el-form
      ref="form"
      size="small"
      :model="form"
      :rules="rules"
      @submit.prevent
    >
      <el-form-item prop="random_seed">
        <el-checkbox
          v-model="form.random_seed"
          true-label="true"
          false-label="false"
          :title="$t('module.AIRandomSeedForTrafficFlowModelTip')"
          @change="handleChange"
        >
          {{ $t('module.AIRandomSeedForTrafficFlowModel') }}
        </el-checkbox>
      </el-form-item>
    </el-form>
  </div>
</template>

<script>
import _ from 'lodash'

export default {
  name: 'Traffic',
  props: {
    modelValue: {
      validator (value) {
        return _.isPlainObject(value) || value === null
      },
      required: true,
    },
  },
  data () {
    return {
      form: {
        random_seed: 'false',
      },
      rules: {},
    }
  },
  watch: {
    modelValue: {
      handler (value) {
        if (value) {
          this.form = {
            ...value,
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
      this.$emit('update:model-value', form)
      this.$emit('change', form)
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .traffic {
    display: inline-block;
    width: 100%;
    vertical-align: top;
  }
</style>
