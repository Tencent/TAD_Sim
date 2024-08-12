/**
 * 更新状态
 * @param {object} state - 当前状态对象
 * @param {object} payload - 需要更新的状态数据
 */
export function updateState (state, payload) {
  Object.assign(state, payload)
}

/**
 * 添加对象
 * @param {object} state - 当前状态对象
 * @param {object} payload - 要添加的对象数据
 */
export function addObject (state, payload) {
  state.objects.push(payload)
  state.isDirty = true
}

/**
 * 更新对象
 * @param {object} state - 当前状态对象
 * @param {object} payload - 包含类型和数据的对象信息
 */
export function updateObject (state, payload) {
  const { type, data } = payload
  const index = state.objects.findIndex(obj => obj.type === type && obj.id === data.id)
  if (index === -1) return
  const target = { ...state.objects[index], ...data }

  state.objects.splice(index, 1, target)
  state.isDirty = true
}

/**
 * 更新场景事件
 * @param {object} state - 当前状态对象
 * @param {Array} sceneevents - 场景事件列表
 */
export function updateSceneevents (state, sceneevents) {
  state.sceneevents = sceneevents
  state.isDirty = true
}

/**
 * 删除对象
 * @param {object} state - 当前状态对象
 * @param {object} payload - 包含类型和ID的对象信息
 */
export function deleteObject (state, payload) {
  const { type, id } = payload
  const index = state.objects.findIndex((obj) => {
    return obj.type === type && obj.id === id
  })

  state.objects.splice(index, 1)
  state.isDirty = true
}

/**
 * 选择对象
 * @param {object} state - 当前状态对象
 * @param {object} payload - 包含类型和ID的对象信息
 */
export function selectObject (state, payload) {
  if (payload) {
    const { type, id } = payload
    state.selectedObjectKey = `${type}.${id}`
  } else {
    state.selectedObjectKey = ''
  }
}

/**
 * 取消选择对象
 * @param {object} state - 当前状态对象
 */
export function deselectObject (state) {
  state.selectedObjectKey = ''
}

/**
 * 切换展开分类
 * @param {object} state - 当前状态对象
 * @param {string} category - 分类名称
 */
export function toggleExpandCategory (state, category) {
  if (state.expandedKeys.includes(category)) {
    state.expandedKeys = state.expandedKeys.filter(key => key !== category)
  } else {
    state.expandedKeys.push(category)
  }
}

/**
 * 更新交通AI
 * @param {object} state - 当前状态对象
 * @param {object} payload - 要更新的交通AI数据
 */
export function updateTraffic (state, payload) {
  Object.assign(state.trafficAI, payload)
}

/**
 * 更新操作模式
 * @param {object} state - 当前状态对象
 * @param {string} payload - 操作模式
 */
export function updateOperationMode (state, payload) {
  state.operationMode = payload
}

/**
 * 更新当前场景
 * @param {object} state - 当前状态对象
 * @param {object} payload - 包含场景ID的场景信息
 */
export function updatePresentScenario (state, payload) {
  const presentScenario = state.currentScenario || state.newScenario
  let scenario = state.scenarioList.find(item => item.id === payload.id)
  scenario = scenario || payload // 没有找到则为新建的场景
  Object.assign(presentScenario, scenario)
}

/**
 * 切换跟随
 * @param {object} state - 当前状态对象
 * @param {number} id - 规划ID
 */
export function switchFollowingPlanner (state, id) {
  state.followingPlannerId = id
}
