<template>
  <div v-loading="loading" class="entity-mgr module-set">
    <section class="entity-mgr-workshop">
      <div class="entity-mgr-workshop-keyword">
        <el-input
          id="module-set-search-name"
          :model-value="keyword"
          :placeholder="($t('operation.searchName'))"
          @input="updateState({ keyword: $event })"
        >
          <template #prefix>
            <el-icon class="el-input__icon">
              <Search />
            </el-icon>
          </template>
        </el-input>
      </div>
      <ul class="entity-mgr-workshop-list">
        <li
          v-for="(item, index) in filteredModuleSetList"
          :id="`module-set-node-${index}`"
          :key="item.id"
          class="entity-mgr-workshop-list-item"
          :class="{ 'entity-mgr-workshop-list-item-active': index === currentIndex }"
          @click="handleSelect(index)"
        >
          <span class="entity-mgr-workshop-list-item-name" :title="item.name">
            <span class="entity-mgr-workshop-list-item-name-text">{{ item.name }}</span>
          </span>
          <!-- 操作按钮 -->
          <span class="entity-mgr-workshop-list-item-icon">
            <span v-if="item.id <= 0" class="entity-mgr-workshop-list-item-preset-icon">{{ $t('preset') }} </span>
            <el-tooltip effect="dark" :content="$t('operation.export')" placement="bottom">
              <el-icon class="icon" @click="handleExport(item)"><Upload /></el-icon>
            </el-tooltip>
            <el-tooltip effect="dark" :content="$t('operation.copy')" placement="bottom">
              <el-icon class="icon" @click.stop="handleClone(item)"><CopyDocument /></el-icon>
            </el-tooltip>
            <el-tooltip effect="dark" :content="$t('operation.delete')" placement="bottom">
              <el-icon v-if="item.id > 0" class="icon" @click.stop="handleDelete(index, item)"><Delete /></el-icon>
            </el-tooltip>
          </span>
        </li>
      </ul>
      <div class="entity-mgr-workshop-operation">
        <el-tooltip effect="dark" :content="$t('operation.create')" placement="bottom">
          <el-icon id="module-set-create" class="icon" @click="handleCreate">
            <Plus />
          </el-icon>
        </el-tooltip>
        <el-tooltip effect="dark" :content="$t('operation.import')" placement="bottom">
          <el-icon id="module-set-import" class="icon" @click="handleImport">
            <Download />
          </el-icon>
        </el-tooltip>
      </div>
    </section>
    <!-- 模组的模块列表 -->
    <section v-if="form" class="entity-mgr-form">
      <el-form
        ref="form"
        :model="form"
        :rules="rules"
        label-width="60px"
        @submit.prevent
      >
        <el-form-item class="entity-mgr-form-item-name" :label="$t('name')" prop="name">
          <el-input id="module-set-name" v-model.trim="form.name" :disabled="disabled" name="name" />
        </el-form-item>
        <el-form-item class="entity-mgr-form-item-category" :label="$t('category')" prop="category">
          <el-select
            id="module-set-category"
            v-model="form.category"
            :disabled="disabled"
            :placeholder="($t('tips.pleaseSelect'))"
            style="width:100%"
            name="category"
          >
            <el-option v-for="op in moduleSetCategoryList" :key="op.id" :label="$t(op.name)" :value="op.id" />
          </el-select>
        </el-form-item>
        <div class="entity-mgr-form-item-list">
          <div class="entity-mgr-form-item-list-header">
            <span>{{ $t('module.list') }}</span>
            <div>
              <el-tooltip effect="dark" :content="$t('operation.add')" placement="bottom">
                <!-- <span
                  id="module-add"
                  class="icon el-icon-plus"
                  :class="{ disabled }"
                  @click="handleModuleAdd"
                ></span> -->
                <el-icon id="module-add" class="icon" :class="{ disabled }" @click="handleModuleAdd">
                  <Plus />
                </el-icon>
              </el-tooltip>
            </div>
          </div>
          <div class="entity-mgr-form-item-list-body">
            <ul v-if="form.modules.length" class="entity-mgr-form-item-list-body-list">
              <li
                v-for="(item, index) in form.modules"
                :key="`${item.name}-${index}`"
                class="entity-mgr-form-item-list-body-list-item"
              >
                <span class="entity-mgr-form-item-list-body-list-item-name" :title="item.name">
                  {{ item.name }}
                </span>
                <div>
                  <span
                    v-if="isPreset(item.name)"
                    class="entity-mgr-form-item-list-body-list-item-preset-icon"
                  >{{ $t('preset') }} </span>
                  <span class="entity-mgr-form-item-list-body-list-item-category">
                    <span>{{ $t(category[getCategory(item.name)]) }}</span>
                  </span>
                  <span v-show="coordinationMode === 3" class="entity-mgr-form-item-list-body-list-item-priority">
                    <span
                      v-show="editingModuleIndex !== index"
                      class="priority-value"
                      @click="switchEditMode(index)"
                    >{{ item.priority }}</span>
                    <InputNumber
                      v-show="editingModuleIndex === index"
                      ref="priority-input"
                      v-model="item.priority"
                      autofocus
                      class="priority-input"
                      :min="1"
                      :max="100"
                      @blur="editingModuleIndex = -1"
                    />
                  </span>
                  <!-- 操作按钮 -->
                  <span class="entity-mgr-form-item-list-body-list-item-icon">
                    <!-- <span
                      class="icon el-icon-minus"
                      :class="{ disabled }"
                      @click.stop="handleModuleRemove(index)"
                    ></span> -->
                    <el-icon
                      class="icon"
                      :class="{ disabled }"
                      @click.stop="handleModuleRemove(index)"
                    >
                      <Minus />
                    </el-icon>
                    <!-- <span
                      class="icon el-icon-top"
                      :class="{ disabled: disabled || index === 0 }"
                      @click.stop="handleModuleMoveUp(index, item)"
                    ></span> -->
                    <el-icon
                      class="icon"
                      :class="{ disabled: disabled || index === 0 }"
                      @click.stop="handleModuleMoveUp(index, item)"
                    ><Top /></el-icon>
                    <!-- <span
                      class="icon el-icon-bottom"
                      :class="{ disabled: disabled || index === form.modules.length - 1 }"
                      @click.stop="handleModuleMoveDown(index, item)"
                    ></span> -->
                    <el-icon
                      class="icon"
                      :class="{ disabled: disabled || index === form.modules.length - 1 }"
                      @click.stop="handleModuleMoveDown(index, item)"
                    ><Bottom /></el-icon>
                  </span>
                </div>
              </li>
            </ul>
            <div v-else class="entity-mgr-form-item-list-body-no-data">
              {{ $t('tips.clickAddModule', { b: '+' }) }}
            </div>
          </div>
          <div class="entity-mgr-form-item-list-footer">
            <el-button id="module-set-save" @click="handleSave">
              {{ $t('operation.save') }}
            </el-button>
          </div>
        </div>
      </el-form>

      <ModuleAdd
        v-if="moduleAddVisible"
        :selected="form.modules"
        @close="moduleAddVisible = false"
        @confirm="handleModuleAddConfirm"
      />
    </section>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import dict from '@/common/dict'
