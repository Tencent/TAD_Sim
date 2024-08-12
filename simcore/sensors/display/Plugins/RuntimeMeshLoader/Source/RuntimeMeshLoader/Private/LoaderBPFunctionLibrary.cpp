// Fill out your copyright notice in the Description page of Project Settings.
#include "LoaderBPFunctionLibrary.h"
#include "RuntimeMeshLoader.h"
#include "ProceduralMeshComponent.h"
#include "StaticMeshAttributes.h"

#include "ProceduralMeshConversion.h"

#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "AssetRegistryModule.h"

#include "Engine/Texture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetSystemLibrary.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "Engine.h"
#include "ImageUtils.h"

#include <assimp/Importer.hpp>     // C++ importer interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

URuntimeMeshLoader* URuntimeMeshLoader::Instance = nullptr;

void FindMeshInfo(const aiScene* scene, aiNode* node, FReturnedData& result, FTransform& tran, int32& MeshIdx)
{
    FMatrix tempMatrix2;

    // e.g
    // _______________
    // | A0,B0,C0,D0 |
    // | A1,B1,C1,D1 |
    // | A2,B2,C2,D2 |
    // | A3,B3,C3,D3 |
    // |_____________|
    //
    tempMatrix2.M[0][0] = node->mTransformation.a1;
    tempMatrix2.M[0][1] = node->mTransformation.b1;
    tempMatrix2.M[0][2] = node->mTransformation.c1;
    tempMatrix2.M[0][3] = node->mTransformation.d1;
    tempMatrix2.M[1][0] = node->mTransformation.a2;
    tempMatrix2.M[1][1] = node->mTransformation.b2;
    tempMatrix2.M[1][2] = node->mTransformation.c2;
    tempMatrix2.M[1][3] = node->mTransformation.d2;
    tempMatrix2.M[2][0] = node->mTransformation.a3;
    tempMatrix2.M[2][1] = node->mTransformation.b3;
    tempMatrix2.M[2][2] = node->mTransformation.c3;
    tempMatrix2.M[2][3] = node->mTransformation.d3;
    tempMatrix2.M[3][0] = node->mTransformation.a4;
    tempMatrix2.M[3][1] = node->mTransformation.b4;
    tempMatrix2.M[3][2] = node->mTransformation.c4;
    tempMatrix2.M[3][3] = node->mTransformation.d4;

    // Mesh transform on scene
    FTransform T = FTransform(tempMatrix2);
    FVector Origin = T.GetLocation();

    FVector Loc = tran.Rotator().RotateVector(Origin);
    tran.SetLocation(tran.GetLocation() + Loc);
    // transform...
    aiMatrix4x4 TranMat, tempMat;

    bool bTran = false;
    if (!tran.GetLocation().Equals(FVector{0.0f}, 0.01f))
    {
        bTran = true;
        TranMat = TranMat * aiMatrix4x4::Translation(
                                aiVector3D{tran.GetLocation().X, tran.GetLocation().Y, tran.GetLocation().Z}, tempMat);
    }

    if (!tran.GetScale3D().Equals(FVector{1.0f}, 0.01f))
    {
        bTran = true;
        TranMat = TranMat * aiMatrix4x4::Scaling(
                                aiVector3D{tran.GetScale3D().X, tran.GetScale3D().Y, tran.GetScale3D().Z}, tempMat);
    }

    if (!tran.GetRotation().Equals(FRotator{0.0f}.Quaternion(), 0.01f))
    {
        bTran = true;
        TranMat = TranMat * aiMatrix4x4::RotationX(PI / 180.f * tran.GetRotation().Rotator().Roll, tempMat);
        TranMat = TranMat * aiMatrix4x4::RotationY(PI / 180.f * tran.GetRotation().Rotator().Yaw, tempMat);
        TranMat = TranMat * aiMatrix4x4::RotationZ(PI / 180.f * tran.GetRotation().Rotator().Pitch, tempMat);
    }

    for (uint32 i = 0; i < node->mNumMeshes; i++)
    {
        std::string TestString = node->mName.C_Str();

        FString Fs = FString(TestString.c_str());

        UE_LOG(LogTemp, Warning, TEXT("FindMeshInfo. %s\n"), *Fs);

        UE_LOG(LogTemp, Warning, TEXT("meshidx. %d\n"), MeshIdx);
        aiMesh* mesh = scene->mMeshes[MeshIdx];
        FMeshInfo& mi = result.meshInfo[MeshIdx];
        MeshIdx++;
        aiMatrix4x4 tempTrans = node->mTransformation;
        // 如果变换
        if (bTran)
        {
            tempTrans = tempTrans * TranMat;
        }

        FMatrix tempMatrix;

        // e.g
        // _______________
        // | A0,B0,C0,D0 |
        // | A1,B1,C1,D1 |
        // | A2,B2,C2,D2 |
        // | A3,B3,C3,D3 |
        // |_____________|
        //
        tempMatrix.M[0][0] = tempTrans.a1;
        tempMatrix.M[0][1] = tempTrans.b1;
        tempMatrix.M[0][2] = tempTrans.c1;
        tempMatrix.M[0][3] = tempTrans.d1;
        tempMatrix.M[1][0] = tempTrans.a2;
        tempMatrix.M[1][1] = tempTrans.b2;
        tempMatrix.M[1][2] = tempTrans.c2;
        tempMatrix.M[1][3] = tempTrans.d2;
        tempMatrix.M[2][0] = tempTrans.a3;
        tempMatrix.M[2][1] = tempTrans.b3;
        tempMatrix.M[2][2] = tempTrans.c3;
        tempMatrix.M[2][3] = tempTrans.d3;
        tempMatrix.M[3][0] = tempTrans.a4;
        tempMatrix.M[3][1] = tempTrans.b4;
        tempMatrix.M[3][2] = tempTrans.c4;
        tempMatrix.M[3][3] = tempTrans.d4;

        // Mesh transform on scene
        mi.RelativeTransform = FTransform(tempMatrix);

        // fill Mesh Vertices 填充Mesh顶点
        for (uint32 j = 0; j < mesh->mNumVertices; ++j)
        {
            FVector vertex = FVector(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);

            FVector V = mi.RelativeTransform.GetLocation();
            FRotator R = mi.RelativeTransform.GetRotation().Rotator();
            FVector Scale = mi.RelativeTransform.GetScale3D();
            // mi.RelativeTransform.SetRotation(FRotator().Quaternion());
            vertex = mi.RelativeTransform.TransformFVector4(vertex);    // TransformFVector4(vertex);
            vertex *= 100.f;
            // vertex = mi.RelativeTransform.Trans
            mi.Vertices.Push(vertex);

            // Normal
            if (mesh->HasNormals())
            {
                FVector normal = FVector(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);

                if (bTran)
                {
                    normal = mi.RelativeTransform.TransformFVector4(normal);
                    normal.Normalize();
                }
                mi.Normals.Push(FVector(0, 0, 1));
                // mi.Normals.Push(normal * -1.f);
            }
            else
            {
                mi.Normals.Push(FVector::ZeroVector);
            }

            // UV0 Coordinates - inconsistent coordinates
            if (mesh->HasTextureCoords(0))
            {
                FVector2D uv = FVector2D(mesh->mTextureCoords[0][j].x, -mesh->mTextureCoords[0][j].y);
                mi.UV0.Add(uv);
            }
            // UV1 Coordinates - inconsistent coordinates
            if (mesh->HasTextureCoords(1))
            {
                FVector2D uv = FVector2D(mesh->mTextureCoords[1][j].x, -mesh->mTextureCoords[1][j].y);
                mi.UV1.Add(uv);
            }
            // UV2 Coordinates - inconsistent coordinates
            if (mesh->HasTextureCoords(2))
            {
                FVector2D uv = FVector2D(mesh->mTextureCoords[2][j].x, -mesh->mTextureCoords[2][j].y);
                mi.UV2.Add(uv);
            }

            // UV3 Coordinates - inconsistent coordinates
            if (mesh->HasTextureCoords(3))
            {
                FVector2D uv = FVector2D(mesh->mTextureCoords[3][j].x, -mesh->mTextureCoords[3][j].y);
                mi.UV3.Add(uv);
            }

            // Tangent /切线
            if (mesh->HasTangentsAndBitangents())
            {
                FProcMeshTangent meshTangent =
                    FProcMeshTangent(mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z);

                mi.Tangents.Push(meshTangent);
            }

            // Vertex color
            if (mesh->HasVertexColors(0))
            {
                FLinearColor color = FLinearColor(
                    mesh->mColors[0][j].r, mesh->mColors[0][j].g, mesh->mColors[0][j].b, mesh->mColors[0][j].a);
                mi.VertexColors.Push(color);
            }
        }
    }
}

