<template>
  <div class="car-trigger-config">
    <div class="car-trigger-config-body" style="height: 500px;width: 100%;display: flex;">
      <div class="left-section">
        <div class="trigger-list">
          <div
            v-for="(item, index) in newTriggerList"
            :key="`trigger-${index}`"
            class="trigger-item"
            :class="{ 'is-active': index === currentTriggerInfoIndex }"
            @click="handleChangeTrigger(index)"
          >
            <span :title="item.name">{{ item.name }}</span>
            <el-tooltip :content="$t('operation.delete')" placement="right">
              <remove-svg @click.stop="handleDeleteTrigger(index, item.id)" />
            </el-tooltip>
          </div>
        </div>
        <div class="add-new-trigger" @click="handleAddTrigger">
          <el-icon class="el-icon-plus">
            <plus />
          </el-icon>
          <span>{{ $t('operation.addTrigger') }}</span>
        </div>
      </div>
      <div class="right-section">
        <el-form
          v-if="newTriggerList.length > 0 && currentTriggerInfo"
          ref="form"
          :model="currentTriggerInfo"
          :rules="rules"
        >
          <div class="item-1">
            <el-form-item
              :label="$t('scenario.triggerName')"
              prop="name"
              maxlength="40"
              style="margin-top: 15px"
            >
              <el-input v-model="currentTriggerInfo.name" />
            </el-form-item>
            <el-collapse v-model="activeCollapse">
              <el-collapse-item :title="$t('scenario.triggerCondition')" name="triggerCondition">
                <div class="item-1-body">
                  <!--    条件类型    -->
                  <el-form-item :label="$t('scenario.triggeringConditionType')" prop="type">
                    <el-select v-model="formType" :disabled="isTrafficLight" @change="handleChangeType">
                      <el-option
                        v-for="condition in triggerConditionTypeList"
                        :key="condition.value"
                        :label="$t(condition.label)"
                        :value="condition.value"
                      />
                    </el-select>
                  </el-form-item>
                  <!-- 仿真时间触发条件表格 -->
                  <template v-if="formType === 'time_trigger'">
                    <!--    仿真时间值    -->
                    <el-form-item
                      :label="$t('scenario.simulationTimeValue')"
                      prop="triggertime"
                    >
                      <InputNumber
                        v-model="currentTriggerInfo.triggertime"
                        :min="0"
                        unit="s"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- TTC触发条件表格 -->
                  <template v-else-if="formType === 'ttc_trigger'">
                    <!--    TTC值    -->
                    <el-form-item
                      :label="$t('scenario.TTCValue')"
                      prop="threshold"
                    >
                      <el-select v-model="currentTriggerInfo.equationop" class="operator-selector">
                        <el-option
                          v-for="operator in comparisonOperatorList"
                          :key="operator.value"
                          :label="operator.label"
                          :value="operator.value"
                        />
                      </el-select>
                      <InputNumber
                        v-model="currentTriggerInfo.threshold"
                        :precision="2"
                        :min="0"
                        unit="s"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                    <!--    距离类型    -->
                    <el-form-item
                      :label="$t('scenario.distanceType')"
                      prop="distype"
                    >
                      <el-select v-model="currentTriggerInfo.distype">
                        <el-option
                          v-for="distanceMode in distanceModeOptionList"
                          :key="distanceMode.id"
                          :label="$t(distanceMode.name)"
                          :value="distanceMode.id"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    对象    -->
                    <el-form-item
                      :label="$t('object')"
                      prop="targetelement"
                    >
                      <el-select v-model="currentTriggerInfo.targetelement">
                        <el-option
                          v-for="distanceMode in newObjectList1"
                          :key="distanceMode.value"
                          :label="distanceMode.label"
                          :value="distanceMode.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件边界    -->
                    <el-form-item
                      :label="$t('scenario.conditionalBoundary')"
                      prop="boundary"
                    >
                      <el-select v-model="currentTriggerInfo.boundary">
                        <el-option
                          v-for="conditionBoundary in conditionBoundaryList"
                          :key="conditionBoundary.value"
                          :label="$t(conditionBoundary.label)"
                          :value="conditionBoundary.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件满足次数    -->
                    <el-form-item
                      :label="$t('scenario.numberOfConditionsSatisfied')"
                      prop="count"
                    >
                      <InputNumber
                        v-model="currentTriggerInfo.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- 距离触发条件表格 -->
                  <template v-else-if="formType === 'distance_trigger'">
                    <!--    距离值    -->
                    <el-form-item
                      :label="$t('scenario.distanceValue')"
                      prop="threshold"
                    >
                      <el-select v-model="currentTriggerInfo.equationop" class="operator-selector">
                        <el-option
                          v-for="operator in comparisonOperatorList"
                          :key="operator.value"
                          :label="operator.label"
                          :value="operator.value"
                        />
                      </el-select>
                      <InputNumber
                        v-model="currentTriggerInfo.threshold"
                        :allow-empty-value="true"
                        :precision="2"
                        :min="0"
                        :max="10000"
                        unit="m"
                      />
                    </el-form-item>
                    <!--    距离类型    -->
                    <el-form-item
                      :label="$t('scenario.distanceType')"
                      prop="distype"
                    >
                      <el-select v-model="currentTriggerInfo.distype">
                        <el-option
                          v-for="distanceMode in distanceModeOptionList"
                          :key="distanceMode.id"
                          :label="$t(distanceMode.name)"
                          :value="distanceMode.id"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    对象    -->
                    <el-form-item
                      :label="$t('object')"
                      prop="targetelement"
                    >
                      <el-select v-model="currentTriggerInfo.targetelement">
                        <el-option
                          v-for="distanceMode in newObjectList1"
                          :key="distanceMode.value"
                          :label="distanceMode.label"
                          :value="distanceMode.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件边界    -->
                    <el-form-item
                      :label="$t('scenario.conditionalBoundary')"
                      prop="boundary"
                    >
                      <el-select v-model="currentTriggerInfo.boundary">
                        <el-option
                          v-for="conditionBoundary in conditionBoundaryList"
                          :key="conditionBoundary.value"
                          :label="$t(conditionBoundary.label)"
                          :value="conditionBoundary.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件满足次数    -->
                    <el-form-item
                      :label="$t('scenario.numberOfConditionsSatisfied')"
                      prop="count"
                    >
                      <InputNumber
                        v-model="currentTriggerInfo.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- 速度触发条件表格 -->
                  <template v-else-if="formType === 'velocity_trigger'">
                    <!--    速度值    -->
                    <el-form-item prop="threshold">
                      <template #label>
                        {{ $t('scenario.speedValue') }}
                        <el-tooltip
                          style="margin-left: 5px"
                          effect="dark"
                          :content="$t('tips.relativeSpeedCalculationMethod')"
                          placement="top"
                        >
                          <el-icon class="el-icon-info">
                            <warning />
                          </el-icon>
                        </el-tooltip>
                      </template>
                      <el-select v-model="currentTriggerInfo.equationop" class="operator-selector">
                        <el-option
                          v-for="operator in comparisonOperatorList"
                          :key="operator.value"
                          :label="operator.label"
                          :value="operator.value"
                        />
                      </el-select>
                      <InputNumber
                        v-model="currentTriggerInfo.threshold"
                        :allow-empty-value="true"
                        :precision="2"
                        :min="-100"
                        :max="100"
                        :unit="currentTriggerInfo.unit === 'm_s' ? 'm/s' : ''"
                      />
                    </el-form-item>
                    <!--    速度类型    -->
                    <el-form-item
                      :label="$t('scenario.speedType')"
                      prop="speedtype"
                    >
                      <el-select v-model="currentTriggerInfo.speedtype">
                        <el-option
                          v-for="speedType in speedTypeList"
                          :key="speedType.value"
                          :label="$t(speedType.label)"
                          :value="speedType.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    对象    -->
                    <el-form-item
                      :label="$t('object')"
                      prop="targetelement"
                    >
                      <el-select v-model="currentTriggerInfo.targetelement">
                        <el-option
                          v-for="distanceMode in newObjectList0"
                          :key="distanceMode.value"
                          :label="distanceMode.label"
                          :value="distanceMode.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    距离类型    -->
                    <el-form-item
                      :label="$t('scenario.distanceType')"
                      prop="distype"
                    >
                      <el-select v-model="currentTriggerInfo.distype">
                        <el-option
                          v-for="distanceMode in distanceModeOptionList"
                          :key="distanceMode.id"
                          :label="$t(distanceMode.name)"
                          :value="distanceMode.id"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件边界    -->
                    <el-form-item
                      :label="$t('scenario.conditionalBoundary')"
                      prop="boundary"
                    >
                      <el-select v-model="currentTriggerInfo.boundary">
                        <el-option
                          v-for="conditionBoundary in conditionBoundaryList"
                          :key="conditionBoundary.value"
                          :label="$t(conditionBoundary.label)"
                          :value="conditionBoundary.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件满足次数    -->
                    <el-form-item
                      :label="$t('scenario.numberOfConditionsSatisfied')"
                      prop="count"
                    >
                      <InputNumber
                        v-model="currentTriggerInfo.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- 到达位置-绝对位置-触发条件表格 -->
                  <template v-else-if="formType === 'reach_position'">
                    <!--    位置类型    -->
                    <el-form-item
                      :label="$t('scenario.locationType')"
                      prop="positiontype"
                    >
                      <el-select v-model="currentTriggerInfo.positiontype" @change="handleChangePositionType">
                        <el-option
                          v-for="positionType in positionTypeList"
                          :key="positionType.value"
                          :label="$t(positionType.label)"
                          :value="positionType.value"
                        />
                      </el-select>
                    </el-form-item>
                    <template v-if="currentTriggerInfo.positiontype === 'absolute_position'">
                      <!--    经度    -->
                      <el-form-item
                        :label="$t('scenario.longitude')"
                        prop="lon"
                      >
                        <InputNumber
                          v-model="currentTriggerInfo.lon"
                          :precision="8"
                          unit="°"
                        />
                      </el-form-item>
                      <!--    纬度    -->
                      <el-form-item
                        :label="$t('scenario.latitude')"
                        prop="lat"
                      >
                        <InputNumber
                          v-model="currentTriggerInfo.lat"
                          :precision="8"
                          unit="°"
                        />
                      </el-form-item>
                      <!--    海拔    -->
                      <el-form-item
                        :label="$t('scenario.altitude')"
                        prop="alt"
                      >
                        <InputNumber
                          v-model="currentTriggerInfo.alt"
                          :precision="3"
                          unit="m"
                        />
                      </el-form-item>
                      <!--    半径    -->
                      <el-form-item
                        :label="$t('radius')"
                        prop="radius"
                      >
                        <InputNumber
                          v-model="currentTriggerInfo.radius"
                          :min="0"
                          :max="100"
                          :precision="2"
                          unit="m"
                        />
                      </el-form-item>
                      <!--    对象    -->
                      <el-form-item
                        :label="$t('object')"
                        prop="targetelement"
                      >
                        <el-select v-model="currentTriggerInfo.targetelement">
                          <el-option
                            v-for="distanceMode in newObjectList0"
                            :key="distanceMode.value"
                            :label="distanceMode.label"
                            :value="distanceMode.value"
                          />
                        </el-select>
                      </el-form-item>
                    </template>
                    <template v-if="currentTriggerInfo.positiontype === 'absolute_lane'">
                      <!--    Road ID    -->
                      <el-form-item
                        :label="$t('scenario.roadID')"
                        prop="roadid"
                      >
                        <el-select v-model="currentTriggerInfo.roadid">
                          <el-option
                            v-for="roadId in newRoadIdList"
                            :key="roadId"
                            :label="roadId"
                            :value="roadId"
                          />
                        </el-select>
                      </el-form-item>
                      <!--    Lane ID    -->
                      <el-form-item
                        :label="$t('scenario.laneId')"
                        prop="laneid"
                      >
                        <el-select v-model="currentTriggerInfo.laneid">
                          <el-option
                            v-for="laneId in newLaneIdList"
                            :key="laneId"
                            :label="laneId"
                            :value="laneId"
                          />
                        </el-select>
                      </el-form-item>
                      <!--    对象    -->
                      <el-form-item
                        :label="$t('object')"
                        prop="targetelement"
                      >
                        <el-select v-model="currentTriggerInfo.targetelement">
                          <el-option
                            v-for="distanceMode in newObjectList0"
                            :key="distanceMode.value"
                            :label="distanceMode.label"
                            :value="distanceMode.value"
                          />
                        </el-select>
                      </el-form-item>
                      <!--    横向偏移    -->
                      <el-form-item
                        :label="$t('scenario.lateralOffset')"
                        prop="lateraloffset"
                      >
                        <InputNumber
                          v-model="currentTriggerInfo.lateraloffset"
                          :precision="3"
                          unit="m"
                        />
                      </el-form-item>
                      <!--    纵向距离    -->
                      <el-form-item
                        :label="$t('scenario.longitudinalDistance')"
                        prop="longitudinaloffset"
                      >
                        <InputNumber
                          v-model="currentTriggerInfo.longitudinaloffset"
                          unit="m"
                          :precision="3"
                        />
                      </el-form-item>
                      <!--    容差    -->
                      <el-form-item
                        :label="$t('scenario.tolerance')"
                        prop="tolerance"
                      >
                        <InputNumber
                          v-model="currentTriggerInfo.tolerance"
                          :precision="3"
                          unit="m"
                        />
                      </el-form-item>
                    </template>
                    <!--    条件边界    -->
                    <el-form-item
                      :label="$t('scenario.conditionalBoundary')"
                      prop="boundary"
                    >
                      <el-select v-model="currentTriggerInfo.boundary">
                        <el-option
                          v-for="conditionBoundary in conditionBoundaryList"
                          :key="conditionBoundary.value"
                          :label="$t(conditionBoundary.label)"
                          :value="conditionBoundary.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件满足次数    -->
                    <el-form-item
                      :label="$t('scenario.numberOfConditionsSatisfied')"
                      prop="count"
                    >
                      <InputNumber
                        v-model="currentTriggerInfo.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- 自定义变量-触发条件表格 -->
                  <template v-else-if="formType === 'ego_attach_laneid_custom' ">
                    <!--    变量    -->
                    <el-form-item
                      :label="$t('variable')"
                      prop="variate"
                    >
                      <el-input
                        v-model="currentTriggerInfo.variate"
                        :disabled="true"
                      />
                    </el-form-item>
                    <!--    Road ID    -->
                    <el-form-item
                      :label="$t('scenario.roadID')"
                      prop="roadid"
                    >
                      <el-select v-model="currentTriggerInfo.roadid">
                        <el-option
                          v-for="roadId in newRoadIdList"
                          :key="roadId"
                          :label="roadId"
                          :value="roadId"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    Section ID    -->
                    <el-form-item
                      :label="$t('scenario.sectionID')"
                      prop="sectionid"
                    >
                      <el-select v-model="currentTriggerInfo.sectionid">
                        <el-option
                          v-for="sectionid in newSectionIdList"
                          :key="sectionid"
                          :label="sectionid"
                          :value="sectionid"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    Lane ID    -->
                    <el-form-item
                      :label="$t('scenario.laneId')"
                      prop="laneid"
                    >
                      <el-select v-model="currentTriggerInfo.laneid">
                        <el-option
                          v-for="laneId in newLaneIdList"
                          :key="laneId"
                          :label="laneId"
                          :value="laneId"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件边界    -->
                    <el-form-item
                      :label="$t('scenario.conditionalBoundary')"
                      prop="boundary"
                    >
                      <el-select v-model="currentTriggerInfo.boundary">
                        <el-option
                          v-for="conditionBoundary in conditionBoundaryList"
                          :key="conditionBoundary.value"
                          :label="$t(conditionBoundary.label)"
                          :value="conditionBoundary.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    条件满足次数    -->
                    <el-form-item
                      :label="$t('scenario.numberOfConditionsSatisfied')"
                      prop="count"
                    >
                      <InputNumber
                        v-model="currentTriggerInfo.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                </div>
              </el-collapse-item>
              <el-collapse-item :title="$t('scenario.triggerResult')" name="triggerResult">
                <div class="item-1-body">
                  <!--    Trigger result    -->
                  <el-form-item>
                    <template #label>
                      <el-checkbox
                        v-model="velocityAction.checked"
                        :label="$t('velocity')"
                        true-label="true"
                        false-label="false"
                      >
                        {{ $t('scenario.speedVariation') }}
                      </el-checkbox>
                    </template>

                    <InputNumber
                      v-model="velocityAction.actionvalue"
                      :precision="2"
                      :min="-66.68"
                      :max="66.68"
                      unit="m/s"
                      :allow-empty-value="true"
                    />
                  </el-form-item>
                  <!--    加速度变化    -->
                  <el-form-item>
                    <template #label>
                      <el-checkbox
                        v-model="accAction.checked"
                        :label="$t('acceleration')"
                        true-label="true"
                        false-label="false"
                        @change="HandleAccActionChange"
                      >
                        {{ $t('scenario.accVariation') }}
                      </el-checkbox>
                    </template>
                    <InputNumber
                      v-model="accAction.actionvalue"
                      :precision="2"
                      unit="m/s²"
                      :allow-empty-value="true"
                    />
                  </el-form-item>
                  <!--    终止条件    -->
                  <el-form-item>
                    <template #label>
                      <span class="indent-22">{{ $t('scenario.endCondition') }}</span>
                    </template>
                    <el-select
                      v-model="accEndCondition.endconditiontype"
                      :disabled="!accAction.checked"
                      :style="{
                        width: accEndCondition.endconditiontype === 'none' ? '100%' : '70px',
                        marginRight: accEndCondition.endconditiontype === 'none' ? '0' : '5px',
                      }"
                      @change="handleChangeEndCondition"
                    >
                      <el-option
                        v-for="endCondition in endConditionList"
                        :key="endCondition.value"
                        :label="$t(endCondition.label)"
                        :value="endCondition.value"
                      />
                    </el-select>
                    <InputNumber
                      v-show="accEndCondition.endconditiontype !== 'none'"
                      v-model="accEndCondition.endconditionvalue"
                      :disabled="!accAction.checked"
                      :precision="2"
                      :min="accEndCondition.endconditiontype === 'time' ? 0 : -33.4 "
                      :max="accEndCondition.endconditiontype === 'time' ? undefined : 33.4"
                      :unit="accEndCondition.endconditiontype === 'time' ? 's' : 'm/s'"
                      style="width:40%"
                    />
                  </el-form-item>
                  <!--    Position offset    -->
                  <el-form-item>
                    <template #label>
                      <el-checkbox
                        v-model="mergeAction.checked"
                        :label="$t('merge')"
                        true-label="true"
                        false-label="false"
                        :disabled="isMachine"
                      >
                        {{ $t('scenario.posOffset') }}
                      </el-checkbox>
                    </template>
                    <el-select v-model="mergeAction.subtype" :disabled="isMachine">
                      <el-option
                        v-for="offset in offsetList"
                        :key="offset.value"
                        :label="$t(offset.label)"
                        :value="offset.value"
                      />
                    </el-select>
                  </el-form-item>
                  <!--    变道时间    -->
                  <el-form-item>
                    <template #label>
                      <span class="indent-22">{{ $t('scenario.laneChangingTime') }}</span>
                    </template>
                    <InputNumber
                      v-model="mergeAction.actionvalue"
                      :precision="2"
                      unit="s"
                      :allow-empty-value="true"
                      :disabled="!mergeAction.checked || isMachine"
                    />
                  </el-form-item>
                  <!--    道内偏移    -->
                  <el-form-item>
                    <template #label>
                      <span class="indent-22">{{ $t('scenario.insideLaneOffset') }}</span>
                    </template>
                    <InputNumber
                      v-model="mergeAction.laneoffset"
                      :precision="2"
                      :min="0"
                      :max="2.5"
                      unit="m"
                      :allow-empty-value="true"
                      :disabled="!(mergeAction.checked
                        && (['left_in_lane', 'right_in_lane'].includes(mergeAction.subtype))) || isMachine"
                    />
                  </el-form-item>
                </div>
              </el-collapse-item>
            </el-collapse>
          </div>
        </el-form>
        <div v-else>
          <div class="add-new-trigger-right" @click="handleAddTrigger">
            <el-icon class="el-icon-plus">
              <plus />
            </el-icon>
            <span>{{ $t('operation.addTrigger') }}</span>
          </div>
        </div>
      </div>
    </div>

    <section class="button-group">
      <el-button class="dialog-cancel" @click="$emit('close')">
        {{ $t('operation.cancel') }}
      </el-button>
      <el-button class="dialog-ok" @click="handleSave">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapGetters, mapMutations, mapState } from 'vuex'
