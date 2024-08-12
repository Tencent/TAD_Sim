<script setup lang="ts">
import type { CarModel } from 'models-manager/src/catalogs/class.ts'
import { computed } from 'vue'
import { MathUtils } from 'three'
import InputNumber from '@/components/input-number.vue'

defineOptions({
  name: 'CarForm',
})

const props = defineProps<{
  modelValue: CarModel
}>()

const emits = defineEmits(['change'])

const frontMaxSteering = computed(() => {
  return ((props.modelValue.catalogParams[0].axles?.frontAxle.maxSteering || 0) * MathUtils.DEG2RAD).toFixed(3)
})
const rearMaxSteering = computed(() => {
  return ((props.modelValue.catalogParams[0].axles?.rearAxle.maxSteering || 0) * MathUtils.DEG2RAD).toFixed(3)
})

function updateMaxSteering (which: 'front' | 'rear', value: number) {
  if (which === 'front') {
    emits('change', 'catalogParams[0].axles.frontAxle.maxSteering', value * MathUtils.RAD2DEG)
  } else if (which === 'rear') {
    emits('change', 'catalogParams[0].axles.rearAxle.maxSteering', value * MathUtils.RAD2DEG)
  }
}
</script>

<template>
  <div class="model-prop-wrap">
    <el-form
      :model="modelValue"
      class="model-view model-view-car"
      label-width="70"
      label-suffix=": "
      inline
    >
      <h4>模型参数</h4>
      <div class="multi-input-wrap">
        <div class="param-cell multi-input">
          <label class="form-label">* 原点坐标</label>
          <div class="form-multi-input">
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams[0].boundingBox.center.x"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams[0].boundingBox.center.x', $event)"
              />
              <span class="input-label">X</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams[0].boundingBox.center.y"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams[0].boundingBox.center.y', $event)"
              />
              <span class="input-label">Y</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams[0].boundingBox.center.z"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams[0].boundingBox.center.z', $event)"
              />
              <span class="input-label">Z</span>
            </div>
          </div>
        </div>
      </div>
      <div class="multi-input-wrap">
        <div class="param-cell multi-input">
          <label class="form-label">* 尺寸参数</label>
          <div class="form-multi-input">
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams[0].boundingBox.dimensions.length"
                :min="0"
                :max="10000"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams[0].boundingBox.dimensions.length', $event)"
              />
              <span class="input-label">长</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams[0].boundingBox.dimensions.width"
                :min="0"
                :max="10000"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams[0].boundingBox.dimensions.width', $event)"
              />
              <span class="input-label">宽</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams[0].boundingBox.dimensions.height"
                :min="0"
                :max="10000"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams[0].boundingBox.dimensions.height', $event)"
              />
              <span class="input-label">高</span>
            </div>
          </div>
        </div>
      </div>

      <template v-if="modelValue.catalogParams[0].axles">
        <h4>前轴</h4>
        <el-form-item label="最大转角" prop="catalogParams[0].axles.frontAxle.maxSteering">
          <InputNumber
            :model-value="frontMaxSteering"
            :min="0"
            :max="360"
            unit="°"
            @update:model-value="updateMaxSteering('front', $event)"
          />
        </el-form-item>
        <el-form-item label="轮径" prop="catalogParams[0].axles.frontAxle.wheelDiameter">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.frontAxle.wheelDiameter"
            :min="0"
            :max="10000"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.frontAxle.wheelDiameter', $event)"
          />
        </el-form-item>
        <el-form-item label="轴宽" prop="catalogParams[0].axles.frontAxle.trackWidth">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.frontAxle.trackWidth"
            :min="0"
            :max="10000"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.frontAxle.trackWidth', $event)"
          />
        </el-form-item>
        <el-form-item label="X轴偏移" prop="catalogParams[0].axles.frontAxle.positionX">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.frontAxle.positionX"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.frontAxle.positionX', $event)"
          />
        </el-form-item>
        <el-form-item label="Z轴偏移" prop="catalogParams[0].axles.frontAxle.positionZ">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.frontAxle.positionZ"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.frontAxle.positionZ', $event)"
          />
        </el-form-item>
        <h4>后轴</h4>
        <el-form-item label="最大转角" prop="catalogParams[0].axles.rearAxle.maxSteering">
          <InputNumber
            :model-value="rearMaxSteering"
            :min="0"
            :max="360"
            unit="°"
            @update:model-value="updateMaxSteering('rear', $event)"
          />
        </el-form-item>
        <el-form-item label="轮径" prop="catalogParams[0].axles.rearAxle.wheelDiameter">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.rearAxle.wheelDiameter"
            :min="0"
            :max="10000"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.rearAxle.wheelDiameter', $event)"
          />
        </el-form-item>
        <el-form-item label="轴宽" prop="catalogParams[0].axles.rearAxle.trackWidth">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.rearAxle.trackWidth"
            :min="0"
            :max="10000"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.rearAxle.trackWidth', $event)"
          />
        </el-form-item>
        <el-form-item label="X轴偏移" prop="catalogParams[0].axles.rearAxle.positionX">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.rearAxle.positionX"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.rearAxle.positionX', $event)"
          />
        </el-form-item>
        <el-form-item label="Z轴偏移" prop="catalogParams[0].axles.rearAxle.positionZ">
          <InputNumber
            :model-value="modelValue.catalogParams[0].axles.rearAxle.positionZ"
            unit="m"
            @update:model-value="$emit('change', 'catalogParams[0].axles.rearAxle.positionZ', $event)"
          />
        </el-form-item>
      </template>
    </el-form>
  </div>
</template>

<style scoped lang="less">
.model-prop-wrap {
  height: 240px;
  overflow: auto;

  h4 {
    font-size: 14px;
    font-weight: normal;
    line-height: 22px;
    color: #fff;
    margin-bottom: 10px;
  }
}

.param-cell {
  display: flex;
  justify-content: space-between;
  align-items: center;

  .el-radio {
    margin-right: 16px;

    &:last-child {
      margin-right: 0;
    }

    :deep(.el-radio__label) {
      padding-left: 5px;
    }
  }

  .el-select {
    width: 80px;

    :deep(.el-input--suffix) {
      .el-input__inner {
        height: 24px;
        line-height: 22px;
        border-radius: 2px;
        padding-left: 6px;
      }

      .el-input__icon {
        line-height: 24px;
      }
    }
  }

  &.multi-input {
    display: block;

    .form-label {
      display: block;
      margin-bottom: 8px;
      color: #c2c2c2;
    }
  }

  .form-multi-input {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    grid-column-gap: 10px;

    .input-cell {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin-bottom: 10px;

      > :deep(.input-number) {
        width: 100%;
      }

      .input-label {
        line-height: 22px;
        color: #fff;
      }
    }
  }
}

.multi-input-wrap {
  .multi-input .input-cell {
    margin-bottom: 0;
  }
}

.model-view-car {
  .el-input-number {
    width: 70px;

    &.is-without-controls .el-input__wrapper {
      padding: 1px 11px;
    }

    .el-input__inner {
      text-align: left;
    }
  }

  .el-form-item {
    margin-right: 8px;
  }
}
</style>
