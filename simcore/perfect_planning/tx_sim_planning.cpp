// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_header.h"
#include "tx_planning_application.h"
int main(int argc, char* argv[]) {
  txApplicationPtr appPtr = std::make_shared<TADSim_Planning_Application>();
  if (NonNull_Pointer(appPtr)) {
    appPtr->Initialize(argc, argv);
    int nExitCode = appPtr->Run(argc, argv);
    appPtr->Terminate();
    return nExitCode;
  } else {
    LOG(WARNING) << "Application create failure.";
    return ErrExitCode;
  }
}
