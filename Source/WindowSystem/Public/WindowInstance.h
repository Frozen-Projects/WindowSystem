// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

// Custom Includes.
#include "WindowManager.h"

#include "WindowInstance.generated.h"

UCLASS()
class WINDOWSYSTEM_API AEachWindow_SWindow : public AActor
{
	GENERATED_BODY()
	
private:

	UPROPERTY()
	UFF_WindowSubsystem* WindowSubsystem = nullptr;

protected:

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;

	// Called when the game ends.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Window Close Delegate.
	void NotifyWindowClosed(const TSharedRef<SWindow>& Window);

	// Window Movement Delegate.
	void NotifyWindowMoved(const TSharedRef<SWindow>& Window);

	// We use it with Timer_Hover.
	virtual void NotifyWindowHovered(bool bUseDirectHover);

	// We use it to construct window.
	virtual bool CreateNewWindow();

	// We use it to destroy contents of window.
	virtual void CloseWindowCallback();

	FTimerHandle Hover_Timer;

	FTimerDelegate Hover_Delegate;

public:

	// Sets default values for this actor's properties.
	AEachWindow_SWindow();

	// Called every frame.
	virtual void Tick(float DeltaTime) override;

	TSharedPtr<SWindow> WindowPtr;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	bool bIsTransparent = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = ""))
	bool bIsFileDropEnabled = false;

	UPROPERTY(BlueprintReadWrite, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	UUserWidget* ContentWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	EWindowTypeBp WindowTypeBp = EWindowTypeBp::Normal;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FName WindowTag = NAME_None;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FText InWindowTitle = INVTEXT("None");

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FText InToolTip = INVTEXT("None");

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "If you hide your taskbar, you have a chance to lost your window when it goes to backward. In that case, use \"Bring Window Front\" function with some delay.", ExposeOnSpawn = "true"))
	bool bShowOnTaskBar = true;

	UPROPERTY(BlueprintReadWrite, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bIsTopMost = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "If you close your window, you will lost your widget and its contents.", ExposeOnSpawn = "true"))
	bool bHasClose = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bForceVolatile = true;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bPreserveAspectRatio = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bMinimized = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bSupportsMaximized = true;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bSupportsMinimized = true;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bSetMirrorWindow = true;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bUseNativeBorder = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	bool bEnableHoverDetection = false;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	float HoverDetectionTime = 0.03;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FLinearColor TitleColor = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FVector2D WindowSize = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FVector2D MinSize = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FVector2D MaxSize = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FVector2D StartPosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "", ExposeOnSpawn = "true"))
	FMargin BorderThick = FMargin(5.f);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set File Drag Drop Support", Keywords = "set, file, drag, drop, support, child, window, windows"), Category = "Frozen Forest|Window System|Set")
	virtual bool SetFileDragDropSupport();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Take Screenshot of Window", ToolTip = "Export To Disk functions should come after a delay node.", Keywords = "take, ss, screenshot, window"), Category = "Frozen Forest|Window System|Export")
	virtual bool TakeSSWindow(UTextureRenderTarget2D*& OutTextureRenderTarget2D);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Window Top Most", Keywords = "is, window, top, most"), Category = "Frozen Forest|Window System|Check")
	virtual bool IsWindowTopMost(bool bUseNative = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bring Window Front", ToolTip = "It brings UE SWindow to front.", Keywords = "bring, window, front"), Category = "Frozen Forest|Window System|Set")
	virtual bool BringWindowFront(bool bFlashWindow);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Top Most Option", ToolTip = "Description.", Keywords = "set, window, positon, location, move"), Category = "Frozen Forest|Window System|Set")
	virtual bool ToggleTopMostOption();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Show On Task Bar", Keywords = "toggle, show, taskbar, hide"), Category = "Frozen Forest|Window System|Set")
	virtual bool ToggleShowOnTaskBar(bool In_bShowOnTaskBar);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Opacity", Keywords = "set, all, window, windows, opacity"), Category = "Frozen Forest|Window System|Set")
	virtual bool ToggleOpacity(bool bEnable, bool bPassDragDrop);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Window Opacity", Keywords = "set, window, opacity"), Category = "Frozen Forest|Window System|Set")
	virtual bool SetWindowOpacity(float NewOpacity);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Window State", Keywords = "set, window, state"), Category = "Frozen Forest|Window System|Set")
	virtual bool SetWindowState(EWindowState OutWindowState);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Window Shape", ToolTip = "Description.", Keywords = "set, window, shape"), Category = "Frozen Forest|Window System|Set")
	virtual bool SetWindowShape(FMargin InExtend, float InDuration, float NewOpacity);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Window Position", ToolTip = "Description.", Keywords = "set, window, positon, location, move"), Category = "Frozen Forest|Window System|Set")
	virtual bool SetWindowPosition(FVector2D InNewPosition);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Window Title", ToolTip = "Description.", Keywords = "set, window, title"), Category = "Frozen Forest|Window System|Set")
	virtual bool SetWindowTitle(FText InNewTitle);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Window State", Keywords = "get, window, state"), Category = "Frozen Forest|Window System|Get")
	virtual bool GetWindowState(EWindowState& OutWindowState);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Window Position", Keywords = "get, window, position, location"), Category = "Frozen Forest|Window System|Get")
	virtual bool GetWindowPosition(FVector2D& OutPosition);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Window Title", Keywords = "get, window, title"), Category = "Frozen Forest|Window System|Get")
	virtual bool GetWindowTitle(FText& OutWindowTitle);

};