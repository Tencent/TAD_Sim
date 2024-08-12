<template>
  <SectionBox class="element-props">
    <template #title>
      {{ $t('scenario.attribute') }}
    </template>
    <div v-if="presentScenario" class="element-props-body">
      <h3 v-if="element" class="element-title">
        {{ $itemName(element) }} {{ $t('scenario.attribute') }}
      </h3>
      <h3 v-else class="element-title">
        {{ $t('scenario.sceneAttribute') }}
      </h3>
      <div id="scenario-property" class="element-props-wrap">
        <component :is="detailView" />
      </div>
    </div>
    <div v-else class="no-property">
      {{ $t('scenario.noAttribute') }}
    </div>
  </SectionBox>
</template>

<script>
import { mapGetters } from 'vuex'
import Planner from './planner.vue'
import Car from './car.vue'
import Moto from './moto.vue'
import Bike from './bike.vue'
import Pedestrian from './pedestrian.vue'
import Obstacle from './obstacle.vue'
import Animal from './animal.vue'
import Signlight from './signlight.vue'
import Machine from './machine.vue'
import ScenarioProp from './scenario.vue'
import SectionBox from '@/components/section-box.vue'

const detailComponents = {
  planner: 'Planner',
  car: 'Car',
  moto: 'Moto',
  bike: 'Bike',
  pedestrian: 'Pedestrian',
  obstacle: 'Obstacle',
  animal: 'Animal',
  machine: 'Machine',
  signlight: 'Signlight',
}

export default {
  name: 'ElementProps',
  components: {
    SectionBox,
    Planner,
    Car,
    Moto,
    Bike,
    Pedestrian,
    Obstacle,
    Animal,
    Machine,
    Signlight,
    ScenarioProp,
  },
  data () {
    return {}
  },
  computed: {
    ...mapGetters('scenario', {
      element: 'selectedObject',
      presentScenario: 'presentScenario',
    }),
    detailView () {
      if (this.element) {
        if (this.element.type === 'sensor') {
          return detailComponents[this.element.sensorType]
        }
        return detailComponents[this.element.type]
      }
      return 'ScenarioProp'
    },
    type () {
      if (this.element?.type === 'sensor') {
        return this.element.sensorType
      }
      return this.element?.type
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.element-props {
  flex: 1;
  min-width: 100px;
}

.element-props-body {
  height: 100%;
  color: @title-font-color;
  flex: 1;
}

.element-props-wrap {
  height: calc(100% - 50px);
  overflow: auto;

  :deep(.el-collapse-item__header) {
    background-color: @hover-bg;
  }
}

.element-title {
  height: 32px;
  line-height: 32px;
  font-weight: normal;
  text-align: center;
  font-size: 12px;
  color: white;
}

.no-property {
  text-align: center;
  font-size: 12px;
  color: @disabled-color;
  line-height: 3em;
  user-select: none;
}
</style>

<style lang="less">
@import "@/assets/less/mixins";

.property-form {
  .section-head {
    font-size: 12px;
    font-weight: normal;
    height: 24px;
    color: @title-font-color;
    margin: 10px 0 0;
  }

  .el-collapse-item__content {
    padding: 10px 22px 0;
  }

  .el-form-item.param-cell {
    padding: 10px 0;
    margin-bottom: 0;

    .el-form-item__label {
      text-align: left;
    }

    .el-select {
      width: 80px;
    }

    &.multi-input {
      display: flex;
      flex-direction: column;
      .el-form-item__label,
      .el-form-item__content {
        float: none;
        display: block;
      }

      .el-form-item__content {
        margin-left: 0 !important;
      }
    }

    .form-multi-input {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      grid-column-gap: 10px;

      .input-cell {
        display: flex;
        flex-direction: column;
        align-items: center;
        margin-bottom: 10px;

        :deep(.el-input__inner) {
          width: 60px;
          height: 24px;
          line-height: 22px;
          border-radius: 2px;
          padding: 0 6px;
        }
      }
    }

    .checkbox-wrap {
      margin: 5px 0 0 -10px;
      width: 140px;

      .checkbox-tip-icon {
        margin-left: 3px;
        vertical-align: middle;
      }
    }
  }

  .property-form-item-separator {
    height: 0;
    border-bottom: 1px solid @darker-bg;
    margin: 5px 0;
  }

  .display-properties {
    .el-form-item__content {
      clear: both;
      margin: 0 !important;
      display: flex;
      justify-content: space-between;

      .sim-label {
        min-width: 60px;
      }

      &::before,
      &::after {
        display: none;
      }
    }
  }

  .solid-button {
    background: #2A2A2A;
    border: 1px solid black;
    border-radius: 2px;
    color: @global-font-color;
    padding: 1px 10px;
    line-height: 24px;

    &:hover {
      color: @active-font-color;
    }

    &:disabled {
      color: #707070;
      background-color: #404040;
      border-color: #606060;
    }
  }
}
</style>
