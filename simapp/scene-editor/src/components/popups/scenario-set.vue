<template>
  <div v-loading="loading" class="entity-mgr">
    <section class="entity-mgr-workshop">
      <div class="entity-mgr-workshop-keyword">
        <el-input
          id="scenario-set-search-name"
          :model-value="keyword"
          prefix-icon="search"
          :placeholder="($t('operation.searchName'))"
          @input="updateState({ keyword: $event })"
        />
      </div>
      <ul class="entity-mgr-workshop-list">
        <li
          v-for="(item, index) in filteredScenarioSetList"
          :id="`scenario-set-node-${index}`"
          :key="item.id"
          class="entity-mgr-workshop-list-item"
          :class="{ 'entity-mgr-workshop-list-item-active': index === currentIndex }"
          @click="handleSelect(index)"
        >
          <span class="entity-mgr-workshop-list-item-name" :title="item.name">{{ item.name }}</span>
          <span class="entity-mgr-workshop-list-item-icon">
            <el-tooltip effect="dark" :content="$t('operation.copy')" placement="bottom">
              <el-icon class="icon el-icon-copy-document" @click.stop="handleClone(item)"><copy-document /></el-icon>
            </el-tooltip>
            <el-tooltip effect="dark" :content="$t('operation.delete')" placement="bottom">
              <el-icon class="icon el-icon-delete" @click.stop="handleDelete(index, item)"><delete /></el-icon>
            </el-tooltip>
          </span>
        </li>
      </ul>
      <div class="entity-mgr-workshop-operation">
        <el-tooltip effect="dark" :content="$t('operation.create')" placement="bottom">
          <el-icon id="scenario-set-create" class="icon el-icon-plus" @click="handleCreate">
            <plus />
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
          <el-input id="scenario-set-name" v-model.trim="form.name" name="name" />
        </el-form-item>
        <div class="entity-mgr-form-item-list">
          <div class="entity-mgr-form-item-list-header">
            <span>{{ $t('scenario.list') }}</span>
            <div>
              <el-tooltip effect="dark" :content="$t('operation.add')" placement="bottom">
                <el-icon id="scenario-add" class="icon el-icon-plus" @click="handleScenarioAdd">
                  <plus />
                </el-icon>
              </el-tooltip>
            </div>
          </div>
          <div class="entity-mgr-form-item-list-body">
            <ul v-if="scenarios.length" class="entity-mgr-form-item-list-body-list">
              <li
                v-for="(item, index) in scenarios"
                :key="`${item.id}-${index}`"
                class="entity-mgr-form-item-list-body-list-item"
              >
                <span
                  class="entity-mgr-form-item-list-body-list-item-name"
                  :title="item.filename"
                >{{ item.filename }}</span>
                <span class="entity-mgr-form-item-list-body-list-item-icon">
                  <el-icon class="icon el-icon-minus" @click.stop="handleScenarioRemove(item.id)"><minus /></el-icon>
                </span>
              </li>
            </ul>
            <div v-else class="entity-mgr-form-item-list-body-no-data">
              {{ $t('tips.noSceneClickToAdd', { b: '+' }) }}
            </div>
          </div>
          <div class="entity-mgr-form-item-list-footer">
            <el-button id="scenario-set-save" @click="handleSave">
              {{ $t('operation.save') }}
            </el-button>
          </div>
        </div>
      </el-form>

      <ScenarioAdd
        v-if="scenarioAddVisible"
        :selected="form.scenes"
        @close="scenarioAddVisible = false"
        @confirm="handleScenarioAddConfirm"
      />
    </section>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { errorHandler } from '@/common/errorHandler'
import { toPlainObject, validateScenarioSetName } from '@/common/utils'
import ScenarioAdd from '@/components/popups/scenario-add.vue'

const defaultForm = {
  name: '',
  scenes: [],
}