void FindMesh(const aiScene* scene, aiNode* node, FReturnedData& retdata, FTransform& tran, int32& MeshIdx)
{
    FindMeshInfo(scene, node, retdata, tran, MeshIdx);

    // tree node
    for (uint32 m = 0; m < node->mNumChildren; ++m)
    {
        FindMesh(scene, node->mChildren[m], retdata, tran, MeshIdx);
    }
}

#pragma region copy

TMap<UMaterialInterface*, FPolygonGroupID> BuildMaterialMapEx(
    UProceduralMeshComponent* ProcMeshComp, FMeshDescription& MeshDescription)
{
    TMap<UMaterialInterface*, FPolygonGroupID> UniqueMaterials;
    const int32 NumSections = ProcMeshComp->GetNumSections();

    UniqueMaterials.Reserve(NumSections);

    FStaticMeshAttributes AttributeGetter(MeshDescription);
    TPolygonGroupAttributesRef<FName> PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();

    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FProcMeshSection* ProcSection = ProcMeshComp->GetProcMeshSection(SectionIdx);
        UMaterialInterface* Material = ProcMeshComp->GetMaterial(SectionIdx);

        if (!UniqueMaterials.Contains(Material))
        {
            FPolygonGroupID NewPolygonGroup = MeshDescription.CreatePolygonGroup();
            UniqueMaterials.Add(Material, NewPolygonGroup);
            PolygonGroupNames[NewPolygonGroup] = Material->GetFName();
        }
    }
    return UniqueMaterials;
}

