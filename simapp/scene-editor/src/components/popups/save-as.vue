<template>
  <div class="save-as">
    <el-form
      ref="form"
      :model="form"
      :rules="rules"
      label-width="120px"
      @submit.prevent
    >
      <el-form-item :label="$t('tips.enterNewSceneName')" prop="name">
        <el-input v-model="form.name" maxlength="50">
          <template #append>
            <el-select
              v-model="form.ext"
              :placeholder="$t('tips.pleaseSelect')"
              class="save-as-select"
              popper-class="save-as-select-global"
              @change="validateForm"
            >
              <el-option
                v-if="!currentScenario || (currentScenario && currentScenario.type === 'sim')"
                label=".sim"
                value="sim"
              />
              <el-option label=".xosc" value="xosc" />
            </el-select>
          </template>
        </el-input>
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
</template>

<script>
import { find } from 'lodash-es'
import { mapActions, mapState } from 'vuex'
import { validateScenarioName } from '@/common/utils'

export default {
  name: 'SaveAs',
  data () {
    const validateName = (rule, value, callback) => {
      if (validateScenarioName(value)) {
        const matched = find(this.scenarioList, { filename: `${value}.${this.form.ext}` })
        if (matched) {
          callback(new Error(this.$t('tips.renameScene')))
        } else {
          callback()
        }
      } else {
        if (/\s/.test(value)) {
          callback(new Error(this.$t('tips.noWhitespaceInScenario')))
        } else {
          callback(new Error(this.$t('tips.sceneNameRuleTips')))
        }
      }
    }
    return {
      form: {
        name: '',
        ext: '',
      },
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterNewSceneName') },
          { validator: validateName, trigger: 'blur' },
        ],
      },
    }
  },
  computed: {
    ...mapState('scenario', [
      'currentScenario',
      'scenarioList',
    ]),
  },
  watch: {
    currentScenario: {
      handler (value) {
        if (value) {
          this.form.ext = value.type
        } else {
          this.form.ext = 'sim'
        }
      },
      immediate: true,
    },
  },
  created () {
    this.getScenarioList()
  },
  mounted () {
    this.setFormName(this.currentScenario.name)
    this.validateForm()
  },
  methods: {
    ...mapActions('scenario', [
      'getScenarioList',
      'saveAs',
    ]),
    validateForm () {
      this.$nextTick(() => {
        try {
          this.$refs.form.validate()
        } catch (e) {
        }
      })
    },
    async handleOK () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return
      }
      await this.saveAs(this.form)
      this.$emit('close')
    },
    setFormName (val) {
      this.form.name = val
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.save-as {
  width: 400px;
  height: auto;
  padding: 23px;

  .save-as-select {
    width: 75px;

    :deep(.el-input__icon) {
      display: none;
    }
  }

  .button-group {
    margin-top: 40px;
    text-align: right;
  }
}
</style>
