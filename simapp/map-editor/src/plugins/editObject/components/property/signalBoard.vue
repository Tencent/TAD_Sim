<script setup lang="ts">
import { computed, ref, watch } from 'vue'
import { useObjectInteractionStore } from '../../store/interaction'
import { Constant } from '@/utils/business'
import InputNumber from '@/components/common/inputNumber.vue'
import { getPoleConfig } from '@/config/pole'
import { fixedPrecision } from '@/utils/common3d'
import { useObjectStore } from '@/stores/object'
import { getObject } from '@/utils/mapCache'

const interactionStore = useObjectInteractionStore()
const objectStore = useObjectStore()
const activeNames = ref(['position', 'rotation'])

const onVerticalPole = ref(false)
// 标志牌在杆上的局部坐标
// 高度和水平距离
const height = ref('')
const distance = ref('')
// 在横杆和竖杆上放置的最大最小值
const minHeight = ref(0)
const maxHeight = ref(0)
const minDistance = ref(0)
const maxDistance = ref(0)
// 如果在竖杆上，局部坐标系下的角度【朝着来车方向，角度为 0】
const angle = ref('')

const precision = Constant.precision

const currentSignalBoard = computed(() => {
  return interactionStore.currentSignalBoard
})

// 根据当前标志牌是否放置在竖杆上，调整变量状态
watch(
  currentSignalBoard,
  (val) => {
    if (!val) return

    if (val.onVerticalPole) {
      // 竖杆上
      onVerticalPole.value = true
      minHeight.value = 0
      const poleData = getObject(val.poleId)
      if (poleData) {
        const poleConfig = getPoleConfig(poleData.name)
        if (poleConfig) {
          maxHeight.value = poleConfig.style.height
        }
      }
      angle.value = String(fixedPrecision(val.angle))
    } else {
      // 横杆上
      const poleData = getObject(val.poleId)
      if (poleData) {
        const poleConfig = getPoleConfig(poleData.name)
        if (poleConfig) {
          const { style } = poleConfig
          onVerticalPole.value = false
          minDistance.value = style.vRadius
          maxDistance.value = style.length + style.vRadius
        }
      }
    }
    // 在杆上的水平和竖直距离，是基于标志牌在杆的局部坐标系下的坐标
    // y 指高度
    height.value = String(fixedPrecision(val.position.y))
    // z 指距离竖杆圆心的距离
    distance.value = String(fixedPrecision(val.position.z))
  },
  {
    immediate: true,
    deep: true,
  },
)

// 更新竖杆上的高度
watch(height, (val) => {
  if (!currentSignalBoard.value) return
  const { id: signalBoardId, position } = currentSignalBoard.value
  // 高度取 y
  const { y: originHeight } = position
  const _height = Number(val)
  if (fixedPrecision(originHeight, precision) === _height) return
  objectStore.moveSignalBoardByHeightAndDist({
    objectId: signalBoardId,
    height: _height,
  })
})

// 更新横杆上的水平距离
watch(distance, (val) => {
  if (!currentSignalBoard.value) return
  const { id: signalBoardId, position } = currentSignalBoard.value
  // 水平距离取 z
  const { z: originDistance } = position
  const _distance = Number(val)
  if (fixedPrecision(originDistance, precision) === _distance) return
  objectStore.moveSignalBoardByHeightAndDist({
    objectId: signalBoardId,
    distance: _distance,
  })
})

// 更新竖杆上标志牌的角度
watch(angle, (val) => {
  if (!currentSignalBoard.value) return
  const { id: signalBoardId, angle } = currentSignalBoard.value
  const _angle = Number(val)
  if (fixedPrecision(angle, precision) === _angle) return
  objectStore.rotateSignalBoard({
    objectId: signalBoardId,
    angle: _angle,
  })
})

// 快速切换朝向
function toggleDirection () {
  if (!currentSignalBoard.value) return
  const { id, position } = currentSignalBoard.value

  // positionX 如果大于0，说明物体处于正向，切换就需要调整为反向
  // positionX 如果小于0，说明物体处于反向，切换就需要调整为正向
  const currentIsForward = position.x > 0
  objectStore.toggleTrafficLightDirection({
    objectId: id,
    isForward: !currentIsForward,
  })
}
</script>

<template>
  <template v-if="currentSignalBoard">
    <div class="property-title">
      {{ $t('desc.editObject.currentSignalBoardProperty') }}
    </div>
    <div class="property-container">
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.signalBoardId') }}</label>
        <div class="content">
          {{ currentSignalBoard.id }}
        </div>
      </div>
      <div class="property-cell">
        <label class="label">{{ $t('desc.editObject.signalBoardName') }}</label>
        <div class="content">
          {{ currentSignalBoard.showName }}
        </div>
      </div>
      <el-form class="property-form" label-width="70" @submit.enter.prevent>
        <el-collapse v-model="activeNames">
          <el-collapse-item
            :title="$t('desc.commonRoad.positionOnPole')"
            name="position"
          >
            <el-form-item
              :label="$t('desc.editObject.verticalDistance')"
              class="param-cell"
            >
              <template v-if="onVerticalPole">
                <!-- 竖杆上的物体垂直高度，可修改 -->
                <InputNumber
                  v-model="height"
                  :disabled="false"
                  :precision="precision"
                  :min="minHeight"
                  :max="maxHeight"
                  unit="m"
                />
              </template>
              <template v-else>
                <!-- 横杆上的物体垂直高度，不可修改 -->
                <InputNumber
                  v-model="height"
                  :disabled="true"
                  :precision="precision"
                  unit="m"
                />
              </template>
            </el-form-item>
            <el-form-item
              :label="$t('desc.editObject.horizontalDistance')"
              class="param-cell"
            >
              <template v-if="onVerticalPole">
                <!-- 竖杆上的物体水平距离，不可修改 -->
                <InputNumber
                  v-model="distance"
                  :disabled="true"
                  :precision="precision"
                  unit="m"
                />
              </template>
              <template v-else>
                <!-- 横杆上的物体水平距离，不可修改 -->
                <InputNumber
                  v-model="distance"
                  :disabled="false"
                  :precision="precision"
                  :min="minDistance"
                  :max="maxDistance"
                  unit="m"
                />
              </template>
            </el-form-item>
          </el-collapse-item>
          <template v-if="onVerticalPole">
            <el-collapse-item
              :title="$t('desc.commonRoad.angleOnPole')"
              name="rotation"
            >
              <el-form-item
                :label="$t('desc.commonRoad.angle')"
                class="param-cell"
              >
                <InputNumber
                  v-model="angle"
                  :disabled="false"
                  :precision="precision"
                  :max="180"
                  :min="-180"
                  :angle="true"
                  unit="°"
                />
              </el-form-item>
            </el-collapse-item>
          </template>
        </el-collapse>
      </el-form>
      <!-- 只有位于横杆上的物体才会展示 -->
      <div v-if="!currentSignalBoard.onVerticalPole" class="property-cell">
        <el-button @click="toggleDirection">
          {{
            $t('actions.object.toggleOrientation')
          }}
        </el-button>
      </div>
    </div>
  </template>
</template>

<style scoped lang="less">
@import '@/assets/property.less';
</style>
