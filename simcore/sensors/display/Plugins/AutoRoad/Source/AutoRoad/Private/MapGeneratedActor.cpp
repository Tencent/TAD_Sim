// Fill out your copyright notice in the Description page of Project Settings.

#include "MapGeneratedActor.h"
#include "HadmapManager.h"
#include "assert.h"
// #include "common/coord_trans.h"

#include "ArrowActor.h"
#include "DrawDebugHelpers.h"
#include "PLineActor.h"
#include "RoadObjectActor.h"
#include "Components/LineBatchComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/OpenDriveBaseStruct.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Classes/Components/SplineMeshComponent.h"

#include "routingmap/routing_utils.h"
#include "common/coord_trans.h"

#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "JsonObjectConverter.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "MapModelComponent.h"
// Sets default values

using namespace hadmap;

TMap<hadmap::OBJECT_TYPE, FString> AMapGeneratedActor::ObjectTypeStrMap;
TMap<hadmap::OBJECT_SUB_TYPE, FString> AMapGeneratedActor::ObjectSubTypeStrMap;

FVector GetCrossPoint(const FVector& Ps1, const FVector& Pe1, const FVector& Ps2, const FVector& Pe2)
{
    float A1 = Pe1.Y - Ps1.Y;
    float B1 = Ps1.X - Pe1.X;
    float C1 = A1 * Ps1.X + B1 * Ps1.Y;

    float A2 = Pe2.Y - Ps2.Y;
    float B2 = Ps2.X - Pe2.X;
    float C2 = A2 * Ps2.X + B2 * Ps2.Y;

    float delta = A1 * B2 - A2 * B1;
    if (delta != 0.f)
    {
        return FVector((B2 * C1 - B1 * C2) / delta, (A1 * C2 - A2 * C1) / delta, Pe1.Z);
    }
    else
        return Pe1;
}

FVector GetRightDir(const FVector& P1, const FVector& P2)
{
    FVector Dir = P2 - P1;
    Dir.Normalize();
    if (Dir.IsZero())
    {
        Dir = FVector(1, 0, 0);
    }
    FVector Res = FVector::CrossProduct(Dir, FVector::UpVector);
    if (Res.IsZero())
    {
        Res = FVector(Dir.Y, Dir.X, Dir.Z).GetSafeNormal();
        return Res;
    }
    else
    {
        return Res.GetSafeNormal();
    }
}

void AMapGeneratedActor::ConvertParkRenderData(const FVector& V1, const FVector& V2, const FVector& V3,
    const FVector& V4, TArray<FVector>& Park_Vertices, TArray<int32>& Park_Triangles, TArray<FVector>& Park_Normals,
    TArray<FVector2D>& Park_UV0, float Width, bool bBroken)
{
    TArray<FVector> PointList;
    TArray<FVector> RightList;
    TArray<FVector> LeftList;

    PointList.Add(V1);
    PointList.Add(V2);
    PointList.Add(V3);
    PointList.Add(V4);
    PointList.Add(V1);
    PointList.Add(V2);
    PointList.Add(V3);

    Width *= 0.5f;
    for (int i = 1; i < PointList.Num() - 1; i++)
    {
        FVector RightDir1 = GetRightDir(PointList[i - 1], PointList[i]);
        FVector LeftDir1 = RightDir1 * -1.f;
        FVector RightDir2 = GetRightDir(PointList[i], PointList[i + 1]);
        FVector LeftDir2 = RightDir2 * -1.f;

        FVector RightPoint1_First = PointList[i - 1] + RightDir1 * Width;
        FVector LeftPoint1_First = PointList[i - 1] + LeftDir1 * Width;
        FVector RightPoint1_Back = PointList[i] + RightDir1 * Width;
        FVector LeftPoint1_Back = PointList[i] + LeftDir1 * Width;

        FVector RightPoint2_First = PointList[i] + RightDir2 * Width;
        FVector LeftPoint2_First = PointList[i] + LeftDir2 * Width;
        FVector RightPoint2_Back = PointList[i + 1] + RightDir2 * Width;
        FVector LeftPoint2_Back = PointList[i + 1] + LeftDir2 * Width;

        FVector RightPoint = GetCrossPoint(RightPoint1_First, RightPoint1_Back, RightPoint2_First, RightPoint2_Back);
        FVector LeftPoint = GetCrossPoint(LeftPoint1_First, LeftPoint1_Back, LeftPoint2_First, LeftPoint2_Back);

        RightList.Add(RightPoint);
        LeftList.Add(LeftPoint);
    }

    int32 PointDelta = bBroken ? 2 : 1;

    // 虚线停车位
    if (bBroken)
    {
        const float BrokenInterval = 60.f;
        const float BrokenLineLength = 60.f;
        const float MinHeadLength = 60.f;
        float SpecialBrokenInterval = BrokenInterval;
        TArray<FVector> OutRightList;
        TArray<FVector> OutLeftList;

        for (int32 i = 0; i < LeftList.Num() - 1; i++)
        {
            FVector StartPoint = (LeftList[i] + RightList[i]) * 0.5;
            FVector EndPoint = (LeftList[i + 1] + RightList[i + 1]) * 0.5;
            FVector CurrentPoint = StartPoint;

            FVector ForwardDir = (EndPoint - StartPoint).GetSafeNormal();
            FVector LeftDir = FVector::CrossProduct(ForwardDir, FVector::UpVector) * -1.f;
            float TotalLength = FVector::Distance(StartPoint, EndPoint);

            if (TotalLength <
                (BrokenInterval + MinHeadLength * 2.f))    // 总长度小于虚线间隔加起始两端的最小长度，则用实线显示
            {
                OutLeftList.Add(LeftList[i]);
                OutLeftList.Add(LeftList[i + 1]);
                OutRightList.Add(RightList[i]);
                OutRightList.Add(RightList[i + 1]);
                break;
            }

            int32 BrokenNum = FMath::CeilToInt(
                (TotalLength - (BrokenInterval + MinHeadLength * 2.f)) / (BrokenInterval + BrokenLineLength));
            float HeadLength = 0.f;
            if (1 == BrokenNum)
            {
                HeadLength = MinHeadLength;
                SpecialBrokenInterval =
                    (TotalLength - HeadLength * 2.f);    // 只切一刀时，起始两端长度固定，中间空白区域为剩余长度
            }
            else
            {
                HeadLength = (TotalLength -
                                 (BrokenInterval /*第一段只有空白间隔部分*/ +
                                     (BrokenInterval + BrokenLineLength) * (BrokenNum - 1) /*第二段开始是间隔+实线*/)) *
                             0.5;    // 起始段、结束段长度都是总距离减中间虚线部分的一半
                SpecialBrokenInterval = BrokenInterval;
            }
            float CurrentDistance = 0.f;
            for (int32 j = 0; j <= BrokenNum; j++)
            {
                if (0 == j)    // 起始部分
                {
                    OutLeftList.Add(LeftList[i]);
                    OutLeftList.Add(StartPoint + ForwardDir * HeadLength + LeftDir * Width);

                    OutRightList.Add(RightList[i]);
                    OutRightList.Add(StartPoint + ForwardDir * HeadLength - LeftDir * Width);

                    CurrentPoint =
                        StartPoint +
                        ForwardDir *
                            (HeadLength + SpecialBrokenInterval);    // 第一刀存在特殊情况，如果只有一刀那虚线间隔为变量
                }
                else if ((BrokenNum) == j)    // 结束部分
                {
                    OutLeftList.Add(EndPoint - ForwardDir * HeadLength + LeftDir * Width);
                    OutLeftList.Add(LeftList[i + 1]);

                    OutRightList.Add(EndPoint - ForwardDir * HeadLength - LeftDir * Width);
                    OutRightList.Add(RightList[i + 1]);
                }
                else    // 中间部分
                {
                    OutLeftList.Add(CurrentPoint + LeftDir * Width);
                    OutLeftList.Add(CurrentPoint + ForwardDir * BrokenLineLength + LeftDir * Width);

                    OutRightList.Add(CurrentPoint - LeftDir * Width);
                    OutRightList.Add(CurrentPoint + ForwardDir * BrokenLineLength - LeftDir * Width);

                    CurrentPoint += ForwardDir * (BrokenLineLength + BrokenInterval);
                }
            }
        }

        LeftList = OutLeftList;
        RightList = OutRightList;
    }

    int32 BoundarySize = (RightList.Num() + LeftList.Num()) / 2;
    int32 RecordIndex = Park_Vertices.Num();
    for (int32 i = RecordIndex; i < BoundarySize - 1 + RecordIndex; i = i + PointDelta)
    {
        Park_Triangles.Add(BoundarySize + i);
        Park_Triangles.Add(i + 1);
        Park_Triangles.Add(i);

        Park_Triangles.Add(BoundarySize + i);
        Park_Triangles.Add(BoundarySize + i + 1);
        Park_Triangles.Add(i + 1);

        Park_UV0.Add(FVector2D(0, 0));
        Park_UV0.Add(FVector2D(1, 0));
        Park_UV0.Add(FVector2D(0, 1));
        Park_UV0.Add(FVector2D(1, 1));

        Park_Normals.Add(FVector::UpVector);
        Park_Normals.Add(FVector::UpVector);
        Park_Normals.Add(FVector::UpVector);
        Park_Normals.Add(FVector::UpVector);
    }
    Park_Vertices.Append(RightList);
    Park_Vertices.Append(LeftList);
}

void AMapGeneratedActor::InitTypeStrMap()
{
    ObjectTypeStrMap.Add(OBJECT_TYPE_None, "OBJECT_TYPE_None");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Curb, "OBJECT_TYPE_Curb");
    ObjectTypeStrMap.Add(OBJECT_TYPE_TrafficBarrier, "OBJECT_TYPE_TrafficBarrier");
    ObjectTypeStrMap.Add(OBJECT_TYPE_OverheadCrossing, "OBJECT_TYPE_OverheadCrossing");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Tunnel, "OBJECT_TYPE_Tunnel");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Gantry, "OBJECT_TYPE_Gantry");
    ObjectTypeStrMap.Add(OBJECT_TYPE_TrafficSign, "OBJECT_TYPE_TrafficSign");
    ObjectTypeStrMap.Add(OBJECT_TYPE_VariableMessageSign, "OBJECT_TYPE_VariableMessageSign");
    ObjectTypeStrMap.Add(OBJECT_TYPE_PermanentDelineator, "OBJECT_TYPE_PermanentDelineator");
    ObjectTypeStrMap.Add(OBJECT_TYPE_TollBooth, "OBJECT_TYPE_TollBooth");
    ObjectTypeStrMap.Add(OBJECT_TYPE_EmergencyCallbox, "OBJECT_TYPE_EmergencyCallbox");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Ditch, "OBJECT_TYPE_Ditch");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Pole, "OBJECT_TYPE_Pole");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Building, "OBJECT_TYPE_Building");
    ObjectTypeStrMap.Add(OBJECT_TYPE_RoadSign, "OBJECT_TYPE_RoadSign");
    ObjectTypeStrMap.Add(OBJECT_TYPE_CrossWalk, "OBJECT_TYPE_CrossWalk");
    ObjectTypeStrMap.Add(OBJECT_TYPE_TrafficLights, "OBJECT_TYPE_TrafficLights");
    ObjectTypeStrMap.Add(OBJECT_TYPE_TunnelWall, "OBJECT_TYPE_TunnelWall");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Arrow, "OBJECT_TYPE_Arrow");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Text, "OBJECT_TYPE_Text");
    ObjectTypeStrMap.Add(OBJECT_TYPE_RoadSign_Text, "OBJECT_TYPE_RoadSign_Text");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Stop, "OBJECT_TYPE_Stop");
    ObjectTypeStrMap.Add(OBJECT_TYPE_LaneBoundary, "OBJECT_TYPE_LaneBoundary");
    ObjectTypeStrMap.Add(OBJECT_TYPE_ParkingSpace, "OBJECT_TYPE_ParkingSpace");
    ObjectTypeStrMap.Add(OBJECT_TYPE_DrivingArea, "OBJECT_TYPE_DrivingArea");
    ObjectTypeStrMap.Add(OBJECT_TYPE_FeaturePoint, "OBJECT_TYPE_FeaturePoint");
    ObjectTypeStrMap.Add(OBJECT_TYPE_LineSegment_PL, "OBJECT_TYPE_LineSegment_PL");
    ObjectTypeStrMap.Add(OBJECT_TYPE_TrafficLight, "OBJECT_TYPE_TrafficLight");
    ObjectTypeStrMap.Add(OBJECT_TYPE_BikeRoadArea, "OBJECT_TYPE_BikeRoadArea");
    ObjectTypeStrMap.Add(OBJECT_TYPE_NoStopArea, "OBJECT_TYPE_NoStopArea");
    ObjectTypeStrMap.Add(OBJECT_TYPE_SpeedBump, "OBJECT_TYPE_SpeedBump");
    ObjectTypeStrMap.Add(OBJECT_TYPE_VerticalWall, "OBJECT_TYPE_VerticalWall");
    ObjectTypeStrMap.Add(OBJECT_TYPE_BusStation, "OBJECT_TYPE_BusStation");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Wall, "OBJECT_TYPE_Wall");
    ObjectTypeStrMap.Add(OBJECT_TYPE_StationArea, "OBJECT_TYPE_StationArea");
    ObjectTypeStrMap.Add(OBJECT_TYPE_ChargingPile, "OBJECT_TYPE_ChargingPile");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Surface, "OBJECT_TYPE_Surface");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Obstacle, "OBJECT_TYPE_Obstacle");
    ObjectTypeStrMap.Add(OBJECT_TYPE_Tree, "OBJECT_TYPE_Tree");
    ObjectTypeStrMap.Add(OBJECT_TYPE_RR_Parking, "OBJECT_TYPE_RR_Parking");
    ObjectTypeStrMap.Add(OBJECT_TYPE_V2X_Sensor, "OBJECT_TYPE_V2X_Sensor");
    ObjectTypeStrMap.Add(OBJECT_TYPE_PedestrianBridge, "OBJECT_TYPE_PedestrianBridge");

    ObjectSubTypeStrMap.Add(OtherSubtype, "OtherSubtype");
    ObjectSubTypeStrMap.Add(Tunnel_001, "Tunnel_001");
    ObjectSubTypeStrMap.Add(POLE_VERTICAL, "POLE_VERTICAL");
    ObjectSubTypeStrMap.Add(POLE_CROSS, "POLE_CROSS");
    ObjectSubTypeStrMap.Add(POLE_Cantilever, "POLE_Cantilever");
    ObjectSubTypeStrMap.Add(POLE_Pillar_6m, "POLE_Pillar_6m");
    ObjectSubTypeStrMap.Add(POLE_Pillar_3m, "POLE_Pillar_3m");
    ObjectSubTypeStrMap.Add(Building_001, "Building_001");
    ObjectSubTypeStrMap.Add(RoadSign_Turn_Left_Waiting, "RoadSign_Turn_Left_Waiting");
    ObjectSubTypeStrMap.Add(RoadSign_Crosswalk_Warning_Line, "RoadSign_Crosswalk_Warning_Line");
    ObjectSubTypeStrMap.Add(RoadSign_White_Broken_Line, "RoadSign_White_Broken_Line");
    ObjectSubTypeStrMap.Add(RoadSign_White_Semicircle_Line, "RoadSign_White_Semicircle_Line");
    ObjectSubTypeStrMap.Add(RoadSign_100_120, "RoadSign_100_120");
    ObjectSubTypeStrMap.Add(RoadSign_80_100, "RoadSign_80_100");
    ObjectSubTypeStrMap.Add(RoadSign_Non_Motor_Vehicle, "RoadSign_Non_Motor_Vehicle");
    ObjectSubTypeStrMap.Add(RoadSign_Non_Motor_Vehicle_Line, "RoadSign_Non_Motor_Vehicle_Line");
    ObjectSubTypeStrMap.Add(RoadSign_Disable_Parking, "RoadSign_Disable_Parking");
    ObjectSubTypeStrMap.Add(RoadSign_Lateral_Dec, "RoadSign_Lateral_Dec");
    ObjectSubTypeStrMap.Add(RoadSign_Longitudinal_Dec, "RoadSign_Longitudinal_Dec");
    ObjectSubTypeStrMap.Add(RoadSign_Circular_Center, "RoadSign_Circular_Center");
    ObjectSubTypeStrMap.Add(RoadSign_Rhombus_Center, "RoadSign_Rhombus_Center");
    ObjectSubTypeStrMap.Add(RoadSign_Mesh_Line, "RoadSign_Mesh_Line");
    ObjectSubTypeStrMap.Add(RoadSign_BusOnly, "RoadSign_BusOnly");
    ObjectSubTypeStrMap.Add(RoadSign_SmallCarOnly, "RoadSign_SmallCarOnly");
    ObjectSubTypeStrMap.Add(RoadSign_BigCarOnly, "RoadSign_BigCarOnly");
    ObjectSubTypeStrMap.Add(RoadSign_Turning_Forbidden, "RoadSign_Turning_Forbidden");

    ObjectSubTypeStrMap.Add(RoadSign_Crosswalk_with_Left_and_Right_Side, "RoadSign_Crosswalk_with_Left_and_Right_Side");
    ObjectSubTypeStrMap.Add(RoadSign_Road_Guide_Lane_Line, "RoadSign_Road_Guide_Lane_Line");
    ObjectSubTypeStrMap.Add(RoadSign_Variable_Direction_Lane_Line, "RoadSign_Variable_Direction_Lane_Line");
    ObjectSubTypeStrMap.Add(RoadSign_Intersection_Guide_Line, "RoadSign_Intersection_Guide_Line");
    ObjectSubTypeStrMap.Add(RoadSign_U_Turning_Forbidden, "RoadSign_U_Turning_Forbidden");
    ObjectSubTypeStrMap.Add(RoadSign_Non_Motor_Vehicle_Area, "RoadSign_Non_Motor_Vehicle_Area");

    ObjectSubTypeStrMap.Add(LIGHT_VERTICAL_ALLDIRECT, "LIGHT_VERTICAL_ALLDIRECT");
    ObjectSubTypeStrMap.Add(LIGHT_VERTICAL_STRAIGHTROUND, "LIGHT_VERTICAL_STRAIGHTROUND");
    ObjectSubTypeStrMap.Add(LIGHT_VERTICAL_LEFTTURN, "LIGHT_VERTICAL_LEFTTURN");
    ObjectSubTypeStrMap.Add(LIGHT_VERTICAL_STRAIGHT, "LIGHT_VERTICAL_STRAIGHT");
    ObjectSubTypeStrMap.Add(LIGHT_VERTICAL_RIGHTTURN, "LIGHT_VERTICAL_RIGHTTURN");
    ObjectSubTypeStrMap.Add(LIGHT_VERTICAL_UTURN, "LIGHT_VERTICAL_UTURN");
    ObjectSubTypeStrMap.Add(LIGHT_HORIZON_PEDESTRIAN, "LIGHT_HORIZON_PEDESTRIAN");
    ObjectSubTypeStrMap.Add(LIGHT_HORIZON_ALLDIRECT, "LIGHT_HORIZON_ALLDIRECT");
    ObjectSubTypeStrMap.Add(LIGHT_HORIZON_STRAIGHTROUND, "LIGHT_HORIZON_STRAIGHTROUND");
    ObjectSubTypeStrMap.Add(LIGHT_HORIZON_LEFTTURN, "LIGHT_HORIZON_LEFTTURN");
    ObjectSubTypeStrMap.Add(LIGHT_HORIZON_STRAIGHT, "LIGHT_HORIZON_STRAIGHT");
    ObjectSubTypeStrMap.Add(LIGHT_HORIZON_RIGHTTURN, "LIGHT_HORIZON_RIGHTTURN");
    ObjectSubTypeStrMap.Add(LIGHT_HORIZON_UTURN, "LIGHT_HORIZON_UTURN");
    ObjectSubTypeStrMap.Add(LIGHT_BICYCLELIGHT, "LIGHT_BICYCLELIGHT");
    ObjectSubTypeStrMap.Add(LIGHT_TWOCOLOR, "LIGHT_TWOCOLOR");
    ObjectSubTypeStrMap.Add(Arrow_Straight, "Arrow_Straight");
    ObjectSubTypeStrMap.Add(Arrow_StraightLeft, "Arrow_StraightLeft");
    ObjectSubTypeStrMap.Add(Arrow_Left, "Arrow_Left");
    ObjectSubTypeStrMap.Add(Arrow_Right, "Arrow_Right");
    ObjectSubTypeStrMap.Add(Arrow_StraightRight, "Arrow_StraightRight");
    ObjectSubTypeStrMap.Add(Arrow_Uturn, "Arrow_Uturn");
    ObjectSubTypeStrMap.Add(Arrow_StraightUturn, "Arrow_StraightUturn");
    ObjectSubTypeStrMap.Add(Arrow_LeftUturn, "Arrow_LeftUturn");
    ObjectSubTypeStrMap.Add(Arrow_LeftRight, "Arrow_LeftRight");
    ObjectSubTypeStrMap.Add(Arrow_TurnStraight, "Arrow_TurnStraight");
    ObjectSubTypeStrMap.Add(Arrow_SkewLeft, "Arrow_SkewLeft");
    ObjectSubTypeStrMap.Add(Arrow_SkewRight, "Arrow_SkewRight");
    ObjectSubTypeStrMap.Add(Stop_Line, "Stop_Line");
    ObjectSubTypeStrMap.Add(Stop_Line_GiveWay, "Stop_Line_GiveWay");
    ObjectSubTypeStrMap.Add(Stop_Slow_Line_GiveWay, "Stop_Slow_Line_GiveWay");
    ObjectSubTypeStrMap.Add(SpeedBump_001, "SpeedBump_001");
    ObjectSubTypeStrMap.Add(BusStation_001, "BusStation_001");
    ObjectSubTypeStrMap.Add(Obstacle_Traffic_Barrier, "Obstacle_Traffic_Barrier");
    ObjectSubTypeStrMap.Add(Obstacle_Road_Curb, "Obstacle_Road_Curb");
    ObjectSubTypeStrMap.Add(Obstacle_Lamp, "Obstacle_Lamp");
    ObjectSubTypeStrMap.Add(Obstacle_Traffic_Cone, "Obstacle_Traffic_Cone");
    ObjectSubTypeStrMap.Add(Obstacle_Traffic_Horse, "Obstacle_Traffic_Horse");
    ObjectSubTypeStrMap.Add(Obstacle_GarbageCan, "Obstacle_GarbageCan");
    ObjectSubTypeStrMap.Add(Obstacle_Obstacle, "Obstacle_Obstacle");
    ObjectSubTypeStrMap.Add(ChargingPile_001, "ChargingPile_001");
    ObjectSubTypeStrMap.Add(Surface_Pothole, "Surface_Pothole");
    ObjectSubTypeStrMap.Add(Surface_Patch, "Surface_Patch");
    ObjectSubTypeStrMap.Add(Surface_Crack, "Surface_Crack");
    ObjectSubTypeStrMap.Add(Surface_Asphalt_Line, "Surface_Asphalt_Line");
    ObjectSubTypeStrMap.Add(Surface_Rut_Track, "Surface_Rut_Track");
    ObjectSubTypeStrMap.Add(Surface_Stagnant_Water, "Surface_Stagnant_Water");
    ObjectSubTypeStrMap.Add(Surface_Protrusion, "Surface_Protrusion");
    ObjectSubTypeStrMap.Add(Surface_Well_Cover, "Surface_Well_Cover");
    ObjectSubTypeStrMap.Add(Obstacle_Reflective_Road_Sign, "Obstacle_Reflective_Road_Sign");
    ObjectSubTypeStrMap.Add(Obstacle_Parking_Hole, "Obstacle_Parking_Hole");
    ObjectSubTypeStrMap.Add(Obstacle_Parking_Lot, "Obstacle_Parking_Lot");
    ObjectSubTypeStrMap.Add(Obstacle_Ground_Lock, "Obstacle_Ground_Lock");
    ObjectSubTypeStrMap.Add(Obstacle_Plastic_Vehicle_Stopper, "Obstacle_Plastic_Vehicle_Stopper");
    ObjectSubTypeStrMap.Add(Obstacle_Parking_Limit_Position_Pole_2m, "Obstacle_Parking_Limit_Position_Pole_2m");
    ObjectSubTypeStrMap.Add(Obstacle_Support_Vehicle_Stopper, "Obstacle_Support_Vehicle_Stopper");
    ObjectSubTypeStrMap.Add(Tree_001, "Tree_001");
    ObjectSubTypeStrMap.Add(Shrub_001, "Shrub_001");
    ObjectSubTypeStrMap.Add(Grass_001, "Grass_001");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_SLOWDOWN, "SIGN_WARNING_SLOWDOWN");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_TURNLEFT, "SIGN_WARNING_TURNLEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_TURNRIGHT, "SIGN_WARNING_TURNRIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_T_RIGHT, "SIGN_WARNING_T_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_T_DOWN, "SIGN_WARNING_T_DOWN");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_CROSS, "SIGN_WARNING_CROSS");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_CHILD, "SIGN_WARNING_CHILD");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_UP, "SIGN_WARNING_UP");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_BUILDING, "SIGN_WARNING_BUILDING");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_NARROW_LEFT, "SIGN_WARNING_NARROW_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_NARROW_BOTH, "SIGN_WARNING_NARROW_BOTH");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_RAILWAY, "SIGN_WARNING_RAILWAY");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_T_LEFT, "SIGN_WARNING_T_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_STEEP_LEFT, "SIGN_WARNING_STEEP_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_STEEP_RIGHT, "SIGN_WARNING_STEEP_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_VILLAGE, "SIGN_WARNING_VILLAGE");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_DIKE_LEFT, "SIGN_WARNING_DIKE_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_DIKE_RIGHT, "SIGN_WARNING_DIKE_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARAING_T_CROSSED, "SIGN_WARAING_T_CROSSED");
    ObjectSubTypeStrMap.Add(SIGN_WARAING_FERRY, "SIGN_WARAING_FERRY");
    ObjectSubTypeStrMap.Add(SIGN_WARAING_FALL_ROCK, "SIGN_WARAING_FALL_ROCK");
    ObjectSubTypeStrMap.Add(SIGN_WARAING_REVERSE_CURVE_LEFT, "SIGN_WARAING_REVERSE_CURVE_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARAING_REVERSE_CURVE_RIGHT, "SIGN_WARAING_REVERSE_CURVE_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARAING_WATER_PAVEMENT, "SIGN_WARAING_WATER_PAVEMENT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_T_BOTH, "SIGN_WARNING_T_BOTH");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_JOIN_LEFT, "SIGN_WARNING_JOIN_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_JOIN_RIGHT, "SIGN_WARNING_JOIN_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_Y_LEFT, "SIGN_WARNING_Y_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_CIRCLE_CROSS, "SIGN_WARNING_CIRCLE_CROSS");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_Y_RIGHT, "SIGN_WARNING_Y_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_CURVE_AHEAD, "SIGN_WARNING_CURVE_AHEAD");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_LONG_DESCENT, "SIGN_WARNING_LONG_DESCENT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_ROUGH_ROAD, "SIGN_WARNING_ROUGH_ROAD");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_SNOW, "SIGN_WARNING_SNOW");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_DISABLE, "SIGN_WARNING_DISABLE");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_ANIMALS, "SIGN_WARNING_ANIMALS");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_ACCIDENT, "SIGN_WARNING_ACCIDENT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_TIDALBU_LANE, "SIGN_WARNING_TIDALBU_LANE");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_BAD_WEATHER, "SIGN_WARNING_BAD_WEATHER");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_LOWLYING, "SIGN_WARNING_LOWLYING");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_HIGHLYING, "SIGN_WARNING_HIGHLYING");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_DOWNHILL, "SIGN_WARNING_DOWNHILL");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_QUEUESLIKELY, "SIGN_WARNING_QUEUESLIKELY");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_CROSS_PLANE, "SIGN_WARNING_CROSS_PLANE");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_TUNNEL, "SIGN_WARNING_TUNNEL");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_TUNNEL_LIGHT, "SIGN_WARNING_TUNNEL_LIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_HUMPBACK_BRIDGE, "SIGN_WARNING_HUMPBACK_BRIDGE");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_NARROW_RIGHT, "SIGN_WARNING_NARROW_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_NON_MOTOR, "SIGN_WARNING_NON_MOTOR");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_SLIPPERY, "SIGN_WARNING_SLIPPERY");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_TRIFFICLIGHT, "SIGN_WARNING_TRIFFICLIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_DETOUR_RIGHT, "SIGN_WARNING_DETOUR_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_NARROW_BRIDGE, "SIGN_WARNING_NARROW_BRIDGE");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_KEEP_DISTANCE, "SIGN_WARNING_KEEP_DISTANCE");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_MERGE_LEFT, "SIGN_WARNING_MERGE_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_MERGE_RIGHT, "SIGN_WARNING_MERGE_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_CROSSWIND, "SIGN_WARNING_CROSSWIND");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_ICY_ROAD, "SIGN_WARNING_ICY_ROAD");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_ROCKFALL, "SIGN_WARNING_ROCKFALL");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_CAUTION, "SIGN_WARNING_CAUTION");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_FOGGY, "SIGN_WARNING_FOGGY");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_LIVESTOCK, "SIGN_WARNING_LIVESTOCK");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_DETOUR_LEFT, "SIGN_WARNING_DETOUR_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_DETOUR_BOTH, "SIGN_WARNING_DETOUR_BOTH");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_BOTHWAY, "SIGN_WARNING_BOTHWAY");
    ObjectSubTypeStrMap.Add(SIGN_BAN_STRAIGHT, "SIGN_BAN_STRAIGHT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_VEHICLE, "SIGN_BAN_VEHICLE");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_120, "SIGN_BAN_SPPED_120");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_100, "SIGN_BAN_SPPED_100");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_80, "SIGN_BAN_SPPED_80");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_70, "SIGN_BAN_SPPED_70");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_60, "SIGN_BAN_SPPED_60");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_50, "SIGN_BAN_SPPED_50");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_40, "SIGN_BAN_SPPED_40");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_30, "SIGN_BAN_SPPED_30");
    ObjectSubTypeStrMap.Add(SIGN_BAN_STOP_YIELD, "SIGN_BAN_STOP_YIELD");
    ObjectSubTypeStrMap.Add(SIGN_BAN_HEIGHT_5, "SIGN_BAN_HEIGHT_5");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_20, "SIGN_BAN_SPPED_20");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SPPED_05, "SIGN_BAN_SPPED_05");
    ObjectSubTypeStrMap.Add(SIGN_BAN_DIVERINTO, "SIGN_BAN_DIVERINTO");
    ObjectSubTypeStrMap.Add(SIGN_BAN_MOTOR_BIKE, "SIGN_BAN_MOTOR_BIKE");
    ObjectSubTypeStrMap.Add(SIGN_BAN_WEIGHT_50, "SIGN_BAN_WEIGHT_50");
    ObjectSubTypeStrMap.Add(SIGN_BAN_WEIGHT_20, "SIGN_BAN_WEIGHT_20");
    ObjectSubTypeStrMap.Add(SIGN_BAN_HONKING, "SIGN_BAN_HONKING");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRUCK, "SIGN_BAN_TRUCK");
    ObjectSubTypeStrMap.Add(SIGN_BAN_WEIGHT_30, "SIGN_BAN_WEIGHT_30");
    ObjectSubTypeStrMap.Add(SIGN_BAN_WEIGHT_10, "SIGN_BAN_WEIGHT_10");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TEMP_PARKING, "SIGN_BAN_TEMP_PARKING");
    ObjectSubTypeStrMap.Add(SIGN_BAN_AXLE_WEIGHT_14, "SIGN_BAN_AXLE_WEIGHT_14");
    ObjectSubTypeStrMap.Add(SIGN_BAN_AXLE_WEIGHT_13, "SIGN_BAN_AXLE_WEIGHT_13");
    ObjectSubTypeStrMap.Add(SIGN_BAN_WEIGHT_40, "SIGN_BAN_WEIGHT_40");
    ObjectSubTypeStrMap.Add(SIGN_BAN_SLOW, "SIGN_BAN_SLOW");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TURN_LEFT, "SIGN_BAN_TURN_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_DANGEROUS_GOODS, "SIGN_BAN_DANGEROUS_GOODS");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRACTORS, "SIGN_BAN_TRACTORS");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRICYCLE, "SIGN_BAN_TRICYCLE");
    ObjectSubTypeStrMap.Add(SIGN_BAN_MINIBUS, "SIGN_BAN_MINIBUS");
    ObjectSubTypeStrMap.Add(SIGN_BAN_STRAIGHT_AND_LEFT, "SIGN_BAN_STRAIGHT_AND_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_VEHICLE_BY_HUMAN, "SIGN_BAN_VEHICLE_BY_HUMAN");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRACYCLE01_BY_HUMAN, "SIGN_BAN_TRACYCLE01_BY_HUMAN");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRACYCLE02_BY_HUMAN, "SIGN_BAN_TRACYCLE02_BY_HUMAN");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TURN_RIGHT, "SIGN_BAN_TURN_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_LEFT_AND_RIGHT, "SIGN_BAN_LEFT_AND_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_STRAIGHT_AND_RIGHT, "SIGN_BAN_STRAIGHT_AND_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_GO, "SIGN_BAN_GO");
    ObjectSubTypeStrMap.Add(SIGN_BAN_GIVE_WAY, "SIGN_BAN_GIVE_WAY");
    ObjectSubTypeStrMap.Add(SIGN_BAN_BUS_TURN_RIGHT, "SIGN_BAN_BUS_TURN_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRUCK_TURN_RIGHT, "SIGN_BAN_TRUCK_TURN_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_BYCICLE_DOWN, "SIGN_BAN_BYCICLE_DOWN");
    ObjectSubTypeStrMap.Add(SIGN_BAN_BYCICLE_UP, "SIGN_BAN_BYCICLE_UP");
    ObjectSubTypeStrMap.Add(SIGN_BAN_NO_OVERTAKING, "SIGN_BAN_NO_OVERTAKING");
    ObjectSubTypeStrMap.Add(SIGN_BAN_BUS_TURN_LEFT, "SIGN_BAN_BUS_TURN_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_OVERTAKING, "SIGN_BAN_OVERTAKING");
    ObjectSubTypeStrMap.Add(SIGN_BAN_ANIMALS, "SIGN_BAN_ANIMALS");
    ObjectSubTypeStrMap.Add(SIGN_BAN_BUS, "SIGN_BAN_BUS");
    ObjectSubTypeStrMap.Add(SIGN_BAN_ELECTRO_TRICYCLE, "SIGN_BAN_ELECTRO_TRICYCLE");
    ObjectSubTypeStrMap.Add(SIGN_BAN_NO_MOTOR, "SIGN_BAN_NO_MOTOR");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRUCK_TURN_LEFT, "SIGN_BAN_TRUCK_TURN_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_BAN_TRAILER, "SIGN_BAN_TRAILER");
    ObjectSubTypeStrMap.Add(SIGN_BAN_HUMAN, "SIGN_BAN_HUMAN");
    ObjectSubTypeStrMap.Add(SIGN_BAN_THE_TWO_TYPES, "SIGN_BAN_THE_TWO_TYPES");
    ObjectSubTypeStrMap.Add(SIGN_BAN_HEIGHT_3_5, "SIGN_BAN_HEIGHT_3_5");
    ObjectSubTypeStrMap.Add(SIGN_BAN_HEIGHT_3, "SIGN_BAN_HEIGHT_3");
    ObjectSubTypeStrMap.Add(SIGN_BAN_AXLE_WEIGHT_10, "SIGN_BAN_AXLE_WEIGHT_10");
    ObjectSubTypeStrMap.Add(SIGN_BAN_CUSTOMS_MARK, "SIGN_BAN_CUSTOMS_MARK");
    ObjectSubTypeStrMap.Add(SIGN_BAN_STOP, "SIGN_BAN_STOP");
    ObjectSubTypeStrMap.Add(SIGN_BAN_LONG_PARKING, "SIGN_BAN_LONG_PARKING");
    ObjectSubTypeStrMap.Add(SIGN_BAN_REMOVE_LIMIT_40, "SIGN_BAN_REMOVE_LIMIT_40");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_STRAIGHT, "SIGN_INDOCATION_STRAIGHT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_LOWEST_SPEED_60, "SIGN_INDOCATION_LOWEST_SPEED_60");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_LOWEST_SPEED_40, "SIGN_INDOCATION_LOWEST_SPEED_40");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_ALONG_RIGHT, "SIGN_INDOCATION_ALONG_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_PEDESTRIAN_CROSSING, "SIGN_INDOCATION_PEDESTRIAN_CROSSING");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_TURN_RIGHT, "SIGN_INDOCATION_TURN_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_ROUNDABOUT, "SIGN_INDOCATION_ROUNDABOUT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_TURN_LEFT, "SIGN_INDOCATION_TURN_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_STRAIGHT_RIGHT, "SIGN_INDOCATION_STRAIGHT_RIGHT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_STRAIGHT_LEFT, "SIGN_INDOCATION_STRAIGHT_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_LOWEST_SPEED_50, "SIGN_INDOCATION_LOWEST_SPEED_50");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_WALK, "SIGN_INDOCATION_WALK");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_NO_MOTOR, "SIGN_INDOCATION_NO_MOTOR");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_MOTOR, "SIGN_INDOCATION_MOTOR");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_ALONG_LEFT, "SIGN_INDOCATION_ALONG_LEFT");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_PASS_STAIGHT_001, "SIGN_INDOCATION_PASS_STAIGHT_001");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_PASS_STAIGHT_002, "SIGN_INDOCATION_PASS_STAIGHT_002");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_WHISTLE, "SIGN_INDOCATION_WHISTLE");
    ObjectSubTypeStrMap.Add(SIGN_BAN_UTURN, "SIGN_BAN_UTURN");
    ObjectSubTypeStrMap.Add(SIGN_WARNING_PED, "SIGN_WARNING_PED");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_PARKING, "SIGN_INDOCATION_PARKING");
    ObjectSubTypeStrMap.Add(SIGN_INDOCATION_LEFT_AND_RIGHT, "SIGN_INDOCATION_LEFT_AND_RIGHT");
    ObjectSubTypeStrMap.Add(PedestrianBridge, "PedestrianBridge");
    ObjectSubTypeStrMap.Add(Sensors_Camera, "Sensors_Camera");
    ObjectSubTypeStrMap.Add(Sensors_Radar, "Sensors_Radar");
    ObjectSubTypeStrMap.Add(Sensors_RSU, "Sensors_RSU");
    ObjectSubTypeStrMap.Add(Sensors_Lidar, "Sensors_Lidar");
}

