import {
  // BufferGeometry,
  DoubleSide,
  // Face3,
  // Float32BufferAttribute,
  // Geometry,
  LineBasicMaterial,
  // Mesh,
  // MeshBasicMaterial,
  Vector3,
} from 'three'
// import GlobalConfig from '../common/Config'
import { MapElementType } from '../common/Constant'
import { MapElement } from './Common'

class LaneData extends MapElement {
  constructor () {
    super()
    this.elemType = MapElementType.LANE
    this.roadId = 0
    this.sectionId = 0
    this.laneId = 0
    this.leftBoundaryId = -1
    this.rightBoundaryId = -1
    this.speedLimit = 0
    this.type = 0
    this.arrow = 0
    this.len = 0
    this.width = 0
  }

  parse (data) {
    this.roadId = data.rid
    this.sectionId = data.sid
    this.laneId = data.id

    this.leftBoundaryId = data.lbid
    this.rightBoundaryId = data.rbid
    this.speedLimit = data.sl
    this.type = data.type
    this.arrow = data.arr
    this.len = +(data.len)
    this.width = +(data.wid)
  }

  dispose () {
    super.dispose()

    if (this.laneMesh) {
      this.laneMesh = null
    }

    if (this.laneMeshGeom) {
      this.laneMeshGeom.dispose()
      this.laneMeshGeom = null
    }
  }

  getPoint (shift) {
    if (this.data.length > 0) {
      if (shift > this.len) {
        return []
      } else {
        if (shift < 0) {
          console.error(`shift is ${shift}, now set to 0`)
          shift = 0
        }
        // let seg = lane.len / lane.data.length * 2;
        const seg = 10
        const nOrder = Math.floor(shift / seg)
        const segShift = shift - seg * nOrder
        const dir = MapElement.defaultDir.clone()
        const startPos = new Vector3()
        // if (this.laneId > 0) {
        //   nOrder = (this.data.length / 2) - nOrder
        // }
        if (nOrder < (this.data.length / 2)) {
          dir.x = this.data[nOrder * 2 + 1].x - this.data[nOrder * 2].x
          dir.y = this.data[nOrder * 2 + 1].y - this.data[nOrder * 2].y
          dir.z = this.data[nOrder * 2 + 1].z - this.data[nOrder * 2].z
          startPos.x = this.data[nOrder * 2].x
          startPos.y = this.data[nOrder * 2].y
          startPos.z = this.data[nOrder * 2].z
        } else {
          dir.x = this.data[nOrder * 2 - 1].x - this.data[nOrder * 2 - 2].x
          dir.y = this.data[nOrder * 2 - 1].y - this.data[nOrder * 2 - 2].y
          dir.z = this.data[nOrder * 2 - 1].z - this.data[nOrder * 2 - 2].z
          startPos.x = this.data[this.data.length - 1].x
          startPos.y = this.data[this.data.length - 1].y
          startPos.z = this.data[this.data.length - 1].z
        }

        // if (this.laneId > 0) {
        //   dir.multiplyScalar(-1)
        // }
        const segLength = dir.length()
        dir.normalize()

        let delta = segShift / seg

        if (nOrder === (this.data.length / 2 - 1)) {
          const curveLen = this.len
          const lastSegLength = curveLen - Math.floor(curveLen / seg) * seg
          delta = segShift / lastSegLength
        }

        const pos = dir.clone().multiplyScalar(delta * segLength)
        startPos.add(pos)

        return [startPos.x, startPos.y, startPos.z]
      }
    } else {
      return []
    }
  }

