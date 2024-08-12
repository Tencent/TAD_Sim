// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <memory>
#include <type_traits>
#include <vector>

class txApplication {
 public:
  txApplication();
  ~txApplication() = default;

 public:
  static void Initialize(int argc, char* argv[]) noexcept;
  static int Run(int argc, char* argv[]) noexcept;
  static void Terminate() noexcept;

 public:
  using InitFunc = std::add_pointer<void(int, char*[])>::type;
  using ReleaseFunc = std::add_pointer<void()>::type;

  static std::vector<InitFunc> ms_VecInitFunc;
  static std::vector<ReleaseFunc> ms_VecReleaseFunc;
  static bool ms_bInitialized;

 protected:
  static void InitLogger(int argc, char* argv[]) noexcept;
  static void ReleaseLogger() noexcept;

  static void InitCommandLineArguments(int argc, char* argv[]) noexcept;
  static void ReleaseCommandLineArguments() noexcept;

  static std::unique_ptr<txApplication> ms_InstancePtr;
};
