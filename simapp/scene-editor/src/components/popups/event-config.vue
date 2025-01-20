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
          <span>{{ $t('operation.addEvent') }}</span>
        </div>
      </div>
      <div v-loading="loading" class="right-section">
        <el-form
          v-if="newTriggerList.length > 0 && currentTriggerInfo"
          ref="form"
          :model="currentTriggerInfo"
          :rules="rules"
        >
          <div class="item-1">
            <el-form-item
              :label="$t('scenario.eventName')"
              prop="name"
              maxlength="40"
              class="event-name-item"
            >
              <el-input v-model="currentTriggerInfo.name" />
            </el-form-item>
            <el-collapse v-model="activeCollapse">
              <el-collapse-item title="事件条件" name="eventCondition">
                <div
                  v-for="(condition, conditionIndex) in currentTriggerInfo.conditions"
                  :key="conditionIndex"
                  class="item-1-body"
                >
                  <!--    条件类型    -->
                  <el-form-item
                    :label="`${$t('scenario.triggeringConditionType')}${conditionIndex + 1}`"
                    prop="type"
                    class="item-1-type-selector"
                  >
                    <el-select
                      :model-value="formType[conditionIndex]"
                      :disabled="isTrafficLight"
                      @change="handleChangeType($event, conditionIndex)"
                    >
                      <el-option
                        v-for="conditionOption in triggerConditionTypeList"
                        :key="conditionOption.value"
                        :label="$t(conditionOption.label)"
                        :value="conditionOption.value"
                      />
                    </el-select>
                    <div class="condition-operators-container">
                      <el-icon
                        v-if="conditionIndex"
                        class="el-icon-minus condition-operator"
                        @click="removeCondition(conditionIndex)"
                      >
                        <minus />
                      </el-icon>
                      <el-icon class="el-icon-plus condition-operator" @click="addCondition">
                        <plus />
                      </el-icon>
                    </div>
                  </el-form-item>
                  <!-- 仿真时间触发条件表格 -->
                  <template v-if="formType[conditionIndex] === 'time_trigger'">
                    <!--    仿真时间值    -->
                    <el-form-item
                      :label="$t('scenario.simulationTimeValue')"
                      prop="triggertime"
                    >
                      <el-select v-model="condition.equationop" class="operator-selector">
                        <el-option label="=" value="eq" />
                        <el-option label=">" value="gt" />
                        <el-option label="<" value="lt" />
                      </el-select>
                      <InputNumber
                        v-model="condition.triggertime"
                        :min="0"
                        unit="s"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- TTC触发条件表格 -->
                  <template v-else-if="formType[conditionIndex] === 'ttc_trigger'">
                    <!--    TTC值    -->
                    <el-form-item
                      :label="$t('scenario.TTCValue')"
                      prop="threshold"
                    >
                      <el-select v-model="condition.equationop" class="operator-selector">
                        <el-option
                          v-for="operator in comparisonOperatorList"
                          :key="operator.value"
                          :label="operator.label"
                          :value="operator.value"
                        />
                      </el-select>
                      <InputNumber
                        v-model="condition.threshold"
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
                      <el-select v-model="condition.distype">
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
                      <el-select v-model="condition.targetelement">
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
                      <el-select v-model="condition.boundary">
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
                        v-model="condition.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- 距离触发条件表格 -->
                  <template v-else-if="formType[conditionIndex] === 'distance_trigger'">
                    <!--    距离值    -->
                    <el-form-item
                      :label="$t('scenario.distanceValue')"
                      prop="threshold"
                    >
                      <el-select v-model="condition.equationop" class="operator-selector">
                        <el-option
                          v-for="operator in comparisonOperatorList"
                          :key="operator.value"
                          :label="operator.label"
                          :value="operator.value"
                        />
                      </el-select>
                      <InputNumber
                        v-model="condition.threshold"
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
                      <el-select v-model="condition.distype">
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
                      <el-select v-model="condition.targetelement">
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
                      <el-select v-model="condition.boundary">
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
                        v-model="condition.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- 速度触发条件表格 -->
                  <template v-else-if="formType[conditionIndex] === 'velocity_trigger'">
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
                      <el-select v-model="condition.equationop" class="operator-selector">
                        <el-option
                          v-for="operator in comparisonOperatorList"
                          :key="operator.value"
                          :label="operator.label"
                          :value="operator.value"
                        />
                      </el-select>
                      <InputNumber
                        v-model="condition.threshold"
                        :allow-empty-value="true"
                        :precision="2"
                        :min="-100"
                        :max="100"
                        :unit="condition.unit === 'm_s' ? 'm/s' : ''"
                      />
                    </el-form-item>
                    <!--    速度类型    -->
                    <el-form-item
                      :label="$t('scenario.speedType')"
                      prop="speedtype"
                    >
                      <el-select v-model="condition.speedtype">
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
                      <el-select v-model="condition.targetelement">
                        <el-option
                          v-for="distanceMode in newObjectList1"
                          :key="distanceMode.value"
                          :label="distanceMode.label"
                          :value="distanceMode.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    Source对象    -->
                    <!--                    <el-form-item -->
                    <!--                      label="对象2" -->
                    <!--                      prop="sourceelement" -->
                    <!--                    > -->
                    <!--                      <el-select v-model="condition.sourceelement" :disabled="true"> -->
                    <!--                        <el-option -->
                    <!--                          v-for="distanceMode in newObjectList1" -->
                    <!--                          :key="distanceMode.value" -->
                    <!--                          :label="distanceMode.label" -->
                    <!--                          :value="distanceMode.value" -->
                    <!--                        /> -->
                    <!--                      </el-select> -->
                    <!--                    </el-form-item> -->
                    <!--    距离类型    -->
                    <!--                    <el-form-item -->
                    <!--                      :label="$t('scenario.distanceType')" -->
                    <!--                      prop="distype" -->
                    <!--                    > -->
                    <!--                      <el-select v-model="condition.distype"> -->
                    <!--                        <el-option -->
                    <!--                          v-for="distanceMode in distanceModeOptionList" -->
                    <!--                          :key="distanceMode.id" -->
                    <!--                          :label="$t(distanceMode.name)" -->
                    <!--                          :value="distanceMode.id" -->
                    <!--                        /> -->
                    <!--                      </el-select> -->
                    <!--                    </el-form-item> -->
                    <!--    条件边界    -->
                    <el-form-item
                      :label="$t('scenario.conditionalBoundary')"
                      prop="boundary"
                    >
                      <el-select v-model="condition.boundary">
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
                        v-model="condition.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!-- 到达位置-绝对位置-触发条件表格 -->
                  <template v-else-if="formType[conditionIndex] === 'reach_position'">
                    <!--    位置类型    -->
                    <el-form-item
                      :label="$t('scenario.locationType')"
                      prop="positiontype"
                    >
                      <el-select
                        v-model="condition.positiontype"
                        @change="handleChangePositionType($event, conditionIndex)"
                      >
                        <el-option
                          v-for="positionType in positionTypeList"
                          :key="positionType.value"
                          :label="$t(positionType.label)"
                          :value="positionType.value"
                        />
                      </el-select>
                    </el-form-item>
                    <template v-if="condition.positiontype === 'absolute_position'">
                      <!--    经度    -->
                      <el-form-item
                        :label="$t('scenario.longitude')"
                        prop="lon"
                      >
                        <InputNumber
                          v-model="condition.lon"
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
                          v-model="condition.lat"
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
                          v-model="condition.alt"
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
                          v-model="condition.radius"
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
                        <el-select v-model="condition.targetelement">
                          <el-option
                            v-for="distanceMode in newObjectList1"
                            :key="distanceMode.value"
                            :label="distanceMode.label"
                            :value="distanceMode.value"
                          />
                        </el-select>
                      </el-form-item>
                    </template>
                    <template v-if="condition.positiontype === 'absolute_lane'">
                      <!--    Road ID    -->
                      <el-form-item
                        :label="$t('scenario.roadID')"
                        prop="roadid"
                      >
                        <el-select v-model="condition.roadid" @change="selectRoadChange">
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
                        <el-select v-model="condition.laneid">
                          <el-option
                            v-for="laneId in newLaneIdList(conditionIndex)"
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
                        <el-select v-model="condition.targetelement">
                          <el-option
                            v-for="distanceMode in newObjectList1"
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
                          v-model="condition.lateraloffset"
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
                          v-model="condition.longitudinaloffset"
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
                          v-model="condition.tolerance"
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
                      <el-select v-model="condition.boundary">
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
                        v-model="condition.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!--    车头时距-触发条件表格    -->
                  <template v-else-if="formType[conditionIndex] === 'timeheadway_trigger'">
                    <!--    车头时距值    -->
                    <el-form-item :label="$t('scenario.timeHeadway')" prop="threshold">
                      <el-select v-model="condition.equationop" class="operator-selector">
                        <el-option
                          v-for="operator in comparisonOperatorList"
                          :key="operator.value"
                          :label="operator.label"
                          :value="operator.value"
                        />
                      </el-select>
                      <InputNumber
                        v-model="condition.threshold"
                        :allow-empty-value="true"
                        :precision="2"
                        :min="0"
                        :max="100"
                        unit="s"
                      />
                    </el-form-item>
                    <!--    对象    -->
                    <el-form-item
                      :label="$t('object')"
                      prop="targetelement"
                    >
                      <el-select v-model="condition.targetelement">
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
                      <el-select v-model="condition.boundary">
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
                        v-model="condition.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                  <!--    故事板事件-触发条件表格    -->
                  <template v-else-if="formType[conditionIndex] === 'element_state'">
                    <!--    故事板要素类型    -->
                    <el-form-item :label="$t('scenario.storyBoardElementType')" prop="element_type">
                      <el-select v-model="condition.element_type">
                        <el-option
                          v-for="type in storyBoardEleTypeList"
                          :key="type.value"
                          :label="$t(type.label)"
                          :value="type.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    故事板引用对象_事件_动作    -->
                    <el-form-item :label="$t('scenario.storyBoardReference')" prop="element_ref">
                      <el-select v-model="condition.element_ref" @change="handleChangeElementRef($event, condition)">
                        <el-option
                          v-for="reference in storyBoardReferenceList"
                          :key="reference.value"
                          :label="reference.label"
                          :value="reference.value"
                        />
                      </el-select>
                    </el-form-item>
                    <!--    故事板要素状态    -->
                    <el-form-item
                      :label="$t('scenario.storyBoardElementState')"
                      prop="state"
                    >
                      <el-select v-model="condition.state">
                        <el-option
                          v-for="state in storyBoardElementStateList(condition.element_ref)"
                          :key="state.value"
                          :label="$t(state.label)"
                          :value="state.value"
                        />
                      </el-select>
                    </el-form-item>
                  </template>
                  <!-- 自定义变量-触发条件表格 -->
                  <template v-else-if="formType[conditionIndex] === 'ego_attach_laneid_custom' ">
                    <!--    变量    -->
                    <el-form-item
                      :label="$t('variable')"
                      prop="variate"
                    >
                      <el-input
                        v-model="condition.variate"
                        :disabled="true"
                      />
                    </el-form-item>
                    <!--    Road ID    -->
                    <el-form-item
                      :label="$t('scenario.roadID')"
                      prop="roadid"
                    >
                      <el-select v-model="condition.roadid" @change="selectRoadChange">
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
                      <el-select v-model="condition.sectionid">
                        <el-option
                          v-for="sectionid in newSectionIdList(conditionIndex)"
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
                      <el-select v-model="condition.laneid">
                        <el-option
                          v-for="laneId in newLaneIdList(conditionIndex)"
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
                      <el-select v-model="condition.boundary">
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
                        v-model="condition.count"
                        :precision="0"
                        :min="0"
                        :unit="$t('indicator.timesUnit')"
                        :allow-empty-value="true"
                      />
                    </el-form-item>
                  </template>
                </div>
              </el-collapse-item>
              <el-collapse-item v-if="isTrafficLight" :title="$t('scenario.eventBehavior')" name="eventBehavior">
                <div class="item-1-body">
                  <el-form-item
                    :label="$t('scenario.phaseChange')"
                    prop=""
                  >
                    <InputNumber
                      v-model="posincycleValue"
                      :min="0"
                      unit="s"
                    />
                  </el-form-item>
                </div>
              </el-collapse-item>
              <el-collapse-item v-else :title="$t('scenario.eventBehavior')" name="eventBehavior">
                <EventBehaviorConfig
                  v-for="(act, index) of currentTriggerInfo.action"
                  ref="behaviorForm"
                  :key="index"
                  :index="Number(act.actionid)"
                  :data="act"
                  @delete="removeBehavior(index)"
                  @change="handleChangeBehavior(index, $event)"
                />
                <el-button class="add-new-behavior" @click="addBehavior">
                  <el-icon class="el-icon-plus">
                    <plus />
                  </el-icon>新增行为
                </el-button>
              </el-collapse-item>
            </el-collapse>
          </div>
        </el-form>
        <div v-else>
          <div class="add-new-trigger-right" @click="handleAddTrigger">
            <el-icon class="el-icon-plus">
              <plus />
            </el-icon>
            <span>{{ $t('operation.addEvent') }}</span>
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
import { mapActions, mapGetters, mapMutations, mapState } from 'vuex'
import { ElMessage } from 'element-plus'
import { editor } from '@/api/interface'
import { validateScenarioSetName } from '@/common/utils'
import InputNumber from '@/components/input-number.vue'
import i18n from '@/locales'
import dict from '@/common/dict'
import EventBehaviorConfig from '@/components/popups/event-behavior-config.vue'
import filters from '@/filters'
import { messageBoxConfirm } from '@/plugins/message-box'

