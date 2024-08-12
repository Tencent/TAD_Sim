<script setup lang="ts">
import { delay } from 'lodash'
import { type Ref, ref, watch } from 'vue'
import {
  type ITrafficLightConfig,
  getTrafficLightConfigByName,
  trafficLightConfig,
} from '@/config/trafficLight'
// @ts-expect-error
import ArrowSvg from '@/assets/icons/arrow.svg?component'

const props = defineProps({
  lightName: {
    type: String,
    default: '',
  },
})

const emit = defineEmits(['typeChange'])
// 弹框的可见性
const visible = ref(false)
// 当前选中的信号灯类型
const currentLight: Ref<ITrafficLightConfig | null> = ref(null)
watch(
  () => props.lightName,
  (val) => {
    if (val) {
      const lightOption = getTrafficLightConfigByName(val)
      if (lightOption) {
        currentLight.value = lightOption
        return
      }
    }

    currentLight.value = null
  },
  // 立即将更新的属性，同步到样式中
  { immediate: true },
)

function pickType (typeName: string) {
  emit('type-change', typeName)
  visible.value = false
}

function onBlur () {
  delay(() => {
    visible.value = false
  }, 100)
}
</script>

<template>
  <template v-if="currentLight">
    <el-popover
      popper-class="light-type-picker"
      trigger="click"
      placement="left"
      :visible="visible"
    >
      <ul class="type-ul">
        <li
          v-for="lightOption of trafficLightConfig"
          :key="lightOption.name"
          class="type-li"
          :class="{ active: lightOption.name === currentLight.name }"
          @click="pickType(lightOption.name)"
        >
          <div class="icon-box">
            <img :src="lightOption.iconUrl" class="icon">
          </div>
          <span class="title">{{ lightOption.showName }}</span>
        </li>
      </ul>
      <template #reference>
        <button class="current-type" @click="visible = true" @blur="onBlur">
          <div class="left-part">
            <div class="icon-box">
              <img :src="currentLight.iconUrl" class="icon">
            </div>
            <span class="title">{{ currentLight.showName }}</span>
          </div>
          <div class="right-part">
            <ArrowSvg class="arrow-icon" />
          </div>
        </button>
      </template>
    </el-popover>
  </template>
</template>

<style scoped lang="less">
.current-type {
  padding: 0;
  outline: none;
  border-radius: 0;
  border: none;
  display: flex;
  align-items: center;
  flex-wrap: wrap;
  cursor: pointer;

  .left-part {
    display: flex;
    flex-direction: column;
    align-items: center;
    background-color: var(--component-area-color);
    padding: 0 4px 4px;
    .icon-box {
      width: 60px;
      height: 60px;
      border: 2px solid transparent;
      box-sizing: border-box;
      .icon {
        width: 56px;
        height: 56px;
      }
    }
    .title {
      color: var(--text-color);
    }
  }
  .right-part {
    width: 12px;
    height: 100%;
    background-color: var(--object-item-background);
    color: var(--property-title-color);
    display: flex;
    align-items: center;
  }
}
</style>
