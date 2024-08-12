<template>
  <div class="pedestrian-or-animal-trigger-condition-config" @click="handleDialogClick">
    <div class="car-trigger-config" @click="handleDialogClick">
      <div class="item">
        <div class="item-head">
          <h6 class="item-title">
            {{ $t('scenario.TimeTriggering') }}
          </h6>
          <el-button size="small" @click.stop="handleAddTriggerTimeItem">
            <el-icon class="el-icon-plus">
              <plus />
            </el-icon>
          </el-button>
        </div>
        <div class="item-body">
          <el-table
            ref="timeList"
            class="table"
            :data="trigger.timeList"
          >
            <el-table-column
              type="index"
              width="50"
            />
            <el-table-column
              :label="`${$t('scenario.TriggeringTimeValue')}(s)`"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.time.updating"
                  @click.stop
                >
                  <InputNumber
                    v-model="scope.row.time.value"
                    :precision="2"
                    :min="0"
                  />
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.time)"
                >
                  {{ scope.row.time.value }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              :label="`${$t('scenario.movementDirection')}(°)`"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.direction.updating"
                  @click.stop
                >
                  <InputNumber
                    v-model="scope.row.direction.value"
                    :precision="2"
                  />
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.direction)"
                >
                  {{ scope.row.direction.value }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              :label="`${$t('scenario.speed')}(m/s)`"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.velocity.updating"
                  @click.stop
                >
                  <InputNumber
                    v-model="scope.row.velocity.value"
                    :precision="2"
                    :min="0"
                    :max="selectedObject.maxVelocity"
                  />
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.velocity)"
                >
                  {{ velocity(scope.row.velocity.value) }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              width="80"
              :label="$t('operation.operation')"
            >
              <template #default="scope">
                <el-button link @click="handleDeleteTriggerTimeItem(scope.$index)">
                  {{ $t('operation.delete') }}
                </el-button>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </div>

      <div class="item">
        <div class="item-head">
          <h6 class="item-title">
            {{ $t('scenario.ConditionTriggering') }}
          </h6>
          <el-button size="small" @click.stop="handleAddTriggerConditionItem">
            <el-icon class="el-icon-plus">
              <plus />
            </el-icon>
          </el-button>
        </div>
        <div class="item-body">
          <el-table
            ref="conditionList"
            class="table"
            :data="trigger.conditionList"
          >
            <el-table-column
              type="index"
              width="50"
            />
            <el-table-column
              :label="$t('scenario.triggeringConditionType')"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.type.updating"
                  @click.stop
                >
                  <el-select
                    v-model="scope.row.type.value"
                    size="small"
                    @click.stop
                    @change="setConditionValue(scope.row)"
                  >
                    <el-option
                      v-for="item in conditionTypeOptionList"
                      :key="item.id"
                      :label="$t(item.name)"
                      :value="item.id"
                    />
                  </el-select>
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.type)"
                >
                  {{ conditionType(scope.row.type.value) }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              :label="$t('scenario.distanceType')"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.distancemode.updating"
                  @click.stop
                >
                  <el-select
                    v-model="scope.row.distancemode.value"
                    size="small"
                    @click.stop
                    @change="setConditionValue(scope.row)"
                  >
                    <el-option
                      v-for="item in distanceModeOptionList"
                      :key="item.id"
                      :label="$t(item.name)"
                      :value="item.id"
                    />
                  </el-select>
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.distancemode)"
                >
                  {{ distanceMode(scope.row.distancemode.value) }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              :label="$t('scenario.TriggeringConditionValue')"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.value.updating"
                  @click.stop
                >
                  <InputNumber
                    v-model="scope.row.value.value"
                    :precision="2"
                    :min="getMinValue(scope.row.type.value, scope.row.distancemode.value)"
                  />
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.value)"
                >
                  {{ scope.row.value.value }}
                  {{ conditionTypeUnitObject[scope.row.type.value] || '' }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              :label="$t('scenario.numberOfConditionsSatisfied')"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.times.updating"
                  @click.stop
                >
                  <InputNumber
                    v-model="scope.row.times.value"
                    :precision="0"
                    :min="1"
                  />
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.times)"
                >
                  {{ scope.row.times.value }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              :label="`${$t('scenario.movementDirection')}(°)`"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.direction.updating"
                  @click.stop
                >
                  <InputNumber
                    v-model="scope.row.direction.value"
                    :precision="2"
                  />
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.direction)"
                >
                  {{ scope.row.direction.value }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              :label="`${$t('scenario.speed')}(m/s)`"
            >
              <template #default="scope">
                <div
                  v-if="scope.row.velocity.updating"
                  @click.stop
                >
                  <InputNumber
                    v-model="scope.row.velocity.value"
                    :precision="2"
                    :min="0"
                    :max="selectedObject.maxVelocity"
                    @click.stop
                  />
                </div>
                <div
                  v-else
                  class="table-cell-value"
                  @click.stop="handleUpdating(scope.row.velocity)"
                >
                  {{ velocity(scope.row.velocity.value) }}
                </div>
              </template>
            </el-table-column>
            <el-table-column
              width="80"
              :label="$t('operation.operation')"
            >
              <template #default="scope">
                <el-button
                  link
                  @click="handleDeleteTriggerConditionItem(scope.$index)"
                >
                  {{ $t('operation.delete') }}
                </el-button>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </div>

      <section class="button-group">
        <el-button class="dialog-cancel" @click="$emit('close')">
          {{ $t('operation.cancel') }}
        </el-button>
        <el-button class="dialog-ok" @click="handleOK">
          {{ $t('operation.ok') }}
        </el-button>
      </section>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapGetters, mapMutations } from 'vuex'