FMeshDescription BuildMeshDescriptionEx(UProceduralMeshComponent* ProcMeshComp)
{
    FMeshDescription MeshDescription;

    FStaticMeshAttributes AttributeGetter(MeshDescription);
    AttributeGetter.Register();

    TPolygonGroupAttributesRef<FName> PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();
    TVertexAttributesRef<FVector> VertexPositions = AttributeGetter.GetVertexPositions();
    TVertexInstanceAttributesRef<FVector> Tangents = AttributeGetter.GetVertexInstanceTangents();
    TVertexInstanceAttributesRef<float> BinormalSigns = AttributeGetter.GetVertexInstanceBinormalSigns();
    TVertexInstanceAttributesRef<FVector> Normals = AttributeGetter.GetVertexInstanceNormals();
    TVertexInstanceAttributesRef<FVector4> Colors = AttributeGetter.GetVertexInstanceColors();
    TVertexInstanceAttributesRef<FVector2D> UVs = AttributeGetter.GetVertexInstanceUVs();

    // Materials to apply to new mesh
    const int32 NumSections = ProcMeshComp->GetNumSections();
    int32 VertexCount = 0;
    int32 VertexInstanceCount = 0;
    int32 PolygonCount = 0;

    TMap<UMaterialInterface*, FPolygonGroupID> UniqueMaterials = BuildMaterialMapEx(ProcMeshComp, MeshDescription);
    TArray<FPolygonGroupID> PolygonGroupForSection;
    PolygonGroupForSection.Reserve(NumSections);

    // Calculate the totals for each ProcMesh element type
    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FProcMeshSection* ProcSection = ProcMeshComp->GetProcMeshSection(SectionIdx);
        VertexCount += ProcSection->ProcVertexBuffer.Num();
        VertexInstanceCount += ProcSection->ProcIndexBuffer.Num();
        PolygonCount += ProcSection->ProcIndexBuffer.Num() / 3;
    }
    MeshDescription.ReserveNewVertices(VertexCount);
    MeshDescription.ReserveNewVertexInstances(VertexInstanceCount);
    MeshDescription.ReserveNewPolygons(PolygonCount);
    MeshDescription.ReserveNewEdges(PolygonCount * 2);
    UVs.SetNumIndices(4);

    // Create the Polygon Groups
    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FProcMeshSection* ProcSection = ProcMeshComp->GetProcMeshSection(SectionIdx);
        UMaterialInterface* Material = ProcMeshComp->GetMaterial(SectionIdx);
        FPolygonGroupID* PolygonGroupID = UniqueMaterials.Find(Material);
        check(PolygonGroupID != nullptr);
        PolygonGroupForSection.Add(*PolygonGroupID);
    }

    // Add Vertex and VertexInstance and polygon for each section
    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FProcMeshSection* ProcSection = ProcMeshComp->GetProcMeshSection(SectionIdx);
        FPolygonGroupID PolygonGroupID = PolygonGroupForSection[SectionIdx];
        // Create the vertex
        int32 NumVertex = ProcSection->ProcVertexBuffer.Num();
        TMap<int32, FVertexID> VertexIndexToVertexID;
        VertexIndexToVertexID.Reserve(NumVertex);

        for (int32 VertexIndex = 0; VertexIndex < NumVertex; ++VertexIndex)
        {
            FProcMeshVertex& Vert = ProcSection->ProcVertexBuffer[VertexIndex];
            const FVertexID VertexID = MeshDescription.CreateVertex();
            VertexPositions[VertexID] = Vert.Position;
            VertexIndexToVertexID.Add(VertexIndex, VertexID);
        }
        // Create the VertexInstance
        int32 NumIndices = ProcSection->ProcIndexBuffer.Num();
        int32 NumTri = NumIndices / 3;
        TMap<int32, FVertexInstanceID> IndiceIndexToVertexInstanceID;
        IndiceIndexToVertexInstanceID.Reserve(NumVertex);
        for (int32 IndiceIndex = 0; IndiceIndex < NumIndices; IndiceIndex++)
        {
            const int32 VertexIndex = ProcSection->ProcIndexBuffer[IndiceIndex];
            const FVertexID VertexID = VertexIndexToVertexID[VertexIndex];
            const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
            IndiceIndexToVertexInstanceID.Add(IndiceIndex, VertexInstanceID);

            FProcMeshVertex& ProcVertex = ProcSection->ProcVertexBuffer[VertexIndex];

            Tangents[VertexInstanceID] = ProcVertex.Tangent.TangentX;
            Normals[VertexInstanceID] = ProcVertex.Normal;
            BinormalSigns[VertexInstanceID] = ProcVertex.Tangent.bFlipTangentY ? -1.f : 1.f;

            Colors[VertexInstanceID] = FLinearColor(ProcVertex.Color);

            UVs.Set(VertexInstanceID, 0, ProcVertex.UV0);
            UVs.Set(VertexInstanceID, 1, ProcVertex.UV1);
            UVs.Set(VertexInstanceID, 2, ProcVertex.UV2);
            UVs.Set(VertexInstanceID, 3, ProcVertex.UV3);
        }

        // Create the polygons for this section
        for (int32 TriIdx = 0; TriIdx < NumTri; TriIdx++)
        {
            FVertexID VertexIndexes[3];
            TArray<FVertexInstanceID> VertexInstanceIDs;
            VertexInstanceIDs.SetNum(3);

            for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
            {
                const int32 IndiceIndex = (TriIdx * 3) + CornerIndex;
                const int32 VertexIndex = ProcSection->ProcIndexBuffer[IndiceIndex];
                VertexIndexes[CornerIndex] = VertexIndexToVertexID[VertexIndex];
                VertexInstanceIDs[CornerIndex] = IndiceIndexToVertexInstanceID[IndiceIndex];
            }

            // Insert a polygon into the mesh
            MeshDescription.CreatePolygon(PolygonGroupID, VertexInstanceIDs);
        }
    }
    return MeshDescription;
}

