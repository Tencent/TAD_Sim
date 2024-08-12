// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "location.pb.h"
#include "traffic.pb.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_sim_time.h"
TX_NAMESPACE_OPEN(Base)
struct Info_NearestObject;

class SceneSketchBase {
 public:
  using SketchEnumType = Base::Enums::SketchEnumType;

  SceneSketchBase() TX_DEFAULT;
  virtual ~SceneSketchBase() TX_DEFAULT;

  /**
   * @brief 初始化画笔
   *
   * 初始化画笔，通常用于创建画笔之前需要执行的操作。
   * 需要注意的是，由于该函数没有返回值，需要确保该方法不会引发异常。
   */
  virtual void InitSketch() TX_NOEXCEPT {}

  /**
   * @brief 释放画笔
   *
   * 释放画笔，释放资源或者将画笔还原到初始状态。
   * 此函数没有返回值，因此可以使用 "void" 作为返回类型。
   */
  virtual void ReleaseSketch() TX_NOEXCEPT {}

  /**
   * @brief 获取绘图类型的大小
   *
   * 这个函数返回当前绘图类型所占用的字节大小。
   *
   * @return Base::txSize 当前绘图类型所占用的字节大小
   */
  const Base::txSize SketchTypeSize() const TX_NOEXCEPT { return Base::Enums::szSketchEnumType; }

  /**
   * @brief 更新绘制素材
   *
   * 更新绘制素材，根据传入的时间参数和交通状况，更新需要绘制的素材大小和素材分布等。
   *
   * @param timeMgr 时间参数管理器
   * @param refTraffic 参考交通状况
   * @return Base::txSize 更新后的素材总大小
   */
  virtual Base::txSize UpdateSketch(const Base::TimeParamManager& timeMgr,
                                    const sim_msg::Traffic& refTraffic) TX_NOEXCEPT {
    return 0;
  }

  /**
   * @brief 触发场景投票
   * @return _plus_(SketchEnumType::default_scene) 投票结果，返回默认场景（使用 _plus_ 宏）
   */
  virtual SketchEnumType SketchVoting() const TX_NOEXCEPT { return _plus_(SketchEnumType::default_scene); }
};
using SceneSketchBasePtr = std::shared_ptr<SceneSketchBase>;

TX_NAMESPACE_CLOSE(Base)
