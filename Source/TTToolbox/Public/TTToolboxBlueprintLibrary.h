

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TTToolboxBlueprintLibrary.generated.h"

UCLASS()
class TTTOOLBOX_API UTTToolboxBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpVirtualBones(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddVirtualBone(const FName& VirtualBoneName, const FName& SourceBoneName, const FName& TargetBoneName, USkeleton* Skeleton);
};