const { distanceModeOptionList } = dict
const { $itemName } = filters

const changeTipInfoList = [
  i18n.t('tips.changeWhenUnfilledItemsInTrigger'),
  i18n.t('tips.changeWhenIllegalTriggerName'),
  i18n.t('tips.changeWhenTriggerNameExist'),
  i18n.t('tips.cantSaveTriggerAction'),
  i18n.t('tips.cantChangeWhenUnfilledItemsInTrigger'),
]
const quitTipInfoList = [
  i18n.t('tips.confirmExitWithoutSaved'),
]

const saveTipInfoList = [
  i18n.t('tips.cantSaveUnfilledItemsInTrigger'),
  i18n.t('tips.cantSaveIllegalTriggerName'),
  i18n.t('tips.cantSaveTriggerNameExist'),
  i18n.t('tips.cantSaveTriggerAction'),
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
  { value: 'timeheadway_trigger', label: 'scenario.timeHeadway' },
  { value: 'element_state', label: 'scenario.storyBoardEventState' },
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
  { value: 'absolute_lane', label: 'scenario.absoluteLane' },
]

const storyBoardEleTypeList = [
  { value: 'action', label: 'scenario.action' },
]

const storyBoardElementStateBase = [
  { value: 'completeState', label: 'scenario.completed' },
]

