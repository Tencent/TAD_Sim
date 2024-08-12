// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <memory>

#include "common/utils/oid_generator.h"
#include "common/utils/spin_lock.h"

using namespace std;
using namespace tad::sim;

TEST(testOidGen, testGen) {
  {
    boost::uuids::random_generator oid_generator;

    boost::uuids::uuid uuid = oid_generator();

    std::cout << "destruct oid generator here." << std::endl;
  }

  cout << "test oid generator " << endl;
  {
    ObjectIdGenerator od;
    std::string id = od.Next();
    // std::string id;
    cout << "id is: " << id << std::endl;
  }
}

// unfortunately, spin lock is not faster than std::mutex ...
TEST(testUtils, testSpinLock) {
  cout << "test spin lock" << endl;

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  SpinLock lk;

  volatile int value = 0;
  const int num_workers = 5;

  std::mutex _m;
  // std::unique_lock<std::mutex> lock(_m);

  const int limit = 1000000;
  auto task = [limit, &lk, &value, &_m]() {
    std::cout << "Started  " << std::this_thread::get_id() << " up to : " << limit << std::endl;
    for (int i = 0; i < limit; ++i) {
      // _m.lock();
      lk.lock();
      value++;
      // std::this_thread::sleep_for(std::chrono::microseconds(1));
      // _m.unlock();
      lk.unlock();
      // std::cout << "Value  " << value << std::endl;
      // l.unlock();
    }

    return;
  };

  std::vector<std::thread> threads;
  std::cout << "SpinLock inc MyTest start" << std::endl;
  value = 0;
  for (int i = 0; i < num_workers; ++i) {
    threads.push_back(std::move(std::thread(task)));
  }

  for (auto it = threads.begin(); it != threads.end(); it++) it->join();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  cout << "start compute lane rel (bg) done. cost: " << (end - begin).count() / 1000 << std::endl;

  cout << "value is: " << value << " gt is: " << limit * num_workers << std::endl;
  EXPECT_EQ(value, limit * num_workers);
  cout << "test spin lock done" << endl;
}
