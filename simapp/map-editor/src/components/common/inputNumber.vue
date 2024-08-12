<script setup lang="ts">
import { type Ref, computed, ref, watch } from 'vue'
import NP from 'number-precision'
import { getValidDegreeInRange } from '@/utils/common3d'

const props = defineProps({
  modelValue: {
    type: [Number, String],
    default: 0,
  },
  disabled: {
    type: Boolean,
    default: false,
  },
  precision: {
    type: Number,
    default: 0,
  },
  unit: {
    type: String,
    default: '',
  },
  max: {
    type: Number,
    default: Number.POSITIVE_INFINITY,
  },
  min: {
    type: Number,
    default: Number.NEGATIVE_INFINITY,
  },
  // 是否是角度类型的输入值
  angle: {
    type: Boolean,
    default: false,
  },
})
const emit = defineEmits(['update:modelValue', 'exceed-max', 'exceed-min'])
// 监听属性中 modelValue 属性的变化
const propValue = ref(props.modelValue)
watch(
  () => props.modelValue,
  (newVal) => {
    propValue.value = newVal
  },
  {
    immediate: true,
  },
)

// 用户输入的内容
const userInput: Ref<undefined | string> = ref(undefined)

// 手动控制输入的内容
function handleInput (val: string) {
  userInput.value = val.trim()
}

// 手动触发更新
function handleChange (val: string) {
  // 判断输入的内容是否为空
  if (!val) {
    displayValue.value = propValue.value
    userInput.value = undefined
    return
  }

  // 判断输入的内容是否符合数字规范
  let tempVal
  const numberRegexp =
    /(^[+-]?[1-9]\d*$)|(^[+-]?[1-9]\d*\.\d+$)|(^[+-]?0$)|(^[+-]?0\.\d+$)/
  if (numberRegexp.test(val)) {
    tempVal = Number(val)
  } else {
    // 如果不是数字，则返回
    displayValue.value = propValue.value
    userInput.value = undefined
    return
  }

  // 如果输入的是角度类型的值，控制角度在 [-180, 180] 区间
  if (props.angle) {
    // 将输入的角度类型的值（角度制）控制在 [-180, 180] 区间
    const _tempVal = getValidDegreeInRange(tempVal)

    tempVal = _tempVal
  }

  // 判断输入的内容是否在最大和最小区间中
  if (tempVal > props.max) {
    emit('exceed-max')
    tempVal = props.max
  }

  if (tempVal < props.min) {
    emit('exceed-min')
    tempVal = props.min
  }

  // tempVal = Math.min(Math.max(tempVal, props.min), props.max)

  // 对符合规范的数据进行位数的限制
  tempVal = NP.round(tempVal, props.precision)

  // 转换成跟原始变量类型一致的类型
  tempVal = String(tempVal)

  // 将符合规范的输入值派发
  emit('update:modelValue', tempVal)
  // 在触发更新后，重置 userInput 的值
  userInput.value = undefined
}

// 在 input 输入框中展示的内容
const displayValue = computed({
  // 只使用 getter
  get () {
    if (userInput.value !== undefined) {
      return userInput.value
    }
    return propValue.value
  },
  set () {},
})
</script>

<template>
  <div
    class="input-number"
    :class="{
      disabled: props.disabled,
    }"
  >
    <el-input
      v-model="displayValue"
      :disabled="props.disabled"
      @input="handleInput"
      @change="handleChange"
    >
      <template #suffix>
        {{ unit }}
      </template>
    </el-input>
  </div>
</template>

<style scoped lang="less">
.input-number {
  display: inline-block;
  box-sizing: border-box;
  vertical-align: top;
  width: 100px;

  &.disabled {
    :deep(.el-input__wrapper) {
      background-color: var(--input-disabled-background);
      border-color: var(--input-disabled-border-color);
    }
  }

  :deep(.el-input__wrapper) {
    border-radius: 0;
    background-color: var(--input-background);
    border: var(--input-border);
    box-shadow: none;
    box-sizing: border-box;
    padding: 0 6px;
    height: 30px;

    .el-input__inner {
      color: var(--text-color);
    }
  }
}
</style>
