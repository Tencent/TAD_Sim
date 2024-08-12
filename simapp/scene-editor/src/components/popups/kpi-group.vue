<template>
  <div v-loading="loading" class="entity-mgr">
    <section class="entity-mgr-workshop">
      <div class="entity-mgr-workshop-keyword">
        <el-input
          id="kpi-group-search-name"
          :model-value="keyword"
          prefix-icon="search"
          :placeholder="$t('operation.searchName')"
          @input="updateState({ keyword: $event })"
        />
      </div>
      <ul id="kpi-group-list" class="entity-mgr-workshop-list">
        <li
          v-for="(item, index) in filteredKpiGroupList"
          :id="`kpi-group-node-${index}`"
          :key="item.id"
          class="entity-mgr-workshop-list-item"
          :class="{ 'entity-mgr-workshop-list-item-active': index === currentIndex }"
          @click="handleSelect(index)"
        >
          <div :id="`kpi-group-node-${item.name}`">
            <span class="entity-mgr-workshop-list-item-name" :title="item.name">
              <span class="entity-mgr-workshop-list-item-name-checked">
                <checked-svg v-if="activeKpiGroupId === item.id" />
              </span>
              <span class="entity-mgr-workshop-list-item-name-text">
                {{ item.name }}
              </span>
            </span>
            <span class="entity-mgr-workshop-list-item-icon">
              <span v-if="item.id === 0" class="entity-mgr-workshop-list-item-preset-icon">{{ $t('preset') }}</span>
              <el-tooltip effect="dark" :content="$t('operation.copy')" placement="bottom">
                <el-icon class="icon el-icon-copy-document" @click.stop="handleClone(item)"><copy-document /></el-icon>
              </el-tooltip>
              <el-tooltip v-if="item.id > 0" effect="dark" :content="$t('operation.delete')" placement="bottom">
                <el-icon class="icon el-icon-delete" @click.stop="handleDelete(index, item)"><Delete /></el-icon>
              </el-tooltip>
            </span>
          </div>
        </li>
      </ul>
      <div class="entity-mgr-workshop-operation">
        <el-tooltip effect="dark" :content="$t('operation.create')" placement="bottom">
          <el-icon id="kpi-group-create" class="icon el-icon-plus" @click="handleCreate">
            <Plus />
          </el-icon>
        </el-tooltip>
      </div>
    </section>
    <section v-if="form" class="entity-mgr-form">
      <el-form
        ref="form"
        :model="form"
        :rules="rules"
        label-width="60px"
        @submit.prevent
      >
        <el-form-item class="entity-mgr-form-item-name" :label="$t('name')" prop="name">
          <el-input id="kpi-group-name" v-model.trim="form.name" :disabled="disabled" name="name" />
        </el-form-item>
        <div class="entity-mgr-form-item-list">
          <div class="entity-mgr-form-item-list-header">
            <span>{{ $t('indicator.list') }}</span>
            <div>
              <el-tooltip effect="dark" :content="$t('operation.add')" placement="bottom">
                <el-icon id="kpi-add" class="icon el-icon-plus" :class="{ disabled }" @click="handleKpiAdd">
                  <Plus />
                </el-icon>
              </el-tooltip>
            </div>
          </div>
          <div class="entity-mgr-form-item-list-body">
            <ul v-if="kpis.length" class="entity-mgr-form-item-list-body-list">
              <li
                v-for="(item, index) in kpis"
                :key="`${item.name}-${index}`"
                class="entity-mgr-form-item-list-body-list-item"
              >
                <span class="entity-mgr-form-item-list-body-list-item-name" :title="$kpiAlias(item)">
                  {{ $kpiAlias(item) }}
                </span>
                <div class="entity-mgr-form-item-list-body-list-item-right">
                  <span
                    class="entity-mgr-form-item-list-body-list-item-category"
                    :title="getCategoryTxt(item)"
                  >
                    {{ getCategoryTxt(item) }}
                  </span>
                  <span class="entity-mgr-form-item-list-body-list-item-icon">
                    <el-icon
                      class="icon el-icon-edit"
                      :class="{ disabled: disabled || form.id === undefined }"
                      @click.stop="handleKpiUpdate(index)"
                    ><edit-pen /></el-icon>
                    <el-icon
                      class="icon el-icon-minus"
                      :class="{ disabled }"
                      @click.stop="handleKpiRemove(index, item.id)"
                    ><minus /></el-icon>
                  </span>
                </div>
              </li>
            </ul>
            <div v-else class="entity-mgr-form-item-list-body-no-data">
              {{ $t('tips.NoIndicatorClickToAdd', { b: '+' }) }}
            </div>
          </div>
          <div class="entity-mgr-form-item-list-footer">
            <el-button id="kpi-group-apply" :disabled="form.id === undefined" @click="handleApplyKpiGroup(form.id)">
              {{ $t('operation.applyScenaryDefaultConf') }}
            </el-button>
            <el-button id="kpi-group-save" :disabled="disabled" @click="handleSave">
              {{
                $t('operation.save')
              }}
            </el-button>
          </div>
        </div>
      </el-form>

      <KpiAdd
        v-if="kpiAddVisible"
        :selected="kpis"
        @close="kpiAddVisible = false"
        @confirm="handleKpiAddConfirm"
      />

      <KpiUpdate
        v-if="currentUpdateIndex >= 0"
        :value="kpis[currentUpdateIndex]"
        @close="currentUpdateIndex = -1"
        @confirm="handleKpiUpdateConfirm"
      />
    </section>
  </div>
