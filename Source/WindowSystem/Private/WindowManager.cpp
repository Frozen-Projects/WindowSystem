// Fill out your copyright notice in the Description page of Project Settings.

#include "WindowManager.h"

// Custom Includes.
#include "WindowInstance.h"		// CloseAllWindows -> Destrow window actor.

void UFF_WindowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	this->TickStartHandle = FWorldDelegates::OnWorldTickStart.AddUObject(this, &UFF_WindowSubsystem::OnWorldTickStart);
}

void UFF_WindowSubsystem::Deinitialize()
{
	if (this->MouseHook_Color)
	{
		UnhookWindowsHookEx(MouseHook_Color);
	}

	this->RemoveDragDropHandlerFromMV();
	this->CloseAllWindows();

	Super::Deinitialize();
}

void UFF_WindowSubsystem::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	if (!IsValid(World))
	{
		return;
	}

	this->CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

	if (IsValid(this->CustomViewport))
	{
		this->CustomViewport->DelegateNewLayout.AddLambda([this](const TArray<FPlayerViews>& Views)
			{
				this->ChangeBackgroundOnNewPlayer(Views);
				this->OnLayoutChanged.Broadcast(Views);
			}
		);
	}

	this->AddDragDropHandlerToMV();

	if (this->TickStartHandle.IsValid())
	{
		FWorldDelegates::OnWorldTickStart.Remove(TickStartHandle);
		TickStartHandle.Reset();
	}
}

void UFF_WindowSubsystem::AddDragDropHandlerToMV()
{
	HWND WindowHandle = reinterpret_cast<HWND>(GEngine->GameViewport.Get()->GetWindow()->GetNativeWindow()->GetOSWindowHandle());

	DragAcceptFiles(WindowHandle, true);

	FSlateApplication& SlateApplication = FSlateApplication::Get();

	if (!SlateApplication.IsInitialized() || !SlateApplication.IsActive())
	{
		return;
	}

	TSharedPtr<GenericApplication> GenericApp = SlateApplication.GetPlatformApplication();

	if (!GenericApp.IsValid())
	{
		return;
	}

	FWindowsApplication* WindowsApplication = (FWindowsApplication*)GenericApp.Get();

	if (WindowsApplication)
	{
		WindowsApplication->AddMessageHandler(DragDropHandler);
	}
}

void UFF_WindowSubsystem::RemoveDragDropHandlerFromMV()
{
	FSlateApplication& SlateApplication = FSlateApplication::Get();

	if (!SlateApplication.IsInitialized())
	{
		return;
	}

	if (!SlateApplication.IsActive())
	{
		return;
	}

	TSharedPtr<GenericApplication> GenericApp = SlateApplication.GetPlatformApplication();

	if (!GenericApp.IsValid())
	{
		return;
	}

	FWindowsApplication* WindowsApplication = (FWindowsApplication*)GenericApp.Get();

	if (WindowsApplication)
	{
		WindowsApplication->RemoveMessageHandler(DragDropHandler);
	}
}

bool UFF_WindowSubsystem::CompareViews(TMap<FVector2D, FVector2D> A, TMap<FVector2D, FVector2D> B)
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