/**
 *
 */
TMap<int32, FPolygonGroupID> BuildMaterialMapExchange(
    FReturnedData& ReturnedData, /* UProceduralMeshComponent* ProcMeshComp ,*/ FMeshDescription& MeshDescription)
{
    TMap<int32, FPolygonGroupID> UniqueMaterials;
    const int32 NumSections = ReturnedData.meshInfo.Num();    // ProcMeshComp->GetNumSections();
    UniqueMaterials.Reserve(NumSections);

    FStaticMeshAttributes AttributeGetter(MeshDescription);
    TPolygonGroupAttributesRef<FName> PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();

    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FMeshInfo MeshInfo = ReturnedData.meshInfo[SectionIdx];
        // MeshInfo.Normals
        UMaterialInterface* Material = UMaterial::GetDefaultMaterial(MD_Surface);
        // UMaterialInterface* Material = UMaterial::GetDefaultMaterial(MD_Surface);
        // if ( !UniqueMaterials.Contains(Material) )
        {
            FPolygonGroupID NewPolygonGroup = MeshDescription.CreatePolygonGroup();

            UniqueMaterials.Add(SectionIdx, NewPolygonGroup);
            PolygonGroupNames[NewPolygonGroup] = Material->GetFName();
        }
    }

    return UniqueMaterials;
}

/**
 *
 */
