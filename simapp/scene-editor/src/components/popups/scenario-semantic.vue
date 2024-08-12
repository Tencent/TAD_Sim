<template>
  <div v-loading="loading" class="scenario-generation">
    <el-form
      ref="form"
      size="mini"
      label-width="115px"
      :model="form"
      :rules="rules"
    >
      <el-form-item prop="scenariosSet" :label="$t('scenario.setName')">
        <el-input v-model="form.scenariosSet" maxlength="50" class="prefix" />
      </el-form-item>
      <!-- <el-form-item prop="fileType" :label="$t('scenario.configType')">
        <div>
          <el-radio v-model="form.fileType" label="xlsx">{{ $t('scenario.localFile') }}</el-radio>
          <el-radio v-model="form.fileType" label="feishu">{{ $t('scenario.onlineFile') }}</el-radio>
        </div>
      </el-form-item> -->
      <!-- 文件模板 -->
      <el-form-item :label="$t('scenario.configFileExample')">
        <span class="profileTemplateName">
          <i class="el-icon-document-remove"></i>
          {{ $t(`scenario.semanticConfigFile`) }}.xlsx
        </span>
        <el-button
          type="text"
          class="btn-download"
          :disabled="exporting"
          @click="downloadTemplate"
        >
          {{ $t('scenario.downloadFile') }}
        </el-button>
        <!-- <div v-else>
          <el-button type="text" class="btn-download" @click="openExternal">{{ $t('scenario.openOnlineFile') }}</el-button>
        </div> -->
      </el-form-item>
      <el-form-item prop="filePath" :label="$t('scenario.configFileUpload')">
        <div v-if="form.filePath" class="filename-wrap">
          <i class="el-icon-document-remove"></i>
          {{ fileName }}
          <el-icon @click="clearFile" :style="{marginLeft: '10px'}"><Close /></el-icon>
        </div>
        <div class="upload-wrap">
          <el-upload
            ref="upload"
            class="upload-demo"
            accept=".xlsx, .xlsm"
            :limit="1"
            drag
            :file-list="[]"
            action="#"
            :on-change="handleChoseFile"
            :on-exceed="handleExceed"
          >
            <div class="el-upload__text"><em>点击上传</em>/ 拖到此区域</div>
          </el-upload>
          <div>仅支持 .xlsx/.xlsm 文件格式</div>
        </div>
      </el-form-item>
      <template v-if="form.fileType === 'feishu'">
        <el-form-item prop="filePath">
          <template #label>
            {{ $t('scenario.semanticFileAddress') }}
            <i class="el-icon-warning-outline"></i>
          </template>
          <el-input v-model="form.filePath" class="prefix" />
        </el-form-item>
        <el-form-item prop="appId">
          <template #label>
            {{ $t('scenario.applicationId') }}
            <i class="el-icon-warning-outline"></i>
          </template>
          <el-input v-model="form.appId" class="prefix" />
        </el-form-item>
        <el-form-item prop="fileSecret">
          <template #label>
            {{ $t('scenario.applicationSecret') }}
            <i class="el-icon-warning-outline"></i>
          </template>
          <el-input v-model="form.appSecret" class="prefix" />
        </el-form-item>
      </template>
      <el-form-item prop="mapType"  :label="$t('scenario.referenceMap')">
        <div class="cus-form-map-item">
          <div>
            <el-radio v-model="form.mapType" :label="0">{{ $t('scenario.semanticMap') }}</el-radio>
            <el-radio v-model="form.mapType" :label="1">{{ $t('scenario.mapsetMap') }}</el-radio>
          </div>
          <div v-if="form.mapType === 1">
            <div class="map-operate">
              <el-icon class="el-icon-plus" @click="openMap(true)">
                <plus />
              </el-icon>
              <el-icon class="el-icon-delete" @click="handleDelete">
                <delete />
              </el-icon>
            </div>
            <el-table
              ref="table"
              size="mini"
              :data="list"
              height="250"
              row-key="id"
              @selection-change="tableSelectionChange"
            >
              <el-table-column type="selection" label-class-name="select-all" width="36" />
              <el-table-column
                :label="$t('name')"
                prop="name"
                class-name="table-row"
                show-overflow-tooltip
              >
                <template #default="scope">
                  <div class="map-item">
                    <span>{{ scope.row.name }}</span>
                  </div>
                </template>
              </el-table-column>
              <el-table-column
                :label="$t('scenario.size')"
                prop="size"
                class-name="table-row"
                width="80"
                show-overflow-tooltip
              >
                <template #default="scope">{{ getSize(scope.row.size) }}</template>
              </el-table-column>
            </el-table>
          </div>
        </div>
      </el-form-item>
    </el-form>

    <section class="button-group">
      <el-button class="dialog-cancel" @click="$emit('close')">{{ $t('operation.cancel') }}</el-button>
      <el-button class="dialog-ok" :disabled="progressInfo.status" @click="handleOK">
        {{ $t('operation.ok') }}
      </el-button>
    </section>
    <el-dialog
      title="添加地图"
      :model-value="mapVisible"
      append-to-body
      :show-close="false"
      :close-on-press-escape="false"
      :close-on-click-modal="false"
    >
      <div class="progress-wrapper">
        <MapSelect
          :selected-map="list"
          :map-visible="mapVisible"
          @close="openMap(false)"
          @handleConfirm="handleConfirm"
        />
      </div>
    </el-dialog>
  </div>
