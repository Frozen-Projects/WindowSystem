#include "Viewport/CustomViewport.h"

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

void UCustomViewport::LayoutPlayers()
{
    UpdateActiveSplitscreenType();

    const TArray<ULocalPlayer*>& PlayerList = GetOuterUEngine()->GetGamePlayers(this);
    const int32 Player_Count = PlayerList.Num();

    if (!UKismetMathLibrary::InRange_IntInt(Player_Count, 1, 4, true, true))
    {
        this->LastPlayerCount = 0;
        this->bIsInitialsLoaded = false;
        this->View_Ratios.Reset();
        this->Old_View.Reset();
        this->CRT->ClearColor = FLinearColor::White;
        this->FrameTarget = FVector2D::ZeroVector;

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

        if (this->View_Ratios != Temp_Views)
        {
            //this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);
            
            this->DelegateNewLayout.Broadcast(this->View_Ratios);
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

        if (this->View_Ratios != Temp_Views)
        {
            //this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);

            this->DelegateNewLayout.Broadcast(this->View_Ratios);
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

        if (this->View_Ratios != Temp_Views)
        {
           // this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);
            
            this->DelegateNewLayout.Broadcast(this->View_Ratios);
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

        if (this->View_Ratios != Temp_Views)
        {
            //this->FrameTarget = FVector2D::ZeroVector;
            this->View_Ratios = MoveTemp(Temp_Views);
           
            this->DelegateNewLayout.Broadcast(this->View_Ratios);
        }

        return;
    }
}

void UCustomViewport::Draw(FViewport* In_Viewport, FCanvas* In_SceneCanvas)
{
    Super::Draw(In_Viewport, In_SceneCanvas);
    this->CalculateBackground(In_Viewport, In_SceneCanvas);
}

void UCustomViewport::UpdateAssets()
{
    if (!IsValid(CRT) || !IsValid(MAT_BG) || !IsValid(MAT_Cut) || !IsValid(MAT_Highlight) || !IsValid(GetWorld()))
    {
        return;
    }

    this->CRT->UpdateResource();

    FDrawToRenderTargetContext Context;
    UCanvas* Canvas = nullptr;
    FVector2D Size;
    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this->World, this->CRT, Canvas, Size, Context);

    if (!IsValid(Canvas))
    {
        UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this->World, Context);
        return;
    }

    for (const FPlayerViews& Each_View : this->Old_View)
    {
        // Draw Frame first to avoid overlapping issues.

        if (this->FrameTarget == Each_View.Position)
        {
            Canvas->K2_DrawMaterial(this->MAT_Highlight, Each_View.Position + FVector2D(-(this->HighLightThickness / 2)), Each_View.Size + FVector2D(this->HighLightThickness), FVector2D(0.f), FVector2D(1.f));
        }

        // Then Cut.
        Canvas->K2_DrawMaterial(this->MAT_Cut, Each_View.Position, Each_View.Size, FVector2D(0.f), FVector2D(1.f));
    }

    UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this->World, Context);
}

void UCustomViewport::CalculateBackground(FViewport* In_Viewport, FCanvas* In_SceneCanvas)
{
    if (!this->bActivateBackground)
    {
        return;
    }

    if (!IsValid(this->World) || !IsValid(this->MAT_BG) || !IsValid(this->MAT_Cut) || !IsValid(this->MAT_Highlight) || CRT_Name.IsNone())
    {
        return;
    }

    const FVector2D ViewportSize = FVector2D(In_Viewport->GetSizeXY());
    if (ViewportSize.X == 0 || ViewportSize.Y == 0)
	{
        return;
	}

    bool bNeedsUpdate = false;

    if (this->CRT->SizeX != ViewportSize.X || this->CRT->SizeY != ViewportSize.Y)
    {
		this->CRT->ResizeTarget(ViewportSize.X, ViewportSize.Y);
        bNeedsUpdate = true;
    }

    TArray<FPlayerViews> Temp_Views;
    for (const FPlayerViews& Each_View : this->View_Ratios)
    {
        FPlayerViews NewView;
        NewView.Position = ViewportSize * Each_View.Position;
        NewView.Size = ViewportSize * Each_View.Size;
        Temp_Views.Add(MoveTemp(NewView));
    }

    if (Temp_Views != this->Old_View)
    {
        this->Old_View = MoveTemp(Temp_Views);
        bNeedsUpdate = true;
	}

    const FGeometry ViewportGeometry = UWidgetLayoutLibrary::GetViewportWidgetGeometry(World);
    const FVector2D LocalViewportSize = ViewportGeometry.GetLocalSize();

    if (this->Old_ViewportSize != LocalViewportSize)
    {
		this->Old_ViewportSize = LocalViewportSize;
        bNeedsUpdate = true;
    }

    if (bNeedsUpdate)
    {
        this->UpdateAssets();
    }

    FCanvasTileItem TileItem(FVector2D(0, 0), this->MI_BG->GetRenderProxy(), ViewportSize, FVector2D(0.f, 0.f), FVector2D(1.f, 1.f));
    TileItem.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;

    In_SceneCanvas->DrawItem(TileItem);
}

void UCustomViewport::ToggleBackground(bool bActive)
{
    this->bActivateBackground = bActive;
}

bool UCustomViewport::ChangePlayerViewSize(const int32 PlayerId, FVector2D NewRatio, FVector2D NewOrigin)
{
    const TArray<ULocalPlayer*>& PlayerList = GetOuterUEngine()->GetGamePlayers(this);

    if (!PlayerList.IsValidIndex(PlayerId))
    {
        return false;
    }

    PlayerList[PlayerId]->Size = NewRatio;
    PlayerList[PlayerId]->Origin = NewOrigin;

    return true;
}

bool UCustomViewport::SetBackgroundMaterial(UMaterialInterface* In_MAT_BG, UMaterialInterface* In_MAT_Cut, UMaterialInterface* In_MAT_Highlight, FName In_CRT_Name, int32 In_Thickness)
{
    if (!IsValid(In_MAT_BG) || !IsValid(In_MAT_Cut) || !IsValid(In_MAT_Highlight) || In_CRT_Name.IsNone())
    {
        return false;
    }

	this->MAT_BG = In_MAT_BG;
	this->MAT_Cut = In_MAT_Cut;
	this->MAT_Highlight = In_MAT_Highlight;
	this->CRT_Name = In_CRT_Name;
	this->HighLightThickness = In_Thickness < 10 ? 10 : In_Thickness;
    
    this->MI_BG = UMaterialInstanceDynamic::Create(this->MAT_BG, this->World);
    this->MI_BG->SetTextureParameterValue(this->CRT_Name, this->CRT);
	
    this->UpdateAssets();

	return true;
}