FMeshDescription BuildMeshDescriptionExtend(FReturnedData& MeshsData /* UProceduralMeshComponent* ProcMeshComp */)
{
    FMeshDescription MeshDescription;

    FStaticMeshAttributes AttributeGetter(MeshDescription);
    AttributeGetter.Register();

    TPolygonGroupAttributesRef<FName> PolygonGroupNames = AttributeGetter.GetPolygonGroupMaterialSlotNames();
    TVertexAttributesRef<FVector> VertexPositions = AttributeGetter.GetVertexPositions();
    TVertexInstanceAttributesRef<FVector> Tangents = AttributeGetter.GetVertexInstanceTangents();
    TVertexInstanceAttributesRef<float> BinormalSigns = AttributeGetter.GetVertexInstanceBinormalSigns();
    TVertexInstanceAttributesRef<FVector> Normals = AttributeGetter.GetVertexInstanceNormals();
    TVertexInstanceAttributesRef<FVector4> Colors = AttributeGetter.GetVertexInstanceColors();
    TVertexInstanceAttributesRef<FVector2D> UVs = AttributeGetter.GetVertexInstanceUVs();

    // Materials to apply to new mesh

    const int32 NumSections = MeshsData.meshInfo.Num();    // ProcMeshComp->GetNumSections();
    int32 VertexCount = 0;
    int32 VertexInstanceCount = 0;
    int32 PolygonCount = 0;

    TMap<int32, FPolygonGroupID> UniqueMaterials = BuildMaterialMapExchange(MeshsData, MeshDescription);
    TArray<FPolygonGroupID> PolygonGroupForSection;
    PolygonGroupForSection.Reserve(NumSections);
    // Calculate the totals for each ProcMesh element type
    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FMeshInfo MeshInfo = MeshsData.meshInfo[SectionIdx];

        VertexCount += MeshInfo.Vertices.Num();             // ProcSection->ProcVertexBuffer.Num();
        VertexInstanceCount += MeshInfo.Triangles.Num();    // ProcSection->ProcIndexBuffer.Num();
        PolygonCount += MeshInfo.Triangles.Num() / 3;       // ProcSection->ProcIndexBuffer.Num() / 3;
    }

    MeshDescription.ReserveNewVertices(VertexCount);
    MeshDescription.ReserveNewVertexInstances(VertexInstanceCount);
    MeshDescription.ReserveNewPolygons(PolygonCount);
    MeshDescription.ReserveNewEdges(PolygonCount * 2);
    UVs.SetNumIndices(4);

    // Create the Polygon Groups
    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FMeshInfo MeshInfo = MeshsData.meshInfo[SectionIdx];

        UMaterialInterface* Material = UMaterial::GetDefaultMaterial(MD_Surface);

        FPolygonGroupID* PolygonGroupID = UniqueMaterials.Find(SectionIdx);
        check(PolygonGroupID != nullptr);
        PolygonGroupForSection.Add(*PolygonGroupID);
    }

    // Add Vertex and VertexInstance and polygon for each section
    for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
    {
        FMeshInfo MeshInfo = MeshsData.meshInfo[SectionIdx];
        FPolygonGroupID PolygonGroupID = PolygonGroupForSection[SectionIdx];
        // Create the vertex
        int32 NumVertex = MeshInfo.Vertices.Num();
        if (NumVertex == 0)
        {
            continue;
        }
        TMap<int32, FVertexID> VertexIndexToVertexID;
        VertexIndexToVertexID.Reserve(NumVertex);

        for (int32 VertexIndex = 0; VertexIndex < NumVertex; ++VertexIndex)
        {
            FVector Vert = MeshInfo.Vertices[VertexIndex];

            const FVertexID VertexID = MeshDescription.CreateVertex();
            VertexPositions[VertexID] = Vert;
            VertexIndexToVertexID.Add(VertexIndex, VertexID);
        }

        // Create the VertexInstance
        int32 NumIndices = MeshInfo.Triangles.Num();

        int32 NumTri = NumIndices / 3;
        TMap<int32, FVertexInstanceID> IndiceIndexToVertexInstanceID;
        IndiceIndexToVertexInstanceID.Reserve(NumVertex);
        for (int32 IndiceIndex = 0; IndiceIndex < NumIndices; IndiceIndex++)
        {
            const int32 VertexIndex = MeshInfo.Triangles[IndiceIndex];
            const FVertexID VertexID = VertexIndexToVertexID[VertexIndex];
            const FVertexInstanceID VertexInstanceID = MeshDescription.CreateVertexInstance(VertexID);
            IndiceIndexToVertexInstanceID.Add(IndiceIndex, VertexInstanceID);

            FVector ProcVertex = MeshInfo.Vertices[VertexIndex];    // FProcMeshVertex& ProcVertex =
                                                                    // ProcSection->ProcVertexBuffer[VertexIndex];
            FProcMeshTangent VertexTanents = MeshInfo.Tangents[VertexIndex];

            FLinearColor VertexColor = MeshInfo.VertexColors.Num() > VertexIndex ? MeshInfo.VertexColors[VertexIndex]
                                                                                 : FLinearColor(1.0, 0.0, 0.0);

            Tangents[VertexInstanceID] = VertexTanents.TangentX;          // ProcVertex.Tangent.TangentX;
            Normals[VertexInstanceID] = MeshInfo.Normals[VertexIndex];    // ProcVertex.Normal;
            BinormalSigns[VertexInstanceID] = VertexTanents.bFlipTangentY ? -1.f : 1.f;

            Colors[VertexInstanceID] = VertexColor;    // FLinearColor(ProcVertex.Color);

            if (MeshInfo.UV0.Num() > VertexIndex)
            {
                UVs.Set(VertexInstanceID, 0, MeshInfo.UV0[VertexIndex]);
            }

            if (MeshInfo.UV1.Num() > VertexIndex)
            {
                UVs.Set(VertexInstanceID, 1, MeshInfo.UV1[VertexIndex]);
            }

            if (MeshInfo.UV2.Num() > VertexIndex)
            {
                UVs.Set(VertexInstanceID, 2, MeshInfo.UV2[VertexIndex]);
            }

            if (MeshInfo.UV3.Num() > VertexIndex)
            {
                UVs.Set(VertexInstanceID, 3, MeshInfo.UV3[VertexIndex]);
            }
        }

        // Create the polygons for this section
        for (int32 TriIdx = 0; TriIdx < NumTri; TriIdx++)
        {
            FVertexID VertexIndexes[3];
            TArray<FVertexInstanceID> VertexInstanceIDs;
            VertexInstanceIDs.SetNum(3);

            for (int32 CornerIndex = 0; CornerIndex < 3; ++CornerIndex)
            {
                const int32 IndiceIndex = (TriIdx * 3) + CornerIndex;
                const int32 VertexIndex =
                    MeshInfo.Triangles[IndiceIndex];    // ProcSection->ProcIndexBuffer[IndiceIndex];
                VertexIndexes[CornerIndex] = VertexIndexToVertexID[VertexIndex];
                VertexInstanceIDs[CornerIndex] = IndiceIndexToVertexInstanceID[IndiceIndex];
            }

            // Insert a polygon into the mesh
            MeshDescription.CreatePolygon(PolygonGroupID, VertexInstanceIDs);
        }
    }
    return MeshDescription;
}

