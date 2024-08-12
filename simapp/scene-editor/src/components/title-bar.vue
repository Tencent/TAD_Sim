<template>
  <div class="title-bar">
    <div class="logo">
      <LogoSvg class="logo-svg" />
      <LogoTextSvg />
      <BaseAuth :perm="['config.app.title.text']">
        <template #default="{ permission: titleText }">
          <span v-if="titleText" class="corporation-partner">{{ titleText }}</span>
        </template>
      </BaseAuth>
    </div>
    <NavMenu />
    <div class="drag-handler" :style="{ height: menuVisible ? '40px' : '3px' }" />
    <div class="btns" />
  </div>
</template>

<script>
import { mapState } from 'vuex'
import NavMenu from './nav-menu.vue'
import LogoSvg from '@/assets/images/logo/logo.svg'
import LogoTextSvg from '@/assets/images/logo/logo-text.svg'

export default {
  name: 'TitleBar',
  components: {
    NavMenu,
    LogoTextSvg,
    LogoSvg,
  },
  computed: {
    ...mapState('title-bar', [
      'menuVisible',
    ]),
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .title-bar {
    flex: 0 0 40px;
    margin-bottom: 1px;
    background-color: @darker-bg;
    color: @title-font-color;
    display: flex;
    align-items: center;
    justify-content: flex-start;
    user-select: none;
    -webkit-app-region: drag;
    z-index: 11;
  }

  .logo {
    flex: 0 0 auto;
    display: flex;
    margin-right: 37px;
    align-items: center;
    -webkit-app-region: no-drag;
    color: @logo-color;

    .logo-svg {
      font-size: 26px;
      margin: 0 11px 0 17px;
    }

    .corporation-icon {
      font-size: 13px;
      font-weight: lighter;
      transform: scale(.7, 1);
      margin-right: 8px;
    }

    .corporation-partner {
      font-size: 15px;
    }
  }

  .drag-handler {
    flex: 1;
    align-self: flex-start;
    -webkit-app-region: no-drag;
  }

  .btns {
    flex: 0 0 120px;
    height: 40px;
    -webkit-app-region: no-drag;
  }
</style>
