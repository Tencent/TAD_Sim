<template>
  <div class="planner-param-form">
    <el-form-item class="param-cell" label="车长">
      <InputNumber
        :model-value="data.boundingBox.dimensions.length"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'boundingBox.dimensions.length': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="车宽">
      <InputNumber
        :model-value="data.boundingBox.dimensions.width"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'boundingBox.dimensions.width': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="车高">
      <InputNumber
        :model-value="data.boundingBox.dimensions.height"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'boundingBox.dimensions.height': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell multi-input" style="display: block;" label="几何中心相当于车辆参考点偏移">
      <div class="form-multi-input">
        <div
          v-for="(value, key) of data.boundingBox.center"
          :key="key"
          class="input-cell"
        >
          <InputNumber
            :style="{ width: '56px' }"
            :model-value="value"
            unit="m"
            @update:model-value="updatePlannerParam({ index, [`boundingBox.center.${key}`]: $event })"
          />
          <span>{{ key }}</span>
        </div>
      </div>
    </el-form-item>
    <template v-if="index > 0">
      <el-form-item class="param-cell" label="参考点到车头参考点的X向距离">
        <InputNumber
          :model-value="data.boundingBox.trailerOffsetX"
          unit="m"
          @update:model-value="updatePlannerParam({ index, 'boundingBox.trailerOffsetX': $event })"
        />
      </el-form-item>
      <el-form-item class="param-cell" label="参考点到车头参考点的Z向距离">
        <InputNumber
          :model-value="data.boundingBox.trailerOffsetZ"
          unit="m"
          @update:model-value="updatePlannerParam({ index, 'boundingBox.trailerOffsetZ': $event })"
        />
      </el-form-item>
    </template>
    <el-form-item class="param-cell" label="前轴到参考点X向距离">
      <InputNumber
        :model-value="data.axles.frontAxle.positionX"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.frontAxle.positionX': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="后轴到参考点X向距离">
      <InputNumber
        :model-value="data.axles.rearAxle.positionX"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.rearAxle.positionX': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="前轮中心点距离">
      <InputNumber
        :model-value="data.axles.frontAxle.trackWidth"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.frontAxle.trackWidth': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="后轮中心点距离">
      <InputNumber
        :model-value="data.axles.rearAxle.trackWidth"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.rearAxle.trackWidth': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="前轴到参考点Z向距离">
      <InputNumber
        :model-value="data.axles.frontAxle.positionZ"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.frontAxle.positionZ': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="后轴到参考点Z向距离">
      <InputNumber
        :model-value="data.axles.rearAxle.positionZ"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.rearAxle.positionZ': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="车身重量">
      <InputNumber
        v-model="weight"
        unit="kg"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="前轮最大转向">
      <InputNumber
        :model-value="frontMaxSteering"
        unit="°"
        @update:model-value="updatePlannerParam({ index, 'axles.frontAxle.maxSteering': formatMaxSteering($event) })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="后轮最大转向">
      <InputNumber
        :model-value="rearMaxSteering"
        unit="°"
        @update:model-value="updatePlannerParam({ index, 'axles.rearAxle.maxSteering': formatMaxSteering($event) })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="前轮直径">
      <InputNumber
        :model-value="data.axles.frontAxle.wheelDiameter"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.frontAxle.wheelDiameter': $event })"
      />
    </el-form-item>
    <el-form-item class="param-cell" label="后轮直径">
      <InputNumber
        :model-value="data.axles.rearAxle.wheelDiameter"
        unit="m"
        @update:model-value="updatePlannerParam({ index, 'axles.rearAxle.wheelDiameter': $event })"
      />
    </el-form-item>
  </div>
</template>

<script>
import { mapMutations, mapState } from 'vuex'
import { MathUtils } from 'three'
import InputNumber from '@/components/input-number.vue'

export default {
  name: 'PlannerParamForm',
  components: { InputNumber },
  props: {
    data: {
      type: Object,
      required: true,
    },
    index: {
      type: Number,
      required: true,
    },
  },
  computed: {
    ...mapState('planners', ['currentPlanner']),
    frontMaxSteering () {
      return Math.round(this.data.axles.frontAxle.maxSteering * MathUtils.RAD2DEG).toFixed(2)
    },
    rearMaxSteering () {
      return Math.round(this.data.axles.rearAxle.maxSteering * MathUtils.RAD2DEG).toFixed(2)
    },
    weight: {
      get () {
        return this.data.properties.weight * 1000
      },
      set (val) {
        this.updatePlannerParam({ index: this.index, weight: val / 1000 })
      },
    },
  },
  methods: {
    ...mapMutations('planners', ['updatePlannerParam']),
    formatMaxSteering (value) {
      return value * MathUtils.DEG2RAD
    },
  },
}
</script>

<style scoped lang="less">
.planner-param-form {
  .el-form-item:not(.multi-input) {
    :deep(.el-form-item__label) {
      display: inline-block;
      width: 51%;
      height: initial;
    }

    :deep(.el-form-item__content) {
      width: 49%;
      float: left;

      .el-select {
        width: 100%;
      }

      .input-number {
        width: 100%;

        .el-input {
          width: 100%;
          padding: 0 5px;

          .el-input__suffix {
            margin-left: 3px;
            font-size: 11px;
          }
        }
      }
    }
  }

  .multi-input {
    :deep(.el-form-item__label) {
      width: 200px !important;
      margin-bottom: 8px;
    }

    .form-multi-input {
      grid-template-columns: repeat(3, 1fr);

      .input-cell {
        margin-bottom: 0;
      }
    }
  }
}
</style>
