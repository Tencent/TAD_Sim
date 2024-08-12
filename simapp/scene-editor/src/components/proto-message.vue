<template>
  <div id="proto-message" class="proto-message">
    <article class="proto-message-json">
      <JsonPretty
        v-if="displayValue"
        :data="displayValue"
        :deep="2"
        theme="dark"
        show-icon
      />
    </article>
  </div>
</template>

<script>
import JsonPretty from 'vue-json-pretty'
import eventBus from '@/event-bus'
import { getProtoMessage } from '@/stored/proto-message'
import 'vue-json-pretty/lib/styles.css'

export default {
  name: 'ProtoMessage',
  components: {
    JsonPretty,
  },
  data () {
    return {
      displayValue: null,
    }
  },
  created () {
    eventBus.$on('proto-message', this.handleProtoMessage)
  },
  mounted () {
    this.handleProtoMessage({ detail: getProtoMessage() })
  },
  beforeUnmount () {
    eventBus.$off('proto-message', this.handleProtoMessage)
  },
  methods: {
    handleProtoMessage (payload) {
      this.displayValue = payload?.detail || null
    },
    clear () {
      this.displayValue = null
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.proto-message {
  min-height: 150px;
  max-height: 100%;

  .vjs-tree {
    :deep(.vjs-tree-node) {
      &.is-highlight, &:hover {
        background-color: #333;
      }
    }
  }
}
</style>