#pragma endregion copy

void URuntimeMeshLoader::Init()
{
    // LoadObject<UMaterialInterface>(NULL,
    // TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    // LoadObject<UMaterialInterface>(NULL, TEXT("Material'/Game/Art/Public/SH/SH_BtORMtNt.SH_BtORMtNt'"));
    UMaterialInterface* Mat_VehicleOpaque = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    UMaterialInterface* Mat_VehicleTransluate = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Art/Library/Vehicle/SUV_Haval_H7/Mat_H7_glass.Mat_H7_glass'"));
    UMaterialInterface* Mat_PedestrianOpaque = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    UMaterialInterface* Mat_PedestrianTransluate = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    UMaterialInterface* Mat_BikeOpaque = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    UMaterialInterface* Mat_BikeTransluate = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    UMaterialInterface* Mat_ObstacleOpaque = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    UMaterialInterface* Mat_ObstacleTransluate = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));
    UMaterialInterface* Mat_Others = LoadObject<UMaterialInterface>(
        NULL, TEXT("MaterialInstanceConstant'/Game/Blueprints/Material/Mat_CustomVehicle.Mat_CustomVehicle'"));

    StandardMaterials.Add(EMeshType::MT_VEHICLE, FMeshMaterialList(Mat_VehicleOpaque, Mat_VehicleTransluate));
    StandardMaterials.Add(EMeshType::MT_CREATURE, FMeshMaterialList(Mat_PedestrianOpaque, Mat_PedestrianTransluate));
    StandardMaterials.Add(EMeshType::MT_BIKE, FMeshMaterialList(Mat_BikeOpaque, Mat_BikeTransluate));
    StandardMaterials.Add(EMeshType::MT_OBSTACLE, FMeshMaterialList(Mat_ObstacleOpaque, Mat_ObstacleTransluate));
    StandardMaterials.Add(EMeshType::MT_MAP_OBJ, FMeshMaterialList(Mat_Others, Mat_Others));
    StandardMaterials.Add(EMeshType::MT_Other, FMeshMaterialList(Mat_Others, Mat_Others));
}

