<template>
  <div class="kpi-report-scene-image">
    <!-- <div class="header">
      <span class="label">场景示意图</span>
      <el-button link @click="handleResetImage">重置</el-button>
    </div> -->
    <div class="preview">
      <div v-if="filePath" class="preview-wrap" @click="handleBrowseFile">
        <img :src="filePath" alt="">
      </div>
      <div v-else class="preview-wrap placeholder" @click="handleBrowseFile">
        <img src="../../../assets/images/scenario-thumb.png" alt="">
        <!-- <div class="tips-wrap">
          <div>点击上传替换</div>
          <div>仅支持 .jpg/.png 文件格式</div>
        </div> -->
      </div>
    </div>
  </div>
</template>

<script>
import { mapActions } from 'vuex'
import { errorHandler } from '@/common/errorHandler'

const { electron: { dialog } } = window

export default {
  props: {
    id: {
      type: String,
      default: '',
    },
  },
  data () {
    return {
      filePath: '',
    }
  },
  watch: {
    id () {
      this.refreshSceneImage()
    },
  },
  mounted () {
    this.refreshSceneImage()
  },
  methods: {
    ...mapActions('kpi-report', [
      'uploadReportImage',
      'deleteReportImage',
      'getReportImage',
      'getImageBase64',
    ]),
    async refreshSceneImage () {
      if (!this.id) return
      const res = await this.getReportImage({ id: this.id })
      if (res) {
        const base64 = await this.getImageBase64(res)
        this.filePath = base64 ? (`data:image/png;base64,${base64}`) : ''
      } else {
        this.filePath = ''
      }
    },
    async handleBrowseFile () {
      try {
        const { filePaths } = await dialog.showOpenDialog({
          title: this.$t('tips.selectFile'),
          properties: ['openFile'],
          filters: [
            { name: this.$t('menu.image'), extensions: ['jpg', 'png'] },
          ],
        })
        const [filePath] = filePaths
        if (filePath) {
          await this.handleFileChange(filePath)
        }
      } catch (e) {
        await errorHandler(e)
      }
    },
    async handleFileChange (filePath) {
      await this.uploadReportImage({ id: this.id, filePath })
      await this.refreshSceneImage()
    },
    async handleResetImage () {
      await this.deleteReportImage({ id: this.id })
      await this.refreshSceneImage()
    },
  },
}
</script>

<style scoped lang="less">
.kpi-report-scene-image {
  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .preview-wrap {
    position: relative;
    width: 200px;
    height: 112px;
    cursor: pointer;

    & > img {
      width: 100%;
      height: 100%;
      display: block;
      object-fit: cover;
      border-radius: 8px;
    }

    &.placeholder > img {
      opacity: 0.5;
    }

    .tips-wrap {
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(0, 0, 0, 0.2);
      color: #888;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
    }
  }
}
</style>
