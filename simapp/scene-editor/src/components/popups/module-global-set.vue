<template>
  <div v-loading="loading" class="entity-mgr module-global-set">
    <!-- 模组的模块列表 -->
    <section v-if="form" class="entity-mgr-form">
      <el-form
        ref="form"
        :model="form"
        :rules="rules"
        label-width="60px"
        @submit.prevent
      >
        <div class="entity-mgr-form-item-list">
          <div class="entity-mgr-form-item-list-header">
            <span>{{ $t('module.list') }}</span>
            <div>
              <el-tooltip effect="dark" :content="$t('operation.add')" placement="bottom">
                <el-icon
                  id="module-add"
                  class="icon el-icon-plus"
                  :class="{ disabled }"
                  @click="handleModuleAdd"
                >
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
                    <span>{{ getCategoryText(item) }}</span>
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
                    <el-icon
                      class="icon el-icon-minus"
                      :class="{ disabled }"
                      @click.stop="handleModuleRemove(index)"
                    ><minus /></el-icon>
                    <el-icon
                      class="icon el-icon-top"
                      :class="{ disabled: disabled || index === 0 }"
                      @click.stop="handleModuleMoveUp(index, item)"
                    ><top /></el-icon>
                    <el-icon
                      class="icon el-icon-bottom"
                      :class="{ disabled: disabled || index === form.modules.length - 1 }"
                      @click.stop="handleModuleMoveDown(index, item)"
                    ><bottom /></el-icon>
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
              {{ $t('operation.apply') }}
            </el-button>
          </div>
        </div>
      </el-form>

      <ModuleAdd
        v-if="moduleAddVisible"
        :selected="form.modules"
        :global="true"
        @close="moduleAddVisible = false"
        @confirm="handleModuleAddConfirm"
      />
    </section>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapState } from 'vuex'
import dict from '@/common/dict'
import { errorHandler } from '@/common/errorHandler'
import { toPlainObject } from '@/common/utils'
import ModuleAdd from '@/components/popups/module-add.vue'
import InputNumber from '@/components/input-number.vue'

const { electron } = window

export default {
  name: 'ModuleSet',
  components: {
    InputNumber,
    ModuleAdd,
  },
  data () {
    return {
      form: {
        id: undefined,
        name: '',
        modules: [],
      },
      rules: {
      },
      moduleAddVisible: false,
      editingModuleIndex: -1,
    }
  },
  computed: {
    ...mapState('sim-modules', {
      moduleList: 'modules',
    }),
    ...mapState('module-set', [
      'loading',
    ]),
    ...mapGetters('module-set', [
      'globalModuleSet',
    ]),
    ...mapState('system', {
      coordinationMode: state => state.simulation.coordinationMode,
    }),
    category () {
      return dict.moduleGlobalCategoryList.reduce((map, item) => {
        return { ...map, [item.id]: item.name }
      }, {})
    },
    disabled () {
      return false
    },
  },
  watch: {
    globalModuleSet (value) {
      const form = toPlainObject(value)
      this.form = form
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
    ...mapActions('module-set', [
      'getModuleSetList',
      'updateModuleSet',
    ]),
    ...mapActions('system', {
      getSystemConfig: 'getConfig',
    }),
    isPreset (name) {
      return electron.modules.isPreset(name)
    },
    handleModuleAdd () {
      this.moduleAddVisible = true
    },
    async handleModuleRemove (index) {
      this.form.modules.splice(index, 1)
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
        return { name: m.name, priority: '50' }
      })
      this.form.modules = _.unionBy(selectedModule, this.form.modules, 'name')
    },
    async handleSave () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      if (this.form.id === undefined || this.form.id === '') {
        return errorHandler('全局模块数据有误')
      }
      const form = toPlainObject(this.form)
      try {
        await this.updateModuleSet(form)
        await this.resetModules()
      } catch (e) {
        await errorHandler(e)
      }
    },
    switchEditMode (index) {
      this.editingModuleIndex = index
      this.$nextTick(() => {
        this.$refs['priority-input'][index].focus()
      })
    },
    getCategoryText (item) {
      const moduleItem = this.moduleList.find(e => e.name === item.name)
      if (!moduleItem) return ''
      return this.$t(this.category[moduleItem.category])
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";
@import "@/assets/less/entity-mgr";

.module-global-set {
  .entity-mgr-form-item-list-body {
    height: 330px !important;
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
}
</style>
