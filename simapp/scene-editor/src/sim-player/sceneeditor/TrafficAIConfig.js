/**
 * 交通流AI配置
 */
export default class TrafficAIConfig {
  constructor () {
    this.speedAverage = 12
    this.trafficDensity = 0.5
    this.aggress = 0.5
    this.sedan = 40
    this.suv = 25
    this.truck = 15
    this.taxi = 20
  }

  parse (config) {
    this.speedAverage = config.averageSpeed
    this.trafficDensity = config.trafficDensity
    this.aggress = config.radicalDegree
    this.sedan = config.carTypes.Car
    this.suv = config.carTypes.SUV
    this.truck = config.carTypes.Truck
    this.taxi = config.carTypes.Taxi
  }

  /**
   * UI消息
   * @return {{}}
   */
  uiMsg () {
    const msg = {}
    msg.averageSpeed = this.speedAverage
    msg.trafficDensity = this.trafficDensity
    msg.radicalDegree = this.aggress
    msg.carTypes = []
    msg.carTypes.push({ name: 'Car', weight: this.sedan })
    msg.carTypes.push({ name: 'SUV', weight: this.suv })
    msg.carTypes.push({ name: 'Truck', weight: this.truck })
    msg.carTypes.push({ name: 'Taxi', weight: this.taxi })
    return msg
  }
}
