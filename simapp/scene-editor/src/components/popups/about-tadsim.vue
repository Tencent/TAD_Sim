<template>
  <div class="about-tadsim">
    <div class="logo">
      <LogoSvg />
    </div>
    <div class="logo-text">
      <LogoTextSvg />
    </div>
    <div class="version-number">
      <BaseAuth :perm="['config.app.about.text', 'text']">
        <template #default="{ text: aboutText }">
          <span v-if="aboutText">{{ aboutText }}&nbsp;</span>
        </template>
      </BaseAuth>
      <span>V{{ version }}</span>
    </div>
    <div v-if="false" class="copyright">
      Copyright &copy; 2017-2024 Tencent. All rights reserved.
    </div>
    <div class="btn-open-source-software-info" role="button" @click="showCopy">
      {{ $t('tips.openSourceSoftwareInformation') }}
    </div>
    <el-dialog
      append-to-body
      :header="$t('tips.openSourceSoftwareInformation')"
      width="860px"
      :show-close="false"
      :model-value="openSourceDialogVisible"
    >
      <div class="open-source-wrap">
        <div class="open-source-text">
          <pre>{{ openSourceTxt }}</pre>
        </div>
        <el-button class="open-source-close" @click="openSourceDialogVisible = false">
          {{ $t('operation.close') }}
        </el-button>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import LogoSvg from '@/assets/images/logo/logo.svg'
import LogoTextSvg from '@/assets/images/logo/logo-text.svg'
import openSourceTxt from '@/assets/lisence/open-source-info.txt?raw'

const { electron } = window

export default {
  name: 'AboutTadsim',
  components: {
    LogoSvg,
    LogoTextSvg,
  },
  data () {
    return {
      version: '2.0.0',
      openSourceDialogVisible: false,
      openSourceTxt: openSourceTxt.replace('<', '&lt;').replace('>', '&gt;'),
    }
  },
  created () {
    const { version } = electron.tadsim
    if (version) {
      this.version = version
    }
  },
  methods: {
    showCopy () {
      this.openSourceDialogVisible = true
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.about-tadsim {
  width: 370px;
  height: 232px;
  text-align: center;

  .logo {
    font-size: 70px;
    height: 80px;
    color: @logo-color;
  }

  .logo-text {
    height: 23px;
    line-height: 23px;
    color: @logo-color;
    margin-bottom: 15px;
  }

  .version-number {
    font-size: 12px;
    color: white;
    margin-bottom: 25px;
  }

  .copyright {
    color: @global-font-color;
    font-size: 12px;
    margin-bottom: 10px;
  }

  .btn-open-source-software-info {
    color: @active-font-color;
    font-size: 12px;
    cursor: pointer;
  }
}
</style>

<style lang="less">
.open-source-wrap {
  padding: 20px;

  .open-source-text {
    height: 600px;
    max-height: 60vh;
    background-color: #111;
    padding: 10px 20px;
    margin-bottom: 18px;
    color: #c2c2c2;
    overflow-y: auto;

    pre {
      margin: 0;
      white-space: pre-wrap;
      word-wrap: break-word;
    }
  }

  .open-source-close {
    float: right;
  }

  &::after {
    content: '';
    width: 0;
    height: 0;
    display: table;
    clear: both;
  }
}
</style>
