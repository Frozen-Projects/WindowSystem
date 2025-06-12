// Fill out your copyright notice in the Description page of Project Settings.

#include "WindowInstance.h"

// Sets default values.
AEachWindow_SWindow::AEachWindow_SWindow()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called every frame.
void AEachWindow_SWindow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the game starts or when spawned.
void AEachWindow_SWindow::BeginPlay()
{
	Super::BeginPlay();

	this->WindowSubsystem = GEngine->GetCurrentPlayWorld()->GetGameInstance()->GetSubsystem<UFF_WindowSubystem>();

	if (!IsValid(WindowSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Window creation aborted because \"Manager\" are not valid for: %s"), *FString(WindowTag.ToString()));
		this->Destroy();
		return;
	}

	if (!IsValid(ContentWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Window creation aborted because \"ContentWidget\" are not valid for: %s"), *FString(WindowTag.ToString()));
		this->Destroy();
		return;
	}

	// If there is a problem with window creation, actor class will be destroyed and won't go any further. 
	if (!this->CreateNewWindow())
	{
		UE_LOG(LogTemp, Warning, TEXT("Window creation aborted because there is a problem in \"CreateNewWindow\" function for: %s"), *FString(WindowTag.ToString()));
		this->Destroy();
		return;
	}

	// Add created window actor class to the list.
	WindowSubsystem->MAP_Windows.Add(WindowTag, this);

	// Start window hover detection.

	if (this->bEnableHoverDetection)
	{
		Hover_Delegate = FTimerDelegate::CreateUObject(this, &AEachWindow_SWindow::NotifyWindowHovered, false);
		GEngine->GetCurrentPlayWorld()->GetTimerManager().SetTimer(Hover_Timer, Hover_Delegate, this->HoverDetectionTime, true);
	}
}

// Called when the game ends.
void AEachWindow_SWindow::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Hover_Timer.IsValid())
	{
		Hover_Timer.Invalidate();
	}
	
	if (IsValid(this->WindowSubsystem))
	{
		this->WindowSubsystem->OnWindowClosed.Broadcast(WindowTag);
	}

	this->CloseWindowCallback();

	Super::EndPlay(EndPlayReason);
}

// Protected Functions.

void AEachWindow_SWindow::NotifyWindowClosed(const TSharedRef<SWindow>& Window)
{
	if (IsValid(this))
	{
		this->Destroy();
	}
}

void AEachWindow_SWindow::NotifyWindowMoved(const TSharedRef<SWindow>& Window)
{
	if (!IsValid(this))
	{
		return;
	}

	if (!IsValid(this->WindowSubsystem))
	{
		return;
	}

	if (!this->WindowPtr.IsValid())
	{
		return;
	}

	this->WindowSubsystem->OnWindowMoved.Broadcast(this);
}

void AEachWindow_SWindow::NotifyWindowHovered(bool bUseDirectHover)
{
	if (!IsValid(this))
	{
		return;
	}

	if (!IsValid(this->WindowSubsystem))
	{
		return;
	}

	if (!this->WindowPtr.IsValid())
	{
		return;
	}

	if (!(bUseDirectHover ? WindowPtr.ToSharedRef().Get().IsDirectlyHovered() : WindowPtr.ToSharedRef().Get().IsHovered()))
	{
		return;
	}

	this->WindowSubsystem->OnWindowHovered.Broadcast(this);
}