AMapGeneratedActor::AMapGeneratedActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // RootComponent
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    MapModelComponent = CreateDefaultSubobject<UMapModelComponent>(TEXT("MapModelComp"));

    // DataTable
    static ConstructorHelpers::FObjectFinder<UDataTable> conHelpers(
        TEXT("/AutoRoad/Art/Config/RoadObjectMaterialConfig.RoadObjectMaterialConfig"));
    RoadObjectMaterialConfig = conHelpers.Object;

    static ConstructorHelpers::FObjectFinder<UDataTable> conHelperLinearColor(
        TEXT("/AutoRoad/Art/Config/RoadMarkColorConfig.RoadMarkColorConfig"));
    RoadMarkColorConfig = conHelperLinearColor.Object;

    AMapGeneratedActor::InitTypeStrMap();
}

// Called when the game starts or when spawned
void AMapGeneratedActor::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void AMapGeneratedActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMapGeneratedActor::DrawMap(FString pathIput, FString DecryptFilePath, bool bArtLevel,
    const TMap<FString, TPair<FString, FVector>>& ModelData, const FString& ModelRootPath)
{
    // Tags.Add(TEXT("EnvOcclusion"));
    UE_LOG(LogTemp, Log, TEXT("DrawMap %s"), *pathIput);
    if (!IFileManager::Get().FileExists(*pathIput))
    {
        UE_LOG(LogTemp, Error, TEXT("FilePath Error"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AMapGeneratedActor DrawMap % s"), *pathIput);

    if (MapModelComponent)
    {
        MapModelComponent->Init(ModelData, ModelRootPath);
    }

    // Init Map
    SHadmap->Init(hadmapue4::MAPENGINE, pathIput, RefX, RefY, RefZ, DecryptFilePath);

    txOdHeaderPtr header = nullptr;
    hadmap::getHeader(SHadmap->GetMapHandle(), header);
    bOldVersion = false;
    if (header != NULL)
    {
        if (header->getVendor() == "tadsim")
        {
            bOldVersion = true;
            UE_LOG(LogTemp, Log, TEXT("AMapGeneratedActor DrawMap old version map"));
        }
    }

    hadmap::MAP_DATA_TYPE MapType = SHadmap->GetMapDataType(pathIput);

    if (bArtLevel)
        bShowObject = ShowObjectOnArtLevel;
    else
        bShowObject = ShowObjectOnAutoRoad;

    UE_LOG(LogTemp, Log, TEXT("ShowObject : %d"), (int32) bShowObject);

    FString GenearateRoadOnXYStr;
    if (FParse::Value(FCommandLine::Get(), TEXT("bGenearateRoadOnXY="), GenearateRoadOnXYStr))
    {
        if (GenearateRoadOnXYStr.Equals(TEXT("true"), ESearchCase::IgnoreCase))
        {
            bGenearateRoadOnXY = true;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("bGenearateRoadOnXY :%d"), bGenearateRoadOnXY);

    if (bGenearateRoadOnXY)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
        for (auto Elem : FoundActors)
        {
            if (Elem->GetName().Contains(TEXT("AutoRoadPlane")))
            {
                AStaticMeshActor* RoadPlane = Cast<AStaticMeshActor>(Elem);
                if (RoadPlane)
                {
                    RoadPlane->GetStaticMeshComponent()->SetMaterial(0, CreateStaticMaterial(MatGround));
                }
            }
        }
    }

    // Generate Road
    GenerateRoad(SHadmap->GetMapHandle(), TCHAR_TO_UTF8(*pathIput), RefX, RefY, RefZ);

    if (!ShowObjectOnly)
    {
        CreateSpline(SHadmap->GetMapHandle(), RefX, RefY, RefZ);
    }
    RegisterAllComponents();
}

static const double LANE_WIDTH = 0.075;
static const double LANE_WIDTH2 = 0.2;
static const double STOP_WIDTH = 0.1;
static const double LANE_HEIGHT_OFFSET = 0.5;
static const double DOT_LEN = 6.0;
static const double SPACE_LEN = 9.0;
static const double DOT_LEN2 = 4.0;
static const double SPACE_LEN2 = 6.0;
static const double DOT_LEN3 = 1.0;
static const double SPACE_LEN3 = 1.0;

bool createOffsetGeom(const PointVec& originGeom, double startOffset, double endOffset, PointVec& geom)
{
    geom.clear();
    if (originGeom.size() <= 1)
        return false;
    else
    {
        double offset = (endOffset - startOffset) / (originGeom.size() - 1);
        for (size_t i = 0; i < originGeom.size(); ++i)
        {
            hadmap::txPoint ver;
            if (i == 0)
            {
                hadmap::txPoint toP(originGeom[i + 1]);
                coord_trans_api::lonlat2local(toP.x, toP.y, toP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
                ver.x = toP.y;
                ver.y = -toP.x;
                ver.z = 0.0;
                double len = RoutingUtils::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
                ver.x /= len;
                ver.y /= len;
            }
            else if (i == originGeom.size() - 1)
            {
                hadmap::txPoint fromP(originGeom[i - 1]);
                coord_trans_api::lonlat2local(
                    fromP.x, fromP.y, fromP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
                ver.x = -fromP.y;
                ver.y = fromP.x;
                ver.z = 0.0;
                double len = RoutingUtils::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
                ver.x /= len;
                ver.y /= len;
            }
            else
            {
                hadmap::txPoint fromP(originGeom[i - 1]);
                hadmap::txPoint toP(originGeom[i + 1]);
                coord_trans_api::lonlat2local(
                    fromP.x, fromP.y, fromP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
                coord_trans_api::lonlat2local(toP.x, toP.y, toP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
                double fromLen = RoutingUtils::pointsDistance(fromP, hadmap::txPoint(0.0, 0.0, 0.0));
                double toLen = RoutingUtils::pointsDistance(toP, hadmap::txPoint(0.0, 0.0, 0.0));
                fromP.x /= -fromLen;
                fromP.y /= -fromLen;
                fromP.z /= -fromLen;
                toP.x /= toLen;
                toP.y /= toLen;
                toP.z /= toLen;
                ver.x = fromP.y + toP.y;
                ver.y = -(fromP.x + toP.x);
                ver.z = 0.0;
                double len = RoutingUtils::pointsDistance(ver, hadmap::txPoint(0.0, 0.0, 0.0));
                ver.x /= len;
                ver.y /= len;
                ver.z /= len;
            }
            hadmap::txPoint tagP;
            double curOffset = startOffset + i * offset;
            tagP.x = ver.x * curOffset;
            tagP.y = ver.y * curOffset;
            tagP.z = ver.z * curOffset;
            coord_trans_api::local2lonlat(tagP.x, tagP.y, tagP.z, originGeom[i].x, originGeom[i].y, originGeom[i].z);
            geom.push_back(tagP);
        }
        if (geom.size() == 2)
            return true;
        for (size_t i = 0; i < geom.size(); ++i)
            coord_trans_api::lonlat2global(geom[i].x, geom[i].y, geom[i].z);
        for (auto frontItr = geom.begin(), curItr = geom.begin() + 1; curItr != geom.end();)
        {
            auto nextItr = curItr + 1;
            if (nextItr == geom.end())
                break;
            hadmap::txPoint frontDir(curItr->x - frontItr->x, curItr->y - frontItr->y, curItr->z - frontItr->z);
            hadmap::txPoint nextDir(nextItr->x - curItr->x, nextItr->y - curItr->y, nextItr->z - curItr->z);
            double frontLen = RoutingUtils::pointsDistance(frontDir, hadmap::txPoint(0.0, 0.0, 0.0));
            double nextLen = RoutingUtils::pointsDistance(nextDir, hadmap::txPoint(0.0, 0.0, 0.0));
            double cosv =
                (frontDir.x * nextDir.x + frontDir.y * nextDir.y + frontDir.z * nextDir.z) / (frontLen * nextLen);
            if (fabs(cosv - 1.0) < 1e-5)
                cosv = 1.0;
            double arccos = acos(cosv);
            if (1 || arccos < 3.1415926 / 2)
            {
                frontItr++;
                curItr++;
            }
            else
            {
                curItr = geom.erase(curItr);
            }
        }
        for (size_t i = 0; i < geom.size(); ++i)
            coord_trans_api::global2lonlat(geom[i].x, geom[i].y, geom[i].z);
        return true;
    }
}

void AMapGeneratedActor::DrawRoadDetails(TArray<LaneBoundaryInfo>& lane_bdids)
{
    // road lane
    hadmap::txRoads _roads;
    if (!ShowObjectOnly)
    {
        hadmap::getRoads(Handle, true, _roads);
    }

    int32 RenderLaneNum = 0;
    TArray<FVector> LaneVertices;
    TArray<int32> LaneTriangles;
    TArray<FVector> LaneNormals;
    TArray<FVector2D> LaneUV0s;
    FName RecordLane;
    int32 RecordTriangles = 0;
    const FName mat_lane = "";
    std::map<hadmap::laneboundarypkid, bool> rdHighway;
    double esp = 0;    // uv缩放值

    for (auto& road_ptr : _roads)
    {
        for (auto section_ptr : road_ptr->getSections())
        {
            for (auto lane_ptr : section_ptr->getLanes())
            {
                if (reverse &&
                    (lane_ptr == section_ptr->getLanes().front() || lane_ptr == section_ptr->getLanes().back()))
                {
                    continue;
                }

                rdHighway[lane_ptr->getLeftBoundaryId()] = road_ptr->getFuncClass() < 2;
                rdHighway[lane_ptr->getRightBoundaryId()] = road_ptr->getFuncClass() < 2;
            }

            TArray<TPair<hadmap::txLaneBoundaryPtr, hadmap::txLanePtr>> bds;

            txLaneBoundaryPtr RoadBoundary1 = nullptr;
            txLaneBoundaryPtr RoadBoundary2 = nullptr;

            for (auto lane_ptr : section_ptr->getLanes())
            {
                if (lane_ptr == section_ptr->getLanes().front())
                {
                    RoadBoundary1 = lane_ptr->getLeftBoundary();
                }
                else if (lane_ptr == section_ptr->getLanes().back())
                {
                    RoadBoundary2 = lane_ptr->getRightBoundary();
                }

                bds.Add(TPair<hadmap::txLaneBoundaryPtr, hadmap::txLanePtr>(lane_ptr->getLeftBoundary(), lane_ptr));
                bds.Add(TPair<hadmap::txLaneBoundaryPtr, hadmap::txLanePtr>(lane_ptr->getRightBoundary(), lane_ptr));
            }

            if (bds.Num() == 0)
            {
                continue;
            }
            SceneFaceData roaddata;
            roaddata.lane_num = bds.Num();

            SceneFaceData curbdata;
            curbdata.lane_num = 2;
            curbdata.veritics.resize(1);

            size_t n0 = 0, i0 = 0;

            for (auto bd : bds)
            {
                hadmap::txLaneBoundaryPtr lane = bd.Key;
                hadmap::PointVec points;
                std::vector<FVector> veritics;

                const hadmap::txLineCurve* LineCurve = static_cast<const hadmap::txLineCurve*>(lane->getGeometry());
                if (!LineCurve)
                    continue;
                if (LineCurve->empty())
                    continue;

                LineCurve->getPoints(points);
                for (int32 i = 0, is = points.size(); i < is; i++)
                {
                    auto& p = points[i];
                    LonLatToLocal(p.x, p.y, p.z);
                }

                PointDilution(points);

                LaneBoundaryInfo BoundaryInfo;
                BoundaryInfo.BoundaryPtr = lane;
                // BoundaryInfo.LanePtr = bd.Value;
                BoundaryInfo.Points = points;

                auto section_ptr = bd.Value->getSection();
                auto road_ptr = section_ptr->getRoad();
                BoundaryInfo.RoadId = road_ptr->getId();
                BoundaryInfo.SectionId = section_ptr->getId();
                lane_bdids.Add(BoundaryInfo);

                // section start point
                hadmap::txPoint pos_start = static_cast<const hadmap::txLineCurve*>(lane->getGeometry())->getStart();
                // coord_trans_api::lonlat2local(pos_start.x, pos_start.y, pos_start.z, x_reference, y_reference,
                // z_reference);
                LonLatToLocal(pos_start.x, pos_start.y, pos_start.z);
                // FVector V_S = FVector(pos_start.x * 100, -pos_start.y * 100, pos_start.z * 100);
                FVector V_S = FVector(pos_start.x, pos_start.y, pos_start.z);

                // section end point
                hadmap::txPoint pos_end = static_cast<const hadmap::txLineCurve*>(lane->getGeometry())->getEnd();
                // coord_trans_api::lonlat2local(pos_end.x, pos_end.y, pos_end.z, x_reference, y_reference,
                // z_reference);
                LonLatToLocal(pos_end.x, pos_end.y, pos_end.z);
                // FVector V_E = FVector(pos_end.x * 100, -pos_end.y * 100, pos_end.z * 100);
                FVector V_E = FVector(pos_end.x, pos_end.y, pos_end.z);

                if (esp == 0)
                {
                    double dis_str_end = FVector::Distance(V_S, V_E);
                    // section length
                    double ds = LineCurve->getLength();
                    esp = 0.01;    // ds / dis_str_end;
                    // UE_LOG(LogTemp, Log, TEXT("esp  = %f"), ds / dis_str_end);
                }

                for (size_t i = 0, is = points.size(); i < is; i++)
                {
                    auto& p = points[i];

                    FVector local = FVector(p.x, p.y, p.z);
                    if (local.ContainsNaN())
                        continue;

                    // current point to start distance
                    double dis_str_cur = FVector::Distance(V_S, local);
                    double dsp = dis_str_cur * esp;

                    size_t edge_index = bds.Num() > 2 ? bds.Num() - 2 : bds.Num() / 2;
                    if (bd == bds.Last())
                    {
                        // 马路牙子数据
                        FVector normal_l, normal_r, p_delta, p0, p1;
                        if (roaddata.veritics.back().size() > i)
                        {
                            p0 = roaddata.veritics.back()[i];
                            normal_l = (p0 - local).GetSafeNormal2D();
                            normal_r = FVector(-normal_l.X, -normal_l.Y, 0);    //(local - p0).GetSafeNormal2D();
                        }
                        else
                        {
                            p0 = veritics.back();
                            normal_l =
                                FVector::CrossProduct((local - veritics.back()).GetSafeNormal2D(), FVector::UpVector);
                            normal_r = FVector(-normal_l.X, -normal_l.Y, 0);    //(local - p0).GetSafeNormal2D();
                        }
                        p_delta = normal_r * CurbWidth * 100;
                        p1 = local + p_delta;

                        curbdata.veritics[0].push_back(local);
                        curbdata.veritics[0].push_back(p1);
                        curbdata.normal_left.push_back(reverse ? normal_l : -normal_l);
                    }

                    veritics.push_back(local);
                }

                roaddata.veritics.push_back(veritics);
            }

            // 填补两侧boudary顶点数量之差
            for (size_t lane_idx = 0; lane_idx <= roaddata.lane_num - 2; lane_idx += 2)
            {
                int sizeOffset = roaddata.veritics[lane_idx].size() - roaddata.veritics[lane_idx + 1].size();
                if (sizeOffset > 0)
                {
                    while (sizeOffset-- != 0)
                    {
                        roaddata.veritics[lane_idx + 1].push_back(roaddata.veritics[lane_idx + 1].back());
                    }
                }
                else if (sizeOffset < 0)
                {
                    while (sizeOffset++ != 0)
                    {
                        roaddata.veritics[lane_idx].push_back(roaddata.veritics[lane_idx].back());
                    }
                }
            }

            roaddata.lane_vnum = 0;
            for (auto veritics : roaddata.veritics)
            {
                roaddata.lane_vnum += veritics.size();
            }
            roaddata.normals.resize(roaddata.lane_num);
            // roaddata.lane_vnum = roaddata.veritics.size() / roaddata.lane_num;
            roaddata.matrix = "Mat_Road030";
            roaddata.name = std::string("road");

            /*TArray<FVector> LaneVertices;
            TArray<int32> LaneTriangles;
            TArray<FVector> LaneNormals;
            TArray<FVector2D> LaneUV0s;
            FName RecordLane;
            int32 RecordTriangles = 0;*/

            for (size_t lane_idx = 0; lane_idx <= roaddata.lane_num - 2; lane_idx += 2)
            {
                RecordTriangles = LaneVertices.Num();
                for (size_t v_idx = 0; v_idx < roaddata.veritics[lane_idx].size(); v_idx++)
                {
                    LaneVertices.Add(roaddata.veritics[lane_idx][v_idx]);
                    // LaneVertices.Add(roaddata.veritics[v_idx]);
                    LaneUV0s.Add(
                        FVector2D(roaddata.veritics[lane_idx][v_idx].X * roaddata.veritics[lane_idx][v_idx].Y * esp));

                    /*if (v_idx % roaddata.lane_vnum < LaneNormals.Num())
                    {
                        FVector normal = LaneNormals[v_idx % roaddata.lane_vnum];
                        LaneNormals.Add(normal);
                        roaddata.normals[lane_idx].push_back(normal);
                    }
                    else */
                    {
                        FVector a = roaddata.veritics[lane_idx][v_idx];
                        FVector b = v_idx + 1 < roaddata.veritics[lane_idx].size()
                                        ? roaddata.veritics[lane_idx][v_idx + 1]
                                        : roaddata.veritics[lane_idx][v_idx - 1];
                        FVector c = roaddata.veritics[lane_idx + 1][v_idx];
                        FVector d = v_idx + 1 < roaddata.veritics[lane_idx].size()
                                        ? FVector::CrossProduct(a - b, a - c)
                                        : FVector::CrossProduct(b - a, a - c);
                        d = d.GetSafeNormal();

                        // LaneNormals.Add(d);
                        LaneNormals.Add(FVector::UpVector);
                        // roaddata.normals[lane_idx].push_back(d);
                    }
                }

                for (size_t v_idx = 0; v_idx < roaddata.veritics[lane_idx + 1].size(); v_idx++)
                {
                    LaneVertices.Add(roaddata.veritics[lane_idx + 1][v_idx]);
                    // LaneVertices.Add(roaddata.veritics[v_idx]);
                    LaneUV0s.Add(FVector2D(
                        roaddata.veritics[lane_idx + 1][v_idx].X * roaddata.veritics[lane_idx + 1][v_idx].Y * esp));

                    /*if (v_idx % roaddata.lane_vnum < LaneNormals.Num())
                    {
                        FVector normal = LaneNormals[v_idx % roaddata.lane_vnum];
                        LaneNormals.Add(normal);
                        roaddata.normals[lane_idx].push_back(normal);
                    }
                    else */
                    {
                        FVector a = roaddata.veritics[lane_idx + 1][v_idx];
                        FVector b = v_idx + 1 < roaddata.veritics[lane_idx + 1].size()
                                        ? roaddata.veritics[lane_idx + 1][v_idx + 1]
                                        : roaddata.veritics[lane_idx + 1][v_idx - 1];
                        FVector c = roaddata.veritics[lane_idx + 1 - 1][v_idx];
                        FVector d = v_idx + 1 < roaddata.veritics[lane_idx + 1].size()
                                        ? FVector::CrossProduct(a - b, a - c)
                                        : FVector::CrossProduct(b - a, a - c);
                        d = d.GetSafeNormal();

                        // LaneNormals.Add(d);
                        LaneNormals.Add(FVector::UpVector);
                        // roaddata.normals[lane_idx + 1].push_back(d);
                    }
                }

                if (roaddata.lane_vnum < 1)
                {
                    continue;
                }

                for (size_t t_idx = 0 /*RecordTriangles*/; t_idx < roaddata.veritics[lane_idx].size() - 1; t_idx++)
                {
                    FVector d;
                    {
                        FVector a = LaneVertices[RecordTriangles + t_idx + roaddata.veritics[lane_idx].size() + 1];
                        FVector b = LaneVertices[RecordTriangles + t_idx + roaddata.veritics[lane_idx].size()];
                        FVector c = LaneVertices[RecordTriangles + t_idx];
                        d = FVector::CrossProduct(a - b, a - c);
                    }
                    d = d.GetSafeNormal();
                    if (d.Z < 0)
                    {
                        LaneTriangles.Add(RecordTriangles + t_idx + roaddata.veritics[lane_idx].size() + 1);
                        LaneTriangles.Add(RecordTriangles + t_idx + roaddata.veritics[lane_idx].size());
                        LaneTriangles.Add(RecordTriangles + t_idx);
                    }
                    else
                    {
                        LaneTriangles.Add(RecordTriangles + t_idx);
                        LaneTriangles.Add(RecordTriangles + t_idx + roaddata.veritics[lane_idx].size());
                        LaneTriangles.Add(RecordTriangles + t_idx + roaddata.veritics[lane_idx].size() + 1);
                    }

                    {
                        FVector a = LaneVertices[RecordTriangles + t_idx + 1];
                        FVector b = LaneVertices[RecordTriangles + t_idx + roaddata.veritics[lane_idx].size() + 1];
                        FVector c = LaneVertices[RecordTriangles + t_idx];
                        d = FVector::CrossProduct(a - b, a - c);
                    }
                    d = d.GetSafeNormal();
                    if (d.Z < 0)
                    {
                        LaneTriangles.Add(RecordTriangles + t_idx + 1);
                        LaneTriangles.Add(RecordTriangles + t_idx + roaddata.veritics[lane_idx].size() + 1);
                        LaneTriangles.Add(RecordTriangles + t_idx);
                    }
                    else
                    {
                        LaneTriangles.Add(RecordTriangles + t_idx);
                        LaneTriangles.Add(RecordTriangles + t_idx + roaddata.veritics[lane_idx].size() + 1);
                        LaneTriangles.Add(RecordTriangles + t_idx + 1);
                    }
                }
            }

            scene.push_back(roaddata);

            curbdata.lane_vnum = curbdata.veritics.front().size() / curbdata.lane_num;
            curbdata.name = std::string("curb");
            section_map_curb[std::make_pair(road_ptr->getId(), section_ptr->getId())] = scene_curb.size();
            scene_curb.push_back(curbdata);

            if (bds.Num() >= 2)
            {
                bdl_bdldsL.insert(bds[0].Key->getId());
                bdl_bdldsR.insert(bds.Last().Key->getId());
            }
        }

        if (!ShowObjectOnly)
        {
            UMaterialInstanceDynamic* LaneMarkMatInstDynamic =
                MID_Map.Contains(mat_lane) ? MID_Map[mat_lane] : CreateMaterial(mat_lane);

            MID_Map.Add(TPair<FName, UMaterialInstanceDynamic*>(mat_lane, LaneMarkMatInstDynamic));
            // CreateProcedualMesh(LaneVertices, LaneTriangles, LaneNormals, LaneUV0s, CreateMaterial(mat_lane), false,
            // 0, 1, 0);
            RenderLaneNum++;
            if (RenderLaneNum > MaxRenderLaneNum || LaneVertices.Num() > MaxRenderLaneVertices)
            {
                CreateStaticMesh(
                    LaneVertices, LaneTriangles, LaneNormals, LaneUV0s, CreateStaticMaterial(mat_lane), true, 1, 0, 0);
                RecordLane = mat_lane;
                LaneVertices.Empty();
                LaneTriangles.Empty();
                LaneNormals.Empty();
                LaneUV0s.Empty();
                RecordTriangles = 0;
                RenderLaneNum = 0;
            }
        }
    }

    CreateStaticMesh(LaneVertices, LaneTriangles, LaneNormals, LaneUV0s, CreateStaticMaterial(mat_lane), true, 1, 0, 0);
    RecordLane = mat_lane;
    LaneVertices.Empty();
    LaneTriangles.Empty();
    LaneNormals.Empty();
    LaneUV0s.Empty();
    RecordTriangles = 0;
    RenderLaneNum = 0;
    UE_LOG(LogTemp, Log, TEXT("DrawMap Road finish"));
}

void AMapGeneratedActor::DrawMarkDetails(TArray<LaneBoundaryInfo>& lane_bdids)
{
    auto getOffsets =
        [](const std::vector<FVector>& opoints, const std::vector<FVector>& rightDirs, const double offsetWidth)
    {
        assert(opoints.size() == rightDirs.size());
        std::vector<FVector> points(opoints.size());
        for (size_t i = 0; i < opoints.size(); ++i)
        {
            FVector rightDir;
            const FVector curLoc = opoints[i];
            if (i < opoints.size() - 1)
            {
                rightDir = FVector::CrossProduct((opoints[i + 1] - curLoc).GetSafeNormal2D(), FVector::UpVector);
            }
            else if (i > 0)
            {
                rightDir = FVector::CrossProduct((curLoc - opoints[i - 1]).GetSafeNormal2D(), FVector::UpVector);
            }

            FVector rightLoc = curLoc + rightDir * (offsetWidth);
            points[i] = rightLoc;
        }
        return points;
    };

    TMap<FName, FRoadMarkData> RoadMark_Map;
    for (auto lbd0 : lane_bdids)
    {
        hadmap::txLaneBoundaryPtr boundaryPtr = lbd0.BoundaryPtr;
        auto lbd = boundaryPtr->getId();
        hadmap::PointVec points = lbd0.Points;
        if (ShowObjectOnly)
            break;
        // hadmap::getBoundary(pHandle, lbd, boundaryPtr);
        if (boundaryPtr)
        {
            LANE_MARK lmark = boundaryPtr->getLaneMark();
            if (lmark == hadmap::LANE_MARK_None)
            {
                continue;
            }

            hadmap::PointVec opoints = points;
            // static_cast<const hadmap::txLineCurve*>(boundaryPtr->getGeometry())->getPoints(opoints);
            // auto coordtype = boundaryPtr->getGeometry()->getCoordType();

            std::vector<FVector> normaldata;
            // auto section_ptr = lbd0.LanePtr->getSection();
            // auto road_ptr = section_ptr->getRoad();

            size_t id = section_map_curb.at(std::make_pair(lbd0.RoadId, lbd0.SectionId));
            auto sce_curb = scene_curb[id];
            for (size_t i = 0; i < sce_curb.normal_left.size(); ++i)
            {
                normaldata.push_back(
                    FVector(sce_curb.normal_left.at(i).X, sce_curb.normal_left.at(i).Y, sce_curb.normal_left.at(i).Z));
            }

            enum lanetype
            {
                SOLID = 0x0,
                BROKEN = 0x1,
                BROKEN_S = 0x2,
                LDM_L = 0x3,
                LDM_R = 0x4,
                VGL_L = 0x5,
                VGL_R = 0x6,
                BOLD = 0x100,    // decorate solid broken
            };
            enum lanecolor
            {
                WHITE,
                YELLOW,
                GREEN,
                BLUE,
                RED,
                UNKNOW
            };
            struct laneData
            {
                std::vector<FVector> points;
                std::string stype = "SOLID";
                std::string color = "WHITE";
            };
            std::vector<FVector> epoints(opoints.size());
            for (size_t i = 0; i < opoints.size(); ++i)
            {
                FVector curLoc = FVector(opoints.at(i).x, opoints.at(i).y, opoints.at(i).z);
                epoints[i] = curLoc;
            }

            std::vector<laneData> lanedata;
            double lanewidth = 0.15f;    // boundaryPtr->getLaneWidth();
            double lanedinterval = boundaryPtr->getDoubleInterval();
            if (lanewidth < 1e-6)
            {
                lanewidth = (lmark & hadmap::LANE_MARK::LANE_MARK_Bold ? LANE_WIDTH2 : LANE_WIDTH) * 2;
            }
            if (lanedinterval < 1e-6)
            {
                lanedinterval = lanewidth * 2;
            }
            lanewidth *= 0.5;
            lanedinterval *= 0.5;

            std::string stype[2] = {"None", "None"};

            if ((lmark & hadmap::LANE_MARK::LANE_MARK_SolidSolid) == hadmap::LANE_MARK::LANE_MARK_SolidSolid)
            {
                stype[0] = "solid";
                stype[1] = "solid";
            }
            else if ((lmark & hadmap::LANE_MARK::LANE_MARK_SolidBroken) == hadmap::LANE_MARK::LANE_MARK_SolidBroken)
            {
                stype[0] = "solid";
                stype[1] = "broken";
            }
            else if ((lmark & hadmap::LANE_MARK::LANE_MARK_BrokenSolid) == hadmap::LANE_MARK::LANE_MARK_BrokenSolid)
            {
                stype[0] = "broken";
                stype[1] = "solid";
            }
            else if ((lmark & hadmap::LANE_MARK::LANE_MARK_BrokenBroken) == hadmap::LANE_MARK::LANE_MARK_BrokenBroken)
            {
                stype[0] = "broken";
                stype[1] = "broken";
            }
            else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Solid) == hadmap::LANE_MARK::LANE_MARK_Solid)
            {
                stype[0] = "solid";
            }
            else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Broken) == hadmap::LANE_MARK::LANE_MARK_Broken)
            {
                stype[0] = "broken";
            }
            else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Solid2) == hadmap::LANE_MARK::LANE_MARK_Solid2)
            {
                stype[0] = "solid";
            }
            else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Broken2) == hadmap::LANE_MARK::LANE_MARK_Broken2)
            {
                stype[0] = "broken";
            }

            if (((lmark & hadmap::LANE_MARK::LANE_MARK_SolidSolid) == hadmap::LANE_MARK::LANE_MARK_SolidSolid ||
                    (lmark & hadmap::LANE_MARK::LANE_MARK_SolidBroken) == hadmap::LANE_MARK::LANE_MARK_SolidBroken ||
                    (lmark & hadmap::LANE_MARK::LANE_MARK_BrokenSolid) == hadmap::LANE_MARK::LANE_MARK_BrokenSolid ||
                    (lmark & hadmap::LANE_MARK::LANE_MARK_BrokenBroken) == hadmap::LANE_MARK::LANE_MARK_BrokenBroken))
            {
                double offsetWidth = lanedinterval;
                lanedata.resize(2);
                // std::vector<FVector> rightDirs = getRightN(epoints);
                lanedata[0].points = getOffsets(epoints, normaldata, offsetWidth * 100);
                lanedata[0].stype = stype[0];
                lanedata[0].color = "WHITE";
                if ((lmark & hadmap::LANE_MARK::LANE_MARK_Blue) == hadmap::LANE_MARK::LANE_MARK_Blue)
                {
                    lanedata[0].color = "BLUE";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Red) == hadmap::LANE_MARK::LANE_MARK_Red)
                {
                    lanedata[0].color = "RED";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Green) == hadmap::LANE_MARK::LANE_MARK_Green)
                {
                    lanedata[0].color = "GREEN";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Yellow) == hadmap::LANE_MARK::LANE_MARK_Yellow)
                {
                    lanedata[0].color = "YELLOW";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Yellow2) == hadmap::LANE_MARK::LANE_MARK_Yellow2)
                {
                    lanedata[0].color = "YELLOW";
                }

                lanedata[1].points = getOffsets(epoints, normaldata, -offsetWidth * 100);
                lanedata[1].stype = stype[1];
                lanedata[1].color = "WHITE";
                if ((lmark & hadmap::LANE_MARK::LANE_MARK_Blue) == hadmap::LANE_MARK::LANE_MARK_Blue)
                {
                    lanedata[1].color = "BLUE";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Red) == hadmap::LANE_MARK::LANE_MARK_Red)
                {
                    lanedata[1].color = "RED";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Green) == hadmap::LANE_MARK::LANE_MARK_Green)
                {
                    lanedata[1].color = "GREEN";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Yellow) == hadmap::LANE_MARK::LANE_MARK_Yellow)
                {
                    lanedata[1].color = "YELLOW";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Yellow2) == hadmap::LANE_MARK::LANE_MARK_Yellow2)
                {
                    lanedata[1].color = "YELLOW";
                }
            }
            else
            {
                lanedata.resize(1);
                lanedata[0].points = epoints;
                lanedata[0].stype = stype[0];
                lanedata[0].color = "WHITE";
                if ((lmark & hadmap::LANE_MARK::LANE_MARK_Blue) == hadmap::LANE_MARK::LANE_MARK_Blue)
                {
                    lanedata[0].color = "BLUE";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Red) == hadmap::LANE_MARK::LANE_MARK_Red)
                {
                    lanedata[0].color = "RED";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Green) == hadmap::LANE_MARK::LANE_MARK_Green)
                {
                    lanedata[0].color = "GREEN";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Yellow) == hadmap::LANE_MARK::LANE_MARK_Yellow)
                {
                    lanedata[0].color = "YELLOW";
                }
                else if ((lmark & hadmap::LANE_MARK::LANE_MARK_Yellow2) == hadmap::LANE_MARK::LANE_MARK_Yellow2)
                {
                    lanedata[0].color = "YELLOW";
                }

                // if (lmark & hadmap::LANE_MARK::LANE_MARK_Bold)
                //{
                //     lanedata[0].stype += "BOLD";
                // }
                if (lmark & hadmap::LANE_MARK::LANE_MARK_Shielded || lmark & hadmap::LANE_MARK::LANE_MARK_Absent)
                {
                    lanedata[0].color = "UNKNOW";
                }
                if (lmark & hadmap::LANE_MARK::LANE_MARK_LDM)
                {
                    laneData lane;
                    lane.color = "WHITE";
                    // std::vector<FVector> rightDirs = getRightN(epoints);
                    if (bdl_bdldsL.find(lbd) == bdl_bdldsL.end())
                    {
                        lane.points = getOffsets(epoints, normaldata, -lanewidth * 2 - LANE_WIDTH2);
                        lane.stype = "LDM_L";
                        lanedata.push_back(lane);
                    }
                    if (bdl_bdldsR.find(lbd) == bdl_bdldsR.end())
                    {
                        lane.points = getOffsets(epoints, normaldata, lanewidth * 2 + LANE_WIDTH2);
                        lane.stype = "LDM_R";
                        lanedata.push_back(lane);
                    }
                }
                else if (lmark & hadmap::LANE_MARK::LANE_MARK_VGL)
                {
                    laneData lane;
                    lane.color = WHITE;
                    if (bdl_bdldsL.find(lbd) == bdl_bdldsL.end())
                    {
                        lane.points = epoints;
                        lane.stype = "VGL_L";
                        lanedata.push_back(lane);
                    }
                    if (bdl_bdldsR.find(lbd) == bdl_bdldsR.end())
                    {
                        lane.points = epoints;
                        lane.stype = "VGL_R";
                        lanedata.push_back(lane);
                    }
                }
            }

            int num = 0;
            for (auto& lane : lanedata)
            {
                std::vector<FVector>& points = lane.points;
                if (points.empty() || normaldata.empty())
                {
                    continue;
                }
                FVector2D offp[4];
                const double lw = lanewidth * 2;
                offp[0] = FVector2D(0, -lw);
                offp[1] = FVector2D(0, lw);
                offp[2] = FVector2D(0, -lw);
                offp[3] = FVector2D(0, lw);
                std::string stype = lane.stype;    // &0xff;

                if (points.size() < 2)
                    continue;

                // get linearcolor
                const FName color_name = *FString(UTF8_TO_TCHAR(lane.color.c_str()));

                FLinearColor linera_color =
                    GetLinerColor(UTF8_TO_TCHAR(lane.color.c_str()));    // = GetLinerColor(color_name);

                // get material
                std::string mark_type = "mark_" + lane.stype;
                FString str_mark_type(UTF8_TO_TCHAR(mark_type.c_str()));
                const FName MatID = FName(*(str_mark_type + color_name.ToString()));
                const FName mat_mark = FName(*str_mark_type);

                UMaterialInstanceDynamic* RoadMarkMatInstDynamic =
                    MID_Map.Contains(MatID) ? MID_Map[MatID] : CreateMaterial(mat_mark);
                // UMaterialInstanceDynamic* RoadMarkMatInstDynamic = CreateMaterial(mat_mark);

                MID_Map.Add(TPair<FName, UMaterialInstanceDynamic*>(MatID, RoadMarkMatInstDynamic));
                // set color with linearcolor parameter
                RoadMarkMatInstDynamic->SetVectorParameterValue(TEXT("color"), linera_color);

                FRoadMarkData& CurrentRoadMark = RoadMark_Map.FindOrAdd(MatID);
                TArray<FVector>& RoadMarkVertices = CurrentRoadMark.RoadMarkVertices;
                TArray<int32>& RoadMarkTriangles = CurrentRoadMark.RoadMarkTriangles;
                TArray<FVector>& RoadMarkNormals = CurrentRoadMark.RoadMarkNormals;
                TArray<FVector2D>& RoadMarkUV0s = CurrentRoadMark.RoadMarkUV0s;
                int32& RecordIndex = CurrentRoadMark.RocordIndex;

                /*// 车道线顶点数据 | 车道线索引数据 | 车道线法线数据 | 车道线UV数据
                TArray<FVector> RoadMarkVertices;
                TArray<int32> RoadMarkTriangles;
                TArray<FVector> RoadMarkNormals;
                TArray<FVector2D> RoadMarkUV0s;*/

                //    FVector upV = FVector(0, 0, LANE_HEIGHT_OFFSET);
                bool flag = false;
                FVector lastLoc;
                float LaneDirgee = 0;
                if (points.size() > 2)
                {
                    FVector Dir = points.back() - points.front();
                    Dir.Normalize();
                    LaneDirgee = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
                }
                for (size_t i = 0; i < points.size(); /*++i*/)
                {
                    if (points.size() < 2)
                        continue;
                    // const FVector& rightDir = normaldata.size() > i ? normaldata[i] : normaldata[normaldata.size() -
                    // 1];
                    const FVector& rightDir =
                        i > 0
                            ? FVector::CrossProduct(((points[i] - points[i - 1])).GetSafeNormal2D(), FVector::UpVector)
                            : FVector::CrossProduct(((points[1] - points[0])).GetSafeNormal2D(), FVector::UpVector);

                    const FVector& curLoc = points[i];
                    float Dis = 0.f;

                    if (i > 0)
                    {
                        Dis = (points[i] - lastLoc).Size() * 0.01 / 5;
                        // 与新版地图编辑器保持一致，暂不启用双向道路虚线UV对齐
                        /*
                       if (LaneDirgee < 0.f)
                       {
                          Dis *= -1.f;

                       }
                        */
                    }
                    lastLoc = curLoc;
                    if (curLoc.ContainsNaN() || rightDir.ContainsNaN())
                    {
                        if (flag)
                        {
                            i += 8;
                            flag = false;
                        }
                        else
                        {
                            i += 1;
                            // i += 4;
                            flag = true;
                        }
                        continue;
                    }
                    const FVector frontDir(-rightDir.Y, rightDir.X, 0);
                    int jo = i % 2 == 0 ? 0 : 1;

                    FVector leftLoc =
                        curLoc + rightDir * offp[jo * 2].Y * 50 + frontDir * offp[jo * 2].X * 50 + FVector(0, 0, 1);
                    FVector rightLoc = curLoc + rightDir * offp[jo * 2 + 1].Y * 50 +
                                       frontDir * offp[jo * 2 + 1].X * 50 + FVector(0, 0, 1);
                    /*    leftLoc += upV;
                        rightLoc += upV;*/

                    RoadMarkVertices.Add(leftLoc);
                    RoadMarkVertices.Add(rightLoc);
                    RoadMarkNormals.Add(FVector::UpVector * -1.f);
                    RoadMarkNormals.Add(FVector::UpVector * -1.f);
                    // RoadMarkUV0s.Add(FVector2D(0, leftLoc.Y));
                    // RoadMarkUV0s.Add(FVector2D(1, rightLoc.Y));

                    float currentY = (i > 0 && RoadMarkUV0s.Num() > 0) ? RoadMarkUV0s.Last().Y : 0;
                    if (num / 2 == 0)
                    {
                        RoadMarkUV0s.Add(FVector2D(0, currentY + Dis));
                        RoadMarkUV0s.Add(FVector2D(1, currentY + Dis));
                    }
                    else
                    {
                        RoadMarkUV0s.Add(FVector2D(0, currentY + Dis));
                        RoadMarkUV0s.Add(FVector2D(1, currentY + Dis));
                    }

                    if (flag)
                    {
                        i += 1;    // 8
                        flag = false;
                    }
                    else
                    {
                        i += 1;    // 4
                        flag = true;
                    }
                }

                // size_t stp = strcmp(stype.c_str(), "solid") ? 4 : 2;
                size_t stp = 2;
                for (int i = RecordIndex; i <= RoadMarkVertices.Num() - 4; i += stp)
                {
                    FVector d;
                    {
                        FVector a = RoadMarkVertices[i];
                        FVector b = RoadMarkVertices[i + 1];
                        FVector c = RoadMarkVertices[i + 3];
                        d = FVector::CrossProduct(a - b, a - c);
                    }
                    d = d.GetSafeNormal();
                    if (d.Z > 0)
                    {
                        RoadMarkTriangles.Add(i);
                        RoadMarkTriangles.Add(i + 1);
                        RoadMarkTriangles.Add(i + 3);
                        RoadMarkTriangles.Add(i);
                        RoadMarkTriangles.Add(i + 3);
                        RoadMarkTriangles.Add(i + 2);
                    }
                    else
                    {
                        RoadMarkTriangles.Add(i + 3);
                        RoadMarkTriangles.Add(i + 1);
                        RoadMarkTriangles.Add(i);
                        RoadMarkTriangles.Add(i + 2);
                        RoadMarkTriangles.Add(i + 3);
                        RoadMarkTriangles.Add(i);
                    }
                }

                // if (mark_data.size() > 1)
                {
                    if (!ShowObjectOnly)
                    {
                        CurrentRoadMark.LineNum++;
                        if (CurrentRoadMark.LineNum >= MaxRenderLineNum ||
                            RoadMarkVertices.Num() > MaxRenderLineVertices)
                        {
                            CreateStaticMesh(RoadMarkVertices, RoadMarkTriangles, RoadMarkNormals, RoadMarkUV0s,
                                RoadMarkMatInstDynamic, false, 0, 0, 0.02, false);
                            RoadMarkVertices.Empty();
                            RoadMarkTriangles.Empty();
                            RoadMarkNormals.Empty();
                            RoadMarkUV0s.Empty();
                            CurrentRoadMark.LineNum = 0;
                        }
                        else
                        {
                            RecordIndex = RoadMarkVertices.Num();
                        }
                    }
                }

                continue;
            }
        }
    }

    for (auto ElemMarkData : RoadMark_Map)
    {
        CreateStaticMesh(ElemMarkData.Value.RoadMarkVertices, ElemMarkData.Value.RoadMarkTriangles,
            ElemMarkData.Value.RoadMarkNormals, ElemMarkData.Value.RoadMarkUV0s, MID_Map[ElemMarkData.Key], false, 0, 0,
            0.02, false);
    }
    RoadMark_Map.Empty();

    UE_LOG(LogTemp, Log, TEXT("DrawMap RoadMarkLine finish"));
}

