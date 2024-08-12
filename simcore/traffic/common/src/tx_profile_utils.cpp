// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_profile_utils.h"

#if __TX_Mark__("profile parser func")

Base::ISceneLoader::acc_invalid_type s2accInvalidType(const Base::txString& strProfile) TX_NOEXCEPT {
  if ("Time" == strProfile) {
    return Base::ISceneLoader::acc_invalid_type::eTime;
  } else if ("Velocity" == strProfile) {
    return Base::ISceneLoader::acc_invalid_type::eVelocity;
  } else {
    return Base::ISceneLoader::acc_invalid_type::eNone;
  }
}

std::vector<Base::ISceneLoader::EventEndCondition_t> acc_invalid_type_threshold_parser(Base::txString strProfile)
    TX_NOEXCEPT {
  TX_MARK("endCondition=\"None, 0.0; Time, 2.3; Velocity, 8.0; None, 0.0\"");
  std::vector<Base::ISceneLoader::EventEndCondition_t> retVec;
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> results_pairs;
    boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(";"));

    for (auto s : results_pairs) {
      Utils::trim(s);
      if (!s.empty()) {
        std::vector<Base::txString> results_type_threshold;
        boost::algorithm::split(results_type_threshold, s, boost::is_any_of(","));
        if ((2 == results_type_threshold.size())) {
          Utils::trim(results_type_threshold[0]);
          Utils::trim(results_type_threshold[1]);
          if ((!results_type_threshold[0].empty()) && (!results_type_threshold[1].empty())) {
            Base::ISceneLoader::EventEndCondition_t tmpNode;
            tmpNode.m_IsValid = true;
            tmpNode.m_type = s2accInvalidType(results_type_threshold[0]);
            tmpNode.m_endCondition = std::stod(results_type_threshold[1]);
            retVec.emplace_back(tmpNode);
            /*retVec.emplace_back(std::make_pair(s2accInvalidType(results_type_threshold[0]),
                std::stod(results_type_threshold[1])));*/
          }
        }
      }
    }
  }
  return retVec;
}
std::vector<std::pair<Base::txFloat, Base::txFloat> > float_float_pair_parser(Base::txString strProfile) TX_NOEXCEPT {
  TX_MARK("profile=\"0.0, 0.0; 5.0, -2.0; 9.1, 0.0\"");
  std::vector<std::pair<Base::txFloat, Base::txFloat> > retVec;
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> results_pairs;
    boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(";"));

    for (auto s : results_pairs) {
      Utils::trim(s);
      if (!s.empty()) {
        std::vector<Base::txString> results_time_acc;
        boost::algorithm::split(results_time_acc, s, boost::is_any_of(","));
        if ((results_time_acc.size() > 1)) {
          Utils::trim(results_time_acc[0]);
          Utils::trim(results_time_acc[1]);
          if ((!results_time_acc[0].empty()) && (!results_time_acc[1].empty())) {
            retVec.emplace_back(std::make_pair(std::stod(results_time_acc[0]), std::stod(results_time_acc[1])));
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > float_int_float_tuple_parser(
    Base::txString strProfile) TX_NOEXCEPT {
  TX_MARK("profile=\"time,direction,speed;time,direction,speed\"");
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > retVec;
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> results_pairs;
    boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(";"));

    for (auto s : results_pairs) {
      Utils::trim(s);
      if (!s.empty()) {
        std::vector<Base::txString> results_time_acc;
        boost::algorithm::split(results_time_acc, s, boost::is_any_of(","));
        if ((3 == results_time_acc.size())) {
          Utils::trim(results_time_acc[0]);
          Utils::trim(results_time_acc[1]);
          Utils::trim(results_time_acc[2]);
          if ((!results_time_acc[0].empty()) && (!results_time_acc[1].empty()) && (!results_time_acc[2].empty())) {
            retVec.emplace_back(std::make_tuple(std::stod(results_time_acc[0]), std::stod(results_time_acc[1]),
                                                std::stod(results_time_acc[2])));
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
float_int_pair_parser(Base::txString strProfile) TX_NOEXCEPT {
  TX_MARK("profile=\"0.0, +1; 5.0, 0; 9.1, -1\"");
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> > retVec;
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> results_pairs;
    boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(";"));

    for (auto s : results_pairs) {
      Utils::trim(s);
      if (!s.empty()) {
        std::vector<Base::txString> results_time_acc;
        boost::algorithm::split(results_time_acc, s, boost::is_any_of(","));
        if ((3 == results_time_acc.size())) {
          Utils::trim(results_time_acc[0]);
          Utils::trim(results_time_acc[1]);
          Utils::trim(results_time_acc[2]);
          if ((!results_time_acc[0].empty()) && (!results_time_acc[1].empty()) && (!results_time_acc[2].empty())) {
            retVec.emplace_back(std::make_tuple(std::stod(results_time_acc[0]), std::stoi(results_time_acc[1]),
                                                std::stod(results_time_acc[2]), FLAGS_DefaultMergeOffset));
          }
        } else if ((4 == results_time_acc.size())) {
          Utils::trim(results_time_acc[0]);
          Utils::trim(results_time_acc[1]);
          Utils::trim(results_time_acc[2]);
          Utils::trim(results_time_acc[3]);
          if ((!results_time_acc[0].empty()) && (!results_time_acc[1].empty()) && (!results_time_acc[2].empty()) &&
              (!results_time_acc[3].empty())) {
            if ("v" == results_time_acc[3]) {
              results_time_acc[3] = "0.0";
            }
            retVec.emplace_back(std::make_tuple(std::stod(results_time_acc[0]), std::stoi(results_time_acc[1]),
                                                std::stod(results_time_acc[2]), std::stod(results_time_acc[3])));
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                       Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
float_int_pair_parser(Base::txString strProfile, Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(strType);
  TX_MARK("profile=\"ttc 3.0,1;egodistance 6.0,-1\"");
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(strType);
  to_lower(strType_LowerCase);
  Utils::trim(strProfile);

  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc 3.0,1   [2]egodistance 6.0,-1");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc 3.0, 1'");
      Utils::trim(subProfile);

      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]3.0,1");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (2 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);

        if (strType_LowerCase == strType) {
          Base::txString strProfile = split_type_profile[1];
          TX_MARK("3.0, 1");
          std::vector<Base::txString> results_pairs;
          boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(","));
          if (2 == results_pairs.size()) {
            Utils::trim(results_pairs[0]);
            Utils::trim(results_pairs[1]);
            if ((!results_pairs[0].empty()) && (!results_pairs[1].empty())) {
              retVec.emplace_back(std::make_tuple(std::stod(results_pairs[0]), std::stoi(results_pairs[1]),
                                                  Base::ISceneLoader::DistanceProjectionType::Lane,
                                                  FLAGS_DefaultMergeDuration, FLAGS_DefaultMergeOffset,
                                                  FLAGS_DefaultTriggerIndex));
            }
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                       Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
float_int_pair_parser_with_projectionType(Base::txString strProfile,
                                          Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(strType);
  TX_MARK("profile=\"ttc laneprojection 3.0, 1;egodistance laneprojection 6.0, -1\"");
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(strType);
  to_lower(strType_LowerCase);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc laneprojection 3.0, 1   [2]egodistance laneprojection 6.0, -1");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc laneprojection 3.0, 1'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]laneprojection [3]3.0, 1");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (3 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString strProjectionType = split_type_profile[1];
          TX_MARK("laneprojection/euclideandistance");
          Utils::trim(strProjectionType);
          to_lower(strProjectionType);
          const Base::ISceneLoader::DistanceProjectionType projType =
              Base::ISceneLoader::IViewer::EvaluateProjectionType(strProjectionType);

          Base::txString strProfile = split_type_profile[2];
          TX_MARK("3.0, 1");
          std::vector<Base::txString> results_pairs;
          boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(","));
          if (2 == results_pairs.size()) {
            Utils::trim(results_pairs[0]);
            Utils::trim(results_pairs[1]);
            if ((!results_pairs[0].empty()) && (!results_pairs[1].empty())) {
              retVec.emplace_back(std::make_tuple(std::stod(results_pairs[0]), std::stoi(results_pairs[1]), projType,
                                                  FLAGS_DefaultMergeDuration, FLAGS_DefaultMergeOffset,
                                                  FLAGS_DefaultTriggerIndex));
            }
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                       Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
float_int_pair_parser_with_projectionType_with_triggerIndex(
    Base::txString strProfile, Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(strType);
  TX_MARK("profile=\"ttc laneprojection 3.0, 1,4.5,v [4];egodistance laneprojection 6.0, -2,3.0,0.5 [1]\"");
  std::vector<std::tuple<Base::txFloat, Base::txInt, Base::ISceneLoader::DistanceProjectionType,
                         Base::txFloat /*Duration*/, Base::txFloat /*offset*/, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(strType);
  to_lower(strType_LowerCase);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc laneprojection 3.0, 1,4.5,v [4]   [2]egodistance laneprojection 6.0, -2,3.0,0.5 [1]");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc laneprojection 3.0, 1'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]laneprojection [3]3.0,1 [4][4]");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (4 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString strProjectionType = split_type_profile[1];
          TX_MARK("laneprojection/euclideandistance");
          Utils::trim(strProjectionType);
          to_lower(strProjectionType);
          const Base::ISceneLoader::DistanceProjectionType projType =
              Base::ISceneLoader::IViewer::EvaluateProjectionType(strProjectionType);

          Base::txString strProfile = split_type_profile[2];
          TX_MARK("3.0,1,4.5,v");
          std::vector<Base::txString> results_pairs;
          boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(","));
          if (4 == results_pairs.size()) {
            Utils::trim(results_pairs[0]);
            Utils::trim(results_pairs[1]);
            Utils::trim(results_pairs[2]);
            Utils::trim(results_pairs[3]);
            if ((!results_pairs[0].empty()) && (!results_pairs[1].empty()) && (!results_pairs[2].empty()) &&
                (!results_pairs[3].empty())) {
              if ("v" == results_pairs[3]) {
                results_pairs[3] = "0.0";
              }

              Base::txString strTriggerIndex = split_type_profile[3];
              TX_MARK("[4]");
              Utils::trim(strTriggerIndex);
              to_lower(strTriggerIndex);
              if (strTriggerIndex.size() >= 3) {
                strTriggerIndex = strTriggerIndex.substr(1, strTriggerIndex.size() - 2);
              }
              retVec.emplace_back(std::make_tuple(std::stod(results_pairs[0]), std::stoi(results_pairs[1]), projType,
                                                  std::stod(results_pairs[2]), std::stod(results_pairs[3]),
                                                  std::stod(strTriggerIndex)));
            }
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_float_pair_parser(Base::txString strProfile, Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(objStrType);
  TX_MARK("profile=\"ttc 3.0, 1;egodistance 6.0, -1\"");
  TX_MARK("projection type default value is laneprojection");
  std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(objStrType);
  to_lower(strType_LowerCase);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc 3.0, 1   [2]egodistance 6.0, -1");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc 3.0, 1'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]3.0, 1");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (2 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString strProfile = split_type_profile[1];
          TX_MARK("3.0, 1");
          std::vector<Base::txString> results_pairs;
          boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(","));
          if (2 == results_pairs.size()) {
            Utils::trim(results_pairs[0]);
            Utils::trim(results_pairs[1]);
            if ((!results_pairs[0].empty()) && (!results_pairs[1].empty())) {
              retVec.emplace_back(std::make_tuple(std::stod(results_pairs[0]), std::stod(results_pairs[1]),
                                                  Base::ISceneLoader::DistanceProjectionType::Lane,
                                                  FLAGS_DefaultTriggerIndex));
            }
          }
        } else {
          retVec.emplace_back(std::make_tuple(0.0, 0.0, Base::ISceneLoader::DistanceProjectionType::Lane, 0));
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_float_pair_parser_with_projectionType(Base::txString strProfile,
                                            Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(strType);
  TX_MARK("profile=\"ttc laneprojection 3.0, 1;egodistance laneprojection 6.0, -1\"");

  std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(strType);
  to_lower(strType_LowerCase);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc laneprojection 3.0, 1   [2]egodistance laneprojection 6.0, -1");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc laneprojection 3.0, 1'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]laneprojection [3]3.0, 1");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (3 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString strProjectionType = split_type_profile[1];
          TX_MARK("laneprojection/euclideandistance");
          Utils::trim(strProjectionType);
          to_lower(strProjectionType);
          const Base::ISceneLoader::DistanceProjectionType projType =
              Base::ISceneLoader::IViewer::EvaluateProjectionType(strProjectionType);

          Base::txString strProfile = split_type_profile[2];
          TX_MARK("3.0, 1");
          std::vector<Base::txString> results_pairs;
          boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(","));
          if (2 == results_pairs.size()) {
            Utils::trim(results_pairs[0]);
            Utils::trim(results_pairs[1]);
            if ((!results_pairs[0].empty()) && (!results_pairs[1].empty())) {
              retVec.emplace_back(std::make_tuple(std::stod(results_pairs[0]), std::stod(results_pairs[1]), projType,
                                                  FLAGS_DefaultTriggerIndex));
            }
          }
        } else {
          retVec.emplace_back(std::make_tuple(0.0, 0.0, Base::ISceneLoader::DistanceProjectionType::Lane, 0));
        }
      }
    }
  }
  return retVec;
}

std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_float_pair_parser_with_projectionType_triggerIndex(Base::txString strProfile,
                                                         Base::txString strType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(strType);
  TX_MARK("profile=\"ttc laneprojection 3.0,1 [4];egodistance laneprojection 6.0,-1 [1]\"");

  std::vector<std::tuple<Base::txFloat, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(strType);
  to_lower(strType_LowerCase);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc laneprojection 3.0,1 [4]   [2]egodistance laneprojection 6.0,-1 [1]");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));
    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc laneprojection 3.0,1 [4]'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]laneprojection [3]3.0,1 [4]");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));
      if (4 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString strProjectionType = split_type_profile[1];
          TX_MARK("laneprojection/euclideandistance");
          Utils::trim(strProjectionType);
          to_lower(strProjectionType);
          const Base::ISceneLoader::DistanceProjectionType projType =
              Base::ISceneLoader::IViewer::EvaluateProjectionType(strProjectionType);

          Base::txString strProfile = split_type_profile[2];
          TX_MARK("3.0,1");
          std::vector<Base::txString> results_pairs;
          boost::algorithm::split(results_pairs, strProfile, boost::is_any_of(","));
          if (2 == results_pairs.size()) {
            Utils::trim(results_pairs[0]);
            Utils::trim(results_pairs[1]);
            if ((!results_pairs[0].empty()) && (!results_pairs[1].empty())) {
              Base::txString strTriggerIndex = split_type_profile[3];
              TX_MARK("[4]");
              Utils::trim(strTriggerIndex);
              to_lower(strTriggerIndex);
              if (strTriggerIndex.size() >= 3) {
                strTriggerIndex = strTriggerIndex.substr(1, strTriggerIndex.size() - 2);
                retVec.emplace_back(std::make_tuple(std::stod(results_pairs[0]), std::stod(results_pairs[1]), projType,
                                                    std::stoi(strTriggerIndex)));
              }
            }
          }
        } else {
          retVec.emplace_back(std::make_tuple(0.0, 0.0, Base::ISceneLoader::DistanceProjectionType::Lane, 0));
        }
      }
    }
  }
  return retVec;
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_int_float_tuple_parser(Base::txString strProfile,
                             Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(objStrType);
  std::vector<
      std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(objStrType);
  to_lower(strType_LowerCase);
  to_lower(strProfile);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc 3.0, 1   [2]egodistance 6.0, -1");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc 3.0, 1'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]3.0, 1");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (2 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString s = split_type_profile[1];
          std::vector<Base::txString> results_threshold_direction_velocity;
          boost::algorithm::split(results_threshold_direction_velocity, s, boost::is_any_of(","));
          if ((3 == results_threshold_direction_velocity.size())) {
            Utils::trim(results_threshold_direction_velocity[0]);
            Utils::trim(results_threshold_direction_velocity[1]);
            Utils::trim(results_threshold_direction_velocity[2]);
            if ((!results_threshold_direction_velocity[0].empty()) &&
                (!results_threshold_direction_velocity[1].empty()) &&
                (!results_threshold_direction_velocity[2].empty())) {
              retVec.emplace_back(
                  std::make_tuple(std::stod(results_threshold_direction_velocity[0]) TX_MARK("threshold"),
                                  std::stoi(results_threshold_direction_velocity[1]) TX_MARK("direction"),
                                  std::stod(results_threshold_direction_velocity[2]) TX_MARK("velocity"),
                                  Base::ISceneLoader::DistanceProjectionType::Lane, FLAGS_DefaultTriggerIndex));
            }
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
shuttle_float_int_float_tuple_parser(Base::txString strProfile,
                                     Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  /*<shuttle id="1" profile="egoDistance 5.000000,90.000000,6.330000 [1];ttc 5.000000,90.000000,6.330000 [1]" />*/
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(objStrType);
  std::vector<
      std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(objStrType);
  to_lower(strType_LowerCase);
  to_lower(strProfile);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]egoDistance 5.000000,90.000000,6.330000 [1]   [2]ttc 5.000000,90.000000,6.330000 [1]");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc 5.000000,90.000000,6.330000 [1]'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]3.0, 1");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (2 == split_type_profile.size()) {
        TX_MARK("subProfile = 'ttc 5.000000,90.000000,6.330000'");
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString s = split_type_profile[1];
          std::vector<Base::txString> results_threshold_direction_velocity;
          boost::algorithm::split(results_threshold_direction_velocity, s, boost::is_any_of(","));
          if ((3 == results_threshold_direction_velocity.size())) {
            Utils::trim(results_threshold_direction_velocity[0]);
            Utils::trim(results_threshold_direction_velocity[1]);
            Utils::trim(results_threshold_direction_velocity[2]);
            if ((!results_threshold_direction_velocity[0].empty()) &&
                (!results_threshold_direction_velocity[1].empty()) &&
                (!results_threshold_direction_velocity[2].empty())) {
              retVec.emplace_back(
                  std::make_tuple(std::stod(results_threshold_direction_velocity[0]) TX_MARK("threshold"),
                                  std::stoi(results_threshold_direction_velocity[1]) TX_MARK("direction"),
                                  std::stod(results_threshold_direction_velocity[2]) TX_MARK("velocity"),
                                  Base::ISceneLoader::DistanceProjectionType::Euclidean, FLAGS_DefaultTriggerIndex));
            }
          }
        }
      } else if (3 == split_type_profile.size()) {
        TX_MARK("subProfile = 'ttc 5.000000,90.000000,6.330000 [1]'");
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString s = split_type_profile[1];
          std::vector<Base::txString> results_threshold_direction_velocity;
          boost::algorithm::split(results_threshold_direction_velocity, s, boost::is_any_of(","));
          if ((3 == results_threshold_direction_velocity.size())) {
            Utils::trim(results_threshold_direction_velocity[0]);
            Utils::trim(results_threshold_direction_velocity[1]);
            Utils::trim(results_threshold_direction_velocity[2]);
            if ((!results_threshold_direction_velocity[0].empty()) &&
                (!results_threshold_direction_velocity[1].empty()) &&
                (!results_threshold_direction_velocity[2].empty())) {
              Base::txString strTriggerIndex = split_type_profile[2];
              TX_MARK("[4]");
              Utils::trim(strTriggerIndex);
              to_lower(strTriggerIndex);
              if (strTriggerIndex.size() >= 3) {
                strTriggerIndex = strTriggerIndex.substr(1, strTriggerIndex.size() - 2);
                retVec.emplace_back(
                    std::make_tuple(std::stod(results_threshold_direction_velocity[0]) TX_MARK("threshold"),
                                    std::stoi(results_threshold_direction_velocity[1]) TX_MARK("direction"),
                                    std::stod(results_threshold_direction_velocity[2]) TX_MARK("velocity"),
                                    Base::ISceneLoader::DistanceProjectionType::Euclidean, std::stoi(strTriggerIndex)));
              }
            }
          }
        }
      }
    }
  }
  return retVec;
}

std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_int_float_tuple_parser_with_projectionType(Base::txString strProfile,
                                                 Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  TX_MARK("profile=\"ttc laneprojection 3.0,1,1.0;egodistance laneprojection 6.0,-1,2.0\"");
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(objStrType);
  std::vector<
      std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(objStrType);
  to_lower(strType_LowerCase);
  to_lower(strProfile);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc laneprojection 3.0, 1,2.0   [2]egodistance laneprojection 6.0, -1,1.0");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));

    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc laneprojection 3.0, 1,2.0'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]laneprojection [3]3.0, 1,2.0");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));

      if (3 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString strProjectionType = split_type_profile[1];
          TX_MARK("laneprojection/euclideandistance");
          Utils::trim(strProjectionType);
          to_lower(strProjectionType);
          const Base::ISceneLoader::DistanceProjectionType projType =
              Base::ISceneLoader::IViewer::EvaluateProjectionType(strProjectionType);

          Base::txString s = split_type_profile[2];
          std::vector<Base::txString> results_threshold_direction_velocity;
          boost::algorithm::split(results_threshold_direction_velocity, s, boost::is_any_of(","));
          if ((3 == results_threshold_direction_velocity.size())) {
            Utils::trim(results_threshold_direction_velocity[0]);
            Utils::trim(results_threshold_direction_velocity[1]);
            Utils::trim(results_threshold_direction_velocity[2]);
            if ((!results_threshold_direction_velocity[0].empty()) &&
                (!results_threshold_direction_velocity[1].empty()) &&
                (!results_threshold_direction_velocity[2].empty())) {
              retVec.emplace_back(
                  std::make_tuple(std::stod(results_threshold_direction_velocity[0]) TX_MARK("threshold"),
                                  std::stoi(results_threshold_direction_velocity[1]) TX_MARK("direction"),
                                  std::stod(results_threshold_direction_velocity[2]) TX_MARK("velocity"), projType,
                                  FLAGS_DefaultTriggerIndex));
            }
          }
        }
      }
    }
  }
  return retVec;
}
std::vector<
    std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
