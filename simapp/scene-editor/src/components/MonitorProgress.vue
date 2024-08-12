<template>
  <div class="container">
    <div class="event-list" />
    <el-tooltip v-for="(item, index) in events.events" :key="index" class="event-tooltip">
      <template #content>
        <div>{{ eventIndex(index) }}<br>{{ eventTime(item.time) }}<br> {{ dateString(item.ts) }}</div>
      </template>
      <div class="item" :style="{ left: `${Math.min(100, item.time / events.duration * 100)}%` }" />
    </el-tooltip>
    <div class="event-progress" :style="{ width: `${percentage}%` }" />
  </div>
</template>

<script>
export default {
  props: {
    events: {
      type: Object,
      default () {
        return {
          events: [],
          durations: 1000,
        }
      },
    },

    percentage: {
      type: Number,
      default: 0,
    },
  },
  data () {
    return {
    }
  },

  computed: {

  },
  methods: {
    eventIndex (index) {
      return `事件点${index + 1}：`
    },

    eventTime (time) {
      return `${(+time / 1000).toFixed(3)} s`
    },

    dateString (time) {
      const date = new Date(time)
      let minutes = date.getMinutes()
      if (minutes < 10) {
        minutes = `0${minutes}`
      }
      let seconds = date.getSeconds()
      if (seconds < 10) {
        seconds = `0${seconds}`
      }
      return `${date.getFullYear()}-${date.getMonth() + 1}-${date.getDate()} ${date.getHours()}:${minutes}:${seconds}`
    },
  },
}
</script>

<style scoped lang="less">
  @progressSize: 5px;
  .container {
    position: relative;
    height: @progressSize;
    .event-list {
      position: absolute;
      width: 100%;
      height: 50%;
      left: 50%;
      top: 50%;
      background-color: #666;
      transform: translate(-50%, -50%);
    }
    .item {
      position: absolute;
      width: @progressSize;
      height: @progressSize;
      border-radius: 50%;
      background-color: #fff;
      transform: translateX(-50%);
    }

    .event-tooltip {
      margin: 0px;
    }

    .event-progress {
      position: absolute;
      height: 50%;
      background: rgba(0,129,157, 0.5);
      pointer-events: none;
      top: 50%;
      transform: translateY(-50%);
    }
  }
</style>
