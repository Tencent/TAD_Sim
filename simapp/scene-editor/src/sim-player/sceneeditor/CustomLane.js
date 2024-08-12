import {
  BufferGeometry,
  Color,
  DoubleSide,
  Float32BufferAttribute,
  Mesh,
  MeshBasicMaterial,
  Vector3,
} from 'three'
import LaneData from './LaneData'

// const vertexShader = `
//   precision mediump float;
//   attribute vec4 position;
//   attribute vec2 a_offset;
//
//   uniform float u_width;
//   uniform mat4 modelViewMatrix;
//   uniform mat4 projectionMatrix;
//
//   varying float v_linesofar;
//
//   varying float v_up;
//
//   void main() {
//
//     float linesofar = a_offset.x / a_offset.y;
//
//     gl_Position = projectionMatrix * modelViewMatrix * vec4(position.xyz, 1.);
//
//     v_linesofar = linesofar;
//
//     v_up = position.w;
//   }
// `
// const fragmentShader = `
//   precision mediump float;
//   uniform vec4 u_color;
//
//   uniform float u_length;
//   uniform float u_width;
//
//   #ifdef USE_MAP
//   uniform sampler2D u_map;
//   uniform vec3 u_mapRange; // (s, ls, lt)between 0. and 1.
//   uniform float u_useCustomColor;
//   uniform float u_offsetT; // meter unit
//   #endif
//
//   varying float v_linesofar;
//   varying float v_up;
//
//   void main() {
//     float alpha = 1.;
//
//     vec4 color = u_color;
//
//     #ifdef USE_MAP
//     vec2 factors = u_mapRange.yz / vec2(u_length, u_width);
//     vec2 road_uv = vec2(v_linesofar, (-v_up + 1.) * 0.5 + u_offsetT / u_width);
//     road_uv -= vec2(u_mapRange.x, 0.5);
//     road_uv /= factors;
//     road_uv += 0.5;
//     vec4 mapColor = texture2D(u_map, road_uv.yx);
//     color = mix(color, mapColor, mapColor.a);
//     #endif
//
//     gl_FragColor = color;
//   }
// `

