// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomViewport.h"

#include "Engine/Canvas.h"
#include "CanvasItem.h"

UCustomViewport::UCustomViewport() : Super(FObjectInitializer::Get())
{
    MaxSplitscreenPlayers = 4;
}

void UCustomViewport::UpdateActiveSplitscreenType()
{
    Super::UpdateActiveSplitscreenType();
}

void UCustomViewport::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void UCustomViewport::LayoutPlayers()
{
    UpdateActiveSplitscreenType();

    const ESplitScreenType::Type SplitType = GetCurrentSplitscreenConfiguration();
    const TArray<ULocalPlayer*>& PlayerList = GetOuterUEngine()->GetGamePlayers(this);
    const size_t Player_Count = PlayerList.Num();

    if (Player_Count <= 0)
    {
        return;
    }

    TArray<FPlayerViews> Array_Views;

    if (Player_Count == 1)
    {
        if (!this->bIsInitialsLoaded || this->LastPlayerCount != Player_Count)
        {
            this->bIsInitialsLoaded = false;

            PlayerList[0]->Size = FVector2D(0.9f);
            PlayerList[0]->Origin = FVector2D(0.05);
            
            this->bIsInitialsLoaded = true;
            this->LastPlayerCount = Player_Count;
        }
        
        for (int32 PlayerIdx = 0; PlayerIdx < Player_Count; PlayerIdx++)
        {
            FPlayerViews View;
            View.Size = PlayerList[PlayerIdx]->Size;
            View.Position = (PlayerList[PlayerIdx]->Origin);
            Array_Views.Add(View);
        }

        DelegateNewLayout.Broadcast(Array_Views);
        return;
    }

    else if (Player_Count == 2)
    {
        if (!this->bIsInitialsLoaded || this->LastPlayerCount != Player_Count)
        {
            this->bIsInitialsLoaded = false;

            // Player 1 = Right

            PlayerList[0]->Size = FVector2D(0.425, 0.9);
            PlayerList[0]->Origin = FVector2D(0.525, 0.05);

            // Player 2 = Left

            PlayerList[1]->Size = FVector2D(0.425, 0.9);
            PlayerList[1]->Origin = FVector2D(0.05);

            this->bIsInitialsLoaded = true;
            this->LastPlayerCount = Player_Count;
        }

        for (int32 PlayerIdx = 0; PlayerIdx < Player_Count; PlayerIdx++)
        {
            FPlayerViews View;
            View.Size = PlayerList[PlayerIdx]->Size;
            View.Position = (PlayerList[PlayerIdx]->Origin);
            Array_Views.Add(View);
        }

        DelegateNewLayout.Broadcast(Array_Views);
        return;
    }

    else if (Player_Count == 3)
    {
        if (!this->bIsInitialsLoaded || this->LastPlayerCount != Player_Count)
        {
            this->bIsInitialsLoaded = false;

            // Player 1 = Right

            PlayerList[0]->Size = FVector2D(0.425, 0.9);
            PlayerList[0]->Origin = FVector2D(0.525, 0.5);

            // Player 2 = Top Left

            PlayerList[1]->Size = FVector2D(0.425);
            PlayerList[1]->Origin = FVector2D(0.05);

            //Player 3 = Bottom Left

            PlayerList[2]->Size = FVector2D(0.425);
            PlayerList[2]->Origin = FVector2D(0.05, 0.525);

            this->bIsInitialsLoaded = true;
            this->LastPlayerCount = Player_Count;
        }

        for (int32 PlayerIdx = 0; PlayerIdx < Player_Count; PlayerIdx++)
        {
            FPlayerViews View;
            View.Size = PlayerList[PlayerIdx]->Size;
            View.Position = (PlayerList[PlayerIdx]->Origin);
            Array_Views.Add(View);
        }

        DelegateNewLayout.Broadcast(Array_Views);
        return;
    }

    else if (Player_Count == 4)
    {
        if (!this->bIsInitialsLoaded || this->LastPlayerCount != Player_Count)
        {
            this->bIsInitialsLoaded = false;

            // Player 1 = Bottom Right

            PlayerList[0]->Size = FVector2D(0.425);
            PlayerList[0]->Origin = FVector2D(0.525);

            // Player 2 = Top Right

            PlayerList[1]->Size = FVector2D(0.425);
            PlayerList[1]->Origin = FVector2D(0.525, 0.05);

            // Player 3 = Top Left

            PlayerList[2]->Size = FVector2D(0.425);
            PlayerList[2]->Origin = FVector2D(0.05);

            // Player 4 = Bottom Left

            PlayerList[3]->Size = FVector2D(0.425);
            PlayerList[3]->Origin = FVector2D(0.05, 0.525);

            this->bIsInitialsLoaded = true;
            this->LastPlayerCount = Player_Count;
        }

        for (int32 PlayerIdx = 0; PlayerIdx < Player_Count; PlayerIdx++)
        {
            FPlayerViews View;
            View.Size = PlayerList[PlayerIdx]->Size;
            View.Position = (PlayerList[PlayerIdx]->Origin);
            Array_Views.Add(View);
        }

        DelegateNewLayout.Broadcast(Array_Views);
        return;
    }

    else if (Player_Count > 4)
    {
        UE_LOG(LogTemp, Fatal, TEXT("Player count shouldn't exceed 4. Requested number = %d"), Player_Count);
        return;
    }
}

void UCustomViewport::Draw(FViewport* In_Viewport, FCanvas* In_SceneCanvas)
{
    Super::Draw(In_Viewport, In_SceneCanvas);

    if (IsValid(this->BG_Material) && this->BG_Material->GetRenderProxy() && !this->bStopBackground)
    {
        const FVector2D RectSize = In_Viewport->GetSizeXY();
        const FVector2D UV_Top_Left = FVector2D(0.0f, 0.0f);
        const FVector2D UV_Bottom_Right = FVector2D(1.0f, 1.0f);

        FCanvasTileItem TileItem(FVector2D(0, 0), this->BG_Material->GetRenderProxy(), RectSize, UV_Top_Left, UV_Bottom_Right);
        TileItem.BlendMode = SE_BLEND_Opaque;

        In_SceneCanvas->DrawItem(TileItem);
    }
}

bool UCustomViewport::ChangePlayerViewSize(const int32 PlayerId, FVector2D NewRatio, FVector2D NewOrigin)
{
    UEngine* const REF_Engine = GameInstance->GetEngine();
    const int32 NumPlayers = REF_Engine->GetNumGamePlayers(this);

    if (NumPlayers > PlayerId + 1)
    {
        return false;
    }

    const TArray<ULocalPlayer*>& PlayerList = GetOuterUEngine()->GetGamePlayers(this);
    PlayerList[PlayerId]->Size = NewRatio;
    PlayerList[PlayerId]->Origin = NewOrigin;

    return true;
}

bool UCustomViewport::SetBackgrounMaterial(UMaterialInterface* In_Material)
{
    if (!IsValid(In_Material))
    {
        return false;
    }

    this->BG_Material = In_Material;
    return true;
}

UMaterialInterface* UCustomViewport::GetBackgroundMaterial()
{
    if (!IsValid(this->BG_Material))
    {
        return nullptr;
    }

    return this->BG_Material;
}

void UCustomViewport::ToggleBackground(bool bStop)
{
    this->bStopBackground = bStop;
}