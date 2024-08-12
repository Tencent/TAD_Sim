/**
 * @property objects 当前场景里的元素列表
 * @property {'', 'type.id', 'multiple.ids'} selectedObjectKey 已选中元素key，'' 未选中，'type.id' 单选，'multiple.ids' 多选
 * @property expandedKeys 元素树中展开的目录
 * @property scenarioList 场景列表
 * @property openHistory 打开的场景历史
 * @property isDirty 场景是否已被改动
 * @property {'playing'|'modifying'|'static'|'renderPlaying'|'mapEditor'} status 当前状态 renderPlaying 与 mapEditor暂未支持
 * @property currentScenario 当前的场景（正在编辑的和正在播放的）
 * @property newScenario 新建的场景，为了和当前的场景区分开（主要是由于新建时不需要openScenario）
 * @property {'uninitialized' | 'ready' | 'playing'} playingStatus 播放模式下内部状态
 * @property {'single' | 'list'} playingMode 播放模式下内部模式
 * @property {'top' | 'fps' | 'third' | 'debugging'} currentPerspective 当前视角
 */
export default {
  objects: [],
  selectedObjectKey: '',
  expandedKeys: [],
  scenarioList: [],
  openHistory: [],
  isDirty: false,
  status: 'modifying',
  currentScenario: null,
  startPlaylistScenarioId: -1,
  newScenario: null,
  generateInfo: null,
  currentMap: null,
  timestamp: -1,
  times: 1,
  loading: false,
  roadObj: {},
  sceneevents: [],
  // plannerEvents: [],

  playingStatus: 'uninitialized',
  playingMode: 'single',
  playingTimes: 1,
  // 播放中跟踪的主车id
  followingPlannerId: null,
  shouldPlayAfterSetup: false,
  isPaused: false,

  currentPerspective: 'top',

  currentPerspectivePlaying: 'top',

  supportedPerspectives: [
    'top',
    'fps',
    'third',
    'debugging',
  ],
  supportedPerspectivesModifying: [
    'top',
    'debugging',
  ],
  trafficAI: {
    averageSpeed: 15,
    trafficDensity: 0.5,
    radicalDegree: 0.5,
    carTypes: [
      {
        name: 'scenarioElements.Cars',
        weight: 40,
      },
      {
        name: 'scenarioElements.SUV',
        weight: 25,
      },
      {
        name: 'scenarioElements.smallTrucks',
        weight: 15,
      },
      {
        name: 'scenarioElements.Taxi',
        weight: 20,
      },
    ],
  },
  factor: 1,
  clipboard: {
    planner: null,
    car: null,
    moto: null,
    bike: null,
    pedestrian: null,
    animal: null,
    obstacle: null,
    machine: null,
    signlight: null,
  },

  operationMode: 1,
}
