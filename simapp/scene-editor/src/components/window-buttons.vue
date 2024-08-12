<template>
  <div class="window-buttons">
    <div class="btn minimize" role="button" @click="minimum">
      <MinimizeSvg />
    </div>
    <div class="btn maximize" role="button" @click="toggleMaximum">
      <RestoreSvg v-if="isMaximized" />
      <MaximizeSvg v-else />
    </div>
    <div class="btn close" role="button" @click="quit">
      <CloseSvg />
    </div>
  </div>
</template>

<script>
import { mapActions, mapState } from 'vuex'
import MinimizeSvg from '@/assets/images/title-bar-btns/minimize.svg'
import MaximizeSvg from '@/assets/images/title-bar-btns/maximize.svg'
import RestoreSvg from '@/assets/images/title-bar-btns/restore.svg'
import CloseSvg from '@/assets/images/title-bar-btns/close.svg'

const { electron: { crossBrowserWindowMessage } } = window

export default {
  name: 'WindowButtons',
  components: {
    MinimizeSvg,
    MaximizeSvg,
    RestoreSvg,
    CloseSvg,
  },
  computed: {
    ...mapState('title-bar', [
      'isMaximized',
    ]),
  },
  mounted () {
    this.initIsMaximized()
    crossBrowserWindowMessage.on('map-editor-closed', this.mapEditorClosed)
  },
  beforeUnmount () {
    crossBrowserWindowMessage.off('map-editor-closed', this.mapEditorClosed)
  },
  methods: {
    ...mapActions('title-bar', [
      'quit',
      'mapEditorClosed',
      'toggleMaximum',
      'minimum',
      'initIsMaximized',
    ]),
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .window-buttons {
    width: 120px;
    height: 40px;
    display: flex;
    position: absolute;
    z-index: 1001;
    top: 0;
    right: 0;
    color: @title-font-color;

    .btn {
      width: 40px;
      height: 40px;
      cursor: pointer;
      display: flex;
      justify-content: center;
      align-items: center;
      transition: background-color .16s linear;

      &:hover {
        background-color: @hover-bg;
      }
    }
  }
</style>
