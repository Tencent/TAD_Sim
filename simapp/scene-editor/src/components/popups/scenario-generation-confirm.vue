<template>
  <el-dialog
    class="scenario-generation-confirm"
    :model-value="true"
    append-to-body
    :close-on-click-modal="false"
    :show-close="true"
    @close="$emit('close')"
  >
    <div v-loading="loading" class="scenario-generation-confirm-wrap">
      <div class="scenario-generation-confirm-title">
        <el-icon class="el-icon-question">
          <warning />
        </el-icon> {{ $t('tips.tips') }}
      </div>

      <i18n-t tag="div" keypath="tips.okToGen" class="scenario-generation-confirm-content">
        <template #count>
          <span class="scenario-generation-confirm-emphasis">{{ count }}</span>
        </template>
        <template #text>
          <span class="scenario-generation-confirm-emphasis">{{ sizeText }}</span>
        </template>
        <template #time>
          <span class="scenario-generation-confirm-emphasis">{{ timeText }}</span>
        </template>
      </i18n-t>

      <section class="scenario-generation-confirm-btns">
        <div class="scenario-generation-confirm-btns-left">
          <el-tooltip
            effect="dark"
            :content="addScenarioSetAutoTip"
            placement="top"
            :disabled="!addScenarioSetAutoDisabled"
          >
            <el-checkbox v-model="addScenarioSetAuto" :disabled="addScenarioSetAutoDisabled">
              {{ $t('operation.addToSceneSet') }}
            </el-checkbox>
          </el-tooltip>
        </div>
        <div class="scenario-generation-confirm-btns-right">
          <el-button class="dialog-cancel" @click="$emit('close')">
            {{ $t('operation.cancel') }}
          </el-button>
          <el-button type="primary" class="dialog-ok" @click="handleOK">
            {{ $t('operation.ok') }}
          </el-button>
        </div>
      </section>
    </div>
  </el-dialog>
</template>

<script>
import { mapState } from 'vuex'
import { getScenarioGenerationInfo } from '@/api/scenario'
import { errorHandler } from '@/common/errorHandler'
import { getFileSize, getTimeName } from '@/common/utils'
import dict from '@/common/dict'

export default {
  name: 'ScenarioGenerationConfirm',
  props: {
    value: {
      type: Object,
      required: true,
    },
  },
  data () {
    return {
      count: '',
      time: '',
      sizeText: '',
      timeText: '',
      addScenarioSetAuto: false,
      loading: false,
    }
  },
  computed: {
    ...mapState('scenario', [
      'scenarioList',
      'currentScenario',
    ]),
    addScenarioSetAutoDisabled () {
      return this.scenarioList.length + (+this.count) > dict.maxScenarioCount
    },
    addScenarioSetAutoTip () {
      return this.$t('tips.cantAddBcofMaximumScenario', { maximum: dict.maxScenarioCount })
    },
  },
  created () {
    this.init()
  },
  methods: {
    async init () {
      this.loading = true
      const { filename: sceneName } = this.currentScenario
      const param = JSON.stringify(this.value)
      try {
        const scenarioGenerationInfo = await getScenarioGenerationInfo({
          sceneName,
          param,
        })
        const { count, time, size } = scenarioGenerationInfo
        this.count = count
        this.time = time
        this.sizeText = getFileSize(size)
        this.timeText = getTimeName(time)
        if (count === 0) {
          await errorHandler(new Error(this.$t('tips.paraCantGenerateScenario')))
        }
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
    },
    handleOK () {
      const { addScenarioSetAuto, time } = this
      this.$emit('confirm', {
        addScenarioSetAuto,
        time,
      })
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .scenario-generation-confirm {
    :deep(.el-dialog .el-dialog__header) {
      padding: 0 15px;
      height: 12px;
      line-height: 12px;
      background-color: #242424;
    }

    .scenario-generation-confirm-wrap {
      width: 260px;
      padding-bottom: 15px;
      background-color: #242424;
      border-radius: 2px;
      box-shadow: 0 4px 4px 0 rgba(0, 0, 0, 0.14);

      .scenario-generation-confirm-title {
        height: 20px;
        padding: 15px 15px 10px;
        font-size: 18px;
        text-align: center;
        color: #c2c2c2;

        i {
          margin-right: 10px;
          color: #EC8215;
          font-size: 24px;
          line-height: 1;
          font-weight: 400;
          vertical-align: bottom;
        }
      }

      .scenario-generation-confirm-content {
        padding: 10px 15px;
        color: #606266;
        font-size: 14px;
        line-height: 24px;
      }

      .scenario-generation-confirm-emphasis {
        color: #ccc;
      }

      .scenario-generation-confirm-btns {
        padding: 5px 15px 0;
        .scenario-generation-confirm-btns-left {
          margin-bottom: 10px;
        }
        .scenario-generation-confirm-btns-right {
          text-align: right;
        }
      }
    }
  }
</style>