import { editor } from '@/api/interface'
import { toPlainObject } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'
import dict from '@/common/dict'
import i18n from '@/locales'

const { triggerDirectionOptionList, conditionTypeOptionList, distanceModeOptionList } = dict

const conditionTypeUnitObject = {
  timeRelative: 's',
  distanceRelative: 'm',
}

export default {
  name: 'PedestrianOrAnimalTriggerConfig',
  components: {
    InputNumber,
  },
  data () {
    return {
      trigger: {
        timeList: [],
        conditionList: [],
      },
      template: {
        timeItem: {
          time: {
            value: 0,
            updating: true,
          },
          direction: {
            value: 0,
            updating: false,
          },
          velocity: {
            value: 2,
            updating: false,
          },
        },
        conditionItem: {
          type: {
            value: conditionTypeOptionList[0].id,
            updating: true,
          },
          distancemode: {
            value: distanceModeOptionList[1].id,
            updating: false,
          },
          value: {
            value: 0,
            updating: false,
          },
          times: {
            value: 1,
            updating: false,
          },
          direction: {
            value: 0,
            updating: false,
          },
          velocity: {
            value: 2,
            updating: false,
          },
        },
      },
      triggerDirectionOptionList,
      conditionTypeOptionList,
      conditionTypeUnitObject,
      distanceModeOptionList,
    }
  },
  computed: {
    ...mapGetters('scenario', [
      'selectedObject',
    ]),
  },
  created () {
    this.parseSelectedObjectTrigger()
  },
  methods: {
    ...mapMutations('scenario', [
      'updateObject',
    ]),
    parseToTemplateList (list) {
      return _.map(list, (item) => {
        const object = {}
        _.forOwn(item, (value, key) => {
          object[key] = {
            value,
            updating: false,
          }
        })
        return object
      })
    },
    parseToSelectedObjectTrigger (list) {
      return _.map(list, (item) => {
        const object = {}
        _.forOwn(item, (v, k) => {
          object[k] = v.value
        })
        return object
      })
    },
    parseSelectedObjectTrigger () {
      let trigger = this.selectedObject.trigger
      if (trigger) {
        const object = {}
        trigger = toPlainObject(trigger)
        _.forOwn(trigger, (value, key) => {
          object[key] = this.parseToTemplateList(value)
        })
        const { timeList, conditionList } = object
        if (timeList) {
          this.trigger.timeList = timeList
        }
        if (conditionList) {
          this.trigger.conditionList = conditionList
        }
      }
    },
    modifyObject (params) {
      const data = { ...this.selectedObject, ...params }
      const payload = { type: this.selectedObject.type, data }
      try {
        editor.object.update(payload)
        this.updateObject(payload)
      } catch (e) {
        console.error(e)
      }
    },
    setConditionValue (item) {
      const { type, distancemode, value } = item
      if (type.value === conditionTypeOptionList[1].id && distancemode.value === distanceModeOptionList[1].id) {
      } else {
        if (value.value < 0) {
          value.value = 0
        }
      }
    },
    getMinValue (type, distancemode) {
      if (type === conditionTypeOptionList[1].id && distancemode === distanceModeOptionList[1].id) {
        return undefined
      } else {
        return 0
      }
    },
    handleDialogClick () {
      _.forEach(this.trigger.timeList, (item) => {
        _.forOwn(item, (value) => {
          value.updating = false
        })
      })
      _.forEach(this.trigger.conditionList, (item) => {
        _.forOwn(item, (value) => {
          value.updating = false
        })
      })
    },
    handleUpdating (item) {
      this.handleDialogClick()
      item.updating = true
    },
    handleAddTriggerTimeItem () {
      const item = toPlainObject(this.template.timeItem)
      this.trigger.timeList.push(item)
      this.$nextTick(() => {
        const el = this.$refs.timeList.$el
        el.scrollTop = el.scrollHeight
      })
    },
    handleAddTriggerConditionItem () {
      const item = toPlainObject(this.template.conditionItem)
      this.trigger.conditionList.push(item)
      this.$nextTick(() => {
        const el = this.$refs.conditionList.$el
        el.scrollTop = el.scrollHeight
      })
    },
    handleDeleteTriggerTimeItem (index) {
      this.trigger.timeList.splice(index, 1)
    },
    handleDeleteTriggerConditionItem (index) {
      this.trigger.conditionList.splice(index, 1)
    },
    handleOK () {
      const trigger = {}
      _.forOwn(this.trigger, (value, key) => {
        trigger[key] = this.parseToSelectedObjectTrigger(value)
      })
      trigger.timeList = _.sortBy(trigger.timeList, ['time'])
      this.modifyObject({ trigger })
      this.$emit('close')
    },
    conditionType (value) {
      return i18n.t(conditionTypeOptionList.find(item => item.id === value).name || value)
    },
    distanceMode (value) {
      return i18n.t(distanceModeOptionList.find(item => item.id === value).name || value)
    },
    velocity (value) {
      if (value === '') {
        return '-'
      } else {
        return value
      }
    },
  },
}
</script>

<style scoped lang="less">
  .pedestrian-or-animal-trigger-condition-config {
    width: 800px;
    height: auto;
    padding: 23px;

    .item {
      margin-bottom: 23px;

      .item-head {
        display: flex;
        justify-content: space-between;
        margin-bottom: 10px;

        .item-title {
          margin-top: 3px;
          color: #c2c2c2;
          font-size: 12px;
          font-weight: normal;
        }
      }

      .item-body {
        margin-bottom: 10px;

        .table {
          height: 200px;
          overflow-y: auto;

          :deep(.el-select .el-input--mini .el-input__inner) {
            height: 24px;
            line-height: 24px;
          }

          .table-cell-value {
            width: 100%;
            height: 23px;
          }
        }
      }
    }

    .button-group {
      margin-top: 20px;
      text-align: right;
    }
  }
</style>
