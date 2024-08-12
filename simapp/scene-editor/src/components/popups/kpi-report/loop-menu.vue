<template>
  <el-sub-menu v-if="item.children && item.children.length" :index="item.id">
    <template #title>
      <span>{{ item.title }}</span>
    </template>
    <LoopMenu v-for="nav of item.children" :key="nav.id" :item="nav" @click-item="handleChildTrigger" />
  </el-sub-menu>
  <el-menu-item v-else :index="item.id" @click="handleClick(item)">
    <span>{{ item.title }}</span>
  </el-menu-item>
</template>

<script>
import { throttle } from 'lodash-es'

export default {
  name: 'LoopMenu',
  recursion: true,
  props: {
    item: {
      type: Object,
      required: true,
    },
  },
  emits: ['click-item'],
  methods: {
    // 递归组件莫名会触发多次，因此只取第一次回调
    handleClick: throttle(function (data) {
      this.$emit('click-item', data)
    }, 200, { leading: true, trailing: false }),
    // 递归组件接收子级来的事件，继续向上报
    handleChildTrigger (...args) {
      this.$emit('click-item', ...args)
    },
  },
}
</script>
