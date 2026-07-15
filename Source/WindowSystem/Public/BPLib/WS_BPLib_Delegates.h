#pragma once

#include "CoreMinimal.h"

#include "BPLib/WS_BPLib_Structs.h"

#include "WS_BPLib_Delegates.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_OneParam(FDelegateOpenFile, FSelectedFiles, OutFileNames);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FDelegateSaveFile, bool, bIsSaveSuccessful, FString, OutFileName);