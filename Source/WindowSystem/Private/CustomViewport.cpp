// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomViewport.h"

#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Materials/MaterialRenderProxy.h"

UCustomViewport::UCustomViewport() : Super(FObjectInitializer::Get())
{
    MaxSplitscreenPlayers = 4;

    this->CRT = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this->World, UCanvasRenderTarget2D::StaticClass(), 1920, 1080);
    this->CRT->OnCanvasRenderTargetUpdate.AddDynamic(this, &UCustomViewport::UpdateCRTColor);
}

void UCustomViewport::UpdateCRTColor(UCanvas* Canvas, int32 Width, int32 Height)
{
    if (Canvas && Canvas->Canvas)
    {
        Canvas->Canvas->Clear(FLinearColor::White); // Clear with specified color
    }
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

    TArray<FPlayerViews> Temp_Views;

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
            Temp_Views.Add(View);
        }

        if (this->View_Ratios.Num() != Temp_Views.Num())
        {
            this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);
            DelegateNewLayout.Broadcast(this->View_Ratios);
        }

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
            Temp_Views.Add(View);
        }

        if (this->View_Ratios.Num() != Temp_Views.Num())
        {
            this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);
            DelegateNewLayout.Broadcast(this->View_Ratios);
        }

        return;
    }

    else if (Player_Count == 3)
    {
        if (!this->bIsInitialsLoaded || this->LastPlayerCount != Player_Count)
        {
            this->bIsInitialsLoaded = false;

            // Player 1 = Right

            PlayerList[0]->Size = FVector2D(0.425, 0.9);
            PlayerList[0]->Origin = FVector2D(0.525, 0.05);

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
            Temp_Views.Add(View);
        }

        if (this->View_Ratios.Num() != Temp_Views.Num())
        {
            this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);
            DelegateNewLayout.Broadcast(this->View_Ratios);
        }

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
            Temp_Views.Add(View);
        }

        if (this->View_Ratios.Num() != Temp_Views.Num())
        {
            this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);
            DelegateNewLayout.Broadcast(this->View_Ratios);
        }

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
    this->CalculateBackground(In_Viewport, In_SceneCanvas);
}

bool UCustomViewport::ComparePixels(TMap<FVector2D, FVector2D> A, TMap<FVector2D, FVector2D> B)
{
    if (A.Num() != B.Num())
    {
        return false;
    }

    TArray<FVector2D> A_Keys;
    A.GenerateKeyArray(A_Keys);

    TArray<FVector2D> A_Values;
    A.GenerateValueArray(A_Values);

    TArray<FVector2D> B_Keys;
    B.GenerateKeyArray(B_Keys);

    TArray<FVector2D> B_Values;
    B.GenerateValueArray(B_Values);

    if (A_Keys == B_Keys && A_Values == B_Values)
    {
        return true;
    }

    else
    {
        return false;
    }
}

void UCustomViewport::UpdateAssets()
{
    if (IsValid(this->CRT))
    {
        this->CRT->UpdateResource();
    }

    this->MI_BG = UMaterialInstanceDynamic::Create(this->MAT_BG, this->World);
    this->MI_BG->SetTextureParameterValue(this->CRT_Name, this->CRT);
}

void UCustomViewport::CalculateBackground(FViewport* In_Viewport, FCanvas* In_SceneCanvas)
{
    if (!this->bActivateBackground)
    {
        return;
    }

    if (!IsValid(this->World) || !IsValid(this->MAT_BG) || !IsValid(this->MAT_Cut) || !IsValid(this->MAT_Frame) || CRT_Name.IsNone())
    {
        return;
    }

    const FVector2D ViewportSize = FVector2D(In_Viewport->GetSizeXY());
    if (ViewportSize.X == 0 || ViewportSize.Y == 0)
	{
        return;
	}

    if (this->CRT->SizeX != ViewportSize.X || this->CRT->SizeY != ViewportSize.Y)
    {
		this->CRT->ResizeTarget(ViewportSize.X, ViewportSize.Y);
    }

    TMap<FVector2D, FVector2D> Temp_Views;
    for (const FPlayerViews Each_View : this->View_Ratios)
    {
        const FVector2D ActualPosition = ViewportSize * Each_View.Position;
        const FVector2D ActualSize = ViewportSize * Each_View.Size;
        Temp_Views.Add(ActualPosition, ActualSize);
    }

    if (!this->ComparePixels(Temp_Views, this->Old_View))
    {
        this->Old_View = Temp_Views;
		this->UpdateAssets();
	}

    FDrawToRenderTargetContext Context;
    UCanvas* Canvas = nullptr;
    FVector2D Size;
    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this->World, this->CRT, Canvas, Size, Context);

    for (const TPair<FVector2D, FVector2D>& Each_View : this->Old_View)
    {
		// Draw Frame first to avoid overlapping issues.

        if (this->FrameTarget == Each_View.Key)
        {
            Canvas->K2_DrawMaterial(this->MAT_Frame, Each_View.Key + FVector2D(-(this->FrameThickness / 2)), Each_View.Value + FVector2D(this->FrameThickness), FVector2D(0.f), FVector2D(1.f));
        }

		// Then Cut.
        Canvas->K2_DrawMaterial(this->MAT_Cut, Each_View.Key, Each_View.Value, FVector2D(0.f), FVector2D(1.f));
    }

    UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this->World, Context);

    FCanvasTileItem TileItem(FVector2D(0, 0), this->MI_BG->GetRenderProxy(), ViewportSize, FVector2D(0.f, 0.f), FVector2D(1.f, 1.f));
    TileItem.BlendMode = SE_BLEND_Opaque;

    In_SceneCanvas->DrawItem(TileItem);
}

void UCustomViewport::ToggleBackground(bool bActive)
{
    this->bActivateBackground = bActive;
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

bool UCustomViewport::SetBackgroundMaterial(UMaterialInterface* In_MAT_BG, UMaterialInterface* In_MAT_Cut, UMaterialInterface* In_MAT_Frame, FName In_CRT_Name, int32 In_Thickness)
{
    if (!IsValid(In_MAT_BG) || !IsValid(In_MAT_Cut) || !IsValid(In_MAT_Frame) || In_CRT_Name.IsNone())
    {
        return false;
    }

	this->MAT_BG = In_MAT_BG;
	this->MAT_Cut = In_MAT_Cut;
	this->MAT_Frame = In_MAT_Frame;
	this->CRT_Name = In_CRT_Name;
	this->FrameThickness = In_Thickness < 10 ? 10 : In_Thickness;

	this->UpdateAssets();

	return true;
}