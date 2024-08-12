/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once
#include "my_buffer_capi.h"
#include "my_buffer.h"

struct MyBuffer_T : MyBuffer {
  explicit MyBuffer_T(int size) : MyBuffer(size) {}
  ~MyBuffer_T() {}
};

MyBuffer_T* NewMyBuffer(int size) {
  auto p = new MyBuffer_T(size);
  return p;
}
void DeleteMyBuffer(MyBuffer_T* p) { delete p; }

char* MyBuffer_Data(MyBuffer_T* p) { return p->Data(); }
int MyBuffer_Size(MyBuffer_T* p) { return p->Size(); }
