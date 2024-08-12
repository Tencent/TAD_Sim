// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_scene_loader.h"

#if __TX_Mark__("profile parser func")

Base::ISceneLoader::acc_invalid_type s2accInvalidType(const Base::txString& strProfile) TX_NOEXCEPT;

std::vector<Base::ISceneLoader::EventEndCondition_t> acc_invalid_type_threshold_parser(Base::txString strProfile)
    TX_NOEXCEPT;

std::vector<std::pair<Base::txFloat, Base::txFloat> > float_float_pair_parser(Base::txString strProfile) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > float_int_float_tuple_parser(
    Base::txString strProfile) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
float_int_pair_parser(Base::txString strProfile) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                       Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
float_int_pair_parser(Base::txString strProfile, Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                       Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
float_int_pair_parser_with_projectionType(Base::txString strProfile,
                                          Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                       Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
float_int_pair_parser_with_projectionType_with_triggerIndex(
    Base::txString strProfile, Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_float_pair_parser(Base::txString strProfile, Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_float_pair_parser_with_projectionType(Base::txString strProfile,
                                            Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_float_pair_parser_with_projectionType_triggerIndex(Base::txString strProfile,
                                                         Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_int_float_tuple_parser(Base::txString strProfile,
                             Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
shuttle_float_int_float_tuple_parser(Base::txString strProfile,
                                     Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_int_float_tuple_parser_with_projectionType(Base::txString strProfile,
                                                 Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT;

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_int_float_tuple_parser_with_projectionType_with_triggerIndex(
    Base::txString strProfile, Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT;

#endif /*__TX_Mark__("profile parser func")*/