float_int_float_tuple_parser_with_projectionType_with_triggerIndex(
    Base::txString strProfile, Base::txString objStrType /* [ttc] [egodistance] */) TX_NOEXCEPT {
  TX_MARK("profile=\"ttc laneprojection 3.0,1,1.0 [4];egodistance laneprojection 6.0,-1,2.0 [1]\"");
  using namespace boost::algorithm;
  to_lower(strProfile);
  to_lower(objStrType);
  std::vector<
      std::tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::ISceneLoader::DistanceProjectionType, Base::txUInt> >
      retVec;
  Base::txString strType_LowerCase(objStrType);
  to_lower(strType_LowerCase);
  to_lower(strProfile);
  Utils::trim(strProfile);
  if (!strProfile.empty()) {
    std::vector<Base::txString> split_subProfile;
    TX_MARK("[1]ttc laneprojection 3.0, 1,2.0 [4]  [2]egodistance laneprojection 6.0, -1,1.0 [1]");
    boost::algorithm::split(split_subProfile, strProfile, boost::is_any_of(";"));
    for (auto subProfile : split_subProfile) {
      TX_MARK("subProfile = 'ttc laneprojection 3.0, 1,2.0 [4]'");
      Utils::trim(subProfile);
      std::vector<Base::txString> split_type_profile;
      TX_MARK("[1]ttc [2]laneprojection [3]3.0, 1,2.0 [4][4]");
      boost::algorithm::split(split_type_profile, subProfile, boost::is_any_of(" "));
      if (4 == split_type_profile.size()) {
        Base::txString strType = split_type_profile[0];
        TX_MARK("ttc");
        Utils::trim(strType);
        to_lower(strType);
        if (strType_LowerCase == strType) {
          Base::txString strProjectionType = split_type_profile[1];
          TX_MARK("laneprojection/euclideandistance");
          Utils::trim(strProjectionType);
          to_lower(strProjectionType);
          const Base::ISceneLoader::DistanceProjectionType projType =
              Base::ISceneLoader::IViewer::EvaluateProjectionType(strProjectionType);

          Base::txString s = split_type_profile[2];
          TX_MARK("3.0,1,2.0");
          std::vector<Base::txString> results_threshold_direction_velocity;
          boost::algorithm::split(results_threshold_direction_velocity, s, boost::is_any_of(","));
          if ((3 == results_threshold_direction_velocity.size())) {
            Utils::trim(results_threshold_direction_velocity[0]);
            Utils::trim(results_threshold_direction_velocity[1]);
            Utils::trim(results_threshold_direction_velocity[2]);
            if ((!results_threshold_direction_velocity[0].empty()) &&
                (!results_threshold_direction_velocity[1].empty()) &&
                (!results_threshold_direction_velocity[2].empty())) {
              Base::txString strTriggerIndex = split_type_profile[3];
              TX_MARK("[4]");
              Utils::trim(strTriggerIndex);
              to_lower(strTriggerIndex);
              if (strTriggerIndex.size() >= 3) {
                strTriggerIndex = strTriggerIndex.substr(1, strTriggerIndex.size() - 2);
                retVec.emplace_back(
                    std::make_tuple(std::stod(results_threshold_direction_velocity[0]) TX_MARK("threshold"),
                                    std::stoi(results_threshold_direction_velocity[1]) TX_MARK("direction"),
                                    std::stod(results_threshold_direction_velocity[2]) TX_MARK("velocity"), projType,
                                    std::stoi(strTriggerIndex)));
              }
            }
          }
        }
      }
    }
  }
  return retVec;
}

#endif /*__TX_Mark__("profile parser func")*/
