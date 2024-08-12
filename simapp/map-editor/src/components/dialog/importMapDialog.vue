<script setup lang="ts">
// @ts-nocheck
import { computed, ref, watch } from 'vue'
import SimButton from '../common/button.vue'
import QuerySvg from './icon/quemark.svg?component'
import i18n from '@/locales'

interface IImportMap {
  name: string
  replaceFlag: boolean
  isPreset?: boolean
}

const props = defineProps({
  visible: {
    type: Boolean,
    default: false,
  },
  sameNameMaps: {
    type: Array,
    default: () => {
      return []
    },
  },
})
const emit = defineEmits(['change'])

// 初始值
const visible = ref(props.visible)
const currentMapIndex = ref(0)
// 是否应用全部同类型重名的地图导入（预设和非预设共同同一个字段，通过 isPreset 来区分阶段）
// 应用全部只针对剩余未操作的地图导入！已经编辑过的不应用
const isApplyAll = ref(false)
// 跟预设地图重名的导入地图
const presetMaps = ref([])
// 跟非预设地图重名的导入地图
const nonPresetMaps = ref([])
// 是否是处理预设地图阶段
const isPreset = ref(false)
// 最后回传给父弹窗组件的导入地图配置
const allMapOptions: Array<IImportMap> = ref([])

// 针对是否是预设地图，先对地图做划分
function sortMap (maps: Array<IImportMap>) {
  const preset = maps.filter(option => option.isPreset)
  const nonPreset = maps.filter(option => !option.isPreset)
  presetMaps.value = preset
  nonPresetMaps.value = nonPreset
}

// 重置组件内的变量
function reset () {
  presetMaps.value = []
  nonPresetMaps.value = []
  currentMapIndex.value = 0
  isPreset.value = false
  isApplyAll.value = false
  allMapOptions.value = []
}

watch(
  () => props.visible,
  (val) => {
    visible.value = val
  },
  {
    immediate: true,
  },
)
watch(
  () => props.sameNameMaps,
  (val) => {
    // 每次有要导入的地图是否替换保留判断，提前重置组件内部状态
    reset()
    sortMap(val)
  },
  {
    immediate: true,
  },
)
// 当前展示的地图信息
const currentMap = computed(() => {
  if (currentMapIndex.value < 0) return null

  if (isPreset.value) {
    return presetMaps.value[currentMapIndex.value]
  } else {
    return nonPresetMaps.value[currentMapIndex.value]
  }
})
// 针对当前地图展示的提示信息
const currentTipContent = computed(() => {
  if (!currentMap.value) return ''

  if (isPreset.value) {
    return `${i18n.global.t('desc.tips.theMap')} ${
      currentMap.value.name
    } ${i18n.global.t('desc.tips.isExistingPresetMapConfirmToKeepBoth')}`
  } else {
    return `${i18n.global.t('desc.tips.theMap')} ${
      currentMap.value.name
    } ${i18n.global.t('desc.tips.isExistingMapConfirmToReplaceIt')}`
  }
})
// 根据当前类型（区分预设和非预设）剩余未操作的地图选项数量，来判断是否展示“应用全部”的选择框
const showApplyAll = computed(() => {
  if (isPreset.value) {
    return presetMaps.value.length - currentMapIndex.value > 1
  } else {
    return nonPresetMaps.value.length - currentMapIndex.value > 1
  }
})

// 切换到预设地图处理阶段
function toggleToPreset () {
  isPreset.value = true
  currentMapIndex.value = 0
  isApplyAll.value = false

  // 如果没有要处理的预设地图重名情况，则直接提交处理结果
  if (presetMaps.value.length < 1) {
    submit()
  }
}

