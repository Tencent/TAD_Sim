<script setup lang="ts">
import type { ObstacleModel, PedestrianModel } from 'models-manager/src/catalogs/class.ts'
import InputNumber from '@/components/input-number.vue'

defineOptions({
  name: 'PedestrianForm',
})

defineProps<{
  modelValue: PedestrianLikeModel
}>()

defineEmits(['change'])

type PedestrianLikeModel = PedestrianModel | ObstacleModel
</script>

<template>
  <div class="model-prop-wrap">
    <el-form
      :model="modelValue"
      class="model-view model-view-pedestrian"
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
                :model-value="modelValue.catalogParams.boundingBox.center.x"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams.boundingBox.center.x', $event)"
              />
              <span class="input-label">X</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams.boundingBox.center.y"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams.boundingBox.center.y', $event)"
              />
              <span class="input-label">Y</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams.boundingBox.center.z"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams.boundingBox.center.z', $event)"
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
                :model-value="modelValue.catalogParams.boundingBox.dimensions.length"
                :min="0"
                :max="10000"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams.boundingBox.dimensions.length', $event)"
              />
              <span class="input-label">长</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams.boundingBox.dimensions.width"
                :min="0"
                :max="10000"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams.boundingBox.dimensions.width', $event)"
              />
              <span class="input-label">宽</span>
            </div>
            <div class="input-cell">
              <InputNumber
                :model-value="modelValue.catalogParams.boundingBox.dimensions.height"
                :min="0"
                :max="10000"
                unit="m"
                @update:model-value="$emit('change', 'catalogParams.boundingBox.dimensions.height', $event)"
              />
              <span class="input-label">高</span>
            </div>
          </div>
        </div>
      </div>
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

.model-view-pedestrian {
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
