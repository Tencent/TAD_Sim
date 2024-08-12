// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils/eval_singleton.h"
#include "utils/eval_types.h"

namespace eval {
// math, unit, shape
class EVector3d;
typedef EVector3d CPosition;
class CEuler;
class CLocation;
class EvalPoints;
class CSimTime;
class CSpeed;
class CAcceleration;
class CJerk;
class CAngularVelocity;

// actor
class CActorBase;
typedef CActorBase *ActorBasePtr;
class CEnvironmentActor;
typedef CEnvironmentActor *CEnvironmentActorPtr;
class CStaticActor;
typedef CStaticActor *CStaticActorPtr;
class CDynamicActor;
typedef CDynamicActor *CDynamicActorPtr;
class CDynamicActorFLU;
typedef CDynamicActorFLU *CDynamicActorFLUPtr;
class CVehicleActor;
typedef CVehicleActor *CVehicleActorPtr;
class CEgoActor;
typedef CEgoActor *CEgoActorPtr;
class CTrafficLightActor;
typedef CTrafficLightActor *CTrafficLightActorPtr;
class CEgoAssemble;
typedef CEgoAssemble *CEgoAssemblePtr;
typedef std::vector<std::shared_ptr<CStaticActor>> CActorMapObjects;

// manager
class ActorManager;
typedef std::shared_ptr<ActorManager> ActorManagerPtr;
using CActorManager = CSingleton<ActorManager>;
class MapManager;
typedef std::shared_ptr<MapManager> MapManagerPtr;
using CMapManager = CSingleton<MapManager>;
class MsgManager;
typedef std::shared_ptr<MsgManager> MsgManagerPtr;
using CMsgManager = CSingleton<MsgManager>;
class EvalManager;
typedef std::shared_ptr<EvalManager> EvalManagerPtr;

// helper
class EvalHelper;
class EvalInit;
class EvalStep;
class EvalStop;

// forward decleration
class EvalBase;
using EvalBasePtr = CSharedPtr<EvalBase>;
using C_KpiName = std::string;
using EvalAlgorithmMap = std::unordered_map<C_KpiName, EvalBasePtr, std::hash<std::string>, std::equal_to<std::string>>;
}  // namespace eval
