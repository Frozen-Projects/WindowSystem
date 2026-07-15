#pragma once

#include "CoreMinimal.h"

#include "WS_BPLib_Structs.generated.h"

USTRUCT(BlueprintType)
struct WINDOWSYSTEM_API FSelectedFiles
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly)
	bool IsSuccessfull = false;

	UPROPERTY(BlueprintReadOnly)
	bool IsFolder = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Paths;

	bool operator == (const FSelectedFiles& Other) const
	{
		return IsSuccessfull == Other.IsSuccessfull && IsFolder == Other.IsFolder && Paths == Other.Paths;
	}

	bool operator != (const FSelectedFiles& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FSelectedFiles& Key)
{
	uint32 Hash_IsSuccessfull = GetTypeHash(Key.IsSuccessfull);
	uint32 Hash_IsFolder = GetTypeHash(Key.IsFolder);
	uint32 Hash_Paths = GetTypeHash(Key.Paths);

	uint32 GenericHash;
	FMemory::Memset(&GenericHash, 0, sizeof(uint32));
	GenericHash = HashCombine(GenericHash, Hash_IsSuccessfull);
	GenericHash = HashCombine(GenericHash, Hash_IsFolder);
	GenericHash = HashCombine(GenericHash, Hash_Paths);

	return GenericHash;
}