// Fill out your copyright notice in the Description page of Project Settings.

#include "DisplayHUD.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "Misc/ConfigCacheIni.h"

ADisplayHUD::ADisplayHUD()
{
    GConfig->GetBool(TEXT("UI"), TEXT("bShowNetMode"), bShowNetMode, GGameIni);
}

void ADisplayHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!DebugCanvas)
    {
        return;
    }

    if (bShowNetMode)
    {
        FString Text;
        switch (GetNetMode())
        {
            case ENetMode::NM_Standalone:
                Text = TEXT("Standalone");
                break;
            case ENetMode::NM_DedicatedServer:
                Text = TEXT("DedicatedServer");
                break;
            case ENetMode::NM_ListenServer:
                Text = TEXT("ListenServer");
                break;
            case ENetMode::NM_Client:
                Text = TEXT("Client");
                break;
            case ENetMode::NM_MAX:
                Text = TEXT("MAX");
                break;
        }

        FFontRenderInfo FontRenderInfo = Canvas->CreateFontRenderInfo(false, true);
        UFont* RenderFont = GEngine->GetSmallFont();
        DebugCanvas->SetDrawColor(64, 64, 255, 255);
        float xl, yl;
        DebugCanvas->StrLen(RenderFont, Text, xl, yl);
        float X = DebugCanvas->SizeX * 0.05f;
        float Y = yl;    //*1.67;
        yl += 2 * Y;

        DebugCanvas->DrawText(RenderFont, Text, X, yl, 1.f, 1.f, FontRenderInfo);
    }
}