void AMapGeneratedActor::DrawJunctionDetails()
{
    // junction curb function
    struct MyStruct
    {
        float Angle;
        FVector Loc;

        MyStruct(const FVector& Right) : Loc(Right)
        {
        }

        bool operator<(const MyStruct& m) const
        {
            return Angle < m.Angle;
        }
        bool operator==(const MyStruct& m) const
        {
            return Loc == m.Loc;
        }
    };

    // 所有路口顶点数据|所有道路索引数据|所有道路法线数据|所有道路UV数据
    TArray<FVector> JunctionVertices;
    TArray<int32> JunctionTriangles;
    TArray<FVector> JunctionNormals;
    TArray<FVector2D> JunctionUV0s;
    // 所有路口Lanelink顶点数据|所有路口Lanelink索引数据|所有路口Lanelink法线数据|所有路口LanelinkUV数据
    TArray<FVector> JunctionLaneVertices;
    TArray<int32> JunctionLaneTriangles;
    TArray<FVector> JunctionLaneNormals;
    TArray<FVector2D> JunctionLaneUV0s;
    // junction
    double minLen = 0;
    const double junc_seq = minLen > 0 ? minLen : 2.0;
    hadmap::txJunctions juncs;
    if (!ShowObjectOnly)
    {
        SHadmap->getJunctions(juncs);
    }

    UE_LOG(LogTemp, Log, TEXT("junction size = %d"), juncs.size());
    double esp = 0.01;
    for (auto junc : juncs)
    {
        std::vector<hadmap::lanelinkpkid> ids;
        // junc->getLaneLink(ids);
        SHadmap->getTxJunctionLanLink(junc, ids);

        FVector cen(0, 0, 0);
        TArray<MyStruct> vertices;

        TArray<TArray<FVector>> LaneVerticesList;
        int32 MaxVerticesLen = 0;
        int32 LastVertexIndex = 0;

        int32 MaxPointNum = 0;
        TMap<roadpkid, lanepkid> LaneMap;
        TMap<roadpkid, int32> LaneLengthMap;
        for (auto i : ids)
        {
            hadmap::txLaneLinkPtr link;
            SHadmap->getLaneLink(i, link);
            auto RoadID = link->fromRoadId();
            auto LaneID = link->fromLaneId();

            hadmap::PointVec LinkPoints;

            if (!LaneLengthMap.Contains(RoadID))
            {
                LaneMap.Add(RoadID, LaneID);
            }
            else if (LaneID < LaneLengthMap[RoadID])
            {
                LaneMap.Add(RoadID, LaneID);
            }
        }

        for (auto i : ids)
        {
            hadmap::txLaneLinkPtr link;
            SHadmap->getLaneLink(i, link);
            TArray<FVector> LaneVertices_Left;
            TArray<FVector> LaneVertices_Right;

            TArray<hadmap::roadpkid> ToRoadList;
            TArray<hadmap::roadpkid> FromRoadList;
            // hadmap::getLaneLink(pHandle, i, link);
            if (link.get() && link->getGeometry())
            {
                txLaneBoundaries Bds;
                txLaneBoundaries Left = link->getLeftBoundaries();
                txLaneBoundaries Right = link->getRightBoundaries();

                auto FromRoadID = link->fromRoadId();
                auto ToRoadID = link->toRoadId();

                hadmap::txRoadPtr FromRoadPtr;
                hadmap::txRoadPtr ToRoadPtr;
                SHadmap->getRoad(FromRoadID, true, FromRoadPtr);
                SHadmap->getRoad(ToRoadID, true, ToRoadPtr);

                if (!FromRoadList.Contains(FromRoadID))
                {
                    FromRoadList.Add(FromRoadID);
                    for (auto section_ptr : FromRoadPtr->getSections())
                    {
                        for (auto lane_ptr : section_ptr->getLanes())
                        {
                            hadmap::PointVec LeftPoints;
                            static_cast<const hadmap::txLineCurve*>(lane_ptr->getLeftBoundary()->getGeometry())
                                ->getPoints(LeftPoints);
                            hadmap::PointVec RightPoints;
                            static_cast<const hadmap::txLineCurve*>(lane_ptr->getRightBoundary()->getGeometry())
                                ->getPoints(RightPoints);
                            if (LeftPoints.size() > 1)
                            {
                                LonLatToLocal(LeftPoints.back().x, LeftPoints.back().y, LeftPoints.back().z);
                                int32 Size = vertices.Num();
                                if (vertices.AddUnique(
                                        FVector(LeftPoints.back().x, LeftPoints.back().y, LeftPoints.back().z)) == Size)
                                {
                                    cen += vertices.Last().Loc;
                                }
                            }

                            if (RightPoints.size() > 1)
                            {
                                LonLatToLocal(RightPoints.back().x, RightPoints.back().y, RightPoints.back().z);
                                int32 Size = vertices.Num();
                                if (vertices.AddUnique(FVector(
                                        RightPoints.back().x, RightPoints.back().y, RightPoints.back().z)) == Size)
                                {
                                    cen += vertices.Last().Loc;
                                }
                            }
                        }
                    }
                }
                if (!ToRoadList.Contains(ToRoadID))
                {
                    ToRoadList.Add(ToRoadID);
                    for (auto section_ptr : ToRoadPtr->getSections())
                    {
                        for (auto lane_ptr : section_ptr->getLanes())
                        {
                            hadmap::PointVec LeftPoints;
                            static_cast<const hadmap::txLineCurve*>(lane_ptr->getLeftBoundary()->getGeometry())
                                ->getPoints(LeftPoints);
                            hadmap::PointVec RightPoints;
                            static_cast<const hadmap::txLineCurve*>(lane_ptr->getRightBoundary()->getGeometry())
                                ->getPoints(RightPoints);
                            if (LeftPoints.size() > 1)
                            {
                                LonLatToLocal(LeftPoints.front().x, LeftPoints.front().y, LeftPoints.front().z);
                                int32 Size = vertices.Num();
                                if (vertices.AddUnique(FVector(
                                        LeftPoints.front().x, LeftPoints.front().y, LeftPoints.front().z)) == Size)
                                {
                                    cen += vertices.Last().Loc;
                                }
                            }

                            if (RightPoints.size() > 1)
                            {
                                LonLatToLocal(RightPoints.front().x, RightPoints.front().y, RightPoints.front().z);
                                int32 Size = vertices.Num();
                                if (vertices.AddUnique(FVector(
                                        RightPoints.front().x, RightPoints.front().y, RightPoints.front().z)) == Size)
                                {
                                    cen += vertices.Last().Loc;
                                }
                            }
                        }
                    }
                }

                for (int32 i = 0; i < Left.size(); i++)
                {
                    Bds.push_back(Left[i]);
                    Bds.push_back(Right[i]);
                }

                for (int32 lane_idx = 0; lane_idx < int32(Bds.size() - 1); lane_idx++)
                {
                    hadmap::PointVec LeftPoints;
                    hadmap::PointVec RightPoints;
                    hadmap::PointVec LinkPoints;

                    static_cast<const hadmap::txLineCurve*>(link->getGeometry())->getPoints(LinkPoints);
                    // if (LaneMap.Contains(link->fromRoadId()) && (LaneMap[link->fromRoadId()] == link->fromLaneId()))
                    {
                        hadmap::txLanePtr lanePtr;
                        getLane(Handle, link->fromTxLaneId(), lanePtr);
                        float LaneWidth = lanePtr->getLaneWidth() * 0.5;
                        createOffsetGeom(LinkPoints, LaneWidth, LaneWidth, LeftPoints);
                        createOffsetGeom(LinkPoints, -LaneWidth, -LaneWidth, RightPoints);
                    }
                    //  else
                    // {
                    // static_cast<const hadmap::txLineCurve*>(Bds[lane_idx]->getGeometry())->getPoints(LeftPoints);
                    // static_cast<const hadmap::txLineCurve*>(Bds[lane_idx +
                    // 1]->getGeometry())->getPoints(RightPoints);
                    // }

                    for (int32 i = 0; i < LeftPoints.size(); i++)
                    {
                        LonLatToLocal(LeftPoints[i].x, LeftPoints[i].y, LeftPoints[i].z);
                        LaneVertices_Left.Add(FVector(LeftPoints[i].x, LeftPoints[i].y, LeftPoints[i].z));
                    }
                    for (int32 i = 0; i < RightPoints.size(); i++)
                    {
                        LonLatToLocal(RightPoints[i].x, RightPoints[i].y, RightPoints[i].z);
                        LaneVertices_Right.Add(FVector(RightPoints[i].x, RightPoints[i].y, RightPoints[i].z));
                    }

                    int32 Left_Right_Max =
                        LeftPoints.size() > RightPoints.size() ? LeftPoints.size() : RightPoints.size();
                    if (MaxVerticesLen < Left_Right_Max)
                        MaxVerticesLen = Left_Right_Max;
                }
                if (LaneVertices_Left.Num() > 0 && LaneVertices_Right.Num() > 0)
                {
                    LaneVerticesList.Add(LaneVertices_Left);
                    LaneVerticesList.Add(LaneVertices_Right);
                }

                for (auto& Elem : LaneVerticesList)
                {
                    int32 sizeOffset = MaxVerticesLen - Elem.Num();
                    // Elem.SetNum(MaxVerticesLen);
                    if (sizeOffset > 0)
                    {
                        while (sizeOffset-- != 0)
                        {
                            Elem.Add(FVector(Elem.Last()));
                        }
                    }
                }
            }
        }

        for (int32 i = 0; i < LaneVerticesList.Num(); i++)
        {
            for (int32 j = 0; j < LaneVerticesList[i].Num(); j++)
            {
                {
                    JunctionLaneVertices.Add(LaneVerticesList[i][j]);
                    JunctionLaneNormals.Add(FVector::UpVector);
                    JunctionLaneUV0s.Add(FVector2D(LaneVerticesList[i][j].X * LaneVerticesList[i][j].Y * esp));
                }
            }
        }

        for (int32 i = 0; i < (LaneVerticesList.Num() - 1); i = i + 1)
        {
            int32 VerticesLen = LaneVerticesList[i].Num();
            for (int32 j = 0; j < (VerticesLen - 1); j = j + 1)
            {
                FVector d;
                {
                    FVector a = JunctionLaneVertices[LastVertexIndex + j + 0];
                    FVector b = JunctionLaneVertices[LastVertexIndex + VerticesLen + j + 0];
                    FVector c = JunctionLaneVertices[LastVertexIndex + VerticesLen + j + 1];

                    if (a == b || b == c || a == c)
                    {
                        d = FVector::UpVector;
                    }
                    else
                    {
                        d = FVector::CrossProduct(a - b, a - c);
                    }
                }
                d = d.GetSafeNormal();
                if (d.ContainsNaN())
                    continue;
                if (d.Z < 0)
                {
                    JunctionLaneTriangles.Add(LastVertexIndex + j + 0);
                    JunctionLaneTriangles.Add(LastVertexIndex + VerticesLen + j + 0);
                    JunctionLaneTriangles.Add(LastVertexIndex + VerticesLen + j + 1);
                }
                else
                {
                    JunctionLaneTriangles.Add(LastVertexIndex + VerticesLen + j + 1);
                    JunctionLaneTriangles.Add(LastVertexIndex + VerticesLen + j + 0);
                    JunctionLaneTriangles.Add(LastVertexIndex + j + 0);
                }

                {
                    FVector a = JunctionLaneVertices[LastVertexIndex + j + 1];
                    FVector b = JunctionLaneVertices[LastVertexIndex + j + 0];
                    FVector c = JunctionLaneVertices[LastVertexIndex + VerticesLen + j + 1];
                    if (a == b || b == c || a == c)
                    {
                        d = FVector::UpVector;
                    }
                    else
                    {
                        d = FVector::CrossProduct(a - b, a - c);
                    }
                    d = FVector::CrossProduct(a - b, a - c);
                }
                d = d.GetSafeNormal();
                if (d.ContainsNaN())
                    continue;
                if (d.Z < 0)
                {
                    JunctionLaneTriangles.Add(LastVertexIndex + j + 1);
                    JunctionLaneTriangles.Add(LastVertexIndex + j + 0);
                    JunctionLaneTriangles.Add(LastVertexIndex + VerticesLen + j + 1);
                }
                else
                {
                    JunctionLaneTriangles.Add(LastVertexIndex + VerticesLen + j + 1);
                    JunctionLaneTriangles.Add(LastVertexIndex + j + 0);
                    JunctionLaneTriangles.Add(LastVertexIndex + j + 1);
                }
            }
            LastVertexIndex += VerticesLen;

            CreateStaticMesh(JunctionLaneVertices, JunctionLaneTriangles, JunctionLaneNormals, JunctionLaneUV0s,
                CreateStaticMaterial(MatRoad), true, 1, 0, -0.01);
            JunctionLaneTriangles.Empty();
        }

        JunctionLaneVertices.Empty();
        JunctionLaneTriangles.Empty();
        JunctionLaneNormals.Empty();
        JunctionLaneUV0s.Empty();

        int JunctionVertices_Length = JunctionVertices.Num();

        if (vertices.Num() > 2)
        {
            double len = 0;

            len /= vertices.Num();
            cen /= vertices.Num();
            int n = std::ceil(len / junc_seq);

            // std::sort(vertices.begin(), vertices.end());

            JunctionVertices.Add(cen);
            JunctionNormals.Add(FVector::UpVector);
            JunctionUV0s.Add(FVector2D(cen.X * esp, cen.Y * esp));

            for (size_t i = 0; i < vertices.Num(); i++)
            {
                FVector Dir = (vertices[i].Loc - cen).GetSafeNormal();
                vertices[i].Angle = FMath::Atan2(Dir.Y, Dir.X);
            }
            vertices.StableSort();
            for (size_t i = 0; i < vertices.Num(); i++)
            {
                JunctionVertices.Add(vertices[i].Loc);
                JunctionUV0s.Add(FVector2D(vertices[i].Loc.X * esp, vertices[i].Loc.Y * esp));
                JunctionNormals.Add(FVector::UpVector);
            }

            // junction triangles set
            for (size_t i = JunctionVertices_Length + 1; i < JunctionVertices.Num(); i++)
            {
                int32 Index1 = JunctionVertices_Length;
                int32 Index2 = (JunctionVertices.Num() - 1) == i ? JunctionVertices_Length + 1 : i + 1;
                int32 Index3 = i;

                FVector d = FVector::CrossProduct(JunctionVertices[Index1] - JunctionVertices[Index2],
                    JunctionVertices[Index1] - JunctionVertices[Index3])
                                .GetSafeNormal();
                if (d.Z < 0)
                {
                    JunctionTriangles.Add(Index1);
                    JunctionTriangles.Add(Index2);
                    JunctionTriangles.Add(Index3);
                }
                else
                {
                    JunctionTriangles.Add(Index3);
                    JunctionTriangles.Add(Index2);
                    JunctionTriangles.Add(Index1);
                }
            }
        }
    }

    if (!ShowObjectOnly)
    {
        CreateStaticMesh(JunctionVertices, JunctionTriangles, JunctionNormals, JunctionUV0s,
            CreateStaticMaterial(MatRoad), true, 1, 0, 0);
    }

    // 注册所有Components
    RegisterAllComponents();
    UE_LOG(LogTemp, Log, TEXT("DrawMap Junction finish"));
}

