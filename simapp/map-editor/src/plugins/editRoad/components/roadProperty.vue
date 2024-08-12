<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useEditRoadStore } from '../store'
import { useRoadInteractionStore } from '../store/interaction'
import { useRoadStore } from '@/stores/road'
import InputNumber from '@/components/common/inputNumber.vue'
import { fixedPrecision } from '@/utils/common3d'

const roadStore = useRoadStore()
const editRoadStore = useEditRoadStore()
const roadInteractionStore = useRoadInteractionStore()

// 隧道的起始点和长度
const tunnelS = ref('')
const tunnelLength = ref('')
const precision = 3

const selectedSection = computed(() => {
  const { currentRoadId, sectionId } = roadInteractionStore
  const querySectionRes = roadStore.getSectionById(currentRoadId, sectionId)
  if (!querySectionRes) return null
  return querySectionRes.section
})

const selectedRoad = computed(() => {
  const { currentRoadId } = roadInteractionStore
  if (currentRoadId) {
    const road = roadStore.getRoadById(currentRoadId)
    if (!road) return null
    return road
  }

  // 选中了道路参考线，展示控制的正向道路信息
  if (roadInteractionStore.refLineCpId) {
    const cp = editRoadStore.getControlPointById(
      roadInteractionStore.refLineCpId,
    )
    if (cp) {
      const [forwardRoadId] = cp.roadId
      const road = roadStore.getRoadById(forwardRoadId)
      if (road) {
        return road
      }
    }
  }

  // 选中了参考线控制点，展示控制点的正向道路信息
  if (roadInteractionStore.refPoint) {
    const cp = editRoadStore.getControlPointById(
      roadInteractionStore.refPoint.parentId,
    )
    if (cp) {
      const [forwardRoadId] = cp.roadId
      const road = roadStore.getRoadById(forwardRoadId)
      if (road) {
        return road
      }
    }
  }

  return null
})
// 当前道路连通的路口 id
const linkJunctionIds = computed(() => {
  if (!selectedRoad.value) return ''
  const ids = selectedRoad.value.linkJunction.join(', ')
  return ids
})

// 监听当前选中的道路，更新属性面板的状态
watch(
  () => selectedRoad.value,
  (val) => {
    if (!val) {
      tunnelS.value = ''
      tunnelLength.value = ''
      return
    }

    // 如果是隧道
    if (val.roadType === 'tunnel') {
      const road = val as biz.ITunnel
      tunnelS.value = String(road.tunnelS)
      tunnelLength.value = String(road.tunnelLength)
    }
  },
  {
    immediate: true,
    deep: true,
  },
)

watch(tunnelS, (val) => {
  if (!selectedRoad.value) return
  if (selectedRoad.value.roadType !== 'tunnel') return

  const {
    tunnelS: lastS,
    tunnelLength: lastLength,
    id: roadId,
    length: roadLength,
  } = selectedRoad.value as biz.ITunnel

  const _tunnelS = Number(val)
  // 如果数值没变，则不更新
  if (_tunnelS === fixedPrecision(lastS, precision)) return

  // 理想情况，长度不变，移动s坐标
  // 如果原有长度+起始点坐标超过道路总长度，则隧道长度需要动态调整
  let _tunnelLength = lastLength
  if (_tunnelS + _tunnelLength > roadLength) {
    _tunnelLength = roadLength - _tunnelS
  }

  // 触发数据层的更新
  roadStore.updateTunnelAttr({
    roadId,
    s: _tunnelS,
    length: _tunnelLength,
    needRedraw: true,
    saveRecord: true,
  })
})
watch(tunnelLength, (val) => {
  if (!selectedRoad.value) return
  if (selectedRoad.value.roadType !== 'tunnel') return

  const {
    tunnelS: lastS,
    tunnelLength: lastLength,
    id: roadId,
    length: roadLength,
  } = selectedRoad.value as biz.ITunnel

  let _tunnelLength = Number(val)
  if (_tunnelLength === fixedPrecision(lastLength, precision)) return

  // 起始点始终保持不动，如果隧道起始坐标+当前长度超过道路总长度，则隧道长度需要动态调整
  if (lastS + _tunnelLength > roadLength) {
    _tunnelLength = roadLength - lastS
  }

  // 触发数据层的更新
  roadStore.updateTunnelAttr({
    roadId,
    s: lastS,
    length: _tunnelLength,
    needRedraw: true,
    saveRecord: true,
  })
})
</script>

<template>
  <template v-if="selectedRoad">
    <!-- 偏展示信息类，没有表单输入的，使用自定义的属性列表结构 -->
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
        <label class="label">{{ $t('desc.commonRoad.roadLength') }}</label>
        <div class="content">
          {{ selectedRoad.length }} m
        </div>
      </div>
      <template v-if="selectedSection">
        <div class="property-cell">
          <label class="label">{{ $t('desc.commonRoad.sectionId') }}</label>
          <div class="content">
            {{ selectedSection.id }}
          </div>
        </div>
        <div class="property-cell">
          <label class="label">{{ $t('desc.commonRoad.sectionLength') }}</label>
          <div class="content">
            {{ selectedSection.length }} m
          </div>
        </div>
      </template>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editRoad.linkJunctionId') }}</label>
        <div class="content">
          {{ linkJunctionIds }}
        </div>
      </div>
      <!-- 如果是隧道，展示隧道相关的属性 -->
      <template v-if="selectedRoad.roadType === 'tunnel'">
        <div class="property-cell">
          <label class="label">{{
            $t('desc.editRoad.tunnelSCoordinate')
          }}</label>
          <div class="content">
            <InputNumber
              v-model="tunnelS"
              :disabled="false"
              :precision="precision"
              :max="selectedRoad.length"
              :min="0"
              unit="m"
            />
          </div>
        </div>
        <div class="property-cell">
          <label class="label">{{ $t('desc.editRoad.tunnelLength') }}</label>
          <div class="content">
            <InputNumber
              v-model="tunnelLength"
              :disabled="false"
              :precision="precision"
              :max="selectedRoad.length"
              :min="0"
              unit="m"
            />
          </div>
        </div>
      </template>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
