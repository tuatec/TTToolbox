

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TTToolboxBlueprintLibrary.generated.h"

UCLASS()
class TTTOOLBOX_API UTTToolboxBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// virtual bone functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpVirtualBones(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddVirtualBone(const FName& VirtualBoneName, const FName& SourceBoneName, const FName& TargetBoneName, USkeleton* Skeleton);


	// socket functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpSockets(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool AddSocket(const FName& BoneName, const FName& SocketName, const FTransform& RelativeTransform, USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool HasSocket(const FName& SocketName, USkeleton* Skeleton);
};