export default class CustomLane extends LaneData {
  // todo; optimize, setDrawRange;
  generateMesh (hadmap, scene) {
    const vV = new Vector3(0, 0, 1)
    if (this.leftBoundaryId < 0 || this.rightBoundaryId < 0) {
      console.log('lane no boundary: ', this.roadId, ', ', this.sectionId, ', ', this.laneId)
      return
    }

    const lb = hadmap.getLaneBoundary(this.leftBoundaryId)
    const rb = hadmap.getLaneBoundary(this.rightBoundaryId)

    if (!lb || !rb) {
      console.log('lane no boundary: ', this.roadId, ', ', this.sectionId, ', ', this.laneId)
      return
    }

    const leftSize = lb.data.length
    const rightSize = rb.data.length

    const maxSize = leftSize > rightSize ? leftSize : rightSize
    const minSize = leftSize > rightSize ? rightSize : leftSize
    if (maxSize <= 1) {
      console.log('lane boundary data count is too small: ', this.leftBoundaryId)
      return
    }

    let nDifCount = leftSize - rightSize

    const positions = []

    const offsets = []

    const indices = []

    const v1 = new Vector3()
    const v2 = new Vector3()
    const v3 = new Vector3()

    v1.x = lb.data[1].x - lb.data[0].x
    v1.y = lb.data[1].y - lb.data[0].y
    v1.z = lb.data[1].z - lb.data[0].z

    v2.x = rb.data[0].x - lb.data[1].x
    v2.y = rb.data[0].x - lb.data[1].y
    v2.z = rb.data[0].x - lb.data[1].z

    v1.normalize()
    v2.normalize()
    v3.crossVectors(v1, v2)
    v3.normalize()
    const d = v3.dot(vV)
    let finalL = lb
    let finalR = rb
    if (d < 0) {
      finalL = rb
      finalR = lb
      nDifCount = nDifCount * -1
    }

    for (let i = 0; i < minSize - 1; ++i) {
      addVertex(positions, offsets, finalL, i, -1)

      addVertex(positions, offsets, finalL, i + 1, -1)

      addVertex(positions, offsets, finalR, i, 1)

      indices.push(6 * i, 6 * i + 1, 6 * i + 2)

      addVertex(positions, offsets, finalR, i, 1)

      addVertex(positions, offsets, finalL, i + 1, -1)

      addVertex(positions, offsets, finalR, i + 1, 1)

      indices.push(6 * i + 3, 6 * i + 4, 6 * i + 5)
    }
    if (nDifCount >= 0) {
      for (let i = minSize - 1; i < maxSize - 1; ++i) {
        addVertex(positions, offsets, finalL, i, -1)

        addVertex(positions, offsets, finalL, i + 1, -1)

        addVertex(positions, offsets, finalR, minSize - 1, 1)

        indices.push(6 * i, 6 * i + 1, 6 * i + 2)

        addVertex(positions, offsets, finalR, minSize - 1, 1)

        addVertex(positions, offsets, finalL, i + 1, -1)

        addVertex(positions, offsets, finalR, minSize - 1, 1)

        indices.push(6 * i + 3, 6 * i + 4, 6 * i + 5)
      }
    } else {
      for (let i = minSize - 1; i < maxSize - 1; ++i) {
        addVertex(positions, offsets, finalL, minSize - 1, -1)

        addVertex(positions, offsets, finalL, minSize - 1, -1)

        addVertex(positions, offsets, finalR, i, 1)

        indices.push(6 * i, 6 * i + 1, 6 * i + 2)

        addVertex(positions, offsets, finalR, i, 1)

        addVertex(positions, offsets, finalL, minSize - 1, -1)

        addVertex(positions, offsets, finalR, i + 1, 1)

        indices.push(6 * i + 3, 6 * i + 4, 6 * i + 5)
      }
    }

    const bufferGeometry = new BufferGeometry()
    bufferGeometry.setAttribute('position', new Float32BufferAttribute(positions, 4))
    bufferGeometry.setAttribute('a_offset', new Float32BufferAttribute(offsets, 2)) // 0 - 1
    bufferGeometry.setIndex(indices)

    const color = this.getColorByType(this.type)
    const colorVec = new Color(color)

    // const mat = new RawShaderMaterial({
    //   uniforms: {
    //     u_width: { value: this.width },
    //     u_color: { value: new Vector4(colorVec.r, colorVec.g, colorVec.b, 1) },
    //     u_length: { value: this.len },
    //
    //     u_useCustomColor: { value: 0 },
    //     u_offsetT: { value: 0 },
    //
    //     // custom data
    //     u_map: { value: null },
    //     u_mapRange: {},
    //   },
    //
    //   vertexShader: vertexShader,
    //   fragmentShader: fragmentShader,
    //   side: DoubleSide,
    //   transparent: true,
    //   depthWrite: false,
    //   polygonOffset: true,
    //   polygonOffsetUnits: 4,
    //   polygonOffsetFactor: 10,
    //   defines: {
    //     USE_MAP: 1,
    //   },
    // })
    //
    // try {
    //   const [w, h] = [3.78, 3.78]
    //
    //   mat.map = null
    //   mat.uniforms.u_map.value = mat.map
    //   mat.uniforms.u_mapRange.value = [0.5, w, h]
    // } catch (error) {
    //   console.warn('generateMesh failed;', error)
    // }

    const mat = new MeshBasicMaterial({
      color: colorVec,
      side: DoubleSide,
      polygonOffset: true,
      polygonOffsetUnits: 4,
      polygonOffsetFactor: 10,
    })
    this.laneMesh = new Mesh(bufferGeometry, mat)
    this.laneMesh.updateMatrix()
    // this.laneMesh.renderOrder = RenderOrder.LANE

    return this.laneMesh
  }

  getDescInfo () {
    return `${this.roadId || 0}.${this.sectionId || 0}.${this.laneId || 0}`
  }
}

/**
 *
 * @param {*} positions
 * @param {*} offsets x, y; x: distance to the start point; y: total length
 * @param {*} array
 * @param {*} index
 * @param {*} up
 */
function addVertex (positions, offsets, array, index, up) {
  const { x, y, z } = array.data[index]
  positions.push(x, y, z, up)
  offsets.push(array.segLength * Math.ceil(index / 2), array.length) // 0 - 1
}
