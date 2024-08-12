<script>
export default {
  name: 'BaseProgress',
  props: {
    percentage: {
      type: [Number, String],
      default: 0,
    },
  },
  computed: {
    progressWidth () {
      return `${this.percentage}%`
    },
  },
}
</script>

<template>
  <div class="progress">
    <div class="outer">
      <div class="inner" :style="{ width: percentage ? progressWidth : 0 }" />
    </div>
    <div class="progress-percentage">
      {{ (percentage || percentage === 0) ? percentage.toFixed(2) : 'N/A' }}%
    </div>
  </div>
</template>

<style scoped>
.progress {
  display: flex;
  align-items: center;
  width: 100%;
  height: 6px;
}
.progress .outer {
  width: 100%;
  height: 100%;
  border-radius: 5px;
  background: rgb(120, 120, 120);
}
.progress .inner {
  height: 100%;
  transition: 0.2s;
  border-radius: 5px;
  background: linear-gradient(
    90deg,
    rgb(61, 152, 243) 0%,
    rgb(86, 245, 250) 100%
  );
}
.progress .progress-percentage {
  width: 3em;
  margin-left: 5px;
  font-size: 12px;
}
</style>
