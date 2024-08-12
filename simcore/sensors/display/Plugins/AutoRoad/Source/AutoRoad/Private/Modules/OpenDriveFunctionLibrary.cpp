// Fill out your copyright notice in the Description page of Project Settings.

#include "Modules/OpenDriveFunctionLibrary.h"
#include "HadmapManager.h"
#include "HAL/FileManager.h"
#include "mapengine/hadmap_engine.h"

FVector UOpenDriveFunctionLibrary::BPGetMapCenterLonlat(
    const FString& pathIput, const float& RefX, const float& RefY, const float& RefZ)
{
    return GetMapCenterLonlat(pathIput, RefX, RefY, RefZ);
}

FVector UOpenDriveFunctionLibrary::GetMapCenterLonlat(
    const FString& pathIput, const double& RefX, const double& RefY, const double& RefZ)
{
    FVector out = FVector::ZeroVector;

    if (!IFileManager::Get().FileExists(*pathIput))
    {
        UE_LOG(LogTemp, Error, TEXT("FilePath Error"));
    }

    if (SHadmap->Init(hadmapue4::MAPENGINE, pathIput, RefX, RefY, RefZ, pathIput))
    {
        double X_MAX = 0;
        double X_MIN = 0;
        double Y_MAX = 0;
        double Y_MIN = 0;
        double Z_MIN = 0;

        hadmap::txRoads _roads;
        hadmap::getRoads(SHadmap->GetMapHandle(), true, _roads);

        for (auto& road_ptr : _roads)
        {
            for (auto section_ptr : road_ptr->getSections())
            {
                hadmap::txLaneBoundaries bds = section_ptr->getBoundaries();
                if (bds.empty())
                {
                    continue;
                }

                for (size_t ii = 0; ii < bds.size(); ii++)
                {
                    hadmap::txLaneBoundaryPtr lane = bds[ii];
                    hadmap::PointVec points;
                    static_cast<const hadmap::txLineCurve*>(lane->getGeometry())->getPoints(points);

                    for (size_t i = 0, is = points.size(); i < is; i++)
                    {
                        auto& p = points[i];
                        X_MAX = X_MAX == 0 || X_MAX < p.x ? p.x : X_MAX;
                        X_MIN = X_MIN == 0 || X_MIN > p.x ? p.x : X_MIN;
                        Y_MAX = Y_MAX == 0 || Y_MAX < p.y ? p.y : Y_MAX;
                        Y_MIN = Y_MIN == 0 || Y_MIN > p.y ? p.y : Y_MIN;

                        if (p.z < Z_MIN)
                        {
                            Z_MIN = p.z;
                        }
                    }
                }
            }
        }

        out.X = (X_MAX + X_MIN) / 2;
        out.Y = (Y_MAX + Y_MIN) / 2;
        out.Z = Z_MIN;
    }

    return out;
}

void UOpenDriveFunctionLibrary::RenameObjName(AActor* Target, const FString& NewName)
{
    if (Target)
    {
        Target->Rename(*NewName);
    }
}
