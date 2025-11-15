// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/GameViewportClient.h"
#include "Engine/ViewportSplitScreen.h"
#include "WindowSystem_Includes.h"

#include "CustomViewport.generated.h"

USTRUCT(BlueprintType)
struct WINDOWSYSTEM_API FPlayerViews
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite)
    FVector2D Size = FVector2D();

    UPROPERTY(BlueprintReadWrite)
    FVector2D Position = FVector2D();
};

DECLARE_MULTICAST_DELEGATE_OneParam(FDelegateNewLayout, const TArray<FPlayerViews>&);

UCLASS()
class WINDOWSYSTEM_API UCustomViewport : public UGameViewportClient
{
	GENERATED_BODY()

private:

    const int32 FrameThickness = 10;

    UPROPERTY()
    FName CRT_Name = "Canvas";

    UPROPERTY()
    UMaterialInterface* MAT_BG = nullptr;

    UPROPERTY()
    UMaterialInterface* MAT_Cut = nullptr;

    UPROPERTY()
    UMaterialInterface* MAT_Frame = nullptr;

    UPROPERTY()
    UCanvasRenderTarget2D* CRT = nullptr;

    UPROPERTY()
    UMaterialInstanceDynamic* MI_BG = nullptr;

    UPROPERTY()
    TArray<FPlayerViews> View_Ratios;

	// We use this to compare if there is a change in views on background calculation.
    UPROPERTY()
    TMap<FVector2D, FVector2D> Old_View;
 
    UFUNCTION()
    virtual void UpdateCRTColor(UCanvas* Canvas, int32 Width, int32 Height);

    virtual bool ComparePixels(TMap<FVector2D, FVector2D> A, TMap<FVector2D, FVector2D> B);
    virtual void CalculateBackground(FViewport* In_Viewport, FCanvas* In_SceneCanvas);

protected:

    // If there is a new player, we need to reset views.
    int32 LastPlayerCount = 0;

    // Views shouldn't reset to defaults if there is no change.
    bool bIsInitialsLoaded = false;

    // We use this to forcefully stop background rendering. For example there is only one view and it is in full screen state.
    bool bActivateBackground = true;

public:

	UCustomViewport();

    virtual void Tick(float DeltaTime) override;
    virtual void UpdateActiveSplitscreenType() override;
    virtual void LayoutPlayers() override;
    virtual void Draw(FViewport* In_Viewport, FCanvas* In_SceneCanvas) override;

    FDelegateNewLayout DelegateNewLayout;
	FVector2D FrameTarget = FVector2D::ZeroVector;

    virtual bool ChangePlayerViewSize(const int32 PlayerId, FVector2D NewRatio, FVector2D NewOrigin);
	virtual bool SetBackgroundMaterial(UMaterialInterface* In_MAT_BG, UMaterialInterface* In_MAT_Cut, UMaterialInterface* In_MAT_Frame, FName In_CRT_Name = "Canvas");
    virtual void ToggleBackground(bool bActive = true);
    virtual void InitTextures();

};