bool AEachWindow_SWindow::CreateNewWindow()
{
	if (!IsValid(this->WindowSubsystem))
	{
		UE_LOG(LogTemp, Error, TEXT("Window manager is not valid : %s"), *FString(WindowTag.ToString()));
		return false;
	}

	if (WindowTag.IsNone() || WindowTag.ToString().IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Window tag is empty."));
		return false;
	}

	if (this->WindowSubsystem->MAP_Windows.Contains(WindowTag))
	{
		UE_LOG(LogTemp, Error, TEXT("There is a window with that tag."));
		return false;
	}

	// Styles
	EWindowType WindowType = EWindowType::GameWindow;
	switch (WindowTypeBp)
	{
		case EWindowTypeBp::Normal:
			WindowType = EWindowType::Normal;
			break;
		case EWindowTypeBp::Menu:
			WindowType = EWindowType::Menu;
			break;
		case EWindowTypeBp::ToolTip:
			WindowType = EWindowType::ToolTip;
			break;
		case EWindowTypeBp::Notification:
			WindowType = EWindowType::Notification;
			break;
		case EWindowTypeBp::CursorDecorator:
			WindowType = EWindowType::CursorDecorator;
			break;
		case EWindowTypeBp::GameWindow:
			WindowType = EWindowType::GameWindow;
			break;
	}

	// Blueprints exposed UObject should contain TSharedPtr NOT TSharedRef.
	TSharedPtr<SWindow> WidgetWindow = SNew(SWindow)
		.bDragAnywhere(true)
		.ClientSize(WindowSize)
		.LayoutBorder(BorderThick)
		.UserResizeBorder(BorderThick)
		.Title(InWindowTitle)
		.ToolTipText(InToolTip)
		.ForceVolatile(bForceVolatile)
		.ShouldPreserveAspectRatio(bPreserveAspectRatio)
		.IsInitiallyMinimized(bMinimized)
		.FocusWhenFirstShown(true)
		.HasCloseButton(bHasClose)
		.SupportsMinimize(bSupportsMinimized)
		.SupportsMaximize(bSupportsMaximized)
		.SupportsTransparency(EWindowTransparency::PerWindow)
		.IsTopmostWindow(bIsTopMost)
		.Type(WindowType)
		.UseOSWindowBorder(bUseNativeBorder)
		.AdjustInitialSizeAndPositionForDPIScale(true)
		;

	if (!WidgetWindow.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Widget window pointer is not valid : %s"), *FString(WindowTag.ToString()));
		return false;
	}

	FWindowSizeLimits SizeLimits = FWindowSizeLimits();
	SizeLimits.SetMinWidth(MinSize.X);
	SizeLimits.SetMinHeight(MinSize.Y);

	if (MaxSize.X == 0)
	{
		MaxSize.X = GEngine->GetGameUserSettings()->GetScreenResolution().X;
	}

	else
	{
		SizeLimits.SetMaxWidth(MaxSize.X);
	}

	if (MaxSize.Y == 0)
	{
		MaxSize.Y = GEngine->GetGameUserSettings()->GetScreenResolution().Y;
	}

	else
	{
		SizeLimits.SetMaxHeight(MaxSize.Y);
	}

	WidgetWindow->SetContent(ContentWidget->TakeWidget());
	WidgetWindow->SetAllowFastUpdate(true);
	WidgetWindow->SetMirrorWindow(bSetMirrorWindow);
	WidgetWindow->MoveWindowTo(StartPosition);
	WidgetWindow->SetTag(WindowTag);
	WidgetWindow->SetNativeWindowButtonsVisibility(bHasClose);
	WidgetWindow->SetForegroundColor(TitleColor);
	WidgetWindow->SetSizeLimits(SizeLimits);
	WidgetWindow->SetOnWindowMoved(FOnWindowClosed::CreateUObject(this, &AEachWindow_SWindow::NotifyWindowMoved));
	WidgetWindow->SetOnWindowClosed(FOnWindowClosed::CreateUObject(this, &AEachWindow_SWindow::NotifyWindowClosed));

	// Add created window to Slate.
	FSlateApplication::Get().AddWindow(WidgetWindow.ToSharedRef(), true);

	// If we use this and hide window from taskbar, minimized window will still be visible at left corner as small form.
	//FSlateApplication::Get().AddWindowAsNativeChild(WidgetWindow.ToSharedRef(), GEngine->GameViewport->GetWindow().ToSharedRef());

	// Hide Window from Taskbar.
	HWND WidgetWindowHandle = reinterpret_cast<HWND>(WidgetWindow.ToSharedRef().Get().GetNativeWindow().ToSharedRef().Get().GetOSWindowHandle());

	if (bShowOnTaskBar == true)
	{
		SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TRANSPARENT);
	}

	else
	{
		SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_NOACTIVATE | WS_EX_TRANSPARENT);
	}

	this->WindowPtr = WidgetWindow;

	return true;
}

