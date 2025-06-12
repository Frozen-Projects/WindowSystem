#pragma once

// Unreal Engine Includes.
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"

#include "Widgets/SWindow.h"				// Create Window.
#include "Widgets/SWidget.h"				// Add Widget to Window.
#include "Slate/WidgetRenderer.h"			// Take Screenshot of Window
#include "Runtime/UMG/Public/UMG.h"         // Take Screenshot of Window
#include "Blueprint/UserWidget.h"

#include "Misc/Optional.h"
#include "Framework/Application/SWindowTitleBar.h"

// Custom Includes.
#include "WindowEnums.h"
#include "CustomViewport.h"

THIRD_PARTY_INCLUDES_START
#ifdef _WIN64
#include "Windows/WindowsHWrapper.h"		// Necessary include.
#include "Windows/WindowsApplication.h"		// File Drag Drop Message Handler.
#include "shellapi.h"						// File Drag Drop Callback.
#include "dwmapi.h"							// Windows 11 Rounded Window Include.
#include <winreg.h>                         // Regedit access.
#include "winuser.h"						// Necessary include.
#include "Windows/MinWindows.h"				// Necessary include.
#endif
THIRD_PARTY_INCLUDES_END