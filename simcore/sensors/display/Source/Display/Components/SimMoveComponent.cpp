#include "SimMoveComponent.h"
#include "Engine/World.h"
#include "Misc/ConfigCacheIni.h"

USimMoveComponent::USimMoveComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    TArray<FString> SnapRangeStr;
    if (GConfig->GetSingleLineArray(TEXT("MapDecrypt"), TEXT("SnapRange"), SnapRangeStr, GGameIni))
    {
        if (SnapRangeStr.Num() >= 2)
        {
            snapRangeTop = FCString::Atof(*SnapRangeStr[0]);
            snapRangeBottom = FCString::Atof(*SnapRangeStr[1]);
        }
    }
}

void USimMoveComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool USimMoveComponent::GetSnapGroundTransform(
    FTransform& _SnappedTransform, const FVector& _Location, const FRotator& _Rotation, bool _IsForce)
{
    // return GetSnapGroundTransform(_SnappedTransform, _Location, _Rotation, 1000000, -1000000);
    // TODO: Save compute

    if (GetSnapGroundTransform(_SnappedTransform, _Location, _Rotation, snapRangeTop, snapRangeBottom))
    {
        return true;
    }
    else
    {
        if (_IsForce)
        {
            return GetSnapGroundTransform(_SnappedTransform, _Location, _Rotation, 1000000, -1000000);
        }
        return false;
    }
}

bool USimMoveComponent::GetSnapGroundTransform(FTransform& _SnappedTransform, const FVector& _Location,
    const FRotator& _Rotation, float _RTTop, float _RTBottom, bool _UseAverage)
{
    FVector RTLocation;
    FRotator RTRotation;
    FVector RTStart = _Location;
    FVector RTEnd = _Location;
    RTStart.Z += _RTTop;
    RTEnd.Z += _RTBottom;
    FHitResult ResultUsed;
    TArray<FHitResult> ResultArry;
    // FCollisionQueryParams CQParam;
    // FCollisionObjectQueryParams COQParam;
    // GetWorld()->LineTraceSingleByObjectType(Result, RTStart, RTEnd, , CQParam);
    // DrawDebugLine(GetWorld(), RTStart, RTEnd, FColor::Red, false, 0.1, 0, 10);
    int32 smoothN = 6;
    FCollisionQueryParams QP = FCollisionQueryParams::DefaultQueryParam;
    FCollisionResponseParams RP = FCollisionResponseParams::DefaultResponseParam;
    QP.bTraceComplex = true;
    QP.bIgnoreTouches = false;
    QP.bFindInitialOverlaps = true;
    QP.AddIgnoredActor(this->GetOwner());
    if (GetWorld()->LineTraceMultiByChannel(ResultArry, RTStart, RTEnd, ECollisionChannel::ECC_GameTraceChannel2, QP))
    {
        if (ResultArry.Num() > 1)
        {
            float MinDistance = FLT_MAX;
            for (auto& Elem : ResultArry)
            {
                float Distance = (Elem.ImpactPoint - _Location).Size();
                if (Distance < MinDistance)
                {
                    MinDistance = Distance;
                    ResultUsed = Elem;
                }
            }
        }
        else
        {
            ResultUsed = ResultArry[0];
        }

        RTRotation = FQuat::FindBetweenNormals(FVector(0, 0, 1), ResultUsed.ImpactNormal).Rotator();
        RTLocation = ResultUsed.ImpactPoint;

        if (_UseAverage)
        {
            sumRpy += RTRotation;
            sumZ += RTLocation.Z;
            if (smoothRpyBuf.Num() < smoothN)
            {
                smoothRpyBuf.Add(RTRotation);
                smoothZBuf.Add(RTLocation.Z);
                curIdx = 0;
            }
            else
            {
                sumRpy -= smoothRpyBuf[curIdx];
                sumZ -= smoothZBuf[curIdx];
                smoothRpyBuf[curIdx] = RTRotation;
                smoothZBuf[curIdx] = RTLocation.Z;
                curIdx = (curIdx + 1) % smoothN;
            }
            RTRotation = sumRpy * (1.0f / smoothRpyBuf.Num());
            RTLocation = _Location;
            RTLocation.Z = sumZ / smoothRpyBuf.Num();

            // SetActorLocation(RTLocation);
            // SetActorRotation(_Input.rotation);
            // AddActorWorldRotation(RTRotation);

            // GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("ImpactPoint: %s"),
            // Result.ImpactPoint.ToString())); GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green,
            // FString::Printf(TEXT("Name: %s"), *Result.Actor->GetFName().ToString())); DrawDebugLine(GetWorld(),
            // RTStart, RTLocation, FColor::Red, false, 0.1, 0, 10);
        }

        RTRotation = (RTRotation.Quaternion() * _Rotation.Quaternion()).Rotator();

        FTransform RTTransform;
        RTTransform.SetLocation(RTLocation);
        RTTransform.SetRotation(RTRotation.Quaternion());
        _SnappedTransform = RTTransform;
        return true;
    }
    else
    {
        // SetActorLocation(_Input.location);
        // SetActorRotation(_Input.rotation);
        FTransform RTTransform;
        RTTransform.SetLocation(_Location);
        RTTransform.SetRotation(_Rotation.Quaternion());
        _SnappedTransform = RTTransform;
        return false;
    }
}

bool USimMoveComponent::GetSnapGroundTransform_NotAverage(
    FTransform& _SnappedTransform, const FVector& _Location, const FRotator& _Rotation, bool _IsForce)
{
    if (GetSnapGroundTransform(_SnappedTransform, _Location, _Rotation, snapRangeTop, snapRangeBottom, false))
    {
        return true;
    }
    else
    {
        if (_IsForce)
        {
            return GetSnapGroundTransform(_SnappedTransform, _Location, _Rotation, 1000000, -1000000, false);
        }
        return false;
    }
}

void USimMoveComponent::MoveSimActor(
    AActor* _Actor, const FVector& _Location, const FRotator& _Rotation, bool _UseSnapGround)
{
    // TODO: simactor movement function

    // if (!_Actor)
    //{
    //     return;
    // }

    // if (_UseSnapGround)
    //{
    //     GetSnapGroundTransform(_Location,  _Rotation);
    // }
}
