#pragma once

#include "CoreMinimal.h"

#include "CustomViewport_Structs.generated.h"

USTRUCT(BlueprintType)
struct WINDOWSYSTEM_API FPlayerViews
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite)
    FVector2D Size = FVector2D();

    UPROPERTY(BlueprintReadWrite)
    FVector2D Position = FVector2D();

	bool operator == (const FPlayerViews& Other) const
	{
		return Size == Other.Size && Position == Other.Position;
	}

	bool operator != (const FPlayerViews& Other) const
	{
		return !(*this == Other);
	}
};

FORCEINLINE uint32 GetTypeHash(const FPlayerViews& Key)
{
	uint32 Hash_Size = GetTypeHash(Key.Size);
	uint32 Hash_Position = GetTypeHash(Key.Position);

	uint32 GenericHash;
	FMemory::Memset(&GenericHash, 0, sizeof(uint32));
	GenericHash = HashCombine(GenericHash, Hash_Size);
	GenericHash = HashCombine(GenericHash, Hash_Position);

	return GenericHash;
}