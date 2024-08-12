<script setup lang="ts">
// @ts-nocheck
import LogoSvg from './logoIcon/logo.svg?component'
import LogoTextSvg from './logoIcon/logo-text.svg?component'
import { globalConfig } from '@/utils/preset'

const { projectName, isCloud } = globalConfig

</script>

<template>
  <div class="logo">
    <LogoSvg class="logo-svg" />
    <LogoTextSvg class="logo-text-svg" />
    <BaseAuth :perm="['config.app.title.text', 'titleText']">
      <template #default="{ titleText }">
        <template v-if="!!titleText">
          <span class="corporation-partner">{{ titleText }}</span>
        </template>
      </template>
    </BaseAuth>
  </div>
</template>

<style scoped lang="less">
.logo {
  display: flex;
  margin-right: 37px;
  align-items: center;
  font-size: 14px;
  color: var(--logo-color);

  &.fix-margin {
    margin: 0 20px;
  }

  .logo-img {
    width: 90%;
  }

  .logo-svg {
    color: var(--logo-color);
    font-size: 26px;
    margin: 0 11px 0 17px;
  }
  .logo-text-svg {
    color: var(--logo-color);
  }
  .corporation-icon {
    font-size: 13px;
    font-weight: lighter;
    transform: scale(0.7, 1);
    margin-right: 8px;
  }
  .corporation-partner {
    font-size: 15px;
  }
}
</style>
