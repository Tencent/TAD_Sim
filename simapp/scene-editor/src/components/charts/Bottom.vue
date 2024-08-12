<template>
  <div class="checkbox-container">
    <div class="checkbox-title">
      {{ $t('scenario.visbleFlow') }}
    </div>
    <el-tabs v-model="activeName">
      <el-tab-pane :label="$t('scenario.ego')" name="ego">
        <el-checkbox
          label="id"
          :value="cityMonitor.simMainCar.id"
          @change="updateViewConfig({ path: 'cityMonitor.simMainCar.id', checked: $event })"
        />
        <el-checkbox
          label="速度"
          :value="cityMonitor.simMainCar.v"
          @change="updateViewConfig({ path: 'cityMonitor.simMainCar.v', checked: $event })"
        />
        <el-checkbox
          label="加速度"
          :value="cityMonitor.simMainCar.acc"
          @change="updateViewConfig({ path: 'cityMonitor.simMainCar.acc', checked: $event })"
        />
        <el-checkbox
          label="经纬高度"
          :value="cityMonitor.simMainCar.lonlat"
          @change="updateViewConfig({ path: 'cityMonitor.simMainCar.lonlat', checked: $event })"
        />
      </el-tab-pane>
      <el-tab-pane :label="$t('scenario.transporters')" name="traffic">
        <el-checkbox
          label="id"
          :value="cityMonitor.simTraffic.id"
          @change="updateViewConfig({ path: 'cityMonitor.simTraffic.id', checked: $event })"
        />
        <el-checkbox
          label="速度"
          :value="cityMonitor.simTraffic.v"
          @change="updateViewConfig({ path: 'cityMonitor.simTraffic.v', checked: $event })"
        />
        <el-checkbox
          label="相对速度"
          :value="cityMonitor.simTraffic.vr"
          @change="updateViewConfig({ path: 'cityMonitor.simTraffic.vr', checked: $event })"
        />
        <el-checkbox
          label="加速度"
          :value="cityMonitor.simTraffic.acc"
          @change="updateViewConfig({ path: 'cityMonitor.simTraffic.acc', checked: $event })"
        />
        <el-checkbox
          label="相对加速度"
          :value="cityMonitor.simTraffic.ar"
          @change="updateViewConfig({ path: 'cityMonitor.simTraffic.ar', checked: $event })"
        />
        <el-checkbox
          label="经纬高度"
          :value="cityMonitor.simTraffic.lonlat"
          @change="updateViewConfig({ path: 'cityMonitor.simTraffic.lonlat', checked: $event })"
        />
      </el-tab-pane>
      <el-tab-pane :label="$t('scenario.otherObject')" name="other">
        <el-checkbox
          label="id"
          :value="cityMonitor.others.id"
          @change="updateViewConfig({ path: 'cityMonitor.others.id', checked: $event })"
        />
      </el-tab-pane>
    </el-tabs>
    <div class="checkbox-bottom">
      <el-button size="small" @click="reset">
        重置
      </el-button>
    </div>
  </div>
</template>

<script>
import { mapMutations, mapState } from 'vuex'

export default {
  props: {
  },
  data () {
    return {
      activeName: 'ego',
    }
  },
  computed: {
    ...mapState('playerViewConfig', ['cityMonitor']),
  },
  methods: {
    ...mapMutations('playerViewConfig', ['updateViewConfig', 'reset']),
  },
}
</script>

<style lang="less" scoped>
@import "@/assets/less/mixins";

.checkbox-container {
  padding: 10px 15px;

  .checkbox-title {
    margin-bottom: 5px;
    color: @title-font-color;
  }

  .checkbox-bottom {
    padding-top: 10px;
    margin: 15px 0;
    text-align: right;
  }

  :deep(.el-tabs__content) {
    padding: 10px 0;
    background: none;
  }

  :deep(.el-checkbox-group) {
    display: flex;
    flex-wrap: wrap;

    .el-checkbox {
      margin: 0 0 10px 0;
      flex-basis: 50%;
    }
  }
}
</style>
