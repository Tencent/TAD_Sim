/**
 * rosbag类
 */
class Rosbag {
  constructor () {
    this.path = ''
  }

  copyNoModel (other) {
    this.path = other.path
  }
}

export default Rosbag