  generateMesh (hadmap, scene) {
    // let vV = new Vector3(0, 0, 1)
    // if (this.leftBoundaryId < 0 || this.rightBoundaryId < 0) {
    //   console.log('lane no boundary: ', this.roadId, ', ', this.sectionId, ', ', this.laneId)
    //   return
    // }
    //
    // let lb = hadmap.getLaneBoundary(this.leftBoundaryId)
    // let rb = hadmap.getLaneBoundary(this.rightBoundaryId)
    //
    // if (!lb || !rb) {
    //   console.log('lane no boundary: ', this.roadId, ', ', this.sectionId, ', ', this.laneId)
    //   return
    // }
    //
    // let leftSize = lb.data.length
    // let rightSize = rb.data.length
    //
    // let maxSize = leftSize > rightSize ? leftSize : rightSize
    // let minSize = leftSize > rightSize ? rightSize : leftSize
    // if (maxSize <= 1) {
    //   console.log('lane boundary data count is too small: ', this.leftBoundaryId)
    //   return
    // }
    //
    // let nDifCount = leftSize - rightSize
    //
    // let vertices = []
    // let faces = []
    //
    // let v1 = new Vector3()
    // let v2 = new Vector3()
    // let v3 = new Vector3()
    // v1.x = lb.data[1].x - lb.data[0].x
    // v1.y = lb.data[1].y - lb.data[0].y
    // v1.z = lb.data[1].z - lb.data[0].z
    //
    // v2.x = rb.data[0].x - lb.data[1].x
    // v2.y = rb.data[0].x - lb.data[1].y
    // v2.z = rb.data[0].x - lb.data[1].z
    //
    // v1.normalize()
    // v2.normalize()
    // v3.crossVectors(v1, v2)
    // v3.normalize()
    // let d = v3.dot(vV)
    // let finalL = lb
    // let finalR = rb
    // if (d < 0) {
    //   finalL = rb
    //   finalR = lb
    //   nDifCount = nDifCount * -1
    // }
    //
    // for (let i = 0; i < minSize - 1; ++i) {
    //   let v = new Vector3(finalL.data[i].x, finalL.data[i].y, finalL.data[i].z)
    //   vertices.push(v)
    //   v = new Vector3(finalL.data[i + 1].x, finalL.data[i + 1].y, finalL.data[i + 1].z)
    //   vertices.push(v)
    //   v = new Vector3(finalR.data[i].x, finalR.data[i].y, finalR.data[i].z)
    //   vertices.push(v)
    //   let f = new Face3(6 * i, 6 * i + 1, 6 * i + 2)
    //   faces.push(f)
    //
    //   v = new Vector3(finalR.data[i].x, finalR.data[i].y, finalR.data[i].z)
    //   vertices.push(v)
    //   v = new Vector3(finalL.data[i + 1].x, finalL.data[i + 1].y, finalL.data[i + 1].z)
    //   vertices.push(v)
    //   v = new Vector3(finalR.data[i + 1].x, finalR.data[i + 1].y, finalR.data[i + 1].z)
    //   vertices.push(v)
    //   f = new Face3(6 * i + 3, 6 * i + 4, 6 * i + 5)
    //   faces.push(f)
    // }
    // if (nDifCount >= 0) {
    //   for (let i = minSize - 1; i < maxSize - 1; ++i) {
    //     let v = new Vector3(finalL.data[i].x, finalL.data[i].y, finalL.data[i].z)
    //     vertices.push(v)
    //     v = new Vector3(finalL.data[i + 1].x, finalL.data[i + 1].y, finalL.data[i + 1].z)
    //     vertices.push(v)
    //     v = new Vector3(finalR.data[minSize - 1].x, finalR.data[minSize - 1].y, finalR.data[minSize - 1].z)
    //     vertices.push(v)
    //     let f = new Face3(6 * i, 6 * i + 1, 6 * i + 2)
    //     faces.push(f)
    //
    //     v = new Vector3(finalR.data[minSize - 1].x, finalR.data[minSize - 1].y, finalR.data[minSize - 1].z)
    //     vertices.push(v)
    //     v = new Vector3(finalL.data[i + 1].x, finalL.data[i + 1].y, finalL.data[i + 1].z)
    //     vertices.push(v)
    //     v = new Vector3(finalR.data[minSize - 1].x, finalR.data[minSize - 1].y, finalR.data[minSize - 1].z)
    //     vertices.push(v)
    //     f = new Face3(6 * i + 3, 6 * i + 4, 6 * i + 5)
    //     faces.push(f)
    //   }
    // } else {
    //   for (let i = minSize - 1; i < maxSize - 1; ++i) {
    //     let v = new Vector3(finalL.data[minSize - 1].x, finalL.data[minSize - 1].y, finalL.data[minSize - 1].z)
    //     vertices.push(v)
    //     v = new Vector3(finalL.data[minSize - 1].x, finalL.data[minSize - 1].y, finalL.data[minSize - 1].z)
    //     vertices.push(v)
    //     v = new Vector3(finalR.data[i].x, finalR.data[i].y, finalR.data[i].z)
    //     vertices.push(v)
    //     let f = new Face3(6 * i, 6 * i + 1, 6 * i + 2)
    //     faces.push(f)
    //
    //     v = new Vector3(finalR.data[i].x, finalR.data[i].y, finalR.data[i].z)
    //     vertices.push(v)
    //     v = new Vector3(finalL.data[minSize - 1].x, finalL.data[minSize - 1].y, finalL.data[minSize - 1].z)
    //     vertices.push(v)
    //     v = new Vector3(finalR.data[i + 1].x, finalR.data[i + 1].y, finalR.data[i + 1].z)
    //     vertices.push(v)
    //     f = new Face3(6 * i + 3, 6 * i + 4, 6 * i + 5)
    //     faces.push(f)
    //   }
    // }
    //
    // if (GlobalConfig.MergeLaneGeom()) {
    //   let position = []
    //   this.laneMeshGeomPointCount = vertices.length
    //   for (let i = 0; i < vertices.length; ++i) {
    //     position.push(vertices[i].x, vertices[i].y, vertices[i].z)
    //   }
    //
    //   this.laneMeshGeom = new BufferGeometry()
    //   this.laneMeshGeom.setAttribute('position', new Float32BufferAttribute(position, 3))
    //   return this.laneMeshGeom
    // } else {
    //   let geom = new Geometry()
    //   geom.vertices = vertices
    //   geom.faces = faces
    //
    //   this.laneMeshGeom = new BufferGeometry()
    //   this.laneMeshGeom.fromGeometry(geom)
    //   geom.dispose()
    //
    //   const mat = getLaneMaterialByType(this.type)
    //   this.laneMesh = new Mesh(this.laneMeshGeom, mat)
    //
    //   this.laneMesh.matrixAutoUpdate = false
    //   this.laneMesh.updateMatrix()
    //   return this.laneMesh
    // }
  }