void AMapGeneratedActor::DrawObjDetails()
{
    // object | OBJECT
    hadmap::txObjects objects;
    // SHadmap->GetObjects(std::vector< hadmap::txLaneId >(), std::vector< hadmap::OBJECT_TYPE >(), objects);
    std::vector<hadmap::OBJECT_TYPE> types;
    if (bShowObject)
    {
        SHadmap->GetObjects(std::vector<hadmap::txLaneId>(), types, objects);
    }

    UE_LOG(LogTemp, Log, TEXT("obj size = %d"), objects.size());
    hadmap::txObjects Softobjects;
    for (const auto& obj : objects)
    {
        if (obj->getObjectType() == OBJECT_TYPE_Pole)
        {
            Softobjects.push_back(obj);
        }
    }
    for (const auto& obj : objects)
    {
        if (obj->getObjectType() != OBJECT_TYPE_Pole)
        {
            Softobjects.push_back(obj);
        }
    }
    for (const auto& obj : Softobjects)
    {
        FString str_obj_name(UTF8_TO_TCHAR(obj->getName().c_str()));
        const FName obj_name = FName(*str_obj_name);
        std::string Type, SubType;
        FString ObjectType, ObjectSubType;
        if (FString* FindStr = ObjectTypeStrMap.Find(obj->getObjectType()))
        {
            ObjectType = *FindStr;
        }
        OBJECT_SUB_TYPE subtype = obj->getObjectSubType();
        if (FString* FindStr = ObjectSubTypeStrMap.Find(obj->getObjectSubType()))
        {
            ObjectSubType = *FindStr;
        }

        FString CombinedName = ObjectType + TEXT("_") + ObjectSubType;

        double len, wid, hei;
        obj->getLWH(len, wid, hei);
        double r, p, y;
        obj->getRPY(r, p, y);

        // hadmap::txPoint pos = obj->getPos();
        hadmap::txPoint pos = SHadmap->getTxObjectPos(obj);

        FRotator obj_rotator;
        FVector obj_location;
        bool bModifyAxis = false;
        bool bSortLengthWidth = false;
        double rr, pp, yy;
        obj->getRPY(rr, pp, yy);
        obj_rotator = FRotator(pp, -FMath::RadiansToDegrees(yy), rr);
        UE_LOG(LogTemp, Log, TEXT("obj id:%llu, CombinedName:%s, pos x:%f, y:%f, Yaw:%f"), obj->getId(), *CombinedName,
            pos.x, pos.y, obj_rotator.Yaw);
        if (OBJECT_TYPE_Pole == obj->getObjectType())
        {
            obj_rotator.Yaw -= 90.f;    // 地图以横杆朝向为正方向，与模型差90度
        }
        if (bOldVersion)
        {
            if (OBJECT_TYPE_Pole == obj->getObjectType())
            {
                obj_rotator.Yaw -= 90;
            }
            else if (OBJECT_TYPE_TrafficSign == obj->getObjectType() ||
                     OBJECT_TYPE_TrafficLights == obj->getObjectType())
            {
                obj_rotator.Yaw += 180.f;
            }
            else if (RoadSign_Turn_Left_Waiting == obj->getObjectSubType())
            {
                obj_rotator.Yaw += 180.f;
            }
            else if (Surface_Protrusion == obj->getObjectSubType() ||
                     Obstacle_Reflective_Road_Sign == obj->getObjectSubType() ||
                     Obstacle_Parking_Lot == obj->getObjectSubType() ||
                     Obstacle_Ground_Lock == obj->getObjectSubType() ||
                     Obstacle_Plastic_Vehicle_Stopper == obj->getObjectSubType() ||
                     Obstacle_Parking_Limit_Position_Pole_2m == obj->getObjectSubType() ||
                     Obstacle_Support_Vehicle_Stopper == obj->getObjectSubType() ||
                     ChargingPile_001 == obj->getObjectSubType() || Tree_001 == obj->getObjectSubType() ||
                     Building_001 == obj->getObjectSubType())
            {
                obj_rotator.Yaw -= 90.f;
            }
        }
        if (OBJECT_TYPE_Pole == obj->getObjectType() || OBJECT_TYPE_TrafficLights == obj->getObjectType() ||
            OBJECT_TYPE_TrafficSign == obj->getObjectType() || OBJECT_TYPE_RoadSign == obj->getObjectType() ||
            OBJECT_TYPE_Arrow == obj->getObjectType() || OBJECT_TYPE_Stop == obj->getObjectType() ||
            OBJECT_TYPE_Surface == obj->getObjectType())
        {
            obj_rotator.Yaw -= 90.f;    // 以下类型模型正方向为Y，调整为X
            bModifyAxis = true;
        }

        if (ChargingPile_001 == obj->getObjectSubType() ||
            Obstacle_Parking_Hole == obj->getObjectSubType()
            //|| Obstacle_Lamp == obj->getObjectSubType()
            || Obstacle_Road_Curb == obj->getObjectSubType())
        {
            bSortLengthWidth = true;
        }

        obj_rotator.Yaw -= 90.f;    // UE正方向是向南，地图正方向为东

        LonLatToLocal(pos.x, pos.y, pos.z);

        obj_location = FVector(pos.x, pos.y, pos.z + 2);
        if (obj_location.ContainsNaN())
        {
            UE_LOG(LogTemp, Error, TEXT("obj_location.ContainsNaN() ,CombinedName:%s"), *CombinedName);
            continue;
        }
        if (!CombinedName.IsEmpty())
        {
            AActor* NewItem = nullptr;
            if (TSubclassOf<AActor> Class = GetActor(CombinedName))
            {
                std::map<std::string, std::string> UserData;
                SHadmap->getTxObjectUserData(obj, UserData);
                if (UserData.size() > 0)
                {
                    std::map<std::string, std::string>::iterator UserIter = UserData.find("pole_id");
                    if (UserIter != UserData.end())
                    {
                        FString PoleID = FString(UTF8_TO_TCHAR(UserIter->second.c_str()));
                        if (FMapObjInfo* FindItem = MapObjs.Find(FName(PoleID)))
                        {
                            TWeakObjectPtr<AActor> Pole = FindItem->Actor;
                            // 地图编辑器中杆子上的灯牌位置有误差，计算灯牌在杆子正朝向的投影距离并增加杆子半径偏移量后得到准确位置
                            if (Pole.IsValid())
                            {
                                FVector PoleLocation = Pole.Get()->GetActorLocation();
                                FRotator PoleRotator = Pole.Get()->GetActorRotation();
                                float Radius_Vertical = 0.f;
                                float Radius_Cross = 0.f;
                                if (OBJECT_SUB_TYPE::POLE_VERTICAL == FindItem->ObjSubType ||
                                    OBJECT_SUB_TYPE::POLE_Pillar_3m == FindItem->ObjSubType ||
                                    OBJECT_SUB_TYPE::POLE_Pillar_6m == FindItem->ObjSubType)
                                {
                                    Radius_Vertical = 20.f;
                                    Radius_Cross = 0.f;
                                }
                                else if (OBJECT_SUB_TYPE::POLE_CROSS == FindItem->ObjSubType ||
                                         OBJECT_SUB_TYPE::POLE_Cantilever == FindItem->ObjSubType)
                                {
                                    Radius_Vertical = 20.f;
                                    Radius_Cross = 14.f;
                                }

                                FRotator MeshRotator = obj_rotator;

                                /* if (OBJECT_TYPE_Pole == obj->getObjectType()
                                     || OBJECT_TYPE_TrafficLights == obj->getObjectType()
                                     || OBJECT_TYPE_TrafficSign == obj->getObjectType()
                                     || OBJECT_TYPE_RoadSign == obj->getObjectType()
                                     || OBJECT_TYPE_Obstacle == obj->getObjectType()
                                     || OBJECT_TYPE_Arrow == obj->getObjectType()
                                     || OBJECT_TYPE_Stop == obj->getObjectType()
                                     || OBJECT_TYPE_Surface == obj->getObjectType())*/
                                if (bModifyAxis)
                                {
                                    MeshRotator.Yaw += 90.f;    // 以下类型模型正方向为Y，上面已减过90,
                                }

                                FVector MeshDir = MeshRotator.Vector();

                                // 杆与灯牌的水平方向向量
                                PoleLocation.Z = obj_location.Z;
                                FVector ObjVector = obj_location - PoleLocation;

                                // 杆子的朝向（与道路方向垂直）
                                FVector PoleDir = PoleRotator.Vector();
                                // 地图中灯牌位置投影到杆子正朝向后的距离
                                float Distance = FVector::DotProduct(ObjVector, PoleDir);
                                obj_location = PoleLocation + Distance * PoleDir;

                                // 判断灯牌是在横杆还是竖杆
                                if ((obj_location - PoleLocation).SizeSquared() < (Radius_Vertical * Radius_Vertical))
                                {
                                    obj_location = PoleLocation + MeshDir * Radius_Vertical * 0.5;
                                }
                                else
                                {
                                    obj_location += MeshDir * Radius_Cross * 0.5;
                                }
                            }
                        }
                    }
                }

                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                FVector Origin, Extent;
                if (AActor* CDO = Cast<AActor>(Class.Get()->GetDefaultObject()))
                {
                    if (!bModifyAxis && CDO->ActorHasTag(TEXT("Axis_Y")))
                    {
                        bModifyAxis = true;
                        obj_rotator.Yaw -= 90.f;
                    }
                }
                FRotator ActorRot = obj_rotator;
                FTransform SpawnTM(FRotator(), GetActorLocation() + obj_location);
                NewItem = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, Class.Get(), SpawnTM));

                if (!NewItem)
                {
                    continue;
                }

                if (obj->getObjectType() == OBJECT_TYPE_PedestrianBridge)
                {
                    std::map<std::string, std::string> userData;
                    std::map<std::string, std::string>::iterator iterData;
                    SHadmap->getTxObjectUserData(obj, userData);
                    iterData = userData.find("bridge_span");
                    bool bRelateRoad = false;

                    if (iterData != userData.end())
                    {
                        float BeidgeLength = std::stof(iterData->second) * 100.f;
                        UFloatProperty* FloatProp =
                            FindField<UFloatProperty>(NewItem->GetClass(), TEXT("CustomLength"));

                        if (FloatProp)
                        {
                            FloatProp->SetPropertyValue_InContainer(NewItem, BeidgeLength);
                        }
                    }
                }
                UGameplayStatics::FinishSpawningActor(NewItem, SpawnTM);

                NewItem->GetActorBounds(false, Origin, Extent);
                Extent *= 2.f;

                NewItem->GetRootComponent()->SetMobility(EComponentMobility::Movable);
                NewItem->SetActorRotation(ActorRot);
                NewItem->GetRootComponent()->SetMobility(EComponentMobility::Static);

                if (bModifyAxis)
                {
                    float tmp = Extent.Y;
                    Extent.Y = Extent.X;
                    Extent.X = tmp;
                }

                if (bSortLengthWidth)
                {
                    float tmp = Extent.Y;
                    Extent.Y = Extent.X;
                    Extent.X = tmp;
                }

                FVector Scale = FVector(len / Extent.X, wid / Extent.Y, hei / Extent.Z) * 100.f;
                Scale.X = fabs(Scale.X);
                Scale.Y = fabs(Scale.Y);
                Scale.Z = fabs(Scale.Z);
                // FVector Scale = FVector(wid, len, hei);
                if (Scale.X <= 0.f)
                    Scale.X = 1.f;
                if (Scale.Y <= 0.f)
                    Scale.Y = 1.f;
                if (Scale.Z <= 0.f)
                    Scale.Z = 1.f;

                FString ObjID = FString::Printf(TEXT("%llu"), obj->getId());
                if (!ObjID.IsEmpty())
                    NewItem->Rename(*(ObjID));
                FMapObjInfo ObjInfo;
                ObjInfo.Actor = NewItem;
                ObjInfo.ObjSubType = subtype;
                ObjInfo.ObjType = obj->getObjectType();
                MapObjs.Add(FName(ObjID), ObjInfo);

                if (CombinedName.Contains("OBJECT_TYPE_Surface"))
                {
                    Scale = FVector(wid, len, hei);
                    if (Scale.X <= 0.f)
                        Scale.X = 1.f;
                    if (Scale.Y <= 0.f)
                        Scale.Y = 1.f;
                    if (Scale.Z <= 0.f)
                        Scale.Z = 1.f;
                    if (!CombinedName.Contains("OBJECT_TYPE_Surface_Surface_Protrusion") &&
                        !CombinedName.Contains("OBJECT_TYPE_Surface_Surface_Well_Cover"))
                    {
                        float tmp = Scale.Z;
                        Scale.Z = Scale.X;
                        Scale.X = tmp;
                        Scale *= 0.5f;
                    }

                    UE_LOG(LogTemp, Log, TEXT("CombinedName:%s, Len:%f, Width:%f"), *CombinedName, len, wid);
                    NewItem->SetActorScale3D(Scale);
                }
                else if (CombinedName.Contains("OBJECT_TYPE_Obstacle") ||
                         CombinedName.Contains("OBJECT_TYPE_ChargingPile") ||
                         CombinedName.Contains("OBJECT_TYPE_Tree") || CombinedName.Contains("OBJECT_TYPE_Building") ||
                         CombinedName.Contains("OBJECT_TYPE_BusStation") ||
                         CombinedName.Contains("OBJECT_TYPE_SpeedBump"))
                {
                    if (bModifyAxis)
                    {
                        float tmp = Scale.Y;
                        Scale.Y = Scale.X;
                        Scale.X = tmp;
                    }

                    if (bSortLengthWidth)
                    {
                        float tmp = Scale.Y;
                        Scale.Y = Scale.X;
                        Scale.X = tmp;
                    }
                    if (EnableModifyMapObjScale)
                        NewItem->SetActorScale3D(Scale);
                }
            }
        }

        if (OBJECT_TYPE_CrossWalk == obj->getObjectType())
        {
            OBJECT_SUB_TYPE subType = obj->getObjectSubType();
            // 人行横道线
            obj_rotator.Yaw -= 90.f;
            DrawCrossWalkLine(obj_location, obj_rotator, wid * 100.f, len * 100.f, false, false);
        }
        else if (RoadSign_Crosswalk_with_Left_and_Right_Side == obj->getObjectSubType())
        {
            OBJECT_SUB_TYPE subType = obj->getObjectSubType();
            // 双向带箭头人行横道线
            //  obj_rotator.Yaw -= 90.f;
            DrawCrossWalkLine(obj_location, obj_rotator, wid * 100.f, len * 100.f, true, true);
        }
        else if (CombinedName.Contains(TEXT("OBJECT_TYPE_RoadSign")))
        {
            if (RoadSign_White_Semicircle_Line == obj->getObjectSubType())
            {
                TArray<FVector> _leftPoints, _rightPoints;
                GetObjRangeGeoms(obj, _leftPoints, _rightPoints);
                DrawSemicircleLine(_leftPoints, _rightPoints);
            }
            else if (RoadSign_Road_Guide_Lane_Line == obj->getObjectSubType())
            {
                TArray<FVector> _leftPoints, _rightPoints;
                GetObjRangeGeoms(obj, _leftPoints, _rightPoints);
                DrawDirectionLane(_leftPoints, _rightPoints, false);
            }
            else if (RoadSign_Variable_Direction_Lane_Line == obj->getObjectSubType())
            {
                TArray<FVector> _leftPoints, _rightPoints;
                GetObjRangeGeoms(obj, _leftPoints, _rightPoints);
                DrawDirectionLane(_leftPoints, _rightPoints, true);
            }
            else if (RoadSign_Longitudinal_Dec == obj->getObjectSubType())
            {
                TArray<FVector> _leftPoints, _rightPoints;
                GetObjRangeGeoms(obj, _leftPoints, _rightPoints);
                DrawLongitudinalDecelerationLine(_leftPoints, _rightPoints);
            }
            else if (RoadSign_Lateral_Dec == obj->getObjectSubType())
            {
                TArray<FVector> _leftPoints, _rightPoints;
                GetObjRangeGeoms(obj, _leftPoints, _rightPoints);
                DrawLateralDecelerationLine(_leftPoints, _rightPoints);
            }
            else if (RoadSign_Intersection_Guide_Line == obj->getObjectSubType())
            {
                PointVec MapPoints;
                static_cast<const hadmap::txLineCurve*>(obj->getGeom(0)->getGeometry())->getPoints(MapPoints);
                TArray<FVector> LocPoints;
                for (hadmap::txPoint elem : MapPoints)
                {
                    hadmap::txPoint pos1 = SHadmap->getTxObjectPos(obj);
                    pos1.x = elem.x;
                    pos1.y = elem.y;
                    pos1.z = elem.z;

                    LonLatToLocal(pos1.x, pos1.y, pos1.z);
                    FVector Location = FVector(pos1.x, pos1.y, pos1.z + 2);
                    LocPoints.Add(Location);
                }
                FVector V1, V2, V3, V4;
                if (LocPoints.Num() >= 4)
                {
                    DrawGuideLine(LocPoints[0], LocPoints[1], LocPoints[2], LocPoints[3]);
                }
            }
            else if (RoadSign_Non_Motor_Vehicle_Area == obj->getObjectSubType())
            {
                DrawNonArea(obj_location, obj_rotator, wid * 100.f, len * 100.f);
            }
            else
            {
                // 交通标示线对象

                FString pLinePath =
                    FString(TEXT("Blueprint'/AutoRoad/traffice_props/BP/BP_ArrowActor.BP_ArrowActor_C'"));
                TSubclassOf<AArrowActor> item = LoadClass<AArrowActor>(this, *pLinePath);
                if (!item)
                {
                    UE_LOG(LogTemp, Log, TEXT("BP_ArrowActor Load Failed"));
                    continue;
                }
                AArrowActor* arrow = GetWorld()->SpawnActor<AArrowActor>(item, obj_location, obj_rotator);
                arrow->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
                arrow->SetActorScale3D(FVector(len / 2, wid / 2, 1));
                arrow->FuncUpdateArrowType(ObjectSubType);
                // UI
                arrow->FuncSetVisibleArrowType(false);

                FTransform Res =
                    GetRoadTrnasfrom(arrow->GetActorLocation(), arrow->GetActorRotation(), arrow->GetActorScale3D());
                Res.SetScale3D(arrow->GetActorScale3D());
                arrow->SetActorTransform(Res);
            }
        }
        else if (CombinedName.Contains(TEXT("OBJECT_TYPE_ParkingSpace")))
        {
            bool bBrokenLine = (Parking_Space_002 == obj->getObjectSubType());

            TArray<FVector> Vertices;
            TArray<FVector> Park_Vertices;
            TArray<int32> Park_Triangles;
            TArray<FVector> Park_Normals;
            TArray<FVector2D> Park_UV0;

            OBJECT_COLOR ObjColorType = (OBJECT_COLOR) obj->getGeom()->getColor();
            FColor Color;
            switch (ObjColorType)
            {
                case OBJECT_COLOR::OBJECT_COLOR_Red:
                    Color = FColor::FromHex(TEXT("FC2D2D"));
                    break;
                case OBJECT_COLOR::OBJECT_COLOR_White:
                    Color = FColor::FromHex(TEXT("F1F1F1"));
                    break;
                case OBJECT_COLOR::OBJECT_COLOR_Yellow:
                    Color = FColor::FromHex(TEXT("F1EA15"));
                    break;
                case OBJECT_COLOR::OBJECT_COLOR_Blue:
                    Color = FColor::FromHex(TEXT("0DC2EC"));
                    break;
                case OBJECT_COLOR::OBJECT_COLOR_Green:
                    Color = Color = FColor::FromHex(TEXT("00D82C"));
                    break;
                case OBJECT_COLOR::OBJECT_COLOR_Black:
                    Color = FColor::Black;
                    break;
                case OBJECT_COLOR::OBJECT_COLOR_Unknown:
                    Color = FColor::FromHex(TEXT("F1F1F1"));
                    break;
                default:
                    Color = FColor::FromHex(TEXT("F1F1F1"));
                    break;
            }
            FLinearColor LinearColor(Color);
            hadmap::PointVec points;
            float LineWidth = obj->getOdData().markWidth * 100;

            for (size_t i = 0; i < obj->getGeomSize(); i++)
            {
                static_cast<const hadmap::txLineCurve*>(obj->getGeom(i)->getGeometry())->getPoints(points);
                for (hadmap::txPoint elem : points)
                {
                    hadmap::txPoint pos1 = SHadmap->getTxObjectPos(obj);
                    pos1.x = elem.x;
                    pos1.y = elem.y;
                    pos1.z = elem.z;

                    LonLatToLocal(pos1.x, pos1.y, pos1.z);
                    FVector location = FVector(pos1.x, pos1.y, pos1.z + 2);
                    Vertices.Add(location);
                }

                for (int32 j = 0; j < Vertices.Num() - 3; j = i + 4)
                {
                    ConvertParkRenderData(Vertices[j + 0], Vertices[j + 1], Vertices[j + 2], Vertices[j + 3],
                        Park_Vertices, Park_Triangles, Park_Normals, Park_UV0, LineWidth, bBrokenLine);
                }
            }
            UMaterialInstanceDynamic* MID = CreateMaterial(MatRoadParking);
            if (MID)
            {
                MID->SetVectorParameterValue(FName(TEXT("Color")), LinearColor);
            }
            // CreateProcedualMesh(Park_Vertices, Park_Triangles, Park_Normals, Park_UV0, MID, false, 0, 1, 0, true);
            CreateStaticMesh(Park_Vertices, Park_Triangles, Park_Normals, Park_UV0, MID, false, 0, 0, 0);
        }
        else if (CombinedName == FString("OBJECT_TYPE_RoadSign_RoadSign_BusOnly"))
        {
            // 公交专用车道线
            std::map<std::string, std::string> userData;
            SHadmap->getTxObjectUserData(obj, userData);
            if (userData.size() > 0)
            {
                std::string obj_road_id = userData["roadId"];
                std::string obj_lane_id = userData["laneId"];
                GenerateObject(Handle, obj_road_id, obj_lane_id, TEXT("RoadSign_BusOnly"), obj_location, len * 4,
                    wid * 2, FVector(0, -150.f, 2), 2);
                GenerateObject(Handle, obj_road_id, obj_lane_id, TEXT("RoadSign_BusOnly"), obj_location, len * 4,
                    wid * 2, FVector(0, 150.f, 2), 2);
            }
        }
        else if (CombinedName.Contains(TEXT("OBJECT_TYPE_RoadSign")) ||
                 CombinedName.Contains(TEXT("OBJECT_TYPE_Arrow")) || CombinedName.Contains(TEXT("OBJECT_TYPE_Stop")) ||
                 CombinedName.Contains(TEXT("OBJECT_TYPE_ParkingSpace")))
        {
            // 禁止标线
            FString pLinePath = FString(TEXT("Blueprint'/AutoRoad/traffice_props/BP/BP_ArrowActor.BP_ArrowActor_C'"));
            TSubclassOf<AArrowActor> item = LoadClass<AArrowActor>(this, *pLinePath);
            if (!item)
            {
                UE_LOG(LogTemp, Log, TEXT("BP_BannerActor Load Failed"));
                continue;
            }
            AArrowActor* arrow = GetWorld()->SpawnActor<AArrowActor>(item, obj_location, obj_rotator);
            arrow->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
            arrow->SetActorScale3D(FVector(len / 2, wid / 2, 1));    // 材质大小200*200，所以此处/2
            arrow->FuncUpdateArrowType(ObjectSubType);
            // UI
            arrow->FuncSetVisibleArrowType(false);

            FTransform Res =
                GetRoadTrnasfrom(arrow->GetActorLocation(), arrow->GetActorRotation(), arrow->GetActorScale3D());
            Res.SetScale3D(arrow->GetActorScale3D());
            arrow->SetActorTransform(Res);
        }
        else if (OBJECT_TYPE_Tunnel == obj->getObjectType())
        {
            GenearateTunnel(obj);
        }
        else if (OBJECT_TYPE_Custom == obj->getObjectType())
        {
            UStaticMesh* RuntimeMesh = MapModelComponent->GetModelByName(UTF8_TO_TCHAR(obj->getName().c_str()));
            if (RuntimeMesh)
            {
                AStaticMeshActor* RuntimeMeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                    AStaticMeshActor::StaticClass(), obj_location, obj_rotator);
                RuntimeMeshActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
                RuntimeMeshActor->GetStaticMeshComponent()->SetStaticMesh(RuntimeMesh);
            }
        }
        else if (!UnknowObjectHide)
        {
            // 其他所有

            FString pLinePath = FString(TEXT("Blueprint'/AutoRoad/traffice_props/BP/BP_RoadObject.BP_RoadObject_C'"));
            TSubclassOf<ARoadObjectActor> item = LoadClass<ARoadObjectActor>(this, *pLinePath);
            if (!item)
            {
                UE_LOG(LogTemp, Log, TEXT("BP_RoadObject Load Failed"));
                continue;
            }
            ARoadObjectActor* arrow = GetWorld()->SpawnActor<ARoadObjectActor>(item, obj_location, obj_rotator);
            arrow->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
            arrow->SetActorScale3D(FVector(0.01, 0.01, 0.01) * 100);

            arrow->UpdateTexture(
                FString::FromInt((int) (obj->getObjectType())), FString(UTF8_TO_TCHAR(obj->getName().c_str())));
            arrow->SetVisibleObjectContent(!UnknowObjectHide);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("DrawMap obj finish"));
    RegisterAllComponents();
}

