// The MIT License (MIT)
// ---------------------
// 
// Copyright 2022 Achim Turan (https://www.instagram.com/tuatec/)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
