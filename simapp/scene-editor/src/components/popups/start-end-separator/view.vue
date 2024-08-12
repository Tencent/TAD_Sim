<template>
  <div class="start-end-separator-view">
    {{ value.start }}
    -
    {{ value.end }}
    <span v-if="value.sep !== ''" class="separator-icon">
      <el-tooltip effect="dark" :content="$t('interval')" placement="bottom">
        <separator-svg />
      </el-tooltip>
    </span>
    {{ value.sep }}
  </div>
</template>

<script>
export default {
  name: 'StartEndSeparatorView',
  props: {
    value: {
      type: Object,
      required: true,
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .start-end-separator-view {
    display: inline-block;

    .separator-icon {
      position: relative;
      top: 2px;
    }
  }
</style>
