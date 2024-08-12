<template>
  <div v-loading="loading" class="create-new-scene">
    <section class="part-choose-map">
      <div class="choose-map-title">
        <sim-label class="choose-map-label" :type="mapType === 1 ? 'primary' : 'default'" @click="mapType = 1">
          {{ $t('scenario.mapLibrary') }}
        </sim-label>
      </div>
      <div class="choose-map-content">
        <ul v-if="mapType === 0" class="scene-list">
          <li
            v-for="item in buildInMapList"
            :key="item.name"
            class="scene-item select-scenario"
            :class="{ 'item-active': form.mapId === item.name }"
            @click="form.mapId = item.name"
          >
            <MapThumbnailSvg class="img" />
            <!-- <img class="img" :src="item.src" alt="" /> -->
            <div class="label">
              {{ item.name }}
            </div>
          </li>
        </ul>
        <el-table
          v-if="mapType === 1"
          :data="userMapList"
          row-class-name="select-scenario"
          @row-click="selectMap"
        >
          <el-table-column
            prop="name"
            width="40"
          >
            <template #default="scope">
              <el-radio v-model="form.mapId" :label="scope.row.name" class="no-label" />
            </template>
          </el-table-column>
          <el-table-column
            prop="name"
            :label="$t('name')"
            show-overflow-tooltip
          />
          <el-table-column
            prop="createTime"
            :label="$t('scenario.creationDate')"
            width="110"
            :formatter="formatDate"
          />
        </el-table>
      </div>
    </section>
    <section class="part-form">
      <el-form ref="form" :model="form" :rules="rules" @submit.prevent>
        <el-form-item class="scene-name" :label="$t('scenario.name')" prop="name" style="display: block;">
          <el-input v-model.trim="form.name" name="name" maxlength="50" class="name" />
          .sim
        </el-form-item>
        <el-form-item :label="$t('scenario.workingCondition')" prop="info" class="param-cell" style="display: block;">
          <el-input
            v-model="form.info"
            type="textarea"
            rows="4"
            name="info"
            maxlength="200"
            show-word-limit
          />
        </el-form-item>
      </el-form>

      <div class="part-button-group">
        <el-button class="dialog-cancel" @click="$emit('close')">
          {{ $t('operation.close') }}
        </el-button>
        <el-button class="dialog-ok" @click="handleCreate">
          {{ $t('operation.create') }}
        </el-button>
      </div>
    </section>
  </div>
</template>

<script>
import _ from 'lodash'
import { mapActions, mapState } from 'vuex'
import { errorHandler } from '@/common/errorHandler'
import MapThumbnailSvg from '@/assets/images/map-thumbnail.svg'
import { validateScenarioName } from '@/common/utils'

export default {
  name: 'CreateNewScene',
  components: { MapThumbnailSvg },
  data () {
    const validateName = (rule, value, callback) => {
      if (validateScenarioName(value)) {
        const matched = _.find(this.scenarioList, { filename: `${value}.sim` })
        if (matched) {
          callback(new Error(this.$t('tips.sameSceneNameExists')))
        } else {
          callback()
        }
      } else {
        callback(new Error(this.$t('tips.sceneNameRuleTips')))
      }
    }

    return {
      form: {
        mapId: null,
        name: '',
        info: '',
        type: 'sim',
      },
      rules: {
        name: [
          { required: true, message: this.$t('tips.enterNewSceneName') },
          { validator: validateName, trigger: 'blur' },
        ],
      },
      mapType: 1, // 0: 地图库， 1: 自定义地图
      loading: false,
    }
  },
  computed: {
    ...mapState('scenario', [
      'scenarioList',
    ]),
    ...mapState('map', [
      'mapList',
    ]),
    buildInMapList () {
      return this.mapList.filter(map => map.isBuildin)
    },
    userMapList () {
      return this.mapList.filter(map => !map.isBuildin)
    },
  },
  async created () {
    this.loading = true
    await this.getMapList()
    this.loading = false
  },
  methods: {
    ...mapActions('map', [
      'getMapList',
    ]),
    ...mapActions('scenario', [
      'createScenario',
    ]),
    async handleCreate () {
      if (!this.form.mapId) {
        await errorHandler(new Error(this.$t('tips.selectMap')))
        return
      }
      try {
        await this.$refs.form.validate()
      } catch (e) {
        return
      }
      this.loading = true
      try {
        await this.createScenario(this.form)
        this.$emit('close')
      } catch (e) {
        await errorHandler(e)
      }
      this.loading = false
    },
    formatDate (row, column, value) {
      return new Date(value).toLocaleDateString('zh')
    },
    selectMap (row) {
      this.form.mapId = row.name
    },
  },
}
</script>

<style scoped lang="less">
  @import "@/assets/less/mixins";

  .create-new-scene {
    box-sizing: border-box;
    width: 600px;
    height: 440px;
    overflow: hidden;
    display: flex;
    padding: 0 20px 16px;

    @bottom-height: 40px;
    @right-width: 220px;

    .choose-map-label {
      cursor: pointer;
    }

    .part-choose-map {
      flex: 1;
      width: @right-width;
      margin-right: 20px;
      overflow: auto;
      display: flex;
      flex-direction: column;

      @map-title-height: 35px;

      .choose-map-title {
        height: @map-title-height;
        line-height: @map-title-height;
      }

      .choose-map-content {
        flex: 1;
        overflow: auto;
        background-color: #161616;

        .scene-list {
          list-style: none;
          display: grid;
          grid-template-columns: 1fr 1fr 1fr;
          justify-items: center;

          .scene-item {
            padding: 8px 10px;
            width: 80px;
            height: 118px;
            color: @global-font-color;

            &:hover,
            &.item-active {
              .menu-active-style;
            }

            .img {
              width: 80px;
              height: 80px;
            }

            .label {
              padding: 5px 0;
              text-align: center;
              font-size: 12px;
              .text-overflow;
            }
          }
        }
      }
    }

    :deep(.select-scenario){
      color: @title-font-color;
    }

    .part-form {
      width: @right-width;
      overflow: auto;
      padding-top: 32px;
      display: flex;
      flex-direction: column;

      .el-form {
        flex: 1;
        :deep(.el-form-item__label){
          color: @title-font-color;
        }

        .name {
          width: 188px;
        }
      }
    }

    .part-button-group {
      text-align: right;
    }

    .param-cell {
      .el-textarea {
        .el-input__count {
          background: rgba(0, 0, 0, 0);
          font-size: 8px;
          bottom: -20px;
          right: 0;
        }
      }
    }

    .scene-name {
      height: 100px;
    }
  }
</style>
