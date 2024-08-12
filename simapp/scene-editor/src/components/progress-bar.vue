<template>
  <div v-show="progressInfo.status" id="progress-bar">
    <h5>{{ titleText }}</h5>
    <div class="progress" style="position: relative">
      <span id="start" class="part">
        <el-progress :percentage="progressInfo.progress" />
      </span>
      <span id="end" class="part">
        <el-icon class="el-icon-error" @mouseup="onClose"><Close /></el-icon>
      </span>
    </div>
  </div>
</template>

<script>
import { mapActions, mapState } from 'vuex'

let taskId = null

const titleMap = {
  0: 'tips.SceneImporting',
  1: 'tips.SceneExporting',
  10: 'tips.MapImporting',
  11: 'tips.MapExporting',
  20: 'tips.SceneGenerating',
  30: 'tips.SceneGenerating',
}

export default {
  name: 'ProgressBar',

  data () {
    return {
      title: 'tips.SceneImporting',
    }
  },

  computed: {
    ...mapState('progressInfo', [
      'progressInfo',
    ]),

    titleText () {
      return this.$t(titleMap[this.progressInfo.mode])
    },
  },

  watch: {
    'progressInfo.status': function (newVal, oldVal) {
      if (newVal) {
        this.onStart()
      } else {
        this.onStop()
      }
    },
  },

  methods: {
    ...mapActions('progressInfo', [
      'queryProgressInfo',
      'stopProgress',
    ]),
    // 轮询进度
    onStart () {
      const step = 1000
      taskId = setInterval(() => {
        this.queryProgressInfo()
      }, step)
    },

    onStop () {
      clearInterval(taskId)
      taskId = null
    },

    onClose () {
      this.stopProgress()
    },
  },
}
</script>

<style lang="less">
@import "@/assets/less/mixins";

  #progress-bar {
    position: absolute;
    top: 30px;
    left: 50%;
    transform: translate(-50%, 0);
    z-index: 4000;
    color: @global-font-color;
    background-color: @dark-bg;
    padding: 10px 20px;
  }

  .progress {
    display: flex;
    align-items: center;

    .part {
      flex: none;
      position: relative;
    }

    #start {
      width: 350px;
    }

    #end {
      cursor: pointer;

      &:hover {
        color: #01AAC8;
      }
    }
  }
</style>
