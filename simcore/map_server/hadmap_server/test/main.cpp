
// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <scene_wrapper.h>
#include <iostream>

int testLoad(const wchar_t* strHadmap) {
  loadHadmap(strHadmap);
  getRoadData(strHadmap);
  getLaneData(strHadmap);
  getLaneBoundaryData(strHadmap);
  getLaneLinkData(strHadmap);
  getMapObjectData(strHadmap);
  return 0;
}

int main(int argc, char** argv) {
  std::cout << "scenario server test service started!" << std::endl;

  init(L"C:\\Users\\wangheng\\AppData\\Roaming\\Electron\\scenario");

  testInfo(L"scenario server test started");

  int nLoadCount = 10;
  for (int i = 0; i < nLoadCount; ++i) {
    std::cout << "loop: " << i << std::endl;
    testLoad(L"d2d_20190726.xodr");
    testLoad(L"1001-1-101-180324-v0.0.1.sqlite");
    testLoad(L"geely.sqlite");
  }

  testInfo(L"scenario server test end!");

  deinit();
  std::cout << "scenario server test service exited!" << std::endl;

  std::cout << "press any key to exit ..." << std::endl;

  std::string strInfo;
  std::cin >> strInfo;

  return 0;
}
