<template>
  <div v-loading="loading" class="module-setting">
    <div class="module-setting-workshop">
      <section class="part-list">
        <div class="list-search">
          <el-input
            id="module-search-name"
            v-model="keyword"
            style="width: 200px;"
            prefix-icon="search"
            :placeholder="($t('operation.searchName'))"
          />
        </div>
        <el-collapse :model-value="listActiveCollapseNames" class="module-category-list">
          <el-collapse-item
            v-for="moduleCategory of filteredComputedModuleCategoryList"
            :id="`module-category-${moduleCategory.name}`"
            :key="moduleCategory.id"
            :name="moduleCategory.id"
            class="module-category-list-item"
          >
            <template #title>
              <div class="module-category-list-item-name" :title="moduleCategory.name">
                {{ $t(moduleCategory.name) }}
              </div>
            </template>
            <ul class="list-container">
              <li
                v-for="item of moduleCategory.modules"
                :id="`module-node-${moduleCategory.id}-${item.name}`"
                :key="`${moduleCategory.id}-${item.name}`"
                class="module-item"
                :class="{ active: item.name === current.name }"
                @click="handleSelect(item)"
              >
                <span class="module-name" :title="item.name">
                  {{ item.name }}
                </span>
                <span class="operate-icon">
                  <span v-if="isPreset(item.name)" class="preset-icon">{{ $t('preset') }} </span>
                  <el-tooltip v-else effect="dark" :content="$t('operation.delete')" placement="bottom">
                    <el-icon class="icon el-icon-delete" @click.stop="handleDelete(item.name)"><delete /></el-icon>
                  </el-tooltip>
                  <el-tooltip effect="dark" :content="$t('operation.copy')" placement="bottom">
                    <el-icon
                      class="icon el-icon-copy-document"
                      @click.stop="handleClone(item)"
                    ><copy-document /></el-icon>
                  </el-tooltip>
                </span>
              </li>
            </ul>
          </el-collapse-item>
        </el-collapse>
        <div class="list-operate">
          <el-tooltip effect="dark" :content="$t('operation.create')" placement="bottom">
            <el-icon id="module-create" class="icon el-icon-plus" @click="handleCreate">
              <plus />
            </el-icon>
          </el-tooltip>
        </div>
      </section>
      <section v-if="form" class="part-detail">
        <el-form
          ref="form"
          label-width="108px"
          class="module-props"
          size="small"
          :model="form"
          :rules="rules"
          @submit.prevent
        >
          <el-collapse v-model="formActiveCollapseNames">
            <el-collapse-item id="module-basic" :title="$t('scenario.basicInformation')" name="basic">
              <el-form-item :label="$t('name')" prop="name">
                <el-input id="module-form-name" v-model="form.name" :disabled="action === 'update'" name="name" />
              </el-form-item>
              <el-form-item :label="$t('classification')" prop="category">
                <el-select
                  id="module-form-category-select"
                  v-model="form.category"
                  :disabled="disabled"
                  style="width: 100px"
                >
                  <el-option
                    v-for="category in categoryList"
                    :id="`module-form-category-option-${category.name}`"
                    :key="category.id"
                    :label="$t(category.name)"
                    :value="category.id"
                  />
                </el-select>
              </el-form-item>
              <el-form-item :label="$t('module.callCycle')" prop="execPeriod">
                <InputNumber
                  id="module-form-exec-period"
                  v-model="form.execPeriod"
                  :disabled="disabled"
                  unit="ms"
                  :min="0"
                  name="execPeriod"
                  style="width: 100px"
                />
              </el-form-item>
              <el-form-item :label="$t('module.expectedExecutionTime')" prop="responseTime">
                <InputNumber
                  id="module-form-response-time"
                  v-model="form.responseTime"
                  :disabled="disabled"
                  unit="ms"
                  :min="0"
                  name="responseTime"
                  style="width: 100px"
                />
                <span class="module-prop-tip" :title="$t('module.validInAsync')">{{ $t('module.validInAsync') }}</span>
              </el-form-item>
              <el-form-item :label="$t('module.startAutomatically')" prop="autoLaunch">
                <el-checkbox
                  id="module-form-auto-launch"
                  v-model="form.autoLaunch"
                  :disabled="disabled"
                  name="autoLaunch"
                />
              </el-form-item>
              <el-form-item :label="$t('module.accessMode')">
                <el-radio-group
                  id="module-form-mode"
                  v-model="current.mode"
                  :disabled="disabled"
                  @change="resetModeValid"
                >
                  <el-radio id="module-form-mode-bin" label="bin" value="bin">
                    {{ $t('module.execFile') }}
                  </el-radio>
                  <el-radio id="module-form-mode-so" label="so" value="so">
                    {{ $t('module.linkLib') }}
                  </el-radio>
                </el-radio-group>
              </el-form-item>
              <template v-if="current.mode === 'bin'">
                <el-form-item :label="$t('module.exePath')" prop="binPath" :required="!!rules.binPath">
                  <InputFileBrowser
                    id="module-form-bin-path"
                    v-model="form.binPath"
                    :title="$t('tips.selectExecutableFile')"
                    :properties="['openFile']"
                    :disabled="disabled"
                  />
                </el-form-item>
                <el-form-item :label="$t('module.exeParam')" prop="binArgs">
                  <InputObject id="module-form-bin-args" v-model="form.binArgs" :disabled="disabled" />
                </el-form-item>
              </template>
              <template v-if="current.mode === 'so'">
                <el-form-item :label="$t('module.linkPath')" prop="soPath" :required="!!rules.soPath">
                  <InputFileBrowser
                    id="module-form-so-path"
                    v-model="form.soPath"
                    :title="$t('tips.selectDynamicLinkLibraryPath')"
                    :properties="['openFile']"
                    :disabled="disabled"
                  />
                </el-form-item>
              </template>
              <el-form-item v-if="platform === 'linux'" :label="$t('module.depPath')" prop="depPaths">
                <InputObject id="module-form-dep-paths" v-model="form.depPaths" :disabled="disabled" />
              </el-form-item>
            </el-collapse-item>
            <el-collapse-item id="module-advanced" :title="$t('module.advancedConf')" name="advanced">
              <el-form-item :label="$t('module.initParam')" prop="initArgs" name="initArgs">
                <component :is="currentInitArgsComponent" v-model="form.initArgs" />
              </el-form-item>
              <el-form-item :label="$t('module.execTimeout')" prop="cmdTimeout">
                <InputNumber
                  id="module-form-cmd-timeout"
                  v-model="form.cmdTimeout"
                  :disabled="disabled"
                  unit="ms"
                  :min="0"
                  name="cmdTimeout"
                />
              </el-form-item>
              <el-form-item :label="$t('module.singleStepTimeout')" prop="stepTimeout">
                <InputNumber
                  id="module-form-step-timeout"
                  v-model="form.stepTimeout"
                  :disabled="disabled"
                  unit="ms"
                  :min="0"
                  name="stepTimeout"
                />
              </el-form-item>
            </el-collapse-item>
          </el-collapse>
          <div class="part-buttons">
            <el-button id="module-save" class="dialog-ok" @click="handleSave">
              {{ $t('operation.save') }}
            </el-button>
          </div>
        </el-form>
      </section>
    </div>
    <div class="module-setting-buttons">
      <el-button
        id="module-dialog-close"
        class="dialog-cancel"
        @click="$emit('close')"
      >
        {{ $t('operation.close') }}
      </el-button>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import Mousetrap from 'mousetrap'
