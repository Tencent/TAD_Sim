/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once
// my_buffer.h
#include <string>

struct MyBuffer {
  std::string* s_;

  explicit MyBuffer(int size) { this->s_ = new std::string(size, static_cast<char>('\0')); }
  ~MyBuffer() { delete this->s_; }

  int Size() const { return this->s_->size(); }
  char* Data() { return reinterpret_cast<char*>(this->s_->data()); }
};
