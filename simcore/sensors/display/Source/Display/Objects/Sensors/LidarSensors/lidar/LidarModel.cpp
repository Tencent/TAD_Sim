#include "LidarModel.h"
#include <algorithm>

#include <time.h>
#include <fstream>
#include <sstream>
#include "fbm/fbm.hpp"

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define POSSON_STDDEV 0.03464f

// 模型初始化
LidarModel::LidarModel(float accuracy)
{
    f_accuracy = accuracy;
    t_intensity = 0.0015f;    // 200m (on 20% reflectivity target)
    f_rainfall = 1.f;
    f_snowfall_range = 1000.f;
    f_snowfall_prob = 1.f;
    f_fog = 1.f;
    _reffree = 0.5f;
    f_noise_dev = POSSON_STDDEV;    // 标准差0.02

    std::default_random_engine random_eng(time(NULL));
    std::uniform_real_distribution<float> random_dis(-1.f, 1.f);
    for (size_t i = 0; i < random_value.size(); i++)
    {
        random_value[i] = random_dis(random_eng);
    }
}

// 获取tag，提前构造缓存数组，加快计算
bool LidarModel::cache_tag(unsigned int tag_c, unsigned int tag_t)
{
    //
    if (_refdatas.empty())
    {
        return false;
        ;
    }
    size_t tag = ((size_t) tag_c) * 100 + tag_t;
    auto fd = _refdatas.find(tag);
    if (fd == _refdatas.end())
    {
        tag -= size_t(tag_c % 10000) * 100;
        fd = _refdatas.find(tag);
        if (fd == _refdatas.end())
        {
            tag -= size_t(tag_c % 10000000) * 100;
            fd = _refdatas.find(tag);
            if (fd == _refdatas.end())
            {
                tag = ((size_t) tag_c) * 100;
                fd = _refdatas.find(tag);
                if (fd == _refdatas.end())
                {
                    tag -= tag % 1000000ull;
                    fd = _refdatas.find(tag);
                    if (fd == _refdatas.end())
                    {
                        tag -= tag % 1000000000ull;
                        fd = _refdatas.find(tag);
                    }
                }
            }
        }
    }
    if (fd == _refdatas.end())
    {
        return false;
    }
    tag = ((size_t) tag_c) * 100 + tag_t;
    _refdatas[tag] = fd->second;
    return true;
}

// 模型仿真
bool LidarModel::simulator(float cosita, unsigned int tag_c, unsigned int tag_t, float& d, float& ref)
{
    if (d < 0.01f)
    {
        return false;
    }
    // 计算反射率
    ref = reflection(tag_c, tag_t, cosita);
    // 计算回波强度
    float v = cosita * ref / (d * d);
    // 添加雨天噪声
    if (f_rainfall < 1.f)
    {
        unsigned int tag1 = tag_c / 10000000;
        if (tag1 >= 1 && tag1 <= 2)
        {
            v *= exp((f_rainfall - 1) * 1024);
        }
        v *= factor_rain(d);
    }
    // 添加雾噪声
    if (f_fog < 1.f)
    {
        v *= factor_fog(d);
    }
    v *= extinctionCoe;
    // 添加高斯噪声
    gausswhite_noise(d);
    if (v < t_intensity)
    {
        d = 0;
        return false;
    }
    // 添加雪噪声
    if (f_snowfall_prob < 1.f)
    {
        snow_noise(d);
    }
    return d >= 0.01f;
}

// 设置衰减阈值
void LidarModel::set_intensity(float i)
{
    t_intensity = i;
}

void LidarModel::set_null()
{
    set_rain(0);
    set_snow(0);
    set_fog(0);
}

// 设置衰减补偿
void LidarModel::set_extinctionCoe(float r)
{
    extinctionCoe = exp(-std::min(std::max(0.f, r), 1.f));
}

// 设置雨量
void LidarModel::set_rain(float w)
{
    raw_rain = w;
    w *= 0.041666667f;
    f_rainfall = exp(-0.02 * pow(w, 0.6));
    f_noise_dev = POSSON_STDDEV + 0.0245f * w;
}

// 设置降雪量
void LidarModel::set_snow(float s)
{
    raw_snow = s;
    if (s <= 1e-5)
    {
        raw_snow = 1e-5;
        f_snowfall_range = 1000.f;
        f_snowfall_prob = 1.f;
        return;
    }
    f_snowfall_range = 1000.f / s;
    f_snowfall_prob = 1.f - s * 0.03f * 2;
}

// 设置大雾能见度
void LidarModel::set_fog(float f)
{
    raw_fog = f;
    float r = 1060.282 * pow(f, -0.91);
    f_fog = exp(-0.02 * pow(r, 0.6));
}

