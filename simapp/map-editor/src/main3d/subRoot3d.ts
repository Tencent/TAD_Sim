// import type DragControls from './controls/dragControls'
// import Base3D from './core'
// import type Axes from './helpers/axes'
// import type Grid from './helpers/gird'

// // 另一个三维场景的核心状态
// // 一个页面最多只维护2个三维核心元素的状态
// // 如果需要2个以上的三维场景，始终用当前 subRoot3d 来切换渲染
// class SubRoot3D {
//   core: Base3D
//   dragControls: DragControls
//   dom: HTMLElement
//   axes: Axes
//   grid: Grid
//   constructor() {}
//   init(params: { dom: HTMLElement }) {
//     const { dom } = params
//     this.dom = dom
//     this.core = new Base3D()
//     this.core.init({
//       dom,
//     })

//     // 初始化网格效果
//     this.initGrid()

//     this.core.render()
//   }
//   // 初始化网格
//   initGrid() {}
//   initDefaultCamera() {}
//   dispose() {}
// }
