<template>
  <div class="signlight-config">
    <section class="part-left">
      <SignalControlLeftBar />
    </section>
    <SignlightMap ref="map" class="part-map" @click-light="onClickLight" />
    <section class="part-right">
      <SignalControlElements />
      <SignalControlProps />
      <div class="buttons">
        <el-button @click="handleSave">
          {{ $t('operation.save') }}
        </el-button>
        <el-button @click="handleApply">
          {{ $t('operation.apply') }}
        </el-button>
      </div>
    </section>
  </div>
</template>

<script>
import { cloneDeep } from 'lodash-es'
import { mapActions, mapGetters, mapState } from 'vuex'
import SignalControlLeftBar from '../signal-control/signal-control-left-bar.vue'
import SignalControlElements from '../signal-control/signal-control-elements.vue'
import SignalControlProps from '../signal-control/signal-control-props.vue'
import SignlightMap from '@/components/popups/signlight-map/index.vue'
import { editor } from '@/api/interface'
import { getColorByTime, getSignlightColorData } from '@/components/signal-control/common/utils'
import { getPermission } from '@/utils/permission'

export default {
  name: 'SignlightConfig',
  components: {
    SignlightMap,
    SignalControlLeftBar,
    SignalControlElements,
    SignalControlProps,
  },
  props: {},
  data () {
    return {}
  },
  computed: {
    ...mapState('signalControl', [
      'currentJunctionId',
      'currentPhaseId',
      'isDirty',
    ]),
    ...mapGetters('signalControl', [
      'allSignlights',
      'currentPlanId',
      'currentJunction',
      'currentPhase',
      'planJunctionPhaseList',
      'allRoutesData',
      'isSystemPlan',
    ]),
  },
  watch: {
    // 当前信控配置变化时，重新渲染方向灯
    currentPlanId () {
      this.showSignlights()
    },
    // 当前路口变化时，重新渲染方向灯
    currentJunctionId () {
      const $map = this.$refs.map
      if ($map && $map.signlights) {
        $map.focusToJunction(this.currentJunctionId)
      }
    },
    // 当前路口变化时，刷新方向灯颜色
    currentJunction () {
      this.changeSignlightColors()
    },
    // 当前相位变化时，框选该相位对应方向灯
    currentPhase () {
      // TODO: 切换信控配置项时，会先触发此处，报未找到 signlights，后续需优化
      this.highlightSignlights()
    },
  },
  async created () {
  },
  mounted () {
    this.showSignlights(this.currentJunctionId)
  },
  methods: {
    ...mapActions('signalControl', [
      'selectJunction',
      'selectPhase',
      'bindSignlight',
      'validateSignlightPlans',
      'saveSignlightPlans',
      'applySignlightPlans',
    ]),
    async beforeClose () {
      if (this.isDirty) {
        await this.$confirmBox(this.$t('tips.saveSignlightPlanWithoutSaving'))
      }
      return true
    },
    // 点击保存
    async handleSave () {
      const valid = await this.validateSignlightPlans()
      if (!valid) return

      await this.saveSignlightPlans()

      // tip: 信控配置保存成功
      const tips = this.$t('scenario.signalControlSetting') + this.$t('operation.save') + this.$t('tips.success')
      this.$message.success(tips)
      this.$emit('close')
    },
    // 点击应用
    async handleApply () {
      const valid = await this.validateSignlightPlans()
      if (!valid) return

      await this.applySignlightPlans(this.currentPlanId)
      // tip: 应用成功
      const tips = this.$t('operation.apply') + this.$t('tips.success')
      this.$message.success(tips)
    },
    // 点击地图中的方向灯
    onClickLight (signlightId) {
      const canModify = getPermission('action.signlightControl.phase.settings.enable')
      if (!canModify) return

      console.log(signlightId)
      // 若点中方向灯反选到相位
      // 默认配置可随意反选到相位，可编辑配置仅其他路口可反选到相位（因为本路口的交互逻辑为勾选语义灯）
      const triggerSignlight = this.allSignlights.find(e => e.id === signlightId)
      if (this.isSystemPlan || this.currentPhase.junctionid !== triggerSignlight.junction) {
        this.selectJunction(triggerSignlight.junction)
        this.selectPhase(`${triggerSignlight.junction}+${triggerSignlight.phaseNumber}`)
        return
      }
      // 系统信控配置不可编辑
      if (this.isSystemPlan) return
      // 选中方向灯，修改相位与方向灯的绑定关系
      this.bindSignlight({ id: signlightId })
    },
    // 切换路口时，显示当前路口的语义灯
    async showSignlights () {
      const $map = this.$refs.map
      let signlights = this.allSignlights || []
      if ($map && $map.signlights) {
        $map.hideAllSignlights()
        $map.focusToJunction(this.currentJunctionId)
        if (signlights && signlights.length) {
          signlights = signlights.filter(item => item.plan === this.currentPlanId).filter(item => !item.isTemp)
          const routes = editor.scenario.getSignlightPlans().routesData
          await $map.showSignlights({ signlights, routes: cloneDeep(routes) })
          this.changeSignlightColors()
          setTimeout(() => { // 必须延时，否则边框位置错误
            this.highlightSignlights()
          }, 200)
        }
      }
    },
    // 切换相位时，高亮该相位绑定的语义灯
    highlightSignlights () {
      const $map = this.$refs.map
      let signlights = this.currentPhase?.signlights || []
      if ($map && $map.signlights && $map.signlights.length) {
        $map.lowlightAllSignlights()
        if (signlights && signlights.length) {
          signlights = signlights.filter(item => !item.isTemp)
          const ids = signlights.map(item => item.id)
          $map.highlightSignlights(ids)
        }
      }
    },
    // 修改 phase 数据时，修改语义灯颜色
    changeSignlightColors () {
      const $map = this.$refs.map
      let signlights = this.allSignlights || []
      if ($map?.signlights?.length) {
        if (signlights?.length) {
          signlights = signlights.filter(item => item.plan === this.currentPlanId).filter(item => !item.isTemp)
          const grayLights = signlights.map(item => ({ id: item.id, color: 'gray' }))
          $map.changeSignlightColors(grayLights)
          const enabledLight = signlights.filter(item => item.phaseNumber)
          const realLight = enabledLight.map((item) => {
            const colorData = getSignlightColorData(item)
            const color = getColorByTime(colorData, 0) // 取每个语义灯的灯态配置在 0s 时的颜色
            return { id: item.id, color, disabled: !item.enabled }
          })
          $map.changeSignlightColors(realLight)
        }
      }
    },
  },
}
</script>

<style scoped lang="less">
@import "@/assets/less/mixins";

.signlight-config {
  // width: 1160px;
  height: 100%;
  display: flex;

  .part-left {
    flex-shrink: 0;
    width: 240px;
    display: flex;
  }

  .part-map {
    flex: 1;
    min-width: 1px;
  }

  .part-right {
    flex-shrink: 0;
    width: 400px;
    display: flex;
    flex-direction: column;
  }

  .buttons {
    padding: 10px 28px;
    display: flex;
    justify-content: flex-end;
  }
}
</style>
