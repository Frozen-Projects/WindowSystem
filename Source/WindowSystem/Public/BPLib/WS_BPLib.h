// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BPLib/WS_BPLib_Delegates.h"
#include "Window/Window_Includes.h"

#include "WS_BPLib.generated.h"

UCLASS()
class WINDOWSYSTEM_API UWindowSystemBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

#pragma region Internal_Functions

	static std::wstring UTF8ToWide(FString InString);
	static void SelectFileFromDialog_Internal(FSelectedFiles& OutFileNames, const FString& InDialogName, const FString& InOkLabel, FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs = true, bool bAllowFolderSelection = false);
	static bool SaveFileDialog_Internal(FString& SavePath, const FString& InDialogName, const FString& InOkLabel, FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs = true);

#pragma endregion Internal_Functions

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Main Window Title", Keywords = "get, window, title, main"), Category = "Frozen Forest|Window System|Get")
	static FText GetMainWindowTitle();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Main Window Position", ToolTip = "Set Main Window Position", Keywords = "set, main, window, position"), Category = "Frozen Forest|Window System|Set")
	static void SetMainWindowPosition(FVector2D InNewPosition);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Select File From Dialog", ToolTip = "If you enable \"Allow Folder Selection\", extension filtering will be disabled. \nExtension filtering uses a String to String MAP variable. \nKey is description and value is extension's itself. You need to write like this without quotes \"*.extension\". \nIf one extension group has multiple extensions, you need to use \";\" after each one.", Keywords = "select, file, folder, dialog, windows, explorer"), Category = "Frozen Forest|Window System|File Dialog")
	static void SelectFileFromDialog(FDelegateOpenFile DelegateFileNames, const FString& InDialogName, const FString& InOkLabel, FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs = true, bool bAllowFolderSelection = false);

	/*
	* This function only give you the path of the file you want to save. You need to handle the saving process by yourself.
	* This is an Async function. Don't use it right after the game start. Wait couple seconds to make sure the game is fully loaded.
	* @param InExtensions Each extension group must have only one extension. If that group has multiple variation, you should define them one by one. Writing style should be "*.extension" without quotes.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save File with Dialog", ToolTip = "", Keywords = "save, file, dialog, windows, explorer"), Category = "Frozen Forest|Window System|File Dialog")
	static void SaveFileDialog(FDelegateSaveFile DelegateSaveFile, const FString& InDialogName, const FString& InOkLabel, FString InDefaultPath, TMap<FString, FString> InExtensions, int32 DefaultExtensionIndex, bool bIsNormalizeOutputs = true);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Posses Local Player", ToolTip = "This is customized version of UGameViewportClient::SSSwapControllers which works on Shipping Builds.\nIf controller id is \"-1\", it will use main player's controller id which is probably 0. But else, it will use given index.", Keywords = "assign, new, controller, player, local"), Category = "Frozen Forest|Window System")
	static bool PossesLocalPlayer(const int32 PlayerId, const int32 ControllerId = 0);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Change Player View Size", ToolTip = "", Keywords = "change, player, view, viewport, size, position, ratio"), Category = "Frozen Forest|Window System")
	static bool ChangePlayerViewSize(const int32 PlayerId, FVector2D NewRatio, FVector2D NewOrigin);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Widget State", ToolTip = "", Keywords = "toggle, switch, widget, state, visible, hidden, collapse"), Category = "Frozen Forest|Window System")
	static bool ToggleWidgetState(UWidget* TargetWidget, ESlateVisibility OffMethod = ESlateVisibility::Collapsed);

	/*
	* @param Views Key = View position ; Value = View size.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Background Material", Keywords = "set, background, material, layout, customize, splitscreen, viewport"), Category = "Frozen Forest|Window System")
	static bool SetBackgroundMaterial_BP(UMaterialInterface* In_MAT_BG, UMaterialInterface* In_MAT_Cut, UMaterialInterface* In_MAT_Frame, FName In_CRT_Name = "Canvas", int32 In_Thickness = 10);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Toggle Background", ToolTip = "We suggest you to use this when there is only one view and it is fullscreen.", Keywords = "background, layout, customize, splitscreen, viewport, toggle"), Category = "Frozen Forest|Window System")
	static bool ToggleBackground(bool bActive = true);

};