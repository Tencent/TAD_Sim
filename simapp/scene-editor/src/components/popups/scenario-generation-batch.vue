<template>
  <div v-loading="loading" class="scenario-generation" @click="handleDialogClick">
    <el-form
      ref="form"
      size="small"
      :model="form"
      :rules="rules"
    >
      <el-form-item prop="prefix" :label="$t('scenario.namePrefix')">
        <el-input v-model="form.prefix" class="prefix" />
      </el-form-item>
      <el-form-item prop="file_type" :label="$t('scenario.GenFileFormat')">
        <el-select
          v-model="form.file_type"
        >
          <el-option v-if="currentScenario.type === 'sim'" label=".sim" value="sim" />
          <el-option label=".xosc" value="xosc" />
        </el-select>
      </el-form-item>
      <div class="scenario-generation-form">
        <div class="scenario-generation-nav">
          <el-collapse v-model="activeCollapseNames" class="scenario-generation-nav-collapse">
            <el-collapse-item
              name="ego"
              :title="$t('scenario.ego')"
              class="scenario-generation-nav-collapse-item"
            >
              <a
                v-for="ego in egoList"
                :key="-ego.id"
                href="javascript:;"
                class="scenario-generation-nav-collapse-item-name"
                :class="{ active: currentVehicleId === `${-ego.id}` }"
                @click="handleVehicleClick(`${-ego.id}`)"
              >Ego_{{ `${ego.id}`.padStart(3, '0') }}</a>
            </el-collapse-item>
            <el-collapse-item
              name="vehicle"
              :title="$t('scenario.TrafficVehicles')"
              class="scenario-generation-nav-collapse-item"
            >
              <a
                v-for="vehicle in vehicleList"
                :key="vehicle.id"
                href="javascript:;"
                class="scenario-generation-nav-collapse-item-name"
                :class="{ active: currentVehicleId === `${vehicle.id}` }"
                @click="handleVehicleClick(vehicle.id)"
              >
                car_{{ `${vehicle.id}`.padStart(3, '0') }}
              </a>
            </el-collapse-item>
          </el-collapse>
        </div>
        <div class="scenario-generation-main">
          <fieldset v-if="+currentVehicleId < 0 && currentEgoForm" class="filedset">
            <legend class="legend">
              {{ $t('scenario.ego') }}
            </legend>
            <el-form-item>
              <div class="form-item-wrap">
                <div class="form-item">
                  <span class="form-item-label">{{ $t('scenario.InitialSpeedRange') }}</span>
                  <InputNumber
                    v-model="currentEgoForm.velocity.start"
                    unit="m/s"
                    :precision="2"
                    :min="0"
                    :max="100"
                  />
                  <span class="form-item-input-connector">-</span>
                  <InputNumber
                    v-model="currentEgoForm.velocity.end"
                    unit="m/s"
                    :precision="2"
                    :min="0"
                    :max="100"
                  />
                  <span class="form-item-label">{{ $t('scenario.SpeedInterval') }}</span>
                  <InputNumber
                    v-model="currentEgoForm.velocity.sep"
                    unit="m/s"
                    :precision="2"
                    :min="0.01"
                    :max="abs(currentEgoForm.velocity.end - currentEgoForm.velocity.start)"
                  />
                </div>
              </div>
            </el-form-item>
          </fieldset>
          <fieldset v-if="+currentVehicleId > 0 && currentVehicleForm" class="filedset">
            <legend class="legend">
              {{ vehicleTitle }}
            </legend>
            <el-form-item>
              <div class="form-item-wrap" style="margin-bottom: 16px;">
                <div class="form-item">
                  <span class="form-item-label">{{ $t('scenario.InitialLateralOffsetRange') }}</span>
                  <InputNumber
                    v-model="currentVehicleForm.offset.start"
                    unit="m"
                    :precision="2"
                    :min="-maxOffset"
                    :max="maxOffset"
                  />
                  <span class="form-item-input-connector">-</span>
                  <InputNumber
                    v-model="currentVehicleForm.offset.end"
                    unit="m"
                    :precision="2"
                    :min="-maxOffset"
                    :max="maxOffset"
                  />
                  <span class="form-item-label">{{ $t('scenario.OffsetInterval') }}</span>
                  <InputNumber
                    v-model="currentVehicleForm.offset.sep"
                    unit="m"
                    :precision="2"
                    :min="0.01"
                    :max="abs(currentVehicleForm.offset.end - currentVehicleForm.offset.start)"
                  />
                </div>
                <div class="form-item last">
                  <span class="form-item-label">{{ $t('scenario.InitialLongOffsetRange') }}</span>
                  <InputNumber
                    v-model="currentVehicleForm.pos.start"
                    unit="m"
                    :precision="2"
                    :min="0"
                    :max="maxShift"
                  />
                  <span class="form-item-input-connector">-</span>
                  <InputNumber
                    v-model="currentVehicleForm.pos.end"
                    unit="m"
                    :precision="2"
                    :min="0"
                    :max="maxShift"
                  />
                  <span class="form-item-label">{{ $t('scenario.OffsetInterval') }}</span>
                  <InputNumber
                    v-model="currentVehicleForm.pos.sep"
                    unit="m"
                    :precision="2"
                    :min="0.01"
                    :max="abs(currentVehicleForm.pos.end - currentVehicleForm.pos.start)"
                  />
                </div>
              </div>
              <div class="form-item-wrap">
                <div class="form-item">
                  <span class="form-item-label">{{ $t('scenario.InitialSpeedRange') }}</span>
                  <InputNumber
                    v-model="currentVehicleForm.velocity.start"
                    unit="m/s"
                    :precision="2"
                    :min="0"
                    :max="100"
                  />
                  <span class="form-item-input-connector">-</span>
                  <InputNumber
                    v-model="currentVehicleForm.velocity.end"
                    unit="m/s"
                    :precision="2"
                    :min="0"
                    :max="100"
                  />
                  <span class="form-item-label">{{ $t('scenario.SpeedInterval') }}</span>
                  <InputNumber
                    v-model="currentVehicleForm.velocity.sep"
                    unit="m/s"
                    :precision="2"
                    :min="0.01"
                    :max="abs(currentVehicleForm.velocity.end - currentVehicleForm.velocity.start)"
                  />
                </div>
              </div>
              <div v-if="currentVehicle.behavior === 'UserDefine'" class="form-item-wrap trigger">
                <el-button size="small" class="btn-add-trigger-item" @click.stop="handleAddTriggerItem">
                  <el-icon class="el-icon-plus">
                    <plus />
                  </el-icon>
                </el-button>
                <el-tabs v-model="activeTriggerBy">
                  <el-tab-pane :label="$t('scenario.TimeTriggering')" name="byTime">
                    <el-table
                      ref="timeList"
                      class="table"
                      height="150"
                      :data="currentVehicleForm.trigger.timeList"
                    >
                      <el-table-column
                        type="index"
                        width="30"
                      />
                      <el-table-column
                        :label="`${$t('scenario.TriggeringTimeValue')}(s)`"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.time.updating"
                            @click.stop
                          >
                            <InputStartEndSeparator
                              :value="scope.row.time"
                              :min="0"
                            />
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.time)"
                          >
                            <ViewStartEndSeparator
                              :value="scope.row.time"
                            />
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
                            <InputStartEndSeparator
                              :value="scope.row.velocity"
                              :min="0"
                              :max="100"
                            />
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.velocity)"
                          >
                            <ViewStartEndSeparator
                              :value="scope.row.velocity"
                            />
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="`${$t('acceleration')}(m/s²)`"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.acceleration.updating"
                            @click.stop
                          >
                            <InputStartEndSeparator
                              :value="scope.row.acceleration"
                            />
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.acceleration)"
                          >
                            <ViewStartEndSeparator
                              :value="scope.row.acceleration"
                            />
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="$t('scenario.AccEndCondition')"
                      >
                        <template #default="scope">
                          <div class="acceleration-termination-type">
                            <div
                              v-if="scope.row.accelerationTerminationType.updating"
                              @click.stop
                            >
                              <el-select
                                v-model="scope.row.accelerationTerminationType.value"
                                size="small"
                                @change="setAccelerationTerminationValue(scope.row)"
                              >
                                <el-option
                                  v-for="item in accelerationTerminationTypeOption"
                                  :key="item.id"
                                  :label="item.name"
                                  :value="item.id"
                                />
                              </el-select>
                            </div>
                            <div
                              v-else
                              class="table-cell-value"
                              @click.stop="handleUpdating(scope.row.accelerationTerminationType)"
                            >
                              {{ accelerationTerminationType(scope.row.accelerationTerminationType.value) }}
                            </div>
                          </div>
                          <div class="acceleration-termination-value">
                            <div v-if="scope.row.accelerationTerminationType.value">
                              <div
                                v-if="scope.row.accelerationTerminationValue.updating"
                                @click.stop
                              >
                                <InputNumber
                                  v-model="scope.row.accelerationTerminationValue.value"
                                  :precision="2"
                                  :min="0"
                                />
                              </div>
                              <div
                                v-else
                                class="table-cell-value"
                                @click.stop="handleUpdating(scope.row.accelerationTerminationValue)"
                              >
                                {{ scope.row.accelerationTerminationValue.value }}
                                {{ getAccelerationTerminationType(scope.row) }}
                              </div>
                            </div>
                            <div v-else>
                              -
                            </div>
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="$t('scenario.posOffset')"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.direction.updating"
                            @click.stop
                          >
                            <el-select
                              v-model="scope.row.direction.value"
                              size="small"
                            >
                              <el-option
                                v-for="item in directionOptionList"
                                :key="item.id"
                                :label="$t(item.name)"
                                :value="item.id"
                              />
                            </el-select>
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.direction)"
                          >
                            {{ direction(scope.row.direction.value) }}
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="`${$t('scenario.OffsetDistance')}(m)`"
                      >
                        <template #default="scope">
                          <div
                            v-if="(
                              scope.row.direction.value === directionOptionList[3].id
                              || scope.row.direction.value === directionOptionList[4].id
                            )"
                          >
                            <div
                              v-if="scope.row.offsets.updating"
                              @click.stop
                            >
                              <InputStartEndSeparator
                                :value="scope.row.offsets"
                                :min="0"
                                :max="2.5"
                              />
                            </div>
                            <div
                              v-else
                              class="table-cell-value"
                              @click.stop="handleUpdating(scope.row.offsets)"
                            >
                              <ViewStartEndSeparator
                                :value="scope.row.offsets"
                              />
                            </div>
                          </div>
                          <div v-else>
                            -
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="`${$t('scenario.OffsetCompletionTime')}(s)`"
                      >
                        <template #default="scope">
                          <div v-if="scope.row.direction.value !== directionOptionList[0].id">
                            <div
                              v-if="scope.row.durations.updating"
                              @click.stop
                            >
                              <InputStartEndSeparator
                                :value="scope.row.durations"
                                :min="0.01"
                              />
                            </div>
                            <div
                              v-else
                              class="table-cell-value"
                              @click.stop="handleUpdating(scope.row.durations)"
                            >
                              <ViewStartEndSeparator
                                :value="scope.row.durations"
                              />
                            </div>
                          </div>
                          <div v-else>
                            -
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        width="20"
                      >
                        <template #default="scope">
                          <el-icon class="el-icon-delete" @click="handleDeleteTriggerTimeItem(scope.$index)">
                            <delete />
                          </el-icon>
                        </template>
                      </el-table-column>
                    </el-table>
                  </el-tab-pane>
                  <el-tab-pane :label="$t('scenario.ConditionTriggering')" name="byCondition">
                    <el-table
                      ref="conditionList"
                      class="table"
                      height="150"
                      style="width: 964px"
                      :data="currentVehicleForm.trigger.conditionList"
                    >
                      <el-table-column
                        type="index"
                        width="30"
                      />
                      <el-table-column
                        :label="$t('scenario.triggeringConditionType')"
                        width="120"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.type.updating"
                            @click.stop
                          >
                            <el-select
                              v-model="scope.row.type.value"
                              size="small"
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
                        width="120"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.distancemode.updating"
                            @click.stop
                          >
                            <el-select
                              v-model="scope.row.distancemode.value"
                              size="small"
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
                        width="150"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.value.updating"
                            @click.stop
                          >
                            <InputStartEndSeparator
                              :value="scope.row.value"
                              :min="getMinValue(scope.row.type.value, scope.row.distancemode.value)"
                            />
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.value)"
                          >
                            <ViewStartEndSeparator
                              :value="scope.row.value"
                            />
                            {{ conditionTypeUnitObject[scope.row.type.value] || '' }}
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="$t('scenario.numberOfConditionsSatisfied')"
                        width="120"
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
                              :allow-empty-value="true"
                            />
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.times)"
                          >
                            <span v-if="scope.row.times.value === ''">
                              -
                            </span>
                            <span v-else>
                              {{ scope.row.times.value }}
                            </span>
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="`${$t('scenario.speed')}(m/s)`"
                        width="150"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.velocity.updating"
                            @click.stop
                          >
                            <InputStartEndSeparator
                              :value="scope.row.velocity"
                              :min="0"
                              :max="100"
                            />
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.velocity)"
                          >
                            <ViewStartEndSeparator
                              :value="scope.row.velocity"
                            />
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="`${$t('acceleration')}(m/s²)`"
                        width="150"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.acceleration.updating"
                            @click.stop
                          >
                            <InputStartEndSeparator
                              :value="scope.row.acceleration"
                            />
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.acceleration)"
                          >
                            <ViewStartEndSeparator
                              :value="scope.row.acceleration"
                            />
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="$t('scenario.AccEndCondition')"
                        width="150"
                      >
                        <template #default="scope">
                          <div class="acceleration-termination-type">
                            <div
                              v-if="scope.row.accelerationTerminationType.updating"
                              @click.stop
                            >
                              <el-select
                                v-model="scope.row.accelerationTerminationType.value"
                                size="small"
                                @change="setAccelerationTerminationValue(scope.row)"
                              >
                                <el-option
                                  v-for="item in accelerationTerminationTypeOption"
                                  :key="item.id"
                                  :label="item.name"
                                  :value="item.id"
                                />
                              </el-select>
                            </div>
                            <div
                              v-else
                              class="table-cell-value"
                              @click.stop="handleUpdating(scope.row.accelerationTerminationType)"
                            >
                              {{ accelerationTerminationType(scope.row.accelerationTerminationType.value) }}
                            </div>
                          </div>
                          <div class="acceleration-termination-value">
                            <div v-if="scope.row.accelerationTerminationType.value">
                              <div
                                v-if="scope.row.accelerationTerminationValue.updating"
                                @click.stop
                              >
                                <InputNumber
                                  v-model="scope.row.accelerationTerminationValue.value"
                                  :precision="2"
                                  :min="0"
                                />
                              </div>
                              <div
                                v-else
                                class="table-cell-value"
                                @click.stop="handleUpdating(scope.row.accelerationTerminationValue)"
                              >
                                {{ scope.row.accelerationTerminationValue.value }}
                                {{ getAccelerationTerminationType(scope.row) }}
                              </div>
                            </div>
                            <div v-else>
                              -
                            </div>
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="$t('scenario.posOffset')"
                        width="120"
                      >
                        <template #default="scope">
                          <div
                            v-if="scope.row.direction.updating"
                            @click.stop
                          >
                            <el-select
                              v-model="scope.row.direction.value"
                              size="small"
                            >
                              <el-option
                                v-for="item in directionOptionList"
                                :key="item.id"
                                :label="$t(item.name)"
                                :value="item.id"
                              />
                            </el-select>
                          </div>
                          <div
                            v-else
                            class="table-cell-value"
                            @click.stop="handleUpdating(scope.row.direction)"
                          >
                            {{ direction(scope.row.direction.value) }}
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="`${$t('scenario.OffsetDistance')}(m)`"
                        width="150"
                      >
                        <template #default="scope">
                          <div
                            v-if="(
                              scope.row.direction.value === directionOptionList[3].id
                              || scope.row.direction.value === directionOptionList[4].id
                            )"
                          >
                            <div
                              v-if="scope.row.offsets.updating"
                              @click.stop
                            >
                              <InputStartEndSeparator
                                :value="scope.row.offsets"
                                :min="0"
                                :max="2.5"
                              />
                            </div>
                            <div
                              v-else
                              class="table-cell-value"
                              @click.stop="handleUpdating(scope.row.offsets)"
                            >
                              <ViewStartEndSeparator
                                :value="scope.row.offsets"
                              />
                            </div>
                          </div>
                          <div v-else>
                            -
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        :label="`${$t('scenario.OffsetCompletionTime')}(s)`"
                        width="150"
                      >
                        <template #default="scope">
                          <div v-if="scope.row.direction.value !== directionOptionList[0].id">
                            <div
                              v-if="scope.row.durations.updating"
                              @click.stop
                            >
                              <InputStartEndSeparator
                                :value="scope.row.durations"
                                :min="0.01"
                              />
                            </div>
                            <div
                              v-else
                              class="table-cell-value"
                              @click.stop="handleUpdating(scope.row.durations)"
                            >
                              <ViewStartEndSeparator
                                :value="scope.row.durations"
                              />
                            </div>
                          </div>
                          <div v-else>
                            -
                          </div>
                        </template>
                      </el-table-column>
                      <el-table-column
                        width="20"
                      >
                        <template #default="scope">
                          <el-icon class="el-icon-delete" @click="handleDeleteTriggerConditionItem(scope.$index)">
                            <delete />
                          </el-icon>
                        </template>
                      </el-table-column>
                    </el-table>
                  </el-tab-pane>
                </el-tabs>
              </div>
            </el-form-item>
          </fieldset>
        </div>
      </div>
    </el-form>

    <section class="button-group">
      <el-button class="dialog-cancel" @click="$emit('close')">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button class="dialog-ok" :disabled="progressInfo.status" @click="handleOK">
        {{ $t('operation.ok') }}
      </el-button>
    </section>

    <ScenarioGenerationConfirm
      v-if="scenarioGenerationConfirm.visible"
      :value="scenarioGenerationConfirm.data"
      @close="scenarioGenerationConfirm = { visible: false, data: null }"
      @confirm="handleScenarioGenerationConfirmConfirm"
    />
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import { ElMessage } from 'element-plus'
import InputStartEndSeparator from './start-end-separator/input.vue'
import ViewStartEndSeparator from './start-end-separator/view.vue'
import { editor } from '@/api/interface'
import { toPlainObject, validateScenarioName } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'
import ScenarioGenerationConfirm from '@/components/popups/scenario-generation-confirm.vue'
import dict from '@/common/dict'
import i18n from '@/locales'

