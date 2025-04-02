// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelegateCursorPosColor, const FVector2D&, Position, const FLinearColor&, Color);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateLayoutChanged, const TArray<FPlayerViews>&, Out_Views);

UCLASS()
class WINDOWSYSTEM_API UFF_WindowSubystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:

#pragma region Viewport

	UCustomViewport* CustomViewport = nullptr;
	TMap<FVector2D, FVector2D> MAP_Views;

	UFUNCTION()
	virtual void DetectLayoutChanges();

	UFUNCTION()
	virtual void ChangeBackgroundOnNewPlayer(TArray<FPlayerViews> const& Out_Views);

	UFUNCTION()
	virtual bool CompareViews(TMap<FVector2D, FVector2D> A, TMap<FVector2D, FVector2D> B);

#pragma endregion Viewport

#pragma region Drag_Drop

	FDragDropHandler DragDropHandler;
	
	UFUNCTION()
	virtual void AddDragDropHandlerToMV();

	UFUNCTION()
	virtual void RemoveDragDropHandlerFromMV();

#pragma endregion Drap_Drop

#pragma region Color_Picker
	HHOOK MouseHook_Color = NULL;
#pragma endregion Color_Picker

#pragma region Window_System
	AEachWindow_SWindow* HoveredWindow = nullptr;
#pragma endregion Window_System

	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime);

public:

	TMap<FName, AEachWindow_SWindow*> MAP_Windows;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ToolTip = "It allows main window to support file drag drop.", ExposeOnSpawn = "true"), Category = "Frozen Forest|Window System|Window")
	bool bAllowMainWindow = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Frozen Forest|Window System|Viewport")
	UMaterialInterface* MAT_BG = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Frozen Forest|Window System|Wiewport")
	UMaterialInterface* MAT_Brush = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Frozen Forest|Window System|Wiewport")
	FName CanvasName = TEXT("Canvas");

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Frozen Forest|Window System|Wiewport")
	FString LastError;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Frozen Forest|Window System|Wiewport")
	TMap<FString, FVector2D> ViewLayout;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateFileDrop OnFileDrop;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateWindowClosed OnWindowClosed;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateWindowMoved OnWindowMoved;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateWindowHovered OnWindowHovered;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Window|Events")
	FDelegateCursorPosColor OnCursorPosColor;

	UPROPERTY(BlueprintAssignable, Category = "Frozen Forest|Window System|Viewport|Events")
	FDelegateLayoutChanged OnLayoutChanged;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Close All Windows", Keywords = "close, all, window"), Category = "Frozen Forest|Window System|Window")
	virtual bool CloseAllWindows();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Window State", ToolTip = "", Keywords = "toggle, switch, window, state, minimize, restore, maximize"), Category = "Frozen Forest|Window System|Window")
	virtual bool ToggleWindowState(FName InTargetWindow, bool bFlashWindow);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bring Front on Hover", ToolTip = "", Keywords = "hover, system, bring, window, front"), Category = "Frozen Forest|Window System|Window")
	virtual bool BringFrontOnHover(AEachWindow_SWindow* TargetWindow);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Color Picker", ToolTip = "It will give cursor position and color under cursor.", Keywords = "cursor, mouse, color, pixel, position, location"), Category = "Frozen Forest|Window System|Window")
	virtual void Toggle_Color_Picker();

	UFUNCTION(BlueprintPure)
	virtual bool IsColorPickerActive();

	UFUNCTION(BlueprintPure)
	virtual FString ViewLayoutLog();

};