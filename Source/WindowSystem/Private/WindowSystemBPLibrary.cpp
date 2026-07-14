// Copyright Epic Games, Inc. All Rights Reserved.

#include "WindowSystemBPLibrary.h"
#include "WindowSystem.h"

UWindowSystemBPLibrary::UWindowSystemBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

void UWindowSystemBPLibrary::SetMainWindowPosition(FVector2D InNewPosition)
{
	GEngine->GameViewport->GetWindow().ToSharedRef().Get().MoveWindowTo(InNewPosition);
}

FText UWindowSystemBPLibrary::GetMainWindowTitle()
{
	return GEngine->GameViewport->GetWindow().ToSharedRef().Get().GetTitle();
}

std::wstring UWindowSystemBPLibrary::UTF8ToWide(FString InString)
{
	if (InString.IsEmpty())
	{
		return {};
	}

	const auto UTF8Converter = StringCast<UTF8CHAR>(*InString);
	const char* UTF8Data = reinterpret_cast<const char*>(UTF8Converter.Get());
	const int32 UTF8Length = UTF8Converter.Length();

	const int RequiredSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, UTF8Data, UTF8Length, nullptr, 0);

	if (RequiredSize <= 0)
	{
		return {};
	}

	std::wstring WideString(static_cast<size_t>(RequiredSize), L'\0');

	const int ConvertedSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, UTF8Data, UTF8Length, WideString.data(), RequiredSize);

	if (ConvertedSize != RequiredSize)
	{
		return {};
	}

	return WideString;
}

void UWindowSystemBPLibrary::SelectFileFromDialog_Internal(FSelectedFiles& OutFileNames, const FString& InDialogName, const FString& InOkLabel, FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs, bool bAllowFolderSelection)
{
	IFileOpenDialog* FileOpenDialog;
	HRESULT FileDialogInstance = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&FileOpenDialog));

	if (!SUCCEEDED(FileDialogInstance))
	{
		FileOpenDialog->Release();
		CoUninitialize();

		OutFileNames.IsSuccessfull = false;
		OutFileNames.IsFolder = bAllowFolderSelection;
		
		return;
	}

	TArray<std::wstring> FilterNames;
	TArray<std::wstring> FilterPatterns;
	TArray<COMDLG_FILTERSPEC> FilterSpecs;

	const int32 ExtensionCount = InExtensions.Num();

	FilterNames.Reserve(ExtensionCount);
	FilterPatterns.Reserve(ExtensionCount);
	FilterSpecs.Reserve(ExtensionCount);

	for (const TPair<FString, FString>& Each_Extension : InExtensions)
	{
		FilterNames.Add(UTF8ToWide(Each_Extension.Key));
		FilterPatterns.Add(UTF8ToWide(Each_Extension.Value));
	}

	for (int32 FilterIndex = 0; FilterIndex < ExtensionCount; ++FilterIndex)
	{
		COMDLG_FILTERSPEC EachFilterSpec{};
		EachFilterSpec.pszName = FilterNames[FilterIndex].c_str();
		EachFilterSpec.pszSpec = FilterPatterns[FilterIndex].c_str();

		FilterSpecs.Add(EachFilterSpec);
	}

	FileOpenDialog->SetFileTypes(InExtensions.Num(), FilterSpecs.GetData());

	// Starts from 1
	FileOpenDialog->SetFileTypeIndex(DefaultExtensionIndex + 1);

	DWORD dwOptions;
	FileOpenDialog->GetOptions(&dwOptions);

	// https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions
	FileOpenDialog->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_OKBUTTONNEEDSINTERACTION | (bAllowFolderSelection ? FOS_PICKFOLDERS : NULL));

	if (!InDialogName.IsEmpty())
	{
		FileOpenDialog->SetTitle(*InDialogName);
	}

	if (!InOkLabel.IsEmpty())
	{
		FileOpenDialog->SetOkButtonLabel(*InOkLabel);
	}

	if (!InDefaultPath.IsEmpty())
	{
		FPaths::MakePlatformFilename(InDefaultPath);

		IShellItem* DefaultFolder = NULL;
		HRESULT DefaultPathResult = SHCreateItemFromParsingName(*InDefaultPath, nullptr, IID_PPV_ARGS(&DefaultFolder));

		if (SUCCEEDED(DefaultPathResult))
		{
			FileOpenDialog->SetFolder(DefaultFolder);
			DefaultFolder->Release();
		}
	}

	HWND WindowHandle = reinterpret_cast<HWND>(GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle());
	FileDialogInstance = FileOpenDialog->Show(WindowHandle);

	if (!SUCCEEDED(FileDialogInstance))
	{
		FileOpenDialog->Release();
		CoUninitialize();

		OutFileNames.IsSuccessfull = false;
		OutFileNames.IsFolder = bAllowFolderSelection;
		return;
	}

	IShellItemArray* ShellItems;
	FileDialogInstance = FileOpenDialog->GetResults(&ShellItems);

	if (!SUCCEEDED(FileDialogInstance))
	{
		FileOpenDialog->Release();
		CoUninitialize();

		OutFileNames.IsSuccessfull = false;
		OutFileNames.IsFolder = bAllowFolderSelection;
		return;
	}

	DWORD ItemCount;
	ShellItems->GetCount(&ItemCount);

	TArray<FString> Array_FilePaths;

	for (DWORD ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++)
	{
		IShellItem* EachItem;
		ShellItems->GetItemAt(ItemIndex, &EachItem);

		PWSTR EachFilePathSTR = NULL;
		EachItem->GetDisplayName(SIGDN_FILESYSPATH, &EachFilePathSTR);

		FString EachFilePath = EachFilePathSTR;

		if (bIsNormalizeOutputs == true)
		{
			FPaths::NormalizeFilename(EachFilePath);
		}

		Array_FilePaths.Add(EachFilePath);

		EachItem->Release();
	}

	ShellItems->Release();
	FileOpenDialog->Release();
	CoUninitialize();

	OutFileNames.IsSuccessfull = !Array_FilePaths.IsEmpty();
	OutFileNames.IsFolder = bAllowFolderSelection;
	OutFileNames.Strings = Array_FilePaths;
}

