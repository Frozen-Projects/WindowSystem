// Fill out your copyright notice in the Description page of Project Settings.

#include "WindowManager.h"

// Custom Includes.
#include "WindowInstance.h"		// CloseAllWindows -> Destrow window actor.

TWeakObjectPtr<UFF_WindowSubsystem> UFF_WindowSubsystem::SelfReference;

void UFF_WindowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	this->WorldTickStartHandle = FWorldDelegates::OnWorldTickStart.AddUObject(this, &UFF_WindowSubsystem::OnWorldTickStart);
}

void UFF_WindowSubsystem::Deinitialize()
{
	if (this->Hook_LMB)
	{
		UnhookWindowsHookEx(Hook_LMB);
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
				this->DelegateLayoutChanged.Broadcast(Views);
			}
		);
	}

	this->AddDragDropHandlerToMV();

	if (this->WorldTickStartHandle.IsValid())
	{
		FWorldDelegates::OnWorldTickStart.Remove(this->WorldTickStartHandle);
		this->WorldTickStartHandle.Reset();
	}
}

void UFF_WindowSubsystem::InitMouseHook()
{
	if (this->Hook_LMB)
	{
		UnhookWindowsHookEx(Hook_LMB);
		this->Hook_LMB = NULL;

		return;
	}

	SelfReference = this;

	this->DelegateLMBHook.AddDynamic(this, &UFF_WindowSubsystem::OnViewportDetected);

	auto Callback_Hook = [](int nCode, WPARAM wParam, LPARAM lParam)->LRESULT
		{
			if (wParam == WM_LBUTTONDOWN)
			{
				if (!SelfReference.IsValid())
				{
					const FString ErrorMsg = "WindowSystem : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : SelfReference is not valid !";
					UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMsg);
				}

				HWND ScreenHandle = GetDesktopWindow();

				if (!ScreenHandle)
				{
					const FString ErrorMsg = "WindowSystem : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Failed to get Screen Handle !";
					UE_LOG(LogTemp, Error, TEXT("%s : %d"), *ErrorMsg, GetLastError());

					return CallNextHookEx(0, nCode, wParam, lParam);
				}

				HDC ScreenContext = GetDC(ScreenHandle);
				if (!ScreenContext)
				{
					const FString ErrorMsg = "WindowSystem : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Failed to get Screen Context !";
					UE_LOG(LogTemp, Error, TEXT("%s : %d"), *ErrorMsg, GetLastError());

					return CallNextHookEx(0, nCode, wParam, lParam);
				}

				POINT RawPos;
				if (!GetCursorPos(&RawPos))
				{
					const FString ErrorMsg = "WindowSystem : " + FString(ANSI_TO_TCHAR(__FUNCSIG__)) + " : Failed to get Cursor Position !";
					UE_LOG(LogTemp, Error, TEXT("%s : %d"), *ErrorMsg, GetLastError());

					ReleaseDC(ScreenHandle, ScreenContext);
					return CallNextHookEx(0, nCode, wParam, lParam);
				}

				COLORREF RawColor = GetPixel(ScreenContext, RawPos.x, RawPos.y);
				FLinearColor PositionColor = FLinearColor();
				PositionColor.R = GetRValue(RawColor);
				PositionColor.G = GetGValue(RawColor);
				PositionColor.B = GetBValue(RawColor);
				PositionColor.A = 255;

				ReleaseDC(ScreenHandle, ScreenContext);

				AsyncTask(ENamedThreads::GameThread, [RawPos, PositionColor]()
					{
						if (SelfReference.IsValid())
						{
							SelfReference->DelegateLMBHook.Broadcast(FVector2D(RawPos.x, RawPos.y), PositionColor);
						}
					}
				);	
			}

			return CallNextHookEx(0, nCode, wParam, lParam);
		};

	this->Hook_LMB = SetWindowsHookEx(WH_MOUSE_LL, Callback_Hook, NULL, 0);
}

void UFF_WindowSubsystem::OnViewportDetected(FVector2D In_Position, FLinearColor In_Color)
{
	if (!IsValid(this->CustomViewport))
	{
		return;
	}
	
	FVector2D ViewportSize = FVector2D();
	this->CustomViewport->GetViewportSize(ViewportSize);

	if (ViewportSize.X == 0 || ViewportSize.Y == 0)
	{
		return;
	}

	const TArray<ULocalPlayer*>& Temp_Player_List = GEngine->GetGamePlayers(this->CustomViewport);
	const int32 Player_Count = Temp_Player_List.Num();

	if (Player_Count == 1)
	{
		//const FVector2D Origin_1 = Temp_Player_List[0]->Origin;
		//this->CustomViewport->FrameTarget = Origin_1 * ViewportSize;

		return;
	}

	if (Player_Count == 2)
	{
		const FVector2D Size_1 = Temp_Player_List[0]->Size;
		const FVector2D Origin_1 = Temp_Player_List[0]->Origin;
		const FVector2D TopLeft_1 = Origin_1 * ViewportSize;
		const FVector2D BottomRight_1 = (Origin_1 + Size_1) * ViewportSize;

		const FVector2D Size_2 = Temp_Player_List[1]->Size;
		const FVector2D Origin_2 = Temp_Player_List[1]->Origin;
		const FVector2D TopLeft_2 = Origin_2 * ViewportSize;
		const FVector2D BottomRight_2 = (Origin_2 + Size_2) * ViewportSize;

		// Player 0
		if (In_Position.X >= TopLeft_1.X && In_Position.X <= BottomRight_1.X && In_Position.Y >= TopLeft_1.Y && In_Position.Y <= BottomRight_1.Y)
		{
			if (this->ActualPlayerIndex == 0)
			{
				this->CustomViewport->FrameTarget = Origin_1 * ViewportSize;
				this->CustomViewport->InitTextures();
			}

			else if (this->ActualPlayerIndex == 1)
			{
				this->CustomViewport->FrameTarget = Origin_1 * ViewportSize;
				this->CustomViewport->InitTextures();

				UWindowSystemBPLibrary::PossesLocalPlayer(1, -1);
				this->ActualPlayerIndex = 0;
			}
		}

		// Player 1
		else if (In_Position.X >= TopLeft_2.X && In_Position.X <= BottomRight_2.X && In_Position.Y >= TopLeft_2.Y && In_Position.Y <= BottomRight_2.Y)
		{
			if (this->ActualPlayerIndex == 0)
			{
				this->CustomViewport->FrameTarget = Origin_2 * ViewportSize;
				this->CustomViewport->InitTextures();

				UWindowSystemBPLibrary::PossesLocalPlayer(1, -1);
				this->ActualPlayerIndex = 1;
			}

			else if (this->ActualPlayerIndex == 1)
			{
				this->CustomViewport->FrameTarget = Origin_2 * ViewportSize;
				this->CustomViewport->InitTextures();
			}
		}

		return;
	}

	else if (Player_Count == 3)
	{
		// To be implemented.
	}

	else if (Player_Count == 4)
	{
		// To be implemented.
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