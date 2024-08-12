/**
 * @file Age.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "age.h"

/**
 * @brief year end
 *
 */
void Age::YearEnd() {
  _data = _tmp;
  _tmp.clear();
}

/**
 * @brief
 *
 * @param timestamp
 */
void Age::NewYear(double timestamp) { this_timestamp = timestamp; }

/**
 * @brief add a person to the age
 *
 * @param id id of person
 * @return age of person
 */
int Age::operator()(int id) {
  auto &na = _tmp[id];
  na.timestamp = this_timestamp;
  na.age = 1;
  auto a = _data.find(id);
  if (a != _data.end()) {
    na.age += a->second.age;
  }
  return na.age;
}

ValueBlock Age::get_value(int id) {
  auto &na = _tmp[id];
  na.timestamp = this_timestamp;
  ValueBlock vb;
  vb.last = 0;
  vb.current = &na;
  auto a = _data.find(id);
  if (a != _data.end()) vb.last = &(_data.at(id));
  return vb;
}