</template>

<script>
import { mapState, mapMutations, mapActions } from 'vuex'
import { errorHandler } from '@/common/errorHandler'
import MapSelect from './map-select.vue'
import { validateScenarioName, getFileSize } from '@/common/utils'
import { CircleCheckFilled, CircleCloseFilled, InfoFilled, QuestionFilled } from '@element-plus/icons-vue'
import {
  startexe,
  generateSemantic,
  queryProgressInfo,
  stopgenprogressval,
} from '@/api/semantic'

const { electron, electron: { dialog } } = window

export default {
  name: 'scenario-semantic',
  components: {
    MapSelect,
  },
  data () {
    const validateName = async (rule, value, callback) => {
      if (validateScenarioName(value)) {
        callback()
      } else {
        callback(new Error(this.$t('tips.namePrefixErr', { name: this.$t('scenario.setName') })))
      }
    }
    return {
      exporting: false,
      loading: false,
      progressVisible: false,
      mapVisible: false,
      currentLogFileName: '',
      statusInterval: null,
      form: {
        scenariosSet: '',
        // fileType: 'xlsx',
        filePath: '',
        // appId: '',
        // appSecret: '',
        mapType: 0, // 0 语义生成地图 1 ： 选择地图
        maps: [],
      },
      fileName: '',
      generateSceneDir: '',
      list: [],
      selectedKeys: [],
      rules: {
        scenariosSet: [
          { required: true, message: this.$t('tips.enterSceneSetName'), trigger: 'blur' },
          { validator: validateName, trigger: 'blur' },
        ],
        filePath: [
          { required: true, message: this.$t('tips.selectFile'), trigger: 'blur' },
        ],
        fileSecret: [
          { required: true, message: this.$t('tips.pleaseInput'), trigger: 'blur' },
        ],
        appId: [
          { required: true, message: this.$t('tips.pleaseInput'), trigger: 'blur' },
        ],
      },
    }
  },
  computed: {
    ...mapState('progressInfo', [
      'progressInfo',
    ]),
  },
  watch: {
    'progressInfo.status': {
      async handler (val) {
        if (!val) {
          await this.getScenarioList()
          await this.getScenarioSetList()
        }
      },
    },
  },
  mounted () {
    // this.startProgressTask(30, "C:/Users\\felixgpeng\\AppData\\Roaming\\tadsim\\data\\scenario\\cache")
    // this.$confirmBox(this.$t('tips.generateScenariosSuccess'), this.$t('tips.tips'), {
    //     cancelButtonText: this.$t('tips.openTheFolder'),
    //      icon: (CircleCheckFilled),

    //     closeOnPressEscape: false,
    //     closeOnClickModal: false,
    //   }).catch(() => {
    //     electron.userLog.openUserLogFolder("C:/Users\\felixgpeng\\AppData\\Roaming\\tadsim\\data\\scenario")
    //   })
  },
  beforeDestroy () {
    this.generateSceneDir = ''
    clearInterval(this.statusInterval)
  },
  methods: {
    ...mapMutations('progressInfo', [
      'updateProgressInfo',
    ]),
    ...mapActions('scenario', [
      'getScenarioList',
    ]),
    ...mapActions('scenario-set', [
      'getScenarioSetList',
    ]),
    getSize: getFileSize,
    startProgressTask (mode = 30, dir) {
      this.updateProgressInfo({
        mode,
        status: true,
        progress: 0,
        dir,
      })
    },
    stopProgressTask () {
      const mockData = false
      clearInterval(this.statusInterval)
      stopgenprogressval()

      this.$confirmBox(this.$t('tips.generateScenariosSuccess'), this.$t('tips.tips'), {
        cancelButtonText: this.$t('tips.openTheFolder'),
        closeOnPressEscape: false,
        closeOnClickModal: false,
      }).catch(() => {
        electron.userLog.openUserLogFolder(this.generateSceneDir)
      })
      if (mockData) {
        setTimeout(() => {
          this.updateProgressInfo({
            status: false,
          })
        }, 10000)
      } else {
        this.updateProgressInfo({
          status: false,
        })
      }
    },
    openMap (flag) {
      this.mapVisible = flag
    },
    handleChoseFile (file) {
      // 大小低于20MB
      if (file.size / 1024 / 1024 > 20) {
        this.$message({
          message: this.$t('scenario.fileMaxMsg'),
          type: 'error',
          customClass: 'custom-message',
        })
        this.$refs.upload.clearFiles()
        return
      }
      const pointIndex = file.name.lastIndexOf('.')
      const suffix = file.name.substr(pointIndex + 1)
      if (/\s/g.test(file.name)) {
        this.fileErrorMessage = this.$t('scenario.mapSpaceMsg')
        this.$refs.upload.clearFiles()
        return false
      }
      // if ((file.name.substr(0, pointIndex)).length > 32) {
      //   this.fileErrorMessage = this.$t('scenario.mapFileErrorMsg2')
      //   this.$refs.upload.clearFiles()
      //   return false
      // }

      // 后缀名
      if (!['xlsx', 'xlsm'].includes(suffix)) {
        this.$message({
          message: this.$t('scenario.fileTypeMsg'),
          type: 'error',
          customClass: 'custom-message',
        })
        this.$refs.upload.clearFiles()
        return
      }
      this.fileErrorMessage = ''
      // 本地文件，将上传文件位置传过去
      this.form.filePath = file.raw.path
      this.fileName = file.name
      this.$message({
        message: this.$t('scenario.uploadSuccess'),
        type: 'success',
        customClass: 'custom-message',
      })
    },
    clearFile () {
      this.form.filePath = ''
      this.fileName = ''
      this.$refs.upload.clearFiles()
    },
    handleExceed () {

    },
    tableSelectionChange (selection) {
      this.selectedKeys = selection.map(item => item.name)
    },
    handleConfirm (mapList) {
      this.list = mapList
      this.mapVisible = false
    },
    handleDelete () {
      this.list = this.list.filter(v => !this.selectedKeys.includes(v.name))
    },
    handleOK () {
      if (this.progressInfo.status) {
        this.$message.warning(this.$t('tips.sceneIsGenerating'))
        return
      }
      this.$refs.form.validate(async valid => {
        if (valid) {
          try {
            this.loading = true
            const params = {
              ...this.form,
              maps: this.list.map(v => v.name),
            }
            if (this.form.mapType === 0) {
              delete params.maps
            }
            const res = await startexe(params)
            if (res.code === 0) {
              this.$confirmBox(
                `经检测，将清除不合格场景${res.nonScenariosCount}个，
                  预计生成场景${res.genScenariosCount}个, 确定生成？`,
                this.$t('tips.tips'),
                {
                  customClass: 'sim-confirm',
                  closeOnPressEscape: false,
                  closeOnClickModal: false }).then(async () => {
                // 开始生成场景, 之后查询进度展示进度条
                const { dir } = await generateSemantic()
                this.generateSceneDir = dir
                this.startProgressTask(30, dir)
                this.statusInterval = setInterval(() => {
                  this.getGenerateProcess()
                }, 1000)
              }).catch(action => {
                // 取消生成场景
                stopgenprogressval()
              })
            }
          } catch (error) {
            errorHandler(error)
          } finally {
            this.loading = false
          }
        }
      })
    },
    // 轮询生成状态， 如成功，取消轮询，
    async getGenerateProcess () {
      // const res = await semanticStatu()
      try {
        const res = await queryProgressInfo()

        if (res.code === 0) {
          const processNum = Number(res.message) * 100
          this.updateProgressInfo({
            progress: processNum,
          })
          if (processNum === 100) {
            this.stopProgressTask()
            // messageBoxConfirm(this.$t('tips.generateScenariosSuccess'), this.$t('tips.tips'), {
            //   cancelButtonText: this.$t('tips.openTheFolder'),
            //   closeOnPressEscape: false,
            //   closeOnClickModal: false,
            // }).catch(() => {
            //   electron.userLog.openUserLogFolder(this.generateSceneDir)
            //   // const isOpened = electron.editor.openBatchGenerateScenarioPath(this.generateSceneDir)
            //   // !isOpened && Message.error(this.$t('tips.openGeneratedScenesFolderFail'))
            // })
          }
        }
      } catch (e) {
        this.stopProgressTask()
        this.$errorBox(
          '场景生成失败，请重试!',
          this.$t('tips.error'),
          { customClass: 'sim-error' })
      }
    },
    async downloadTemplate () {
      try {
        this.exporting = true
        const { filePath } = await dialog.showSaveDialog({
          title: this.$t('operation.export'),
          defaultPath: `场景语义生成模板文件.xlsx`,
        })
        if (!filePath) return
        electron.userLog.copyFile(filePath)
        this.$message.success(this.$t('tips.exportSuccess'))
      } catch (err) {
        console.log(err)
        this.$message.error(this.$t('tips.exportFail'))
      } finally {
        this.exporting = false
      }
    },
    openExternal () {
      const url = 'https://c0dyndwlrc.feishu.cn/base/SLD7b2ITeaH7MwssrflcRZcsnzb?from=from_copylink'
      electron.userLog.openExternalUrl(url, {})
    },
    showItemInFolder () {
      if (this.currentLogFileName) {
        electron.userLog.showItemInFolder(this.currentLogFileName)
      } else {
        electron.userLog.openUserLogFolder()
      }
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .scenario-generation {
    width: 1120px;
    height: 450px;
    overflow: auto;
    padding: 23px;
    .btn-download {
      color: @active-font-color;
    }
    .filename-wrap {
      margin-bottom: 10px;
      width: 100%;
      display: flex;
      align-items: center;
      i {
        cursor: pointer;
      }
    }
    .upload-wrap {
      display: flex;
      align-items: flex-end;
      gap: 10px;
    }
    .upload-demo {
      width: 500px;
      :deep(.el-upload-dragger) {
        line-height: 180px;
        background: transparent;
        border: 1px dashed #fff;
        .el-upload__text em {
          color: @active-font-color;
        }
      }
    }
    .cus-form-map-item {
      flex: 1;
      display: flex;
      flex-direction: column;
    }
    .map-operate {
      margin: 20px;
      display: flex;
      align-items: center;
      justify-content: flex-end;
      gap: 10px;
      color: #fff;
      i {
        cursor: pointer;
      }
    }

    :deep(.el-input) {
      width: 500px;
    }

    :deep(.el-form-item) {
      margin-bottom: 15px;
      min-height: 28px;
    }

    :deep(.el-tabs__header) {
      margin-bottom: 0;
    }

    :deep(.el-tabs__nav) {
      .el-tabs__item {
        background-color: @hover-bg;

        &.is-active {
          background-color: #0f0f0f;
        }
      }
    }
    .el-table {
      color: @disabled-color;
      :deep(.cell) {
        padding: 0 10px;
      }
    }
    .button-group {
      margin-top: 20px;
      text-align: right;
    }
  }

</style>
