#pragma once

#include "CoreMinimal.h"
#include "WindowEnums.generated.h"

UENUM(BlueprintType)
enum class EWindowState : uint8
{
	None		UMETA(DisplayName = "None"),
	Minimized	UMETA(DisplayName = "Minimized"),
	Restored	UMETA(DisplayName = "Restored"),
	Maximized	UMETA(DisplayName = "Maximized"),
};
ENUM_CLASS_FLAGS(EWindowState)

UENUM(BlueprintType)
enum class EWindowTypeBp : uint8
{
	Normal				UMETA(DisplayName = "Normal"),
	Menu				UMETA(DisplayName = "Menu"),
	ToolTip				UMETA(DisplayName = "ToolTip"),
	Notification		UMETA(DisplayName = "Notification"),
	CursorDecorator		UMETA(DisplayName = "CursorDecorator"),
};
ENUM_CLASS_FLAGS(EWindowTypeBp)