export default {
  name: 'ScenarioSet',
  components: {
    ScenarioAdd,
  },
  data () {
    const nameValidator = (rule, value, callback) => {
      if (!validateScenarioSetName(value)) {
        callback(new Error(this.$t('tips.namePrefixErr', { name: this.$t('scenario.setName') })))
        return
      }
      const index = this.getIndex(value)
      const matched = _.find(this.scenarioSetList, { name: value })
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
        scenes: [],
      },
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterName'), trigger: 'blur' },
          { validator: nameValidator, trigger: 'blur' },
        ],
      },
      scenarioAddVisible: false,
      currentIndex: -1,
      deletingScenario: false,
    }
  },
  computed: {
    ...mapState('scenario-set', [
      'loading',
      'keyword',
      'scenarioSetList',
    ]),
    ...mapGetters('scenario-set', [
      'filteredScenarioSetList',
    ]),
    ...mapState('scenario', [
      'scenarioList',
    ]),
    scenarios () {
      if (this.form) {
        const scenes = _.map(this.form.scenes, (id) => {
          return _.find(this.scenarioList, { id })
        })
        scenes.sort((a, b) => {
          return a.name.localeCompare(b.name)
        })
        return scenes
      } else {
        return []
      }
    },
  },
  watch: {
    currentIndex (value) {
      if (value !== -1) {
        this.$nextTick(() => {
          const el = document.getElementById(`scenario-set-node-${value}`)
          if (el) {
            el.scrollIntoView()
          }
        })
      }
    },
  },
  async created () {
    await this.getScenarioSetList()
    if (this.filteredScenarioSetList[0]) {
      this.handleSelect(0)
    }
  },
  methods: {
    ...mapActions('scenario', [
      'getScenarioList',
    ]),
    ...mapMutations('scenario-set', [
      'updateState',
    ]),
    ...mapActions('scenario-set', [
      'getScenarioSetList',
      'createScenarioSet',
      'updateScenarioSet',
      'destroyScenarioSet',
      'removeScenario',
      'addScenario',
    ]),
    getIndex (name) {
      return _.findIndex(this.filteredScenarioSetList, { name })
    },
    async handleCreate () {
      let isNotSaved = false
      if (!this.filteredScenarioSetList.length) {
        isNotSaved = !!this.scenarios.length
      } else if (this.currentIndex === -1) {
        isNotSaved = true
      } else {
        const currentSceneSet = this.filteredScenarioSetList[this.currentIndex]
        isNotSaved = !_.isEqual(currentSceneSet, this.form)
      }
      if (isNotSaved) {
        try {
          await this.$confirmBox(this.$t('tips.createScenarioWithoutSaved'))
        } catch (e) {
          return
        }
      }
      this.currentIndex = -1
      this.form = toPlainObject(defaultForm)
    },
    handleScenarioAdd () {
      this.scenarioAddVisible = true
    },
    async handleScenarioRemove (id) {
      if (this.deletingScenario) return
      this.deletingScenario = true
      this.form.scenes = this.form.scenes.filter(scene => scene !== id)

      try {
        await this.$refs.form.validate()
      } catch (e) {
        this.deletingScenario = false
        return false
      }
      const form = toPlainObject(this.form)
      form.scenes = form.scenes.join(',')
      try {
        await this.removeScenario(form)
      } catch (e) {
        await errorHandler(e)
      }
      this.deletingScenario = false
    },
    handleScenarioMoveUp (index, item) {
      if (index === 0) {
        return
      }
      const targetIndex = index - 1
      this.form.scenes.splice(index, 1)
      this.form.scenes.splice(targetIndex, 0, item)
    },
    handleScenarioMoveDown (index, item) {
      if (index === this.form.scenes.length - 1) {
        return
      }
      const targetIndex = index + 1
      this.form.scenes.splice(index, 1)
      this.form.scenes.splice(targetIndex, 0, item)
    },
    async handleScenarioAddConfirm (payload) {
      this.form.scenes = [
        ...payload,
        ...this.form.scenes,
      ]
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return false
      }
      const form = toPlainObject(this.form)
      form.scenes = form.scenes.join(',')
      try {
        await this.addScenario(form)
      } catch (e) {
        await errorHandler(e)
      }
    },
    async handleSelect (index, force = false) {
      if (this.currentIndex === index && !force) return
      this.currentIndex = index
      this.form = toPlainObject(this.filteredScenarioSetList[index])
      this.$nextTick(() => {
        this.$refs.form.clearValidate()
      })
    },
    async handleClone (item) {
      const form = toPlainObject(item)
      form.name = `${form.name}_clone`
      delete form.id
      this.currentIndex = -1
      this.form = form
    },
    async handleDelete (index, item) {
      const { id, scenes } = item
      try {
        await this.$confirmBox(this.$t('tips.whetherDeleteScenarioSet'))
      } catch (e) {
        return
      }
      try {
        await this.destroyScenarioSet(id)
        if (index === this.currentIndex) {
          let currentIndex = -1
          if (index === 0) {
            if (this.filteredScenarioSetList.length) {
              currentIndex = 0
            }
          } else if (index > 0) {
            currentIndex = index - 1
          }
          if (currentIndex === -1) {
            this.form = null
            await this.handleCreate()
          } else {
            this.handleSelect(currentIndex, true)
          }
        } else if (index < this.currentIndex) {
          this.currentIndex = this.currentIndex - 1
        }
        if (scenes.length) {
          await this.getScenarioList()
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
      const form = toPlainObject(this.form)
      form.scenes = form.scenes.join(',')
      try {
        if (this.currentIndex === -1) {
          await this.createScenarioSet(form)
          const index = this.getIndex(form.name)
          if (index === -1) {
            this.form = null
          } else {
            this.currentIndex = index
            this.form = this.filteredScenarioSetList[index]
          }
        } else {
          await this.updateScenarioSet(form)
        }
        if (form.scenes) {
          await this.getScenarioList()
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
</style>
