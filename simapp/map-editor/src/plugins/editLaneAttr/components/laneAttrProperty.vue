<script setup lang="ts">
/* global biz */
import { computed, ref, watch } from 'vue'
import { useLaneAttrInteractionStore } from '../store/interaction'
import { useRoadStore } from '@/stores/road'
import InputNumber from '@/components/common/inputNumber.vue'
import { LaneType, LaneTypeEnum } from '@/utils/business'
import i18n from '@/locales'

const speedLimit = ref('') // 限速
const laneType = ref(0)
const friction = ref('') // 摩擦力系数
const sOffset = ref('') // 材质纵向距离
const maxSOffset = ref(Number.POSITIVE_INFINITY) // 材质纵向距离最大值（受限于当前 section 的长度）
const roadStore = useRoadStore()
const laneAttrInteractionStore = useLaneAttrInteractionStore()

const laneTypeOptions = LaneType.map((type: biz.ILaneType) => ({
  value: LaneTypeEnum[type] as number,
  label: i18n.global.t(`desc.editLaneAttr.${type}`),
}))

const selectedRoad = computed(() => {
  return laneAttrInteractionStore.currentRoad
})
const selectedSection = computed(() => {
  return laneAttrInteractionStore.currentSection
})
const selectedLane = computed(() => {
  return laneAttrInteractionStore.currentLane
})

// 展示的车道属性，从选中的车道中实时获取
watch(
  selectedLane,
  (val) => {
    if (!val) return
    speedLimit.value = String(val.speedlimit)
    laneType.value = val.type
    // 车道材质的同步
    friction.value = String(val.friction)
    sOffset.value = String(val.sOffset)
  },
  {
    immediate: true,
    deep: true,
  },
)
watch(
  selectedSection,
  (val) => {
    if (!val) {
      maxSOffset.value = Number.POSITIVE_INFINITY
    } else {
      // 如果存在选中的 section 车道段，则限制最大的长度
      maxSOffset.value = val.length
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

// 监听限速的数值变化
watch(speedLimit, (val) => {
  if (!selectedLane.value || !selectedSection.value || !selectedRoad.value) {
    return
  }
  const { id: laneId, speedlimit } = selectedLane.value
  // 选中的车道存在，那么选中的道路和 section 应该也都存在
  const { id: roadId } = selectedRoad.value
  const { id: sectionId } = selectedSection.value
  const _speedLimit = Number(val)
  // 如果数值没变就不更新
  if (_speedLimit === speedlimit) return

  // 从数据状态层面更新车道限速
  roadStore.updateLaneAttr({
    roadId,
    sectionId,
    laneId,
    attrName: 'speedlimit',
    value: _speedLimit,
  })
})

// 监听车道类型的变化
watch(laneType, (val) => {
  if (!selectedLane.value || !selectedSection.value || !selectedRoad.value) {
    return
  }

  const { id: laneId, type } = selectedLane.value
  const { id: roadId } = selectedRoad.value
  const { id: sectionId } = selectedSection.value

  // 如果类型没变就不更新
  if (Number(val) === Number(type)) return

  // 从数据层面更新车道类型
  roadStore.updateLaneAttr({
    roadId,
    sectionId,
    laneId,
    attrName: 'type',
    value: Number(val),
  })
})

// 点击按钮触发数据层面删除当前车道逻辑
function removeLane () {
  const { roadId, sectionId, laneId } = laneAttrInteractionStore
  if (!roadId || !sectionId || !laneId) return

  roadStore.removeLane({
    roadId,
    sectionId,
    laneId,
  })
}

// 监听摩擦力系数的数值变化
watch(friction, (val) => {
  if (!selectedLane.value || !selectedSection.value || !selectedRoad.value) {
    return
  }
  const { id: laneId, friction } = selectedLane.value
  // 选中的车道存在，那么选中的道路和 section 应该也都存在
  const { id: roadId } = selectedRoad.value
  const { id: sectionId } = selectedSection.value
  const _friction = Number(val)
  // 如果数值没变就不更新
  if (_friction === friction) return

  // 从数据状态层面更新属性
  roadStore.updateLaneAttr({
    roadId,
    sectionId,
    laneId,
    attrName: 'friction',
    value: _friction,
  })
})

// 监听材质纵向距离的数值变化
watch(sOffset, (val) => {
  if (!selectedLane.value || !selectedSection.value || !selectedRoad.value) {
    return
  }
  const { id: laneId, sOffset } = selectedLane.value
  // 选中的车道存在，那么选中的道路和 section 应该也都存在
  const { id: roadId } = selectedRoad.value
  const { id: sectionId } = selectedSection.value
  const _sOffset = Number(val)
  // 如果数值没变就不更新
  if (_sOffset === sOffset) return

  // 从数据状态层面更新属性
  roadStore.updateLaneAttr({
    roadId,
    sectionId,
    laneId,
    attrName: 'sOffset',
    value: _sOffset,
  })
})
</script>

<template>
  <template v-if="selectedRoad && selectedSection">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentRoadProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.roadId') }}</label>
        <div class="content">
          {{ selectedRoad.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.sectionId') }}</label>
        <div class="content">
          {{ selectedSection.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.laneNumber') }}</label>
        <div class="content">
          {{ selectedSection.lanes.length }}
        </div>
      </div>
    </div>
  </template>
  <template v-if="selectedLane">
    <div class="property-title">
      {{ $t('desc.commonRoad.currentLaneProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.laneId') }}</label>
        <div class="content">
          {{ selectedLane.id }}
        </div>
      </div>
      <BaseAuth :perm="['action.mapEditor.lane.props.speedLimit.enable']">
        <div class="property-cell">
          <label class="label">{{ $t('desc.editLaneAttr.speedLimit') }}</label>
          <div class="content">
            <InputNumber
              v-model="speedLimit"
              :style="{ width: '142px' }"
              :disabled="false"
              :precision="0"
              :max="200"
              :min="0"
              unit="km/h"
            />
          </div>
        </div>
      </BaseAuth>
      <BaseAuth :perm="['action.mapEditor.lane.props.type.enable']">
        <div class="property-cell">
          <label class="label">{{ $t('desc.editLaneAttr.laneType') }}</label>
          <div class="content">
            <el-select
              v-model="laneType"
              placement="bottom"
              :style="{ width: '142px' }"
            >
              <el-option
                v-for="option in laneTypeOptions"
                :key="option.value"
                :label="option.label"
                :value="option.value"
              />
            </el-select>
          </div>
        </div>
      </BaseAuth>
      <BaseAuth :perm="['action.mapEditor.lane.props.friction.enable']">
        <div class="property-cell">
          <label class="label">{{ $t('desc.editLaneAttr.friction') }}</label>
          <div class="content">
            <InputNumber
              v-model="friction"
              :style="{ width: '142px' }"
              :disabled="false"
              :precision="2"
              :max="Infinity"
              :min="0.01"
            />
          </div>
        </div>
      </BaseAuth>
      <div class="property-cell">
        <label class="label">{{
          $t('desc.editLaneAttr.materialSOffset')
        }}</label>
        <div class="content">
          <InputNumber
            v-model="sOffset"
            :style="{ width: '142px' }"
            :disabled="false"
            :precision="3"
            :max="maxSOffset"
            :min="0"
            unit="m"
          />
        </div>
      </div>
      <div class="property-cell">
        <el-button @click="removeLane">
          {{
            $t('desc.editLaneNumber.removeCurrentLane')
          }}
        </el-button>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
