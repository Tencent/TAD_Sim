/**
 * @file Noise.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "noise.h"
#include <random>
#include <sstream>
#include "math.h"

/**
 * @brief noise model init
 *
 * @param a coefficient of a
 * @param b coefficient of b
 * @param c coefficient of c
 * @param d coefficient of d
 * @param sigma sigma of gauss distribution
 * @param pmax maximum of probability
 */
void NoiseDrop::init(double a, double b, double c, double d, double sigma, double pmax) {
  this->a = a;
  this->b = b;
  this->c = c;
  this->d = d;
  this->sigma = sigma;
  this->pmax = pmax;
  gauss = std::normal_distribution<double>(0, sigma);
  normal = std::uniform_real_distribution<double>(0.0, 1.0);
}

/**
 * @brief judge if a drop occurs
 *
 * @param dis distance
 * @return true if drop occurs
 * @return false if drop does not occur
 */
bool NoiseDrop::is_drop(double dis) {
  double h = d + c * dis;
  dis *= dis;
  h += b * dis;
  dis *= dis;
  h += a * dis;
  h += 0.01 * gauss(generator);
  double p = normal(generator);
  return p <= std::min(pmax, h);
}

/**
 * @brief noise model init
 *
 * @param a coefficient of a
 * @param b coefficient of b
 * @param c coefficient of c
 * @param d coefficient of d
 */
void NoiseJitter::init(double a, double b, double c, double d) {
  this->a = a;
  this->b = b;
  this->c = c;
  this->d = d;
}

/**
 * @brief jitter noise
 *
 * @param dis distance
 * @return jittered distance
 */
double NoiseJitter::jitter(double dis) {
  double h = d + c * dis;
  dis *= dis;
  h += b * dis;
  dis *= dis;
  h += a * dis;
  std::normal_distribution<double> gauss(0, h);
  return gauss(generator);
}
/**
 * @brief initialize noise drop model by reading config string
 *
 * @param str string of format 'a, b, c, d, [pmax]'
 * @return true on success
 * @return false on failure
 */
bool Noise::initDrop(const std::string &str) {
  auto abcd = ParseABCD(str);
  if (abcd.size() < 4) {
    std::cout << "Noise drop parse error: " << str << std::endl;
    return false;
  }
  hasDrop = abcd.size() > 5 ? abcd[5] : 1;
  drop.init(abcd[0], abcd[1], abcd[2], abcd[3], abcd.size() > 4 ? abcd[4] : 1, hasDrop);
  return true;
}
/**
 * @brief initialize noise stay model by reading config string
 *
 * @param str string of format 'a, b, c, d, [pmax]'
 * @return true on success
 * @return false on failure
 */
bool Noise::initStay(const std::string &str) {
  auto abcd = ParseABCD(str);
  if (abcd.size() < 4) {
    std::cout << "Noise stay parse error: " << str << std::endl;
    return false;
  }
  hasStay = abcd.size() > 5 ? abcd[5] : 1;
  stay.init(abcd[0], abcd[1], abcd[2], abcd[3], abcd.size() > 4 ? abcd[4] : 1, hasStay);
  return true;
}
/**
 * @brief initialize noise position model by reading config string
 *
 * @param str string of format 'a, b, c, d, [pmax]'
 * @return true on success
 * @return false on failure
 */
bool Noise::initPosition(const std::string &str) {
  auto abcd = ParseABCD(str);
  if (abcd.size() < 4) {
    std::cout << "Noise position parse error: " << str << std::endl;
    return false;
  }
  position.init(abcd[0], abcd[1], abcd[2], abcd[3]);
  hasPos = abcd.size() > 4 ? abcd[4] : 999;
  return true;
}
/**
 * @brief initialize noise size model by reading config string
 *
 * @param str string of format 'a, b, c, d, [pmax]'
 * @return true on success
 * @return false on failure
 */
bool Noise::initSize(const std::string &str) {
  auto abcd = ParseABCD(str);
  if (abcd.size() < 4) {
    std::cout << "Noise size parse error: " << str << std::endl;
    return false;
  }
  size.init(abcd[0], abcd[1], abcd[2], abcd[3]);
  hasSize = abcd.size() > 4 ? abcd[4] : 999;
  return true;
}
/**
 * @brief initialize noise veolity model by reading config string
 *
 * @param str string of format 'a, b, c, d, [pmax]'
 * @return true on success
 * @return false on failure
 */
bool Noise::initVeolity(const std::string &str) {
  auto abcd = ParseABCD(str);
  if (abcd.size() < 4) {
    std::cout << "Noise veolity parse error: " << str << std::endl;
    return false;
  }
  veolity.init(abcd[0], abcd[1], abcd[2], abcd[3]);
  hasVeo = abcd.size() > 4 ? abcd[4] : 999;
  return true;
}
/**
 * @brief noise data
 *
 * @param data data to be noise
 * @return true on success
 * @return false on failure
 */
bool Noise::noise(Data &data) {
  if (hasStay > 0) {
    // stay noise
    auto bck = data;
    if (stay.is_drop(data.distance)) {
      if (cache.find(data.id) != cache.end()) {
        if (data.time - cache.at(data.id).time <= age) {
          data = cache.at(data.id);
        }
      }
    }
    cache[bck.id] = bck;
  }
  // drop noise
  if (hasDrop > 0 && drop.is_drop(data.distance)) {
    return false;
  }
  // position noise
  if (hasPos > 0) {
    data.x += std::max(-hasPos, std::min(hasPos, position.jitter(data.distance)));
    data.y += std::max(-hasPos, std::min(hasPos, position.jitter(data.distance)));
    data.z += std::max(-hasPos, std::min(hasPos, position.jitter(data.distance)));
  }
  // size noise
  if (hasSize > 0) {
    data.len = std::max(0.0, data.len + std::max(-hasSize, std::min(hasSize, size.jitter(data.distance))));
    data.wid = std::max(0.0, data.wid + std::max(-hasSize, std::min(hasSize, size.jitter(data.distance))));
    data.hei = std::max(0.0, data.hei + std::max(-hasSize, std::min(hasSize, size.jitter(data.distance))));
  }
  // veolity noise
  if (hasVeo > 0) {
    data.vx += std::max(-hasVeo, std::min(hasVeo, veolity.jitter(data.distance)));
    data.vy += std::max(-hasVeo, std::min(hasVeo, veolity.jitter(data.distance)));
    data.vz += std::max(-hasVeo, std::min(hasVeo, veolity.jitter(data.distance)));
  }
  return true;
}
/**
 * @brief parse abcd from string
 *
 * @param str string to parse
 * @return vector of abcd
 */
std::vector<double> Noise::ParseABCD(const std::string &str) {
  std::vector<double> rt;
  if (str.empty()) {
    return rt;
  }
  std::stringstream sstr(str);
  std::string token;
  while (std::getline(sstr, token, ',')) {
    rt.push_back(atof(token.c_str()));
  }
  return rt;
}
