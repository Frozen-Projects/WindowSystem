// Fill out your copyright notice in the Description page of Project Settings.

#include "WindowManager.h"

// Custom Includes.
#include "WindowInstance.h"		// CloseAllWindows -> Destrow window actor.

void UFF_WindowSubystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnWorldTickStart.AddUObject(this, &UFF_WindowSubystem::OnWorldTickStart);
}

void UFF_WindowSubystem::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	if (IsValid(World))
	{
		this->CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

		if (IsValid(this->CustomViewport))
		{
			this->DetectLayoutChanges();
		}

		this->AddDragDropHandlerToMV();
	}
}

void UFF_WindowSubystem::Deinitialize()
{
	Super::Deinitialize();

	this->RemoveDragDropHandlerFromMV();
	this->CloseAllWindows();

	if (this->MouseHook_Color)
	{
		UnhookWindowsHookEx(MouseHook_Color);
	}
}

void UFF_WindowSubystem::AddDragDropHandlerToMV()
{
	if (!this->CustomViewport)
	{
		return;
	}

	HWND WindowHandle = reinterpret_cast<HWND>(this->CustomViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle());

	DragAcceptFiles(WindowHandle, true);

	FWindowsApplication* WindowApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();

	if (WindowApplication)
	{
		WindowApplication->AddMessageHandler(DragDropHandler);
	}
}