// 加载反射率参数
bool LidarModel::load_refdata(const std::string& fname)
{
    std::ifstream infile(fname);
    if (!infile || infile.fail())
        return false;
    std::string strLine;
    _refdatas.clear();
    std::vector<size_t> tags;
    // 读取配置
    while (std::getline(infile, strLine))
    {
        std::stringstream ss(strLine);
        refdata rd;
        size_t tag;
        if (ss >> tag >> rd.minr >> rd.maxr >> rd.deta)
        {
            int N = 1024;
            std::vector<double> x(N);
            for (int i = 0; i < N; i++)
            {
                x[i] = (get_random() + 1) * 0.5;
            }
            rd.brownian = fbm::fractional_brownian_bridge_1d(0.5, N, x);
            rd.bid = MIN(MAX(0, N - 1), int((get_random() + 1) * 0.5 * N));

            _refdatas[tag] = rd;
            tags.push_back(tag);
        }
    }
    // 处理成tagc, tag_t
    for (auto tag : tags)
    {
        size_t tag_c = tag / 100;
        size_t tag_t = tag % 100;

        size_t tag_c3 = tag_c - (tag_c % 10000);
        size_t tag_c2 = tag_c - (tag_c % 10000000);

        tag_c3 *= 100;
        tag_c2 *= 100;
        tag_c *= 100;
        if (_refdatas.find(tag_c3 + tag_t) == _refdatas.end())
        {
            _refdatas[tag_c3 + tag_t] = _refdatas[tag];
        }
        if (_refdatas.find(tag_c2 + tag_t) == _refdatas.end())
        {
            _refdatas[tag_c2 + tag_t] = _refdatas[tag_c3 + tag_t];
        }

        if (_refdatas.find(tag_c) == _refdatas.end())
        {
            _refdatas[tag_c] = _refdatas[tag];
        }
        if (_refdatas.find(tag_c3) == _refdatas.end())
        {
            _refdatas[tag_c3] = _refdatas[tag_c];
        }
        if (_refdatas.find(tag_c2) == _refdatas.end())
        {
            _refdatas[tag_c2] = _refdatas[tag_c3];
        }
    }

    return true;
}

// 布朗运动更新
void LidarModel::update_brownion_pos()
{
    for (auto& ref : _refdatas)
    {
        ref.second.bid = (ref.second.bid + 1) % ref.second.brownian.size();
    }
}

// 获取反射率参数
bool LidarModel::get_refection_param(
    unsigned int tag_c, unsigned int tag_t, float& refmin, float& refmax, float& refdelta)
{
    if (_refdatas.empty())
    {
        return false;
    }
    size_t tag = ((size_t) tag_c) * 100 + tag_t;
    auto fd = _refdatas.find(tag);
    if (fd == _refdatas.end())
    {
        if (!cache_tag(tag_c, tag_t))
        {
            return false;
        }
        fd = _refdatas.find(tag);
    }
    refmin = fd->second.minr;
    refmax = fd->second.maxr;
    refdelta = fd->second.deta;
    return true;
}

// 反射率仿真
float LidarModel::reflection(unsigned int tag_c, unsigned int tag_t, float& sita)
{
    if (_refdatas.empty())
    {
        return 1.f;
    }
    // 查找参数
    size_t tag = ((size_t) tag_c) * 100 + tag_t;
    auto fd = _refdatas.find(tag);
    if (fd == _refdatas.end())
    {
        if (!cache_tag(tag_c, tag_t))
        {
            return 1.f;
        }
        fd = _refdatas.find(tag);
    }
    // 随机噪声
    sita = MIN(MAX(0.f, sita + 0.2f * (get_random() + 0.5f)), 1.f);    // 30du
    // 添加布朗噪声
    float u = fd->second.brownian.at(fd->second.bid) + fd->second.deta * get_random();
    return fd->second.minr + (fd->second.maxr - fd->second.minr) * MIN(MAX(0.f, u), 1.f);
}

// 雨量仿真
float LidarModel::factor_rain(float d)
{
    return pow(f_rainfall, d);
}

// 大雾仿真
float LidarModel::factor_fog(float d)
{
    return pow(f_fog, d);
}

// 高斯白噪声
void LidarModel::gausswhite_noise(float& d)
{
    d += f_accuracy * get_random() * f_noise_dev;
}

// 雪量仿真
void LidarModel::snow_noise(float& d)
{
    float p = get_random();
    if (p < f_snowfall_prob)
        return;
    float nd = f_snowfall_range * (get_random() * 0.5f + 0.5f) + 0.2;
    d = MIN(nd, d);
}

// 随机
float LidarModel::get_random()
{
    return random_value[rand() % random_value.size()];
}
