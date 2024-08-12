// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_environment.h"
#include "environment.pb.h"

namespace eval {
// time dependent
void CEnvironmentActor::SetPartsOfDay(uint32_t seconds) {
  if (seconds >= 0 && seconds < 10800) {
    _parts_of_day = PARTS_OF_DAY_MIDNIGHT;
  } else if (seconds >= 10800 && seconds < 21600) {
    _parts_of_day = PARTS_OF_DAY_DAWN;
  } else if (seconds >= 21600 && seconds < 32400) {
    _parts_of_day = PARTS_OF_DAY_MORNING;
  } else if (seconds >= 32400 && seconds < 39600) {
    _parts_of_day = PARTS_OF_DAY_FORENOON;
  } else if (seconds >= 39600 && seconds < 46800) {
    _parts_of_day = PARTS_OF_DAY_NOON;
  } else if (seconds >= 46800 && seconds < 54000) {
    _parts_of_day = PARTS_OF_DAY_AFTERNOON;
  } else if (seconds >= 54000 && seconds < 64800) {
    _parts_of_day = PARTS_OF_DAY_EVENING;
  } else if (seconds >= 64800 && seconds < 75600) {
    _parts_of_day = PARTS_OF_DAY_DUSK;
  } else if (seconds >= 75600 && seconds < 86400) {
    _parts_of_day = PARTS_OF_DAY_NIGHT;
  } else {
    _parts_of_day = PARTS_OF_DAY_UNKNOWN;
  }
}

// weather dependent
void CEnvironmentActor::SetRain(double intensity) {
  if (intensity >= 0 && intensity < 0.1) {
    _rain = RAIN_NONE;
  } else if (intensity >= 0.1 && intensity < 0.5) {
    _rain = RAIN_VERY_LIGHT;
  } else if (intensity >= 0.5 && intensity < 1.9) {
    _rain = RAIN_LIGHT;
  } else if (intensity >= 1.9 && intensity < 8.1) {
    _rain = RAIN_MODERATE;
  } else if (intensity >= 8.1 && intensity < 34) {
    _rain = RAIN_HEAVY;
  } else if (intensity >= 34 && intensity < 149) {
    _rain = RAIN_VERY_HEAVY;
  } else if (intensity >= 149) {
    _rain = RAIN_EXTREME;
  } else {
    _rain = RAIN_UNKNOWN;
  }
}

void CEnvironmentActor::SetSnow(double intensity) {
  if (intensity >= 0 && intensity < 0.004) {
    _snow = SNOW_NONE;
  } else if (intensity >= 0.004 && intensity < 0.1) {
    _snow = SNOW_LIGHT;
  } else if (intensity >= 0.1 && intensity < 1.225) {
    _snow = SNOW_MODERATE;
  } else if (intensity >= 1.225 && intensity < 4.16) {
    _snow = SNOW_HEAVY;
  } else if (intensity >= 4.16) {
    _snow = SNOW_EXTREME;
  } else {
    _snow = SNOW_UNKNOWN;
  }
}

void CEnvironmentActor::SetFog(double visibility) {
  if (visibility >= 40000) {
    _fog = FOG_EXCELLENT_VISIBILITY;
  } else if (visibility >= 10000 && visibility < 40000) {
    _fog = FOG_GOOD_VISIBILITY;
  } else if (visibility >= 4000 && visibility < 10000) {
    _fog = FOG_MODERATE_VISIBILITY;
  } else if (visibility >= 2000 && visibility < 4000) {
    _fog = FOG_POOR_VISIBILITY;
  } else if (visibility >= 1000 && visibility < 2000) {
    _fog = FOG_MIST;
  } else if (visibility >= 200 && visibility < 1000) {
    _fog = FOG_LIGHT;
  } else if (visibility >= 50 && visibility < 200) {
    _fog = FOG_THICK;
  } else if (visibility >= 0 && visibility < 50) {
    _fog = FOG_DENSE;
  } else {
    _fog = FOG_UNKNOWN;
  }
}

void CEnvironmentActor::SetFogVisibility(double visibility) { _fog_visibility = visibility; }

void CEnvironmentActor::SetCloudy(FractionalCloudCover cloud_cover) {
  if (cloud_cover == FRACTIONAL_CLOUD_COVER_ZERO_OKTAS || cloud_cover == FRACTIONAL_CLOUD_COVER_ONE_OKTAS ||
      cloud_cover == FRACTIONAL_CLOUD_COVER_TWO_OKTAS) {
    _cloudy = CLOUDY_FINE;
  } else if (cloud_cover == FRACTIONAL_CLOUD_COVER_THREE_OKTAS || cloud_cover == FRACTIONAL_CLOUD_COVER_FOUR_OKTAS ||
             cloud_cover == FRACTIONAL_CLOUD_COVER_FIVE_OKTAS) {
    _cloudy = CLOUDY_PARTLY_CLOUDY;
  } else if (cloud_cover == FRACTIONAL_CLOUD_COVER_SIX_OKTAS || cloud_cover == FRACTIONAL_CLOUD_COVER_SEVEN_OKTAS) {
    _cloudy = CLOUDY_CLOUDY;
  } else if (cloud_cover == FRACTIONAL_CLOUD_COVER_EIGHT_OKTAS || cloud_cover == FRACTIONAL_CLOUD_COVER_SKY_OBSCURED) {
    _cloudy = CLOUDY_OVERCAST;
  } else {
    _cloudy = CLOUDY_UNKNOWN;
  }
}

void CEnvironmentActor::SetWind(double speed) {
  if (speed >= 0 && speed < 0.5) {
    _wind = WIND_LEVEL0;
  } else if (speed >= 0.5 && speed < 1.5) {
    _wind = WIND_LEVEL1;
  } else if (speed >= 1.5 && speed < 3.3) {
    _wind = WIND_LEVEL2;
  } else if (speed >= 3.3 && speed < 5.5) {
    _wind = WIND_LEVEL3;
  } else if (speed >= 5.5 && speed < 8.0) {
    _wind = WIND_LEVEL4;
  } else if (speed >= 8.0 && speed < 10.8) {
    _wind = WIND_LEVEL5;
  } else if (speed >= 10.8 && speed < 13.9) {
    _wind = WIND_LEVEL6;
  } else if (speed >= 13.9 && speed < 17.2) {
    _wind = WIND_LEVEL7;
  } else if (speed >= 17.2 && speed < 20.8) {
    _wind = WIND_LEVEL8;
  } else if (speed >= 20.8 && speed < 24.5) {
    _wind = WIND_LEVEL9;
  } else if (speed >= 24.5 && speed < 28.5) {
    _wind = WIND_LEVEL10;
  } else if (speed >= 28.5 && speed < 32.7) {
    _wind = WIND_LEVEL11;
  } else if (speed >= 32.7) {
    _wind = WIND_LEVEL12;
  } else {
    _wind = WIND_UNKNOWN;
  }
}

void CEnvironmentActor::SetWeather(PrecipitationType precipitation_type, double visibility,
                                   FractionalCloudCover cloud_cover) {
  bool is_fog = false;
  bool is_cloudy = false;
  if (visibility <= 4000) {
    is_fog = true;
  }
  if (cloud_cover >= FRACTIONAL_CLOUD_COVER_THREE_OKTAS) {
    is_cloudy = true;
  }

  if (precipitation_type == PRECIPITATION_TYPE_RAIN) {
    _weather = WEATHER_RAIN;
  } else if (precipitation_type == PRECIPITATION_TYPE_SNOW) {
    _weather = WEATHER_SNOW;
  } else if (precipitation_type == PRECIPITATION_TYPE_DRY) {
    if (is_fog) {
      _weather = WEATHER_FOG;
    } else if (is_fog == false && is_cloudy) {
      _weather = WEATHER_CLOUDY;
    } else {
      _weather = WEATHER_SUNNY;
    }
  } else {
    _weather = WEATHER_UNKNOWN;
  }
}

void CEnvironmentActorBuilder::Build(const EvalMsg& msg, ActorReposity& actor_repo) {
  if (msg.GetTopic() == topic::ENVIRONMENTAL) {
    CEnvironmentActorBuilder::BuildFrom(msg, actor_repo);
  }
}

void CEnvironmentActorBuilder::BuildFrom(const EvalMsg& msg, ActorReposity& actor_map) {
  if (msg.GetPayload().size() > 0) {
    // get sim time
    // double sim_t_ms = msg.GetSimTime().GetMilliseond();

    // build Environment
    // osi3::EnvironmentalConditions env;
    sim_msg::EnvironmentalConditions env;
    env.ParseFromString(msg.GetPayload());

    CEnvironmentActorPtr actor_ptr = nullptr;
    ActorAgentPtr actor_agent = actor_map[Actor_Environment]->at(0);
    actor_ptr = dynamic_cast<CEnvironmentActorPtr>(actor_agent->GetActorPtr());

    if (actor_ptr == nullptr) throw "Environment actor is nullptr.\n";

    // reset Environment
    actor_ptr->Reset();

    // set sim time
    actor_ptr->MutableSimTime()->FromSecond(msg.GetSimTime().GetSecond());

    // set type
    actor_ptr->SetType(Actor_Environment);

    // set parts of day
    actor_ptr->SetPartsOfDay(env.time_of_day().seconds_since_midnight());

    if (env.precipitation().type() == sim_msg::EnvironmentalConditions_Precipitation_Type_RAIN) {
      // set rain
      actor_ptr->SetRain(env.precipitation().intensity());
    }

    if (env.precipitation().type() == sim_msg::EnvironmentalConditions_Precipitation_Type_SNOW) {
      // set snow
      actor_ptr->SetSnow(env.precipitation().intensity());
    }

    // set fog
    actor_ptr->SetFog(env.fog().visibility());

    // set fog visibility
    actor_ptr->SetFogVisibility(env.fog().visibility());

    // set cloudy
    actor_ptr->SetCloudy(FractionalCloudCover(env.clouds().fractional_cloud_cover()));

    // set wind
    actor_ptr->SetWind(env.wind().speed());

    // set weather
    actor_ptr->SetWeather(PrecipitationType(env.precipitation().type()), env.fog().visibility(),
                          FractionalCloudCover(env.clouds().fractional_cloud_cover()));

    actor_agent->SetState(Actor_Valid);

  } else {
    LOG_ERROR << "msg is empty.\n";
  }
}
}  // namespace eval