FReturnedData URuntimeMeshLoader::LoadMesh(const FString& filepath)
{
    FReturnedData result;
    result.bSuccess = false;
    result.meshInfo.Empty();
    result.NumMeshes = 0;

    if (filepath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Runtime Mesh Loader: filepath is empty.\n"));
        return result;
    }

    std::string file;
    file = TCHAR_TO_UTF8(*filepath);

    Assimp::Importer mImporter;

    const aiScene* mScenePtr =
        mImporter.ReadFile(file, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace |
                                     aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes);

    if (mScenePtr == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Runtime Mesh Loader: Read mesh file failure.\n"));
        return result;
    }

    if (mScenePtr->HasMeshes())
    {
        result.meshInfo.SetNum(mScenePtr->mNumMeshes, false);
        int32 MeshIdx = 0;
        FTransform T;
        T.SetRotation(FRotator(0.f, 0.f, 180.f).Quaternion());

        // 司机视角Socket
        std::string DriverSocketName = "SOCKET_Camera_DriverView";
        // 左车灯Socket
        std::string BeamLeftSocketName = "SOCKET_Lamp_HighBeam_L";
        // 右车灯Socket
        std::string BeamRightSocketName = "SOCKET_Lamp_HighBeam_R";
        TArray<std::string> SocketList;
        SocketList.Add(DriverSocketName);
        SocketList.Add(BeamLeftSocketName);
        SocketList.Add(BeamRightSocketName);
        for (auto& Elem : SocketList)
        {
            aiNode* node = mScenePtr->mRootNode->FindNode(Elem.c_str());
            if (node)
            {
                FMatrix tempMatrix;
                tempMatrix.M[0][0] = node->mTransformation.a1;
                tempMatrix.M[0][1] = node->mTransformation.b1;
                tempMatrix.M[0][2] = node->mTransformation.c1;
                tempMatrix.M[0][3] = node->mTransformation.d1;
                tempMatrix.M[1][0] = node->mTransformation.a2;
                tempMatrix.M[1][1] = node->mTransformation.b2;
                tempMatrix.M[1][2] = node->mTransformation.c2;
                tempMatrix.M[1][3] = node->mTransformation.d2;
                tempMatrix.M[2][0] = node->mTransformation.a3;
                tempMatrix.M[2][1] = node->mTransformation.b3;
                tempMatrix.M[2][2] = node->mTransformation.c3;
                tempMatrix.M[2][3] = node->mTransformation.d3;
                tempMatrix.M[3][0] = node->mTransformation.a4;
                tempMatrix.M[3][1] = node->mTransformation.b4;
                tempMatrix.M[3][2] = node->mTransformation.c4;
                tempMatrix.M[3][3] = node->mTransformation.d4;

                FVector Loc = T.TransformFVector4(FTransform(tempMatrix).GetLocation());
                result.SocketInfo.Add(TPair<FString, FVector>(UTF8_TO_TCHAR(Elem.c_str()), Loc * 100.f));
            }
        }

        FindMesh(mScenePtr, mScenePtr->mRootNode, result, T, MeshIdx);

        for (uint32 i = 0; i < mScenePtr->mNumMeshes; ++i)
        {
            // Triangle number
            for (uint32 l = 0; l < mScenePtr->mMeshes[i]->mNumFaces; ++l)
            {
                for (uint32 m = 0; m < mScenePtr->mMeshes[i]->mFaces[l].mNumIndices; ++m)
                {
                    result.meshInfo[i].Triangles.Push(mScenePtr->mMeshes[i]->mFaces[l].mIndices[m]);
                }
            }
        }

        result.bSuccess = true;
    }

    if (mScenePtr->HasMaterials())
    {
        for (int32 m = 0; m < (int32) mScenePtr->mNumMaterials; ++m)
        {
            aiMaterial* material = mScenePtr->mMaterials[m];
            aiString materialName;
            aiReturn ret;

            ret = material->Get(AI_MATKEY_NAME, materialName);
            if (ret != AI_SUCCESS)
                materialName = "";

            // Diffuse maps
            int numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
            aiString BasetextureName;
            aiString ORMtextureName;
            aiString NormaltextureName;
            // if (numTextures > 0)
            {
                bool IsValid = true;
                ret = material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), BasetextureName);
                if (ret == aiReturn_SUCCESS)
                {
                    std::string BaseFileName = BasetextureName.data;
                    result.BaseColorTexPath.Add(TPair<int32, FString>(m, UTF8_TO_TCHAR(BaseFileName.c_str())));
                }

                ret = material->Get(AI_MATKEY_TEXTURE(aiTextureType_METALNESS, 0), ORMtextureName);
                if (ret == aiReturn_SUCCESS)
                {
                    std::string ORMFileName = ORMtextureName.data;
                    result.ORMTexPath.Add(TPair<int32, FString>(m, UTF8_TO_TCHAR(ORMFileName.c_str())));
                }
                //
                ret = material->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), NormaltextureName);
                if (ret == aiReturn_SUCCESS)
                {
                    std::string NormalFileName = NormaltextureName.data;
                    result.NormalTexPath.Add(TPair<int32, FString>(m, UTF8_TO_TCHAR(NormalFileName.c_str())));
                }
                // ret = material->Get(AI_MATKEY_TEXTURE(aiTextureType_UNKNOWN, 0), textureName);
            }
        }
    }

    return result;
}