// 取消导入（作用于预设和非预设）
function cancel () {
  if (!isPreset.value) {
    // 先处理非预设
    if (isApplyAll.value) {
      // 直接跳过所有未处理的非预设地图，进入到预设地图处理阶段
      toggleToPreset()
    } else {
      if (currentMapIndex.value < nonPresetMaps.value.length - 1) {
        // 如果还剩余未处理的非预设地图，则递增继续
        currentMapIndex.value = currentMapIndex.value + 1
      } else {
        // 否则也切换到预设地图阶段
        toggleToPreset()
      }
    }
  } else {
    // 后处理预设
    if (isApplyAll.value) {
      // 预设地图取消后续地图的导入，则可以直接提交
      submit()
    } else {
      if (currentMapIndex.value < presetMaps.value.length - 1) {
        // 继续到下一个预设地图
        currentMapIndex.value = currentMapIndex.value + 1
      } else {
        // 最后一个预设地图处理结束，则直接提交
        submit()
      }
    }
  }
}
// 确认替换（只作用于非预设地图）
function replace () {
  if (isApplyAll.value) {
    // 如果是全部应用
    for (let i = currentMapIndex.value; i < nonPresetMaps.value.length; i++) {
      const mapOption = nonPresetMaps.value[i]
      allMapOptions.value.push({
        name: mapOption.name,
        replaceFlag: true,
      })
    }
    // 直接切换到预设地图处理阶段
    toggleToPreset()
  } else {
    // 确认替换的地图，推入到透传的数组中，replaceFlag 为 true
    allMapOptions.value.push({
      name: currentMap.value.name,
      replaceFlag: true,
    })

    if (currentMapIndex.value < nonPresetMaps.value.length - 1) {
      // 如果还剩余未处理的非预设地图，则递增继续
      currentMapIndex.value = currentMapIndex.value + 1
    } else {
      // 如果已经处理完非预设地图，则切换到处理预设地图的阶段
      toggleToPreset()
    }
  }
}
// 保留两者（作用于预设和非预设）
function keepBoth () {
  if (!isPreset.value) {
    //  先处理非预设
    if (isApplyAll.value) {
      for (let i = currentMapIndex.value; i < nonPresetMaps.value.length; i++) {
        const mapOption = nonPresetMaps.value[i]
        allMapOptions.value.push({
          name: mapOption.name,
          replaceFlag: false,
        })
      }
      // 直接切换到预设地图处理阶段
      toggleToPreset()
    } else {
      allMapOptions.value.push({
        name: currentMap.value.name,
        replaceFlag: false,
      })
      if (currentMapIndex.value < nonPresetMaps.value.length - 1) {
        // 如果还剩余未处理的非预设地图，则递增继续
        currentMapIndex.value = currentMapIndex.value + 1
      } else {
        toggleToPreset()
      }
    }
  } else {
    // 再处理预设
    if (isApplyAll.value) {
      for (let i = currentMapIndex.value; i < presetMaps.value.length; i++) {
        const mapOption = presetMaps.value[i]
        allMapOptions.value.push({
          name: mapOption.name,
          replaceFlag: false,
        })
      }

      // 全部应用完后，直接提交
      submit()
    } else {
      allMapOptions.value.push({
        name: currentMap.value.name,
        replaceFlag: false,
      })
      if (currentMapIndex.value < presetMaps.value.length - 1) {
        // 如果还剩余未处理的预设地图，则递增继续
        currentMapIndex.value = currentMapIndex.value + 1
      } else {
        submit()
      }
    }
  }
}

// 完成全部的地图替换选项调整，回传给父组件处理
function submit () {
  emit('change', allMapOptions.value)
  // 透传后重置组件内部状态
  reset()
}
</script>

<template>
  <el-dialog
    v-model="visible"
    width="340px"
    class="tip-dialog import-map-dialog"
    :close-on-click-modal="false"
    :show-close="false"
    :destroy-on-close="true"
    :close-on-press-escape="false"
    append-to-body
    align-center
  >
    <div class="title">
      <el-icon size="24px">
        <QuerySvg />
      </el-icon>
      <span>{{ $t('desc.tips.tips') }}</span>
    </div>
    <div class="tips">
      {{ currentTipContent }}
    </div>
    <div class="button-part">
      <el-checkbox
        v-if="showApplyAll"
        v-model="isApplyAll"
        class="apply-all"
        :label="$t('desc.tips.applyAll')"
      />
      <template v-if="isPreset">
        <!-- 预设地图，只展示“取消”和“确认保留”的选项 -->
        <SimButton @click="cancel">
          {{ $t('desc.tips.cancel') }}
        </SimButton>
        <SimButton class="primary" @click="keepBoth">
          {{
            $t('desc.tips.confirm')
          }}
        </SimButton>
      </template>
      <template v-else>
        <!-- 非预设地图，展示“取消”、“确认替换”、“保留两者”三种选项 -->
        <SimButton @click="keepBoth">
          {{
            $t('desc.tips.keepBoth')
          }}
        </SimButton>
        <SimButton @click="cancel">
          {{ $t('desc.tips.cancel') }}
        </SimButton>
        <SimButton class="primary" @click="replace">
          {{
            $t('desc.tips.confirm')
          }}
        </SimButton>
      </template>
    </div>
  </el-dialog>
</template>

<style lang="less">
.el-dialog.import-map-dialog {
  .el-dialog__body {
    .button-part {
      text-align: right;
      .apply-all {
        position: absolute;
        left: 16px;
        height: 24px;
        font-size: var(--font-size);

        .el-checkbox__inner {
          background-color: var(--main-dark-color);
          border-width: 2px;
          border-color: var(--property-border-color);
          width: 12px;
          height: 12px;

          &::after {
            width: 4px;
            height: 6px;
            left: 2px;
            top: 0px;
          }
        }
        .is-checked {
          .el-checkbox__inner {
            background-color: #0491aa;
            border-color: #0491aa;
          }
        }
        .el-checkbox__label {
          font-size: var(--font-size);
          color: var(--property-title-color);
        }
        &.is-checked {
          .el-checkbox__label {
            color: var(--property-title-color);
          }
        }
      }
    }
  }
}
</style>