import { errorHandler } from '@/common/errorHandler'
import { toPlainObject, validateModuleSetName } from '@/common/utils'
import ModuleAdd from '@/components/popups/module-add.vue'
import InputNumber from '@/components/input-number.vue'

const { electron } = window
const { path, dialog } = electron
const defaultForm = {
  name: '',
  modules: [],
}

export default {
  name: 'ModuleSet',
  components: {
    InputNumber,
    ModuleAdd,
  },
  data () {
    const nameValidator = (rule, value, callback) => {
      if (!validateModuleSetName(value)) {
        callback(new Error(this.$t('tips.namePrefixErr', { name: this.$t('module.groupName') })))
        return
      }
      const index = this.getIndex(value)
      const matched = _.find(this.moduleSetList, { name: value })
      let existing = false
      if (this.currentIndex === -1) {
        existing = matched
      } else {
        if (this.currentIndex !== index) {
          existing = matched
        }
      }
      if (existing) {
        callback(new Error(this.$t('tips.nameExists')))
      } else {
        callback()
      }
    }

    return {
      form: {
        name: '',
        category: '',
        modules: [],
      },
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterName'), trigger: 'blur' },
          { validator: nameValidator, trigger: 'blur' },
        ],
        category: [
          { required: true, message: this.$t('tips.enterCategory'), trigger: 'blur' },
        ],
      },
      moduleAddVisible: false,
      currentIndex: -1,
      category: dict.moduleCategory,
      moduleSetCategoryList: dict.moduleSetCategoryList,
      editingModuleIndex: -1,
    }
  },
  computed: {
    ...mapState('sim-modules', {
      moduleList: 'modules',
    }),
    ...mapState('module-set', [
      'loading',
      'keyword',
    ]),
    ...mapGetters('module-set', [
      'moduleSetList',
      'filteredModuleSetList',
    ]),
    ...mapState('system', {
      coordinationMode: state => state.simulation.coordinationMode,
    }),
    disabled () {
      return this.form.id <= 0
    },
  },
  watch: {
    currentIndex (value) {
      if (value !== -1) {
        this.$nextTick(() => {
          const el = document.getElementById(`module-set-node-${value}`)
          if (el) {
            el.scrollIntoView()
          }
        })
      }
    },
    moduleList (value) {
      for (let i = 0; i < this.form.modules.length; i++) {
        const matched = _.find(value, { name: this.form.modules[i].name })

        if (!matched) {
          this.form.modules.splice(i, 1)
          i--
        }
      }
    },
  },
  async created () {
    await this.getModuleList()
    await this.getModuleSetList()
    await this.getSystemConfig()
  },
  methods: {
    ...mapActions('sim-modules', [
      'getModuleList',
      'resetModules',
    ]),
    ...mapMutations('module-set', [
      'updateState',
    ]),
    ...mapActions('module-set', [
      'getModuleSetList',
      'createModuleSet',
      'updateModuleSet',
      'destroyModuleSet',
      // 'activeModuleSet',
      'importModuleSet',
      'exportModuleSet',
      'removeModuleSet',
      'addModuleSet',
    ]),
    ...mapActions('system', {
      getSystemConfig: 'getConfig',
    }),
    isPreset (name) {
      return electron.modules.isPreset(name)
    },
    getIndex (name) {
      return _.findIndex(this.filteredModuleSetList, { name })
    },
    handleCreate () {
      this.currentIndex = -1
      this.form = toPlainObject(defaultForm)
    },
    async handleImport () {
      try {
        const { filePaths: sources } = await dialog.showOpenDialog({
          title: this.$t('tips.selectImModuleConfigFile'),
          properties: ['openFile'],
          filters: [
            { name: `Scheme ${this.$t('menu.file')}`, extensions: ['scheme'] },
          ],
        })
        if (sources && sources.length) {
          const [filePath] = sources
          const [filename] = filePath.split(electron.path.sep).pop().split('.')
          this.rules.name[1].validator(null, filename, (err) => {
            if (err) {
              this.$errorBox(err.message)
              return
            }
            this.importModuleSet(filePath)
          })
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
    handleModuleAdd () {
      this.moduleAddVisible = true
    },
    async handleModuleRemove (index) {
      this.form.modules.splice(index, 1)
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      const form = toPlainObject(this.form)
      if (this.currentIndex > -1) {
        try {
          await this.removeModuleSet(form)
        } catch (e) {
          await errorHandler(e)
        }
      }
    },
    handleModuleMoveUp (index, item) {
      if (index === 0) {
        return
      }
      const targetIndex = index - 1
      this.form.modules.splice(index, 1)
      this.form.modules.splice(targetIndex, 0, item)
    },
    handleModuleMoveDown (index, item) {
      if (index === this.form.modules.length - 1) {
        return
      }
      const targetIndex = index + 1
      this.form.modules.splice(index, 1)
      this.form.modules.splice(targetIndex, 0, item)
    },
    async handleModuleAddConfirm (payload) {
      const selectedModule = payload.map((n) => {
        const m = this.moduleList.find(({ name }) => name === n)
        return { name: m.name, priority: 50 }
      })
      this.form.modules = _.unionBy(selectedModule, this.form.modules, 'name')
      if (this.currentIndex !== -1) {
        try {
          await this.$refs.form.validate()
        } catch (e) {
          return false
        }
        const form = toPlainObject(this.form)
        try {
          await this.addModuleSet(form)
        } catch (e) {
          await errorHandler(e)
        }
      }
    },
    async handleSelect (index) {
      if (this.currentIndex === index) return
      this.currentIndex = index
      this.form = toPlainObject(this.filteredModuleSetList[index])
      this.$nextTick(() => {
        this.$refs.form.clearValidate()
      })
    },
    async handleExport (item) {
      const { name } = item
      try {
        const { filePaths } = await dialog.showOpenDialog({
          title: this.$t('tips.selectImModuleConfigFileDir'),
          properties: ['openDirectory'],
        })
        const [dest] = filePaths
        if (dest) {
          this.exportModuleSet({
            name,
            path: `${dest + path.sep + name}.scheme`,
          })
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
    async handleClone (item) {
      const form = toPlainObject(item)
      form.name = `${form.name}_clone`
      delete form.id
      this.currentIndex = -1
      this.form = form
    },
    async handleDelete (index, item) {
      const { id } = item
      try {
        await this.$confirmBox(this.$t('tips.whetherDeleteModuleConfig'))
      } catch (e) {
        return
      }
      try {
        await this.destroyModuleSet(id)
        if (index === this.currentIndex) {
          let currentIndex = -1
          if (index === 0) {
            if (this.filteredModuleSetList.length) {
              currentIndex = 0
            }
          } else if (index > 0) {
            currentIndex = index - 1
          }
          if (currentIndex === -1) {
            this.currentIndex = -1
            this.form = null
          } else {
            this.handleSelect(currentIndex)
          }
        } else if (index < this.currentIndex) {
          this.currentIndex = this.currentIndex - 1
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
    async handleSave () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      // if (this.form.modules.length === 0) {
      //   return errorHandler(this.$t('tips.selectAtLeastOneModule'))
      // }
      const form = toPlainObject(this.form)
      try {
        if (this.currentIndex === -1) {
          await this.createModuleSet(form)
          const index = this.getIndex(form.name)
          if (index === -1) {
            this.form = null
          } else {
            this.currentIndex = index
            this.form = this.filteredModuleSetList[index]
          }
        } else {
          await this.updateModuleSet(form)
        }
        await this.resetModules()
      } catch (e) {
        await errorHandler(e)
      }
    },
    getCategory (name) {
      const { category } = this.moduleList.find(m => m.name === name)
      return category
    },
    switchEditMode (index) {
      this.editingModuleIndex = index
      this.$nextTick(() => {
        this.$refs['priority-input'][index].focus()
      })
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";
  @import "@/assets/less/entity-mgr";

  .entity-mgr-workshop-list-item-name {
    max-width: 120px !important;

    .entity-mgr-workshop-list-item-name-text {
      margin-left: 3px;
    }
  }

  .entity-mgr.module-set {
    .entity-mgr-form-item-list-body {
      height: 250px !important;
    }
  }

  .entity-mgr-workshop-list-item-preset-icon,
  .entity-mgr-form-item-list-body-list-item-preset-icon {
    padding: 0 2px;
    font-size: 10px;
    line-height: 10px;
    color: @disabled-color;
    border: 1px solid @disabled-color;
    border-radius: 1px;
  }

  .entity-mgr-workshop-list-item-preset-icon {
    margin-left: 10px;
    font-size: 12px;
    line-height: 12px;
  }

  .entity-mgr-form-item-list-body-list-item-category {
    display: inline-block;
    width: 70px;
    margin-left: 10px;
  }

  .priority-value {
    display: inline-block;
    width: 33px;
    padding-left: 7px;
    height: 24px;
    line-height: 24px;
  }

  .priority-input {
    width: 40px;
  }
  .entity-mgr-form-item-list-footer {
    .el-button {
      background-color: @darker-bg;
      border-color: #404040;
      color: #878787;
    }
  }
</style>
