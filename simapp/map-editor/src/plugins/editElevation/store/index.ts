import { defineStore } from 'pinia'

interface IControlPoint {
  p: [number, number]
  cp1: [number, number]
  cp2: [number, number]
}

interface IState {
  roadId: string
  controlPoints: Array<IControlPoint>
}

const storeName = 'elevationStore'
function createInitValue (): IState {
  return {
    roadId: '',
    controlPoints: [],
  }
}

export const useElevationStore = defineStore(storeName, {
  state: (): IState => {
    return createInitValue()
  },
  getters: {},
  actions: {},
})
