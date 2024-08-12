import axios from 'axios'
import { cloneDeep } from 'lodash-es'

/**
 * L3状态机
 */
class L3StateMachine {
  constructor () {
    this.statemachinerawdata = null
    this.statesData = []
  }

  getL3StateMachineRawData () {
    return this.statemachinerawdata
  }

  getL3StatesData () {
    return this.statesData
  }

  /**
   * 设置L3状态机
   * @param states
   */
  setL3StatesData (states) {
    if (states == null || states == undefined) {
      this.statesData = []
    } else {
      this.statesData = states
    }
  }

  /**
   * 获取L3状态机
   * @returns {Promise<AxiosResponse<any>>}
   */
  getL3StateMachine () {
    const _this = this
    return axios({
      method: 'get',
      url: L3StateMachine.ConfigURL,
      withCredentials: true,
    })
      .then((data, textStatus, jqXHR) => {
        const statemachine = data.data.l3statemachine
        if (statemachine != undefined) {
          _this.statemachinerawdata = {}
          _this.statemachinerawdata = cloneDeep(statemachine)
        }
        return data.data
      })
  }
}

L3StateMachine.ConfigURL = 'http://localhost:9000/l3statemachineinfo'

export default L3StateMachine
