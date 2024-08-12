// import _ from 'lodash'

export function createDefaultEnv () {
  return {
    TimeStamp: 0,
    Weather: {
      // Visibility: 30,
      // RainFall: 0,
      // SnowFall: 0,
      // CloudDensity: 0.2,

      WindSpeed: 2,
      VisualRange: 30,
      CloudState: 'cloudy',
      PrecipitationType: 'dry',
      PrecipitationIntensity: 0.5,
      Temperature: 25,
    },
    Date: {
      Year: 2023,
      Month: 3,
      Day: 20,
    },
    Time: {
      Hour: 14,
      Minute: 30,
      Second: 0,
      Millisecond: 0,
    },
  }
}
