// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "status.h"

namespace tx_sim {
namespace coordinator {

class SimStatusWriter {
 public:
  virtual void Write(const CommandStatus& st){};
};

}  // namespace coordinator
}  // namespace tx_sim