void AMapGeneratedActor::GenerateRoad(hadmap::txMapHandle* pHandle, const std::string& filename, double x_reference,
    double y_reference, double z_reference, bool gcj02, bool fix_connect, double road_expand_right,
    double road_expand_left, double minLen)
{
    // Clear old cache
    scene.clear();
    bdl_bdldsL.clear();
    bdl_bdldsR.clear();
    scene_curb.clear();
    section_map_curb.clear();
    lane_bdids.Empty();

    // for get lane type, road mark type, object type
    Handle = pHandle;

    UE_LOG(LogTemp, Log, TEXT("GenerateRoad Begin"));

    DrawRoadDetails(lane_bdids);
    DrawMarkDetails(lane_bdids);
    DrawJunctionDetails();
    DrawObjDetails();

    UE_LOG(LogTemp, Log, TEXT("DrawMap finish"));
}

// 根据材质类型创建动态材质
UMaterialInstanceDynamic* AMapGeneratedActor::CreateMaterial(const FName& MaterialName)
{
    FRoadObjectMaterialConfig* TargetConfig =
        RoadObjectMaterialConfig->FindRow<FRoadObjectMaterialConfig>(MaterialName, TEXT("UsingMaterial"), false);

    if (!TargetConfig)
    {
        // UE_LOG(LogTemp, Log, TEXT("%s not in table RoadObjectMaterialConfig"), *MaterialName.ToString());
        TargetConfig = RoadObjectMaterialConfig->FindRow<FRoadObjectMaterialConfig>(MatRoad, TEXT("UsingMaterial"));
    }

    return UMaterialInstanceDynamic::Create(TargetConfig->UsingMaterial, this);
}

UMaterialInterface* AMapGeneratedActor::CreateStaticMaterial(const FName& MaterialName)
{
    FRoadObjectMaterialConfig* TargetConfig =
        RoadObjectMaterialConfig->FindRow<FRoadObjectMaterialConfig>(MaterialName, TEXT("UsingMaterial"), false);

    if (!TargetConfig)
    {
        // UE_LOG(LogTemp, Log, TEXT("%s not in table RoadObjectMaterialConfig"), *MaterialName.ToString());
        TargetConfig = RoadObjectMaterialConfig->FindRow<FRoadObjectMaterialConfig>(MatRoad, TEXT("UsingMaterial"));
    }

    return TargetConfig->UsingMaterial;
}

TSubclassOf<AActor> AMapGeneratedActor::GetActor(const FString& ActorName)
{
    FString Path;
    const FString IniFile = FPaths::ProjectDir() + TEXT("/Config/DefaultAutoRoad.ini");

    GConfig->GetString(TEXT("/TypeList"), *ActorName, Path, IniFile);
    int32 Pos = INDEX_NONE;
    Path.FindLastChar(**FString(TEXT("\'")), Pos);
    Path.InsertAt(Pos, TEXT("_C"));
    TSubclassOf<AActor> Resource = LoadClass<AActor>(NULL, *Path);
    if (!Resource.Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("Type: %s Can not load BP asset!"), *Path);
    }
    return Resource;
}

// 根据object名字获得StaticMesh
UStaticMesh* AMapGeneratedActor::GetStaticMesh(const FName& StaticMeshName)
{
    FRoadObjectStaticMeshConfig* TargetConfig =
        RoadObjectStaticMeshConfig->FindRow<FRoadObjectStaticMeshConfig>(StaticMeshName, TEXT("UsingMesh"), false);
    if (TargetConfig)
        return TargetConfig->UsingMesh;
    else
    {
        UE_LOG(LogTemp, Log, TEXT("%s not in table RoadObjectStaticMeshConfig"), *StaticMeshName.ToString());
        return NULL;
    }
}

void AppendTriangle(FMeshDescriptionBuilder& meshDescBuilder, TArray<FVertexID> vertexIDs, FPolygonGroupID polygonGroup,
    const TArray<VertexInfo>& vertex)
{
    TArray<FVertexInstanceID> vertexInsts;    // 三角形中的每个顶点
    for (int i = 0; i < 3; i++)
    {
        FVertexInstanceID instance = meshDescBuilder.AppendInstance(vertexIDs[vertex[i].ID]);
        meshDescBuilder.SetInstanceNormal(instance, vertex[i].InstanceNormal);
        meshDescBuilder.SetInstanceUV(instance, vertex[i].InstanceUV, 0);
        meshDescBuilder.SetInstanceColor(instance, FVector4(1.0f, 1.0f, 1.0f, 1.0f));
        vertexInsts.Add(instance);
    }
    // 增加此三角形
    meshDescBuilder.AppendTriangle(vertexInsts[0], vertexInsts[1], vertexInsts[2], polygonGroup);
};

void InitMeshDesciption(FMeshDescription& meshDesc, const TArray<FVector>& Vertices, const TArray<int32>& Triangles,
    const TArray<FVector>& Normals, const TArray<FVector2D>& UV0)
{
    FStaticMeshAttributes Attributes(meshDesc);

    Attributes.Register();

    // 为 MeshDescription 创建一个 MeshDescriptionBuilder 协助构建数据
    FMeshDescriptionBuilder meshDescBuilder;
    meshDescBuilder.SetMeshDescription(&meshDesc);
    meshDescBuilder.EnablePolyGroups();
    meshDescBuilder.SetNumUVLayers(1);

    TArray<FVertexID> vertexIDs;
    for (auto Vertice : Vertices)
    {
        vertexIDs.Add(meshDescBuilder.AppendVertex(Vertice));
    }

    // 使用 MeshDescriptionBuilder 来构建数据：
    {
        // 分配一个 polygon group
        FPolygonGroupID polygonGroup = meshDescBuilder.AppendPolygonGroup();
        // int32 Length = vertexIDs.Num() - 0 < Triangles.Num() / 3 ? vertexIDs.Num() - 0 : Triangles.Num() / 3;
        int32 Length = Triangles.Num() / 3;

        for (int32 i = 0; i < Length; i = i + 1)
        {
            AppendTriangle(meshDescBuilder, vertexIDs, polygonGroup,
                {VertexInfo(Triangles[i * 3 + 0], Normals[Triangles[i * 3 + 0]], UV0[Triangles[i * 3 + 0]]),
                    VertexInfo(Triangles[i * 3 + 1], Normals[Triangles[i * 3 + 1]], UV0[Triangles[i * 3 + 1]]),
                    VertexInfo(Triangles[i * 3 + 2], Normals[Triangles[i * 3 + 2]], UV0[Triangles[i * 3 + 2]])});
        }
    }
}