import { ElMessage } from 'element-plus'
import { editor } from '@/api/interface'
import { validateScenarioSetName } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'
import i18n from '@/locales'
import dict from '@/common/dict'
import { messageBoxConfirm } from '@/plugins/message-box'

const { distanceModeOptionList } = dict

const changeTipInfoList = [
  i18n.t('tips.changeWhenUnfilledItemsInTrigger'),
  i18n.t('tips.changeWhenIllegalTriggerName'),
  i18n.t('tips.changeWhenTriggerNameExist'),
]
const quitTipInfoList = [
  i18n.t('tips.confirmExitWithoutSaved'),
]

const saveTipInfoList = [
  i18n.t('tips.cantSaveUnfilledItemsInTrigger'),
  i18n.t('tips.cantSaveIllegalTriggerName'),
  i18n.t('tips.cantSaveTriggerNameExist'),
]

const addTipInfoList = [
  i18n.t('tips.cantAddUnfilledItemsInTrigger'),
  i18n.t('tips.cantAddIllegalTriggerName'),
  i18n.t('tips.cantAddTriggerNameExist'),
]

const triggerConditionTypeList = [
  { value: 'time_trigger', label: 'scenario.simTime' },
  { value: 'ttc_trigger', label: 'scenario.TTC' },
  { value: 'reach_position', label: 'scenario.arrivalPos' },
  // { value: 'reach_abs_lane', label: 'Arrival position' },
  { value: 'distance_trigger', label: 'scenario.distanceTrigger' },
  { value: 'velocity_trigger', label: 'scenario.speedTrigger' },
  { value: 'ego_attach_laneid_custom', label: 'scenario.customVarParam' },
]

