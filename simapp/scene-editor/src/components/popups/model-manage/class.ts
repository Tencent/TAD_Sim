export interface Dimensions {
  height: number
  length: number
  width: number
}

export interface BoundingBox {
  center: {
    x: number
    y: number
    z: number
  }
  dimensions: Dimensions
  trailerOffsetX?: number
  trailerOffsetZ?: number
}

interface Performance {
  maxSpeed: number
  maxAcceleration: number
  maxDeceleration: number
}

interface Axle {
  maxSteering: number
  wheelDiameter: number
  trackWidth: number
  positionX: number
  positionZ: number
}

export interface Axles {
  frontAxle: Axle
  rearAxle: Axle
}

export enum CatalogCategory {
  EGO = 'ego',
  CAR = 'car',
  PEDESTRIAN = 'pedestrian',
  OBSTACLE = 'obstacle',
}

export enum InterfaceCategory {
  EGO = 'ego',
  CAR = 'car',
  PEDESTRIAN = 'pedestrian',
  OBSTACLE = 'obstacle',
  MOTO_BIKE = 'moto',
  BIKE = 'bike',
  ANIMAL = 'animal',
  MACHINE = 'machine',
}

const EgoSubCategories = ['car', 'van', 'truck', 'trailer', 'semitrailer', 'bus']
const CarSubCategories = [
  'car',
  'van',
  'truck',
  'trailer',
  'semitrailer',
  'bus',
  'train',
  'tram',
]
const MotoBikeSubCategories = ['motorbike']
const BicycleSubCategories = ['bicycle']
const PedestrianSubCategories = ['pedestrian']
const AnimalSubCategories = ['animal']
const MachineSubCategories = ['trailer']
const ObstacleSubCategories = [
  'obstacle',
  'pole',
  'tree',
  'vegetation',
  'barrier',
  'railing',
  'building',
  'streetLamp',
  'gantry',
  'roadMark',
]

export const SubCategoryMap = {
  [InterfaceCategory.EGO]: EgoSubCategories,
  [InterfaceCategory.CAR]: CarSubCategories,
  [InterfaceCategory.MOTO_BIKE]: MotoBikeSubCategories,
  [InterfaceCategory.BIKE]: BicycleSubCategories,
  [InterfaceCategory.PEDESTRIAN]: PedestrianSubCategories,
  [InterfaceCategory.ANIMAL]: AnimalSubCategories,
  [InterfaceCategory.OBSTACLE]: ObstacleSubCategories,
  [InterfaceCategory.MACHINE]: MachineSubCategories,
}

export type EgoSubCategory = typeof EgoSubCategories[number]
export type CarSubCategory = typeof CarSubCategories[number]
export type MotoBikeSubCategory = typeof MotoBikeSubCategories[number]
export type BicycleSubCategory = typeof BicycleSubCategories[number]
export type PedestrianSubCategory = typeof PedestrianSubCategories[number]
export type AnimalSubCategory = typeof AnimalSubCategories[number]
export type MachineSubCategory = typeof MachineSubCategories[number]
export type ObstacleSubCategory = typeof ObstacleSubCategories[number]

export type CatalogSubCategory =
  EgoSubCategory
  | CarSubCategory
  | PedestrianSubCategory
  | ObstacleSubCategory
  | MotoBikeSubCategory
  | BicycleSubCategory
  | AnimalSubCategory
  | MachineSubCategory

export enum ReferencePoint {
  FRONT = 'FrontAxle_SnappedToGround',
  REAR = 'RearAxle_SnappedToGround',
  CENTER = 'WheelBaseCenter_SnappedToGround',
  TRAILER_HITCH = 'TrailerHitch_SnappedToGround',
}

export interface CatalogParams {
  model3d: string
  boundingBox: BoundingBox
  performance?: Performance
  axles?: Axles
  referencePoint?: {
    origin: ReferencePoint
  }
  properties: {
    controller?: string
    interfaceCategory: InterfaceCategory
    zhName?: string
    enName?: string
    sensorGroup?: number
    weight?: number
    alias?: string
    preset: boolean
    dynamic?: number
    modelId: number
  }
}

interface Model {
  variable: string
  // 放在哪个Catalog里
  catalogCategory: CatalogCategory
  // Catalog子类型
  catalogSubCategory: CatalogSubCategory
}

export interface EgoModel extends Model {
  catalogCategory: CatalogCategory.EGO
  catalogParams: Array<CatalogParams>
}

export interface CarModel extends Model {
  catalogCategory: CatalogCategory.CAR
  catalogParams: Array<CatalogParams>
}

export interface PedestrianModel extends Model {
  catalogCategory: CatalogCategory.PEDESTRIAN
  catalogParams: CatalogParams
}

export interface ObstacleModel extends Model {
  catalogCategory: CatalogCategory.OBSTACLE
  catalogParams: CatalogParams
}

export type CatalogModel = EgoModel | CarModel | PedestrianModel | ObstacleModel

type CategoryMap = {
  [key in InterfaceCategory]: CatalogCategory
}

export const categoryMap: CategoryMap = {
  [InterfaceCategory.EGO]: CatalogCategory.EGO,
  [InterfaceCategory.CAR]: CatalogCategory.CAR,
  [InterfaceCategory.BIKE]: CatalogCategory.CAR,
  [InterfaceCategory.MOTO_BIKE]: CatalogCategory.CAR,
  [InterfaceCategory.MACHINE]: CatalogCategory.CAR,
  [InterfaceCategory.PEDESTRIAN]: CatalogCategory.PEDESTRIAN,
  [InterfaceCategory.ANIMAL]: CatalogCategory.PEDESTRIAN,
  [InterfaceCategory.OBSTACLE]: CatalogCategory.OBSTACLE,
}

export interface ElectronFile extends File {
  path: string
}

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

export interface DynamicData {
  id: string
  dynamicData: object
}

export interface SensorOutsideParams {
  Device: string
  ID: string
  InstallSlot: string
  LocationX: number
  LocationY: number
  LocationZ: number
  RotationX: number
  RotationY: number
  RotationZ: number
  idx: number
}

export interface Planner extends EgoModel {
  sensor: {
    groupName: number
    group: Array<SensorOutsideParams>
  }
}
