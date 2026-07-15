#pragma once

#include "CoreMinimal.h"
#include "Viewport/CustomViewport_Structs.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDelegateNewLayout, const TArray<FPlayerViews>&, Out_Views);