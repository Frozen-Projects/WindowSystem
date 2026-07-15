#pragma once

#include "CoreMinimal.h"

#include "Window_Structs.generated.h"

USTRUCT(BlueprintType)
struct WINDOWSYSTEM_API FDroppedFileStruct
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FString FilePath;

	UPROPERTY(BlueprintReadWrite)
	FVector2D DropLocation = FVector2D();

	UPROPERTY(BlueprintReadWrite)
	bool bIsFolder = false;

	bool operator == (const FDroppedFileStruct& Other) const
	{
		return FilePath == Other.FilePath && DropLocation == Other.DropLocation && bIsFolder == Other.bIsFolder;
	}

	bool operator != (const FDroppedFileStruct& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FDroppedFileStruct& Key)
{
	uint32 Hash_FilePath = GetTypeHash(Key.FilePath);
	uint32 Hash_DropLocation = GetTypeHash(Key.DropLocation);
	uint32 Hash_bIsFolder = GetTypeHash(Key.bIsFolder);

	uint32 GenericHash;
	FMemory::Memset(&GenericHash, 0, sizeof(uint32));
	GenericHash = HashCombine(GenericHash, Hash_FilePath);
	GenericHash = HashCombine(GenericHash, Hash_DropLocation);
	GenericHash = HashCombine(GenericHash, Hash_bIsFolder);

	return GenericHash;
}