void UFF_WindowSubsystem::ChangeBackgroundOnNewPlayer(TArray<FPlayerViews> const& Out_Views)
{
	if (!this->CustomViewport)
	{
		this->LastError = "Custom viewport is not valid !";
		return;
	}

	if (Out_Views.IsEmpty())
	{
		this->LastError = "Views are empty !";
		return;
	}

	if (!IsValid(this->MAT_BG))
	{
		this->LastError = "Background material is not valid !";
		return;
	}

	if (!IsValid(this->MAT_Brush))
	{
		this->LastError = "Brush material is not valid !";
		return;
	}

	if (this->CanvasName.IsNone())
	{
		this->LastError = "Canvas name is empty !";
		return;
	}

	FVector2D ViewportSize = FVector2D();
	this->CustomViewport->GetViewportSize(ViewportSize);

	if (ViewportSize == FVector2D(0.f))
	{
		this->LastError = "Viewport size shouldn't be zero !";
		return;
	}

	TMap<FVector2D, FVector2D> Temp_Views;

	for (const FPlayerViews Each_View : Out_Views)
	{
		const FVector2D ActualPosition = ViewportSize * Each_View.Position;
		const FVector2D ActualSize = ViewportSize * Each_View.Size;
		Temp_Views.Add(ActualPosition, ActualSize);

		TMap<FString, FVector2D> Temp_ViewLayout;
		Temp_ViewLayout.Add("Full Size", ViewportSize);
		Temp_ViewLayout.Add("Actual Position", ActualPosition);
		Temp_ViewLayout.Add("Actual Size", ActualSize);
		Temp_ViewLayout.Add("UV Position", Each_View.Position);
		Temp_ViewLayout.Add("UV Size", Each_View.Size);
		this->ViewLayout = MoveTemp(Temp_ViewLayout);
	}

	if (Temp_Views.IsEmpty())
	{
		this->LastError = "Actual size map is empty !";
		return;
	}

	if (this->CompareViews(this->MAP_Views, Temp_Views))
	{
		this->LastError = "Views are not changed !";
		return;
	}

	this->MAP_Views = MoveTemp(Temp_Views);
	const bool RetVal = UWindowSystemBPLibrary::SetBackgroundMaterial(this->MAT_BG, this->MAT_Brush, this->CanvasName, this->MAP_Views);

	if (!RetVal)
	{
		this->LastError = "There was a problem while changing background !";
	}
}

bool UFF_WindowSubsystem::CloseAllWindows()
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

bool UFF_WindowSubsystem::ToggleWindowState(FName InTargetWindow, bool bFlashWindow)
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

bool UFF_WindowSubsystem::BringFrontOnHover(AEachWindow_SWindow* TargetWindow)
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

void UFF_WindowSubsystem::Toggle_Color_Picker(bool& bIsActive)
{
	if (this->MouseHook_Color)
	{
		UnhookWindowsHookEx(MouseHook_Color);
		this->MouseHook_Color = NULL;

		bIsActive = false;
		return;
	}

	else
	{
		thread_local FColorPickerStruct ColorPickerContainer = FColorPickerStruct();

		auto Callback_Hook = [](int nCode, WPARAM wParam, LPARAM lParam)->LRESULT
			{
				if (wParam == WM_LBUTTONDOWN)
				{
					HWND ScreenHandle = GetDesktopWindow();

					if (!ScreenHandle)
					{
						UE_LOG(LogTemp, Error, TEXT("Read Screen Color -> Error -> Screen Handle is not valid !"));
						return CallNextHookEx(0, nCode, wParam, lParam);
					}

					HDC ScreenContext = GetDC(ScreenHandle);
					if (!ScreenContext)
					{
						UE_LOG(LogTemp, Error, TEXT("Read Screen Color -> Error -> Screen Context is not valid !"));
						return CallNextHookEx(0, nCode, wParam, lParam);
					}

					POINT RawPos;
					if (!GetCursorPos(&RawPos))
					{
						UE_LOG(LogTemp, Warning, TEXT("Read Screen Color -> Error -> Got Cursor Pos : %d"), GetLastError());
					}

					COLORREF RawColor = GetPixel(ScreenContext, RawPos.x, RawPos.y);
					FLinearColor PositionColor = FLinearColor();
					PositionColor.R = GetRValue(RawColor);
					PositionColor.G = GetGValue(RawColor);
					PositionColor.B = GetBValue(RawColor);
					PositionColor.A = 255;

					ColorPickerContainer.Color = PositionColor;
					ColorPickerContainer.Position = FVector2D(RawPos.x, RawPos.y);

					ReleaseDC(ScreenHandle, ScreenContext);
				}

				return CallNextHookEx(0, nCode, wParam, lParam);
			};

		this->LastPickedColor = MoveTemp(ColorPickerContainer);

		this->MouseHook_Color = SetWindowsHookEx(WH_MOUSE_LL, Callback_Hook, NULL, 0);
		bIsActive = true;
	}
}

bool UFF_WindowSubsystem::IsColorPickerActive()
{
	return this->MouseHook_Color ? true : false;
}

FString UFF_WindowSubsystem::ViewLayoutLog()
{
	FString OutString = "";
	
	for (TPair<FString, FVector2D> EachPair : this->ViewLayout)
	{
		OutString += EachPair.Key + EachPair.Value.ToString() + "\n";
	}
	
	return OutString;
}