/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include <iostream>
#include <string>
#include "cgo_message.pb.h"
using namespace std;

int main(int argc, char* argv[]) {
  // GOOGLE_PROTOBUF_VERIFY_VERSION;

  hadmap::CloudUploadHadmapCallback cb;

  // 给消息类Student对象student赋值

  cb.add_mapnames("map1");
  cb.add_mapnames("map2");
  cb.add_mapnames("利用anaconda的解决");

  // 对消息对象student序列化到string容器
  string serializedStr;
  cb.SerializeToString(&serializedStr);
  // cout<<"serialization result:"<<serializedStr<<endl; //序列化后的字符串内容是二进制内容，非可打印字符，预计输出乱码
  cout << endl << "debugString:" << cb.DebugString();

  /*----------------上面是序列化，下面是反序列化-----------------------*/
  // 解析序列化后的消息对象，即反序列化
  hadmap::CloudUploadHadmapCallback deserCb;
  if (!deserCb.ParseFromString(serializedStr)) {
    cerr << "Failed to parse student." << endl;
    return -1;
  }

  cout << "-------------上面是序列化，下面是反序列化---------------" << endl;
  // 打印解析后的student消息对象
  cout << "deserCb debugString:" << deserCb.DebugString();

  for (int j = 0; j < deserCb.mapnames_size(); j++) {
    std::cout << deserCb.mapnames(j) << std::endl;
  }
}