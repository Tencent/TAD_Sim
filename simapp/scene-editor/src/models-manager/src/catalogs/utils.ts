import type {
  Axles,
  BackEndCatalogData,
  BoundingBox,
  CatalogModel,
  CatalogParams,
  CatalogSubCategory,
  InterfaceCategory,
} from './class'
import { CatalogCategory, ReferencePoint, SubCategoryMap, categoryMap, trafficCategoryMap } from './class'

export function createCatalogModel (interfaceCategory: InterfaceCategory): CatalogModel {
  const catalogCategory: CatalogCategory = categoryMap[interfaceCategory]

  const boundingBox: BoundingBox = {
    center: {
      x: 0,
      y: 0,
      z: 0,
    },
    dimensions: {
      height: 0,
      length: 0,
      width: 0,
    },
  }
  const axles: Axles = {
    frontAxle: {
      maxSteering: 0,
      wheelDiameter: 0,
      trackWidth: 0,
      positionX: 0,
      positionZ: 0,
    },
    rearAxle: {
      maxSteering: 0,
      wheelDiameter: 0,
      trackWidth: 0,
      positionX: 0,
      positionZ: 0,
    },
  }
  const performance = {
    maxSpeed: 0,
    maxAcceleration: 0,
    maxDeceleration: 0,
  }
  const catalogSubCategory: CatalogSubCategory = SubCategoryMap[interfaceCategory][0]
  switch (catalogCategory) {
    case CatalogCategory.EGO: {
      return {
        variable: '',
        catalogCategory,
        catalogSubCategory,
        catalogParams: [{
          model3d: '',
          boundingBox,
          performance,
          axles,
          referencePoint: { origin: ReferencePoint.REAR },
          properties: {
            controller: 'none',
            interfaceCategory,
            zhName: '',
            enName: '',
            sensorGroup: 0,
            weight: 0,
            alias: '',
            preset: false,
            dynamic: undefined,
            modelId: -1,
          },
        }],
      }
    }
    case CatalogCategory.CAR: {
      return {
        variable: '',
        catalogCategory,
        catalogSubCategory,
        catalogParams: [{
          model3d: '',
          boundingBox,
          performance,
          axles,
          properties: {
            controller: '',
            sensorGroup: -1,
            preset: false,
            modelId: 0,
            zhName: '',
            enName: '',
            weight: 0,
            alias: '',
            interfaceCategory,
          },
        }],
      }
    }
    case CatalogCategory.PEDESTRIAN: {
      return {
        variable: '',
        catalogCategory,
        catalogSubCategory,
        catalogParams: {
          model3d: '',
          boundingBox,
          properties: {
            preset: false,
            modelId: 0,
            interfaceCategory,
            zhName: '',
            enName: '',
          },
        },
      }
    }
    case CatalogCategory.OBSTACLE: {
      return {
        variable: '',
        catalogCategory,
        catalogSubCategory,
        catalogParams: {
          model3d: '',
          boundingBox,
          properties: {
            interfaceCategory,
            preset: false,
            modelId: 0,
            zhName: '',
            enName: '',
          },
        },
      }
    }
  }
}

type Properties = keyof CatalogParams['properties']
type PropertyValueType = CatalogParams['properties'][Properties]
// 获取和设置模型属性
export function getModelProperty (model: CatalogModel, name: Properties): PropertyValueType {
  switch (model.catalogCategory) {
    case CatalogCategory.EGO:
    case CatalogCategory.CAR:
      return model.catalogParams[0].properties[name]
    case CatalogCategory.OBSTACLE:
    case CatalogCategory.PEDESTRIAN:
      return model.catalogParams.properties[name]
  }
}

export function findPedestrianInCatalog (data: BackEndCatalogData, modelId: number) {
  const { pedestrianList = [], vehicleList = [] } = data
  let target: CatalogModel | undefined = pedestrianList.find(p => p.catalogParams.properties.modelId === modelId)
  if (!target) {
    target = vehicleList.find((v) => {
      // pedInCarTypes里的类型在交通流里算行人，警车1算车，自动售卖车算行人
      const key = getModelProperty(v, 'interfaceCategory') as InterfaceCategory
      if (trafficCategoryMap[key] === CatalogCategory.PEDESTRIAN) {
        return v.catalogParams[0].properties.modelId === modelId
      }
      return false
    })
  }
  return target
}

export function findCarInCatalog ({ vehicleList = [] }: BackEndCatalogData, modelId: number) {
  return vehicleList.find((v) => {
    // pedInCarTypes里的不算车，自动售卖车不算车，警车1算车
    const key = getModelProperty(v, 'interfaceCategory') as InterfaceCategory
    if (trafficCategoryMap[key] === CatalogCategory.CAR) {
      return v.catalogParams[0].properties.modelId === modelId
    }
    return false
  })
}