const { triggerDirectionOptionList: directionOptionList, conditionTypeOptionList, distanceModeOptionList } = dict

const accelerationTerminationTypeOption = [
  { id: 'time', name: 'Time' },
  { id: 'velocity', name: 'Velocity' },
]

const conditionTypeUnitObject = {
  timeRelative: 's',
  distanceRelative: 'm',
}

const accelerationTerminationTypeUnit = {
  time: 's',
  velocity: 'm/s',
}

export default {
  name: 'ScenarioGeneration',
  components: {
    InputNumber,
    InputStartEndSeparator,
    ViewStartEndSeparator,
    ScenarioGenerationConfirm,
  },
  data () {
    const validateName = async (rule, value, callback) => {
      if (validateScenarioName(value)) {
        callback()
      } else {
        callback(new Error(this.$t('tips.namePrefixErr', { name: this.$t('scenario.name') })))
      }
    }
    return {
      loading: false,
      form: {
        prefix: '',
        file_type: '',
        ego: {},
        vehicle: {},
      },
      rules: {
        prefix: [
          { required: true, message: this.$t('tips.scenarioPrefixCantEmpty'), trigger: 'blur' },
          { validator: validateName, trigger: 'blur' },
        ],
        file_type: [
          { required: true, message: this.$t('tips.fileFormatCantEmpty'), trigger: 'blur' },
        ],
      },
      activeCollapseNames: ['ego', 'vehicle'],
      egoList: [],
      vehicleList: [],
      // 右边公用一个界面，故 ego < 0 | vehicle > 0
      currentVehicleId: '',
      currentVehicleForm: null,
      currentEgoForm: null,
      activeTriggerBy: 'byTime',
      directionOptionList,
      conditionTypeOptionList,
      conditionTypeUnitObject,
      accelerationTerminationTypeOption,
      accelerationTerminationTypeUnit,
      distanceModeOptionList,
      template: {
        vehicleItem: {
          offset: {
            start: 0,
            end: 0,
            sep: 0.01,
          },
          pos: {
            start: 0,
            end: 0,
            sep: 0.01,
          },
          velocity: {
            start: 0,
            end: 10,
            sep: 1,
          },
          trigger: {
            timeList: [],
            conditionList: [],
          },
        },
        timeItem: {
          time: {
            start: '',
            end: '',
            sep: '',
            updating: true,
          },
          velocity: {
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
          acceleration: {
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
          accelerationTerminationType: {
            value: 'time',
            updating: false,
          },
          accelerationTerminationValue: {
            value: 3,
            updating: false,
          },
          direction: {
            value: directionOptionList[0].id,
            updating: false,
          },
          offsets: {
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
          durations: {
            start: '',
            end: '',
            sep: '',
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
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
          times: {
            value: '',
            updating: false,
          },
          velocity: {
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
          acceleration: {
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
          accelerationTerminationType: {
            value: 'time',
            updating: false,
          },
          accelerationTerminationValue: {
            value: 3,
            updating: false,
          },
          direction: {
            value: directionOptionList[0].id,
            updating: false,
          },
          offsets: {
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
          durations: {
            start: '',
            end: '',
            sep: '',
            updating: false,
          },
        },
      },
      scenarioGenerationConfirm: {
        visible: false,
        data: null,
      },
    }
  },
  computed: {
    ...mapState('scenario', [
      'currentScenario',
    ]),
    ...mapState('progressInfo', [
      'progressInfo',
    ]),
    maxOffset () {
      return editor.scenario.getObjectMaxOffset('car', this.currentVehicleId)
    },
    maxShift () {
      return editor.scenario.getVehicleMaxShift(this.currentVehicleId)
    },
    currentVehicle () {
      return _.find(this.vehicleList, { id: this.currentVehicleId }) || {}
    },
    vehicleTitle () {
      const { behavior } = this.currentVehicle
      let title = `${this.$t('scenario.TrafficVehicleTypes')}: `
      if (behavior === 'TrafficVehicle') {
        title += this.$t('scenario.aiControl')
      } else if (behavior === 'UserDefine') {
        title += this.$t('operation.custom')
      }
      return title
    },
  },
  watch: {
    currentScenario: {
      handler (value) {
        const { name, type } = value
        this.form.prefix = name
        this.form.file_type = type
      },
      immediate: true,
    },
  },
  mounted () {
    this.egoList = editor.scenario.getObjectsByType('planner')
    this.vehicleList = editor.scenario.getObjectsByType('car')
    this.setFormScenario()
    if (this.egoList.length) {
      this.handleVehicleClick(`${-this.egoList[0].id}`)
    }
  },
  methods: {
    ...mapActions('scenario', [
      'getScenarioList',
      'paramScene',
      'paramSceneNew',
    ]),
    ...mapActions('scenario-set', [
      'getScenarioSetList',
    ]),
    getAccelerationTerminationType (row) {
      return accelerationTerminationTypeUnit[row.accelerationTerminationType.value] || ''
    },
    abs (x) {
      return Math.abs(x)
    },
    setFormScenario () {
      const ego = {}
      _.forEach(this.egoList, ({ id }) => {
        ego[id] = {
          velocity: {
            start: 0,
            end: 10,
            sep: 1,
          },
        }
      })
      this.form.ego = ego
      const vehicle = {}
      _.forEach(this.vehicleList, (value) => {
        const { id, startOffset, startShift: pos, startVelocity: velocity } = value
        const item = toPlainObject(this.template.vehicleItem)
        item.offset.start = +startOffset
        item.offset.end = +startOffset
        item.pos.start = +pos
        item.pos.end = +pos
        item.velocity.start = +velocity
        item.velocity.end = +velocity
        vehicle[id] = item
      })
      this.form.vehicle = vehicle
    },
    parseTriggerList (list) {
      const keys = ['value', 'start', 'end', 'sep']
      _.forEach(list, (item) => {
        if (
          item.direction.value === directionOptionList[0].id ||
          item.direction.value === directionOptionList[1].id ||
          item.direction.value === directionOptionList[2].id
        ) {
          item.offsets.start = ''
          item.offsets.end = ''
          item.offsets.sep = ''
        }
        if (item.direction.value === directionOptionList[0].id) {
          item.durations.start = ''
          item.durations.end = ''
          item.durations.sep = ''
        }
        _.forOwn(item, (value) => {
          delete value.updating
          _.forEach(keys, (key) => {
            if (value[key] !== undefined && !_.isString(value[key])) {
              value[key] = `${value[key]}`
            }
          })
        })
      })
    },
    setConditionValue (item) {
      const { type, distancemode, value } = item
      if (type.value === conditionTypeOptionList[1].id && distancemode.value === distanceModeOptionList[1].id) {
        // pass
      } else {
        if (value.start < 0) {
          value.start = 0
        }
        if (value.end < 0) {
          value.end = 0
        }
      }
    },
    setAccelerationTerminationValue (item) {
      const { accelerationTerminationValue } = item
      accelerationTerminationValue.value = 3
    },
    getMinValue (type, distancemode) {
      if (type === conditionTypeOptionList[1].id && distancemode === distanceModeOptionList[1].id) {
        return undefined
      } else {
        return 0
      }
    },
    handleVehicleClick (id) {
      this.currentVehicleId = id
      if (+id < 0) {
        this.currentEgoForm = this.form.ego[-id]
        this.currentVehicleForm = null
      } else {
        this.currentVehicleForm = this.form.vehicle[id]
        this.currentEgoForm = null
      }
    },
    handleDialogClick () {
      if (this.currentVehicleForm) {
        _.forEach(this.currentVehicleForm.trigger.timeList, (item) => {
          _.forOwn(item, (value) => {
            value.updating = false
          })
        })
        _.forEach(this.currentVehicleForm.trigger.conditionList, (item) => {
          _.forOwn(item, (value) => {
            value.updating = false
          })
        })
      }
    },
    handleAddTriggerItem () {
      if (this.activeTriggerBy === 'byTime') {
        this.handleAddTriggerTimeItem()
      } else if (this.activeTriggerBy === 'byCondition') {
        this.handleAddTriggerConditionItem()
      }
    },
    handleAddTriggerTimeItem () {
      const item = toPlainObject(this.template.timeItem)
      this.currentVehicleForm.trigger.timeList.push(item)
      this.$nextTick(() => {
        const el = this.$refs.timeList.$el
        el.scrollTop = el.scrollHeight
      })
    },
    handleAddTriggerConditionItem () {
      const item = toPlainObject(this.template.conditionItem)
      this.currentVehicleForm.trigger.conditionList.push(item)
      this.$nextTick(() => {
        const el = this.$refs.conditionList.$el
        el.scrollTop = el.scrollHeight
      })
    },
    handleUpdating (item) {
      this.handleDialogClick()
      item.updating = true
    },
    handleDeleteTriggerTimeItem (index) {
      this.currentVehicleForm.trigger.timeList.splice(index, 1)
    },
    handleDeleteTriggerConditionItem (index) {
      this.currentVehicleForm.trigger.conditionList.splice(index, 1)
    },
    handleOK () {
      if (this.progressInfo.status) {
        ElMessage.warning(this.$t('tips.sceneIsGenerating'))
        return
      }
      this.$refs.form.validate(async (valid) => {
        if (valid) {
          const errors = []
          _.forOwn(this.form.ego, (value, key) => {
            if (value.velocity.start > value.velocity.end) {
              errors.push(this.$t('tips.egoSpeedCantExceedMaximum'), { key })
            }
          })
          _.forOwn(this.form.vehicle, (value, key) => {
            if (value.offset.start > value.offset.end) {
              errors.push(this.$t('tips.lateralOffsetCantExceedMaximum', { key }))
            }
            if (value.pos.start > value.pos.end) {
              errors.push(this.$t('tips.longitudinalOffsetCantExceedMaximum', { key }))
            }
            if (value.velocity.start > value.velocity.end) {
              errors.push(this.$t('tips.speedCantExceedMaximum', { key }))
            }
          })
          if (errors.length) {
            await this.$message.error({
              message: errors.join('<br>'),
              dangerouslyUseHTMLString: true,
            })
            return
          }
          const form = toPlainObject(this.form)
          _.forOwn(form.vehicle, (value) => {
            this.parseTriggerList(value.trigger.timeList)
            this.parseTriggerList(value.trigger.conditionList)
          })
          this.scenarioGenerationConfirm.data = form
          this.scenarioGenerationConfirm.visible = true
        }
      })
    },
    async handleScenarioGenerationConfirmConfirm (payload) {
      const { addScenarioSetAuto, time } = payload
      const params = {
        ...this.scenarioGenerationConfirm.data,
        id: this.currentScenario.id,
        export_flag: +addScenarioSetAuto,
        time,
      }
      this.scenarioGenerationConfirm = { visible: false, data: null }
      try {
        await this.paramSceneNew(params)
        await this.getScenarioList()
        await this.getScenarioSetList()
        this.$emit('close')
      } catch (e) {
        if (e.toString() === 'cancel' || e.err === -2) { // -2: 操作中断
          return
        }
        this.$errorBox(this.$t('tips.generationOfScenariosFailed', { error: e }))
      }
    },
    direction (value) {
      return i18n.t(directionOptionList.find(item => item.id === value).name || value)
    },
    conditionType (value) {
      return i18n.t(conditionTypeOptionList.find(item => item.id === value).name || value)
    },
    distanceMode (value) {
      return i18n.t(distanceModeOptionList.find(item => item.id === value).name || value)
    },
    accelerationTerminationType (value) {
      const matched = accelerationTerminationTypeOption.find(item => item.id === value)
      if (matched) {
        return matched.name
      } else {
        return '-'
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.scenario-generation {
  width: 1120px;
  height: 500px;
  padding: 23px;

  :deep(.el-input.prefix) {
    width: 40%;
  }

  :deep(.el-select .el-input) {
    width: 100px;
  }

  :deep(.el-form-item) {
    margin-bottom: 15px;
    min-height: 28px;
  }

  :deep(.input-number) {
    width: 68px;
    vertical-align: baseline;
  }

  :deep(.el-tabs__header) {
    margin-bottom: 0;
  }

  :deep(.el-tabs__nav) {
    .el-tabs__item {
      background-color: @hover-bg;

      &.is-active {
        background-color: #0f0f0f;
      }
    }
  }

  .scenario-generation-form {
    height: 368px;
    display: flex;
    justify-content: space-between;

    .scenario-generation-nav {
      flex-shrink: 0;
      width: 120px;
      margin-top: 8px;
      margin-right: 8px;
      border-right: 1px solid #404040;
      overflow-y: auto;

      .scenario-generation-nav-collapse {
        .scenario-generation-nav-collapse-item {
          :deep(.el-collapse-item__header) {
            padding-left: 4px;
          }

          :deep(.el-collapse-item__arrow) {
            margin-right: 0;
          }

          .scenario-generation-nav-collapse-item-name {
            display: block;
            padding: 1px 20px;
            text-decoration: none;
            color: @title-font-color;
            border-bottom: 1px solid @list-bd;

            &.active,
            &:hover {
              color: @active-font-color;
              background-color: @hover-bg;
            }
          }
        }
      }
    }

    .scenario-generation-main {
      flex: 1;
      overflow: auto;
    }
  }

  .filedset {
    margin-bottom: 5px;
    padding-top: 5px;
    border: 1px solid #404040;

    .legend {
      margin-left: 10px;
      color: #c2c2c2;
    }

    .el-form-item-select :deep(.el-form-item__label) {
      color: #606266;
    }

    .form-item-input-connector {
      margin: 0 6px;
    }

    .form-item-wrap {
      margin: 5px 0;

      &.trigger {
        position: relative;
        margin-top: 10px;
        width: 100%;

        .btn-add-trigger-item {
          position: absolute;
          right: 0;
          top: 0;
          cursor: pointer;
          z-index: 1;
        }

        .table {
          width: 100%;

          :deep(.input-number) {
            width: 38px;
            vertical-align: baseline;

            .el-input--mini .el-input__inner {
              height: 24px;
              line-height: 24px;
            }
          }

          :deep(.el-input--mini .el-input__inner) {
            height: 24px;
            line-height: 24px;
          }

          .table-cell-value {
            height: 24px;
            line-height: 24px;
          }

          .acceleration-termination-type,
          .acceleration-termination-value {
            display: inline-block;
          }

          .acceleration-termination-type {
            margin-right: 10px;

            :deep(.el-select),
            :deep(.el-select .el-input) {
              width: 66px;
            }
          }
        }
      }

      .form-item {
        display: inline-block;

        &.last {
          margin-left: 20px;
        }

        .form-item-label {
          display: inline-block;
          margin: 0 15px;
        }

        .el-form-item {
          display: inline-block;
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
