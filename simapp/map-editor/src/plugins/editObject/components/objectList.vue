<script setup lang="ts">
import { type Ref, ref } from 'vue'
import Pole from './list/pole.vue'
import SignalBoard from './list/signalBoard.vue'
import Other from './list/other.vue'
import RoadSign from './list/roadSign.vue'
import TrafficLight from './list/trafficLight.vue'
import Sensor from './list/sensor.vue'
import CustomModel from './list/customModel.vue'
import { getPermission } from '@/utils/permission'

type ObjectType =
  | 'pole'
  | 'signalBoard'
  | 'trafficLight'
  | 'roadSign'
  | 'sensor'
  | 'other'
  | 'customModel'

const _objectTypes: Array<ObjectType> = [
  'pole',
  'roadSign',
  'signalBoard',
  'trafficLight',
  'sensor',
  'other',
  'customModel',
]
const objectTypes = _objectTypes.filter((type) => {
  switch (type) {
    case 'pole':
      return [
        'action.mapEditor.models.pole.vertical.view.enable',
        'action.mapEditor.models.pole.horizontal.view.enable',
      ].some(getPermission)
    case 'roadSign':
      return [
        'action.mapEditor.models.roadSign.indicator.view.enable',
        'action.mapEditor.models.roadSign.warning.view.enable',
        'action.mapEditor.models.roadSign.prohibition.view.enable',
        'action.mapEditor.models.roadSign.other.view.enable',
      ].some(getPermission)
    case 'signalBoard':
      return [
        'action.mapEditor.models.signalBoard.indicator.view.enable',
        'action.mapEditor.models.signalBoard.warning.view.enable',
        'action.mapEditor.models.signalBoard.prohibition.view.enable',
      ].some(getPermission)
    case 'trafficLight':
      return [
        'action.mapEditor.models.trafficLight.vertical.view.enable',
        'action.mapEditor.models.trafficLight.horizontal.view.enable',
      ].some(getPermission)
    case 'sensor':
      return [
        'action.mapEditor.models.sensor.camera.view.enable',
        'action.mapEditor.models.sensor.radar.view.enable',
        'action.mapEditor.models.sensor.lidar.view.enable',
        'action.mapEditor.models.sensor.rsu.view.enable',
      ].some(getPermission)
    case 'other':
      return [
        'action.mapEditor.models.other.roadDamageOrDefect.view.enable',
        'action.mapEditor.models.other.trafficManagement.view.enable',
        'action.mapEditor.models.other.vegetation.view.enable',
        'action.mapEditor.models.other.structure.view.enable',
        'action.mapEditor.models.other.bridge.view.enable',
      ].some(getPermission)
    case 'customModel':
      return getPermission('router.mapEditor.importCustomModels.show')
  }
  return []
})

const objectComponentList = {
  pole: Pole,
  signalBoard: SignalBoard,
  trafficLight: TrafficLight,
  roadSign: RoadSign,
  sensor: Sensor,
  other: Other,
  customModel: CustomModel,
}
const currentTab: Ref<ObjectType> = ref(objectTypes[0])

function changeObjectType (objectType: ObjectType) {
  currentTab.value = objectType
}
</script>

<template>
  <ul class="tab-ul">
    <li
      v-for="name in objectTypes"
      :key="name"
      class="tab-li"
      :title="$t(`desc.editObject.${name}`)"
      :class="{ active: currentTab === name }"
      @click="() => changeObjectType(name)"
    >
      {{ $t(`desc.editObject.${name}`) }}
    </li>
  </ul>
  <div class="tab-content">
    <component
      :is="objectComponentList[name]"
      v-for="name in objectTypes"
      v-show="currentTab === name"
      :key="name"
    />
  </div>
</template>

<style scoped lang="less">
.tab-ul {
  display: flex;
  flex-wrap: wrap;
  .tab-li {
    padding: 0 10px;
    height: 28px;
    line-height: 28px;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    cursor: pointer;

    &:hover {
      color: var(--active-color);
    }
    &.active {
      color: var(--active-color);
    }
  }
}

// 在父组件中定义所有子列表项的样式
.tab-content {
  // 通过父组件控制每一种类型物体的列表样式，需要穿透
  :deep(.object-list) {
    .item-tab {
      background-color: var(--object-tab-background);
      height: 28px;
      border-top: 1px solid var(--object-tab-divider-color);
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 12px;

      .arrow-icon {
        transform: rotateZ(0deg);

        &.folded {
          transform: rotateZ(180deg);
        }
      }
    }
    .object-ul {
      display: flex;
      justify-content: flex-start;
      flex-wrap: wrap;
      overflow: hidden;
      height: auto;

      &.folded {
        height: 0;
      }
      .object-li {
        display: inline-block;
        width: 80px;
        margin: 6px;
        .icon-box {
          width: 80px;
          height: 80px;
          box-sizing: border-box;
          border: 2px solid transparent;
          display: flex;
          align-items: center;
          justify-content: center;
          background-color: var(--object-item-background);
          .icon {
            max-width: 76px;
            max-height: 76px;
          }
        }
        .title {
          display: block;
          text-align: center;
          font-size: var(--font-size);
          padding-top: 6px;
        }

        &:hover {
          .icon-box {
            border-color: var(--active-color);
          }
          .title {
            color: var(--active-color);
          }
        }
      }
    }
  }
}
</style>