const storyBoardElementStateList = [
  ...storyBoardElementStateBase,
  { value: 'runningState', label: 'scenario.running' },
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
  name: 'EventConfig',
  components: {
    EventBehaviorConfig,
    InputNumber,
  },
  data () {
    const nameValidator = (rule, value, callback) => {
      if (!validateScenarioSetName(value)) {
        callback(new Error(this.$t('tips.namePrefixErr', { name: this.$t('scenario.eventName') })))
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
      loading: false,
      activeCollapse: ['eventCondition', 'eventBehavior'],
      triggerTemp: {
        conditions: [
          {
            triggertime: '1',
            threshold: '',
            variate: 'ego_laneid',
            equationop: 'eq',
            distype: 'euclideandistance',
            boundary: 'none',
            count: '1',
            unit: 'm_s',
            targetelement: '',
            sourceelement: '',
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
            element_type: 'action',
            element_ref: '',
            state: 'completeState',
            info: '',
          },
        ],
        action: [],
      },
      velocityTriggerConditionKeys: ['threshold', 'equationop', 'boundary', 'count', 'unit', 'targetelement',
        // 'sourceelement','distype',
        'speedtype'],
      absolutePositionConditionKeys: ['boundary', 'count', 'targetelement', 'positiontype', 'lon', 'lat', 'alt', 'radius'],
      absoluteLaneConditionKeys: ['boundary', 'count', 'targetelement', 'positiontype', 'roadid', 'laneid', 'lateraloffset', 'longitudinaloffset', 'tolerance'],
      timeTriggerConditionKeys: ['triggertime'],
      ttcTriggerConditionKeys: ['threshold', 'equationop', 'distype', 'boundary', 'count', 'targetelement'],
      distanceTriggerConditionKeys: ['threshold', 'equationop', 'distype', 'boundary', 'count', 'targetelement'],
      egoAttachLaneidCustomConditionKeys: ['variate', 'boundary', 'count', 'roadid', 'sectionid', 'laneid'],
      timeheadwayTriggerConditionKeys: ['threshold', 'equationop', 'unit', 'targetelement', 'boundary', 'count'],
      elementStateConditionKeys: ['element_type', 'element_ref', 'state'],
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
      deletedActions: new Set(), // 记录此次操作中被删除 action
      triggerConditionTypeList,
      comparisonOperatorList,
      distanceModeOptionList,
      conditionBoundaryList,
      speedTypeList,
      roadIdList,
      laneIdList,
      positionTypeList,
      storyBoardEleTypeList,
      changeTipInfoList,
      quitTipInfoList,
      saveTipInfoList,
      addTipInfoList,
      roadChangeFlag: 0,
    }
  },
  computed: {
    ...mapGetters('scenario', [
      'selectedObject',
    ]),
    ...mapState('scenario', ['roadObj', 'sceneevents', 'objects']),
    plannerEvents () {
      return this.isPlanner ? _.get(this, 'selectedObject.sceneevents', []) : []
    },
    eventId () {
      return this.selectedObject.eventId?.split(',') || []
    },
    currentTriggerId () {
      return this.currentTriggerInfo.id
    },
    formType () {
      return this.currentTriggerInfo ?
        this.currentTriggerInfo.conditions.map((condition) => {
          if (condition.type === 'reach_abs_position' || condition.type === 'reach_abs_lane') {
            return 'reach_position'
          }
          return condition.type
        }) :
          []
    },
    currentTriggerInfoIsDirty () {
      return !_.isEqual(this.originCurrentTriggerInfo, this.currentTriggerInfo)
    },
    isDirty () {
      return this.currentTriggerInfoIsDirty || this.isAdd || this.isDelete
    },
    newRoadIdList () {
      return _.keys(this.roadObj)
    },
    newLaneIdList () {
      // eslint-disable-next-line no-unused-expressions
      this.roadChangeFlag
      return (
        conditionIndex => this.currentTriggerInfo.conditions[conditionIndex].roadid ?
          this.roadObj[this.currentTriggerInfo.conditions[conditionIndex].roadid].laneIds :
            []
      )
    },
    newSectionIdList () {
      // eslint-disable-next-line no-unused-expressions
      this.roadChangeFlag
      return (
        conditionIndex => this.currentTriggerInfo.conditions[conditionIndex].roadid ?
          this.roadObj[this.currentTriggerInfo.conditions[conditionIndex].roadid].sectionIds :
            []
      )
    },
    formConditionHasUnfilledItem () {
      const validFn = (condition, key) => !(condition[key] || condition[key] === 0)
      return this.currentTriggerInfo.conditions.some((condition, conditionIndex) => {
        if (this.formType[conditionIndex] === 'reach_position') {
          return condition.positiontype ?
            this[`${_.camelCase(condition.positiontype)}ConditionKeys`].some((key) => {
              return validFn(condition, key)
            }) :
            true
        }
        return this[`${_.camelCase(this.formType[conditionIndex])}ConditionKeys`].some((key) => {
          return validFn(condition, key)
        })
      })
    },
    formActionHasUnfilledItem () {
      return this.currentTriggerInfo.action.some(({ actionvalue, subtype, checked, offset }) => {
        // 如果checked为字符串，则转换为布尔值
        checked = typeof checked === 'string' ? checked === 'true' : checked
        if (checked) {
          // 如果位置偏移为跨车道向左、向右，则检测actionvalue以及laneoffset
          // 如果位置偏移不为跨车道向左、向右，则只检测actionvalue
          if (['left', 'right'].includes(subtype)) {
            return !((actionvalue || actionvalue === 0) && (offset || offset === 0))
          } else {
            return !(actionvalue || actionvalue === 0)
          }
        } else return false
      })
    },
    newObjectList1 () {
      return this.objects.filter((o) => {
        if (o.id === this.selectedObject.id && o.type === this.selectedObject.type) {
          return false
        }
        return !['signlight', 'obstacle'].includes(o.type)
      })
        .map((o) => {
          return {
            value: o.type === 'planner' ? `ego_${String(o.id).padStart(3, '0')}` : `${o.type === 'car' ? 'v' : 'p'}_${o.id}`,
            label: $itemName(o),
          }
        })
    },
    storyBoardReferenceList () {
      const actionList = []

      // 故事面板引用规则： 主车只能引用主车的行为，交通车只能引用交通车的行为

      // 获取其他同类型(主车类型/非主车类型) object 中所有 trigger 的 action。
      this.objects.forEach((object) => {
        if (!(object.id === this.selectedObject.id && this.selectedObject.type === object.type)) {
          if (this.isPlanner ? object.type === 'planner' : object.type !== 'planner') {
            // 普通 object 的所有触发信息存储在 triggers 字段下, 主车的存储在 sceneevents 字段下
            const triggers = this.isPlanner ? object.sceneevents : object.triggers
            if (triggers) {
              triggers.forEach((trigger) => {
                trigger.action.forEach((action) => {
                  actionList.push({
                    value: `${trigger.id}_${action.actionid}`,
                    label: `${$itemName(object)}-${trigger.name}-${this.$t('scenario.behavior')}${Number(action.actionid) + 1}`,
                    actionType: action.actiontype,
                  })
                })
              })
            }
          }
        }
      })

      // 获取当前 object 中除当前 trigger 所有其他 trigger 的 action
      this.newTriggerList.forEach((trigger, triggerIndex) => {
        if (triggerIndex !== this.currentTriggerInfoIndex) {
          if (trigger.action.length) {
            trigger.action.forEach((action) => {
              const newAction = {
                value: `${trigger.id}_${action.actionid}`,
                label: `${this.selectedObject.label}-${trigger.name}-${this.$t('scenario.behavior')}${Number(action.actionid) + 1}`,
                actionType: action.actiontype,
              }
              const indexInActionList = actionList.findIndex(({ value }) => value === `${trigger.id}_${action.actionid}`)
              if (indexInActionList > -1) {
                actionList.splice(indexInActionList, 1, newAction)
              } else {
                actionList.push(newAction)
              }
            })
          }
        }
      })

      return actionList
    },
    storyBoardElementStateList () {
      return (elementRef) => {
        return this.storyBoardReferenceList.find(item => item.value === elementRef)?.actionType === 'velocity' ?
          storyBoardElementStateBase :
          storyBoardElementStateList
      }
    },
    isTrafficLight () {
      return _.get(this, 'selectedObject.type') === 'signlight'
    },
    isPlanner () {
      return _.get(this, 'selectedObject.type') === 'planner'
    },
    posincycleValue: {
      // 交通灯相位变化的值
      get () {
        return this.currentTriggerInfo.action[0]?.actionvalue || 0
      },
      set (val) {
        if (this.currentTriggerInfo.action[0]) {
          this.currentTriggerInfo.action[0].actionvalue = val
        } else {
          this.currentTriggerInfo.action.push({
            actiontype: 'posincycle',
            actionvalue: val,
          })
        }
      },
    },
  },
  async created () {
    this.loading = true
    await this.getModuleSetList()
    this.loading = false

    this.triggerTemp.conditions[0].targetelement = ''
    this.triggerTemp.conditions[0].sourceelement = this.selectedObject.type === 'planner' ?
      `ego_${this.selectedObject.id}` :
      `${this.selectedObject.type === 'car' ? 'v' : 'p'}_${this.selectedObject.id}`
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
    ...mapActions('module-set', ['getModuleSetList']),
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
      if (this.isPlanner) {
        this.newTriggerList = this.plannerEvents.slice()
        // 主车的 this.originSceneevents 只做增删触发时计算 Id 使用，实际不会更新
        this.originSceneevents = this.objects.filter(obj => obj.type === 'planner').reduce((acc, cur) => {
          if (cur.sceneevents) {
            acc.push(..._.cloneDeep(cur.sceneevents))
          }
          return acc
        }, [])
      } else {
        this.newTriggerList = this.sceneevents.filter(trigger => this.eventId.includes(String(trigger.id)))
        this.originSceneevents = _.cloneDeep(this.sceneevents)
      }

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
        const namePrefix = 'old_trigger'

        if (timeList) {
          this.trigger.timeList = timeList
          for (const t of timeList) {
            await this.handleAddTrigger(namePrefix)
            const type = 'time_trigger'
            // 重写解析老数据函数
            const action = []
            if (_.isNumber(t.velocity)) {
              action.push({
                actiontype: 'velocity',
                actionvalue: t.velocity,
              })
            }
            if (t.direction !== 'static') {
              // merge action
              action.push({
                actiontype: 'merge',
                subtype: oldDirectionToNewMergeSubtype[t.direction],
                actionvalue: t.durations,
                offset: t.offsets,
              })
            }

            if (t.acceleration !== 0) {
              // acc action
              action.push({
                actiontype: 'acc',
                actionvalue: t.acceleration,
                endconditiontype: '',
                endconditionvalue: '',
              })
            }

            if (t.accelerationTerminationType) {
              const act = action.find(a => a.actiontype === 'acc')
              if (act) {
                act.endconditiontype = t.accelerationTerminationType
                act.endconditionvalue = `${t.accelerationTerminationValue}`
              }
            }

            Object.assign(this.currentTriggerInfo, {
              conditions: [{ type, triggertime: `${t.time}` }],
              name: `${namePrefix}_${this.currentTriggerInfo.id + 1}`,
              action,
            })
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
            await this.handleAddTrigger(namePrefix)
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
            relativeConditions.targetelement = ''
            relativeConditions.count = t.times
            relativeConditions.boundary = 'none'
            const action = []
            if (_.isNumber(t.velocity)) {
              // velocity action
              action.push({
                actiontype: 'velocity',
                actionvalue: t.velocity,
              })
            }
            if (t.direction !== 'static') {
              // merge action
              action.push({
                actiontype: 'merge',
                subtype: oldDirectionToNewMergeSubtype[t.direction],
                actionvalue: t.durations,
                offset: t.offsets,
              })
            }

            if (t.acceleration !== 0) {
              // acc action
              action.push({
                actiontype: 'acc',
                actionvalue: t.acceleration,
                endconditiontype: '',
                endconditionvalue: '',
              })
            }

            if (t.accelerationTerminationType) {
              const act = action.find(a => a.actiontype === 'acc')
              if (act) {
                act.endconditiontype = t.accelerationTerminationType
                act.endconditionvalue = `${t.accelerationTerminationValue}`
              }
            }

            Object.assign(this.currentTriggerInfo, {
              conditions: [{ type, ...relativeConditions }],
              name: `${namePrefix}_${this.currentTriggerInfo.id + 1}`,
              action,
            })
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
      this.newTriggerList.forEach((trigger) => {
        trigger.conditions.forEach((condition, cIndex, cArr) => {
          // 将type为reach_abs_position 或者
          // reach_abs_lane的数据转换为type为reach_position和positiontype为absolute_position/absolute_lane
          if (condition.type === 'reach_abs_position') {
            cArr[cIndex].type = 'reach_abs_position'
            cArr[cIndex].positiontype = 'absolute_position'
          }
          if (condition.type === 'reach_abs_lane') {
            cArr[cIndex].type = 'reach_abs_lane'
            cArr[cIndex].positiontype = 'absolute_lane'
          }
          // 如果type为ego_attach_laneid_custom并且没有variate属性，那么variate始终为ego_laneid(目前后端无法返回variate参数，待后端修复)
          if (condition.type === 'ego_attach_laneid_custom' && !_.get(condition, 'variate')) {
            cArr[cIndex] = {
              ...cArr[cIndex],
              variate: 'ego_laneid',
            }
          }
          // 如果boundary为''，则改为none
          if (!condition.boundary) cArr[cIndex].boundary = 'none'
        })
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
      if (this.isPlanner) {
        const data = { ...this.selectedObject, ...params }
        const payload = {
          type: this.selectedObject.type,
          data: {
            ...data,
            sceneevents: this.newTriggerList || [],
          },
        }
        this.beforeModifyObject()
        try {
          editor.object.update(payload)
          this.updateObject(payload)
        } catch (e) {
          console.error(e)
        }
      } else {
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
        this.beforeModifyObject()
        try {
          editor.object.update(payload)
          this.updateObject(payload)
          this.updateSceneevents(this.originSceneevents)
        } catch (e) {
          console.error(e)
        }
      }
    },
    beforeModifyObject () {
      // 如果其他"同类型"（主车类型/非主车类型） object 内触发中的"故事板事件状态"
      // 触发的引用对象为该 object 内此次修改被删除触发的某个 action, 则置空。同时修改 originSceneevent 中对应 event 的 element_ref
      this.objects.forEach((object) => {
        if (Number(object.id) !== Number(this.selectedObject.id) && ((object.type === 'planner') === this.isPlanner)) {
          let ifUpdate = false
          // 普通 object 的所有触发信息存储在 triggers 字段下, 主车的存储在 sceneevents 字段下
          const triggerPath = this.isPlanner ? 'sceneevents' : 'triggers'
          const objTriggers = object[triggerPath] ? _.cloneDeep(object[triggerPath]) : []
          objTriggers.forEach((trigger) => {
            trigger.conditions && trigger.conditions.forEach((condition, cIndex, cArr) => {
              if (condition.type === 'element_state' && this.deletedActions.has(condition.element_ref)) {
                // 修改 this.originSceneevents 对应元素
                const orignSceneevent = this.originSceneevents.find(item => Number(item.id) === Number(trigger.id))
                const targetCondition = orignSceneevent &&
                  orignSceneevent.conditions.find(condition => this.deletedActions.has(condition.element_ref))
                if (targetCondition) {
                  targetCondition.element_ref = ''
                }

                // 修改 object.triggers 里对应元素
                cArr[cIndex].element_ref = ''
                ifUpdate = true
              }
            })
          })
          if (ifUpdate) {
            const _payload = {
              type: object.type,
              data: {
                ...object,
                [object.type === 'planner' ? 'sceneevents' : 'triggers']: objTriggers,
              },
            }
            editor.object.update(_payload)
            this.updateObject(_payload)
          }
        }
      })
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
      const deletedTrigger = this.newTriggerList.splice(index, 1)[0]
      const deletedActions = deletedTrigger.action.map(action => `${deletedTrigger.id}_${action.actionid}`)
      deletedActions.forEach((item) => {
        this.deletedActions.add(item)
      })

      this.originSceneevents.splice(this.originSceneevents.findIndex(item => item.id === id), 1)

      // 若被删除的 trigger 包含当前 object 中其他 trigger "故事板时间状态"触发中"故事板引用对象_事件_动作"的引用对象，则将包含它的引用置空
      this.newTriggerList.forEach((trigger) => {
        trigger.conditions.forEach((condition, cIndex, cArr) => {
          if (condition.type === 'element_state' && deletedActions.includes(condition.element_ref)) {
            cArr[cIndex].element_ref = ''
          }
        })
      })
      this.currentTriggerInfo.conditions.forEach((condition, cIndex, cArr) => {
        if (condition.type === 'element_state' && deletedActions.includes(condition.element_ref)) {
          cArr[cIndex].element_ref = ''
        }
      })

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
    async handleAddTrigger (namePrefix) {
      namePrefix = typeof namePrefix === 'string' ? namePrefix : 'new_trigger'

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
          conditions: [{ ...this.triggerTemp.conditions[0], type: 'time_trigger' }],
          action: [...this.triggerTemp.action],
          id,
          name: `${namePrefix}_${missIndex || Number(indexList.slice(-1)[0]) + 1}`,
        })
        this.originSceneevents.push({
          conditions: [{ ...this.triggerTemp.conditions[0], type: 'time_trigger' }],
          action: [...this.triggerTemp.action],
          id,
          name: `${namePrefix}_${missIndex || Number(indexList.slice(-1)[0]) + 1}`,
        })
        await this.handleChangeTrigger(this.newTriggerList.length - 1)
        if (this.isTrafficLight) {
          // this.formType[this.newTriggerList.length - 1] = 'reach_position'
          this.handleChangeType('reach_position', this.newTriggerList.length - 1)
          this.addBehavior()
          // 自动给交通灯的触发新建一个值为0的相位变化
          this.handleChangeBehavior(0, {
            actiontype: 'posincycle',
            actionvalue: 0,
          })
        }
        this.isAdd = true
      }
    },
    async handleChangeTrigger (index) {
      if (this.currentTriggerInfoIndex === index) {
        return
      }
      const res = await this.validateForm('change')
      if (!res) return
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
    handleChangeType (type, conditionIndex) {
      type = type === 'reach_position' ? 'reach_abs_position' : type

      let positiontype = ''
      if (type === 'reach_position') {
        positiontype = 'absolute_position'
      }

      this.currentTriggerInfo.conditions[conditionIndex] = {
        ...this.currentTriggerInfo.conditions[conditionIndex],
        ...this.triggerTemp.conditions[0],
        unit: type === 'timeheadway_trigger' ? 's' : this.triggerTemp.conditions[0].unit,
        lon: this.selectedObject.endPosArr && this.selectedObject.endPosArr.length ? this.selectedObject.endPosArr[0].lon : '0',
        lat: this.selectedObject.endPosArr && this.selectedObject.endPosArr.length ? this.selectedObject.endPosArr[0].lat : '0',
        alt: this.selectedObject.endPosArr && this.selectedObject.endPosArr.length ? this.selectedObject.endPosArr[0].alt : '0',
        type,
        positiontype,
      }
      this.isChangeType = true
    },
    handleChangePositionType (value, conditionIndex) {
      let { type } = this.currentTriggerInfo.conditions[conditionIndex]
      if (value === 'absolute_position') {
        type = 'reach_abs_position'
      } else if (value === 'absolute_lane') {
        type = 'reach_abs_lane'
      }

      // this.$set(this.currentTriggerInfo.conditions, conditionIndex, {
      //   ..._.cloneDeep(this.triggerTemp.conditions[0]),
      //   ...this.currentTriggerInfo.conditions[conditionIndex],
      //   lon: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].lon : '0',
      //   lat: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].lat : '0',
      //   alt: this.selectedObject.endPosArr ? this.selectedObject.endPosArr[0].alt : '0',
      //   lateraloffset: '0',
      //   longitudinaloffset: '0',
      //   tolerance: '0',
      //   radius: '5',
      //   type,
      // })
      this.currentTriggerInfo.conditions[conditionIndex] = {
        ..._.cloneDeep(this.triggerTemp.conditions[0]),
        ...this.currentTriggerInfo.conditions[conditionIndex],
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
    handleChangeElementRef (value, condition) {
      const actionType = this.storyBoardReferenceList.find(item => item.value === value)?.actionType
      if (actionType === 'velocity') {
        condition.state = 'completeState'
      }
    },
    async validateForm (type) {
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
          if (!this.currentTriggerInfo.action.length) {
            ElMessage.error(this.saveTipInfoList[3])
            return false
          } else {
            if (this.$refs.behaviorForm) {
              for (const behaviorForm of this.$refs.behaviorForm) {
                // 校验行为表单
                try {
                  await behaviorForm.validate()
                } catch (e) {
                  if (['save', 'add'].includes(type)) {
                    ElMessage.error(this[`${type}TipInfoList`][0])
                  } else {
                    ElMessage.error(this[`${type}TipInfoList`][4])
                  }
                  return false
                }
              }
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
    addBehavior () {
      // action 的 id 与 index 保持一致
      // const id = this.currentTriggerInfo.action.reduce((acc, cur) => {
      //   return Math.max(acc + 1, cur.id)
      // }, 0)
      const id = (() => {
        const ids = this.currentTriggerInfo.action.map(item => item.actionid)
        for (let i = 0; i < this.currentTriggerInfo.action.length + 1; i++) {
          if (!ids.includes(`${i}`)) {
            return `${i}`
          }
        }
      })()
      this.currentTriggerInfo.action.splice(Number(id), 0, {
        actionid: `${id}`,
        actiontype: '',
        actionvalue: '',
        subtype: '',
        multi: {
          isValid: false,
        },
        offset: '',
      })
    },
    removeBehavior (index) {
      const removeAction = this.currentTriggerInfo.action.splice(index, 1)[0]
      const removeActionRefValue = `${this.currentTriggerInfo.id}_${removeAction.actionid}`
      this.deletedActions.add(removeActionRefValue)

      // 若被删除的 action(behavior) 包含当前 object 中其他 trigger "故事板时间状态"触发中"故事板引用对象_事件_动作"的引用对象, 则将其置空
      this.newTriggerList.forEach((trigger) => {
        trigger.conditions.forEach((condition, cIndex, cArr) => {
          if (condition.type === 'element_state' && removeActionRefValue === condition.element_ref) {
            cArr[cIndex].element_ref = ''
          }
        })
      })
      this.currentTriggerInfo.conditions.forEach((condition, cIndex, cArr) => {
        if (condition.type === 'element_state' && removeActionRefValue === condition.element_ref) {
          cArr[cIndex].element_ref = ''
        }
      })
    },
    handleChangeBehavior (index, obj) {
      if (obj instanceof Event) return
      this.currentTriggerInfo.action.splice(index, 1, obj)
    },
    addCondition () {
      this.currentTriggerInfo.conditions.push({ ...this.triggerTemp.conditions[0], type: 'time_trigger' })
    },
    removeCondition (index) {
      this.currentTriggerInfo.conditions.splice(index, 1)
    },
    selectRoadChange () {
      this.roadChangeFlag += 1
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.car-trigger-config {
  width: 670px;
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
      //padding: 0 40px;

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

        .el-input {
          box-sizing: border-box;
          width: 100%;
        }
      }
    }

    .event-name-item {
      padding: 15px 40px 0;
    }

    .item-1-body {
      margin: 20px 60px 0 40px;

      &:not(:first-child) {
        margin-top: 40px;
      }

      .item-1-type-selector {
        position: relative;

        .condition-operators-container {
          position: absolute;
          right: 0;
          height: 100%;
          transform: translateX(100%);

          .condition-operator {
            padding: 2px;
            cursor: pointer;
            border: 1px dashed #979797;
            margin-right: 5px;

            &:nth-child(1) {
              margin-left: 7px;
            }

            &:hover {
              background: rgba(128, 128, 128, 0.4);
            }
          }
        }
      }
    }

    :deep(.el-checkbox__label) {
      color: #c2c2c2;
    }

    .add-new-behavior {
      display: block;
      margin: 20px auto 0;
      width: 224px;
      height: 32px;
      border-style: dashed;
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
