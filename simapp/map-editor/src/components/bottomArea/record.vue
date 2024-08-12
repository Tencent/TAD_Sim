<script setup lang="ts">
import moment from 'moment'
import { computed } from 'vue'
import { type IHistoryState, useHistoryStore } from '@/stores/history'

const historyStore = useHistoryStore()
const recordList = computed(() => {
  return (
    historyStore.recordsOnDisplay
      .map((record: IHistoryState) => ({
        id: record.id,
        time: moment(record.timestamp).format('YYYY-MM-DD HH:mm:ss'),
        content: record.name,
      }))
      // 倒序展示操作日志，即最新的操作展示在最上方
      .reverse()
  )
})
</script>

<template>
  <div class="record-area">
    <ul class="record-ul">
      <li v-for="record in recordList" :key="record.id" class="record-li">
        {{ record.time }}: {{ record.content }}
      </li>
    </ul>
  </div>
</template>

<style scoped lang="less">
.record-area {
  .record-ul {
    .record-li {
      margin: 0.3em;
      font-size: var(--font-size);
    }
  }
}
</style>
