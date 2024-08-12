/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <boost/noncopyable.hpp>
#include <string>

class FileLock {
 public:
  FileLock(const char* name, int fd) : fd_(fd), name_(name) {
    if (fd_ > 0) {
      Lock();
    }
  }
  ~FileLock() {
    if (fd_ > 0) {
      Unlock();
    }
  }

 private:
  int Create() {
    fd_ = open(name_.c_str(), O_RDWR, 0664);
    if (fd_ < 0) {
      std::cerr << "open file failed, name: " << name_ << ", error: " << strerror(errno) << std::endl;
      return -1;
    }
    return 0;
  }

  int Lock() {
    if (fd_ < 0) {
      return 0;
    }
    struct flock fl;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (fcntl(fd_, F_SETLKW, &fl) < 0) {
      std::cerr << "lock file failed, name: " << name_ << ", error: " << strerror(errno) << std::endl;
      return -1;
    }
    return 0;
  }

  int UnLock() {
    if (fd_ < 0) {
      return 0;
    }
    struct flock fl;
    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(fd_, F_SETLKW, &fl) < 0) {
      std::cerr << "unlock file failed, name: " << name_ << ", error: " << strerror(errno) << std::endl;
      return -1;
    }
    return 0;
  }

 public:
  int fd_;
  std::string name_;
};
