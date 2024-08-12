<template>
  <div class="view-config">
    <div class="view-title">
      {{ $t('scenario.visbleSet') }}
    </div>
    <el-tree
      :data="treeData"
      :props="treeProps"
      default-expand-all
      @node-click="handleNodeClick"
    >
      <template #default="{ node }">
        <span v-if="node.data.children" class="custom-file-node">
          <FolderSvg type="file" class="custom-node-svg" />
          <span :title="node.label" class="custom-node-content">{{ node.label }}</span>
        </span>
        <span v-else class="custom-node">
          <CubeSvg type="cube" class="custom-node-svg custom-node-svg-cube" />
          <span :title="node.label" class="custom-node-content">{{ node.label }}</span>
          <VisibleSvg v-if="node.data.visible" />
          <InvisibleSvg v-else />
        </span>
      </template>
    </el-tree>
    <Bottom />
  </div>
</template>

<script>
import { get } from 'lodash-es'
import { mapGetters, mapMutations, mapState } from 'vuex'
import Bottom from './Bottom.vue'
import { editor, player } from '@/api/interface'
import CubeSvg from '@/assets/images/chart-bar-btns/cube.svg'
import InvisibleSvg from '@/assets/images/chart-bar-btns/invisible.svg'
import VisibleSvg from '@/assets/images/chart-bar-btns/visible.svg'
import FolderSvg from '@/assets/images/scenario-elements/folder.svg'

// if (this.isLog2World) {
//   sceneObjectTypes = [
//     { label: '主车(仿真)', value: 'simMainCar', visible: true },
//     { label: '主车(回放)', value: 'replayMainCar', visible: true },
//     { label: '动态交通流(仿真)', value: 'simTraffic', visible: true },
//     { label: '动态交通流(回放)', value: 'replayTraffic', visible: true },
//     { label: '交通信号灯', value: 'signlight', visible: true },
//     { label: '静态障碍物', value: 'trafficobstacle', visible: true },
//   ]
// }
export default {
  name: 'ViewConfig',
  components: {
    Bottom,
    CubeSvg,
    InvisibleSvg,
    VisibleSvg,
    FolderSvg,
  },
  props: {
    isLog2World: {
      type: Boolean,
      default: false,
    },
  },
  data () {
    return {
      treeProps: {
        children: 'children',
        label: 'label',
      },
      mapName: '',
    }
  },
  computed: {
    ...mapGetters('scenario', [
      'presentScenario',
      'isPlaying',
    ]),
    ...mapState('playerViewConfig', ['sceneViewConfig', 'mapViewConfig']),
    scenarioFullname () {
      if (this.presentScenario) {
        return this.presentScenario.filename || `${this.presentScenario.name}.${this.presentScenario.type}`
      }
      return ''
    },
    treeData () {
      const { sceneViewConfig, mapViewConfig } = this
      return [
        {
          label: this.scenarioFullname,
          value: 'sceneElement',
          children: [
            { label: '主车', value: 'sceneViewConfig.ego', visible: sceneViewConfig.ego },
            { label: '动态交通流', value: 'sceneViewConfig.trafficDynamic', visible: sceneViewConfig.trafficDynamic },
            { label: '交通信号灯', value: 'sceneViewConfig.signlight', visible: sceneViewConfig.signlight },
            { label: '静态障碍物', value: 'sceneViewConfig.obstacle', visible: sceneViewConfig.obstacle },
          ],
        },
        {
          label: this.mapName,
          value: 'map',
          children: [
            { label: '车道线', value: 'mapViewConfig.lane', visible: mapViewConfig.lane },
            { label: '车道连接线', value: 'mapViewConfig.lanelink', visible: mapViewConfig.lanelink },
            { label: '车道分界线', value: 'mapViewConfig.laneboundary', visible: mapViewConfig.laneboundary },
            { label: '路面', value: 'mapViewConfig.roadmesh', visible: mapViewConfig.roadmesh },
            { label: '网格线', value: 'mapViewConfig.grid', visible: mapViewConfig.grid },
          ],
        },
      ]
    },
  },
  watch: {
    presentScenario: {
      handler (newValue) {
        if (newValue) {
          let mapInfo
          if (this.isPlaying) {
            mapInfo = player.scenario.getMapInfo()
          } else {
            mapInfo = editor.scenario.getMapInfo()
          }
          this.mapName = get(mapInfo, 'mapName')
        }
      },
      immediate: true,
    },
  },
  methods: {
    ...mapMutations('playerViewConfig', ['updateViewConfig']),
    handleNodeClick (data) {
      const value = get(this, data.value)
      this.updateViewConfig({ path: data.value, checked: !value })
    },
  },
}
</script>

<style scoped lang='less'>
@import "@/assets/less/mixins";

@p-line-width: 10px;
@node-content-padding: 40px;

.view-config {
  width: 452px;
}

.custom-file-node {
  display: flex;
  position: relative;
  align-items: center;
  width: 100%;
  height: 100%;
}

.custom-node {
  display: flex;
  position: relative;
  align-items: center;
  width: 100%;
  height: 100%;
  user-select: none;

  &-content {
    width: 260px;
    flex: 1;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  &-svg {
    margin-right: 11px;
  }

  &:before {
    content: "";
    position: absolute;
    left: -@p-line-width;
    top: 0;
    height: calc(~"100% + @{node-content-padding}");
    border-left: 1px dashed #474747;
  }

  &:after {
    content: "";
    position: absolute;
    left: -@p-line-width;
    top: 50%;
    width: @p-line-width;
    border-top: 1px dashed #474747;
  }

}

.view-title {
  padding: 20px 10px 5px 10px;
  font-size: 16px;
  color: @title-font-color;
}

:deep(.el-tree) {
  background: transparent;
  color: #fff;

  .el-tree-node__content:hover {
    background-color: @dark-bg;
  }

  .el-tree-node .is-expanded .is-current .is-focusable {
    background-color: @dark-bg;
  }

  &-node__content {
    box-sizing: border-box;
    padding: @node-content-padding/2 10px @node-content-padding/2 0;
  }

  &-node__children {
    .el-tree-node:last-child {
      .custom-node:before {
        height: 50%;
      }
    }

    .el-tree-node__content {
      padding-left: 0 !important;
      margin-left: -3px;
    }

    .el-tree-node:first-child {
      .custom-node:before {
        top: -@node-content-padding / 2;
        height: calc(~"100% + @{node-content-padding} * 1.5");
      }
    }
  }
}
</style>