const comparisonOperatorList = [
  { value: 'lt', label: '<' },
  { value: 'lte', label: '≤' },
  { value: 'eq', label: '=' },
  { value: 'gt', label: '>' },
  { value: 'gte', label: '≥' },
]

const conditionBoundaryList = [
  { value: 'none', label: 'none' },
  { value: 'rising', label: 'rise' },
  { value: 'falling', label: 'fall' },
  { value: 'both', label: 'riseOrFall' },
]

const speedTypeList = [
  { value: 'absolute', label: 'scenario.absoluteSpeed' },
  { value: 'relative', label: 'scenario.relativeVelocity' },
]

const positionTypeList = [
  { value: 'absolute_position', label: 'scenario.absolutePosition' },
  // { value: 'relative_position', label: '相对位置' },
  { value: 'absolute_lane', label: 'scenario.absoluteLane' },
  // { value: 'relative_lane', label: '相对车道' },
]

const offsetList = [
  // { value: 'none', label: 'None' },
  { value: 'left', label: 'scenario.moveToTheLeftLane' },
  { value: 'right', label: 'scenario.moveToTheRightLane' },
  { value: 'left_in_lane', label: 'scenario.moveToTheLeftOfTheLane' },
  { value: 'right_in_lane', label: 'scenario.moveToTheRightOfTheLane' },
]

