<template>
  <div id="finite-state-machine" class="finite-state-machine">
    <el-form
      ref="form"
      class="form"
      size="small"
      label-width="80px"
      :model="currentForm"
      :rules="rules"
      @submit.prevent
    >
      <el-form-item :label="$t('type')" prop="type">
        <el-select v-model="currentForm.type" class="select-type">
          <el-option
            v-for="(value, key) in optionTypeObject"
            :key="key"
            :label="key"
            :value="key"
          />
        </el-select>
      </el-form-item>
      <el-form-item :label="$t('name')" prop="name">
        <el-select v-model="currentForm.name" class="select-name">
          <el-option
            v-for="name in (optionTypeObject[currentForm.type] || [])"
            :key="name"
            :label="name"
            :value="name"
          />
        </el-select>
      </el-form-item>
      <el-form-item :label="$t('value')" prop="value">
        <InputNumber
          v-model="currentForm.value"
          :precision="2"
        />
      </el-form-item>
      <el-form-item :label="$t('scenario.triggerTime')" prop="triggerTime">
        <InputNumber
          v-model="currentForm.triggerTime"
          :precision="2"
        />
      </el-form-item>
      <el-form-item>
        <el-button @click="handleSubmit">
          {{ $t('operation.add') }}/{{ $t('operation.update') }}
        </el-button>
      </el-form-item>
    </el-form>

    <el-table
      ref="list"
      class="table"
      :data="list"
    >
      <el-table-column
        width="120"
        prop="type"
        :label="$t('type')"
      />
      <el-table-column
        prop="name"
        :label="$t('name')"
      />
      <el-table-column
        width="80"
        prop="value"
        :label="$t('value')"
      />
      <el-table-column
        :label="$t('scenario.triggerTime')"
        width="80"
      >
        <template #default="scope">
          <span>{{ scope.row.triggerTime }}s</span>
        </template>
      </el-table-column>
      <el-table-column
        width="130"
        :label="$t('operation.operation')"
      >
        <template #default="scope">
          <el-button link @click="handleUpdateItem(scope.$index, scope.row)">
            {{ $t('operation.modify') }}
          </el-button>
          <el-button link @click="handleDeleteItem(scope.$index)">
            {{ $t('operation.delete') }}
          </el-button>
        </template>
      </el-table-column>
    </el-table>

    <section class="button-group">
      <el-button class="dialog-cancel" @click="$emit('close')">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button class="dialog-ok" @click="handleOK">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { mapMutations } from 'vuex'
import { editor } from '@/api/interface'
import { toPlainObject } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'

export default {
  name: 'FiniteStateMachine',
  components: {
    InputNumber,
  },
  data () {
    return {
      currentForm: {
        type: '',
        name: '',
        value: 0,
        triggerTime: 0,
      },
      rules: {
        type: [
          { required: true, message: this.$t('tips.cannotBeEmpty') },
        ],
        name: [
          { required: true, message: this.$t('tips.cannotBeEmpty') },
        ],
        value: [
          { required: true, message: this.$t('tips.cannotBeEmpty') },
        ],
        triggerTime: [
          { required: true, message: this.$t('tips.cannotBeEmpty') },
        ],
      },
      list: [],
      optionTypeObject: {},
      updatingIndex: '',
    }
  },
  watch: {
    'currentForm.type': function (value) {
      this.currentForm.name = this.optionTypeObject[value][0]
    },
  },
  mounted () {
    this.optionTypeObject = editor.scenario.getL3StateMachineRawData() || {}
    this.list = editor.scenario.getL3StateMachineData() || []
  },
  methods: {
    ...mapMutations('scenario', {
      updateScenario: 'updateState',
    }),
    handleSubmit () {
      this.$refs.form.validate((valid) => {
        if (valid) {
          const form = toPlainObject(this.currentForm)
          form.triggerTime = +form.triggerTime
          if (/\d+/.test(`${this.updatingIndex}`)) {
            this.list[this.updatingIndex] = form
            // this.$set(this.list, this.updatingIndex, form)
            this.updatingIndex = ''
          } else {
            this.list.unshift(form)
            this.$nextTick(() => {
              const el = this.$refs.list.$el
              el.scrollTop = el.scrollHeight
            })
          }
        }
      })
    },
    handleUpdateItem (index, item) {
      const updatingItem = toPlainObject(item)
      this.updatingIndex = index
      this.currentForm.type = updatingItem.type
      this.$nextTick(() => {
        this.currentForm = updatingItem
      })
    },
    handleDeleteItem (index) {
      this.list.splice(index, 1)
    },
    handleOK () {
      const list = toPlainObject(this.list)
      editor.scenario.setL3StateMachineData(list)
      this.updateScenario({
        isDirty: true,
      })
      this.$emit('close')
    },
  },
}
</script>

<style scoped lang="less">
  .finite-state-machine {
    width: 660px;
    height: auto;
    padding: 23px;

    .form {
      display: flex;
      flex-wrap: wrap;

      :deep(.el-form-item) {
        width: 50%;

        .el-select {
          height: 30px;
        }

        .select-type {
          width: 60%;
        }

        .select-name {
          width: 100%;
        }
      }
    }

    .table {
      height: 240px;
      overflow-y: auto;

      :deep(.el-table td) {
        padding: 3px 0;
      }
    }

    .button-group {
      margin-top: 20px;
      text-align: right;
    }
  }
</style>
