<script setup lang="ts">
/* global biz */
import { type Ref, ref, watch } from 'vue'
import { useBoundaryInteractionStore } from '../store/interaction'
import { useRoadStore } from '@/stores/road'
import { getBoundaryById } from '@/stores/road/boundary'
import {
  type IBoundaryShapeType,
  LaneBoundaryMark,
  LaneBoundaryType,
  parseLaneBoundaryMark,
} from '@/utils/business'
import i18n from '@/locales'
import ColorPicker from '@/components/common/colorPicker.vue'
import { DefaultColor } from '@/utils/common3d'

const markType = ref('')
const markColor = ref('')
const selectedRoad: Ref<biz.IRoad | null> = ref(null)
const selectedSection: Ref<biz.ISection | null> = ref(null)
const selectedBoundary: Ref<biz.ILaneBoundary | null> = ref(null)

const roadStore = useRoadStore()
const boundaryInteractionStore = useBoundaryInteractionStore()

watch(
  () => boundaryInteractionStore,
  (val) => {
    const { roadId, sectionId, boundaryId } = val
    if (!roadId || !sectionId || !boundaryId) {
      selectedRoad.value = null
      selectedSection.value = null
      selectedBoundary.value = null
      return
    }
    const querySectionRes = roadStore.getSectionById(roadId, sectionId)
    if (!querySectionRes) return
    selectedRoad.value = querySectionRes.road
    selectedSection.value = querySectionRes.section

    const boundary = getBoundaryById(
      querySectionRes.section.boundarys,
      boundaryId,
    )
    if (!boundary) return
    selectedBoundary.value = boundary
  },
  {
    immediate: true,
    deep: true,
  },
)

const boundaryType = LaneBoundaryType.map(type => ({
  value: type,
  label: i18n.global.t(`desc.editBoundary.${type}`),
}))

// 基于当前选中的边界实时计算对应的车道边界线样式参数
watch(
  selectedBoundary,
  (val) => {
    if (!val) return
    if (val.mark === 0) {
      // 无边界
      markColor.value = 'White'
      markType.value = 'None'
      return
    }
    const { color, lineStyle } = parseLaneBoundaryMark(val.mark)
    markColor.value = color.name
    if (lineStyle.length > 1) {
      // 双线
      const [firstStyle, secondStyle] = lineStyle
      if (firstStyle === secondStyle) {
        if (firstStyle === 'dash') {
          markType.value = 'BrokenBroken'
        } else {
          markType.value = 'SolidSolid'
        }
      } else {
        if (firstStyle === 'dash') {
          markType.value = 'BrokenSolid'
        } else {
          markType.value = 'SolidBroken'
        }
      }
    } else {
      // 单线
      const [firstStyle] = lineStyle
      if (firstStyle === 'dash') {
        markType.value = 'Broken'
      } else {
        markType.value = 'Solid'
      }
    }
  },
  {
    deep: true,
  },
)

// 监听车道线【线形】的变化
watch(markType, (val) => {
  if (!val) return

  // 计算出新的边界线 mark 值
  // 颜色的位值
  let colorBit = 0
  switch (markColor.value) {
    case 'Yellow':
    case 'Red':
    case 'Green':
    case 'Blue': {
      colorBit = LaneBoundaryMark[markColor.value]
      break
    }
    // 不单独针对两条黄色标线做特殊处理
    // case 'Yellow': {
    //   if (
    //     val === 'SolidSolid' ||
    //     val === 'BrokenBroken' ||
    //     val === 'SolidBroken' ||
    //     val === 'BrokenSolid'
    //   ) {
    //     // 如果线形是双线，则黄色需要更换另一个标识
    //     colorBit = LaneBoundaryMark['YellowYellow']
    //   } else {
    //     colorBit = LaneBoundaryMark[markColor.value]
    //   }
    //   break
    // }
    default:
      break
  }

  // 线形的位值
  const typeBit = val === 'None' ? 0 : LaneBoundaryMark[val as IBoundaryShapeType]

  // 如果是无边界，则不考虑颜色
  const markBit = typeBit ? colorBit + typeBit : typeBit

  const { roadId, sectionId, boundaryId } = boundaryInteractionStore
  if (!roadId || !sectionId || !boundaryId) return
  roadStore.updateLaneBoundaryMark({
    roadId,
    sectionId,
    boundaryId,
    mark: markBit,
  })
})

function updateBoundaryColor (val: string) {
  if (!val || val === markColor.value) return
  // 更新当前展示的颜色
  markColor.value = val
  let colorBit = 0
  const currentType = markType.value as IBoundaryShapeType
  switch (val) {
    case 'Yellow':
    case 'Red':
    case 'Green':
    case 'Blue': {
      colorBit = LaneBoundaryMark[val]
      break
    }
    // 不单独针对两条黄色的标线做特殊处理
    // case 'Yellow': {
    //   if (
    //     currentType === 'SolidSolid' ||
    //     currentType === 'BrokenBroken' ||
    //     currentType === 'SolidBroken' ||
    //     currentType === 'BrokenSolid'
    //   ) {
    //     // 如果线形是双线，则黄色需要更换另一个标识
    //     colorBit = LaneBoundaryMark['YellowYellow']
    //   } else {
    //     colorBit = LaneBoundaryMark[val]
    //   }
    //   break
    // }
    default:
      break
  }
  const markBit = colorBit + LaneBoundaryMark[currentType]
  const { roadId, sectionId, boundaryId } = boundaryInteractionStore
  if (!roadId || !sectionId || !boundaryId) return
  roadStore.updateLaneBoundaryMark({
    roadId,
    sectionId,
    boundaryId,
    mark: markBit,
  })
}
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
  <template v-if="selectedBoundary">
    <div class="property-title">
      {{ $t('desc.editBoundary.currentLaneBoundaryProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editBoundary.boundaryId') }}</label>
        <div class="content">
          {{ selectedBoundary.id }}
        </div>
      </div>
      <BaseAuth :perm="['action.mapEditor.boundary.props.type.enable']">
        <div class="property-cell">
          <label class="label">{{ $t('desc.editBoundary.type') }}</label>
          <div class="content">
            <el-select v-model="markType" placement="bottom">
              <el-option
                v-for="(type, index) in boundaryType"
                :key="index"
                :label="type.label"
                :value="type.value"
              />
            </el-select>
          </div>
        </div>
      </BaseAuth>
      <!-- 如果线形为无边界，则不显示颜色选择项 -->
      <div v-if="markType !== 'None'" class="property-cell">
        <label class="label">{{ $t('desc.commonRoad.color') }}</label>
        <div class="content">
          <ColorPicker
            :display-color="markColor"
            :predefine="DefaultColor"
            @color-change="updateBoundaryColor"
          />
        </div>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
