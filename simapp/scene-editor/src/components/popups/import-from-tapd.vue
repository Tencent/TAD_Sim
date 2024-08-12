<template>
  <div v-loading="loading" class="import-from-tapd">
    <el-form ref="form" :model="form" :rules="rules" @submit.prevent>
      <el-form-item :label="$t('scenario.testPlanName')" prop="testPlanId">
        <el-select
          v-model="form.testPlanId"
          @change="handleTestPlanIdChange"
        >
          <el-option
            v-for="testPlan in optionTestPlanList"
            :key="testPlan.id"
            :label="testPlan.name"
            :value="testPlan.id"
          />
        </el-select>
      </el-form-item>
    </el-form>

    <section class="button-group">
      <el-button id="import-from-tapd-cancel" class="dialog-cancel" @click="$emit('close')">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button id="import-from-tapd-confirm" class="dialog-ok" @click="handleConfirm">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import { errorHandler } from '@/common/errorHandler'

export default {
  name: 'ImportFromTapd',
  data () {
    return {
      form: {
        testPlanId: '',
      },
      rules: {
        testPlanId: [
          { required: true, message: this.$t('tips.enterTestPlaneName') },
        ],
      },
    }
  },
  computed: {
    ...mapState('scenario', [
      'scenarioList',
    ]),
    ...mapState('scenario-set', [
      'scenarioSetList',
    ]),
    ...mapState('tapd', [
      'loading',
      'testPlanList',
      'tcaseNames',
    ]),
    optionTestPlanList () {
      return [
        { id: '', name: this.$t('tips.pleaseSelect') },
        ...this.testPlanList,
      ]
    },
    testPlanObject () {
      const object = {}
      _.forEach(this.testPlanList, (item) => {
        object[item.id] = item.name
      })
      return object
    },
  },
  async created () {
    await this.getScenarioList()
    await this.getScenarioSetList()
    await this.getTestPlanList()
  },
  methods: {
    ...mapActions('scenario', [
      'getScenarioList',
    ]),
    ...mapActions('scenario-set', [
      'getScenarioSetList',
      'createScenarioSet',
    ]),
    ...mapActions('tapd', [
      'getTestPlanList',
      'getTcaseNames',
    ]),
    getScenarioSetName (name) {
      const names = this.scenarioSetList.map((item) => {
        return item.name
      })
      if (names.includes(name)) {
        let index = 1
        while (true) {
          const newName = `${name}(${index})`
          if (names.includes(newName)) {
            index++
          } else {
            return newName
          }
        }
      } else {
        return name
      }
    },
    async handleTestPlanIdChange (value) {
      await this.getTcaseNames(value)
    },
    async handleConfirm () {
      if (this.form.testPlanId) {
        try {
          const existingIds = []
          const nonExistingNames = []
          _.forEach(this.tcaseNames, (name) => {
            const matched = _.find(this.scenarioList, { name })
            if (matched) {
              existingIds.push(matched.id)
            } else {
              nonExistingNames.push(name)
            }
          })
          if (nonExistingNames.length) {
            await this.$message.error(this.$t('tips.sceneNotExist', { scene: nonExistingNames.join(',') }))
          }
          if (existingIds.length) {
            const name = this.getScenarioSetName(this.testPlanObject[this.form.testPlanId])
            await this.createScenarioSet({
              name,
              scenes: existingIds.join(','),
            })
            this.$emit('close')
          }
        } catch (e) {
          await errorHandler(e)
        }
      }
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .import-from-tapd {
    width: 360px;
    padding: 23px;

    :deep(.el-select) {
      width: 266px;
    }

    .button-group {
      margin-top: 20px;
      text-align: right;
    }
  }
</style>