void UFF_WindowSubystem::RemoveDragDropHandlerFromMV()
{
	FWindowsApplication* WindowsApplication = (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();

	if (WindowsApplication)
	{
		WindowsApplication->RemoveMessageHandler(DragDropHandler);
	}
}

bool UFF_WindowSubystem::CloseAllWindows()
{
	if (this->MAP_Windows.IsEmpty())
	{
		return false;
	}

	for (TPair<FName, AEachWindow_SWindow*> EachPair : this->MAP_Windows)
	{
		AEachWindow_SWindow* EachWindow = EachPair.Value;
		if (IsValid(EachWindow))
		{
			EachWindow->Destroy();
		}
	}

	this->MAP_Windows.Empty();
	return true;
}

void UFF_WindowSubystem::Toggle_Color_Picker()
{
	if (this->MouseHook_Color)
	{
		UnhookWindowsHookEx(MouseHook_Color);
		this->MouseHook_Color = NULL;
	}

	else
	{
		thread_local UFF_WindowSubystem* WindowSubsystem = this;

		auto Callback_Hook = [](int nCode, WPARAM wParam, LPARAM lParam)->LRESULT
			{
				if (wParam == WM_LBUTTONDOWN && IsValid(WindowSubsystem))
				{
					HWND ScreenHandle = GetDesktopWindow();
					if (!ScreenHandle)
					{
						UE_LOG(LogTemp, Warning, TEXT("Read Screen Color -> Error -> Screen Handle"));
						return CallNextHookEx(0, nCode, wParam, lParam);
					}

					HDC ScreenContext = GetDC(ScreenHandle);
					if (!ScreenContext)
					{
						UE_LOG(LogTemp, Warning, TEXT("Read Screen Color -> Error -> Screen Context"));
						return CallNextHookEx(0, nCode, wParam, lParam);
					}

					POINT RawPos;
					bool GotCursorPos = GetCursorPos(&RawPos);
					if (!GotCursorPos)
					{
						UE_LOG(LogTemp, Warning, TEXT("Read Screen Color -> Error -> Got Cursor Pos : %d"), GetLastError());
						return CallNextHookEx(0, nCode, wParam, lParam);
					}

					COLORREF RawColor = GetPixel(ScreenContext, RawPos.x, RawPos.y);
					FLinearColor PositionColor = FLinearColor();
					PositionColor.R = GetRValue(RawColor);
					PositionColor.G = GetGValue(RawColor);
					PositionColor.B = GetBValue(RawColor);
					PositionColor.A = 255;

					ReleaseDC(ScreenHandle, ScreenContext);
					WindowSubsystem->OnCursorPosColor.Broadcast(FVector2D(RawPos.x, RawPos.y), PositionColor);
				}

				return CallNextHookEx(0, nCode, wParam, lParam);
			};

		this->MouseHook_Color = SetWindowsHookEx(WH_MOUSE_LL, Callback_Hook, NULL, 0);
	}
}

bool UFF_WindowSubystem::IsColorPickerActive()
{
	return this->MouseHook_Color ? true : false;
}

bool UFF_WindowSubystem::ToggleWindowState(FName InTargetWindow, bool bFlashWindow)
{
	if (InTargetWindow.IsNone())
	{
		return false;
	}

	if (InTargetWindow.ToString().IsEmpty())
	{
		return false;
	}

	AEachWindow_SWindow* TargetWindow = *this->MAP_Windows.Find(InTargetWindow);

	if (!TargetWindow)
	{
		return false;
	}

	EWindowState WindowState = EWindowState::None;
	if (!TargetWindow->GetWindowState(WindowState))
	{
		return false;
	}

	switch (WindowState)
	{
		case EWindowState::None:

			return false;

		case EWindowState::Minimized:

			TargetWindow->SetWindowOpacity(1.f);
			TargetWindow->ToggleOpacity(false, true);
			TargetWindow->SetWindowState(EWindowState::Restored);
			TargetWindow->BringWindowFront(bFlashWindow);

			return true;

		case EWindowState::Restored:

			TargetWindow->SetWindowOpacity(1.f);
			TargetWindow->ToggleOpacity(false, true);

			if (TargetWindow->IsWindowTopMost())
			{
				TargetWindow->SetWindowState(EWindowState::Minimized);
			}

			else
			{
				TargetWindow->BringWindowFront(bFlashWindow);
			}

			return true;

		case EWindowState::Maximized:

			TargetWindow->SetWindowOpacity(1.f);
			TargetWindow->ToggleOpacity(false, true);

			if (TargetWindow->IsWindowTopMost())
			{
				TargetWindow->SetWindowState(EWindowState::Minimized);
			}

			else
			{
				TargetWindow->BringWindowFront(bFlashWindow);
			}

			return true;

		default:

			return false;
	}
}

bool UFF_WindowSubystem::BringFrontOnHover(AEachWindow_SWindow* TargetWindow)
{
	if (!TargetWindow)
	{
		return false;
	}

	if (this->HoveredWindow == TargetWindow)
	{
		return false;
	}

	TargetWindow->SetWindowOpacity(1.f);
	TargetWindow->ToggleOpacity(false, true);

	for (TPair<FName, AEachWindow_SWindow*> Each_Pair : this->MAP_Windows)
	{
		AEachWindow_SWindow* EachWindow = Each_Pair.Value;

		if (IsValid(EachWindow) && this->HoveredWindow != TargetWindow)
		{
			EWindowState WindowState = EWindowState::None;
			EachWindow->GetWindowState(WindowState);

			if (WindowState == EWindowState::Minimized)
			{
				continue;
			}

			EachWindow->ToggleOpacity(true, true);
			EachWindow->SetWindowOpacity(0.5f);
		}
	}

	this->HoveredWindow = TargetWindow;

	return true;
}

bool UFF_WindowSubystem::CompareViews(TMap<FVector2D, FVector2D> A, TMap<FVector2D, FVector2D> B)
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

void UFF_WindowSubystem::DetectLayoutChanges()
{
	if (!this->CustomViewport)
	{
		return;
	}

	this->CustomViewport->DelegateNewLayout.AddLambda([this](const TArray<FPlayerViews>& Views)
		{
			this->ChangeBackgroundOnNewPlayer(Views);
			this->OnLayoutChanged.Broadcast(Views);
		}
	);
}

void UFF_WindowSubystem::ChangeBackgroundOnNewPlayer(TArray<FPlayerViews> const& Out_Views)
{
	if (!this->CustomViewport)
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("Custom viewport is not valid !"));
		}

		return;
	}

	if (Out_Views.IsEmpty())
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("Views are empty !"));
		}

		return;
	}

	if (!IsValid(this->MAT_BG))
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("Background material is not valid !"));
		}

		return;
	}

	if (!IsValid(this->MAT_Brush))
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("Brush material is not valid !"));
		}

		return;
	}

	if (this->CanvasName.IsNone())
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("Canvas name is empty !"));
		}

		return;
	}

	FVector2D ViewportSize = FVector2D();
	this->CustomViewport->GetViewportSize(ViewportSize);

	if (ViewportSize == FVector2D(0.f))
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("Viewport size shouldn't be zero !"));
		}

		return;
	}

	TMap<FVector2D, FVector2D> Temp_Views;

	for (const FPlayerViews Each_View : Out_Views)
	{
		const FVector2D ActualPosition = ViewportSize * Each_View.Position;
		const FVector2D ActualSize = ViewportSize * Each_View.Size;

		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("Full Size = %s ; ActualPosition = %s, ActualSize = %s // UV Position = %s ; UV Size = %s"), *ViewportSize.ToString(), *ActualPosition.ToString(), *ActualSize.ToString(), *Each_View.Position.ToString(), *Each_View.Size.ToString());

		}

		Temp_Views.Add(ActualPosition, ActualSize);
	}

	if (Temp_Views.IsEmpty())
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("Actual size map is empty !"));
		}

		return;
	}

	if (this->CompareViews(this->MAP_Views, Temp_Views))
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("Views are not changed !"));
		}

		return;
	}

	this->MAP_Views = Temp_Views;
	const bool RetVal = UWindowSystemBPLibrary::SetBackgroundMaterial(this->MAT_BG, this->MAT_Brush, this->CanvasName, this->MAP_Views);

	if (RetVal)
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Warning, TEXT("Background changed successfully."));
		}
	}

	else
	{
		if (this->bEnableDebugLogs)
		{
			UE_LOG(LogTemp, Error, TEXT("There was a problem while changing background !"));
		}
	}
}