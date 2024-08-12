#pragma once
#include <string>
#include <vector>
#include <random>
#include <map>
#include <mutex>
#include <array>

class LidarModelBase
{
public:
    LidarModelBase() = default;
    virtual ~LidarModelBase() = default;

    virtual bool simulator(float cosita, unsigned int tag_c, unsigned int tag_t, float& d, float& reflection) = 0;
};

class LidarModel : public LidarModelBase
{
public:
    LidarModel(float accuracy = 0.2f);
    virtual ~LidarModel() = default;

    bool cache_tag(unsigned int tag_c, unsigned int tag_t);

    bool simulator(float cosita, unsigned int tag_c, unsigned int tag_t, float& d, float& reflection);

    void set_intensity(float i);
    float get_intensity() const
    {
        return t_intensity;
    }
    void set_null();

    void set_extinctionCoe(float r);
    float get_extinctionCoe() const
    {
        return raw_extinctionCoe;
    }

    //    小雨,    中雨    ,    大雨,        暴雨,        大暴雨,        特大暴雨
    // < 10,    10 - 25,    25 - 50,    50 - 100,    100 - 250,    >250
    // 设置雨量大小，mm/24h
    void set_rain(float r);
    float get_rain() const
    {
        return raw_rain;
    }

    //
    // 小雪,    中雪,        大雪,        暴雪    ,    大暴雪,        特大暴雪
    // <2.4,    2.4-4.9,    5.0-9.9,    10-19.9,    20-29.9,    >30
    // 设置降雪大小，mm/24h
    void set_snow(float s);
    float get_snow() const
    {
        return raw_snow;
    }

    // 设置雾大小，能见度m
    void set_fog(float f);
    float get_fog() const
    {
        return raw_fog;
    }

    bool load_refdata(const std::string& fname);

    bool get_refection_param(unsigned int tag_c, unsigned int tag_t, float& refmin, float& refmax, float& refdelta);

    void update_brownion_pos();

protected:
    virtual float reflection(unsigned int tag_c, unsigned int tag_t, float& sita);
    virtual float factor_rain(float d);
    virtual float factor_fog(float d);
    virtual void gausswhite_noise(float& d);
    virtual void snow_noise(float& d);

    float get_random();

private:
    float raw_rain = 0;
    float raw_snow = 0;
    float raw_fog = 0;
    float raw_extinctionCoe = 0;

    float t_intensity = 0;
    float f_noise_dev = 0;
    float f_accuracy = 0;
    float extinctionCoe = 1;

    // rain members
    float f_rainfall = 0;

    // snow members
    float f_snowfall_range = 0;
    float f_snowfall_prob = 0;

    // fog members
    float f_fog = 0;

    // reflection members
    struct refdata
    {
        float minr, maxr, deta;

        std::vector<double> brownian;    // 0~1
        size_t bid = 0;
    };
    std::map<size_t, refdata> _refdatas;
    float _reffree = 0;

    std::array<float, 65536> random_value;
};
