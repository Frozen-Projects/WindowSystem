#include "DragDropHandler.h"
#include "WindowManager.h"

bool FDragDropHandler::ProcessMessage(HWND Hwnd, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult)
{
	TObjectPtr<UGameViewportClient> Viewport = GEngine->GameViewport;

	if (!IsValid(Viewport))
	{
		return false;
	}

	UWorld* World = Viewport->GetWorld();

	if (!IsValid(World))
	{
		return false;
	}

	UFF_WindowSubystem* WindowSubsystem = World->GetSubsystem<UFF_WindowSubystem>();

	if (!IsValid(WindowSubsystem))
	{
		return false;
	}

	HWND MainWindowHandle;
	HDROP DropInfo = (HDROP)WParam;

	// Drop Location.
	POINT DropLocation;

	// Out Structure.
	FDroppedFileStruct DropFileStruct;
	TArray<FDroppedFileStruct> OutArray;

	// Read Regedit To Get Windows Build Number.
	HKEY hKey;
	LONG Result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
	DWORD BufferSize;
	RegQueryValueEx(hKey, L"CurrentBuildNumber", 0, nullptr, NULL, &BufferSize);
	TCHAR* Buffer = (TCHAR*)malloc(BufferSize);
	RegQueryValueEx(hKey, L"CurrentBuildNumber", 0, nullptr, reinterpret_cast<LPBYTE>(Buffer), &BufferSize);
	int32 BuildNumber = FCString::Atoi(Buffer);
	
	free(Buffer);
	Buffer = nullptr;

	switch (Message)
	{
		case WM_ERASEBKGND:
			return false;

		case WM_PAINT:
		{
			if (BuildNumber >= 22000)
			{
				/*
					* Window Roundness Preference.
					* DWMWCP_DEFAULT = 0
					* DWMWCP_DONOTROUND = 1
					* DWMWCP_ROUND = 2
					* DWMWCP_ROUNDSMALL = 3
				*/
				DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_ROUND;
				DwmSetWindowAttribute(Hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
			}

			return true;
		}
		
		case WM_DROPFILES:
		{
			// If message sender window is main window and user not want to get files on it, return false.
			if (WindowSubsystem->bAllowMainWindow == false)
			{
				MainWindowHandle = reinterpret_cast<HWND>(GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle());
				if (Hwnd == MainWindowHandle)
				{
					return false;
				}
			}

			DragQueryPoint(DropInfo, &DropLocation);

			const UINT DroppedFileCount = DragQueryFileA(DropInfo, 0xFFFFFFFF, NULL, NULL);
			for (UINT FileIndex = 0; FileIndex < DroppedFileCount; FileIndex++)
			{
				UINT PathSize = DragQueryFileA(DropInfo, FileIndex, NULL, 0);
				if (PathSize > 0)
				{
					DropFileStruct.DropLocation = FVector2D(DropLocation.x, DropLocation.y);

					char* DroppedFile = (char*)malloc(size_t(PathSize));
					DragQueryFileA(DropInfo, FileIndex, DroppedFile, PathSize + 1);

					if (GetFileAttributesA(DroppedFile) != FILE_ATTRIBUTE_DIRECTORY)
					{
						DropFileStruct.FilePath = DroppedFile;
						DropFileStruct.bIsFolder = false;
					}

					if (GetFileAttributesA(DroppedFile) == FILE_ATTRIBUTE_DIRECTORY)
					{
						DropFileStruct.FilePath = DroppedFile;
						DropFileStruct.bIsFolder = true;
					}

					OutArray.Add(DropFileStruct);

					free(DroppedFile);
					DroppedFile = nullptr;
				}
			}

			WindowSubsystem->OnFileDrop.Broadcast(OutArray);
			OutArray.Empty();

			DragFinish(DropInfo);

			return true;
		}

		default:
			return false;
	}
}