// 根据colorname 获得LinearColor
FLinearColor AMapGeneratedActor::GetLinerColor(const FName& ColorName)
{
    FRoadMarkColorConfig* TargetConfig =
        RoadMarkColorConfig->FindRow<FRoadMarkColorConfig>(ColorName, TEXT("UsingColor"), false);
    if (!TargetConfig)
    {
        UE_LOG(LogTemp, Log, TEXT("%s not in table RoadMarkColorConfig"), *ColorName.ToString());
        TargetConfig = RoadMarkColorConfig->FindRow<FRoadMarkColorConfig>(FName("standard"), TEXT("UsingColor"));
    }
    return TargetConfig->UsingColor;
}

// 根据数据创建StaticMesh
UStaticMesh* AMapGeneratedActor::CreateStaticMesh(const TArray<FVector>& Vertices, const TArray<int32>& Triangles,
    const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, UMaterialInterface* UsingMaterial, bool CastShadow,
    uint8 bCastDynamicShadow, uint8 bCastStaticShadow, float RelativeZ, bool bCreateCollision, bool bLod,
    bool bNewActor)
{
    if (Vertices.Num() == 0)
        return nullptr;

    UStaticMesh* staticMesh = NewObject<UStaticMesh>(this);
    staticMesh->GetStaticMaterials().Add(FStaticMaterial());    // 至少添加一个材质
    staticMesh->GetStaticMaterials()[0].UVChannelData.bInitialized = true;
    bool Res = staticMesh->GetIsBuiltAtRuntime();
    staticMesh->SetIsBuiltAtRuntime(true);

    UStaticMesh::FBuildMeshDescriptionsParams mdParams;

    if (bCreateCollision)
    {
        mdParams.bAllowCpuAccess = true;
        staticMesh->bAllowCPUAccess = true;
    }

    FMeshDescription meshDesc;
    InitMeshDesciption(meshDesc, Vertices, Triangles, Normals, UV0);

    TArray<const FMeshDescription*> meshDescPtrs;
    meshDescPtrs.Emplace(&meshDesc);
    FMeshDescription meshDesc2;
    if (bLod)
    {
        TArray<FVector> Vertices2;
        TArray<int32> Triangles2;
        TArray<FVector> Normals2;
        TArray<FVector2D> UV02;
        int32 rad = 4;
        int32 recordNum = 0;
        for (int i = 0; i < Vertices.Num() - 1; i++)
        {
            if (i % rad == 0)
            {
                Vertices2.Add(Vertices[i]);
                Vertices2.Add(Vertices[i + 1]);

                Normals2.Add(Normals[i]);
                Normals2.Add(Normals[i + 1]);

                UV02.Add(UV0[i]);
                UV02.Add(UV0[i + 1]);
                recordNum += 2;
            }
        }
        Triangles2 = Triangles;
        Triangles2.SetNum(Triangles.Num() / 2 - 3);
        InitMeshDesciption(meshDesc2, Vertices2, Triangles2, Normals2, UV02);
        meshDescPtrs.Emplace(&meshDesc2);
    }

    // 构建！
    staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);

    for (int i = 0; i < staticMesh->GetRenderData()->LODResources.Num(); i++)
    {
        if (0 == staticMesh->GetRenderData()->LODResources[i].Sections.Num())
        {
            continue;
        }
        staticMesh->GetRenderData()->LODResources[i].Sections[0].MaterialIndex = 0;
        if (bCreateCollision)
        {
            staticMesh->GetBodySetup()->bDoubleSidedGeometry = true;
            staticMesh->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
            staticMesh->GetRenderData()->LODResources[i].Sections[0].bEnableCollision = true;
        }
    }

    int32 MaterialIndex = 0;
    staticMesh->GetStaticMaterials()[MaterialIndex].MaterialInterface = UsingMaterial;
    if (UsingMaterial != nullptr)
    {
        // Set the Material slot name to a good default one
        if (staticMesh->GetStaticMaterials()[MaterialIndex].MaterialSlotName == NAME_None)
        {
            staticMesh->GetStaticMaterials()[MaterialIndex].MaterialSlotName = UsingMaterial->GetFName();
        }

        {
            auto IsMaterialNameUnique = [staticMesh, MaterialIndex](const FName TestName)
            {
                for (int32 MatIndex = 0; MatIndex < staticMesh->GetStaticMaterials().Num(); ++MatIndex)
                {
                    if (MatIndex == MaterialIndex)
                    {
                        continue;
                    }
                    if (staticMesh->GetStaticMaterials()[MatIndex].ImportedMaterialSlotName == TestName)
                    {
                        return false;
                    }
                }
                return true;
            };

            int32 MatchNameCounter = 0;
            // Make sure the name is unique for imported material slot name
            bool bUniqueName = false;
            FString MaterialSlotName = UsingMaterial->GetName();
            while (!bUniqueName)
            {
                bUniqueName = true;
                if (!IsMaterialNameUnique(FName(*MaterialSlotName)))
                {
                    bUniqueName = false;
                    MatchNameCounter++;
                    MaterialSlotName = UsingMaterial->GetName() + TEXT("_") + FString::FromInt(MatchNameCounter);
                }
            }
            staticMesh->GetStaticMaterials()[MaterialIndex].ImportedMaterialSlotName = FName(*MaterialSlotName);
        }

        // Make sure adjacency information fit new material change
        TArray<bool> FixLODAdjacencyOption;
        FixLODAdjacencyOption.AddZeroed(staticMesh->GetNumLODs());
        bool bPromptUser = false;
        /*for (int32 LODIndex = 0; LODIndex < GetNumLODs(); ++LODIndex)
        {
            FixLODAdjacencyOption[LODIndex] = FixLODRequiresAdjacencyInformation(LODIndex);
            bPromptUser |= FixLODAdjacencyOption[LODIndex];
        }*/
    }

    if (!bNewActor)
    {
        // staticMesh->Built
        UStaticMeshComponent* SMC = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());

        // 将 StaticMesh 指定给 StaticMeshComponent组件
        SMC->SetStaticMesh(staticMesh);
        SMC->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        SMC->SetCollisionProfileName(FName(TEXT("RoadObject")));
        SMC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        SMC->SetMaterial(ElementIndex, UsingMaterial);

        SMC->CastShadow = CastShadow;
        SMC->bCastDynamicShadow = bCastDynamicShadow;
        SMC->bCastStaticShadow = bCastStaticShadow;
        SMC->AddRelativeLocation(FVector(0, 0, +RelativeZ));
    }

    return staticMesh;
}

// 根据数据创建ProcedualMesh
void AMapGeneratedActor::CreateProcedualMesh(const TArray<FVector>& Vertices, const TArray<int32>& Triangles,
    const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, UMaterialInterface* UsingMaterial, bool CastShadow,
    uint8 bCastDynamicShadow, uint8 bCastStaticShadow, float RelativeZ, bool bCreateCollision)
{
    if (Vertices.Num() == 0)
        return;
    // 创建ProceduralMeshComponent
    UProceduralMeshComponent* Mesh = NewObject<UProceduralMeshComponent>(this, UProceduralMeshComponent::StaticClass());
    Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

    // 启用复杂碰撞
    Mesh->SetCollisionProfileName(FName(TEXT("RoadObject")));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // 绘制
    Mesh->CreateMeshSection(ElementIndex, Vertices, Triangles, Normals, UV0, TArray<FColor>(),
        TArray<FProcMeshTangent>(), bCreateCollision);

    // 赋值材质
    Mesh->SetMaterial(ElementIndex, UsingMaterial);

    // 设置参数
    Mesh->CastShadow = CastShadow;
    Mesh->bCastDynamicShadow = bCastDynamicShadow;
    Mesh->bCastStaticShadow = bCastStaticShadow;
    Mesh->AddRelativeLocation(FVector(0, 0, +RelativeZ));

    // 加入数组
    RoadObjectMeshes.Add(Mesh);
}

void AMapGeneratedActor::GenerateObject(hadmap::txMapHandle* pHandle, const std::string RoadId,
    const std::string LaneId, const FName& MaterialName, const FVector Location, const double Length,
    const double Width, const FVector RelativeLocation, int interval)
{
    hadmap::txRoadPtr roadPtr;
    SHadmap->getRoad(std::stoull(RoadId), true, roadPtr);
    //    hadmap::getRoad(pHandle, std::stoull(RoadId), true, roadPtr);
    if (!roadPtr)
        return;
    TArray<FVector> Vertices;
    int Vertices_Length;

    TArray<FVector> spline_points;
    double obj_dis = 0;

    // check OBJ full of lane
    bool full_of_lane = false;
    if (roadPtr->getLength() <= Length + 5)    // 5:margin of error
        full_of_lane = true;

    for (auto section_ptr : roadPtr->getSections())
    {
        hadmap::sectionpkid secId = section_ptr->getId();
        hadmap::lanepkid laneId = std::stoll(LaneId);
        hadmap::txLanePtr lanePtr = roadPtr->getLane(secId, laneId);
        if (!lanePtr)
            continue;

        std::vector<hadmap::txLaneBoundaryPtr> bds;
        bds.push_back(lanePtr->getLeftBoundary());
        bds.push_back(lanePtr->getRightBoundary());

        Vertices_Length = Vertices.Num();
        int spline_points_length = spline_points.Num();

        for (size_t ii = 0; ii < bds.size(); ii++)
        {
            hadmap::txLaneBoundaryPtr lane = bds[ii];
            if (!full_of_lane)
            {
                // get lane start point
                hadmap::txPoint startPoint = static_cast<const hadmap::txLineCurve*>(lane->getGeometry())->getStart();
                // coord_trans_api::lonlat2local(startPoint.x, startPoint.y, startPoint.z, RefX, RefY, RefZ);
                LonLatToLocal(startPoint.x, startPoint.y, startPoint.z);
                /*startPoint.x *= 100;
                startPoint.y *= -100;
                startPoint.z *= 100;*/

                // distance between startPoint and OBJ's Location
                obj_dis = FVector::Dist(FVector(startPoint.x, startPoint.y, startPoint.z), Location) / 100;
            }

            hadmap::PointVec points;
            if (full_of_lane)
                // get all points
                static_cast<const hadmap::txLineCurve*>(lane->getGeometry())->getPoints(points);
            else
                // get points from startS to endS| startS: obj_dis,endS:obj_dis + Length
                static_cast<const hadmap::txLineCurve*>(lane->getGeometry())->cut(obj_dis, obj_dis + Length, points);

            auto coordtype = lane->getGeometry()->getCoordType();
            for (size_t i = 0, is = points.size(); i < is; i = i + interval)
            {
                auto& p = points[i];
                if (coordtype == hadmap::COORD_WGS84)
                {
                    /*if (gcj02)
                    {
                        WGS84ToGCJ02(p.x, p.y);
                    }*/
                    // coord_trans_api::lonlat2local(p.x, p.y, p.z, RefX, RefY, RefZ);
                    LonLatToLocal(p.x, p.y, p.z);
                }

                // FVector local = FVector(p.x * 100, -p.y * 100, p.z * 100);
                FVector local = FVector(p.x, p.y, p.z);
                if (local.ContainsNaN())
                    continue;

                Vertices.Add(local);
            }
        }
        size_t lane_ver_num = (Vertices.Num() - Vertices_Length) / 2;

        // add spline point
        for (int id = 0; id < lane_ver_num; id++)
        {
            FVector from = Vertices[Vertices_Length + id];
            FVector to = Vertices[Vertices_Length + id + lane_ver_num];
            spline_points.Add((from + to) / 2);
        }
    }

    // reset spline_points
    if (!full_of_lane)
    {
        FVector Off = Location - (Vertices[0] + Vertices[Vertices.Num() / 2]) / 2;
        for (int i = 0; i < spline_points.Num(); i++)
            spline_points[i] += Off;
    }

    // create spline & mesh
    if (spline_points.Num() > 2)
    {
        USplineComponent* Spline = NewObject<USplineComponent>(this, USplineComponent::StaticClass());
        Spline->SetupAttachment(RootComponent);
        Spline->RegisterComponent();
        Spline->SetSplineWorldPoints(spline_points);

        for (size_t i = 0; i < Spline->GetNumberOfSplinePoints() - 1; i++)
        {
            USplineMeshComponent* Mesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
            //    Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
            Mesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
            Mesh->SetStaticMesh(SplineStaticMesh);
            //    Mesh->SetupAttachment(SplineComponrnt);
            Mesh->SetMaterial(ElementIndex, CreateMaterial(MaterialName));
            FVector StartPos;
            FVector StartTangent;
            Spline->GetLocationAndTangentAtSplinePoint(i, StartPos, StartTangent, ESplineCoordinateSpace::World);
            FVector EndPos;
            FVector EndTangent;
            Spline->GetLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent, ESplineCoordinateSpace::World);
            Mesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
            Mesh->SetStartScale(FVector2D(Width, 1));
            Mesh->SetEndScale(FVector2D(Width, 1));
            float Yaw = (EndPos - StartPos).Rotation().Yaw;
            const FVector RelativeLocationRes = RelativeLocation.RotateAngleAxis(Yaw, FVector::UpVector);
            Mesh->AddRelativeLocation(RelativeLocationRes);
        }
    }
}

void AMapGeneratedActor::PointDilution(hadmap::PointVec& points)
{
    // return;
    if (points.size() > 2)
    {
        FVector LastDir;
        {
            FVector p1(points[points.size() - 1].x, points[points.size() - 1].y, points[points.size() - 1].z);
            FVector p2(points[points.size() - 2].x, points[points.size() - 2].y, points[points.size() - 2].z);
            LastDir = (p2 - p1).GetSafeNormal();
        }

        for (int32 i = points.size() - 2; i > 2; i--)
        {
            FVector p1(points[i - 0].x, points[i - 0].y, points[i - 0].z);
            FVector p2(points[i - 1].x, points[i - 1].y, points[i - 1].z);
            FVector CurrentDir = (p2 - p1).GetSafeNormal();
            float Degree = acos(CurrentDir | LastDir);
            float Angle = FMath::RadiansToDegrees(Degree);
            float DeltaAlt = fabs(p1.Z - p2.Z);
            LastDir = CurrentDir;
            if (Angle < AngleDeltaToRemovePoint && DeltaAlt < 0.1)
            {
                hadmap::PointVec::iterator it = points.begin() + i - 1;
                if (bRemovePoints)
                {
                    points.erase(it);
                }
            }
        }
    }
}

void AMapGeneratedActor::GenearateTunnel(hadmap::txObjectPtr Obj)
{
    if (!Obj)
    {
        return;
    }

    hadmap::txRoadPtr RoadPtr;
    hadmap::txRoadPtr Related_RoadPtr;
    TArray<hadmap::txRoadPtr> RoadPtrs;
    roadpkid RoadID = Obj->getRoadId();
    if (HasGeneratedTunnelRoadID.Contains(RoadID))
    {
        return;
    }
    int Res = getRoad(Handle, Obj->getRoadId(), true, RoadPtr);
    if (!RoadPtr)
    {
        return;
    }
    RoadPtrs.Add(RoadPtr);

    std::map<std::string, std::string> userData;
    std::map<std::string, std::string>::iterator iterData;
    SHadmap->getTxObjectUserData(Obj, userData);
    iterData = userData.find("relate_road");
    bool bRelateRoad = false;
    roadpkid RelateRoadID;
    hadmap::txObjectPtr RelateTunnel;

    if (iterData != userData.end())
    {
        bRelateRoad = true;
        RelateRoadID = std::stoull(iterData->second);
        getRoad(Handle, RelateRoadID, true, Related_RoadPtr);
        if (!Related_RoadPtr)
        {
            UE_LOG(LogTemp, Error, TEXT("Related_RoadP is null,  tunnel id %d"), Obj->getId());
            return;
        }
        RoadPtrs.Add(Related_RoadPtr);
        HasGeneratedTunnelRoadID.Add(RelateRoadID);

        hadmap::txObjects TunnelObjects;
        std::vector<hadmap::OBJECT_TYPE> types;
        types.push_back(OBJECT_TYPE_Tunnel);
        SHadmap->GetObjects(std::vector<hadmap::txLaneId>(), types, TunnelObjects);

        for (const auto& obj : TunnelObjects)
        {
            int testid = obj->getRoadId();
            if (obj->getRoadId() == RelateRoadID)
            {
                RelateTunnel = obj;
                break;
            }
        }
        if (!RelateTunnel)
        {
            UE_LOG(LogTemp, Error, TEXT("RelateTunnel is null, tunnel id %d"), Obj->getId());
            return;
        }
    }

    TArray<FVector> TunnelVertices;
    TArray<int32> TunnelTriangles;
    TArray<FVector> TunnelNormals;
    TArray<FVector2D> TunnelUV0s;
    double S = 0.0, T = 0.0;
    double Len = 0.0, Width = 0.0, Height = 0.0;

    hadmap::txPoint TunnelPos = Obj->getPos();
    LonLatToLocal(TunnelPos.x, TunnelPos.y, TunnelPos.z);
    FVector TunnelLocation((float) TunnelPos.x, (float) TunnelPos.y, (float) TunnelPos.z);

    TArray<TPair<double, double>> RoadRangeList;    // ST坐标 起点的s ，length

    if (RelateTunnel) {
        RelateTunnel->getST(S, T);
        RelateTunnel->getLWH(Len, Width, Height);
    }
   
    RoadRangeList.Add(TPair<double, double>(S, Len));

    Obj->getST(S, T);
    Obj->getLWH(Len, Width, Height);
    RoadRangeList.Add(TPair<double, double>(S, Len));

    TArray<FVector> Veritics_Left;
    TArray<FVector> Veritics_Right;
    int32 MaxLaneNum = 0;

    bool bOutWidth = false;

    for (auto Road : RoadPtrs)
    {
        float Start = 0.f, End = 0.f;
        TPair<double, double> RoadRange = RoadRangeList.Pop();
        S = RoadRange.Key;
        Len = RoadRange.Value;
        int32 CurrentMaxLane = 0;
        for (auto Section_ptr : Road->getSections())
        {
            if (Len < 0.f)
            {
                break;
            }
            float SectionLength = Section_ptr->getLength();
            if (SectionLength < S)    // 起始点在该section之后
            {
                S -= SectionLength;
                continue;
            }
            else if (SectionLength > (S + Len))    // 该section包含隧道
            {
                Start = S;
                End = S + Len;
                S -= SectionLength;
                Len -= SectionLength;
            }
            else    // 该section比隧道短，后续section也是隧道
            {
                Start = S;
                End = SectionLength;
                S = 0;
                Len -= End - Start;
            }

            if (CurrentMaxLane < Section_ptr->getLanes().size())
            {
                CurrentMaxLane = Section_ptr->getLanes().size();
            }

            txLaneBoundaries AllBd = Section_ptr->getBoundariesByRange(Start, End);
            if (AllBd.size() < 2)
            {
                continue;
            }
            txLaneBoundaries Bds;
            if (bRelateRoad)
            {
                Bds.push_back(AllBd.back());
            }
            else
            {
                Bds.push_back(AllBd.front());
                Bds.push_back(AllBd.back());
            }

            for (int32 i = 0; i < Bds.size(); i++)
            {
                hadmap::PointVec Points;

                const hadmap::txLineCurve* LineCurve = static_cast<const hadmap::txLineCurve*>(Bds[i]->getGeometry());
                if (!LineCurve)
                    continue;
                if (LineCurve->empty())
                    continue;

                LineCurve->getPoints(Points);
                for (int32 ii = 0; ii < Points.size(); ii++)
                {
                    LonLatToLocal(Points[ii].x, Points[ii].y, Points[ii].z);
                }

                PointDilution(Points);
                TArray<FVector>* pVeritics;
                if (bRelateRoad)
                {
                    if (RelateRoadID == Road->getId())
                    {
                        pVeritics = &Veritics_Left;
                    }
                    else
                    {
                        pVeritics = &Veritics_Right;
                    }
                }
                else
                {
                    if (i == 0)
                    {
                        pVeritics = &Veritics_Left;
                    }
                    else
                    {
                        pVeritics = &Veritics_Right;
                    }
                }

                for (auto p : Points)
                {
                    pVeritics->Add(FVector(p.x, p.y, p.z) - TunnelLocation);
                }
            }
        }

        MaxLaneNum += CurrentMaxLane;
        if (MaxLaneNum > 4)
        {
            bOutWidth = true;
        }
    }

    if (bRelateRoad)
    {
        FVector tmp;
        for (int32 i = 0; i < Veritics_Left.Num() / 2; i++)
        {
            tmp = Veritics_Left[Veritics_Left.Num() - i - 1];
            Veritics_Left[Veritics_Left.Num() - i - 1] = Veritics_Left[i];
            Veritics_Left[i] = tmp;
        }
    }

    int32 LengthDelta = Veritics_Left.Num() - Veritics_Right.Num();
    FVector FillPoint = LengthDelta < 0 ? Veritics_Left.Last() : Veritics_Right.Last();
    while (LengthDelta != 0)
    {
        if (LengthDelta < 0)    // Left < Right
        {
            Veritics_Left.Add(FillPoint);
            LengthDelta++;
        }
        else    // Left > Right
        {
            Veritics_Right.Add(FillPoint);
            LengthDelta--;
        }
    }

    TunnelVertices.Append(Veritics_Left);
    TunnelVertices.Append(Veritics_Right);

    TArray<FVector> CirclePoints_First;
    TArray<FVector> CirclePoints_End;
    int32 LanePointNum = TunnelVertices.Num() / 2;
    int32 CircleNum = bOutWidth ? 5 : 20;

    TArray<TArray<FVector>> CirclePoints;
    FVector2D RecordUV;

    FVector LastDir1, LastDir2;
    for (int32 i = 0; i < LanePointNum; i++)
    {
        TArray<FVector> CirclePointList;
        TArray<FVector2D> CircleUVs;
        FVector StartPoint = TunnelVertices[i];
        FVector EndPoint = TunnelVertices[i + LanePointNum];
        FVector Delta = EndPoint - StartPoint;
        float TunnelHeight = StartPoint.Z + 500.f;
        float LinePercent = bOutWidth ? 1.f : 0.3f;
        RecordUV.Y = 0.f;
        for (int j = 0; j <= CircleNum; j++)
        {
            float X = 0.f, Y = 0.f, Z = 0.f;
            if (j == 0)
            {
                X = StartPoint.X;
                Y = StartPoint.Y;
                Z = StartPoint.Z;
            }
            else if (j == 1)
            {
                X = StartPoint.X;
                Y = StartPoint.Y;
                Z = LinePercent * TunnelHeight;
            }
            else if (j == CircleNum)
            {
                X = EndPoint.X;
                Y = EndPoint.Y;
                Z = EndPoint.Z;
            }
            else if (j == CircleNum - 1)
            {
                X = EndPoint.X;
                Y = EndPoint.Y;
                Z = LinePercent * TunnelHeight;
            }
            else
            {
                float EllipseNum = CircleNum - 2;    // 减去起始点
                float thelta = (PI / EllipseNum) * (float) (j - 1);

                // 椭圆的A为Delta中的xy，B为Circle中的剩余高度
                X = (StartPoint.X + EndPoint.X) * 0.5f + cos(PI - thelta) * Delta.X * 0.5f;
                Y = (StartPoint.Y + EndPoint.Y) * 0.5f + cos(PI - thelta) * Delta.Y * 0.5f;
                Z = (1.f - LinePercent) * sin(PI - thelta) * TunnelHeight + LinePercent * TunnelHeight;
            }

            FVector NewPoint(X, Y, Z);
            CirclePointList.Add(NewPoint);
            FVector2D UV;
            FVector Normal;
            FVector Dir1, Dir2;

            if (i == 0)
            {
                UV.X = 0.f;
                Dir1 = TunnelVertices.Num() > 1 ? (TunnelVertices[1] - TunnelVertices[0]).GetSafeNormal()
                                                : FVector::UpVector;
            }
            else if (i == (LanePointNum - 1))
            {
                UV.X = RecordUV.X + (TunnelVertices[i - 1] - TunnelVertices[i]).Size() / 100.f;
                Dir1 = FVector(1, 0, 0);    // TunnelVertices.Num() > 1 ? (TunnelVertices[1] -
                                            // TunnelVertices[0]).GetSafeNormal() : FVector::UpVector;
            }
            else if (i > 0)
            {
                UV.X = RecordUV.X + (TunnelVertices[i] - TunnelVertices[i - 1]).Size() / 100.f;

                Dir1 = (TunnelVertices[i] - TunnelVertices[i - 1]).GetSafeNormal();
                if (Dir1.IsNearlyZero())
                {
                    Dir1 = LastDir1;
                }
            }
            LastDir1 = Dir1;

            int32 CurrentPointIndex = CirclePointList.Num() - 1;
            if (CurrentPointIndex > 0)
            {
                UV.Y = RecordUV.Y +
                       (CirclePointList[CurrentPointIndex] - CirclePointList[CurrentPointIndex - 1]).Size() / 100.f;
                Dir2 =
                    CirclePointList.Num() > 1
                        ? (CirclePointList[CurrentPointIndex] - CirclePointList[CurrentPointIndex - 1]).GetSafeNormal()
                        : FVector::UpVector;

                if (Dir2.IsNearlyZero())
                {
                    Dir2 = LastDir2;
                }
            }
            else
            {
                UV.Y = 0.f;
                Dir2 = CirclePointList.Num() > 1 ? (CirclePointList[1] - CirclePointList[0]).GetSafeNormal()
                                                 : FVector::UpVector;
            }
            LastDir2 = Dir2;
            TunnelUV0s.Add(UV);
            RecordUV.Y = UV.Y;
            TunnelNormals.Add(FVector::CrossProduct(Dir1, Dir2).GetSafeNormal());
            // TunnelNormals.Add(FVector::UpVector);
        }
        RecordUV.X = TunnelUV0s.Last().X;
        CirclePoints.Add(CirclePointList);
    }
    TunnelVertices.Empty();

    for (int32 i = 0; i < CirclePoints.Num(); i++)
    {
        int32 VerticesStart = TunnelVertices.Num();
        TunnelVertices.Append(CirclePoints[i]);
        if (i < CirclePoints.Num() - 1)
        {
            for (int32 j = 0; j < CirclePoints[i].Num() - 1; j++)
            {
                TunnelTriangles.Add(VerticesStart + j);
                TunnelTriangles.Add(VerticesStart + j + 1);
                TunnelTriangles.Add(VerticesStart + j + CirclePoints[i].Num());

                TunnelTriangles.Add(VerticesStart + j + 1);
                TunnelTriangles.Add(VerticesStart + j + CirclePoints[i].Num() + 1);
                TunnelTriangles.Add(VerticesStart + j + CirclePoints[i].Num());
            }
        }
    }

    UStaticMesh* TunnelMesh = CreateStaticMesh(TunnelVertices, TunnelTriangles, TunnelNormals, TunnelUV0s,
        CreateStaticMaterial(MatTunnel), true, 1, 0, 0.02, true, false, true);
    if (TunnelMesh)
    {
        FString TunnelPath = FString(TEXT("Blueprint'/Game/Blueprints/MapElement/BP_Tunnel.BP_Tunnel_C'"));
        TSubclassOf<AActor> TunnelClass = LoadClass<AActor>(this, *TunnelPath);
        if (!TunnelClass)
        {
            UE_LOG(LogTemp, Error, TEXT("BP_TunnelObject Load Failed"));
            return;
        }

        AStaticMeshActor* TunnelActor =
            GetWorld()->SpawnActor<AStaticMeshActor>(TunnelClass, TunnelLocation, FRotator(0, 0, 0));
        TunnelActor->Tags.Add(TEXT("EnvOcclusion"));
        TunnelActor->Tags.Add(TEXT("Visibility"));
        TunnelActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        TunnelActor->GetStaticMeshComponent()->SetStaticMesh(TunnelMesh);
    }
}