  getColorByType (type) {
    return colorMaps[type]
  }
}

// LaneData.mat = new LineBasicMaterial({ color: 0x087C85 })
// LaneData.matMesh = new LineBasicMaterial({
//   color: 0x2C3652,
//   side: DoubleSide,
//   transparent: true,
//   opacity: 0.5,
// })

const laneMat = new LineBasicMaterial({ color: 0x087C85 })
const laneMatMesh = new LineBasicMaterial({
  color: 0x2C3652,
  side: DoubleSide,
  transparent: true,
  opacity: 0.5,
})

const LANE_TYPE = {
  LANE_TYPE_Others: 9999,
  LANE_TYPE_None: 0,
  LANE_TYPE_Driving: 1,
  LANE_TYPE_Stop: 2,
  LANE_TYPE_Shoulder: 3,
  LANE_TYPE_Biking: 4,
  LANE_TYPE_Sidewalk: 5,
  LANE_TYPE_Border: 6,
  LANE_TYPE_Restricted: 7, // BUS
  LANE_TYPE_Parking: 8,
  LANE_TYPE_MwyEntry: 9,
  LANE_TYPE_MwyExit: 10,
  LANE_TYPE_Acceleration: 11,
  LANE_TYPE_Deceleration: 12,
  LANE_TYPE_Compound: 13,
  LANE_TYPE_HOV: 14,
  LANE_TYPE_Slow: 15,
  LANE_TYPE_Reversible: 16,
  LANE_TYPE_EmergencyParking: 17,
  LANE_TYPE_Emergency: 20,
  LANE_TYPE_TurnWaiting: 21,
}

const colorMaps = {
  [LANE_TYPE.LANE_TYPE_Driving]: 0x2C3652,
  [LANE_TYPE.LANE_TYPE_Biking]: 0xC17471,
  [LANE_TYPE.LANE_TYPE_Sidewalk]: 0xD8DDD3,
  [LANE_TYPE.LANE_TYPE_None]: 0x2C3652,
  [LANE_TYPE.LANE_TYPE_Emergency]: 0x2C3652,
}

// const MaterialMaps = {
//   [LANE_TYPE.LANE_TYPE_Driving]: new LineBasicMaterial({
//     color: 0x2C3652,
//     side: DoubleSide,
//     transparent: true,
//     opacity: 0.5,
//   }),
//   [LANE_TYPE.LANE_TYPE_Biking]: new MeshBasicMaterial({
//     color: 0xC17471,
//     side: DoubleSide,
//     transparent: true,
//     opacity: 0.5,
//   }),
//   [LANE_TYPE.LANE_TYPE_Sidewalk]: new MeshBasicMaterial({
//     color: 0xD8DDD3,
//     side: DoubleSide,
//     transparent: true,
//     opacity: 0.5,
//   }),
// }

// function getLaneMaterialByType (type) {
//   return MaterialMaps[type] || laneMatMesh
// }

export default LaneData

export {
  laneMat,
  laneMatMesh,
}
