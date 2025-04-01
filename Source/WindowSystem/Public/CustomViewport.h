// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/GameViewportClient.h"
#include "Engine/ViewportSplitScreen.h"

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

protected:

    // If there is a new player, we need to reset views.
    int32 LastPlayerCount = 0;

    // Views shouldn't reset to defaults if there is no change.
    bool bIsInitialsLoaded = false;

    // We use this to forcefully stop background rendering. For example there is only one view and it is in full screen state.
    bool bStopBackground = false;

    UMaterialInterface* BG_Material = nullptr;

public:

	UCustomViewport();

    virtual void Tick(float DeltaTime) override;

    virtual void UpdateActiveSplitscreenType() override;

    virtual void LayoutPlayers() override;

    virtual void Draw(FViewport* In_Viewport, FCanvas* In_SceneCanvas) override;

    FDelegateNewLayout DelegateNewLayout;

    // This is customized version of UGameViewportClient::SSSwapControllers which works on Shipping Builds.
    virtual bool PossesLocalPlayer(const int32 PlayerId, const int32 ControllerId = -1);

    virtual bool ChangePlayerViewSize(const int32 PlayerId, FVector2D NewRatio, FVector2D NewOrigin);
    virtual bool SetBackgrounMaterial(UMaterialInterface* In_Material);
    virtual UMaterialInterface* GetBackgroundMaterial();
    virtual void ToggleBackground(bool bStop);

};