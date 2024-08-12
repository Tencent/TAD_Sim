import { LineDashedMaterial, Vector3 } from 'three'
import { MapElementType } from '../common/Constant'
import { MapElement } from './Common.js'

class LaneLinkData extends MapElement {
  constructor () {
    super()
    this.elemType = MapElementType.LANELINK
    this.id = 0
    this.fromRoadId = 0
    this.fromSectionId = 0
    this.fromLaneId = 0
    this.toRoadId = 0
    this.toSectionId = 0
    this.toLaneId = 0
    this.len = 0

    this.junctionId = 0
    this.textMesh = null
  }

  parse (data) {
    this.id = data.id
    this.fromRoadId = data.frid
    this.fromSectionId = data.fsid
    this.fromLaneId = data.fid
    this.toRoadId = data.trid
    this.toSectionId = data.tsid
    this.toLaneId = data.tid
    this.len = data.len

    this.junctionId = data.junctionid || 0
  }

  getPoint (shift) {
    if (this.data.length > 0) {
      if (shift > this.len) {
        return []
      } else {
        if (shift < 0) {
          console.error(`getPoint shift is ${shift}, now set to 0`)
          shift = 0
        }
        const seg = 10
        const nOrder = Math.floor(shift / seg)
        const segShift = shift - seg * nOrder
        const dir = MapElement.defaultDir.clone()
        const startPos = new Vector3()

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

  composeSceneData (material) {
    super.composeSceneData(material)
  }

  // addTextMesh (scene) {
  //   try {
  //     getTextMesh(`${this.junctionId}.${this.id}`).then((text) => {
  //       this.textMesh = text
  //       const { x, y, z } = this.getLastPosition()
  //       if (this.textMesh) {
  //         this.textMesh.position.set(x, y, z)
  //         scene.add(this.textMesh)
  //       }
  //     })
  //   } catch (error) {
  //     console.warn(error)
  //   }
  // }

  getDescInfo () {
    return `${this.junctionId}.${this.id}`
  }
}

LaneLinkData.mat = new LineDashedMaterial({
  color: 0x009DFF,
  dashSize: 2,
  gapSize: 2,
})
export default LaneLinkData
