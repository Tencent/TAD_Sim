import ajax from '@/api/axios'

const baseUrl = 'https://api.tapd.cn'
const workspaceId = '46419970'
const authorization = 'Basic TkZVbklIVVE6RUYzNjM0M0YtNDdFNC1CQzg5LUUwQzQtREZGREIzMzE5OTEz'

/**
 * 封装 GET 请求，添加 authorization 请求头
 * @param {string} url - 请求的 URL
 * @returns {Promise} 返回一个 GET 请求结果的 Promise 对象
 */
function get (url) {
  return ajax.get(url, {
    headers: {
      Authorization: authorization,
    },
  })
}

/**
 * 获取测试计划列表
 * @returns {Promise} 返回一个包含测试计划列表的 Promise 对象
 */
export const getTestPlanList = function () {
  const url = `${baseUrl}/test_plans?workspace_id=${workspaceId}`
  return get(url)
}

/**
 * 获取指定测试计划的测试用例列表
 * @param {number} id - 测试计划的 ID
 * @returns {Promise} 返回一个包含指定测试计划的测试用例列表的 Promise 对象
 */
export const getTestPlanTcaseList = function (id) {
  const url = `${baseUrl}/test_plans/get_test_plan_tcase?workspace_id=${workspaceId}&test_plan_id=${id}`
  return get(url)
}

/**
 * 获取指定 ID 的测试用例信息
 * @param {number} id - 测试用例的 ID
 * @returns {Promise} 返回一个包含指定 ID 测试用例信息的 Promise 对象
 */
export const getTcase = async function (id) {
  const url = `${baseUrl}/tcases?workspace_id=${workspaceId}&id=${id}`
  return get(url)
}
