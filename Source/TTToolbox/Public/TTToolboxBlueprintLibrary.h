

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IKRigDefinition.h"
#include "TTToolboxBlueprintLibrary.generated.h"

// forward declarations
class USkeleton;
class UIKRigDefinition;


USTRUCT(Blueprintable)
struct TTTOOLBOX_API FBoneChain_BP
{
	GENERATED_BODY()

	FBoneChain_BP() = default;

	FBoneChain_BP(const FBoneChain& BoneChain)
		: ChainName(BoneChain.ChainName)
		, StartBone(BoneChain.StartBone.BoneName)
		, EndBone(BoneChain.EndBone.BoneName)
		, IKGoalName(BoneChain.IKGoalName)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoneChain)
	FName ChainName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoneChain)
	FName StartBone = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoneChain)
	FName EndBone = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = IK)
	FName IKGoalName = NAME_None;
};

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


	// skeleton functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool DumpSkeletonCurveNames(USkeleton* Skeleton);

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool CheckForMissingCurveNames(const TArray<FName>& CurveNamesToCheck, USkeleton* Skeleton);


	// AnimMontage functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
	static bool CopyAnimMontageCurves(UAnimMontage* SourceAnimMontage, UAnimMontage* TargetAnimMontage);


	// IK Rig functions

	UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool DumpIKChains(const UIKRigDefinition* IKRigDefinition);

  UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool AddIKBoneChains(UIKRigDefinition* IKRigDefinition, const TArray<FBoneChain_BP>& BoneChains);

  UFUNCTION(BlueprintCallable, Category = "TTToolbox")
  static bool SetIKBoneChainGoal(UIKRigDefinition* IKRigDefinition, const FName& ChainName, const FName& GoalName);
};
