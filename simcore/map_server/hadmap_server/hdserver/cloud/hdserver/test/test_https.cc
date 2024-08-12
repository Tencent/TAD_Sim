// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include "https_client.h"

using namespace std;

TEST(testHTTPS, test) {
  cout << "test https " << endl;

  try {
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    ctx.set_default_verify_paths();

    boost::asio::io_service io_service;
    ::https::client c(io_service, ctx, "localhost", "/");
    io_service.run();
  } catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << "\n";
  }
}
