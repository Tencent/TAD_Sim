// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "actors/c_actor_static.h"

namespace eval {
//
// Defines parts of a day
//
// \par References:
// [1] https://en.wikipedia.org/wiki/Category:Parts_of_a_day
//
enum PartsOfDay {
  PARTS_OF_DAY_UNKNOWN = 0,    //
  PARTS_OF_DAY_MIDNIGHT = 1,   // 00:01 ~ 03:00 [    0, 10800[ s  凌晨
  PARTS_OF_DAY_DAWN = 2,       // 03:01 ~ 06:00 [10800, 21600[ s  黎明
  PARTS_OF_DAY_MORNING = 3,    // 06:01 ~ 09:00 [21600, 32400[ s  早晨
  PARTS_OF_DAY_FORENOON = 4,   // 09:01 ~ 11:00 [32400, 39600[ s  上午
  PARTS_OF_DAY_NOON = 5,       // 11:01 ~ 13:00 [39600, 46800[ s  中午
  PARTS_OF_DAY_AFTERNOON = 6,  // 13:01 ~ 15:00 [46800, 54000[ s  下午
  PARTS_OF_DAY_EVENING = 7,    // 15:01 ~ 18:00 [54000, 64800[ s  傍晚
  PARTS_OF_DAY_DUSK = 8,       // 18:01 ~ 21:00 [64800, 75600[ s  黄昏
  PARTS_OF_DAY_NIGHT = 9       // 21:01 ~ 24:00 [75600, 86400[ s  深夜
};

//
// Defines Rain grade
//
// \par References:
// [1] asam osi_environment.proto
//
enum Rain {
  RAIN_UNKNOWN = 0,     // must not be used in ground truth
  RAIN_OTHER = 1,       // Other (unspecified but known) intensity.
  RAIN_NONE = 2,        // [  0, 0.1[ mm/h
  RAIN_VERY_LIGHT = 3,  // [0.1, 0.5[ mm/h
  RAIN_LIGHT = 4,       // [0.5, 1.9[ mm/h
  RAIN_MODERATE = 5,    // [1.9, 8.1[ mm/h
  RAIN_HEAVY = 6,       // [8.1, 34[ mm/h
  RAIN_VERY_HEAVY = 7,  // [ 34, 149[ mm/h
  RAIN_EXTREME = 8      // [149, infinity[ mm/h
};

//
// Defines Snow grade
//
// \par References:
// [1]
// https://baike.baidu.com/item/%E9%9B%AA%E9%87%8F/2099146?fromtitle=%E9%99%8D%E9%9B%AA%E9%87%8F&fromid=10643516
//
enum Snow {
  SNOW_UNKNOWN = 0,   // must not be used in ground truth
  SNOW_OTHER = 1,     // Other (unspecified but known) intensity.
  SNOW_NONE = 2,      // [     0,    0.004[ mm/h
  SNOW_LIGHT = 3,     // [ 0.004,      0.1[ mm/h  小雪
  SNOW_MODERATE = 4,  // [   0.1,    1.225[ mm/h  中雪
  SNOW_HEAVY = 5,     // [ 1.225,     4.16[ mm/h  大雪
  SNOW_EXTREME = 6    // [  4.16, infinity[ mm/h  暴雪
};

//
// Defines Fog grade
//
enum Fog {
  FOG_UNKNOWN = 0,               // must not be used in ground truth
  FOG_OTHER = 1,                 // Other (unspecified but known) intensity.
  FOG_EXCELLENT_VISIBILITY = 2,  // [40000,infinity[ m
  FOG_GOOD_VISIBILITY = 3,       // [10000,40000[ m
  FOG_MODERATE_VISIBILITY = 4,   // [4000,10000[ m
  FOG_POOR_VISIBILITY = 5,       // [2000,4000[ m
  FOG_MIST = 6,                  // [1000,2000[ m
  FOG_LIGHT = 7,                 // [200,1000[ m
  FOG_THICK = 8,                 // [50,200[ m
  FOG_DENSE = 9                  // [0,50[ m
};

//
// Defines the fractional cloud cover [1] given by observation of total cloud
// amount in eights (oktas) of the sky.
//
enum FractionalCloudCover {
  FRACTIONAL_CLOUD_COVER_UNKNOWN = 0,       // must not be used in ground truth
  FRACTIONAL_CLOUD_COVER_OTHER = 1,         // Other(unspecified but known).
  FRACTIONAL_CLOUD_COVER_ZERO_OKTAS = 2,    // [0,0.2[ oktas0/8.
  FRACTIONAL_CLOUD_COVER_ONE_OKTAS = 3,     // 1/8.
  FRACTIONAL_CLOUD_COVER_TWO_OKTAS = 4,     // 2/8.
  FRACTIONAL_CLOUD_COVER_THREE_OKTAS = 5,   // 3/8.
  FRACTIONAL_CLOUD_COVER_FOUR_OKTAS = 6,    // 4/8.
  FRACTIONAL_CLOUD_COVER_FIVE_OKTAS = 7,    // 5/8.
  FRACTIONAL_CLOUD_COVER_SIX_OKTAS = 8,     // 6/8.
  FRACTIONAL_CLOUD_COVER_SEVEN_OKTAS = 9,   // 7/8.
  FRACTIONAL_CLOUD_COVER_EIGHT_OKTAS = 10,  // 8/8.
  FRACTIONAL_CLOUD_COVER_SKY_OBSCURED = 11  // Sky obscured
};

//
// Defines Cloudy grade
//
// \par References:
// [1] https://en.wikipedia.org/wiki/Okta
// [2] https://worldweather.wmo.int/oktas.htm
//
enum Cloudy {
  CLOUDY_UNKNOWN = 0,        // must not be used in ground truth
  CLOUDY_OTHER = 1,          // Other (unspecified but known) intensity.
  CLOUDY_FINE = 2,           // clear ~ 2/8 of sky covered
  CLOUDY_PARTLY_CLOUDY = 3,  // 3/8 ~ 5/8 of sky covered
  CLOUDY_CLOUDY = 4,         // 6/8 ~ 7/8 of sky covered
  CLOUDY_OVERCAST = 5        // 8/8 of sky covered
};

//
// Defines Wind grade
//
// \par References:
// [1] https://en.wikipedia.org/wiki/Beaufort_scale
//
enum Wind {
  WIND_UNKNOWN = 0,   // must not be used in ground truth
  WIND_OTHER = 1,     // Other (unspecified but known) intensity.
  WIND_LEVEL0 = 2,    // [0,0.5[ m/s  Calm
  WIND_LEVEL1 = 3,    // [0.5,1.5[ m/s  Light air
  WIND_LEVEL2 = 4,    // [1.5,3.3[ m/s  Light breeze
  WIND_LEVEL3 = 5,    // [3.3,5.5[ m/s  Gentle breeze
  WIND_LEVEL4 = 6,    // [5.5,8.0[ m/s  Moderate breeze
  WIND_LEVEL5 = 7,    // [8.0,10.8[ m/s  Fresh breeze
  WIND_LEVEL6 = 8,    // [10.8,13.9[ m/s  Strong breeze
  WIND_LEVEL7 = 9,    // [13.9,17.2[ m/s  High wind
  WIND_LEVEL8 = 10,   // [17.2,20.8[ m/s  Gale
  WIND_LEVEL9 = 11,   // [20.8,24.5[ m/s  Strong gale
  WIND_LEVEL10 = 12,  // [24.5,28.5[ m/s  Storm
  WIND_LEVEL11 = 13,  // [28.5,32.7[ m/s  Violent storm
  WIND_LEVEL12 = 14   // [32.7,infinity[ m/s  Hurricane
};

//
// Defines precipitation type
//
enum PrecipitationType {
  PRECIPITATION_TYPE_DRY = 0,      // No precipitation.
  PRECIPITATION_TYPE_RAIN = 1,     // Rain
  PRECIPITATION_TYPE_SNOW = 2,     // Snow
  PRECIPITATION_TYPE_HAIL = 3,     // Hail
  PRECIPITATION_TYPE_QRAUPEL = 4,  // QRAUPEL
  PRECIPITATION_TYPE_OTHER = 5     // OTHER
};

// Weather
enum Weather {
  WEATHER_UNKNOWN = 0,
  WEATHER_SUNNY = 1,
  WEATHER_CLOUDY = 2,
  WEATHER_RAIN = 3,
  WEATHER_SNOW = 4,
  WEATHER_FOG = 5,
  WEATHER_WIND = 6,
  WEATHER_HAIL = 7,
  WEATHER_DUST = 8,
  WEATHER_SAND = 9,
  WEATHER_SNOW_RAIN = 10
};

/**
 * @brief "环境", environment actor type, which is a static actor.
 */
class CEnvironmentActor final : public CStaticActor {
 private:
  // time dependent
  PartsOfDay _parts_of_day;
  // weather dependent
  Rain _rain;
  Snow _snow;
  Fog _fog;
  double _fog_visibility;
  Cloudy _cloudy;
  Wind _wind;
  Weather _weather;
  // illumination dependent
  // Illumination _illumination;

