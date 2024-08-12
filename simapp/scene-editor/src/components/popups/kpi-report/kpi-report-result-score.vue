<template>
  <span v-if="result && !hidden" class="job-report-result-score" :class="[type]">
    <!-- tag 样式 -->
    <el-tag
      v-if="type === 'plain'"
      :class="{ ...colorClass }"
    >
      {{ result.score }}
    </el-tag>
    <!-- 文本样式 -->
    <span
      v-else
      :class="{ ...colorClass }"
      :title="result.reason"
    >
      {{ result.score }}
    </span>
  </span>
</template>

<script>
import _ from 'lodash'

export default {
  props: {
    value: {
      type: Object,
      default: () => {},
    },
    type: {
      type: String,
      default: 'text', // text plain
    },
  },
  computed: {
    result () {
      return _.get(this.value, 'info.result')
    },
    hidden () {
      return this.result.score === undefined || this.result.score < 0
    },
    colorClass () {
      return {
        green: this.result.score >= 60,
        red: this.result.score < 60,
      }
    },
  },
}
</script>

<style lang="less" scoped>
@import "@/assets/less/mixins";

@green-bg: rgba(0, 250, 255, 0.2);
@green-color: #00faff;

@red-bg: rgba(247, 50, 34, 0.2);
@red-color: #F73222;

@yellow-bg: rgba(216, 150, 20, 0.2);
@yellow-color: #D89614;

.job-report-result-score {
  :deep(.green) {
    color: @green-color;
  }

  :deep(.red) {
    color: @red-color;
  }

  :deep(.yellow) {
    color: @yellow-color;
  }

  &.text {
    :deep(.yellow) {
      color: inherit;
    }
  }

  :deep(.el-tag) {
    min-width: 56px;
    text-align: center;
    height: 24px;
    line-height: 24px;
    border: none;

    &.green {
      background-color: @green-bg;
    }

    &.red {
      background-color: @red-bg;
    }

    &.yellow {
      background-color: @yellow-bg;
    }
  }
}
</style>
