<template>
  <div class="sim-compass" :style="styleObject">
    <div class="sim-compass-pointer">
      <CompassPointerSvg />
    </div>
    <div class="sim-compass-north">
      <CompassNorthSvg />
    </div>
  </div>
</template>

<script>
import CompassPointerSvg from '@/assets/images/compass_pointer.svg'
import CompassNorthSvg from '@/assets/images/compass_north.svg'

export default {
  name: 'SimCompass',
  components: {
    CompassPointerSvg,
    CompassNorthSvg,
  },
  props: {
    degree: {
      type: Number,
      default: 0,
    },
  },
  computed: {
    styleObject () {
      return {
        transform: `rotate(${this.degree}deg)`,
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.sim-compass {
  border-radius: 100%;
  background-color: #333;
  text-align: center;

  .sim-compass-pointer {
    padding-top: 6px;
  }

  .sim-compass-north {
    margin-top: -10px;
  }
}
</style>