void AEachWindow_SWindow::CloseWindowCallback()
{
	if (IsValid(ContentWidget))
	{
		ContentWidget->ReleaseSlateResources(true);
	}

	if (WindowPtr.IsValid())
	{
		WindowPtr->HideWindow();
		WindowPtr->RequestDestroyWindow();
		WindowPtr.Reset();
	}

	if (IsValid(this->WindowSubsystem) && this->WindowSubsystem->MAP_Windows.Contains(WindowTag))
	{
		this->WindowSubsystem->MAP_Windows.Remove(WindowTag);
	}
}

// UFUNCTIONS.

bool AEachWindow_SWindow::SetFileDragDropSupport()
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	if (bIsFileDropEnabled)
	{
		return false;
	}

	bIsFileDropEnabled = true;

	HWND WidgetWindowHandle = reinterpret_cast<HWND>(WindowPtr.ToSharedRef().Get().GetNativeWindow().ToSharedRef().Get().GetOSWindowHandle());
	DragAcceptFiles(WidgetWindowHandle, true);

	return true;
}

bool AEachWindow_SWindow::TakeSSWindow(UTextureRenderTarget2D*& OutTextureRenderTarget2D)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	AEachWindow_SWindow::BringWindowFront(false);

	FVector2D TargetWindowSize = WindowPtr->GetClientSizeInScreen();
	UTextureRenderTarget2D* TextureTarget = FWidgetRenderer::CreateTargetFor(TargetWindowSize, TextureFilter::TF_Default, false);

	FWidgetRenderer* WidgetRenderer = new FWidgetRenderer(true);
	WidgetRenderer->DrawWidget(TextureTarget, ContentWidget->TakeWidget(), TargetWindowSize, 0, false);

	if (!IsValid(TextureTarget))
	{
		return false;
	}

	OutTextureRenderTarget2D = TextureTarget;
	return true;
}

bool AEachWindow_SWindow::IsWindowTopMost(bool bUseNative)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	if (bUseNative == true)
	{
		HWND WidgetWindowHandle = reinterpret_cast<HWND>(WindowPtr.ToSharedRef().Get().GetNativeWindow().ToSharedRef().Get().GetOSWindowHandle());

		if (GetWindowLong(WidgetWindowHandle, GWL_EXSTYLE) & WS_EX_TOPMOST)
		{
			return true;
		}

		else
		{
			return false;
		}
	}

	else
	{
		return WindowPtr->IsTopmostWindow();
	}
}

bool AEachWindow_SWindow::BringWindowFront(bool bFlashWindow)
{
	if (WindowPtr.IsValid() == false)
	{
		return false;
	}

	WindowPtr.Get()->BringToFront();

	if (bFlashWindow == true)
	{
		WindowPtr.Get()->FlashWindow();
	}

	return true;
}

bool AEachWindow_SWindow::ToggleTopMostOption()
{
	if (WindowPtr.IsValid() == false)
	{
		return false;
	}

	HWND WidgetWindowHandle = reinterpret_cast<HWND>(WindowPtr.ToSharedRef().Get().GetNativeWindow().ToSharedRef().Get().GetOSWindowHandle());

	if (GetWindowLong(WidgetWindowHandle, GWL_EXSTYLE) & WS_EX_TOPMOST)
	{
		SetWindowPos(WidgetWindowHandle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return true;
	}

	else
	{
		SetWindowPos(WidgetWindowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return true;
	}
}

bool AEachWindow_SWindow::ToggleShowOnTaskBar(bool In_bShowOnTaskBar)
{
	if (WindowPtr.IsValid() == false)
	{
		return false;
	}

	HWND WidgetWindowHandle = reinterpret_cast<HWND>(WindowPtr.ToSharedRef().Get().GetNativeWindow().ToSharedRef().Get().GetOSWindowHandle());

	if (In_bShowOnTaskBar == true)
	{
		if (this->bIsTransparent == true)
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
		}

		else
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW);
		}

		AEachWindow_SWindow::BringWindowFront(true);

		this->bShowOnTaskBar = In_bShowOnTaskBar;
	}

	else
	{
		if (this->bIsTransparent == true)
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED);
		}

		else
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_NOACTIVATE);
		}

		this->bShowOnTaskBar = In_bShowOnTaskBar;
	}

	return true;
}