void UWindowSystemBPLibrary::SelectFileFromDialog(FDelegateOpenFile DelegateFileNames, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs, bool bAllowFolderSelection)
{
	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateFileNames, InDialogName, InOkLabel, InDefaultPath, InExtensions, DefaultExtensionIndex, bIsNormalizeOutputs, bAllowFolderSelection]()
		{
			FSelectedFiles OutFileNames;
			UWindowSystemBPLibrary::SelectFileFromDialog_Internal(OutFileNames, InDialogName, InOkLabel, InDefaultPath, InExtensions, DefaultExtensionIndex, bIsNormalizeOutputs, bAllowFolderSelection);

			AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, OutFileNames]()
				{
					DelegateFileNames.ExecuteIfBound(OutFileNames);
				}
			);
		}
	);
}

void UWindowSystemBPLibrary::SaveFileDialog(FDelegateSaveFile DelegateSaveFile, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs)
{
	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateSaveFile, InDialogName, InOkLabel, InDefaultPath, InExtensions, DefaultExtensionIndex, bIsNormalizeOutputs]()
		{
			IFileSaveDialog* SaveFileDialog;
			HRESULT SaveDialogInstance = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&SaveFileDialog));

			IShellItem* ShellItem;

			// If dialog instance successfully created.
			if (SUCCEEDED(SaveDialogInstance))
			{
				// https://stackoverflow.com/questions/70174174/c-com-comdlg-filterspec-array-overrun
				int32 ExtensionCount = InExtensions.Num();
				COMDLG_FILTERSPEC* ExtensionArray = new COMDLG_FILTERSPEC[ExtensionCount];
				COMDLG_FILTERSPEC* EachExtension = ExtensionArray;

				TArray<FString> ExtensionKeys;
				InExtensions.GetKeys(ExtensionKeys);

				TArray<FString> ExtensionValues;
				InExtensions.GenerateValueArray(ExtensionValues);

				for (int32 ExtensionIndex = 0; ExtensionIndex < ExtensionCount; ExtensionIndex++)
				{
					EachExtension->pszName = *ExtensionKeys[ExtensionIndex];
					EachExtension->pszSpec = *ExtensionValues[ExtensionIndex];
					++EachExtension;
				}

				SaveFileDialog->SetFileTypes(ExtensionCount, ExtensionArray);

				// Starts from 1
				SaveFileDialog->SetFileTypeIndex(DefaultExtensionIndex + 1);

				DWORD dwOptions;
				SaveFileDialog->GetOptions(&dwOptions);

				if (InDialogName.IsEmpty() != true)
				{
					SaveFileDialog->SetTitle(*InDialogName);
				}

				if (InOkLabel.IsEmpty() != true)
				{
					SaveFileDialog->SetOkButtonLabel(*InOkLabel);
				}

				if (InDefaultPath.IsEmpty() != true)
				{
					FString DefaultPathString = InDefaultPath;

					FPaths::MakePlatformFilename(DefaultPathString);

					IShellItem* DefaultFolder = NULL;
					HRESULT DefaultPathResult = SHCreateItemFromParsingName(*DefaultPathString, nullptr, IID_PPV_ARGS(&DefaultFolder));

					if (SUCCEEDED(DefaultPathResult))
					{
						SaveFileDialog->SetFolder(DefaultFolder);
						DefaultFolder->Release();
					}
				}

				HWND WindowHandle = reinterpret_cast<HWND>(GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle());
				SaveDialogInstance = SaveFileDialog->Show(WindowHandle);

				// Dialog didn't show up.
				if (SUCCEEDED(SaveDialogInstance))
				{
					SaveFileDialog->GetResult(&ShellItem);

					UINT FileTypeIndex = 0;
					SaveFileDialog->GetFileTypeIndex(&FileTypeIndex);
					FString ExtensionString = ExtensionValues[FileTypeIndex - 1];

					TArray<FString> ExtensionParts;
					ExtensionString.ParseIntoArray(ExtensionParts, TEXT("."), true);

					PWSTR pFileName;
					ShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pFileName);

					FString FilePath = FString(pFileName) + TEXT(".") + ExtensionParts[1];

					if (bIsNormalizeOutputs == true)
					{
						FPaths::NormalizeFilename(FilePath);
					}

					ShellItem->Release();
					SaveFileDialog->Release();
					CoUninitialize();

					AsyncTask(ENamedThreads::GameThread, [DelegateSaveFile, FilePath]()
						{
							DelegateSaveFile.ExecuteIfBound(true, FilePath);
						}
					);
				}

				else
				{
					AsyncTask(ENamedThreads::GameThread, [DelegateSaveFile]()
						{
							DelegateSaveFile.ExecuteIfBound(false, TEXT(""));
						}
					);
				}
			}

			// Function couldn't create dialog.
			else
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateSaveFile]()
					{
						DelegateSaveFile.ExecuteIfBound(false, TEXT(""));
					}
				);
			}
		}
	);
}