 public:
  // time dependent
  inline const PartsOfDay& GetPartsOfDay() const { return _parts_of_day; }
  // weather dependent
  inline const Rain& GetRain() const { return _rain; }
  inline const Snow& GetSnow() const { return _snow; }
  inline const Fog& GetFog() const { return _fog; }
  inline const double& GetFogVisibility() const { return _fog_visibility; }
  inline const Cloudy& GetCloudy() const { return _cloudy; }
  inline const Wind& GetWind() const { return _wind; }
  inline const Weather& GetWeather() const { return _weather; }

 public:
  void Reset() {
    // time dependent
    _parts_of_day = PARTS_OF_DAY_NOON;
    // weather dependent
    _rain = RAIN_NONE;
    _snow = SNOW_NONE;
    _fog = FOG_EXCELLENT_VISIBILITY;
    _fog_visibility = 500;
    _cloudy = CLOUDY_FINE;
    _wind = WIND_LEVEL0;
    _weather = WEATHER_SUNNY;
  }

  // time dependent
  void SetPartsOfDay(uint32_t seconds);
  // weather dependent
  void SetRain(double intensity);
  void SetSnow(double intensity);
  void SetFog(double visibility);
  void SetFogVisibility(double visibility);
  void SetCloudy(FractionalCloudCover cloud_cover);
  void SetWind(double speed);
  void SetWeather(PrecipitationType precipitation_type, double visibility, FractionalCloudCover cloud_cover);
};

/**
 * @brief "环境建造者", environment actor builder, which build the actors from the map and the msg.
 */
class CEnvironmentActorBuilder : public CActorBuilder {
 public:
  virtual void Build(const EvalMsg& msg, ActorReposity& actor_repo) override;

 protected:
  static void BuildFrom(const EvalMsg& msg, ActorReposity& actor_map);
};
}  // namespace eval
