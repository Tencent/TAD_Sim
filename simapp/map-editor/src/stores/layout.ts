import { defineStore } from 'pinia'
import { LayoutConstant } from '@/utils/common'

// 初始化时，从 localStorage 中读取默认的布局属性。如果缓存中没有则用默认值
// 左侧区域的宽度
const _leftAreaWidth: string | null = localStorage.getItem(
  LayoutConstant.leftKey,
)
const leftAreaWidth =
  _leftAreaWidth === null || Number.isNaN(Number(_leftAreaWidth)) ?
    LayoutConstant.initLeftWidth :
    Number(_leftAreaWidth)

// 右侧区域的宽度
const _rightAreaWidth: string | null = localStorage.getItem(
  LayoutConstant.rightKey,
)
const rightAreaWidth =
  _rightAreaWidth === null || Number.isNaN(Number(_rightAreaWidth)) ?
    LayoutConstant.initRightWidth :
    Number(_rightAreaWidth)

// 底部区域的高度
const _bottomAreaHeight: string | null = localStorage.getItem(
  LayoutConstant.bottomKey,
)
const bottomAreaHeight =
  _bottomAreaHeight === null || Number.isNaN(Number(_bottomAreaHeight)) ?
    LayoutConstant.initBottomHeight :
    Number(_bottomAreaHeight)

export const useLayoutStore = defineStore('layout', {
  state: () => {
    return {
      leftAreaWidth,
      rightAreaWidth,
      bottomAreaHeight,
    }
  },
  getters: {},
  actions: {
    resizeLeftAreaWidth (width: number) {
      // 左右区域的尺寸总和如果超出了整个窗口的宽度，则手动限定宽度
      if (width + this.rightAreaWidth >= window.innerWidth) {
        width = window.innerWidth - this.rightAreaWidth
      }
      this.leftAreaWidth = width
      localStorage.setItem(LayoutConstant.leftKey, String(width))
    },
    resizeRightAreaWidth (width: number) {
      // 左右区域的尺寸总和如果超出了整个窗口的宽度，则手动限定宽度
      if (width + this.leftAreaWidth >= window.innerWidth) {
        width = window.innerWidth - this.leftAreaWidth
      }
      this.rightAreaWidth = width
      localStorage.setItem(LayoutConstant.rightKey, String(width))
    },
    resizeBottomAreaHeight (height: number) {
      // TODO 目前顶部菜单栏和工具栏总共高度 100px，后续可能会调整
      const topAreaHeight = 100
      if (height + topAreaHeight >= window.innerHeight) {
        height = window.innerHeight - topAreaHeight
      }
      this.bottomAreaHeight = height
      localStorage.setItem(LayoutConstant.bottomKey, String(height))
    },
  },
})
