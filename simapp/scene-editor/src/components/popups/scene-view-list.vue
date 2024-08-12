<template>
  <div class="scene-view-list">
    <section class="part-statistics">
      <ul class="list">
        <li class="item">
          <div class="title">
            <sim-label>
              {{ $t('scenario.numberOf', { o: $t('scenario.vehicles') }) }}
              ({{ carTypes.reduce((prev, item) => prev + item.amount, 0) }}{{ $t('units.liang') }})
            </sim-label>
          </div>
          <div
            v-for="(item, index) in carTypes"
            :key="index"
            class="item-row"
          >
            <sim-label type="info" class="key">
              {{ item.name }}
            </sim-label>
            <sim-label type="info" class="value">
              {{ item.amount }}{{ $t('units.liang') }}
            </sim-label>
          </div>
        </li>
        <li class="item" style="margin: 0 12%;">
          <div class="title">
            <sim-label>
              {{ $t('scenario.numberOf', { o: $t('model.obstacle') }) }}
              ({{ barrierTypes.reduce((prev, item) => prev + item.amount, 0) }}{{ $t('units.ge') }})
            </sim-label>
          </div>
          <div
            v-for="(item, index) in barrierTypes"
            :key="index"
            class="item-row"
          >
            <sim-label type="info" class="key">
              {{ item.name }}
            </sim-label>
            <sim-label type="info" class="value">
              {{ item.amount }}{{ $t('units.ge') }}
            </sim-label>
          </div>
        </li>
        <li class="item">
          <div class="title">
            <sim-label>
              {{ $t('scenario.numberOf', { o: $t('model.pedestrian') }) }}
              ({{ pedestrianTypes.reduce((prev, item) => prev + item.amount, 0) }}{{ $t('units.ge') }})
            </sim-label>
          </div>
          <div
            v-for="(item, index) in pedestrianTypes"
            :key="index"
            class="item-row"
          >
            <sim-label type="info" class="key">
              {{ item.name }}
            </sim-label>
            <sim-label type="info" class="value">
              {{ item.amount }}{{ $t('units.ge') }}
            </sim-label>
          </div>
        </li>
      </ul>
    </section>
    <section class="part-tabs">
      <el-button class="tab">
        {{ $t('sensor.camera') }}（6{{ $t('units.ge') }}）
      </el-button>
      <el-button class="tab" style="margin: 0 20px;">
        {{ $t('sensor.lidar') }}（8{{ $t('units.ge') }}）
      </el-button>
      <el-button class="tab">
        {{ $t('sensor.radar') }}（3{{ $t('units.ge') }}）
      </el-button>
    </section>
    <section class="part-details">
      <div class="title">
        <sim-label class="cell0">
          {{ $t('sensor.serialNumber') }}
        </sim-label>
        <sim-label class="cell1">
          {{ $t('sensor.uChannels') }}
        </sim-label>
        <sim-label class="cell2">
          {{ $t('sensor.Frequency') }}
        </sim-label>
        <sim-label class="cell3">
          {{ $t('sensor.ScanDistance') }}
        </sim-label>
        <sim-label class="cell4">
          {{ $t('sensor.rotateFrequency') }}
        </sim-label>
        <sim-label class="cell5">
          {{ $t('sensor.divergenceAngle') }}
        </sim-label>
      </div>
      <div
        v-for="(item, index) in radars"
        :key="index"
        class="row-details"
      >
        <sim-label type="info" class="cell0">
          {{ index + 1 }}
        </sim-label>
        <sim-label type="info" class="cell1">
          {{ item.line }}
        </sim-label>
        <sim-label type="info" class="cell2">
          {{ item.frequency }}
        </sim-label>
        <sim-label type="info" class="cell3">
          {{ getDistance(item.distance) }}
        </sim-label>
        <sim-label type="info" class="cell4">
          {{ item.rotateFrequency }}
        </sim-label>
        <sim-label type="info" class="cell5">
          {{ item.degree }}°
        </sim-label>
      </div>
    </section>
    <section class="part-buttons">
      <el-button @click="close">
        {{ $t('operation.close') }}
      </el-button>
    </section>
  </div>
</template>

<script>
import { getSceneViewData } from '@/components/services/scene-view.vue'

export default {
  name: 'SceneViewList',
  props: {
    close: {
      type: Function,
      default () {
      },
    },
  },
  data () {
    return {
      carTypes: [],
      barrierTypes: [],
      pedestrianTypes: [],
      radars: [],
    }
  },
  mounted () {
    getSceneViewData().then((data) => {
      data.carTypes.forEach((item) => {
        this.carTypes.push(item)
      })
      data.barrierTypes.forEach((item) => {
        this.barrierTypes.push(item)
      })
      data.pedestrianTypes.forEach((item) => {
        this.pedestrianTypes.push(item)
      })
      data.radars.forEach((item) => {
        this.radars.push(item)
      })
    }).catch((reason) => {
      console.log(reason)
    })
  },
  methods: {
    getDistance (value) {
      if (value > 1000) {
        return `${Math.round(value / 1000)}km`
      }
      return `${value}m`
    },
  },
}
</script>

<style scoped lang="less">
  .scene-view-list {
    width: 550px;
    height: auto;
    padding: 0 23px 23px 23px;

    .part-statistics {
      padding: 10px 5px;

      .list {
        list-style: none;

        &:after {
          content: '';
          display: block;
          clear: both;
        }

        .item {
          float: left;
          width: 25%;
          text-align: center;

          .title {
            height: 30px;
            line-height: 30px;
          }

          .item-row {
            height: 25px;
            line-height: 25px;

            .key {
              display: inline-block;
              width: 50px;
              text-align: left;
            }

            .value {
              display: inline-block;
              width: 30px;
              text-align: right;
            }
          }
        }
      }
    }

    .part-tabs {
      margin-top: 15px;
      text-align: center;

      .tab {
        width: 160px;
        height: 28px;
        line-height: 28px;
        border-radius: 4px;
        padding: 0;
      }
    }

    .part-details {
      margin-top: 15px;
      background-color: #111;

      .title {
        height: 30px;
        line-height: 30px;
      }

      .row-details {
        height: 28px;
        line-height: 28px;
        border-top: solid 1px #2a2a2a;
        box-sizing: content-box;
      }

      .cells() {
        display: inline-block;
        text-align: center;
      }

      .cell0 {
        .cells();
        width: 12%;
      }

      .cell1 {
        .cells();
        width: 12%;
      }

      .cell2 {
        .cells();
        width: 15%;
      }

      .cell3 {
        .cells();
        width: 20%;
      }

      .cell4 {
        .cells();
        width: 20%;
      }

      .cell5 {
        .cells();
        width: 20%;
      }
    }

    .part-buttons {
      margin-top: 20px;
      direction: rtl;
      height: 40px;
      line-height: 40px;
    }
  }
</style>