</template>

<script>
import { find, findIndex } from 'lodash-es'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { errorHandler } from '@/common/errorHandler'
import { toPlainObject, validateKpiGroupName } from '@/common/utils'
import KpiAdd from '@/components/popups/kpi-add.vue'
import KpiUpdate from '@/components/popups/kpi-update.vue'
import KpiMixin from '@/components/popups/kpi-form-new/mixin'

const defaultForm = {
  name: '',
  filePath: '',
}

export default {
  name: 'KpiGroup',
  components: {
    KpiAdd,
    KpiUpdate,
  },
  mixins: [KpiMixin],
  data () {
    const nameValidator = (rule, value, callback) => {
      if (!validateKpiGroupName(value)) {
        callback(new Error(this.$t('tips.namePrefixErr', { name: this.$t('indicator.groupName') })))
        // callback(new Error('指标组配置名称前缀只能包含字母、数字、中文、加号、减号、下划线、点号，且首字符必须是字母、数字或中文'))
        return
      }
      const index = this.getIndexByName(value)
      const matched = find(this.kpiGroupList, { name: value })
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
        ...defaultForm,
      },
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterName'), trigger: 'blur' },
          { validator: nameValidator, trigger: 'blur' },
        ],
      },
      kpiAddVisible: false,
      currentIndex: -1,
      kpis: [],
      currentUpdateIndex: -1,
    }
  },
  computed: {
    ...mapState('kpi-group', [
      'loading',
      'keyword',
      'activeKpiGroupId',
      'kpiGroupList',
    ]),
    ...mapGetters('kpi-group', [
      'filteredKpiGroupList',
    ]),
    disabled () {
      return this.form.id <= 0
    },
  },
  watch: {
    currentIndex (value) {
      if (value !== -1) {
        this.$nextTick(() => {
          const el = document.getElementById(`kpi-group-node-${value}`)
          if (el) {
            el.scrollIntoView()
          }
        })
      }
    },
  },
  async created () {
    await this.getKpiGroupList()
    const index = this.getIndexById(this.activeKpiGroupId)
    if (index >= 0) {
      this.handleSelect(index)
    }
  },
  methods: {
    ...mapActions('kpi', [
      'updateKpi',
      'destroyKpi',
    ]),
    ...mapMutations('kpi-group', [
      'updateState',
    ]),
    ...mapActions('kpi-group', [
      'getKpiGroupList',
      'createKpiGroup',
      'updateKpiGroup',
      'destroyKpiGroup',
      'activeKpiGroup',
      'createKpisByKpiGroup',
      'getKpisByKpiGroup',
    ]),
    getIndexById (id) {
      return findIndex(this.filteredKpiGroupList, { id })
    },
    getIndexByName (name) {
      return findIndex(this.filteredKpiGroupList, { name })
    },
    handleCreate () {
      this.currentIndex = -1
      this.form = toPlainObject(defaultForm)
      this.kpis = []
    },
    handleKpiAdd () {
      this.kpiAddVisible = true
    },
    handleKpiUpdate (index) {
      this.currentUpdateIndex = index
    },
    async handleKpiRemove (index, id) {
      if (this.form.id > 0) {
        await this.destroyKpi(id)
      }
      this.kpis.splice(index, 1)
    },
    async handleKpiAddConfirm (payload) {
      const { id } = this.form
      if (payload.length) {
        if (id > 0) {
          await this.createKpisByKpiGroup({
            groupId: id,
            kpis: payload,
          })
          this.kpis = await this.getKpisByKpiGroup(id)
        } else {
          this.kpis = [
            ...payload,
            ...this.kpis,
          ]
        }
      }
    },
    async handleKpiUpdateConfirm (payload) {
      console.log('kpi modify confirm', payload)
      await this.updateKpi(payload)
      // this.$set(this.kpis, this.currentUpdateIndex, payload)
      this.kpis[this.currentUpdateIndex] = payload
      this.currentUpdateIndex = -1
    },
    async handleSelect (index) {
      this.currentIndex = index
      this.form = toPlainObject(this.filteredKpiGroupList[index])
      if (this.form.id >= 0) {
        this.kpis = await this.getKpisByKpiGroup(this.form.id)
      }
      this.$nextTick(() => {
        this.$refs.form.clearValidate()
      })
    },
    async handleClone (item) {
      const index = this.getIndexById(item.id)
      this.handleSelect(index)
      const form = toPlainObject(item)
      form.name = `${form.name}_clone`
      delete form.id
      this.currentIndex = -1
      this.form = form
    },
    async handleDelete (index, item) {
      const { id } = item
      try {
        await this.$confirmBox(this.$t('tips.whetherDeleteIndicatorGroupConfig'))
      } catch (e) {
        return
      }
      try {
        await this.destroyKpiGroup(id)
        if (index === this.currentIndex) {
          let currentIndex = -1
          if (index === 0) {
            if (this.filteredKpiGroupList.length) {
              currentIndex = 0
            }
          } else if (index > 0) {
            currentIndex = index - 1
          }
          if (currentIndex === -1) {
            this.currentIndex = -1
            this.form = null
            this.kpis = []
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
    async handleApplyKpiGroup (id) {
      await this.activeKpiGroup(id)
    },
    async handleSave () {
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      const form = toPlainObject({
        ...this.form,
        kpis: this.kpis,
      })
      console.log('save kpi group', form)
      try {
        if (this.currentIndex === -1) {
          await this.createKpiGroup(form)
          const index = this.getIndexByName(form.name)
          if (index === -1) {
            this.form = null
            this.kpis = []
          } else {
            this.currentIndex = index
            this.form = this.filteredKpiGroupList[index]
            this.kpis = await this.getKpisByKpiGroup(this.form.id)
          }
        } else {
          await this.updateKpiGroup(form)
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";
@import "@/assets/less/entity-mgr";

.entity-mgr-workshop-list-item-name {
  max-width: 140px !important;

  .entity-mgr-workshop-list-item-name-checked {
    display: inline-block;
    width: 20px;
  }

  .entity-mgr-workshop-list-item-name-text {
    margin-left: 3px;
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

.entity-mgr-form-item-list-body-list-item-right {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.entity-mgr-form-item-list-body-list-item-category {
  .text-overflow;
  display: inline-block;
  width: 100px;
  margin-left: 10px;
}

.el-icon-edit {
}
</style>