import dict from '@/common/dict'
import { errorHandler } from '@/common/errorHandler'
import { toPlainObject } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'
import InputFileBrowser from '@/components/input-file-browser.vue'
import InputObject from '@/components/input-object.vue'
import InputKeyValue from '@/components/input-key-value.vue'
import Traffic from '@/components/popups/module-init-arguments/traffic.vue'
import ProtobufLogPlayer from '@/components/popups/module-init-arguments/protobuf-log-player.vue'
import Grading from '@/components/popups/module-init-arguments/grading.vue'
import { messageBoxConfirm } from '@/plugins/message-box'

const { electron } = window
const { platform } = electron

export default {
  name: 'ModuleSetting',
  components: {
    InputNumber,
    InputFileBrowser,
    InputObject,
    InputKeyValue,
    Traffic,
    ProtobufLogPlayer,
    Grading,
  },
  props: {
    // false主车算法，true全局模块
    global: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    const existingValidator = (rule, value, callback) => {
      if (this.action === 'create') {
        const name = value.trim()
        if (name === '') {
          callback(new Error(this.$t('tips.enterNonnullValue')))
          return
        }
        const matched = _.find(this.modules, { name })
        if (matched) {
          callback(new Error(this.$t('tips.nameExists')))
          return
        }
      }
      callback()
    }
    const requiredValidator = (rule, value, callback) => {
      const { autoLaunch } = this.form
      if (autoLaunch) {
        if (value) {
          callback()
        } else {
          const { fullField } = rule
          if (fullField === 'binPath') {
            callback(new Error(this.$t('tips.enterExecutableFilePath')))
          } else if (fullField === 'soPath') {
            callback(new Error(this.$t('tips.enterDynamicLinkLibraryPath')))
          }
        }
      } else {
        callback()
      }
    }
    const jsonValidator = (rule, value, callback) => {
      if (value === null) {
        callback(new Error(this.$t('tips.enterFormat', { format: 'JSON' })))
      } else {
        const { fullField } = rule
        if (fullField === 'binArgs' || fullField === 'depPaths') {
          if (_.isArray(value)) {
            callback()
          } else {
            callback(new Error(this.$t('tips.enterArrayFormat', { format: 'JSON' })))
          }
        }
      }
    }

    return {
      platform,
      keyword: '',
      current: {
        name: '',
        mode: 'bin',
      },
      form: null,
      originalForm: null,
      binRules: {
        binPath: [
          { required: true, validator: requiredValidator, trigger: 'blur' },
        ],
        binArgs: [
          { validator: jsonValidator, trigger: 'blur' },
        ],
      },
      depPathRules: {
        depPaths: [
          { validator: jsonValidator, trigger: 'blur' },
        ],
      },
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterName'), trigger: 'blur' },
          { pattern: '^[^:]*$', message: this.$t('tips.moduleNameNotContainColon'), trigger: 'blur' },
          { validator: existingValidator, trigger: 'blur' },
        ],
        category: [
          { required: true, message: this.$t('tips.selectType'), trigger: 'blur' },
        ],
        execPeriod: [
          { required: true, message: this.$t('tips.enterCallingCycle'), trigger: 'blur' },
          { type: 'number', message: this.$t('tips.enterNumber'), trigger: 'blur' },
          {
            validator (rule, val, callback) {
              if (val <= 0) {
                callback(new Error(this.$t('tips.callPeriodMustGreaterThan', { value: 0 })))
                return
              }
              callback()
            },
            trigger: 'blur',
          },
        ],
        responseTime: [
          { required: true, message: this.$t('tips.enterExpectedExecutionTime'), trigger: 'blur' },
        ],
        soPath: [
          { required: true, validator: requiredValidator, trigger: 'blur' },
        ],
        // ...this.binRules, 由 watch 函数的immediate添加
      },
      formActiveCollapseNames: ['basic'],
      administratorMode: false,
      isDirty: false,
    }
  },
  computed: {
    ...mapState('sim-modules', [
      'loading',
      'modules',
    ]),
    ...mapGetters('sim-modules', [
      'modulesActive',
    ]),
    action () {
      if (this.current.name === '') {
        return 'create'
      } else {
        return 'update'
      }
    },
    disabled () {
      if (this.administratorMode) {
        return false
      } else {
        return this.action === 'update' && this.isPreset(this.form.name)
      }
    },
    categoryList () {
      if (this.global) return dict.moduleGlobalCategoryList
      return dict.moduleCategoryList
    },
    listActiveCollapseNames () {
      return _.map(this.categoryList, 'id')
    },
    computedModuleCategoryList () {
      return _.map(this.categoryList, (category) => {
        const modules = []
        _.forEach(this.modules, (item) => {
          if (item.category === category.id) {
            modules.push(item)
          }
        })
        return {
          ...category,
          modules,
        }
      })
    },
    filteredComputedModuleCategoryList () {
      const list = []
      _.forEach(this.computedModuleCategoryList, (category) => {
        const modules = _.filter(category.modules, (item) => {
          return item.name.includes(this.keyword)
        })
        let showItemWhenListEmpty = true
        if (this.keyword && _.size(modules) === 0) {
          showItemWhenListEmpty = false
        }
        if (showItemWhenListEmpty) {
          list.push({
            ...category,
            modules,
          })
        }
      })
      return list
    },
    sortedModuleList () {
      const moduleList = []
      _.forEach(this.filteredComputedModuleCategoryList, (category) => {
        _.forEach(category.modules, (item) => {
          moduleList.push(item)
        })
      })
      return moduleList
    },
    currentInitArgsComponent () {
      const object = {
        Traffic,
        Protobuf_Log_Player: ProtobufLogPlayer,
        Grading,
      }
      return object[this.form.name] || InputKeyValue
    },
  },
  watch: {
    'form.autoLaunch': {
      immediate: true,
      handler: 'toggleValidMode',
    },
  },
  created () {
    this.init()
    Mousetrap.bind('ctrl+shift+d', () => {
      this.administratorMode = !this.administratorMode
    })
  },
  beforeUnmount () {
    Mousetrap.unbind('ctrl+shift+d')
  },
  methods: {
    ...mapMutations('sim-modules', [
      'moveModuleUp',
      'moveModuleDown',
    ]),
    ...mapActions('sim-modules', [
      'getModuleList',
      'createModule',
      'updateModule',
      'destroyModule',
      'resetModules',
    ]),
    ...mapActions('module-set', [
      'getModuleSetList',
    ]),
    async beforeClose () {
      const result = await this.promptFormChanged()
      if (this.isDirty && result) {
        await this.resetModules()
      }
      return result
    },
    isPreset (name) {
      return electron.modules.isPreset(name)
    },
    getFirstModuleFromFilteredComputedModuleCategoryList () {
      let first = null
      _.forEach(this.filteredComputedModuleCategoryList, (category) => {
        if (first === null) {
          first = _.first(category.modules)
        }
      })
      return first
    },
    getCurrentModule (name) {
      const index = _.findIndex(this.sortedModuleList, { name })
      if (index === 0) {
        return this.sortedModuleList[index + 1]
      } else if (index > 0) {
        return this.sortedModuleList[index - 1]
      }
    },
    async init () {
      try {
        await this.getModuleList()
        const first = this.getFirstModuleFromFilteredComputedModuleCategoryList()
        if (first) {
          this.handleSelect(first)
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
    setForm (form) {
      const defaultCategory = this.global ? 1 : 101
      const defaultForm = {
        name: '',
        category: defaultCategory,
        execPeriod: 20,
        responseTime: 20,
        autoLaunch: true,
        binPath: '',
        binArgs: [],
        soPath: '',
        depPaths: [],
        cmdTimeout: 60000,
        stepTimeout: 30000,
        initArgs: {},
      }
      if (form === undefined) {
        this.form = defaultForm
      } else if (form === null) {
        this.form = form
      } else {
        this.form = {
          ...defaultForm,
          ...form,
        }
      }
    },
    setCurrentMode (mode) {
      if (mode) {
        this.current.mode = mode
      } else {
        if (this.form) {
          if (this.form.binPath) {
            this.current.mode = 'bin'
          }
          if (this.form.soPath) {
            this.current.mode = 'so'
          }
        }
      }
    },
    async promptFormChanged () {
      if (this.originalForm === null) {
        return true
      }
      if (!_.isEqual(this.originalForm, toPlainObject(this.form))) {
        try {
          await messageBoxConfirm(this.$t('tips.whetherToSaveConfigChange'))
          return this.handleSave()
        } catch (e) {
          return true
        }
      }
      return true
    },
    async handleSelect (item) {
      const result = await this.promptFormChanged()
      if (!result) {
        return
      }
      console.log('edit module', item)
      this.current.name = item.name
      this.setForm(item)
      this.setCurrentMode()
      this.formActiveCollapseNames = ['basic']
      this.originalForm = toPlainObject(this.form)
      this.$nextTick(() => {
        this.$refs.form.validate()
      })
    },
    async handleDelete (name) {
      try {
        await messageBoxConfirm(this.$t('tips.confirmDeleteModule'))
      } catch (e) {
        return
      }
      try {
        const currentModule = this.getCurrentModule(name)
        await this.destroyModule(name)
        await this.getModuleSetList()
        if (this.current.name === name) {
          if (currentModule) {
            this.handleSelect(currentModule)
          } else {
            this.current.name = ''
            this.setForm(null)
            this.originalForm = null
          }
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
    async handleClone (item) {
      const form = toPlainObject(item)
      form.name = `${form.name}_clone`
      this.current.name = ''
      this.$refs.form.clearValidate()
      this.setForm(form)
      this.setCurrentMode()
      this.formActiveCollapseNames = ['basic']
    },
    handleCreate () {
      this.current.name = ''
      this.setForm()
      this.setCurrentMode('bin')
      this.formActiveCollapseNames = ['basic']
    },
    async handleSave () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      const form = toPlainObject(this.form)
      if (this.current.mode === 'bin') {
        form.soPath = ''
      } else if (this.current.mode === 'so') {
        form.binPath = ''
        form.binArgs = []
      }
      try {
        if (this.action === 'create') {
          await this.createModule(form)
          this.current.name = form.name
        } else {
          await this.updateModule(form)
        }
        this.originalForm = toPlainObject(this.form)
        this.isDirty = true
        return true
      } catch (e) {
        await errorHandler(e)
        return false
      }
    },
    resetModeValid () {
      _.invoke(this, '$refs.form.clearValidate', ['binPath', 'soPath', 'binArgs', 'depPaths'])
    },
    toggleValidMode (val) {
      try {
        if (!val) {
          delete this.rules.binPath
          delete this.rules.binArgs
          delete this.rules.depPaths
          this.resetModeValid()
          // this.$forceUpdate()
        } else {
          Object.assign(this.rules, this.binRules)
          if (this.platform === 'linux') {
            Object.assign(this.rules, this.depPathRules)
          }
          this.resetModeValid()
        }
      } catch (e) {
        console.error(e)
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.module-setting {
  display: flex;
  flex-direction: column;
  height: 400px;

  @left-width: 250px;
  @right-bottom-height: 40px;

  .icon {
    color: @global-font-color;
    font-size: 16px;
    cursor: pointer;

    &.disabled {
      opacity: 0.3;
    }
  }

  .part-list {
    height: 100%;
    display: flex;
    flex-direction: column;
    flex: 0 0 @left-width;
    padding-top: 10px;
    border-right: 1px solid @darker-bg;
    font-size: 12px;
    color: @title-font-color;
    overflow-y: auto;

    .list-search {
      height: 50px;
      line-height: 50px;
      text-align: center;
    }

    .list-container {
      flex: 1;
      list-style: none;

      .module-item {
        height: 32px;
        line-height: 32px;
        padding: 0 15px;
        border-bottom: solid 1px @list-bd;
        display: flex;
        justify-content: space-between;
        align-items: center;

        .module-name {
          .text-overflow();
          width: 120px;
          margin-left: 20px;
        }

        &:hover,
        &.active {
          color: @active-font-color;
          background-color: @dark-bg;
        }

        .operate-icon {
          .preset-icon {
            margin-left: 10px;
            padding: 0 2px;
            font-size: 10px;
            color: @disabled-color;
            border: 1px solid @disabled-color;
            border-radius: 1px;
          }

          .icon {
            margin-left: 5px;
            vertical-align: middle;
          }
        }
      }
    }

    .list-operate {
      height: 30px;
      margin: 10px 0;
      line-height: 30px;
      border-top: 1px solid @darker-bg;
      text-align: right;

      .icon {
        margin: 0 5px;
      }
    }

    .module-category-list {
      flex: 1;
      padding-bottom: 5px;
      overflow-y: auto;

      .module-category-list-item {
        .module-category-list-item-name {
          width: 180px;
          color: #9d9d9d;
          text-align: left;
          .text-overflow();
        }

        :deep(.el-collapse-item__arrow) {
          color: #9d9d9d;
        }
      }
    }
  }

  .part-detail {
    height: 100%;
    flex: 1;
    overflow-y: auto;
    padding: 0 20px 0 5px;
    color: @title-font-color;
    font-size: 12px;
    display: flex;
    flex-direction: column;

    .module-props {
      padding-top: 16px;
      flex: 1;

      .module-prop-tip {
        .text-overflow;
        margin-left: 10px;
        font-size: 12px;
        color: @disabled-color;
      }
    }

    :deep(.el-textarea .el-textarea__inner) {
      word-break: break-all;
    }

    :deep(.el-collapse-item .el-collapse-item__content) {
      margin-bottom: 0;
      padding: 10px 0;
    }

    .form-row {
      margin-top: 10px;
      margin-left: 10px;
      display: flex;
    }

    .part-buttons {
      margin: 10px 0;
      text-align: right;
    }
  }

  .module-setting-workshop {
    display: flex;
    align-items: stretch;
    flex: 1;
    overflow-y: hidden;
  }

  .module-setting-buttons {
    padding: 10px 20px;
    border-top: 1px solid @darker-bg;
  }
}

.input-unit {
  color: @disabled-color;
}
</style>
