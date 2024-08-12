// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <map>
#include <string>

struct AgeValue {
  int age = 0;
  double timestamp = 0;
  std::map<std::string, double> values;
};

struct ValueBlock {
  const AgeValue* last = 0;
  AgeValue* current = 0;
  bool last_has(const std::string& key) const {
    if (last) {
      return last->values.find(key) != last->values.end();
    }
    return false;
  }
};

class Age {
 public:
  void NewYear(double timestamp = 0);
  void YearEnd();
  int operator()(int id);
  ValueBlock get_value(int id);

 private:
  double this_timestamp = 0;
  std::map<int, AgeValue> _data;
  std::map<int, AgeValue> _tmp;
};