/// 求两个向量交点算法

int AMapGeneratedActor::dblcmp(double a, double b)
{
    if (fabs(a - b) <= 1E-6)
        return 0;
    if (a > b)
        return 1;
    else
        return -1;
}
// 点积判点是否在线段上
double AMapGeneratedActor::dot(double x1, double y1, double x2, double y2)
{
    return x1 * x2 + y1 * y2;
}

// 求a点是不是在线段bc上，>0不在，=0与端点重合，<0在
int AMapGeneratedActor::point_on_line(FVector a, FVector b, FVector c)
{
    return dblcmp(dot(b.X - a.X, b.Y - a.Y, c.X - a.X, c.Y - a.Y), 0);
}

double AMapGeneratedActor::cross(double x1, double y1, double x2, double y2)
{
    return x1 * y2 - x2 * y1;
}

// ab与ac的叉积
double AMapGeneratedActor::ab_cross_ac(FVector a, FVector b, FVector c)
{
    return cross(b.X - a.X, b.Y - a.Y, c.X - a.X, c.Y - a.Y);
}

// 求ab是否与cd相交，交点为p。1规范相交，0交点是一线段的端点，-1不相交
int AMapGeneratedActor::ab_cross_cd(FVector a, FVector b, FVector c, FVector d)
{
    double s1, s2, s3, s4;
    int d1, d2, d3, d4;

    d1 = dblcmp(s1 = ab_cross_ac(a, b, c), 0);
    d2 = dblcmp(s2 = ab_cross_ac(a, b, d), 0);
    d3 = dblcmp(s3 = ab_cross_ac(c, d, a), 0);
    d4 = dblcmp(s4 = ab_cross_ac(c, d, b), 0);

    // 如果规范相交则求交点
    if ((d1 ^ d2) == -2 && (d3 ^ d4) == -2)
    {
        return 1;
    }

    // 如果不规范相交
    if (d1 == 0 && point_on_line(c, a, b) <= 0)
    {
        return 0;
    }
    if (d2 == 0 && point_on_line(d, a, b) <= 0)
    {
        return 0;
    }
    if (d3 == 0 && point_on_line(a, c, d) <= 0)
    {
        return 0;
    }
    if (d4 == 0 && point_on_line(b, c, d) <= 0)
    {
        return 0;
    }
    // 如果不相交
    return -1;
}
/// 求两个向量交点 END

// 为每一个Road创建一个Spline
void AMapGeneratedActor::CreateSpline(
    hadmap::txMapHandle* pHandle, double x_reference, double y_reference, double z_reference)
{
    hadmap::txRoads _roads;
    hadmap::getRoads(pHandle, true, _roads);
    for (auto& road_ptr : _roads)
    {
        TArray<FVector> spline_points;
        for (auto section_ptr : road_ptr->getSections())
        {
            hadmap::txLaneBoundaries bds = section_ptr->getBoundaries();
            if (bds.empty())
            {
                continue;
            }
            // create spline

            size_t n0 = 0, i0 = 0;
            size_t idx = bds.size() - 1;
            hadmap::txLaneBoundaryPtr lane_start = bds[0];
            hadmap::txLaneBoundaryPtr lane_last = bds[idx];
            hadmap::PointVec points_start;
            hadmap::PointVec points_last;
            static_cast<const hadmap::txLineCurve*>(lane_start->getGeometry())->getPoints(points_start);
            static_cast<const hadmap::txLineCurve*>(lane_last->getGeometry())->getPoints(points_last);

            // auto coordtype = lane_start->getGeometry()->getCoordType();

            for (size_t i = pillar_interval; i < points_start.size(); i += pillar_interval)
            {
                auto p_start = points_start[i];
                auto p_last = points_last[i];

                // if (coordtype == hadmap::COORD_WGS84)
                {
                    /*if (gcj02)
                    {
                        WGS84ToGCJ02(p.x, p.y);
                    }*/

                    LonLatToLocal(p_start.x, p_start.y, p_start.z);
                    LonLatToLocal(p_last.x, p_last.y, p_last.z);
                }

                FVector local =
                    FVector((p_start.x + p_last.x) / 2, (p_start.y + p_last.y) / 2, (p_start.z + p_last.z) / 2);
                if (local.ContainsNaN())
                    continue;
                pillar_points.Add(local);
            }
        }
    }

    TArray<FHitResult> HitResList;
    for (auto& Point : pillar_points)
    {
        FVector Start = Point;
        Start.Z = -100.f;
        FVector End = Point + FVector(0, 0, 100);
        FHitResult OutHit;

        if (UKismetSystemLibrary::BoxTraceSingle(this, Start, End, FVector(400, 400, 1), FRotator(),
                UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_GameTraceChannel2), true, TArray<AActor*>(),
                EDrawDebugTrace::None, OutHit, false))
        {
            HitResList.Add(OutHit);
        }
    }

    RenderRoadSplines(HitResList);
}

// 读取配置文件柱子融合参数
float AMapGeneratedActor::BPGetBlendHeight()
{
    float BlendHeight;
    GConfig->GetFloat(
        TEXT("/AutoRoad.MapGeneratedActor"), TEXT("BlendHeight"), BlendHeight, GetDefaultConfigFilename());
    return BlendHeight;
}

void AMapGeneratedActor::DebugGetLane(FVector Pos)
{
    if (!Handle)
        return;
    hadmap::txLanePtr lane;
    hadmap::txPoint pos;
    double Px, Py, Pz = 0.0;
    hadmapue4::HadmapManager::Get()->LocalToLonLat(Pos, Px, Py, Pz);

    pos.x = Px;
    pos.y = Py;
    pos.z = Pz;
    getLane(Handle, pos, lane);

    if (lane)
    {
        auto RoadID = lane->getRoadId();
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Debug Lane ID : %d"), RoadID));
    }
}

FTransform AMapGeneratedActor::GetRoadTrnasfrom(FVector Pos, FRotator Rot, FVector Box)
{
    FTransform Res;
    Res.SetLocation(Pos);
    Res.SetRotation(Rot.Quaternion());
    FCollisionQueryParams QP = FCollisionQueryParams::DefaultQueryParam;
    FCollisionResponseParams RP = FCollisionResponseParams::DefaultResponseParam;
    QP.bTraceComplex = true;
    QP.bIgnoreTouches = false;
    QP.bFindInitialOverlaps = true;
    QP.AddIgnoredActor(this->GetOwner());
    FHitResult ResultRes;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

    // FVector RTLocation;
    FRotator RTRotation;
    if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), Pos + FVector(0, 0, 1000.f), Pos - FVector(0, 0, 1000.f),
            ETraceTypeQuery::TraceTypeQuery4, true, TArray<AActor*>(), EDrawDebugTrace::None, ResultRes, false))
    {
        UE_LOG(LogTemp, Log, TEXT(" Arrow Trace HitResult:%s "), *ResultRes.ToString());
        RTRotation = FQuat::FindBetweenNormals(FVector(0, 0, 1), ResultRes.ImpactNormal).Rotator();
        // RTLocation = ResultRes.Location;
        FVector Test = Rot.Vector().RotateAngleAxis(RTRotation.Pitch, FVector(1, 0, 0));

        RTRotation = (RTRotation.Quaternion() * Rot.Quaternion()).Rotator();
        Res.SetLocation(Pos);
        Res.SetRotation(RTRotation.Quaternion());
    }

    return Res;
}

void AMapGeneratedActor::GetObjRangeGeoms(
    hadmap::txObjectPtr Obj, TArray<FVector>& LeftPoints, TArray<FVector>& RightPoints)
{
    hadmap::txRoadPtr RoadPtr;
    hadmap::txRoadPtr Related_RoadPtr;
    TArray<hadmap::txRoadPtr> RoadPtrs;

    hadmap::txLanePtr lanePtr;
    getLane(Handle, Obj->getPos(), lanePtr);

    hadmap::txRoadPtr _cur_Road = NULL;
    hadmap::getRoad(Handle, lanePtr->getRoadId(), true, _cur_Road);
    objectpkid objID = Obj->getId();
    SetGeomsFromRoad(lanePtr, Obj, LeftPoints, RightPoints);
}

void AMapGeneratedActor::LonLatToLocal(double& _Lon, double& _Lat, double& _Alt)
{
    if (bGenearateRoadOnXY)
    {
        _Alt = SHadmap->mapOriginAlt + 0.1;
    }

    SHadmap->LonLatToLocal(_Lon, _Lat, _Alt);
}

UStaticMesh* AMapGeneratedActor::GeneareteCrossWalkLineArrow(float Length, float Width)
{
    TArray<FVector> VertexPoints;
    TArray<int32> VertexTranigles;
    TArray<FVector> VertexNormals;
    TArray<FVector2D> UV;

    float ArrowTailWidth = 15.f;
    float ArrowTailLength = 40.f;
    float ArrowHeadWidth = 45.f;
    float ArrowHeadLength = 60.f;
    float TotalWidth = 100.f;
    float TotalLength = 100.f;

    // ArrowTail
    VertexPoints.Add(FVector(0.f, (TotalWidth - ArrowTailWidth) / 2.f, 0.f));
    VertexPoints.Add(FVector(0.f, (TotalWidth + ArrowTailWidth) / 2.f, 0.f));
    VertexPoints.Add(FVector(ArrowTailLength, (TotalWidth - ArrowTailWidth) / 2.f, 0.f));
    VertexPoints.Add(FVector(ArrowTailLength, (TotalWidth + ArrowTailWidth) / 2.f, 0.f));

    VertexTranigles.Add(0);
    VertexTranigles.Add(1);
    VertexTranigles.Add(2);
    VertexTranigles.Add(1);
    VertexTranigles.Add(3);
    VertexTranigles.Add(2);

    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);

    UV.Add(FVector2D(0, 0));
    UV.Add(FVector2D(0, 1));
    UV.Add(FVector2D(1, 0));
    UV.Add(FVector2D(1, 1));

    // ArrowHead
    VertexPoints.Add(FVector(ArrowTailLength, (TotalWidth - ArrowHeadWidth) / 2.f, 0.f));
    VertexPoints.Add(FVector(ArrowTailLength, (TotalWidth + ArrowHeadWidth) / 2.f, 0.f));
    VertexPoints.Add(FVector(TotalLength, (TotalWidth) / 2.f, 0.f));

    VertexTranigles.Add(4);
    VertexTranigles.Add(5);
    VertexTranigles.Add(6);

    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);

    UV.Add(FVector2D(0, 0));
    UV.Add(FVector2D(0, 1));
    UV.Add(FVector2D(0.5, 0));

    FVector CenterDelta = FVector(Length, Width, 0.f) * 0.5f;
    for (auto& Elem : VertexPoints)
    {
        Elem -= CenterDelta;
    }

    UStaticMesh* CrossArrowMesh = CreateStaticMesh(VertexPoints, VertexTranigles, VertexNormals, UV,
        CreateStaticMaterial(MatWhite), false, 0, 0, 1, false, false, true);
    return CrossArrowMesh;
}

void AMapGeneratedActor::DrawCrossWalkLine(
    FVector Location, FRotator Rotation, float Width, float Length, bool bDrawArrow, bool bDouble)
{
    const float Interval = 40.f;
    if (bDouble)
    {
        AActor* CrossWalkLineActor1 = GeneareteCrossWalkLine(Location, Rotation, Width * 0.5f, Length, bDrawArrow);
        AActor* CrossWalkLineActor2 = GeneareteCrossWalkLine(Location, Rotation, Width * 0.5f, Length, bDrawArrow);
        if (CrossWalkLineActor1 && CrossWalkLineActor2)
        {
            // CrossWalkLineActor1->SetActorLocation(CrossWalkLineActor1->GetActorLocation() +
            // CrossWalkLineActor1->GetActorRightVector() * Interval);

            // CrossWalkLineActor2->SetActorLocation(CrossWalkLineActor1->GetActorLocation() +
            // CrossWalkLineActor1->GetActorRightVector() * Interval);
            CrossWalkLineActor2->AddActorWorldRotation(FRotator(0, 180.f, 0));
            CrossWalkLineActor2->AddActorLocalOffset(FVector(0, Width * 0.5f + Interval, 0));
        }
    }
    else
    {
        GeneareteCrossWalkLine(Location, Rotation, Width, Length, bDrawArrow);
    }
}

AActor* AMapGeneratedActor::GeneareteCrossWalkLine(
    FVector Location, FRotator Rotation, float Width, float Length, bool bDrawArrow)
{
    TArray<FVector> VertexPoints;
    TArray<int32> VertexTranigles;
    TArray<FVector> VertexNormals;
    TArray<FVector2D> UV;

    float LineStepLength = 45.f;
    float SpaceStepLength = 60.f;
    float CurrentLength = 0.f;
    int32 VertexIndex = 0;

    if (bDrawArrow)
    {
        CurrentLength += 100.f;
    }
    while (CurrentLength < Length)
    {
        VertexPoints.Add(FVector(CurrentLength, 0.f, 0.f));
        VertexPoints.Add(FVector(CurrentLength, Width, 0.f));
        float DeltaLineLength = Length - CurrentLength;
        if (DeltaLineLength < LineStepLength)
        {
            VertexPoints.Add(FVector(CurrentLength + DeltaLineLength, 0.f, 0.f));
            VertexPoints.Add(FVector(CurrentLength + DeltaLineLength, Width, 0.f));
        }
        else
        {
            VertexPoints.Add(FVector(CurrentLength + LineStepLength, 0.f, 0.f));
            VertexPoints.Add(FVector(CurrentLength + LineStepLength, Width, 0.f));
        }

        VertexTranigles.Add(VertexIndex);
        VertexTranigles.Add(VertexIndex + 1);
        VertexTranigles.Add(VertexIndex + 2);
        VertexTranigles.Add(VertexIndex + 1);
        VertexTranigles.Add(VertexIndex + 3);
        VertexTranigles.Add(VertexIndex + 2);
        VertexIndex += 4;

        VertexNormals.Add(FVector::UpVector);
        VertexNormals.Add(FVector::UpVector);
        VertexNormals.Add(FVector::UpVector);
        VertexNormals.Add(FVector::UpVector);

        UV.Add(FVector2D(0, 0));
        UV.Add(FVector2D(0, 1));
        UV.Add(FVector2D(1, 0));
        UV.Add(FVector2D(1, 1));

        CurrentLength += LineStepLength + SpaceStepLength;
    }

    FVector CenterDelta = FVector(Length, Width, 0.f) * 0.5f;
    for (auto& Elem : VertexPoints)
    {
        Elem -= CenterDelta;
    }

    UStaticMesh* CrossLineMesh = CreateStaticMesh(VertexPoints, VertexTranigles, VertexNormals, UV,
        CreateStaticMaterial(MatWhite), false, 0, 0, 1, false, false, true);

    if (CrossLineMesh)
    {
        AStaticMeshActor* CrossWalkActor =
            GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation);
        CrossWalkActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        CrossWalkActor->GetStaticMeshComponent()->SetStaticMesh(CrossLineMesh);
        if (bDrawArrow)
        {
            UStaticMesh* ArrowMesh = GeneareteCrossWalkLineArrow(Length, Width);
            UStaticMeshComponent* ArrowMeshComp1 =
                NewObject<UStaticMeshComponent>(CrossWalkActor, UStaticMeshComponent::StaticClass());
            if (ArrowMesh && ArrowMeshComp1)
            {
                ArrowMeshComp1->SetStaticMesh(ArrowMesh);
            }
            ArrowMeshComp1->RegisterComponent();
            ArrowMeshComp1->AttachToComponent(
                CrossWalkActor->GetStaticMeshComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

            UStaticMeshComponent* ArrowMeshComp2 =
                NewObject<UStaticMeshComponent>(CrossWalkActor, UStaticMeshComponent::StaticClass());
            if (ArrowMesh && ArrowMeshComp2)
            {
                ArrowMeshComp2->SetStaticMesh(ArrowMesh);
            }
            ArrowMeshComp2->RegisterComponent();
            ArrowMeshComp2->AttachToComponent(
                CrossWalkActor->GetStaticMeshComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
            ArrowMeshComp2->SetRelativeLocation(FVector(0, Width - 100.f, 0));
        }

        return CrossWalkActor;
    }
    return nullptr;
}

void AMapGeneratedActor::DrawLongitudinalDecelerationItem(const FVector& StartPoint, TArray<FVector>& VertexPoints,
    TArray<int32>& VertexTranigles, TArray<FVector>& VertexNormals, TArray<FVector2D>& UVs, const FVector& RoadDir,
    const FVector& CenterDir, const float Width, const float Length, int32& VertexIndex)
{
    FVector V1 = StartPoint;
    FVector V2 = StartPoint + CenterDir * Width;
    FVector V3 = StartPoint + RoadDir * Length;
    FVector V4 = StartPoint + CenterDir * Width + RoadDir * Length;

    VertexPoints.Add(V1);
    VertexPoints.Add(V2);
    VertexPoints.Add(V3);
    VertexPoints.Add(V4);

    VertexTranigles.Add(VertexIndex + 2);
    VertexTranigles.Add(VertexIndex + 1);
    VertexTranigles.Add(VertexIndex + 0);
    VertexTranigles.Add(VertexIndex + 2);
    VertexTranigles.Add(VertexIndex + 3);
    VertexTranigles.Add(VertexIndex + 1);
    VertexIndex += 4;

    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);
    VertexNormals.Add(FVector::UpVector);

    UVs.Add(FVector2D(0, 0));
    UVs.Add(FVector2D(0, 1));
    UVs.Add(FVector2D(1, 0));
    UVs.Add(FVector2D(1, 1));
}
// 车行道横向减速标线
void AMapGeneratedActor::DrawLateralDecelerationLine(
    const TArray<FVector>& LeftLinePoints, const TArray<FVector>& RightLinePoints)
{
    if (RightLinePoints.Num() == 0 || LeftLinePoints.Num() == 0)
    {
        return;
    }
    int32 MaxPoints = FMath::Min(LeftLinePoints.Num(), RightLinePoints.Num());

    const float ItemIntervalWidth = 5.f;
    const float ItemIntervalLength = 45.f;
    const float ItemWidth =
        (FVector::Distance(RightLinePoints[0], LeftLinePoints[0]) - ItemIntervalWidth * 2.f) / 3.f;    // 100.f;
    const float ItemLength = 45.f;

    TMap<int32, int32> StepLengthMap = {TPair<int32, int32>(1, 17), TPair<int32, int32>(2, 20),
        TPair<int32, int32>(3, 23), TPair<int32, int32>(4, 26), TPair<int32, int32>(5, 28), TPair<int32, int32>(6, 30),
        TPair<int32, int32>(7, 32)};

    TArray<FVector> TotalVertexPoints;
    TArray<int32> TotalVertexTranigles;
    TArray<FVector> TotalVertexNormals;
    TArray<FVector2D> TotalUV;
    int32 VertexIndex = 0;

    int32 StepIndex = 0;
    int32 CurrentStepLength = 0;
    while (CurrentStepLength <= MaxPoints)
    {
        {
            // draw
            FVector RoadDir;
            if (CurrentStepLength == 0)
            {
                RoadDir =
                    ((LeftLinePoints[1] + RightLinePoints[1]) / 2.f - (LeftLinePoints[0] + RightLinePoints[0]) / 2.f)
                        .GetSafeNormal();
            }
            else if (CurrentStepLength == LeftLinePoints.Num())
            {
                CurrentStepLength = LeftLinePoints.Num() - 1;
                RoadDir = ((LeftLinePoints[CurrentStepLength] + RightLinePoints[CurrentStepLength]) / 2.f -
                           (LeftLinePoints[CurrentStepLength - 1] + RightLinePoints[CurrentStepLength - 1]) / 2.f)
                              .GetSafeNormal();
            }
            else
            {
                RoadDir = ((LeftLinePoints[CurrentStepLength] + RightLinePoints[CurrentStepLength]) / 2.f -
                           (LeftLinePoints[CurrentStepLength - 1] + RightLinePoints[CurrentStepLength - 1]) / 2.f)
                              .GetSafeNormal();
            }

            FVector CenterDir = FVector::CrossProduct(RoadDir, FVector::UpVector);

            FVector StartCenter = (LeftLinePoints[CurrentStepLength] + RightLinePoints[CurrentStepLength]) / 2.f;

            FVector V1 = StartCenter + CenterDir * -1.f * (ItemWidth * 3 + ItemIntervalWidth * 2) * 0.5;
            FVector V2 = V1 + CenterDir * (ItemWidth + ItemIntervalWidth);
            FVector V3 = V2 + CenterDir * (ItemWidth + ItemIntervalWidth);

            FVector V4 = V1 + RoadDir * (ItemLength + ItemIntervalLength);
            FVector V5 = V4 + CenterDir * (ItemWidth + ItemIntervalWidth);
            FVector V6 = V5 + CenterDir * (ItemWidth + ItemIntervalWidth);

            DrawLongitudinalDecelerationItem(V1, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
                RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
            DrawLongitudinalDecelerationItem(V2, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
                RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
            DrawLongitudinalDecelerationItem(V3, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
                RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
            DrawLongitudinalDecelerationItem(V4, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
                RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
            DrawLongitudinalDecelerationItem(V5, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
                RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
            DrawLongitudinalDecelerationItem(V6, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
                RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);

            if (StepIndex >= 6)
            {
                FVector V7 = V4 + RoadDir * (ItemLength + ItemIntervalLength);
                FVector V8 = V7 + CenterDir * (ItemWidth + ItemIntervalWidth);
                FVector V9 = V8 + CenterDir * (ItemWidth + ItemIntervalWidth);
                DrawLongitudinalDecelerationItem(V7, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals,
                    TotalUV, RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
                DrawLongitudinalDecelerationItem(V8, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals,
                    TotalUV, RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
                DrawLongitudinalDecelerationItem(V9, TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals,
                    TotalUV, RoadDir, CenterDir, ItemWidth, ItemLength, VertexIndex);
            }
        }

        StepIndex++;
        if (StepLengthMap.Contains(StepIndex))
        {
            CurrentStepLength += StepLengthMap[StepIndex];
        }
        else if (CurrentStepLength != 0)
        {
            CurrentStepLength += StepLengthMap[7];
        }
    }

    UStaticMesh* Mesh = CreateStaticMesh(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
        CreateStaticMaterial(MatWhite), false, 0, 0, 1, false, false, true);

    if (Mesh)
    {
        AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), FVector(0, 0, 1), FRotator(0, 0, 0));
        Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
    }
}

// 白色半圆状车距确认线
void AMapGeneratedActor::DrawSemicircleLine(
    const TArray<FVector>& LeftLinePoints, const TArray<FVector>& RightLinePoints)
{
    const float StepLength = 5000.f;
    const float CurrentLength = 0.f;
    const float Interval = 5.f + 7.5f;    // 道路线半宽为7.5
    const float Radius = 30.f;

    if (LeftLinePoints.Num() < 2 || RightLinePoints.Num() < 2)
    {
        return;
    }

    TArray<const TArray<FVector>*> Boundary;
    Boundary.Add(&LeftLinePoints);
    Boundary.Add(&RightLinePoints);

    int32 LineVertexIndex = 0;

    for (int32 i = 0; i < Boundary.Num(); i++)
    {
        TArray<FVector> TotalVertexPoints;
        TArray<int32> TotalVertexTranigles;
        TArray<FVector> TotalVertexNormals;
        TArray<FVector2D> TotalUV;
        int32 VertexIndex = 0;
        const TArray<FVector>& SidePoints = *Boundary[i];

        for (int32 j = 0; j <= SidePoints.Num(); j = j + 50 /*每米取样一次，间隔为50米*/)
        {
            // Draw
            FVector RoadDir;

            if (j == 0)
            {
                RoadDir = (SidePoints[1] - SidePoints[0]).GetSafeNormal();
            }
            else
            {
                if (j == SidePoints.Num())
                {
                    j = SidePoints.Num() - 1;
                }
                RoadDir = (SidePoints[j] - SidePoints[j - 1]).GetSafeNormal();
            }
            FVector CenterDir = FVector::CrossProduct(RoadDir, FVector::UpVector);

            FVector V1 = SidePoints[j] + CenterDir * (0 + Interval) * (i == 0 ? 1.f : -1.f);
            FVector V2 = V1 + RoadDir * Radius * 2.f;
            FVector V3 = V1 + CenterDir * Radius * (i == 0 ? 1.f : -1.f);
            FVector V4 = V3 + RoadDir * Radius * 2.f;
            TotalVertexPoints.Add(V1);
            TotalVertexPoints.Add(V2);
            TotalVertexPoints.Add(V3);
            TotalVertexPoints.Add(V4);
            if (i == 0)
            {
                TotalVertexTranigles.Add(VertexIndex);
                TotalVertexTranigles.Add(VertexIndex + 1);
                TotalVertexTranigles.Add(VertexIndex + 2);
                TotalVertexTranigles.Add(VertexIndex + 1);
                TotalVertexTranigles.Add(VertexIndex + 3);
                TotalVertexTranigles.Add(VertexIndex + 2);
            }

            else
            {
                TotalVertexTranigles.Add(VertexIndex + 2);
                TotalVertexTranigles.Add(VertexIndex + 1);
                TotalVertexTranigles.Add(VertexIndex + 0);
                TotalVertexTranigles.Add(VertexIndex + 2);
                TotalVertexTranigles.Add(VertexIndex + 3);
                TotalVertexTranigles.Add(VertexIndex + 1);
            }
            VertexIndex += 4;

            TotalVertexNormals.Add(FVector::UpVector);
            TotalVertexNormals.Add(FVector::UpVector);
            TotalVertexNormals.Add(FVector::UpVector);
            TotalVertexNormals.Add(FVector::UpVector);

            TotalUV.Add(FVector2D(1, 0));
            TotalUV.Add(FVector2D(1, 1));
            TotalUV.Add(FVector2D(0, 0));
            TotalUV.Add(FVector2D(0, 1));
        }

        UStaticMesh* Mesh = CreateStaticMesh(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
            CreateStaticMaterial(TEXT("Mat_Semicircle")), false, 0, 0, 1, false, false, true);

        if (Mesh)
        {
            AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), FVector(0, 0, 1), FRotator(0, 0, 0));
            Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
            Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        }
    }
}

void AMapGeneratedActor::DrawNonAreaLine(TArray<FVector>& VertexPoints, TArray<int32>& VertexTranigles,
    TArray<FVector>& VertexNormals, TArray<FVector2D>& UVs, FVector StartPoint, const FVector& LineDir,
    const float MaxLength)
{
    const float LineWidth = 20.f;
    const float FirstStepDistance = 300.f;
    const float StepDistance = 100.f;
    const float StepBrokenDistance = 100.f;
    const FVector CenterDistance = FVector::CrossProduct(LineDir, FVector::UpVector);
    float CurrentDistance = 0.f;
    int32 CurrentIndex = VertexPoints.Num();
    bool bLast = false;
    float CurrentStep = FirstStepDistance + StepDistance;

    while (MaxLength > CurrentDistance)
    {
        if (MaxLength - CurrentDistance < CurrentStep)
        {
            CurrentStep = MaxLength - CurrentDistance;
            bLast = true;
        }
        FVector V1 = StartPoint;
        FVector V2 = V1 + LineDir * CurrentStep;
        FVector V3 = V1 + CenterDistance * LineWidth;
        FVector V4 = V1 + CenterDistance * LineWidth + LineDir * CurrentStep;
        VertexPoints.Add(V1);
        VertexPoints.Add(V2);
        VertexPoints.Add(V3);
        VertexPoints.Add(V4);

        VertexTranigles.Add(CurrentIndex);
        VertexTranigles.Add(CurrentIndex + 1);
        VertexTranigles.Add(CurrentIndex + 2);
        VertexTranigles.Add(CurrentIndex + 1);
        VertexTranigles.Add(CurrentIndex + 3);
        VertexTranigles.Add(CurrentIndex + 2);

        VertexNormals.Add(FVector::UpVector);
        VertexNormals.Add(FVector::UpVector);
        VertexNormals.Add(FVector::UpVector);
        VertexNormals.Add(FVector::UpVector);

        UVs.Add(FVector2D(0, 0));
        UVs.Add(FVector2D(0, 1));
        UVs.Add(FVector2D(1, 0));
        UVs.Add(FVector2D(1, 1));

        if (bLast)
        {
            break;
        }

        StartPoint += LineDir * (CurrentStep + StepBrokenDistance);
        CurrentDistance += CurrentStep + StepBrokenDistance;
        CurrentStep = StepDistance;
        CurrentIndex += 4;
    }
}

// 禁驶区绘制
void AMapGeneratedActor::DrawNonArea(FVector Location, FRotator Rotation, float Width, float Length)
{
    TArray<FVector> TotalVertexPoints;
    TArray<int32> TotalVertexTranigles;
    TArray<FVector> TotalVertexNormals;
    TArray<FVector2D> TotalUV;
    const FVector LeftUpPoint = FVector(Width / -2.f, Length / -2.f, 0);
    const float FirstStepDistance = 300.f;

    FVector StartPoint = LeftUpPoint + FVector(0, FirstStepDistance, 0);
    FVector LineDir = FVector::ForwardVector;
    DrawNonAreaLine(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV, StartPoint, LineDir,
        Length - FirstStepDistance);

    StartPoint = LeftUpPoint + FVector(Length - FirstStepDistance, 0, 0);
    LineDir = FVector::RightVector;
    DrawNonAreaLine(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV, StartPoint, LineDir,
        Length - FirstStepDistance);

    StartPoint = LeftUpPoint + FVector(Length, Width - FirstStepDistance, 0);
    LineDir = FVector::BackwardVector;
    DrawNonAreaLine(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV, StartPoint, LineDir,
        Length - FirstStepDistance);

    StartPoint = LeftUpPoint + FVector(FirstStepDistance, Width, 0);
    LineDir = FVector::LeftVector;
    DrawNonAreaLine(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV, StartPoint, LineDir,
        Length - FirstStepDistance);

    UStaticMesh* Mesh = CreateStaticMesh(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
        CreateStaticMaterial(MatYellow), false, 0, 0, 1, false, false, true);

    if (Mesh)
    {
        AStaticMeshActor* Actor =
            GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation);
        Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
    }
}

// 路口引导线
void AMapGeneratedActor::DrawGuideLine(const FVector& V1, const FVector& V2, const FVector& V3, const FVector& V4)
{
    TArray<FVector> VertexPoints;
    TArray<int32> VertexTranigles;
    TArray<FVector> VertexNormals;
    TArray<FVector2D> UV;

    const float LineWidth = 15.f;
    const float LineStepLength = 100.f;
    const float BrokenStepLength = 100.f;

    float CurrentStepLength = LineStepLength;
    float Distance = FVector::Distance(V1, V4);
    int32 SampleNum = Distance / 100.f * 10;
    TArray<FVector> SamplePoints;
    SamplePoints.Add(V1);
    for (int i = 1; i < SampleNum; i++)
    {
        float t = (float) i / (float) SampleNum;
        FVector SamplePoint = FMath::Pow((1 - t), 3) * V1 + 3 * t * FMath::Pow((1 - t), 2) * V2 +
                              3 * t * t * (1 - t) * V3 + t * t * t * V4;
        SamplePoints.Add(SamplePoint);
    }
    SamplePoints.Add(V4);

    int32 VertexIndex = 0;
    bool bDrawBroken = false;
    for (int32 i = 0; i < SamplePoints.Num(); i++)
    {
        for (int32 j = i + 1; j < SamplePoints.Num(); j++)
        {
            if (FVector::Distance(SamplePoints[i], SamplePoints[j]) > LineStepLength || j == (SamplePoints.Num() - 1))
            {
                if (j == (SamplePoints.Num() - 1))
                {
                    CurrentStepLength = FVector::Distance(SamplePoints[i], SamplePoints[j]);
                }

                if (bDrawBroken)
                {
                    bDrawBroken = false;
                }
                else
                {
                    FVector LineDir = (SamplePoints[j] - SamplePoints[i]).GetSafeNormal();
                    FVector Point1 = SamplePoints[i];
                    FVector Point2 = SamplePoints[i] + LineDir * CurrentStepLength;
                    FVector Point3 =
                        SamplePoints[i] + FVector::CrossProduct(LineDir, FVector::UpVector).GetSafeNormal() * LineWidth;
                    FVector Point4 = Point3 + LineDir * CurrentStepLength;

                    VertexPoints.Add(Point1);
                    VertexPoints.Add(Point2);
                    VertexPoints.Add(Point3);
                    VertexPoints.Add(Point4);

                    VertexTranigles.Add(VertexIndex);
                    VertexTranigles.Add(VertexIndex + 1);
                    VertexTranigles.Add(VertexIndex + 2);
                    VertexTranigles.Add(VertexIndex + 1);
                    VertexTranigles.Add(VertexIndex + 3);
                    VertexTranigles.Add(VertexIndex + 2);
                    VertexIndex += 4;

                    VertexNormals.Add(FVector::UpVector);
                    VertexNormals.Add(FVector::UpVector);
                    VertexNormals.Add(FVector::UpVector);
                    VertexNormals.Add(FVector::UpVector);

                    UV.Add(FVector2D(0, 0));
                    UV.Add(FVector2D(0, 1));
                    UV.Add(FVector2D(1, 0));
                    UV.Add(FVector2D(1, 1));

                    bDrawBroken = true;
                }

                i = j - 1;
                break;
            }
        }
    }

    UStaticMesh* Mesh = CreateStaticMesh(VertexPoints, VertexTranigles, VertexNormals, UV,
        CreateStaticMaterial(MatYellow), false, 0, 0, 1, false, false, true);

    if (Mesh)
    {
        AStaticMeshActor* LineActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), FVector(0, 0, 1), FRotator(0, 0, 0));
        LineActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        LineActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
    }
}

