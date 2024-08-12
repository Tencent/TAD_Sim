<script setup lang="ts">
// @ts-nocheck
import { computed, ref, watch } from 'vue'
import ElementItem from './elementItem.vue'
import RoadSvg from './elementIcon/road.svg?component'
import PoleSvg from './elementIcon/pole.svg?component'
import TrafficLightSvg from './elementIcon/light.svg?component'
import SignboardSvg from './elementIcon/board.svg?component'
import SensorSvg from './elementIcon/sensor.svg?component'
import { useRoadStore } from '@/stores/road'
import { useJunctionStore } from '@/stores/junction'
import { getJunction, getObject, getRoad } from '@/utils/mapCache'
import { useObjectStore } from '@/stores/object'

const roadStore = useRoadStore()
const junctionStore = useJunctionStore()
const objectStore = useObjectStore()

// 跟道路、路口、杆容器关联的物体映射
const roadMap = ref({})
const junctionMap = ref({})
const poleMap = ref({})

const roads = computed(() => {
  return roadStore.ids.map(id => getRoad(id)).filter(road => road)
})
const junctions = computed(() => {
  return junctionStore.ids
    .map(id => getJunction(id))
    .filter(junction => junction)
})

// 监听物体 ids 数组的变换，计算跟道路或是路口的关联映射关系
watch(
  () => objectStore.ids,
  (val) => {
    // 计算
    if (val.length < 1) {
      roadMap.value = []
      junctionMap.value = []
      return
    }

    // 初始化局部的对象
    const _roads = {}
    const _junctions = {}
    const _poles = {}

    // 遍历所有的物体列表
    for (const objectId of val) {
      const object = getObject(objectId)
      if (!object) continue

      // 杆容器判断优先级最高
      if (object.poleId) {
        const poleKey = `pole-${object.poleId}`
        // 如果当前杆的子属性没有定义，则初始化一个默认值
        if (!_poles[poleKey]) {
          _poles[poleKey] = {
            signalBoard: [],
            trafficLight: [],
            sensor: [],
          }
        }
        switch (object.mainType) {
          case 'signalBoard':
            _poles[poleKey].signalBoard.push({
              id: objectId,
              name: object.showName,
            })
            break
          case 'trafficLight':
            _poles[poleKey].trafficLight.push({
              id: objectId,
              name: object.showName,
            })
            break
          case 'sensor':
            _poles[poleKey].sensor.push({
              id: objectId,
              name: object.showName,
            })
            break
          default:
            break
        }
        // 判断完 pole 就跳到下一个
        continue
      }

      if (object.roadId) {
        const roadKey = `road-${object.roadId}`
        // 如果当前道路的子属性没有定义，则初始化一个默认值
        if (!_roads[roadKey]) {
          _roads[roadKey] = {
            pole: [],
            roadSign: [],
            other: [],
            customModel: [], // 自定义模型
          }
        }
        switch (object.mainType) {
          case 'parkingSpace':
          case 'roadSign': {
            _roads[roadKey].roadSign.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          case 'pole': {
            _roads[roadKey].pole.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          case 'other': {
            _roads[roadKey].other.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          case 'customModel': {
            _roads[roadKey].customModel.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          default:
            break
        }
      }

      if (object.junctionId) {
        const junctionKey = `junction-${object.junctionId}`
        // 如果当前路口的子属性没有定义，则初始化一个默认值
        if (!_junctions[junctionKey]) {
          _junctions[junctionKey] = {
            pole: [],
            roadSign: [],
            other: [],
            customModel: [],
          }
        }
        switch (object.mainType) {
          case 'parkingSpace':
          case 'roadSign': {
            _junctions[junctionKey].roadSign.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          case 'pole': {
            _junctions[junctionKey].pole.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          case 'other': {
            _junctions[junctionKey].other.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          case 'customModel': {
            _junctions[junctionKey].customModel.push({
              id: objectId,
              name: object.showName,
            })
            break
          }
          default:
            break
        }
      }
    }

    roadMap.value = _roads
    junctionMap.value = _junctions
    poleMap.value = _poles
  },
  {
    immediate: true,
    deep: true,
  },
)

const poleChildState = computed(() => {
  const childState = {}

  for (const poleKey in poleMap.value) {
    const onePoleOption = poleMap.value[poleKey]
    childState[poleKey] =
      onePoleOption.signalBoard.length > 0 ||
      onePoleOption.trafficLight.length > 0 ||
      onePoleOption.sensor.length > 0
  }

  return childState
})
</script>

<template>
  <div class="structure-tree">
    <div class="title">
      {{ $t('desc.mapElement') }}
    </div>
    <div class="content">
      <ElementItem
        v-for="road of roads"
        :id="`road-${road.id}`"
        :key="`road-${road.id}`"
        :icon="RoadSvg"
        :is-active="false"
        :name="`${$t(`desc.road`)} ${road.id}`"
        :has-child="false"
      >
        <template v-if="roadMap[`road-${road.id}`]">
          <ElementItem
            v-for="poleOption of roadMap[`road-${road.id}`].pole"
            :id="`pole-${poleOption.id}`"
            :key="`pole-${poleOption.id}`"
            :icon="PoleSvg"
            :is-active="false"
            :name="`${poleOption.name} ${poleOption.id}`"
            :has-child="poleChildState[`pole-${poleOption.id}`]"
          >
            <template v-if="poleMap[`pole-${poleOption.id}`]">
              <template v-if="poleMap[`pole-${poleOption.id}`].trafficLight">
                <ElementItem
                  v-for="trafficLight of poleMap[`pole-${poleOption.id}`]
                    .trafficLight"
                  :id="`trafficLight-${trafficLight.id}`"
                  :key="`trafficLight-${trafficLight.id}`"
                  :icon="TrafficLightSvg"
                  :is-active="false"
                  :name="`${trafficLight.name} ${trafficLight.id}`"
                  :has-child="false"
                />
              </template>
              <template v-if="poleMap[`pole-${poleOption.id}`].signalBoard">
                <ElementItem
                  v-for="signalBoard of poleMap[`pole-${poleOption.id}`]
                    .signalBoard"
                  :id="`signalBoard-${signalBoard.id}`"
                  :key="`signalBoard-${signalBoard.id}`"
                  :icon="SignboardSvg"
                  :is-active="false"
                  :name="`${signalBoard.name} ${signalBoard.id}`"
                  :has-child="false"
                />
              </template>
              <template v-if="poleMap[`pole-${poleOption.id}`].trafficLight">
                <ElementItem
                  v-for="sensor of poleMap[`pole-${poleOption.id}`].sensor"
                  :id="`sensor-${sensor.id}`"
                  :key="`sensor-${sensor.id}`"
                  :icon="SensorSvg"
                  :is-active="false"
                  :name="`${sensor.name} ${sensor.id}`"
                  :has-child="false"
                />
              </template>
            </template>
          </ElementItem>
          <ElementItem
            v-for="roadSign of roadMap[`road-${road.id}`].roadSign"
            :id="`roadSign-${roadSign.id}`"
            :key="`roadSign-${roadSign.id}`"
            :icon="RoadSvg"
            :is-active="false"
            :name="`${roadSign.name} ${roadSign.id}`"
            :has-child="false"
          />
          <ElementItem
            v-for="other of roadMap[`road-${road.id}`].other"
            :id="`other-${other.id}`"
            :key="`other-${other.id}`"
            :icon="RoadSvg"
            :is-active="false"
            :name="`${other.name} ${other.id}`"
            :has-child="false"
          />
          <ElementItem
            v-for="customModel of roadMap[`road-${road.id}`].customModel"
            :id="`customModel-${customModel.id}`"
            :key="`customModel-${customModel.id}`"
            :icon="RoadSvg"
            :is-active="false"
            :name="`${customModel.name} ${customModel.id}`"
            :has-child="false"
          />
        </template>
      </ElementItem>
      <ElementItem
        v-for="junction of junctions"
        :id="`junction-${junction.id}`"
        :key="`junction-${junction.id}`"
        :icon="RoadSvg"
        :is-active="false"
        :name="`${$t(`desc.junction`)} ${junction.id}`"
        :has-child="false"
      >
        <template v-if="junctionMap[`junction-${junction.id}`]">
          <ElementItem
            v-for="poleOption of junctionMap[`junction-${junction.id}`].pole"
            :id="`pole-${poleOption.id}`"
            :key="`pole-${poleOption.id}`"
            :icon="PoleSvg"
            :is-active="false"
            :name="`${poleOption.name} ${poleOption.id}`"
            :has-child="poleChildState[`pole-${poleOption.id}`]"
          >
            <template v-if="poleMap[`pole-${poleOption.id}`]">
              <template v-if="poleMap[`pole-${poleOption.id}`].trafficLight">
                <ElementItem
                  v-for="trafficLight of poleMap[`pole-${poleOption.id}`]
                    .trafficLight"
                  :id="`trafficLight-${trafficLight.id}`"
                  :key="`trafficLight-${trafficLight.id}`"
                  :icon="TrafficLightSvg"
                  :is-active="false"
                  :name="`${trafficLight.name} ${trafficLight.id}`"
                  :has-child="false"
                />
              </template>
              <template v-if="poleMap[`pole-${poleOption.id}`].signalBoard">
                <ElementItem
                  v-for="signalBoard of poleMap[`pole-${poleOption.id}`]
                    .signalBoard"
                  :id="`signalBoard-${signalBoard.id}`"
                  :key="`signalBoard-${signalBoard.id}`"
                  :icon="SignboardSvg"
                  :is-active="false"
                  :name="`${signalBoard.name} ${signalBoard.id}`"
                  :has-child="false"
                />
              </template>
              <template v-if="poleMap[`pole-${poleOption.id}`].trafficLight">
                <ElementItem
                  v-for="sensor of poleMap[`pole-${poleOption.id}`].sensor"
                  :id="`sensor-${sensor.id}`"
                  :key="`sensor-${sensor.id}`"
                  :icon="SensorSvg"
                  :is-active="false"
                  :name="`${sensor.name} ${sensor.id}`"
                  :has-child="false"
                />
              </template>
            </template>
          </ElementItem>
          <ElementItem
            v-for="roadSign of junctionMap[`junction-${junction.id}`].roadSign"
            :id="`roadSign-${roadSign.id}`"
            :key="`roadSign-${roadSign.id}`"
            :icon="RoadSvg"
            :is-active="false"
            :name="`${roadSign.name} ${roadSign.id}`"
            :has-child="false"
          />
          <ElementItem
            v-for="other of junctionMap[`junction-${junction.id}`].other"
            :id="`other-${other.id}`"
            :key="`other-${other.id}`"
            :icon="RoadSvg"
            :is-active="false"
            :name="`${other.name} ${other.id}`"
            :has-child="false"
          />
          <ElementItem
            v-for="customModel of junctionMap[`junction-${junction.id}`]
              .customModel"
            :id="`customModel-${customModel.id}`"
            :key="`customModel-${customModel.id}`"
            :icon="RoadSvg"
            :is-active="false"
            :name="`${customModel.name} ${customModel.id}`"
            :has-child="false"
          />
        </template>
      </ElementItem>
    </div>
  </div>
</template>

<style scoped lang="less">
.structure-tree {
  flex: 1 1;
  position: relative;
  width: 100%;
  min-height: 100px;

  --title-height: 24px;
  --padding: 12px;

  .title {
    height: var(--title-height);
    line-height: var(--title-height);
    padding: 0 var(--padding);
    background-color: var(--main-dark-color);
  }
  .content {
    position: absolute;
    top: var(--title-height);
    bottom: 0;
    left: 0;
    right: 0;
    overflow: auto;
    padding-right: var(--padding);
  }
}
</style>