bool UWindowSystemBPLibrary::PossesLocalPlayer(const int32 PlayerId, const int32 ControllerId)
{
	UWorld* World = GEngine->GetCurrentPlayWorld();

	if (!World)
	{
		return false;
	}

	UEngine* const REF_Engine = GEngine->GameViewport->GetGameInstance()->GetEngine();
	const int32 NumPlayers = REF_Engine->GetNumGamePlayers(World);

	if (NumPlayers <= PlayerId)
	{
		return false;
	}

	REF_Engine->GetGamePlayer(World, PlayerId)->SetControllerId(ControllerId);

	return true;
}

bool UWindowSystemBPLibrary::ChangePlayerViewSize(const int32 PlayerId, FVector2D NewRatio, FVector2D NewOrigin)
{
	UCustomViewport* CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

	if (!CustomViewport)
	{
		return false;
	}

	return CustomViewport->ChangePlayerViewSize(PlayerId, NewRatio, NewOrigin);
}

bool UWindowSystemBPLibrary::ToggleWidgetState(UWidget* TargetWidget, ESlateVisibility OffMethod)
{
	if (!TargetWidget)
	{
		return false;
	}

	ESlateVisibility CurrentState = TargetWidget->GetVisibility();

	if (CurrentState == ESlateVisibility::Visible)
	{
		TargetWidget->SetVisibility(OffMethod);
		return true;
	}

	else
	{
		TargetWidget->SetVisibility(ESlateVisibility::Visible);
		return true;
	}
}

bool UWindowSystemBPLibrary::SetBackgroundMaterial_BP(UMaterialInterface* In_MAT_BG, UMaterialInterface* In_MAT_Cut, UMaterialInterface* In_MAT_Frame, FName In_CRT_Name, int32 In_Thickness)
{
	if (!IsValid(In_MAT_BG) || !IsValid(In_MAT_Cut) || !IsValid(In_MAT_Frame) || In_CRT_Name.IsNone())
	{
		return false;
	}

	UCustomViewport* CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

	if (!CustomViewport)
	{
		return false;
	}

	return CustomViewport->SetBackgroundMaterial(In_MAT_BG, In_MAT_Cut, In_MAT_Frame, In_CRT_Name, In_Thickness);
}

bool UWindowSystemBPLibrary::ToggleBackground(bool bActive)
{
	UCustomViewport* CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

	if (!CustomViewport)
	{
		return false;
	}

	CustomViewport->ToggleBackground(bActive);
	return true;
}