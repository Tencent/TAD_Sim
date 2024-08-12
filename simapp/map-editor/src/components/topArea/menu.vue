<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref } from 'vue'
import FileMenu from './fileMenu.vue'
import EditMenu from './editMenu.vue'
import { globalConfig } from '@/utils/preset'

const currentTab = ref('')
const showMenuList = ref(false)

const isFileMenuListShow = computed(() => {
  return currentTab.value === 'file'
})
const isEditMenuListShow = computed(() => {
  return currentTab.value === 'edit'
})

// 如果是云端环境，则不显示 edit 菜单栏
const { isCloud } = globalConfig

function handleMeneClick () {
  showMenuList.value = !showMenuList.value
}
function handleMouseEnter (tabName: string) {
  currentTab.value = tabName
}
function handleMouseLeave () {
  if (!showMenuList.value) {
    currentTab.value = ''
  }
}

function globalClick () {
  showMenuList.value = false
  handleMouseLeave()
}
onMounted(() => {
  window.addEventListener('click', globalClick)
})

onBeforeUnmount(() => {
  window.removeEventListener('click', globalClick)
})
</script>

<template>
  <ul class="menu-ul">
    <li
      v-if="!isCloud"
      class="menu-li"
      :class="{ active: isFileMenuListShow }"
      @click.stop="handleMeneClick"
      @mouseenter="handleMouseEnter('file')"
      @mouseleave="handleMouseLeave"
    >
      <label>{{ $t('menu.file') }}</label>
      <FileMenu
        v-show="showMenuList && isFileMenuListShow"
        class="folded-menu"
      />
    </li>
    <li
      class="menu-li"
      :class="{ active: isEditMenuListShow }"
      @click.stop="handleMeneClick"
      @mouseenter="handleMouseEnter('edit')"
      @mouseleave="handleMouseLeave"
    >
      <label>{{ $t('menu.edit') }}</label>
      <EditMenu
        v-show="showMenuList && isEditMenuListShow"
        class="folded-menu"
      />
    </li>
    <!-- <li class="menu-li">
      <label>{{ $t('menu.view') }}</label>
    </li> -->
  </ul>
</template>

<style scoped lang="less">
.menu-ul {
  display: flex;
  align-items: center;
  font-size: 14px;

  .menu-li {
    padding: 0 6px;
    cursor: pointer;
    position: relative;

    &.active {
      color: var(--active-color);
      background-color: var(--menu-transparent-background);
    }
    &:hover {
      color: var(--active-color);
      background-color: var(--menu-transparent-background);
    }

    label {
      cursor: pointer;
    }

    .folded-menu {
      position: absolute;
      left: 0;
      z-index: 1;
      background-color: var(--component-area-color);
      width: 140px;
    }
  }
}
</style>
