// Copyright Epic Games, Inc. All Rights Reserved.

#include "WindowSystemBPLibrary.h"

#include "Kismet/KismetRenderingLibrary.h"
#include "CustomViewport.h"

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

void UWindowSystemBPLibrary::SelectFileFromDialog(FDelegateOpenFile DelegateFileNames, const FString InDialogName, const FString InOkLabel, const FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs, bool bAllowFolderSelection)
{
	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [DelegateFileNames, InDialogName, InOkLabel, InDefaultPath, InExtensions, DefaultExtensionIndex, bIsNormalizeOutputs, bAllowFolderSelection]()
		{
			IFileOpenDialog* FileOpenDialog;
			HRESULT FileDialogInstance = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&FileOpenDialog));

			IShellItemArray* ShellItems;
			TArray<FString> Array_FilePaths;

			// If dialog instance successfully created.
			if (SUCCEEDED(FileDialogInstance))
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

				FileOpenDialog->SetFileTypes(ExtensionCount, ExtensionArray);

				// Starts from 1
				FileOpenDialog->SetFileTypeIndex(DefaultExtensionIndex + 1);

				DWORD dwOptions;
				FileOpenDialog->GetOptions(&dwOptions);

				if (bAllowFolderSelection == true)
				{
					// https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions
					FileOpenDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_OKBUTTONNEEDSINTERACTION);
				}

				else
				{
					// https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-_fileopendialogoptions
					FileOpenDialog->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_OKBUTTONNEEDSINTERACTION);
				}

				if (InDialogName.IsEmpty() != true)
				{
					FileOpenDialog->SetTitle(*InDialogName);
				}

				if (InOkLabel.IsEmpty() != true)
				{
					FileOpenDialog->SetOkButtonLabel(*InOkLabel);
				}

				if (InDefaultPath.IsEmpty() != true)
				{
					FString DefaultPathString = InDefaultPath;

					FPaths::MakePlatformFilename(DefaultPathString);

					IShellItem* DefaultFolder = NULL;
					HRESULT DefaultPathResult = SHCreateItemFromParsingName(*DefaultPathString, nullptr, IID_PPV_ARGS(&DefaultFolder));

					if (SUCCEEDED(DefaultPathResult))
					{
						FileOpenDialog->SetFolder(DefaultFolder);
						DefaultFolder->Release();
					}
				}

				HWND WindowHandle = reinterpret_cast<HWND>(GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle());
				FileDialogInstance = FileOpenDialog->Show(WindowHandle);

				// If dialog successfully showed up.
				if (SUCCEEDED(FileDialogInstance))
				{
					FileDialogInstance = FileOpenDialog->GetResults(&ShellItems);

					// Is results got.
					if (SUCCEEDED(FileDialogInstance))
					{
						DWORD ItemCount;
						ShellItems->GetCount(&ItemCount);

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

						AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, Array_FilePaths, bAllowFolderSelection]()
							{
								if (Array_FilePaths.IsEmpty() == false)
								{
									FSelectedFiles SelectedFiles;
									SelectedFiles.IsSuccessfull = true;
									SelectedFiles.IsFolder = bAllowFolderSelection;
									SelectedFiles.Strings = Array_FilePaths;

									DelegateFileNames.ExecuteIfBound(SelectedFiles);
								}

								else
								{
									FSelectedFiles SelectedFiles;
									SelectedFiles.IsSuccessfull = false;
									SelectedFiles.IsFolder = bAllowFolderSelection;

									DelegateFileNames.ExecuteIfBound(SelectedFiles);
								}
							}
						);

					}

					// Function couldn't get results.
					else
					{
						AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, ShellItems, FileOpenDialog, bAllowFolderSelection]()
							{
								FileOpenDialog->Release();
								CoUninitialize();

								FSelectedFiles SelectedFiles;
								SelectedFiles.IsSuccessfull = false;
								SelectedFiles.IsFolder = bAllowFolderSelection;

								DelegateFileNames.ExecuteIfBound(SelectedFiles);
							}
						);
					}
				}

				// Dialog didn't show up.
				else
				{
					AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, FileOpenDialog, ShellItems, bAllowFolderSelection]()
						{
							FileOpenDialog->Release();
							CoUninitialize();

							FSelectedFiles SelectedFiles;
							SelectedFiles.IsSuccessfull = false;
							SelectedFiles.IsFolder = bAllowFolderSelection;

							DelegateFileNames.ExecuteIfBound(SelectedFiles);
						}
					);
				}
			}

			// Function couldn't create dialog.
			else
			{
				AsyncTask(ENamedThreads::GameThread, [DelegateFileNames, FileOpenDialog, ShellItems, bAllowFolderSelection]()
					{
						FileOpenDialog->Release();
						CoUninitialize();

						FSelectedFiles SelectedFiles;
						SelectedFiles.IsSuccessfull = false;
						SelectedFiles.IsFolder = bAllowFolderSelection;

						DelegateFileNames.ExecuteIfBound(SelectedFiles);
					}
				);
			}
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
	UCustomViewport* CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

	if (!CustomViewport)
	{
		return false;
	}

	return CustomViewport->PossesLocalPlayer(PlayerId, ControllerId);
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

bool UWindowSystemBPLibrary::SetBackgroundMaterial(UMaterialInterface* MAT_BG, UMaterialInterface* MAT_Brush, FName CRT_Name, TMap<FVector2D, FVector2D> Views)
{
	if (Views.IsEmpty())
	{
		return false;
	}

	if (!IsValid(MAT_BG) || !IsValid(MAT_Brush))
	{
		return false;
	}

	UCustomViewport* CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

	if (!CustomViewport)
	{
		return false;
	}

	UWorld* World = GEngine->GetCurrentPlayWorld();

	if (!IsValid(World))
	{
		return false;
	}

	FVector2D CRT_Size;
	CustomViewport->GetViewportSize(CRT_Size);

	if (CRT_Size == FVector2D(0.f))
	{
		return false;
	}

	UCanvasRenderTarget2D* CRT = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(World, UCanvasRenderTarget2D::StaticClass(), CRT_Size.X, CRT_Size.Y);

	if (!IsValid(CRT))
	{
		return false;
	}

	UCanvas* Canvas = nullptr;
	FVector2D Size = FVector2D();
	FDrawToRenderTargetContext Context = FDrawToRenderTargetContext();
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(World, CRT, Canvas, Size, Context);

	for (const TPair<FVector2D, FVector2D> Each_View : Views)
	{
		Canvas->K2_DrawMaterial(MAT_Brush, Each_View.Key, Each_View.Value, FVector2D(0.f), FVector2D(1.f));
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(World, Context);

	UMaterialInstanceDynamic* MI_BG = UMaterialInstanceDynamic::Create(MAT_BG, World);
	MI_BG->SetTextureParameterValue(CRT_Name, CRT);

	return CustomViewport->SetBackgrounMaterial(MI_BG);
}

bool UWindowSystemBPLibrary::ToggleBackground(bool bStop)
{
	UCustomViewport* CustomViewport = Cast<UCustomViewport>(GEngine->GameViewport.Get());

	if (!CustomViewport)
	{
		return false;
	}

	CustomViewport->ToggleBackground(bStop);
	return true;
}
