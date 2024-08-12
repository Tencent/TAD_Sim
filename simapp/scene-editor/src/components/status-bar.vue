<template>
  <div class="status-bar">
    <div class="scenario-info">
      <template v-if="!!presentScenario">
        <span id="scenarioName" class="scenario-name">
          <span v-if="isDirty" id="isDirty" class="is-dirty">*</span>
          <span :title="scenarioFullname">{{ $t('scenario.name') }}: {{ scenarioFullname }}</span>
        </span>
        <span id="mapName" class="map-name" :title="mapName">{{ $t('scenario.MapName') }}: {{ mapName }}</span>
      </template>
    </div>
    <div v-show="position && isModifying" class="position-info">
      <span id="locationX" class="location-cell">x: {{ locationX }}</span>
      <span id="locationY" class="location-cell">y: {{ locationY }}</span>
      <span id="locationLon" class="location-cell">lon: {{ lon }}</span>
      <span id="locationLat" class="location-cell">lat: {{ lat }}</span>
    </div>
    <div class="current-planner">
      <span>当前主车：</span>
      <span>{{ currentPlannerName }}</span>
    </div>
  </div>
</template>

<script>
import { get } from 'lodash-es'
import { mapGetters, mapState } from 'vuex'
import { editor, player } from '@/api/interface'
import filters from '@/filters'

export default {
  name: 'StatusBar',
  data () {
    return {
      mapName: '',
      position: {},
    }
  },
  computed: {
    ...mapGetters('scenario', [
      'presentScenario',
      'isModifying',
      'isPlaying',
      'selectedObject',
    ]),
    ...mapState('scenario', [
      'isDirty',
      'followingPlannerId',
    ]),
    scenarioFullname () {
      return this.presentScenario.filename || `${this.presentScenario.name}.${this.presentScenario.type}`
    },
    locationX () {
      return get(this.position, 'x', '')
    },
    locationY () {
      return get(this.position, 'y', '')
    },
    lon () {
      return get(this.position, 'lon', '')
    },
    lat () {
      return get(this.position, 'lat', '')
    },
    currentPlannerName () {
      if (this.isPlaying) {
        if (this.followingPlannerId !== null) {
          return `Ego_${`${this.followingPlannerId}`.padStart(3, '0')}`
        }
      } else if (this.selectedObject?.type === 'planner') {
        return filters.$itemName(this.selectedObject)
      }
      return ''
    },
  },
  watch: {
    presentScenario (newValue) {
      if (newValue) {
        let mapInfo
        if (this.isPlaying) {
          mapInfo = player.scenario.getMapInfo()
        } else {
          mapInfo = editor.scenario.getMapInfo()
        }
        console.log(mapInfo)
        this.mapName = get(mapInfo, 'mapName')
      }
    },
  },
  mounted () {
    editor.addEventListener('positionChanged', this.positionChanged)
  },
  beforeUnmount () {
    editor.removeEventListener('positionChanged', this.positionChanged)
  },
  methods: {
    positionChanged (payload) {
      this.position = payload
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.status-bar {
  color: @title-font-color;
  position: absolute;
  top: 0;
  left: 0;
  z-index: 10;
  box-sizing: border-box;
  width: 100%;
  height: 24px;
  flex: 0 0 24px;
  display: flex;
  font-size: 12px;
  line-height: 24px;
  background-color: @darker-bg;
  opacity: .8;
}

.scenario-info {
  flex: 1;
  padding-left: 22px;
  margin-right: 22px;
  display: flex;
  overflow: hidden;
}

.position-info {
  flex: 0 0 380px;
}

.current-planner {
  flex: 0 0 128px;
}

.scenario-name {
  flex-shrink: 1;
  margin-right: 20px;
  .text-overflow;
}

.map-name {
  flex-shrink: 1;
  .text-overflow;
}

.location-cell {
  display: inline-block;
  min-width: 60px;
  margin-right: 15px;
}

.is-dirty {

}
</style>
