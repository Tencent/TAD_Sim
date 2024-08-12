/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

struct tagStateItem {
  std::string strStateType;
  std::string strStateName;
  double dTriggerTime;
  int nValue;
};

class CL3States {
 public:
  typedef std::vector<tagStateItem> StateItems;

  CL3States() {}

  bool IsEmpty();
  StateItems& States() { return m_states; }
  void AddState(tagStateItem& item);

 protected:
  StateItems m_states;
};