UStaticMesh* URuntimeMeshLoader::LoadStaticMeshFromFBX(const FString& FilePath, bool bTexByRef, EMeshType Type)
{
    if (FLoadMeshData* pLoadCache = LoadMeshCache.Find(FName(FilePath)))
    {
        if (pLoadCache->StaticMesh.IsValid())
        {
            return pLoadCache->StaticMesh.Get();
        }
    }

    FLoadMeshData LoadedCache;
    LoadedCache.LoadPath = FilePath;
    FString FileDir = FilePath.Replace(TEXT("\\"), TEXT("/"));
    int32 FindPos = 0;
    FileDir.FindLastChar('/', FindPos);
    FileDir = FileDir.Left(FindPos) + TEXT("/");

    FReturnedData&& MeshInfo = LoadMesh(FilePath);

    TMap<int32, UTexture2D*> MeshMaterialTex;
    for (int32 i = 0; i < MeshInfo.BaseColorTexPath.Num(); i++)
    {
        MeshInfo.BaseColorTexPath[i].Value.FindLastChar('\\', FindPos);
        int32 Length = MeshInfo.BaseColorTexPath[i].Value.Len();
        MeshInfo.BaseColorTexPath[i].Value = MeshInfo.BaseColorTexPath[i].Value.Right(Length - FindPos);
        UTexture2D* NewTexture = FImageUtils::ImportFileAsTexture2D(FileDir + MeshInfo.BaseColorTexPath[i].Value);
        if (NewTexture)
        {
            MeshMaterialTex.Add(i, NewTexture);
        }
    }

    FString NewNameSuggestion = FString(TEXT("RuntimeStaticMesh"));
    FString PackageName = FString(TEXT("/Game/Meshes/")) + NewNameSuggestion;
    FString Name;
    FString UserPackageName = TEXT("");
    FName MeshName(*FPackageName::GetLongPackageAssetName(UserPackageName));

    // Check if the user inputed a valid asset name, if they did not, give it the generated default name
    if (MeshName == NAME_None)
    {
        // Use the defaults that were already generated.
        UserPackageName = PackageName;
        MeshName = *Name;
    }

    FMeshDescription MeshDescription = BuildMeshDescriptionExtend(MeshInfo);

    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(this, MeshName, RF_Public | RF_Standalone);

    StaticMesh->InitResources();
    StaticMesh->LightingGuid = FGuid::NewGuid();
    TArray<const FMeshDescription*> arr;
    arr.Add(&MeshDescription);

    UStaticMesh::FBuildMeshDescriptionsParams mdParams;
    mdParams.bAllowCpuAccess = true;
    StaticMesh->BuildFromMeshDescriptions(arr, mdParams);

    for (auto& Elem : MeshInfo.SocketInfo)
    {
        UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(StaticMesh);
        check(Socket);

        Socket->SocketName = *Elem.Key;
        StaticMesh->AddSocket(Socket);
        Socket->RelativeLocation = Elem.Value;
    }

    FStaticMeshRenderData* const RenderData = StaticMesh->RenderData.Get();

    int32 LODIndex = 0;
    int32 MaxLODs = RenderData->LODResources.Num();

    for (; LODIndex < MaxLODs; ++LODIndex)
    {
        FStaticMeshLODResources& LOD = RenderData->LODResources[LODIndex];

        for (int32 SectionIndex = 0; SectionIndex < LOD.Sections.Num(); ++SectionIndex)
        {
            FStaticMeshSection& Section = LOD.Sections[SectionIndex];
            Section.MaterialIndex = SectionIndex;    // MaterialID;
            Section.bEnableCollision = true;
            Section.bCastShadow = true;
            Section.bForceOpaque = false;

            const int32 NumSections = 1;
            for (int32 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
            {
                UMaterialInterface* Material = nullptr;
                if (UTexture2D** pTex = MeshMaterialTex.Find(SectionIndex))
                {
                    UTexture2D* Tex = *pTex;
                    if (Tex)
                    {
                        FMeshMaterialList& MaterialList = StandardMaterials[Type];

                        UMaterialInstanceDynamic* MID = nullptr;
                        if (IsOpaqueTexture(Tex))
                        {
                            MID = UMaterialInstanceDynamic::Create(MaterialList.Mat_Opaque, this, FName(""));
                        }
                        else
                        {
                            MID = UMaterialInstanceDynamic::Create(MaterialList.Mat_Translucent, this, FName(""));
                        }

                        if (MID)
                        {
                            MID->SetTextureParameterValue(TEXT("BaseColor"), Tex);
                            LoadedCache.Texs.Add(Tex);
                        }
                        Material = MID;
                    }
                }

                FStaticMaterial&& StaticMat = FStaticMaterial(Material);
                StaticMat.UVChannelData.bInitialized = true;
                StaticMesh->StaticMaterials.Add(StaticMat);

                LoadedCache.MIDs.Add(SectionIdx, Material);
            }
        }
    }
    LoadedCache.StaticMesh = StaticMesh;
    LoadMeshCache.Add(FName(LoadedCache.LoadPath), LoadedCache);

    return StaticMesh;
}

bool URuntimeMeshLoader::IsOpaqueTexture(UTexture2D* Tex)
{
    FTexture2DMipMap* MyMipMap = &Tex->PlatformData->Mips[0];
    FByteBulkData* RawImageData = &MyMipMap->BulkData;
    // FColor* FormatedImageData = static_cast<FColor*>(RawImageData->Lock(LOCK_READ_ONLY));

    uint8* MipData = static_cast<uint8*>(Tex->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));
    FColor Test = FColor(MipData[0], MipData[1], MipData[2], MipData[3]);
    uint32 PixelX = 0, PixelY = 0;
    uint32 TextureWidth = MyMipMap->SizeX, TextureHeight = MyMipMap->SizeY;
    FColor PixelColor;
    FLinearColor LinearPixelColor;

    float AlphaStencil = 0.f;
    for (PixelX = 0; PixelX < TextureWidth; PixelX++)
    {
        for (PixelY = 0; PixelY < TextureHeight; PixelY++)
        {
            PixelColor = FColor(PixelX * PixelY + 3);
            LinearPixelColor = PixelColor.ReinterpretAsLinear();
            if (PixelX == 0 && PixelY == 0)
            {
                if (LinearPixelColor.A == 1.f || LinearPixelColor.A == 0.f)
                {
                    AlphaStencil = LinearPixelColor.A;
                }
                else
                {
                    Tex->PlatformData->Mips[0].BulkData.Unlock();
                    return false;
                }
            }
            else if (LinearPixelColor.A != AlphaStencil)
            {
                Tex->PlatformData->Mips[0].BulkData.Unlock();
                return false;
            }
        }
    }
    Tex->PlatformData->Mips[0].BulkData.Unlock();
    return true;
}
