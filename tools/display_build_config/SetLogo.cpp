/**
 * @file SetLogo.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-12
 * 
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 * 
 */
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "mini/ini.h"


int main(int argn, char** argv) {
  if (argn < 2) {
    std::cout << "input logo name\n";
    return 1;
  }
  std::string curpath = argv[0];
  int p = std::max((int)curpath.find_last_of('\\'), (int)curpath.find_last_of('/'));
  if (p > 0) {
    curpath = curpath.substr(0, p);
  } else {
    curpath = ".";
  }
  std::cout << curpath + "/Config/DefaultGame.ini";
  mINI::INIFile file(curpath + "/Config/DefaultGame.ini");
  mINI::INIStructure ini;

  // file.read(ini);
  ini["LogoConfig"]["LogoName"] = std::string(argv[1]);
  file.write(ini);
  return 0;
}