// 可变导向线
void AMapGeneratedActor::DrawDirectionLane(
    const TArray<FVector>& _leftPoints, const TArray<FVector>& _rightPoints, bool bVariable)
{
    TArray<FVector> TotalVertexPoints;
    TArray<int32> TotalVertexTranigles;
    TArray<FVector> TotalVertexNormals;
    TArray<FVector2D> TotalUV;

    const float Step = 1.414f * 20.f;
    const int interval = 6;
    const float LineWidth = 20.f;

    TArray<const TArray<FVector>*> Boundary;
    Boundary.Add(&_leftPoints);
    Boundary.Add(&_rightPoints);

    int32 LineVertexIndex = 0;

    for (int32 i = 0; i < Boundary.Num(); i++)
    {
        const TArray<FVector>& SidePoints = *Boundary[i];

        int32 PolyVertexIndex = 0;
        float CurrentDis = 0.f;
        float PointDis = 0.f;
        float RecordDis = 0;

        TArray<FVector> PolyVertexPoints;
        TArray<int32> PolyVertexTranigles;
        TArray<FVector> PolyVertexNormals;
        TArray<FVector2D> PolyUV;

        TArray<FVector> LineVertexPoints;
        TArray<int32> LineVertexTranigles;
        TArray<FVector> LineVertexNormals;
        TArray<FVector2D> LineUV;

        for (int32 j = 0; j < SidePoints.Num() - 1; j++)
        {
            double X_Start = SidePoints[j].X;
            double Y_Start = SidePoints[j].Y;
            double Z_Start = SidePoints[j].Z;
            // LonLatToLocal(X_Start, Y_Start, Z_Start);

            double X_End = SidePoints[j + 1].X;
            double Y_End = SidePoints[j + 1].Y;
            double Z_End = SidePoints[j + 1].Z;
            // LonLatToLocal(X_End, Y_End, Z_End);

            FVector PointStart = FVector(X_Start, Y_Start, Z_Start);
            FVector PointEnd = FVector(X_End, Y_End, Z_End);
            FVector RoadDir = (PointEnd - PointStart).GetSafeNormal();
            FVector CenterDir = FVector::CrossProduct(RoadDir, FVector::UpVector);
            RecordDis = PointDis;
            PointDis += FVector::Distance(PointEnd, PointStart);

            PointStart -= CenterDir * LineWidth * 0.5;
            PointEnd -= CenterDir * LineWidth * 0.5;
            {
                // draw line

                LineVertexPoints.Add(PointStart);
                LineVertexPoints.Add(PointEnd);
                LineVertexPoints.Add(PointStart + CenterDir * LineWidth);
                LineVertexPoints.Add(PointEnd + CenterDir * LineWidth);

                LineVertexTranigles.Add(LineVertexIndex);
                LineVertexTranigles.Add(LineVertexIndex + 1);
                LineVertexTranigles.Add(LineVertexIndex + 2);
                LineVertexTranigles.Add(LineVertexIndex + 1);
                LineVertexTranigles.Add(LineVertexIndex + 3);
                LineVertexTranigles.Add(LineVertexIndex + 2);
                LineVertexIndex += 4;

                LineVertexNormals.Add(FVector::UpVector);
                LineVertexNormals.Add(FVector::UpVector);
                LineVertexNormals.Add(FVector::UpVector);
                LineVertexNormals.Add(FVector::UpVector);

                LineUV.Add(FVector2D(0, 0));
                LineUV.Add(FVector2D(0, 1));
                LineUV.Add(FVector2D(1, 0));
                LineUV.Add(FVector2D(1, 1));
            }

            if (bVariable)
            {
                while (CurrentDis <= PointDis)
                {
                    {
                        // draw a little item
                        /*
                        v1:{s:0,t:0}
                        v2:{s:step,t:0}
                        v3:{s:step*1.5,t:step*1.5}
                        v4:{s:step*2,t:step}
                        */
                        FVector Point1 = (PointStart + CenterDir * LineWidth * (i == 0 ? 1.f : 1.f) * 0.5f) +
                                         RoadDir * (CurrentDis - RecordDis);
                        FVector Point2 = Point1 + RoadDir * Step;
                        FVector Point3 =
                            Point1 + RoadDir * Step * 1.5f + CenterDir * Step * 1.5f * (i == 0 ? -1.f : 1.f);
                        FVector Point4 = Point1 + RoadDir * Step * 2.f + CenterDir * Step * 1.f * (i == 0 ? -1.f : 1.f);

                        PolyVertexPoints.Add(Point1);
                        PolyVertexPoints.Add(Point2);
                        PolyVertexPoints.Add(Point3);
                        PolyVertexPoints.Add(Point4);

                        if (i == 0)
                        {
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                            PolyVertexTranigles.Add(PolyVertexIndex + 0);
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                            PolyVertexTranigles.Add(PolyVertexIndex + 3);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                        }
                        else
                        {
                            PolyVertexTranigles.Add(PolyVertexIndex);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                            PolyVertexTranigles.Add(PolyVertexIndex + 3);
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                        }

                        PolyVertexIndex += 4;

                        PolyVertexNormals.Add(FVector::UpVector);
                        PolyVertexNormals.Add(FVector::UpVector);
                        PolyVertexNormals.Add(FVector::UpVector);
                        PolyVertexNormals.Add(FVector::UpVector);

                        PolyUV.Add(FVector2D(0, 0));
                        PolyUV.Add(FVector2D(0, 1));
                        PolyUV.Add(FVector2D(1, 0));
                        PolyUV.Add(FVector2D(1, 1));
                    }
                    CurrentDis += interval * Step;
                }
            }
        }

        TotalVertexPoints.Append(LineVertexPoints);

        TotalVertexTranigles.Append(LineVertexTranigles);
        for (auto& index : PolyVertexTranigles)
        {
            index += TotalVertexPoints.Num();
        }

        TotalVertexPoints.Append(PolyVertexPoints);
        TotalVertexTranigles.Append(PolyVertexTranigles);
        LineVertexIndex = TotalVertexPoints.Num();

        TotalVertexNormals.Append(LineVertexNormals);
        TotalVertexNormals.Append(PolyVertexNormals);

        TotalUV.Append(LineUV);
        TotalUV.Append(PolyUV);

        //  continue;
        UStaticMesh* Mesh = CreateStaticMesh(TotalVertexPoints, TotalVertexTranigles, TotalVertexNormals, TotalUV,
            CreateStaticMaterial(MatWhite), false, 0, 0, 1, false, false, true);
        // Mesh->AddToRoot();
        if (Mesh)
        {
            AStaticMeshActor* LineActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), FVector(0, 0, 1), FRotator(0, 0, 0));
            LineActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
            LineActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        }
    }
}

// 纵向减速线
void AMapGeneratedActor::DrawLongitudinalDecelerationLine(
    const TArray<FVector>& _leftPoints, const TArray<FVector>& _rightPoints)
{
    const int32 DeltaNum = 17;           // 前30米，共分为17段
    const float DeltaLength = 4.1875;    // 前30米，长度33.3->100, 每段增量4.1875cm
    const float DeltaWidth = 1.1764;     // 前30米,宽度10—>30 每段递增1.1764cm
    const float StartLength = 33.33;
    ;
    const float StartWidth = 10.f;
    const float LineInterval = 5.f + 7.5f;    // 减速线与boundary间隔,7.5为道路线半宽

    TArray<const TArray<FVector>*> Boundary;
    Boundary.Add(&_leftPoints);
    Boundary.Add(&_rightPoints);

    int32 LineVertexIndex = 0;

    for (int32 i = 0; i < Boundary.Num(); i++)
    {
        const TArray<FVector>& SidePoints = *Boundary[i];

        int32 PolyVertexIndex = 0;
        float CurrentDis = 0.f;
        float PointDis = 0.f;
        float RecordDis = 0;
        float CurrentLength = StartLength;
        float CurrentWidth = StartWidth;

        TArray<FVector> PolyVertexPoints;
        TArray<int32> PolyVertexTranigles;
        TArray<FVector> PolyVertexNormals;
        TArray<FVector2D> PolyUV;

        for (int32 j = 0; j < SidePoints.Num() - 1; j++)
        {
            double X_Start = SidePoints[j].X;
            double Y_Start = SidePoints[j].Y;
            double Z_Start = SidePoints[j].Z;

            double X_End = SidePoints[j + 1].X;
            double Y_End = SidePoints[j + 1].Y;
            double Z_End = SidePoints[j + 1].Z;

            FVector PointStart = FVector(X_Start, Y_Start, Z_Start);
            FVector PointEnd = FVector(X_End, Y_End, Z_End);
            FVector RoadDir = (PointEnd - PointStart).GetSafeNormal();
            FVector CenterDir = FVector::CrossProduct(RoadDir, FVector::UpVector);
            RecordDis = PointDis;
            PointDis += FVector::Distance(PointEnd, PointStart);

            if (1)
            {
                while (CurrentDis <= PointDis)
                {
                    {
                        FVector Point1 = (PointStart + CenterDir * LineInterval * (i == 0 ? -1.f : 1.f)) +
                                         RoadDir * (CurrentDis - RecordDis);
                        FVector Point2 = Point1 + RoadDir * CurrentLength;
                        FVector Point3 =
                            Point1 + RoadDir * CurrentWidth * 0.5f + CenterDir * CurrentWidth * (i == 0 ? -1.f : 1.f);
                        FVector Point4 =
                            Point2 + RoadDir * CurrentWidth * 0.5f + CenterDir * CurrentWidth * (i == 0 ? -1.f : 1.f);

                        PolyVertexPoints.Add(Point1);
                        PolyVertexPoints.Add(Point2);
                        PolyVertexPoints.Add(Point3);
                        PolyVertexPoints.Add(Point4);

                        if (i == 0)
                        {
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                            PolyVertexTranigles.Add(PolyVertexIndex + 0);
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                            PolyVertexTranigles.Add(PolyVertexIndex + 3);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                        }
                        else
                        {
                            PolyVertexTranigles.Add(PolyVertexIndex);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                            PolyVertexTranigles.Add(PolyVertexIndex + 1);
                            PolyVertexTranigles.Add(PolyVertexIndex + 3);
                            PolyVertexTranigles.Add(PolyVertexIndex + 2);
                        }

                        PolyVertexIndex += 4;

                        PolyVertexNormals.Add(FVector::UpVector);
                        PolyVertexNormals.Add(FVector::UpVector);
                        PolyVertexNormals.Add(FVector::UpVector);
                        PolyVertexNormals.Add(FVector::UpVector);

                        PolyUV.Add(FVector2D(0, 0));
                        PolyUV.Add(FVector2D(0, 1));
                        PolyUV.Add(FVector2D(1, 0));
                        PolyUV.Add(FVector2D(1, 1));
                    }
                    CurrentDis += CurrentLength * 2.f;
                    if ((PolyVertexPoints.Num() / 4) < DeltaNum)    // 顶点数/4等于当前段数，前17段的长宽需要等比递增
                    {
                        CurrentLength += DeltaLength;
                        CurrentWidth += DeltaWidth;
                    }
                }
            }
        }

        //  continue;
        UStaticMesh* Mesh = CreateStaticMesh(PolyVertexPoints, PolyVertexTranigles, PolyVertexNormals, PolyUV,
            CreateStaticMaterial(MatWhite), false, 0, 0, 1, false, false, true);
        // Mesh->AddToRoot();
        if (Mesh)
        {
            AStaticMeshActor* LineActor = GetWorld()->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), FVector(0, 0, 1), FRotator(0, 0, 0));
            LineActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
            LineActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
        }
    }
}

bool AMapGeneratedActor::SetGeomsFromRoad(const hadmap::txLanePtr lanePtr, hadmap::txObjectPtr obj,
    TArray<FVector>& _leftPoints, TArray<FVector>& _rightPoints)
{
    if (!lanePtr)
        return false;
    if (1)
    {
        double starts = obj->getOdData().s - obj->getOdData().length / 2.0;
        double ends = obj->getOdData().s + obj->getOdData().length / 2.0;
        int index = 0;
        txPoint Point;
        hadmap::PointVec TestPoints;
        while (starts + index < ends)
        {
            // getleftboundary points
            double _s = (starts + index);
            hadmap::txLaneBoundaryPtr _leftBoundary = lanePtr->getLeftBoundary();
            double _leftx = 0;
            double _lefty = 0;
            double _yaw = 0;
            double _t = 0;
            if (_leftBoundary->getGeometry() != NULL)
            {
                Point = _leftBoundary->getGeometry()->getPoint(_s);
                TestPoints.push_back(Point);
                LonLatToLocal(Point.x, Point.y, Point.z);
                _leftPoints.Add(FVector(Point.x, Point.y, Point.z));
            }
            // get rightboundary points
            hadmap::txLaneBoundaryPtr _rightBoundary = lanePtr->getRightBoundary();
            double _rightx = 0;
            double _righty = 0;
            if (_rightBoundary->getGeometry() != NULL)
            {
                Point = _rightBoundary->getGeometry()->getPoint(_s);
                LonLatToLocal(Point.x, Point.y, Point.z);
                _rightPoints.Add(FVector(Point.x, Point.y, Point.z));
            }
            index++;
        }

        //  m_geoms.sPoints.push_back(_leftPoints);
        //  m_geoms.sPoints.push_back(_rightPoints);
    }
    return true;
}

#if WITH_EDITOR
void AMapGeneratedActor::GenerateGeo()
{
    SHadmap->Init(hadmapue4::MAPENGINE, "", RefX, RefY, RefZ, "");

    FString Json;
    if (FFileHelper::LoadFileToString(Json, *GeoPath))
    {
        TSharedPtr<FJsonObject> json;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
        if (FJsonSerializer::Deserialize(Reader, json))
        {
            TArray<TSharedPtr<FJsonValue>> features = json->GetArrayField("features");
            for (auto feature : features)
            {
                FVector Center = FVector::ZeroVector;

                TSharedPtr<FJsonObject> properties = feature->AsObject()->GetObjectField("properties");

                float Yaw = properties->GetNumberField("YAW");
                int32 TL_ID = properties->GetNumberField("TL_ID");
                FString PhaseStr = properties->GetStringField("PHASE");
                int32 Phase = FCString::Atoi(*PhaseStr);
                FString PhaseMark = "Turn";
                if ((Phase >> 0) & 1)
                    continue;    // 无需制作
                if ((Phase >> 1) & 1)
                    PhaseMark.Append("1");    // 未分类
                if ((Phase >> 2) & 1)
                    PhaseMark.Append("2");    // 直行
                if ((Phase >> 3) & 1)
                    PhaseMark.Append("3");    // 左转
                if ((Phase >> 4) & 1)
                    PhaseMark.Append("4");    // 右转
                if ((Phase >> 5) & 1)
                    PhaseMark.Append("5");    // 掉头

                TSharedPtr<FJsonObject> geometry = feature->AsObject()->GetObjectField("geometry");
                TArray<TSharedPtr<FJsonValue>> coordinates = geometry->GetArrayField("coordinates");

                int32 PolyNum = 0;
                for (auto Point : coordinates)
                {
                    PolyNum = Point->AsArray().Num();
                    for (const TSharedPtr<FJsonValue>& PointValue : Point->AsArray())
                    {
                        const TArray<TSharedPtr<FJsonValue>>& Values = PointValue->AsArray();
                        if (Values.Num() >= 3)
                        {
                            double x = Values[0]->AsNumber();
                            double y = Values[1]->AsNumber();
                            double z = Values[2]->AsNumber();
                            LonLatToLocal(x, y, z);
                            Center.X += x;
                            Center.Y += y;
                            Center.Z += z;
                        }
                    }
                }
                Center = Center / PolyNum;

                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                FRotator ActorRot;
                ActorRot.Yaw = Yaw + 90;
                AActor* NewItem =
                    GetWorld()->SpawnActor<AActor>(GeoClass.Get(), GetActorLocation() + Center, ActorRot, Params);
                if (NewItem)
                {
                    FString ObjName = TEXT("MapGeo_TrafficLight_") + PhaseMark + TEXT("_") + FString::FromInt(TL_ID);

                    NewItem->SetActorLabel(ObjName);
                    NewItem->Rename(*ObjName);
                }
            }
        }
    }
}

#endif
