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

    UPROPERTY()
    FName CRT_Name;

    UPROPERTY()
    UMaterialInterface* MAT_BG = nullptr;

    UPROPERTY()
    UMaterialInterface* MAT_Brush = nullptr;

    UPROPERTY()
    TArray<FPlayerViews> View_Ratios;

    UPROPERTY()
    TMap<FVector2D, FVector2D> Old_View;

    UPROPERTY()
    UCanvasRenderTarget2D* CRT = nullptr;

    UPROPERTY()
    UMaterialInstanceDynamic* MI_BG = nullptr;
    
    virtual bool ComparePixels(TMap<FVector2D, FVector2D> A, TMap<FVector2D, FVector2D> B);
    virtual void InitTextures();
    virtual void CalculateBackground(FViewport* In_Viewport, FCanvas* In_SceneCanvas);

protected:

    // If there is a new player, we need to reset views.
    int32 LastPlayerCount = 0;

    // Views shouldn't reset to defaults if there is no change.
    bool bIsInitialsLoaded = false;

    // We use this to forcefully stop background rendering. For example there is only one view and it is in full screen state.
    bool bStopBackground = false;


public:

	UCustomViewport();

    virtual void Tick(float DeltaTime) override;
    virtual void UpdateActiveSplitscreenType() override;
    virtual void LayoutPlayers() override;
    virtual void Draw(FViewport* In_Viewport, FCanvas* In_SceneCanvas) override;

    FDelegateNewLayout DelegateNewLayout;

    virtual bool ChangePlayerViewSize(const int32 PlayerId, FVector2D NewRatio, FVector2D NewOrigin);
	virtual bool SetBackgroundMaterial(UMaterialInterface* In_MAT_BG, UMaterialInterface* In_MAT_Brush, FName In_CRT_Name);
    virtual void ToggleBackground(bool bStop);

};