bool AEachWindow_SWindow::ToggleOpacity(bool bEnable, bool bPassDragDrop)
{
	if (WindowPtr.IsValid() == false)
	{
		return false;
	}

	if (bIsFileDropEnabled == true && bPassDragDrop == false)
	{
		return false;
	}

	HWND WidgetWindowHandle = reinterpret_cast<HWND>(WindowPtr.ToSharedRef().Get().GetNativeWindow().ToSharedRef().Get().GetOSWindowHandle());

	if (bEnable)
	{
		if (bShowOnTaskBar == true)
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
		}

		else
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED);
		}

		bIsTransparent = true;
	}

	else
	{
		if (bShowOnTaskBar == true)
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TRANSPARENT);
		}

		else
		{
			SetWindowLongPtr(WidgetWindowHandle, GWL_EXSTYLE, WS_EX_NOACTIVATE | WS_EX_TRANSPARENT);
		}

		bIsTransparent = false;
	}

	return true;
}

bool AEachWindow_SWindow::SetWindowOpacity(float NewOpacity)
{
	if (WindowPtr.IsValid() == false)
	{
		return false;
	}

	WindowPtr->SetOpacity(NewOpacity);
	return true;
}

bool AEachWindow_SWindow::SetWindowState(EWindowState OutWindowState)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	switch (OutWindowState)
	{

	case EWindowState::None:
		return false;

	case EWindowState::Minimized:
		WindowPtr->Minimize();
		return true;
		break;

	case EWindowState::Restored:
		WindowPtr->Restore();
		return true;
		break;

	case EWindowState::Maximized:
		WindowPtr->Maximize();
		return true;
		break;
	}

	return true;
}

bool AEachWindow_SWindow::SetWindowShape(FMargin InExtend, float InDuration, float NewOpacity)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	FSlateRect CurrentShape = WindowPtr.ToSharedRef().Get().GetRectInScreen();

	FSlateRect NewShape;
	NewShape.Top = CurrentShape.Top + InExtend.Top;
	NewShape.Left = CurrentShape.Left + InExtend.Left;
	NewShape.Bottom = CurrentShape.Bottom + InExtend.Bottom;
	NewShape.Right = CurrentShape.Right + InExtend.Right;

	FCurveSequence CurveSequence;
	CurveSequence.AddCurve(0, InDuration, ECurveEaseFunction::CubicInOut);

	WindowPtr.ToSharedRef().Get().MorphToShape(CurveSequence, NewOpacity, NewShape);

	return true;
}

bool AEachWindow_SWindow::SetWindowPosition(FVector2D InNewPosition)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	WindowPtr.Get()->MoveWindowTo(InNewPosition);
	return true;
}

bool AEachWindow_SWindow::SetWindowTitle(FText InNewTitle)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	WindowPtr.Get()->SetTitle(InNewTitle);

	return true;
}

bool AEachWindow_SWindow::GetWindowState(EWindowState& OutWindowState)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	WINDOWPLACEMENT WindowPlacement;
	GetWindowPlacement(reinterpret_cast<HWND>(WindowPtr.ToSharedRef().Get().GetNativeWindow().ToSharedRef().Get().GetOSWindowHandle()), &WindowPlacement);

	switch (WindowPlacement.showCmd)
	{
		case SW_NORMAL:

			OutWindowState = EWindowState::Restored;
			return true;

		case SW_MAXIMIZE:

			OutWindowState = EWindowState::Maximized;
			return true;

		case SW_SHOWMINIMIZED:

			OutWindowState = EWindowState::Minimized;
			return true;
	}

	return false;
}

bool AEachWindow_SWindow::GetWindowPosition(FVector2D& OutPosition)
{
	if (!WindowPtr.IsValid())
	{
		return false;
	}

	OutPosition = WindowPtr->GetPositionInScreen();
	return true;
}

bool AEachWindow_SWindow::GetWindowTitle(FText& OutWindowTitle)
{
	if (!WindowPtr.IsValid())
	{
		OutWindowTitle = FText::FromString(TEXT(""));
		return false;
	}

	OutWindowTitle = WindowPtr.ToSharedRef().Get().GetTitle();
	return true;
}