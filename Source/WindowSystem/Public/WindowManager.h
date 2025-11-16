// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

// Custom Includes.
#include "DragDropHandler.h"

#include "WindowManager.generated.h"

class AEachWindow_SWindow;

// File drag drop system.
USTRUCT(BlueprintType)
struct WINDOWSYSTEM_API FDroppedFileStruct
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FString FilePath;

	UPROPERTY(BlueprintReadWrite)
	FVector2D DropLocation = FVector2D();

	UPROPERTY(BlueprintReadWrite)
	bool bIsFolder = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateFileDrop, const TArray<FDroppedFileStruct>&, OutMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateWindowClosed, const FName&, WindowTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateWindowMoved, AEachWindow_SWindow* const&, Window);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateWindowHovered, AEachWindow_SWindow* const&, OutHovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateLayoutChanged, const TArray<FPlayerViews>&, Out_Views);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateLMBHook, FVector2D, Out_Position, FLinearColor, Out_Color);

UCLASS()
class WINDOWSYSTEM_API UFF_WindowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:

	UPROPERTY()
	UCustomViewport* CustomViewport = nullptr;

	UPROPERTY()
	AEachWindow_SWindow* HoveredWindow = nullptr;

	FDelegateHandle WorldTickStartHandle;
	FDragDropHandler DragDropHandler;

	// Reference for SetMouseHookEx lambda functor.
	static TWeakObjectPtr<UFF_WindowSubsystem> SelfReference;
	HHOOK Hook_LMB = NULL;

	virtual void AddDragDropHandlerToMV();
	virtual void RemoveDragDropHandlerFromMV();
	virtual void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime);

	UFUNCTION()
	virtual void OnLayoutChanged(const TArray<FPlayerViews>& In_Views);

	UFUNCTION()
	virtual void OnViewportDetected(FVector2D In_Position, FLinearColor In_Color);

public:

	UPROPERTY()
	TMap<FName, AEachWindow_SWindow*> MAP_Windows;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "It allows main window to support file drag drop.", ExposeOnSpawn = "true"), Category = "Frozen Forest|Window System|Window")
	bool bAllowMainWindow = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Frozen Forest|Window System|Wiewport")
	FName CanvasName = TEXT("Canvas");

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateLMBHook DelegateLMBHook;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Viewport|Events")
	FDelegateLayoutChanged DelegateLayoutChanged;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateFileDrop OnFileDrop;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateWindowClosed OnWindowClosed;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateWindowMoved OnWindowMoved;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateWindowHovered OnWindowHovered;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Close All Windows", Keywords = "close, all, window"), Category = "Frozen Forest|Window System|Window")
	virtual bool CloseAllWindows();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Window State", ToolTip = "", Keywords = "toggle, switch, window, state, minimize, restore, maximize"), Category = "Frozen Forest|Window System|Window")
	virtual bool ToggleWindowState(FName InTargetWindow, bool bFlashWindow);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bring Front on Hover", ToolTip = "", Keywords = "hover, system, bring, window, front"), Category = "Frozen Forest|Window System|Window")
	virtual bool BringFrontOnHover(AEachWindow_SWindow* TargetWindow);

	UFUNCTION(BlueprintCallable)
	virtual void InitMouseHook();

};