const endConditionList = [
  { value: 'none', label: 'none' },
  { value: 'time', label: 'time' },
  { value: 'velocity', label: 'velocity' },
]

const roadIdList = [12, 13, 14, 22]
const laneIdList = [12, 13, 14, 22]

const oldDirectionToNewMergeSubtype = {
  static: 'none',
  left: 'left',
  right: 'right',
  laneleft: 'left_in_lane',
  laneright: 'right_in_lane',
}

export default {
  name: 'CarTriggerConfig',
  components: {
    InputNumber,
  },
  data () {
    const nameValidator = (rule, value, callback) => {
      if (!validateScenarioSetName(value)) {
        callback(new Error(this.$t('tips.namePrefixErr', { name: this.$t('scenario.triggerName') })))
      }
      const existing = this.newTriggerList.some((_) => {
        if (_.id === this.currentTriggerInfo.id) {
          return false
        } else return _.name === value
      })
      if (existing) {
        callback(new Error(this.$t('tips.nameExists')))
      } else {
        callback()
      }
    }
    return {
      activeCollapse: ['triggerCondition', 'triggerResult'],
      triggerTemp: {
        triggertime: '1',
        threshold: '',
        variate: 'ego_laneid',
        equationop: 'eq',
        distype: 'euclideandistance',
        boundary: 'none',
        count: '1',
        unit: 'm_s',
        targetelement: '',
        sourceelement: 'ego',
        speedtype: 'absolute',
        positiontype: '',
        lon: '',
        lat: '',
        alt: '',
        radius: '5',
        roadid: '',
        laneid: '',
        sectionid: '',
        lateraloffset: '0',
        longitudinaloffset: '0',
        tolerance: '0',
        info: '',
        endCondition: [{ endconditiontype: 'none', endconditionvalue: '0.0' }, {
          endconditiontype: 'none',
          endconditionvalue: '0.0',
        }, { endconditiontype: 'none', endconditionvalue: '0.0' }],
        action: [
          {
            actiontype: 'velocity',
            actionvalue: '0',
            subtype: 'none',
            laneoffset: '0',
            checked: 'false',
          },
          {
            actiontype: 'merge',
            actionvalue: '0',
            subtype: 'left',
            laneoffset: '0',
            checked: 'false',
          },
          { actiontype: 'acc', actionvalue: '0', subtype: 'none', laneoffset: '0', checked: 'false' },
        ],
      },
      velocityTriggerConditionKeys: ['threshold', 'equationop', 'distype', 'boundary', 'count', 'unit', 'targetelement',
        // 'sourceelement',
        'speedtype'],
      absolutePositionConditionKeys: ['boundary', 'count', 'targetelement', 'positiontype', 'lon', 'lat', 'alt', 'radius'],
      absoluteLaneConditionKeys: ['boundary', 'count', 'targetelement', 'positiontype', 'roadid', 'laneid', 'lateraloffset', 'longitudinaloffset', 'tolerance'],
      timeTriggerConditionKeys: ['triggertime'],
      ttcTriggerConditionKeys: ['threshold', 'equationop', 'distype', 'boundary', 'count', 'targetelement'],
      distanceTriggerConditionKeys: ['threshold', 'equationop', 'distype', 'boundary', 'count', 'targetelement'],
      egoAttachLaneidCustomConditionKeys: ['variate', 'boundary', 'count', 'roadid', 'sectionid', 'laneid'],
      isAdd: false,
      isDelete: false,
      isChangeType: false,
      newTriggerList: [],
      trigger: {
        timeList: [],
        conditionList: [],
      },
      currentTriggerInfoIndex: -1,
      currentTriggerInfo: '',
      originCurrentTriggerInfo: '',
      originSceneevents: '',
      noItemInOriginTriggerList: '',
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterName'), trigger: 'blur' },
          { validator: nameValidator, trigger: 'blur' },
        ],
      },
      triggerConditionTypeList,
      comparisonOperatorList,
      distanceModeOptionList,
      conditionBoundaryList,
      speedTypeList,
      roadIdList,
      laneIdList,
      positionTypeList,
      offsetList,
      endConditionList,
      changeTipInfoList,
      quitTipInfoList,
      saveTipInfoList,
      addTipInfoList,
    }
  },
  computed: {
    ...mapGetters('scenario', [
      'selectedObject',
    ]),
    ...mapState('scenario', ['roadObj', 'sceneevents', 'objects']),
    eventId () {
      return this.selectedObject.eventId.split(',')
    },
    currentTriggerId () {
      return this.currentTriggerInfo.id
    },
    formType: {
      get () {
        if (this.currentTriggerInfo) {
          if (this.currentTriggerInfo.type === 'reach_abs_position' ||
            this.currentTriggerInfo.type === 'reach_abs_lane') {
            return 'reach_position'
          }
          return this.currentTriggerInfo.type
        }
        return ''
      },
      set (value) {
        if (value === 'reach_position') {
          this.currentTriggerInfo.type = 'reach_abs_position'
        } else {
          this.currentTriggerInfo.type = value
        }
      },
    },
    currentTriggerInfoIsDirty () {
      return !_.isEqual(this.originCurrentTriggerInfo, this.currentTriggerInfo)
    },
    isDirty () {
      return this.currentTriggerInfoIsDirty || this.isAdd || this.isDelete
    },
    velocityActionIndex () {
      return this.currentTriggerInfo.action.findIndex(_ => _.actiontype === 'velocity')
    },
    mergeActionIndex () {
      return this.currentTriggerInfo.action.findIndex(_ => _.actiontype === 'merge')
    },
    accActionIndex () {
      return this.currentTriggerInfo.action.findIndex(_ => _.actiontype === 'acc')
    },
    accEndConditionIndex () {
      return this.currentTriggerInfo.action.findIndex(_ => _.actiontype === 'acc')
    },
    velocityAction () {
      return this.currentTriggerInfo.action[this.velocityActionIndex]
    },
    mergeAction () {
      return this.currentTriggerInfo.action[this.mergeActionIndex]
    },
    accAction () {
      return this.currentTriggerInfo.action[this.accActionIndex]
    },
    accEndCondition () {
      return this.currentTriggerInfo.endCondition[this.accEndConditionIndex]
    },
    newRoadIdList () {
      return _.keys(this.roadObj)
    },
    newLaneIdList () {
      return this.currentTriggerInfo.roadid ? this.roadObj[this.currentTriggerInfo.roadid].laneIds : []
    },
    newSectionIdList () {
      return this.currentTriggerInfo.roadid ? this.roadObj[this.currentTriggerInfo.roadid].sectionIds : []
    },
    formConditionHasUnfilledItem () {
      const validFn = key => !(this.currentTriggerInfo[key] || this.currentTriggerInfo[key] === 0)

      if (this.formType === 'reach_position') {
        return this[`${_.camelCase(this.currentTriggerInfo.positiontype)}ConditionKeys`].some(validFn)
      }
      return this[`${_.camelCase(this.formType)}ConditionKeys`].some(validFn)
    },
    formActionHasUnfilledItem () {
      return this.currentTriggerInfo.action.some(({ actionvalue, subtype, checked, laneoffset }) => {
        // 如果checked为字符串，则转换为布尔值
        checked = typeof checked === 'string' ? checked === 'true' : checked
        if (checked) {
          // 如果位置偏移为跨车道向左、向右，则检测actionvalue以及laneoffset
          // 如果位置偏移不为跨车道向左、向右，则只检测actionvalue
          if (['left', 'right'].includes(subtype)) {
            return !((actionvalue || actionvalue === 0) && (laneoffset || laneoffset === 0))
          } else {
            return !(actionvalue || actionvalue === 0)
          }
        } else return false
      })
    },
    newObjectList0 () {
      return this.objects.filter((o) => {
        return !['signlight', 'obstacle'].includes(o.type)
      })
        .map((o) => {
          return {
            value: o.type === 'planner' ? 'ego' : `${o.type === 'car' ? 'v' : 'p'}_${o.id}`,
            label: o.type === 'planner' ? 'ego' : `${o.type === 'car' ? 'v' : 'p'}_${o.id}`,
          }
        })
    },
    newObjectList1 () {
      return this.objects.filter((o) => {
        return !['signlight', 'obstacle'].includes(o.type) && o.id !== this.selectedObject.id
      })
        .map((o) => {
          return {
            value: o.type === 'planner' ? 'ego' : `${o.type === 'car' ? 'v' : 'p'}_${o.id}`,
            label: o.type === 'planner' ? 'ego' : `${o.type === 'car' ? 'v' : 'p'}_${o.id}`,
          }
        })
    },
    isMachine () {
      return this.selectedObject?.type === 'machine'
    },
  },
  async created () {
    this.triggerTemp.targetelement = 'ego'
    this.triggerTemp.sourceelement = `${this.selectedObject.type === 'car' ? 'v' : 'p'}_${this.selectedObject.id}`
    await this.parseSelectedObjectTrigger()
    if (this.newTriggerList.length) {
      await this.handleChangeTrigger(0)
    } else {
      this.noItemInOriginTriggerList = true
    }
  },
  methods: {
    ...mapMutations('scenario', [
      'updateObject',
      'updateSceneevents',
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
    async parseSelectedObjectTrigger () {
      this.newTriggerList = this.sceneevents.filter(trigger => this.eventId.includes(trigger.id))
      this.originSceneevents = _.cloneDeep(this.sceneevents)

      const { trigger } = this.selectedObject
      if (trigger) {
        let { timeList, conditionList } = trigger
        timeList = timeList.filter(item => !_.isEqual(item, {
          time: 0,
          direction: 'static',
          acceleration: 0,
          velocity: '',
          durations: 4.5,
          offsets: 0,
          accelerationTerminationType: '',
          accelerationTerminationValue: 0,
        }))
        if (timeList) {
          this.trigger.timeList = timeList
          for (const t of timeList) {
            await this.handleAddTrigger()
            const type = 'time_trigger'
            const endCondition = [
              { endconditiontype: 'none', endconditionvalue: '0.0' },
              { endconditiontype: 'none', endconditionvalue: '0.0' },
              { endconditiontype: 'none', endconditionvalue: '0.0' },
            ]
            const action = [
              {
                actiontype: 'velocity',
                actionvalue: '0',
                subtype: 'none',
                laneoffset: '0',
                checked: 'false',
              },
              {
                actiontype: 'merge',
                actionvalue: '0',
                subtype: 'none',
                laneoffset: '0',
                checked: 'false',
              },
              {
                actiontype: 'acc',
                actionvalue: '0',
                subtype: 'none',
                laneoffset: '0',
                checked: 'false',
              },
            ]
            if (_.isNumber(t.velocity)) {
              // velocity action
              action[0].checked = 'true'
              action[0].actionvalue = t.velocity
            }
            if (t.direction !== 'static') {
              // merge action
              action[1].checked = 'true'
              action[1].subtype = oldDirectionToNewMergeSubtype[t.direction]
              action[1].actionvalue = t.durations
              action[1].laneoffset = t.offsets
            }

            if (t.acceleration !== 0) {
              // acc action
              action[2].checked = 'true'
              action[2].actionvalue = t.acceleration
            }

            if (t.accelerationTerminationType) {
              action[2].checked = 'true'
              endCondition[2].endconditiontype = t.accelerationTerminationType
              endCondition[2].endconditionvalue = `${t.accelerationTerminationValue}`
            }

            Object.assign(this.currentTriggerInfo, {
              type,
              triggertime: `${t.time}`,
              name: `old_trigger_${this.currentTriggerInfo.id}`,
              endCondition,
              action,
            })
            console.log({ ...this.currentTriggerInfo }, this.currentTriggerInfoIndex)
          }
        }
        conditionList = conditionList.filter(item => !_.isEqual(item, {
          type: 'timeRelative',
          value: 0,
          distancemode: 'laneprojection',
          direction: 'static',
          acceleration: 0,
          velocity: '',
          durations: 4.5,
          offsets: 0,
          times: 1,
          accelerationTerminationType: '',
          accelerationTerminationValue: 0,
        }))
        if (conditionList) {
          this.trigger.conditionList = conditionList
          for (const t of conditionList) {
            await this.handleAddTrigger()
            let type
            const relativeConditions = {}
            if (t.type === 'timeRelative') {
              type = 'ttc_trigger'
            } else if (t.type === 'distanceRelative') {
              type = 'distance_trigger'
            }
            relativeConditions.distype = t.distancemode
            relativeConditions.equationop = 'eq'
            relativeConditions.threshold = t.value
            relativeConditions.targetelement = 'ego'
            relativeConditions.count = t.times
            relativeConditions.boundary = 'none'
            const endCondition = [
              { endconditiontype: 'none', endconditionvalue: '0.0' },
              { endconditiontype: 'none', endconditionvalue: '0.0' },
              { endconditiontype: 'none', endconditionvalue: '0.0' },
            ]
            const action = [
              {
                actiontype: 'velocity',
                actionvalue: '0',
                subtype: 'none',
                laneoffset: '0',
                checked: 'false',
              },
              {
                actiontype: 'merge',
                actionvalue: '0',
                subtype: 'none',
                laneoffset: '0',
                checked: 'false',
              },
              {
                actiontype: 'acc',
                actionvalue: '0',
                subtype: 'none',
                laneoffset: '0',
                checked: 'false',
              },
            ]
            if (_.isNumber(t.velocity)) {
              // velocity action
              action[0].checked = 'true'
              action[0].actionvalue = t.velocity
            }
            if (t.direction !== 'static') {
              // merge action
              action[1].checked = 'true'
              action[1].subtype = oldDirectionToNewMergeSubtype[t.direction]
              action[1].actionvalue = t.durations
              action[1].laneoffset = t.offsets
            }

            if (t.acceleration !== 0) {
              // acc action
              action[2].checked = 'true'
              action[2].actionvalue = t.acceleration
            }

            if (t.accelerationTerminationType) {
              action[2].checked = 'true'
              endCondition[2].endconditiontype = t.accelerationTerminationType
              endCondition[2].endconditionvalue = `${t.accelerationTerminationValue}`
            }

            Object.assign(this.currentTriggerInfo, {
              type,
              ...relativeConditions,
              name: `old_trigger_${this.currentTriggerInfo.id}`,
              endCondition,
              action,
            })
            console.log({ ...this.currentTriggerInfo }, this.currentTriggerInfoIndex)
          }
        }
        if (timeList.length || conditionList.length) {
          this.$message.info(this.$t('tips.vehicleBoundOldEvent'))
        }
      }

      // 如果开局没有触发，则自动新增一条
      if (!this.newTriggerList.length) {
        await this.handleAddTrigger()
      }
      this.newTriggerList.forEach((trigger, index) => {
        // 将type为reach_abs_position 或者reach_abs_lane的数据
        // 转换为type为reach_position和positiontype为absolute_position/absolute_lane
        if (trigger.type === 'reach_abs_position') {
          this.newTriggerList[index].type = 'reach_abs_position'
          this.newTriggerList[index].positiontype = 'absolute_position'
        }
        if (trigger.type === 'reach_abs_lane') {
          this.newTriggerList[index].type = 'reach_abs_lane'
          this.newTriggerList[index].positiontype = 'absolute_lane'
        }
        // 如果type为ego_attach_laneid_custom并且没有variate属性，那么variate始终为ego_laneid(目前后端无法返回variate参数，待后端修复)
        if (trigger.type === 'ego_attach_laneid_custom' && !_.get(trigger, 'variate')) {
          this.newTriggerList[index] = {
            ...this.newTriggerList[index],
            variate: 'ego_laneid',
          }
        }
        // 如果boundary为''，则改为none
        if (!trigger.boundary) this.newTriggerList[index].boundary = 'none'
      })
    },
    transferNumToStr (sourceObject) {
      _.forIn(sourceObject, (value, key) => {
        if (typeof value === 'number') {
          sourceObject[key] = String(value)
        } else if (typeof value === 'object') this.transferNumToStr(sourceObject[key])
      })
    },
    modifyObject (params) {
      // 将当前触发中的所有数字变为字符串;转换reach_position为reach_abs_position或者reach_abs_lane
      this.newTriggerList.forEach((trigger, index) => {
        if (String(trigger.id) === this.currentTriggerInfo.id) {
          this.transferNumToStr(this.newTriggerList[index])
        }
      })
      this.originSceneevents.forEach((trigger, index) => {
        this.transferNumToStr(this.originSceneevents[index])
      })
      const data = { ...this.selectedObject, ...params }
      const payload = {
        type: this.selectedObject.type,
        data: {
          ...data,
          triggers: this.newTriggerList || [],
          eventId: this.newTriggerList.map(_ => _.id).join(',') || '',
          sceneevents: this.originSceneevents || [],
        },
      }
      try {
        editor.object.update(payload)
        this.updateObject(payload)
        this.updateSceneevents(this.originSceneevents)
      } catch (e) {
        console.error(e)
      }
    },
    async handleSave () {
      // 新增触发代码部分
      const res = await this.validateForm('save')
      if (res) {
        if (this.currentTriggerInfo) {
          this.newTriggerList.splice(this.currentTriggerInfoIndex, 1, _.cloneDeep(this.currentTriggerInfo))
          this.originCurrentTriggerInfo = _.cloneDeep(this.currentTriggerInfo)
          this.originSceneevents.splice(
            this.originSceneevents.findIndex(item => item.id === this.currentTriggerInfo.id),
            1,
            _.cloneDeep(this.currentTriggerInfo),
          )
        }
        ElMessage.success(this.$t('tips.saveSuccess'))
        // 清除旧的触发数据
        this.modifyObject({
          trigger: {
            timeList: [],
            conditionList: [],
          },
        })
        this.isAdd = false
        this.isDelete = false
        this.isChangeType = false
        this.$emit('close')
      }
    },
    async beforeClose () {
      if (this.isDirty) {
        try {
          const res = await messageBoxConfirm(this.quitTipInfoList[0])
          return res === 'confirm'
        } catch (e) {
          return false
        }
      }
    },
    handleDeleteTrigger (index, id) {
      // 分别在newTriggerList和originSceneevents中删除对应触发
      this.newTriggerList.splice(index, 1)
      this.originSceneevents.splice(this.originSceneevents.findIndex(item => item.id === id), 1)
      // 如果删除的触发为当前触发内容
      if (index === this.currentTriggerInfoIndex) {
        // 如果删除后触发列表长度不为0,则修改当前触发内容；若删除后出发列表长度为0，则将当前触发内容置空
        if (this.newTriggerList.length) {
          this.currentTriggerInfoIndex = this.currentTriggerInfoIndex ?
            this.currentTriggerInfoIndex - 1 :
            this.currentTriggerInfoIndex
          this.currentTriggerInfo = _.cloneDeep(this.newTriggerList[this.currentTriggerInfoIndex])
          this.originCurrentTriggerInfo = _.cloneDeep(this.currentTriggerInfo)
        } else {
          this.currentTriggerInfo = ''
          this.originCurrentTriggerInfo = ''
          this.currentTriggerInfoIndex = -1
        }
      }
      // 删除的触发index小于当前触发index的情况
      if (index < this.currentTriggerInfoIndex) {
        this.currentTriggerInfoIndex = this.currentTriggerInfoIndex - 1
      }
      // 删除的触发index大于当前触发index的情况，不用修改
      this.isDelete = true
    },
    async handleAddTrigger () {
      // 新增之前先验证当前表格，若验证不通过，则不能新增
      const res = await this.validateForm('add')
      if (res) {
        // 新增触发id为sceneevents里的id递增
        const sceneeventsIdList = this.originSceneevents.map(_ => Number(_.id))
        const id = (() => {
          for (let i = 0; i < sceneeventsIdList.length > 0 ? Math.max(...sceneeventsIdList) + 2 : 1; i++) {
            if (!sceneeventsIdList.includes(i)) return i
          }
        })()
        // 处理新增触发的名称
        const nameReg = /^new_trigger_(\d+)$/
        const indexList = this.newTriggerList.reduce((acc, { name }) => {
          const matched = nameReg.exec(name)
          if (matched && matched.length) {
            acc.push(+matched[1])
          }
          return acc
        }, [0])
        const missIndex = (() => {
          for (let i = 0; i < Math.max(...indexList) + 2; i++) {
            if (!indexList.includes(i)) return i
          }
        })()
        // 新增触发后往newTriggerList和originSceneevents
        this.newTriggerList.push({
          ...this.triggerTemp,
          id,
          type: 'time_trigger',
          name: `new_trigger_${missIndex || Number(indexList.slice(-1)[0]) + 1}`,
        })
        this.originSceneevents.push({
          ...this.triggerTemp,
          id,
          type: 'time_trigger',
          name: `new_trigger_${missIndex || Number(indexList.slice(-1)[0]) + 1}`,
        })
        await this.handleChangeTrigger(this.newTriggerList.length - 1)
        this.isAdd = true
      }
    },
    async handleChangeTrigger (index) {
      const res = await this.validateForm('change')
      if (res === 'confirm') {
        this.currentTriggerInfo = this.originCurrentTriggerInfo
      }
      if (this.currentTriggerInfo) {
        this.$refs.form.clearValidate('name')
        this.newTriggerList.splice(this.currentTriggerInfoIndex, 1, this.currentTriggerInfo)
        this.originSceneevents.splice(
          this.originSceneevents.findIndex(item => item.id === this.currentTriggerInfo.id),
          1,
          this.currentTriggerInfo,
        )
      }
      this.currentTriggerInfo = _.cloneDeep(this.newTriggerList[index])
      this.originCurrentTriggerInfo = _.cloneDeep(this.currentTriggerInfo)
      this.currentTriggerInfoIndex = index
    },

    handleChangeEndCondition () {
      this.currentTriggerInfo.endCondition[this.accEndConditionIndex].endconditionvalue = 0
    },
    handleChangeType () {
      let positiontype = ''
      if (this.formType === 'reach_position') {
        positiontype = 'absolute_position'
      }
      this.currentTriggerInfo = {
        ...this.currentTriggerInfo,
        ..._.cloneDeep(this.triggerTemp),
        lon: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].lon : '0',
        lat: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].lat : '0',
        alt: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].alt : '0',
        positiontype,
      }
      this.isChangeType = true
    },
    handleChangePositionType (value) {
      let { type } = this.currentTriggerInfo
      if (value === 'absolute_position') {
        type = 'reach_abs_position'
      } else if (value === 'absolute_lane') {
        type = 'reach_abs_lane'
      }
      this.currentTriggerInfo = {
        ..._.cloneDeep(this.triggerTemp),
        ...this.currentTriggerInfo,
        lon: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].lon : '0',
        lat: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].lat : '0',
        alt: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].alt : '0',
        lateraloffset: '0',
        longitudinaloffset: '0',
        tolerance: '0',
        radius: '5',
        type,
      }
      this.isChangeType = true
    },
    HandleAccActionChange (val) {
      if (!val) {
        this.currentTriggerInfo.endCondition[this.accEndConditionIndex].endconditiontype = 'none'
        this.currentTriggerInfo.endCondition[this.accEndConditionIndex].endconditionvalue = 0
      }
    },
    validateForm (type) {
      if (this.currentTriggerInfo) {
        const nameExisting = this.newTriggerList.some((_) => {
          if (_.id === this.currentTriggerInfo.id) {
            return false
          } else return _.name === this.currentTriggerInfo.name
        })
        try {
          // 检测是否有未填项
          if (this.formConditionHasUnfilledItem || this.formActionHasUnfilledItem) {
            if (['save', 'add'].includes(type)) {
              ElMessage.error(this[`${type}TipInfoList`][0])
              return false
            } else {
              return messageBoxConfirm(this[`${type}TipInfoList`][0])
            }
            // 检测名称是否合法
          } else if (!validateScenarioSetName(this.currentTriggerInfo.name)) {
            if (['save', 'add'].includes(type)) {
              ElMessage.error(this[`${type}TipInfoList`][1])
              return false
            } else {
              return messageBoxConfirm(this[`${type}TipInfoList`][1])
            }
            // 检测名称是否存在
          } else if (nameExisting) {
            if (['save', 'add'].includes(type)) {
              ElMessage.error(this[`${type}TipInfoList`][2])
              return false
            } else {
              return messageBoxConfirm(this[`${type}TipInfoList`][2])
            }
          }
        } catch (e) {
          return false
        }
        return true
      } else {
        return true
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.car-trigger-config {
  width: 600px;
  height: auto;
  padding-top: 23px;
  background-color: @darker-bg;

  .car-trigger-config-body {
    height: 500px;
    width: 100%;
    display: flex;
    justify-content: space-around;
  }

  .left-section {
    width: 220px;
    margin-right: 4px;
    height: 100%;
    font-size: 12px;
    background-color: @dark-bg;
    color: #9d9d9d;
    display: flex;
    flex-direction: column;
    justify-content: space-between;

    .trigger-list {
      overflow-y: auto;

      .trigger-item {
        padding: 10px 20px;
        cursor: pointer;
        display: flex;
        justify-content: space-between;
        align-items: center;

        span {
          .text-overflow;
          flex: 1;
        }

        &:hover {
          background-color: @hover-bg;

          span {
            color: @active-font-color;
          }
        }
      }

      .is-active {
        background-color: @hover-bg;

        span {
          color: @active-font-color;
        }
      }
    }

    .add-new-trigger {
      text-align: center;
      background-color: @hover-bg;
      cursor: pointer;
      padding: 10px 0;

      .el-icon-plus {
        margin-right: 15px;
      }
    }
  }

  .right-section {
    flex: 1;
    font-size: 12px;
    background-color: @dark-bg;
    color: #9d9d9d;
    height: 100%;
    overflow-y: auto;
    position: relative;

    .add-new-trigger-right {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      cursor: pointer;
      height: 150px;
      width: 150px;
      display: flex;
      font-size: 20px;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      border: 1px dashed #9d9d9d;

      .el-icon-plus {
        margin-bottom: 10px;
      }

      &:hover {
        border: 1px dashed @active-font-color;
        color: @active-font-color;
      }
    }
  }

  .item-1 {
    :deep(.el-collapse-item__header) {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 5px 20px;
    }

    :deep(.el-form-item) {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 0 40px;

      &__label {
        flex: 0.5;
        text-align: left;
      }

      &__content {
        flex: 0.5;
        display: flex;
        text-align: left;

        .el-select {
          width: 100%;
        }

        .operator-selector {
          width: 50px;
          margin-right: 5px;
        }

        .input-number {
          flex: 1;
        }
      }
    }

    &-body {
      margin-top: 20px;
    }

    :deep(.el-checkbox__label) {
      color: #c2c2c2;
    }
  }

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

        .acceleration-termination-value {
          :deep(.input-number) {
            width: 38px;
          }
        }

        .trigger-condition {
          display: flex;
          align-items: center;

          .trigger-value {
            .text-overflow();
          }

          .trigger-unit {
            flex: 0;
          }
        }
      }
    }
  }

  .button-group {
    margin-top: 20px;
    text-align: right;
  }

  .indent-22 {
    padding-left: 22px;
  }
}
</style>
